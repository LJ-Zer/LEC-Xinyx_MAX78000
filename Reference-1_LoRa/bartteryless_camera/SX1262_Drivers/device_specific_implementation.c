/*
__/\\\\____________/\\\\_____/\\\\\\\\\\\\_        
 _\/\\\\\\________/\\\\\\___/\\\//////////__       
  _\/\\\//\\\____/\\\//\\\__/\\\_____________      
   _\/\\\\///\\\/\\\/_\/\\\_\/\\\____/\\\\\\\_     
    _\/\\\__\///\\\/___\/\\\_\/\\\___\/////\\\_    
     _\/\\\____\///_____\/\\\_\/\\\_______\/\\\_   
      _\/\\\_____________\/\\\_\/\\\_______\/\\\_  
       _\/\\\_____________\/\\\_\//\\\\\\\\\\\\/__ 
        _\///______________\///___\////////////____

Author: Marco Giordano
*/

// Pin assignemnts
// Sobstitute here the your specific pin assignment

#include "device_specific_implementation.h"
#include "sx126x_commands.h"


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "mxc_pins.h"
#include "nvic_table.h"
#include "uart.h"
#include "spi.h"
#include "dma.h"
#include "board.h"

#define SPI_SPEED       100000      // Bit Rate
#define SPI         MXC_SPI0

mxc_spi_req_t req;
mxc_spi_pins_t spi_pins;

uint16_t rx_data[10];

// Device-specific implementations
// Sobstitute here the functions related to your specific microcontroller

uint8_t read_pin(const uint8_t pin){
    return gpio_get_pin_level(pin);
}

void write_pin(const uint8_t pin, const uint8_t status){
    gpio_set_pin_level(pin, status);
}

int32_t SPI_init(void)
{
    spi_pins.clock = TRUE;
    spi_pins.miso = TRUE;
    spi_pins.mosi = TRUE;
    spi_pins.sdio2 = FALSE;
    spi_pins.sdio3 = FALSE;
    spi_pins.ss0 = TRUE;
    spi_pins.ss1 = TRUE;
    spi_pins.ss2 = FALSE;

        if (MXC_SPI_Init(SPI, 1, 0, 2, 0, SPI_SPEED, spi_pins) != E_NO_ERROR) {
            printf("\nSPI INITIALIZATION ERROR\n");

            return 1;
        }

    return 0;
}

int32_t SendSpi(uint8_t *data, uint8_t len){
    // Return number of bytes written
    //SPI Request
        req.spi = SPI;
        req.txData = (uint8_t*) data;
        //req.rxData = (uint8_t*) rx_data;
        req.txLen = len;
        //req.rxLen = len;
        req.ssIdx = 0;
        req.ssDeassert = 1;
        req.txCnt = 0;
        req.rxCnt = 0;
        //req.completeCB = (spi_complete_cb_t) SPI_Callback;
        
        uint8_t retVal = MXC_SPI_SetDataSize(SPI, 8);
        
        if (retVal != E_NO_ERROR) {
            printf("\nSPI SET DATASIZE ERROR: %d\n", retVal);
            
            return 1;
        }
        
        retVal = MXC_SPI_SetWidth(SPI, SPI_WIDTH_STANDARD);
        
        if (retVal != E_NO_ERROR) {
            printf("\nSPI SET WIDTH ERROR: %d\n", retVal);
            
            return 1;
        }
        

            MXC_SPI_MasterTransaction(&req);

    return 0;
}

int32_t ReadSpi(uint8_t *rx_data, uint8_t len){	

    // We are only sending in this example
    return 0;
}

void IRQ_Init(void)
{
	//ext_irq_register(PIN_PC00, DIO1_IRQ);
    // Possibility to add DIO2 and DIO3 interrupts
}

void DIO1_IRQ(void)
{
    /* NOT USING INTERRUPT THIS TIME

	gpio_toggle_pin_level(LED);
    // Clear the interrupt in the radio
    //SX126x_ClearIrqStatus( IRQ_RADIO_ALL );
    // Do something in the ISR
	uint8_t tx_done_see[10] = "Received!\n";
	io_write(usart, tx_done_see, 10);
	
	uint8_t buffer_g[4];

	SX126x_GetPayload(buffer_g, 4, 4);
	
	io_write(usart, buffer_g, 4);
	SX126x_ClearIrqStatus(2);
	//delay_ms(10);
	//SX126x_SendPayload((uint8_t *) "PONG", 4, 0); // Be careful timeout
	//delay_ms(5);
	SX126x_SetRx(0);
*/
}
