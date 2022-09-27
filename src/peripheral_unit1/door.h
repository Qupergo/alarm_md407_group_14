
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
	
	uint16_t Switch;
	uint16_t Closed;
	uint16_t Local_alarm;
	uint16_t Locked;
	char Status_Central_alarm;
	char Status_Lokal_alarm;
	int Time_threshold;
	int Timer
	
	
	
}Door;
