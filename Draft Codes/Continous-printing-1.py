#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
#include "uart.h"
#include "dma.h"
#include "nvic_table.h"

#define DMA
#define UART_BAUD 115200
#define BUFF_SIZE 1024

volatile int READ_FLAG;

#if defined(BOARD_EVKIT_V1)
#define READING_UART MXC_UART1
#define WRITING_UART MXC_UART2
#elif defined(BOARD_FTHR_REVA)
#define READING_UART MXC_UART2
#define WRITING_UART MXC_UART3
#else
#warning "This example has been written for the MAX78000 Ev Kit or FTHR board."
#endif

#ifndef DMA
void Reading_UART_Handler(void)
{
    MXC_UART_AsyncHandler(READING_UART);
}
#endif

void readCallback(mxc_uart_req_t *req, int error)
{
    READ_FLAG = error;
}

/******************************************************************************/ 
int main(void)
{
    int error, fail = 0;
    uint8_t TxData[] = "5";  
    uint8_t RxData[BUFF_SIZE];  

    memset(RxData, 0x0, BUFF_SIZE);

#ifndef DMA
    NVIC_ClearPendingIRQ(MXC_UART_GET_IRQ(READING_UART_IDX));
    NVIC_DisableIRQ(MXC_UART_GET_IRQ(READING_UART_IDX));
    MXC_NVIC_SetVector(MXC_UART_GET_IRQ(READING_UART_IDX), Reading_UART_Handler);
    NVIC_EnableIRQ(MXC_UART_GET_IRQ(READING_UART_IDX));
#endif

    if ((error = MXC_UART_Init(READING_UART, UART_BAUD, MXC_UART_APB_CLK)) != E_NO_ERROR) {
        printf("-->Error initializing UART: %d\n", error);
        printf("-->Example Failed\n");
        return error;
    }

    if ((error = MXC_UART_Init(WRITING_UART, UART_BAUD, MXC_UART_APB_CLK)) != E_NO_ERROR) {
        printf("-->Error initializing UART: %d\n", error);
        printf("-->Example Failed\n");
        return error;
    }

    printf("-->UART Initialized\n\n");

#ifdef DMA
    MXC_UART_SetAutoDMAHandlers(READING_UART, true);
    MXC_UART_SetAutoDMAHandlers(WRITING_UART, true);
#endif

    while (1) {  // Continuous transmission loop
        mxc_uart_req_t write_req;
        write_req.uart = WRITING_UART;
        write_req.txData = TxData;
        write_req.txLen = strlen((const char*)TxData);  // Cast to const char* to fix warning
        // Alternatively, use write_req.txLen = 1;
        write_req.rxLen = 0;
        write_req.callback = NULL;

        error = MXC_UART_Transaction(&write_req);

        if (error != E_NO_ERROR) {
            printf("-->Error starting sync write: %d\n", error);
            printf("-->Example Failed\n");
            return error;
        }

        // Wait for the transmission to complete
        while (READ_FLAG) {}

        // Print success after each successful transmission
        printf("Transmission Success!\n");

        // Optional delay to avoid flooding the receiver with data too quickly
        MXC_Delay(MXC_DELAY_MSEC(100000));  // Add a 100ms delay between transmissions
    }

    // If the program somehow exits the loop (it won't in this case)
    printf("-->Transaction complete\n\n");

    if (READ_FLAG != E_NO_ERROR) {
        printf("-->Error from UART read callback; %d\n", READ_FLAG);
        fail++;
    }

    if (fail != 0) {
        printf("\n-->Example Failed\n");
        return E_FAIL;
    }

    LED_On(LED1); // Indicates SUCCESS
    printf("\n-->Example Succeeded\n");
    return E_NO_ERROR;
}
