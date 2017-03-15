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
#define max_clock_count 10000
#define timer_delay 32

#define buf_size 100
#define max_pilot_count 40

int count=0, mod_counter=0;
int output;
int val;
int inbit=0;
int array[5]={1,1,1,1,1};
int time_period;

int in_array[buf_size];
int i = 0;

int sending_pilot = 4;
int pilot_count = 0;

void Timer1AHandler(void);
void Timer0AHandler(void);
void Timer2AHandler(void);
void UpISR(void);
void DownISR(void);

void timer_setup()
{
	time_period = SysCtlClockGet()/(4*max_clock_count);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable Timer 0 Clock
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler);
	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
		TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
		TimerIntRegister(TIMER1_BASE, TIMER_A, Timer1AHandler);
		time_period = SysCtlClockGet()/(4*max_clock_count);
		TimerLoadSet(TIMER1_BASE, TIMER_A, time_period);
		//TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

		SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
		TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
		TimerIntRegister(TIMER2_BASE, TIMER_A, Timer2AHandler);
		TimerLoadSet(TIMER2_BASE, TIMER_A, time_period);
		//TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void Timer0AHandler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE,TIMER_A );

	if(sending_pilot != 0)
	{
		if(pilot_count == max_pilot_count)
		{
			sending_pilot--;
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			//UARTprintf("Finished sending pilot\n");
		}
		else
		{
			//UARTprintf("Sending pilot\n");
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			pilot_count++;
		}
	}
	else
	{
		if(array[inbit]==mod_counter)
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			in_array[i] = array[inbit]; i=(i+1)%buf_size;
		}
		else
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
		}
		if(mod_counter==3)
		{	//UARTprintf("Sending %d\n",array[inbit]);
			mod_counter =0;
			inbit=(inbit+1)%5;
		}
		else
		{
			mod_counter =(mod_counter +1);
		}
	}
	MAP_TimerIntClear(TIMER0_BASE, TIMER_A);

	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);

	timer_setup();
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
void Timer1AHandler()
{}
void Timer2AHandler()
{}
