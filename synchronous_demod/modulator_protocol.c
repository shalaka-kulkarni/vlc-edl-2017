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

#define max_clock_count 240000 // output is 101kHz at 240000
#define timer_delay 32
#define buf_size 1024
#define max_pilot_count 200000

int test_var=0;
int sending_pilot_change=0;
int entered=0;
int count=0, mod_counter=0;
int output;
int val;
int inbit=0;
int start_bit=0;
int start_byte[16]={0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0};
uint16_t no_bytes=0;
int no_bytes_array[16];
int no_bytes_array_cntr=0;
int no_bytes_const = 0;

int array[buf_size];
int time_period;
int in_array[buf_size];

int i = 0;
int index = 0;
int uart_i=0;

int getting_bitcount = 3;
int stopbit = 0;
int bitcount = 0;

int send_start = 0;
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
	//TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
}

void Timer0AHandler()
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
	TimerDisable(TIMER0_BASE,TIMER_A );
	sending_pilot_change=0;
	if(sending_pilot == 0)  //Data
	{
		send_start = 0;
		if(inbit<bitcount)
		{
			if(array[inbit]==mod_counter)
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
				in_array[i] = array[inbit]; i=(i+1)%buf_size;
				inbit=(inbit+ mod_counter); //%bitcount;

			}
			else
			{
				GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
				inbit=(inbit+ mod_counter); //%bitcount;
			}
			mod_counter =(mod_counter +1)%2;
			//send_start = send_start + inbit/bitcount;
			TimerLoadSet(TIMER0_BASE, TIMER_A, time_period);
			TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
			TimerEnable(TIMER0_BASE, TIMER_A); // Start Timer 0A
		}
		else
		{
			sending_pilot = 2;
			start_bit=0;
			inbit = 0;
			no_bytes_array_cntr=0;
		}
	}

	if(sending_pilot==2)   //Start byte
	{

		if(start_byte[start_bit]==mod_counter)
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			in_array[i] = start_byte[start_bit]; i=(i+1);
			start_bit=(start_bit+ mod_counter);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x01);
			start_bit=(start_bit+ mod_counter);
		}
		mod_counter =(mod_counter +1)%2;
		sending_pilot= sending_pilot -(start_bit/16);
	}

	else if(sending_pilot==3)    //Pilot
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
		sending_pilot = sending_pilot - (pilot_count/max_pilot_count);

	}

	else if(sending_pilot==1)  //Number of bytes
	{
		if(no_bytes_array[no_bytes_array_cntr]==mod_counter)
		{
			GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_0, 0x00);
			in_array[i] = no_bytes_array[no_bytes_array_cntr]; i=(i+1);
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
	timer_setup();
}

void UARTIntHandler()
{
	while(UARTCharsAvail(UART0_BASE))
	{
		entered = entered+1;
		int ucData = UARTCharGet(UART0_BASE) - 48;
		if(getting_bitcount != 0) //init 3
		{
			bitcount = 10*bitcount + ucData; //init 0
			getting_bitcount--;
		}
		else
		{
			if(ucData == 2)
			{
				stopbit = 1; //init 0
				UARTDisable(UART0_BASE);

				no_bytes = bitcount/8;
				no_bytes_const = no_bytes;

				int j=15;
				for(j=15;j>=0;--j)
				{
					no_bytes_array[j]=no_bytes%2;
					no_bytes= no_bytes>>1;
				}

				TimerEnable(TIMER0_BASE, TIMER_A); // Start sending
				return;
			}
			else
			{
				array[uart_i] = ucData;
				uart_i = (uart_i+1);//%array_length;
			}
		}
	}
}

int main(void)
{
	setup();

	while(1)
	{
	}
}
void Timer1AHandler()
{}
void Timer2AHandler()
{}
