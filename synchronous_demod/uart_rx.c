/*
 * uart_rx.c
 *
 *  Created on: 12-Apr-2017
 *      Author: eskay
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

#define array_length 96

//"Sample text!"
int array[array_length] = {0,1,0,1,0,0,1,1,0,1,1,0,0,0,0,1,0,1,1,0,1,1,0,1,0,1,1,1,0,0,0,0,0,1,1,0,1,1,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,0,1,1,1,0,1,0,0,0,1,1,0,0,1,0,1,0,1,1,1,1,0,0,0,0,1,1,1,0,1,0,0,0,0,1,0,0,0,0,1};
//{1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,1,1,1,1,0};
char str[array_length];
int uart_i = 0;
int tp;

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
	IntMasterEnable();
	IntEnable(INT_UART0);
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_TX);
}

void UARTSend(const uint8_t *pui8Buffer, uint32_t ui32Count)
{
	while(ui32Count--)
		UARTCharPutNonBlocking(UART0_BASE, *pui8Buffer++);
}

void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);

	ConfigureUART();

	for(tp=0;tp<array_length;tp++)
	{
		str[tp] = (char)(array[tp]+48);
	}
}

int main(void)
{
	setup();
	//UARTprintf("\nEntered main\n");
	UARTprintf(str);

	while(1)
	{
		//UARTCharPut(UART0_BASE, 'A');
	}
}

void Timer1AHandler()
{}
void Timer2AHandler()
{}
void Timer0AHandler()
{}
void UARTIntHandler()
{}





