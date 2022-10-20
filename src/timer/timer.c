#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "timer.h"

void timer_default_irq( void ){
    timer_ms++;
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void timer_init( void ){
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	*((int *) 0x2001C0B0) = timer_default_irq;
	TIM_TimeBaseInitTypeDef TIM_timer;
	TIM_timer.TIM_Prescaler = (42000 - 1);
	TIM_timer.TIM_Period = (2-1);
	TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_timer);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef nvic_init;
	nvic_init.NVIC_IRQChannel = TIM2_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);
	timer_ms = 0;
	
	*((void (**) (void)) 0x2001C0B0) = timer_default_irq;
}