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

//#define max_clock_count 150000  //max attained
//#define timer_delay 2			//min attained
#define max_clock_count 50000
#define timer_delay 6
#define buf_size 100
#define max_pilot_count 40
#define array_length 50

int count=0, mod_counter=0;
int output;
int val;
int array[array_length]={0,1,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,0};
//{1,1,1,2,1,1,1,1,1,1};
int inbit=0;

int time_period;
//int decoder[4] = {2,3,0,1};
//int decoder[4] = {3,0,1,2};
int decoder[4] = {0,1,2,3};
int in_array[buf_size];
int out_array[buf_size];
int i = 0; int j=0;
int sending_pilot = 4;
int pilot_count = 0;

int pilot_width = 0;
int pilot_width_at_falling = 0;

int up_count=0; int down_count=0; //ISR entry flags

void Timer1AHandler(void);
void Timer0AHandler(void);
void Timer2AHandler(void);
void UpISR(void);
void DownISR(void);

void interrupt_setup(void);
void timer_setup(void);
void GPIO_setup(void);

void interrupt_setup()
{
	//set up rising edge interrupt on PE1
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntRegister(GPIO_PORTE_BASE, UpISR);
	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_RISING_EDGE);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
}

void UpISR()
{
	//UARTprintf("Entered UpISR\n");
	up_count++;
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntRegister(GPIO_PORTE_BASE, DownISR); // register UpISR() with the PORT E interrupt handler
	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_FALLING_EDGE); // set the interrupt type as falling edge
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1); // clear the interrupt
	GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1); // enable interrupts

	if(pilot_width == 0)
		TimerEnable(TIMER2_BASE, TIMER_A);
}

void DownISR(void)
{
	//UARTprintf("Entered DownISR\n");
	down_count++;

	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);

	pilot_width_at_falling = pilot_width;

	//	TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	//	TimerLoadSet(TIMER2_BASE, TIMER_A, time_period);
	//	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	if(pilot_width_at_falling >= max_pilot_count-2) // changes made here
	{
		//UARTprintf("Started Timer1A\n");
		TimerDisable(TIMER2_BASE, TIMER_A);
		SysCtlDelay(1*timer_delay); //The sender sends HHHH....HHHL followed by msg. So skip 3/8th of time_period.
		TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
	}
	else
	{
		//register a rising edge interrupt for the next pilot signal, but don't enable
		GPIOIntRegister(GPIO_PORTE_BASE, UpISR);
		GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_RISING_EDGE);
		GPIOIntClear(GPIO_PORTE_BASE, GPIO_PIN_1);
		GPIOIntEnable(GPIO_PORTE_BASE, GPIO_PIN_1);
		//TimerEnable(TIMER2_BASE, TIMER_A);
	}

}

void timer_setup()
{
	time_period = SysCtlClockGet()/(2*max_clock_count);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable Timer 0 Clock
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler);
	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER1_BASE, TIMER_A, Timer1AHandler);
	TimerLoadSet(TIMER1_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	TimerIntRegister(TIMER2_BASE, TIMER_A, Timer2AHandler);
	TimerLoadSet(TIMER2_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A

}

void Timer1AHandler()
{
	TimerIntClear(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER1_BASE,TIMER_A);

	val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1);

	if(val==2)
	{
		output = decoder[count];
		out_array[i] = output;
		i++;
		//i = (i+1)%buf_size;
		//UARTprintf("                           OUTPUT=%d\n",output);
	}

	count =(count+1)%2;


	MAP_TimerIntClear(TIMER1_BASE, TIMER_A);

	TimerLoadSet(TIMER1_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
}

void Timer0AHandler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE,TIMER_A );

	if(sending_pilot != 0)
	{
		if(pilot_count == max_pilot_count)
		{
			sending_pilot = 0;
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			//UARTprintf("Finished sending pilot\n");
		}
		else
		{
			//UARTprintf("Sending pilot\n");
			if(pilot_count%2 == 1)
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			}
			else
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			}
			pilot_count++;
		}
	}
	else
	{	if(inbit<array_length)
		{
			if(array[inbit]==mod_counter)
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
				in_array[j] = array[inbit];
				j++;
			}
			else
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			}
			if(mod_counter==1)
			{	//UARTprintf("Sending %d\n",array[inbit]);
				mod_counter =0;
				inbit=(inbit+1);
				//in_array[i] = array[inbit]; i=(i+1)%buf_size;
			}
			else
			{
				mod_counter =(mod_counter +1);
			}
		}
	}
	MAP_TimerIntClear(TIMER0_BASE, TIMER_A);

	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void Timer2AHandler()
{
	TimerIntClear(TIMER2_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER2_BASE,TIMER_A );
	pilot_width++;
	TimerLoadSet(TIMER2_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER2_BASE, TIMER_A);
}
void GPIO_setup()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);  // Init PF4 as input
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);  // Enable weak pullup resistor for PF4
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);
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
	ConfigureUART();
	timer_setup();
	interrupt_setup();
}

int main(void)
{
	setup();
	UARTprintf("Start\n");
	while(1)
	{

	}
	return 0;
}
void SysTickHandler()
{}
