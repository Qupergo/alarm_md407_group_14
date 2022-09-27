/*
 * 	startup.c
 *
 */
 
//#include <stdbool.h>
 
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "usart.h"
//#include "stdio.h"
//#include "stdlib.h"
//#include "stm32f4xx_syscfg.h"

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}
/*
void GG (){
	GPIO_InitTypeDef gpiod_init;
	GPIO_StructInit(&gpiod_init);
	gpiod_init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	gpiod_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpiod_init.GPIO_Mode = GPIO_Mode_IN;
	
	GPIO_Init(GPIOD,&gpiod_init);
	
}
 */
 
 void init_vib(void){
	 //GPIO_InitTypeDef analog;
	 GPIO_InitTypeDef digital;
	 
	 //GPIO_StructInit(&analog);
	 GPIO_StructInit(&digital);
	 /*
	 analog.GPIO_Pin = GPIO_Pin_1;
	 analog.GPIO_PuPd = GPIO_PuPd_NOPULL;
	 analog.GPIO_Mode = GPIO_Mode_AN;
	  */
	 
	 
	 digital.GPIO_Pin = GPIO_Pin_0 ;
	 digital.GPIO_PuPd = GPIO_PuPd_DOWN;
	 digital.GPIO_Mode = GPIO_Mode_IN;
	 
	GPIO_Init(GPIOD,&digital);
	//GPIO_Init(GPIOD,&analog);
	//Serial.begin(9600);


	 
	 
	 
	 
	 
 }

void delay_milli( void)
{
//print("a\n");
*STK_CTRL = 0;
*STK_LOAD = ((168000000) -1 );
*STK_VAL = 0;
*STK_CTRL = 5;
while( (*STK_CTRL & 0x10000 )== 0 );
*STK_CTRL = 0;
}
 

void main(void)
{
	init_vib();
	char value ;
	char v; 
	
	print("\nwelcome\n");
	//char x = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1);
	
	while(1){
	value = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0);
	//v= '0'+value;
	//print(&v);
	//v = 48 +value;
	//test_char();
	
	if (!value)
		{print("h");
	
	}
	
	//print(x);
	//test_char();
	//print("s\n");
	//test_char();
	
	
	}
	

}

