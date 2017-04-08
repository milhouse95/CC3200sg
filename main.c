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

	MAP_PRCMPeripheralClkEnable(PRCM_UARTA1, PRCM_RUN_MODE_CLK);
	MAP_PinTypeUART(PIN_07,PIN_MODE_5); //Tx
	MAP_PinTypeUART(PIN_08,PIN_MODE_5); //Rx
	//operating at 9600 Hz Baud Rate
	MAP_UARTConfigSetExpClk(UARTA1_BASE,MAP_PRCMPeripheralClockGet(PRCM_UARTA1),9600,UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
	MAP_UARTEnable(UARTA1_BASE);

}



//printing a string to the console
void UART_PRINT(char* string){


	while(*string != '\0'){
		MAP_UARTCharPut(UARTA1_BASE,*string);

			 //updates to next point in memory
		string++;
	}

}


void UART_CHAR(uint8_t tx){
	//check if UART transmitter is busy for fast transfer rates
	while(MAP_UARTBusy(UARTA1_BASE));
	MAP_UARTCharPut(UARTA1_BASE,tx);
}

unsigned char UART_READ_Xbee(){

	return MAP_UARTCharGet(UARTA1_BASE);

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

void DIG_POT(void){

	//enable clock for pheriperal
	 MAP_PRCMPeripheralClkEnable(PRCM_I2CA0, PRCM_RUN_MODE_CLK);
	 MAP_PRCMPeripheralReset(PRCM_I2CA0);
	//initializes the i2c Master Block, false keeps it at 400kbps
	MAP_I2CMasterInitExpClk(I2CA0_BASE,SYS_CLK,true);
	//MAP_PinTypeI2C(PIN_05, PIN_MODE_5); // scl
	//MAP_PinTypeI2C(PIN_06, PIN_MODE_5); // sda

	MAP_PinTypeI2C(PIN_01, PIN_MODE_1); // scl
	MAP_PinTypeI2C(PIN_02, PIN_MODE_1); //sda
	MAP_I2CMasterEnable(I2CA0_BASE);

}

void DIG_WRITE(uint8_t value){
	MAP_I2CMasterControl(I2CA0_BASE, I2C_MASTER_CMD_BURST_SEND_START); //start cond
	MAP_I2CMasterSlaveAddrSet(I2CA0_BASE,0b0101110,false); //write slave address
	DELAY(2000);
	MAP_I2CMasterDataPut(I2CA0_BASE,value);
	MAP_I2CMasterControl(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_CONT);
	//while(MAP_I2CMasterBusBusy(I2CA0_BASE)==true);
	DELAY(2000);
	//MAP_I2CMasterDataPut(I2CA0_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);

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


void SOUND_CONTROL(uint8_t zero_to_max){
	//send only data from 0-255
	DIG_POT(); //initalize the digital pot bus incase of LCD use
	DELAY(10000);
	DIG_WRITE(zero_to_max);


}
void main()


{

		BoardInit(); //initializes the CC3200 Interrupt Vector Table and Pheriphal Clocks
		//I2C_INIT(); //initialize the conditions for the i2c protocol
		//LCD_INIT(); //initalize the LCD to write data into it
		//LCD_STRING("Mist Makers 2.0");

		//see if UART works with LCD
		UART_INIT();
		SOUND_CONTROL(0);

		SOUND_CONTROL(0xff);


		SOUND_CONTROL(0x3d);

		//unsigned char xbeeRx=0;


		//DIG_POT();
		//DELAY(10000); //wait for i2c to set up
		//DIG_WRITE(31);
		//DELAY(10000);
		//DELAY(1000);



		//DIG_POT();
		//DELAY(10000); //wait for i2c to set up
		///DIG_WRITE(0xff);
		//DELAY(10000);
		//DELAY(1000);
		//DIG_WRITE(128);

		//DELAY(1000);
		//DIG_POT();
		//LCD_INIT();

		//data buffer from MSP430
		volatile uint8_t dataXbee[100];
		volatile int currentData=0;
		uint8_t start = 0x30; //dummy value to start sync
		int i =0;

		//LCD_STRING("Start Sync");

/*

		char dataBase[] ="";
		//int i =0;
		while(true){

			//send a start request to MSP430 to start sync
			while(true){
				//send data until we get the end sync character back from the MSP
				UART_CHAR(0x30);

				if(UART_READ_Xbee() == 0x30){
					break;
				}
			}
			//start reading the values from the MSP now after sycn
			while(currentData <100){
			dataXbee[currentData++]=UART_READ_Xbee();

			}

			int a =10;


		}

*/




		}







