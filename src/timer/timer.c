#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "timer.h"

void timer_default_irq( void ){
    timer_ms++;
    TIM_ClearITPendingBit(TIM2, TIM2_IRQn);
}

void timer_init( void ){
    //default timer irq
    *((int *) 0x2001C0B0) = timer_default_irq;

    //Prescaler 42000 - 1 = 0.5 ms
    //Counter mode up
    //ms between every interrupt
    TIM_TimeBaseInitTypeDef TIM_initStructure = {
        42000 - 1,
        TIM_CounterMode_Up,
        0xffffffff,
        TIM_CKD_DIV1
    };
    TIM_TimeBaseInit(TIM2, &TIM_initStructure);
    
    //Init nvic with highest priorities
    NVIC_InitTypeDef NVIC_initStructure = {
        TIM2_IRQn,
        0,
        0,
        ENABLE
    };
    NVIC_Init(&NVIC_initStructure);
    
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    timer_ms = 0;
    TIM_Cmd(TIM2, ENABLE);
}