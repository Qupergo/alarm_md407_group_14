

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "usart.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"


typedef struct {
	int ID;
	GPIO_TypeDef* GPIOUnit;
	uint16_t Vibration_Pin;
	
}VibrationSensor;

typedef struct {
	int ID;
	GPIO_TypeDef* GPIOUnit;
	uint16_t Trig_Pin;
	uint16_t Echo_Pin;
	int Distance;
	int Threshold;
}UltraSonicSensor;

unsigned int echo_start, echo_end, echo_time;
char a[30];