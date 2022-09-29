/*
 * 	startup.c
 *
 */
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "misc.h"
#include "usart.h"

__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void) {
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n"); /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

void init_gpio(void) {
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef gpiod_init;

    gpiod_init.GPIO_Mode = GPIO_Mode_OUT;
    gpiod_init.GPIO_Pin = GPIO_Pin_0;
    gpiod_init.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOD, &gpiod_init);

    gpiod_init.GPIO_Pin = GPIO_Pin_1;
    gpiod_init.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOD, &gpiod_init);
}

volatile long time_us;
volatile int send_time, receive_time;

void sensor_receive(void) {
    receive_time = time_us - 42;
    int range = (((receive_time - send_time) * 340) / 2) / 1000; //convert to cm
    char snum[10];
    itoa(range, snum, 10);
    print(snum);
    print(":cm \n");

    EXTI_ClearITPendingBit(EXTI_Line1); //clear pending interrupt flag flag
}

void systick_handler(void) {
    time_us++;
    if (time_us % 7000 == 0) { //Initiate cycle every 70 ms
        send_time = time_us;
        receive_time = send_time;
        GPIO_WriteBit(GPIOD, GPIO_Pin_0, 1); //send trigger
    }
    if (time_us == send_time + 1) { //
        GPIO_WriteBit(GPIOD, GPIO_Pin_0, 0); //stop trigger after 10 us
    }
}

void init_exti(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    EXTI_InitTypeDef exti_init;
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource1);

    exti_init.EXTI_Line = EXTI_Line1; //connect exti line 1 to PD1
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    NVIC_InitTypeDef nvic_init;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    nvic_init.NVIC_IRQChannel = EXTI1_IRQn;
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&nvic_init);

    *((int *) 0x2001C05C) = sensor_receive;
}

void init_systick(void) {
    SysTick_Config(1680 - 1); //set systick to interrupt every us
    NVIC_SetPriority(SysTick_IRQn, 0);
    *((int *) 0x2001C03C) = systick_handler;
}

void main(void) {
    init_gpio();
    init_exti();
    init_systick();
    print("hello");
    while (1);
    print("done");
}