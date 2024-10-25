#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "fcr_regs.h"
#include "icc.h"
#include "led.h"
#include "tmr.h"
#include "dma.h"
#include "pb.h"
#include "cnn.h"
#include "weights.h"
#include "sampledata.h"
#include "mxc_delay.h"
#include "camera.h"


#include "sx126x_commands.h"
#include "gpio.h"

#define MXC_GPIO_PORT_INTERRUPT_IN      MXC_GPIO0
#define MXC_GPIO_PIN_INTERRUPT_IN       MXC_GPIO_PIN_2
#define MXC_GPIO_PORT_RESET_LORA               MXC_GPIO0
#define MXC_GPIO_PIN_RESET_LORA                MXC_GPIO_PIN_19
#define MXC_GPIO_PORT_BUSY_LORA               MXC_GPIO1
#define MXC_GPIO_PIN_BUSY_LORA                MXC_GPIO_PIN_6
mxc_gpio_cfg_t gpio_interrupt;
mxc_gpio_cfg_t gpio_nres_lora;
mxc_gpio_cfg_t gpio_busy_lora;



#define IMAGE_SIZE_X (64 * 2)
#define IMAGE_SIZE_Y (64 * 2)
#define DATA_SIZE (128 * 128)

static uint32_t input_0[DATA_SIZE];

#define CAMERA_FREQ (5 * 1000 * 1000)

#ifdef BOARD_FTHR_REVA
int image_bitmap_1 = (int)&img_1_rgb565[0];
int image_bitmap_2 = (int)&logo_rgb565[0];
int font_1 = (int)&Liberation_Sans16x16[0];
int font_2 = (int)&Liberation_Sans16x16[0];
#endif

const char classes[CNN_NUM_OUTPUTS][12] = { "cirrus", "cumulus", "nimbostratus", "stratus" };

// Classification layer:
static int32_t ml_data[CNN_NUM_OUTPUTS];
static q15_t ml_softmax[CNN_NUM_OUTPUTS];

volatile uint32_t cnn_time; // Stopwatch

#ifdef USE_SAMPLEDATA
// Data input: HWC 3x128x128 (49152 bytes total / 16384 bytes per channel):
static const uint32_t input_0[] = SAMPLE_INPUT_0; // input data from header file
#else
static uint32_t input_0[IMAGE_SIZE_X * IMAGE_SIZE_Y]; // buffer for camera image
#endif

/* **************************************************************************** */

void fail(void)
{
    printf("\n*** FAIL ***\n\n");

    while (1) {}
}

/* **************************************************************************** */

void cnn_load_input(void)
{
    int i;
    const uint32_t *in0 = input_0;

    for (i = 0; i < 16384; i++) {
        // Remove the following line if there is no risk that the source would overrun the FIFO:
        while (((*((volatile uint32_t *)0x50000004) & 1)) != 0) {}
        // Wait for FIFO 0
        *((volatile uint32_t *)0x50000008) = *in0++; // Write FIFO 0
    }
}

/* **************************************************************************** */

void capture_process_camera(void)
{
    uint8_t *raw;
    uint32_t imgLen;
    uint32_t w, h;

    int cnt = 0;

    uint8_t r, g, b;

    uint8_t *data = NULL;
    stream_stat_t *stat;

    camera_start_capture_image();

    // Get the details of the image from the camera driver.
    camera_get_image(&raw, &imgLen, &w, &h);
    printf("W:%d H:%d L:%d \n", w, h, imgLen);

    // Get image line by line
    for (int row = 0; row < h; row++) {
        // Wait until camera streaming buffer is full
        while ((data = get_camera_stream_buffer()) == NULL) {
            if (camera_is_image_rcv()) {
                break;
            }
        }

        for (int k = 0; k < 4 * w; k += 4) {
            // data format: 0x00bbggrr
            r = data[k];
            g = data[k + 1];
            b = data[k + 2];
            // skip k+3

            // change the range from [0,255] to [-128,127] and store in buffer for CNN
            input_0[cnt++] = ((b << 16) | (g << 8) | r) ^ 0x00808080;
        }

        // Release stream buffer
        release_camera_stream_buffer();
    }

    stat = get_camera_stream_statistic();

    if (stat->overflow_count > 0) {
        printf("OVERFLOW DISP = %d\n", stat->overflow_count);
        LED_On(LED2); // Turn on red LED if overflow detected
        while (1) {}
    }
}

