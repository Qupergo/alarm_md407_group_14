#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "usart.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"


typedef struct {
	int id;
	GPIO_TypeDef* GPIO_unit;
	uint16_t vibration_pin;
} u_vibration_sensor;

typedef struct {
	int id;
	GPIO_TypeDef* GPIO_unit;
	uint16_t trig_pin;
	uint16_t echo_pin;
	int distance;
	int threshold;
} u_ultrasonic_sensor;

unsigned int echo_start, echo_end, echo_time;
char a[30];