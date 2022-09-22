/*
 * 	startup.c
 *
 */
 
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "usart.h"


__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void init_met(void){
	 
	 GPIO_InitTypeDef trig;
	 GPIO_InitTypeDef echo;
	
	 GPIO_StructInit(&trig);
	 GPIO_StructInit(&echo);
	 
	 trig.GPIO_Pin = GPIO_Pin_0;
	 trig.GPIO_PuPd = GPIO_PuPd_DOWN;
	 trig.GPIO_OType = GPIO_OType_PP;
	 trig.GPIO_Mode = GPIO_Mode_OUT;
	 
	 
	 echo.GPIO_Pin = GPIO_Pin_1;
	 echo.GPIO_PuPd = GPIO_PuPd_DOWN;
	 echo.GPIO_OType = GPIO_OType_PP;
	 echo.GPIO_Mode = GPIO_Mode_IN;
	
	GPIO_Init(GPIOD,&echo);
	GPIO_Init(GPIOD,&trig);
} 

 void TIM_Configration(void){
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseInitTypeDef TIM_timer;
	TIM_timer.TIM_Prescaler = 0x0;
	TIM_timer.TIM_Period = 0xFFFFFFFF;
	TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_timer);
	TIM_Cmd(TIM2, ENABLE);
 }
 
 


void main(void)
{
	
	unsigned int echo_start, echo_end, echo_time, distance;
	char a[30];
	init_met();
	TIM_Configration();
	GPIO_ResetBits(GPIOD, GPIO_Pin_0);
	print("\nwelcome");
	
	while(1){
		
	TIM_SetCounter(TIM2,0);				// delay between measurments 
	while(TIM_GetCounter(TIM2) < 168 * 60000);
	
	GPIO_SetBits(GPIOD, GPIO_Pin_0); // Sets trigger pin	

	TIM_SetCounter(TIM2,0);				// 10 mikroseconds delay
	while(TIM_GetCounter(TIM2) < 168 * 10);
	
	GPIO_ResetBits(GPIOD, GPIO_Pin_0);	// Clear trigger pin
	
	// wait for echo 
	
	while(!(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1))) {
		echo_start = TIM_GetCounter(TIM2);
	}
	
	while(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1)) {
		echo_end = TIM_GetCounter(TIM2);
		
	}
	
	// Correction for clock overflow during measurment of the echo.
	if(echo_start >= echo_end){	
		echo_time = echo_end + (0xFFFFFFFF - echo_start);
	} else{
		echo_time = echo_end - echo_start;
	}
	distance = 343 * echo_time / ((168000000/100));
	if(distance<400){
	
	print(itoa(distance,a,10));
	}else{print("distance is too far");}
	
	
	print("\n");
	 }
	 }
	 



