
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


#include "uart.h"  // Make sure this header is included
#define MXC_UART0 ((mxc_uart_regs_t *)MXC_BASE_UART0)
int MXC_UART_Init(mxc_uart_regs_t *uart, unsigned int baud, mxc_uart_clock_t clock);
// Use this prototype in your implementation
int MXC_UART_Write(mxc_uart_regs_t *uart, const uint8_t *byte, int *len);
mxc_uart_clock_t uart_clock = MXC_UART_ERTCO_CLK; // Use the appropriate clock constant



#define IMAGE_SIZE_X (64 * 2)
#define IMAGE_SIZE_Y (64 * 2)
#define DATA_SIZE (128 * 128)

static uint32_t input_0[DATA_SIZE];

#define UART_BAUD_RATE 115200
// #define DATA_SIZE 256  // Adjust DATA_SIZE according to your requirements

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

void uart_init(void) {
    int result = MXC_UART_Init(MXC_UART0, UART_BAUD_RATE, uart_clock);
    if (result != 0) { // Check if result indicates an error
        // Handle the initialization error
        // For example: log the error, blink an LED, or display a message
        printf("UART initialization failed with error code: %d\n", result);
    }
}




void uart_transmit(const uint8_t *data, size_t len) {
    int result;
    int length = (int)len; // Convert size_t to int
    int status;
    // Transmit data over UART
    status = MXC_UART_Write(MXC_UART0, data, &length);
}

void uart_transmit_uint32(const uint32_t *data, size_t num_elements) {
    // Convert uint32_t array to uint8_t array
    size_t num_bytes = num_elements * sizeof(uint32_t);
    uint8_t *byte_data = (uint8_t *)data;

    uart_transmit(byte_data, num_bytes);
}



/* **************************************************************************** */

int main(void)
{
    int i;
    int digs, tens;
    int ret = 0;
    int result[CNN_NUM_OUTPUTS];
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

        int max_index = 0;
        int max_value = ml_softmax[0];

        for (i = 1; i < CNN_NUM_OUTPUTS; i++) {
            if (ml_softmax[i] > max_value) {
                max_value = ml_softmax[i];
                max_index = i;
            }
        }

        digs = (1000 * ml_softmax[max_index] + 0x4000) >> 15;
        tens = digs % 10;
        digs = digs / 10;
        result[max_index] = digs;

        // Print the highest class percentage
        char resultz[100]; // Make sure this is large enough to hold the formatted string
        sprintf(resultz, "Highest Class %d %8s: %d.%d%%\n", max_index, classes[max_index], result[max_index], tens);
        printf("%s", resultz);


        printf("Time for CNN: %d us\n\n", cnn_time);
        printf("Classification results:\n");

        for (i = 0; i < CNN_NUM_OUTPUTS; i++) {
            digs = (1000 * ml_softmax[i] + 0x4000) >> 15;
            tens = digs % 10;
            digs = digs / 10;
            result[i] = digs;
            printf("[%7d] -> Class %d %8s: %d.%d%%\n", ml_data[i], i, classes[i], result[i], tens);
        }

        printUint32Data(input_0, DATA_SIZE);
        uart_transmit_uint32(input_0, DATA_SIZE);

        MXC_Delay(1000000000); // Delay for 1 second 1000000
    }

    return 0;
}
