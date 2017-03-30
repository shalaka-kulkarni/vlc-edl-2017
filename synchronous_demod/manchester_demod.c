/*
 * demod.c
 *
 *  Created on: 07-Mar-2017
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


#define buffer_length 100
int val;
int received_array[buffer_length];
int counter=0;




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

void UpISR()
{
	//UARTprintf("Entered UpISR\n");

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1); // enable interrupts

	val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_0);
	received_array[counter]= val;
	counter= (counter+1) % buffer_length;
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


void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	GPIO_setup();

	interrupt_setup();
}

int main(void)
{
	setup();

	while(1)
	{

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



