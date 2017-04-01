

//Standard includes
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stdint.h"
#include <stdbool.h>




//Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "utils.h"
#include "uart.h"
#include "pin.h"
#include "gpio.h"
#include "pinmux.h"
#include "i2c.h"
#include "cpu.h"
#include "hw_gpio.h"
#include "hw_i2c.h"



//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.1.1"
#define APP_NAME                "I2C Demo"
#define UART_PRINT              Report
#define FOREVER                 1
#define CONSOLE                 UARTA0_BASE
#define FAILURE                 -1
#define SUCCESS                 0
#define RETERR_IF_TRUE(condition) {if(condition) return FAILURE;}
#define RET_IF_ERR(Func)          {int iRetVal = (Func); \
                                   if (SUCCESS != iRetVal) \
                                     return  iRetVal;}

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



	///THIS IS VERY IMPORTANT TO INCLUDE!
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}





//*****************************************************************************
//                    	DEFINES and Global Variables
//*****************************************************************************

#define I2C_BASE                I2CA0_BASE
#define SYS_CLK                 80000000


//*****************************************************************************
//                      FUNCTIONS DEFINITIONS
//*****************************************************************************


void I2C_INIT(void);
void DELAY( int n);
void GPIO_INIT(void);
void GPIO_WRITE_TO_PIN(unsigned port_pins, unsigned mode);
void LCD_STRING(char* character);

//small delay function
void DELAY(int n){
	int i=0;
	for( i=0; i<n; i++);
}




void UART_INIT(void){
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);
	MAP_PinTypeUART(PIN_55,PIN_MODE_3); //Tx
	MAP_PinTypeUART(PIN_57,PIN_MODE_3); //Rx
	//operating at 9600 Hz Baud Rate
	MAP_UARTConfigSetExpClk(UARTA0_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA0),9600,UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
	MAP_UARTEnable(UARTA0_BASE);
}

//printing a string to the console
void UART_PRINT(char* string){


	while(*string != '\0'){
		MAP_UARTCharPut(UARTA0_BASE,*string++);
			 //updates to next point in memory
	}

}
//i2c protocol
void I2C_INIT(void){

	//enable clock for pheriperal
	 MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	 MAP_PRCMPeripheralReset(PRCM_I2CA0);




	//initializes the i2c Master Block, false keeps it default timing
	MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,true);

	//pkg pin 1 on CC3200 I2C_SCL this can be found in the pin.h header file
	MAP_PinTypeI2C(PIN_01, PIN_MODE_1);

	//pkg pin 2 on CC3200 I2C_SDA
	MAP_PinTypeI2C(PIN_02, PIN_MODE_1);

	//MAP_I2CMasterDisable(I2C_BASE);
	MAP_I2CMasterEnable(I2CA0_BASE);



}



//mapped out of I/O ports
//GPIOA0 0-7				pkg pins correlate to this
//GPIOA1 8-15
//GPIOA2 16-23
//GPI0A3 24-31

//going to use this to debug and turn on/off the code
void GPIO_INIT(void){

	//enable the clock for the output
	 MAP_PRCMPeripheralClkEnable(PRCM_GPIOA1, PRCM_RUN_MODE_CLK);
	//we are going to use pkg pin 8 GPIO this is found in A2 section for GPIO


	MAP_PinTypeGPIO(PIN_04, PIN_MODE_0, true );
	//setting the directon for the pin 08 as output bit on the GPIO port
	MAP_GPIODirModeSet(GPIOA1_BASE,0x20,GPIO_DIR_MODE_OUT);
}

//writing to the bits or bit on the specific port to output low or high
//have to change the GPIO Base Address
void GPIO_WRITE_TO_PIN(unsigned port_pins, unsigned mode ){

	MAP_GPIOPinWrite(GPIOA2_BASE, port_pins , mode);

	//example code for turning off and on port output
	//MAP_GPIOPinWrite(GPIOA1_BASE, 0x20, 0x20);

}


void LCD_INIT(void){

	//starts the communication between the LCD i2c module with the CC3200
	MAP_I2CMasterSlaveAddrSet(I2CA0_BASE,0x3f,false);

	//writing command instructions to the LCD, RW=0, EN= 1 and 0 for each byte put
	//data is being sent nibble by nibble

	////////////////////////  LCD INIT \\\\\\\\\\\\\\\\\\\\\\\\\\\\

	//0x33
	MAP_I2CMasterDataPut(I2CA0_BASE,0x34);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_START);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x30);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	//lower nibble
	MAP_I2CMasterDataPut(I2CA0_BASE,0x34);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	///while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x30);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);



	//0x32
	MAP_I2CMasterDataPut(I2CA0_BASE,0x34);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x30);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	//lower nibble
	MAP_I2CMasterDataPut(I2CA0_BASE,0x24);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x20);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);


	// 0x28
	MAP_I2CMasterDataPut(I2CA0_BASE,0x24);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x20);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	//lower nibble
	MAP_I2CMasterDataPut(I2CA0_BASE,0x84);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x80);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);

	//0x0F
	MAP_I2CMasterDataPut(I2CA0_BASE,0x04);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x00);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	//lower nibble
	MAP_I2CMasterDataPut(I2CA0_BASE,0xf4);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0xf0);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);

	//
	//0x01
	MAP_I2CMasterDataPut(I2CA0_BASE,0x04);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x00);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	//lower nibble
	MAP_I2CMasterDataPut(I2CA0_BASE,0x14);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusy(I2CA0_BASE));
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,0x10);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);
	while(MAP_I2CMasterBusy(I2CA0_BASE));



}

void LCD_WRITE(unsigned char data){

		//this is the first byte to be sent
		uint8_t nibble1 = (data & 0xF0) | (0x0D); //take the upper nibble only
		uint8_t nibble2 = (data & 0xF0) | (0x09);
		uint8_t nibble3 = (data << 4 & 0xF0) | (0x0D); //lower nibble of data to be sent
		uint8_t nibble4 = (data <<4 & 0xF0) | (0x09);

		//upper nibble
		MAP_I2CMasterDataPut(I2CA0_BASE,nibble1);
		MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_START);
		DELAY(10000);
		MAP_I2CMasterDataPut(I2CA0_BASE,nibble2);
		MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
		DELAY(10000);

		//lower nibble
		MAP_I2CMasterDataPut(I2CA0_BASE,nibble3);
		MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
		DELAY(10000);
		MAP_I2CMasterDataPut(I2CA0_BASE,nibble4);
		MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);
		DELAY(1000);
}



void LCD_STRING(char* character){

	while(*character != '\0' ){
		LCD_WRITE(*character);
		character++; //goes to next character in memory of the array
	}
}
void main()
{

		BoardInit(); //initializes the CC3200 Interrupt Vector Table and Pheriphal Clocks
		I2C_INIT(); //initialize the conditions for the i2c protocol
		LCD_INIT(); //initalize the LCD to write data into it
		LCD_STRING("Mist Makers 2.0");

		//see if UART works with LCD
		UART_INIT();



		UART_PRINT("Hello");

		while(1);




}





