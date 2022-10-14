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

u_vibration_sensor vibrationSensor = {
    1, // ID
    GPIOD, // GPIO_Unit
    GPIO_Pin_4 // DI pin
};

u_ultrasonic_sensor ultraSonicSensor = {
    2, // ID 
    GPIOD, // GPIO_Unit
    GPIO_Pin_0, // trig pin
    GPIO_Pin_1, // echo pin
    GPIO_Pin_2, // led pin
    0, // distance
    0, // local alarm distance 
    0 // central alarm distance
};

void Init_GPIO(void) {
    // Configration of GPIO pins and ports 
    GPIO_InitTypeDef GPIOInit;
    GPIO_StructInit(&GPIOInit);
    GPIOInit.GPIO_Pin = ultraSonicSensor.trig_pin | ultraSonicSensor.alarm_led;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIOInit.GPIO_OType = GPIO_OType_PP;
    GPIOInit.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(ultraSonicSensor.GPIO_unit, &GPIOInit);

    GPIO_StructInit(&GPIOInit);
    GPIOInit.GPIO_Pin = ultraSonicSensor.echo_pin | vibrationSensor.vibration_pin;
    GPIOInit.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIOInit.GPIO_OType = GPIO_OType_PP;
    GPIOInit.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOD, &GPIOInit);

}

void TIM_Configration(void) {
    // Configration of timer on TIM5 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_timer;
    TIM_timer.TIM_Prescaler = (84 - 1); // set prescaler 83 to get one tick each micro second
    TIM_timer.TIM_Period = 0xFFFFFFFF; // maximum value of prescaler
    TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_timer);
    TIM_Cmd(TIM5, ENABLE);
}

void vibration_irq_handler(void) {
    if (!GPIO_ReadInputDataBit(vibrationSensor.GPIO_unit, vibrationSensor.vibration_pin)) {
        print("vib");
    }

    EXTI_ClearFlag(EXTI_Line4);
}

void ultraSonic_irq_handler(void) {
    if (echo_distance < ultraSonicSensor.central_alarm) {
        print("Central alarm");

    }
    else if (echo_distance < ultraSonicSensor.local_alarm) {
        GPIO_SetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.alarm_led);   // local alarm
       

    }
    else {
        GPIO_ResetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.alarm_led);

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

    nvic_init.NVIC_IRQChannel = EXTI1_IRQn; //Configration of NVIC for ultraSonic_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void( ** )(void)) 0x2001C05C) = ultraSonic_irq_handler;

    nvic_init.NVIC_IRQChannel = EXTI4_IRQn; //Configration of NVIC for vibration_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void( ** )(void)) 0x2001C068) = vibration_irq_handler;
}

void check_distance(void) {
    TIM_SetCounter(TIM5, 0); // 65 microseconds delay between measurments 
    while (TIM_GetCounter(TIM5) < 84 );

    GPIO_SetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.trig_pin); // Set trigger pin	

    TIM_SetCounter(TIM5, 0); // 10 microseconds delay
    while (TIM_GetCounter(TIM5) <  10);

    GPIO_ResetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.trig_pin); // Reset trigger pin

    // wait for echo 

    while (!(GPIO_ReadInputDataBit(ultraSonicSensor.GPIO_unit, ultraSonicSensor.echo_pin))) {
        echo_start = TIM_GetCounter(TIM5);
    }

    while (GPIO_ReadInputDataBit(ultraSonicSensor.GPIO_unit, ultraSonicSensor.echo_pin)) {
        echo_end = TIM_GetCounter(TIM5);

    }

    // Correction for clock during measurment.
    if (echo_start >= echo_end) {
        echo_time = echo_end + (0xFFFFFFFF - echo_start);
    }
    else {
        echo_time = (echo_end - echo_start);
    }
    echo_distance = echo_time / 58;  // distance in centimeters
    return echo_distance;
}

void InitialValues(void) {
    GPIO_ResetBits(ultraSonicSensor.GPIOUnit, ultraSonicSensor.trig_pin);      //Reset trig pin
    ultraSonicSensor.distance = check_distance();  // initial value for distance 
    ultraSonicSensor.local_alarm = 0.8 * echo_distance;  // initial value for local alaram distance 
    ultraSonicSensor.central_alarm = 0.5 * echo_distance; // initial value for central alarm distance 

    print(" \ndistance is :");
    print(itoa(echo_distance, a, 10));
    print(" \nLocal alarm distance is :");
    print(itoa(ultraSonicSensor.local_alarm, a, 10));
    print(" \ncentral distance is :");
    print(itoa(ultraSonicSensor.central_alarm, a, 10));


}



void main(void) {

    print("\nwelcome");

    Init_GPIO();
    TIM_Configration();
    InitialValues();
    can_init(CAN1);
    timer_init();	
    EXTIInit();
    USART1_Init();

    while (1) {
        check_distance();
    }
}