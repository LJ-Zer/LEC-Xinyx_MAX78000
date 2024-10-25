/*******************************************************************************
* Copyright (C) Maxim Integrated Products, Inc., All Rights Reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
* OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of Maxim Integrated
* Products, Inc. shall not be used except as stated in the Maxim Integrated
* Products, Inc. Branding Policy.
*
* The mere transfer of this software does not imply any licenses
* of trade secrets, proprietary technology, copyrights, patents,
* trademarks, maskwork rights, or any other form of intellectual
* property whatsoever. Maxim Integrated Products, Inc. retains all
* ownership rights.
*
******************************************************************************/

/**
 * @file    main.c
 * @brief   Parallel camera example with the OV7692.
 *
 * @details This example uses the UART to stream out the image captured from the camera.
 *          The image is prepended with a header that is interpreted by the grab_image.py
 *          python script.  The data from this example through the UART is in a binary format.
 *          Instructions: 1) Load and execute this example. The example will initialize the camera
 *                        and start the repeating binary output of camera frame data.
 *                        2) Run 'sudo grab_image.py /dev/ttyUSB0 115200'
 *                           Substitute the /dev/ttyUSB0 string for the serial port on your system.
 *                           The python program will read in the binary data from this example and
 *                           output a png image.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "uart.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
#include "camera.h"
#include "dma.h"

#include <stdlib.h>
#include <string.h>
#include "mxc.h"
#include "cnn.h"
#include "sampledata.h"

#include "sx126x_commands.h"
#include "gpio.h"

#define IMAGE_XRES  32
#define IMAGE_YRES  32
#define CAMERA_FREQ (10 * 1000 * 1000)

#define N_CH 3

#define MXC_GPIO_PORT_INTERRUPT_IN      MXC_GPIO0
#define MXC_GPIO_PIN_INTERRUPT_IN       MXC_GPIO_PIN_2

// #define MXC_GPIO_PORT_INTERRUPT_IN      MXC_GPIO2
// #define MXC_GPIO_PIN_INTERRUPT_IN       MXC_GPIO_PIN_6

#define MXC_GPIO_PORT_RESET_LORA               MXC_GPIO0
#define MXC_GPIO_PIN_RESET_LORA                MXC_GPIO_PIN_19

#define MXC_GPIO_PORT_BUSY_LORA               MXC_GPIO1
#define MXC_GPIO_PIN_BUSY_LORA                MXC_GPIO_PIN_6

mxc_gpio_cfg_t gpio_interrupt;
mxc_gpio_cfg_t gpio_nres_lora;
mxc_gpio_cfg_t gpio_busy_lora;

volatile uint32_t cnn_time; // Stopwatch

int8_t img_to_nn[IMAGE_XRES*IMAGE_YRES*N_CH];

// *****************************************************************************
int main(void)
{
    LED_On(LED1);
    int ret = 0;
    int slaveAddress;
    int id;
    int dma_channel;

    int digs, tens;

    static int32_t ml_data[CNN_NUM_OUTPUTS];
    static q15_t ml_softmax[CNN_NUM_OUTPUTS];

    int8_t image_bw[IMAGE_XRES*IMAGE_YRES];

    MXC_ICC_Enable(MXC_ICC0); // Enable cache

    // Switch to 100 MHz clock
    // MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    // SystemCoreClockUpdate();

    // Enable peripheral, enable CNN interrupt, turn on CNN clock
    // CNN clock: 50 MHz div 1
    cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK, MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);


    cnn_init(); // Bring state machine into consistent state
    cnn_load_weights(); // Load kernels
    cnn_load_bias();
    cnn_configure(); // Configure state machine

    // Initialize DMA for camera interface
    MXC_DMA_Init();
    dma_channel = MXC_DMA_AcquireChannel();


    gpio_interrupt.port = MXC_GPIO_PORT_INTERRUPT_IN;
    gpio_interrupt.mask = MXC_GPIO_PIN_INTERRUPT_IN;
    gpio_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_Config(&gpio_interrupt);
    //MXC_GPIO_RegisterCallback(&gpio_interrupt, gpio_isr, &gpio_interrupt_status);
    MXC_GPIO_IntConfig(&gpio_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt.port, gpio_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO_PORT_INTERRUPT_IN)));
    MXC_LP_EnableGPIOWakeup(&gpio_interrupt);

    /* Setup LORA reset pin. */
    gpio_nres_lora.port = MXC_GPIO_PORT_RESET_LORA;
    gpio_nres_lora.mask = MXC_GPIO_PIN_RESET_LORA;
    gpio_nres_lora.pad = MXC_GPIO_PAD_NONE;
    gpio_nres_lora.func = MXC_GPIO_FUNC_OUT;
    gpio_nres_lora.vssel = MXC_GPIO_VSSEL_VDDIO;
    MXC_GPIO_Config(&gpio_nres_lora);

    gpio_busy_lora.port = MXC_GPIO_PORT_BUSY_LORA;
    gpio_busy_lora.mask = MXC_GPIO_PIN_BUSY_LORA;
    gpio_busy_lora.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_busy_lora.func = MXC_GPIO_FUNC_IN;
    MXC_GPIO_Config(&gpio_busy_lora);


    // Initialize the camera driver.
    camera_init(CAMERA_FREQ);

    // Setup the camera image dimensions, pixel format and data aquiring details.
    ret = camera_setup(IMAGE_XRES, IMAGE_YRES, PIXFORMAT_RGB888, FIFO_THREE_BYTE, USE_DMA, dma_channel);
    
    // if (ret != STATUS_OK) {
    //     printf("Error returned from setting up camera. Error %d\n", ret);
    //     return -1;
    // }
    
    // Start off the first camera image frame.
    
    MXC_Delay(MSEC(200));

    camera_start_capture_image();


    LED_On(LED1);
    while (1) {
        // Check if image is aquired.
        if (camera_is_image_rcv()) {
            // Process the image, send it through the UART console.
            uint8_t*   raw;
            uint32_t  imgLen;
            uint32_t  w, h;
            
            // Get the details of the image from the camera driver.
            camera_get_image(&raw, &imgLen, &w, &h);

            // MXC_Delay(SEC(1));
            Camera_Power(0);
            LED_Off(LED1);

            // MXC_Delay(SEC(2));

            for(int i=0; i<IMAGE_XRES; i+=1){
            	for(int f=0; f<IMAGE_YRES*N_CH; f+=N_CH){
            		image_bw[i*IMAGE_YRES + f/(N_CH)] = (int8_t)  ( ((float) raw[i*IMAGE_XRES*3 + f])*0.29 + ((float) raw[i*IMAGE_XRES*3 + f + 1])*0.58 + ((float) raw[i*IMAGE_XRES*3 + f + 2])*0.11) - 128;
                    // printf("%d,", image_bw[i*IMAGE_YRES + f/(N_CH)]);
                    // printf("%d,%d,%d,", raw[i*IMAGE_XRES*3 + f], raw[i*IMAGE_XRES*3 + f + 1], raw[i*IMAGE_XRES*3 + f + 2]);
                    // fflush(stdout);
            	}
            }

            // printf("\n");
            

            // for (int i = 0; i < IMAGE_XRES*IMAGE_YRES*N_CH; i++)
            // {
            //     img_to_nn[i] = (int8_t) (((int16_t) raw[i])-128);
            //     // printf("%d,", raw[i]);
            //     // fflush(stdout);
            // }
            
            // fflush(stdout);
            // while(1);

            // memcpy((uint8_t *) 0x50400000, img_to_nn, IMAGE_XRES*IMAGE_YRES*N_CH);
            memcpy((uint8_t *) 0x50400000, image_bw, IMAGE_XRES*IMAGE_YRES);

            cnn_start(); // Start CNN processing

            while (cnn_time == 0)
            __WFI(); // Wait for CNN

            // if (check_output() != CNN_OK) fail();
            // Classification layer
            cnn_unload((uint32_t *) ml_data);
            softmax_q17p14_q15((const q31_t *) ml_data, CNN_NUM_OUTPUTS, ml_softmax);

            uint8_t argmax = 0;
            int max_temp = (1000 * ml_softmax[0] + 0x4000) >> 15;

            // printf("Classification results:\n");
            for (int i = 0; i < CNN_NUM_OUTPUTS; i++) {
                digs = (1000 * ml_softmax[i] + 0x4000) >> 15;

                if(max_temp < digs){
                	max_temp = digs;
                	argmax = i;
                }

                tens = digs % 10;
                digs = digs / 10;
                // printf("[%7d] -> Class %d: %d.%d%%\n", ml_data[i], i, digs, tens);
            }

            printf("%d\n", argmax);
            // Send stuff with LoRa
            MXC_GPIO_OutSet(gpio_nres_lora.port, gpio_nres_lora.mask);
            MXC_Delay(MSEC(1));

            SX126x_Init();

            set_tx(868000000, LORA_BW_500, LORA_SF7, LORA_CR_4_5, LORA_PACKET_VARIABLE_LENGTH, 0x04, 14, RADIO_RAMP_200_US);

            uint8_t payload[4] = {'P', 'I', 'N', 'G'};
            payload[0] = argmax + 48;
            SX126x_SendPayload(payload, 4, 0); // Be careful timeout

            //SX126x_GetStatus();
            MXC_Delay(MSEC(10));
            //SX126x_GetStatus();

            MXC_GPIO_OutClr(gpio_nres_lora.port, gpio_nres_lora.mask);

            // MXC_GCR->pm &= ~MXC_F_GCR_PM_ISO_PD;
            // MXC_LP_EnterMicroPowerMode();
            __WFI(); // Already inside function before

            LED_On(LED1);
            Camera_Power(1);
            camera_reset();
            camera_init(CAMERA_FREQ);
            // Prepare for another frame capture.
            camera_start_capture_image();
        }
    }
    
    return ret;
}
