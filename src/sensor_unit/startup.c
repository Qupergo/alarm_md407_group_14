#include "sensor_unit.h"

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
    GPIO_Pin_4, // DI pin
    0  // initial alarm status value
};

u_ultrasonic_sensor ultraSonicSensor = {
    2, // ID 
    GPIOD, // GPIO_Unit
    GPIO_Pin_0, // trig pin
    GPIO_Pin_1, // echo pin
    GPIO_Pin_2, // led pin
    0, // distance
    0, // local alarm distance 
    0, // central alarm distance
    0  // initial alarm status value
};

unsigned char SELF_TYPE = TYPE_SENSOR_UNIT;
unsigned char num_sub_units = 2;
unsigned char self_id;
int send_alarm = 0;


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
    TIM_timer.TIM_Period = 0xFFFFFFFF; // maximum value of period
    TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_timer);
    TIM_Cmd(TIM5, ENABLE);
}

void ultraSonic_irq_handler(void) {
    if (distance < ultraSonicSensor.local_alarm_distance_threshold) {
        GPIO_SetBits(GPIOD, ultraSonicSensor.alarm_led);
        ultraSonicSensor.status_local_alarm = 1; // set value to 1 indicating local alarm is sat off

        if (distance < ultraSonicSensor.central_alarm_distance_threshold) {
            send_alarm = 1;
        }
    }
    else {
        // Turn off the led
        GPIO_ResetBits(GPIOD, ultraSonicSensor.alarm_led);
        ultraSonicSensor.status_local_alarm = 0; // set value to 0 indicating no alarm is sat off
    }

    EXTI_ClearFlag(EXTI_Line1);
}

void vibration_irq_handler(void) {
    if (!GPIO_ReadInputDataBit(vibrationSensor.GPIO_unit, vibrationSensor.vibration_pin)) {
        send_alarm = 1;
    }

    EXTI_ClearFlag(EXTI_Line4);
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

    nvic_init.NVIC_IRQChannel = EXTI1_IRQn;        //Configration of NVIC for UltraSonic_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void (**) (void)) 0x2001C05C) = ultraSonic_irq_handler;


    nvic_init.NVIC_IRQChannel = EXTI4_IRQn; //Configration of NVIC for vibration_irq_handler
    nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
    nvic_init.NVIC_IRQChannelSubPriority = 0x00;
    nvic_init.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init);

    *((void(**)(void)) 0x2001C068) = vibration_irq_handler;
}

int check_distance(void) {
    TIM_SetCounter(TIM5, 0); // 65 microseconds delay between measurments 
    while (TIM_GetCounter(TIM5) < 65000);

    GPIO_SetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.trig_pin); // Set trigger pin	

    TIM_SetCounter(TIM5, 0); // 10 microseconds delay
    while (TIM_GetCounter(TIM5) < 10);

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

    distance = echo_time / 58;  // distance in centimeters
    return distance;
}

// sets initial values for local alarm distance and central alarm distance 
void set_threshold_values(void) {
    GPIO_ResetBits(ultraSonicSensor.GPIO_unit, ultraSonicSensor.trig_pin);  // reset trig pin
    ultraSonicSensor.initial_distance = check_distance();
    ultraSonicSensor.local_alarm_distance_threshold = LOCAL_ALARM_DISTANCE_FACTOR_THRESHOLD * ultraSonicSensor.initial_distance;  // initial value for local alaram distance 
    ultraSonicSensor.central_alarm_distance_threshold = CENTRAL_ALARM_DISTANCE_FACTOR_THRESHOLD * ultraSonicSensor.initial_distance; // initial value for central alarm distance 

    print_line("The alarmed zone is ");
    print_int(ultraSonicSensor.initial_distance);
    print("cm away");

    print_line("The local alarm threshold is between ");
    print_int(ultraSonicSensor.local_alarm_distance_threshold);
    print("cm and ");
    print_int(ultraSonicSensor.central_alarm_distance_threshold);
    print("cm");

    print_line("The central alarm threshold activates at ");
    print_int(ultraSonicSensor.central_alarm_distance_threshold);
    print("cm");
}

void main(void) {
    print("\nSensor unit started!\n");
    Init_GPIO();
    TIM_Configration();
    set_threshold_values();
    timer_init();
    EXTIInit();

    rx_can_msg rx_msg;
    rt_info _rt_info;
    ls_info _ls_info;

    can_init(&_rt_info, &_ls_info, CAN1, 0);

    // Send initial alive message to central unit
    tx_can_msg initial_alive;
    initial_alive.priority = 1;
    initial_alive.message_type = MSGID_NEW_ALIVE;
    initial_alive.reciever_id = 0;
    initial_alive.content[0] = SELF_TYPE;
    initial_alive.content[1] = num_sub_units;
	initial_alive.sequence_n = _rt_info.transmit_sequence_num[0];
    can_send_message(&_rt_info, CAN1, initial_alive);
    unsigned char waiting_for_alive_response = 1;

    while (1) {
        check_distance();
        if (send_alarm) {
            tx_can_msg msg_alarm = {
                .priority = 0,
                .message_type = MSGID_START_ALARM,
                .reciever_id = 0,
				.sequence_n = _rt_info.transmit_sequence_num[0],
            };

            can_send_message(&_rt_info, CAN1, msg_alarm);
            send_alarm = 0;
        }
        // Only start updating units after they have recieved their alive response
        // So they have an assigned ID
        if (!waiting_for_alive_response) {
            can_update(&_rt_info, &_ls_info);
        }

        if (can_receive_message(&_rt_info, &_ls_info, CAN1, &rx_msg)) {
            switch (rx_msg.message_type) {
            case MSGID_NEW_ALIVE_RESPONSE:
                if (waiting_for_alive_response) {
                    if (rx_msg.content[0] == SELF_TYPE) {
                        waiting_for_alive_response = 0;
                        self_id = rx_msg.content[1];
						can_init_filter(self_id);
                        _rt_info.recieve_sequence_num[rx_msg.sender_id] = 0;
                        _rt_info.transmit_sequence_num[rx_msg.sender_id] = 1;
                    }
                }
                break;
            case MSGID_SENSOR_DISTANCE_THRESHOLD:;
                ultraSonicSensor.local_alarm_distance_threshold = rx_msg.content[0]; //rx_msg.content[0] is in centimeters
                // Divide by local alarm factor to get the total distance 
                ultraSonicSensor.central_alarm_distance_threshold = (ultraSonicSensor.local_alarm_distance_threshold / LOCAL_ALARM_DISTANCE_FACTOR_THRESHOLD) * CENTRAL_ALARM_DISTANCE_FACTOR_THRESHOLD;
                break;

            case MSGID_RECALIBERATE:
                set_threshold_values();
                break;
            case MSGID_RESET_UNIT:
                Init_GPIO();
                TIM_Configration();
                set_threshold_values();
                timer_init();
                EXTIInit();
                break;
            }
        }
    }
}

