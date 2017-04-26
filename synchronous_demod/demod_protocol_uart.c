/*
 * Group no.: B02
 * 
 * This is the main.c code for the receiver Tiva.
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"


//#define buffer_length 8192
#define buffer_length 512

//Variables of interest for demod
int val;
uint16_t received_array[buffer_length];
int counter=0;
int mode =0; // mode 0 is searching for start sequence, mode 1 is searching for number of bytes, mode 2 is receiving data
uint16_t start_seq_rec=0b0000000000000000;
uint16_t start_seq= 0b0111100001111000;
uint16_t no_bytes=0;
int no_bytes_cntr=0;
uint16_t no_bits=0;
int i=0;
char end_char[1] ={'2'};
//Variables of interest for UART
char str[buffer_length];
int uart_i = 0;
int tp;

void UpISR(void);
void interrupt_setup(void);
void Timer0AHandler();
void Timer1AHandler();
void Timer2AHandler();

void GPIO_setup(void);

void interrupt_setup()
{
	//set up rising edge interrupt on PE1
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntRegister(GPIO_PORTE_BASE, UpISR);
	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
}

/**
 * GPIO_FALLING_EDGE interrupts are enabled on the pin which receives the recovered clock. At every
 * falling edge, the signal on the pin which receives demodulated data is sampled. This directly gives
 * the received bit.
 * The handler UpISR has a sequence of three modes of operation:
 * Mode 0: Searching for start sequence. This signifies that mode 1 can be entered.
 * Mode 1: Receiving and computing number of bytes of data. After 16 bits are received, mode 2 entered.
 * Mode 2: Receiving data. After no_bits number of bits are received:
 *         i. Bits are concatenated to form a string
 *	  ii. GPIO interrupts are disabled on the pin
 *       iii. Mode is changed to 3
 */
void UpISR()
{
	//UARTprintf("Entered UpISR\n");

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1); // enable interrupts

	val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);

	if(mode ==2)
	{	if(counter< no_bits)
		{
			received_array[counter]= val;
			counter= (counter+1) ;
		}
		else
		{
			mode=3;
			GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);

			for(tp=0;tp<no_bits;tp++)
			{
				str[tp] = (char)(received_array[tp]+48);
			}

		}

	}
	else if(mode==1)
	{
		no_bytes_cntr=no_bytes_cntr+1;
		no_bytes = (no_bytes<<1) +val;
		mode= mode+ (int)( no_bytes_cntr/16);
		no_bits=no_bytes*8;

	}
	else if(mode==0)
	{
		start_seq_rec = (start_seq_rec<<1) +val;
		if((start_seq_rec ==start_seq))
		{
			mode=1;
		}
		i++;
	}
}

void GPIO_setup()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);  // Init PF4 as input
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);  // Init PF4 as input
		GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_0,
				GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4

}

void ConfigureUART(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
	UARTStdioConfig(0, 115200, 16000000);
	//UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet() , 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
	//IntMasterEnable();
	//IntEnable(INT_UART0);
	//UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_TX);
}

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
	while(ui32Count--)
		UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
}


void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	GPIO_setup();
	interrupt_setup();
	ConfigureUART();

}

int main(void)
{
	setup();

	while(1)
	{
		/**
		 * Polls for mode=3 at every loop. This signifies that complete data has been received.
		 * The data is now sent to PC over UART, mode is changed to zero and then GPIO interrupts
		 * are re-enabled on the pin to ready the Tiva for the next transmission.
		 */
		if(mode==3)
		{
			UARTprintf(str);
			UARTprintf(end_char);
			mode = 0;
			GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
			GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1); // enable interrupts
		}
	}
	return 0;
}
void SysTickHandler()
{}

void Timer0AHandler()
{
}

void Timer1AHandler()
{
}
void Timer2AHandler()
{
}