// /* **************************************************************************** */
void printUint32Data(uint32_t *data, size_t length) {
    // Function to convert a 32-bit RGB value to grayscale
    uint32_t rgbToGrayscale(uint32_t rgb) {
        // Extract RGB components from the 32-bit integer
        uint8_t r = (rgb >> 16) & 0xFF;
        uint8_t g = (rgb >> 8) & 0xFF;
        uint8_t b = rgb & 0xFF;

        // Convert to grayscale using the weighted average formula
        uint8_t gray = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
        
        return gray;
    }

    for (size_t i = 0; i < length; i++) {
        // Convert each 32-bit RGB value to grayscale
        uint8_t gray = rgbToGrayscale(data[i]);

        // Print each grayscale value in hexadecimal
        printf("%02X", gray);

        // Print a comma and space if it's not the last value
        if (i < length - 1) {
            printf(", ");
        }

        // Print a new line every 16 values for readability
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");  // Add a new line at the end
}


void send_data_via_lora(uint32_t *data, size_t size)
{
    uint8_t payload[256]; // Ensure this is large enough for your data
    size_t i;
    
    // Convert data to byte array (if needed)
    for (i = 0; i < size; i++) {
        payload[i*4] = (data[i] >> 24) & 0xFF;
        payload[i*4+1] = (data[i] >> 16) & 0xFF;
        payload[i*4+2] = (data[i] >> 8) & 0xFF;
        payload[i*4+3] = data[i] & 0xFF;
    }

    // Prepare LoRa settings
    MXC_GPIO_OutSet(gpio_nres_lora.port, gpio_nres_lora.mask);
    MXC_Delay(MSEC(1));

    SX126x_Init();
    set_tx(868000000, LORA_BW_500, LORA_SF7, LORA_CR_4_5, LORA_PACKET_VARIABLE_LENGTH, 0x04, 14, RADIO_RAMP_200_US);

    // Send the payload
    SX126x_SendPayload(payload, size * 4, 0); // Send payload, adjust size accordingly

    MXC_Delay(MSEC(10));
    MXC_GPIO_OutClr(gpio_nres_lora.port, gpio_nres_lora.mask);
}


/* **************************************************************************** */

int main(void)
{
    int i;
    int digs, tens;
    int result[CNN_NUM_OUTPUTS];
  // ****************************************************************************
    int ret = 0;
  // *****************************************************************************

    int dma_channel;

#if defined(BOARD_FTHR_REVA)
    MXC_Delay(200000);
    Camera_Power(POWER_ON);
    printf("\n\nLAB-EARTH + C - Cloud Sensing Detection\n");
#else
    printf("\n\nLAB-EARTH + C - Cloud Sensing Detection\n");
#endif

    MXC_ICC_Enable(MXC_ICC0);

    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_IPO);
    SystemCoreClockUpdate();

    cnn_enable(MXC_S_GCR_PCLKDIV_CNNCLKSEL_PCLK, MXC_S_GCR_PCLKDIV_CNNCLKDIV_DIV1);
    cnn_boost_enable(MXC_GPIO2, MXC_GPIO_PIN_5);
    cnn_init();
    cnn_load_weights();
    cnn_load_bias();
    cnn_configure();
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
    // MXC_LP_EnableGPIOWakeup(&gpio_interrupt);

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



    printf("Init Camera.\n");
    camera_init(CAMERA_FREQ);

    ret = camera_setup(IMAGE_SIZE_X, IMAGE_SIZE_Y, PIXFORMAT_RGB888, FIFO_THREE_BYTE, STREAMING_DMA,
                       dma_channel);
    if (ret != STATUS_OK) {
        printf("Error returned from setting up camera. Error %d\n", ret);
        return -1;
    }

#ifdef BOARD_EVKIT_V1
    camera_write_reg(0x11, 0x1); // set camera clock prescaler to prevent streaming overflow
#else
    camera_write_reg(0x11, 0x0); // set camera clock prescaler to prevent streaming overflow
#endif

    printf("********** Press PB1(SW1) to capture an image **********\r\n");
    while (!PB_Get(0)) {}

    // Enable CNN clock
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_CNN);

    while (1) {
        LED_Off(LED1);
        LED_Off(LED2);
#ifdef USE_SAMPLEDATA
#else
        capture_process_camera();
#endif

        cnn_start();
        cnn_load_input();

        SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk; // SLEEPDEEP=0
        while (cnn_time == 0) {
            __WFI(); // Wait for CNN interrupt
        }

        // Unload CNN data
        cnn_unload((uint32_t *)ml_data);
        cnn_stop();

        // Softmax
        softmax_q17p14_q15((const q31_t *)ml_data, CNN_NUM_OUTPUTS, ml_softmax);

        printf("Time for CNN: %d us\n\n", cnn_time);

        printf("Classification results:\n");

        for (i = 0; i < CNN_NUM_OUTPUTS; i++) {
            digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
            tens = digs % 10;
            digs = digs / 10;
            result[i] = digs;
            printf("[%7d] -> Class %d %8s: %d.%d%%\n", ml_data[i], i, classes[i], result[i], tens);
        }

        // printUint32Data(input_0, DATA_SIZE);

        send_data_via_lora(input_0, DATA_SIZE);

        MXC_Delay(1000000000); // Delay for 1 second 1000000
    }

    return 0;
}
