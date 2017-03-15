/*
 * main.c
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

#define max_clock_count 150000  //max attained
#define timer_delay 2			//min attained

//#define max_clock_count 30000
//#define timer_delay 11
#define buf_size 100

int count=0,mod_counter=0;
int output;
int val;
int inbit=0;
int array[5]={3,1,2,3,0};
int time_period;

int out_array[buf_size];
int i = 0;

//prototypes
void Timer1AHandler(void);
void Timer0AHandler(void);


void timer_setup(void)
{
	time_period = SysCtlClockGet()/(4*max_clock_count);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1); // Enable Timer 1 Clock
	TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER1_BASE, TIMER_A, Timer1AHandler);
	TimerLoadSet(TIMER1_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); // Enable Timer 1 Clock
	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); // Configure Timer Operation as Periodic
	TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AHandler);
	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	IntMasterEnable();

	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
	SysCtlDelay(timer_delay);
	TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
}

void Timer1AHandler()
{
	TimerIntClear(TIMER1_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER1_BASE,TIMER_A );
	//UARTprintf("In Handler A\n");
	val = GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_1);
	//UARTprintf("val=%d\n",val);
	if(val==2)
	{
		output =count;
		out_array[i] = output;
		i = (i+1)%buf_size;
		//UARTprintf("                           OUTPUT=%d\n",output);
	}

	count =(count+1)%4;



	MAP_TimerIntClear(TIMER1_BASE, TIMER_A);
	//int time_period =SysCtlClockGet()/(max_clock_count*4);
	TimerLoadSet(TIMER1_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	TimerEnable(TIMER1_BASE, TIMER_A); // Start Timer 1A
}

void Timer0AHandler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE,TIMER_A );
	//UARTprintf("In Handler 0A\n");
	if(array[inbit]==mod_counter)
	{
		GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
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
	else{
		mod_counter =(mod_counter +1);
	}

	MAP_TimerIntClear(TIMER0_BASE, TIMER_A);
	TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}
void GPIO_setup()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_0);
}
void ConfigureUART(void)
{
	//
	// Enable the GPIO Peripheral used by the UART.
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	//
	// Enable UART0
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	//
	// Configure GPIO Pins for UART mode.
	//
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	//
	// Use the internal 16MHz oscillator as the UART clock source.
	//
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

	//
	// Initialize the UART for console I/O.
	//
	UARTStdioConfig(0, 115200, 16000000);
	//
	//    UARTStdioConfig(0, 115200, 16000000);
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
void Timer2AHandler()
{}
