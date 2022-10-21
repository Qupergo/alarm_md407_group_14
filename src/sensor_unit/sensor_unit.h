
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include "can.h"
#include "timer.h"
#include "usart1.h"

#define LOCAL_ALARM_DISTANCE_FACTOR_THRESHOLD 0.8
#define CENTRAL_ALARM_DISTANCE_FACTOR_THRESHOLD 0.5

typedef struct {
	int id;
	GPIO_TypeDef* GPIO_unit;
	uint16_t vibration_pin;
	int	status_local_alarm;
} u_vibration_sensor;

typedef struct {
	int id;
	GPIO_TypeDef* GPIO_unit;
	uint16_t trig_pin;
	uint16_t echo_pin;
	uint16_t alarm_led;
	int initial_distance;
	int local_alarm_distance_threshold;
	int central_alarm_distance_threshold;
	int	status_local_alarm;
} u_ultrasonic_sensor;


int echo_start, echo_end, echo_time, echo_distance, distance;