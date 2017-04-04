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
#define max_clock_count 320000 // output is 101kHz at 240000
#define timer_delay 32
#define array_length 64
#define buf_size 100
int test_var=0;

int count=0, mod_counter=0;
int output;
int val;
int inbit=0;
int start_bit=0;
int start_byte[8]={1,1,0,1,1,0,1,1};
int16_t no_bytes= (array_length/8);
int no_bytes_array[16];
int no_bytes_array_cntr=0;
int array[array_length]={1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,1,0,1,1,0,1,1,1,0,1,1,1,1,0,0,0,1,1};
//{1,1,0,1,0,0,0,1,0,1,0,1,1,0,0};

//{1,1,1,2,1,1,1,1,1,1};
int time_period;

int in_array[buf_size];
int i = 0;

#define max_pilot_count 400
int sending_pilot = 3;
int pilot_count = 0;

void Timer1AHandler(void);
void Timer0AHandler(void);
void Timer2AHandler(void);
void UpISR(void);
void DownISR(void);

void timer_setup()
{
	time_period = SysCtlClockGet()/(2*max_clock_count);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable Timer 0 Clock
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler);
	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);


	IntMasterEnable();
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void Timer0AHandler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE,TIMER_A );

	if(sending_pilot == 0)
	{
		if(inbit<array_length)
		{
			if(array[inbit]==mod_counter)
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
				in_array[i] = array[inbit]; i=(i+1)%buf_size;
				inbit=(inbit+ mod_counter);

			}
			else
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
				inbit=(inbit+ mod_counter);
			}
			mod_counter =(mod_counter +1)%2;
		}

	}
	else if(sending_pilot==3)
	{

		if(pilot_count == max_pilot_count)
		{
			sending_pilot=2;
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);

		}
		else
		{
			if(pilot_count%2==1)
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);

			}
			else
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			}

			pilot_count=pilot_count+1;

		}

	}
	else if(sending_pilot==2)
	{

		if(start_byte[start_bit]==mod_counter)
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			in_array[i] = start_byte[start_bit]; i=(i+1)%buf_size;
			start_bit=(start_bit+ mod_counter);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			start_bit=(start_bit+ mod_counter);
		}
		mod_counter =(mod_counter +1)%2;
		sending_pilot= sending_pilot -(start_bit/8);
	}
	else
	{
		if(no_bytes_array[no_bytes_array_cntr]==mod_counter)
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			in_array[i] = no_bytes_array[no_bytes_array_cntr]; i=(i+1)%buf_size;
			no_bytes_array_cntr=(no_bytes_array_cntr+ mod_counter);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			no_bytes_array_cntr=(no_bytes_array_cntr+ mod_counter);
		}
		mod_counter =(mod_counter +1)%2;
		sending_pilot= sending_pilot -(no_bytes_array_cntr/16);

	}


	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void setup(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5| SYSCTL_USE_PLL | SYSCTL_OSC_INT | SYSCTL_XTAL_16MHZ);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);


}

int main(void)
{

	setup();
	test_var=no_bytes;
	int j=15;
	for( j=15;j>=0;--j)
	{
		no_bytes_array[j]=no_bytes%2;
		no_bytes= no_bytes>>1;

	}

	timer_setup();
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
