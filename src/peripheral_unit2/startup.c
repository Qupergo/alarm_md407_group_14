/*
 * 	startup.c
 *
 */

#include "peripheral_unit2.h"


__attribute__((naked)) __attribute__((section(".start_section")))
void startup(void) {
    __asm__ volatile(" LDR R0,=0x2001C000\n"); /* set stack */
    __asm__ volatile(" MOV SP,R0\n");
    __asm__ volatile(" BL main\n"); /* call main */
    __asm__ volatile(".L1: B .L1\n"); /* never return */
}

VibrationSensor vibrationSensor = {
    0,
    GPIOD,
    GPIO_Pin_4
};

UltraSonicSensor ultraSonicSensor = {
    1,
    GPIOD,
    GPIO_Pin_0,
    GPIO_Pin_1,
    0,
    50
};

void Init_GPIO(void) {
    // Configration of GPIO pins and ports 
    GPIO_InitTypeDef GPIOInit;
    GPIO_StructInit(&GPIOInit);
    GPIOInit.GPIO_Pin = ultraSonicSensor.Trig_Pin;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIOInit.GPIO_OType = GPIO_OType_PP;
    GPIOInit.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(ultraSonicSensor.GPIOUnit, &GPIOInit);

    GPIO_StructInit(&GPIOInit);
    GPIOInit.GPIO_Pin = ultraSonicSensor.Echo_Pin | vibrationSensor.Vibration_Pin;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIOInit.GPIO_OType = GPIO_OType_PP;
    GPIOInit.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOD, &GPIOInit);

}

void TIM_Configration(void) {
    // Configration of timer on TIM2 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_timer;
    TIM_timer.TIM_Prescaler = 0x0;
    TIM_timer.TIM_Period = 0xFFFFFFFF;
    TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_timer);
    TIM_Cmd(TIM2, ENABLE);
}

void vibration_irq_handler(void) {
    if (!GPIO_ReadInputDataBit(vibrationSensor.GPIOUnit, vibrationSensor.Vibration_Pin)) {
        print("vib");
    }

    EXTI_ClearFlag(EXTI_Line4);
}

void UltraSonic_irq_handler(void) {
    print("\n");
    print(itoa(ultraSonicSensor.Distance, a, 10));

    if (ultraSonicSensor.Distance < ultraSonicSensor.Threshold) {
        //print("alarm");
        //print("\n");
    }

    EXTI_ClearFlag(EXTI_Line1);
}

void EXTIInit(void) {

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    EXTI_InitTypeDef exti_init;

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource4);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource1);

    exti_init.EXTI_Line = EXTI_Line1; //Configration of EXTI for Trig pin
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    exti_init.EXTI_Line = EXTI_Line4; //Configration of EXTI for Vibration pin
    exti_init.EXTI_Mode = EXTI_Mode_Interrupt;
    exti_init.EXTI_Trigger = EXTI_Trigger_Falling;
    exti_init.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti_init);

    NVIC_InitTypeDef nvic_init;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    nvic_init.NVIC_IRQChannel = EXTI1_IRQn; //Configration of NVIC for UltraSonic_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void( ** )(void)) 0x2001C05C) = UltraSonic_irq_handler;

    nvic_init.NVIC_IRQChannel = EXTI4_IRQn; //Configration of NVIC for vibration_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void( ** )(void)) 0x2001C068) = vibration_irq_handler;
}

void check_distance(void) {
    TIM_SetCounter(TIM2, 0); // 65 microseconds delay between measurments 
    while (TIM_GetCounter(TIM2) < 84 * 60000);

    GPIO_SetBits(GPIOD, GPIO_Pin_0); // Set trigger pin	

    TIM_SetCounter(TIM2, 0); // 10 microseconds delay
    while (TIM_GetCounter(TIM2) < 84 * 10);

    GPIO_ResetBits(GPIOD, GPIO_Pin_0); // Reset trigger pin

    // wait for echo 

    while (!(GPIO_ReadInputDataBit(ultraSonicSensor.GPIOUnit, ultraSonicSensor.Echo_Pin))) {
        echo_start = TIM_GetCounter(TIM2);
    }

    while (GPIO_ReadInputDataBit(ultraSonicSensor.GPIOUnit, ultraSonicSensor.Echo_Pin)) {
        echo_end = TIM_GetCounter(TIM2);

    }

    // Correction for clock during measurment.
    if (echo_start >= echo_end) {
        echo_time = (echo_end + (0xFFFFFFFF - echo_start)) / 84;
    } else {
        echo_time = (echo_end - echo_start) / 84;
    }
    ultraSonicSensor.Distance = echo_time / 58;
}

void main(void) {

    print("\nwelcome");

    Init_GPIO();
    TIM_Configration();
    EXTIInit();
    GPIO_ResetBits(ultraSonicSensor.GPIOUnit, ultraSonicSensor.Trig_Pin); //Reset trig pin

    while (1) {

        check_distance();

    }
}