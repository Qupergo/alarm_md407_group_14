#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

volatile long timer_ms;

//timer functions
void timer_default_irq( void );
void timer_init( void );