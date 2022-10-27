#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "buffer.h"

#define ROWS 4
#define COLS 4

unsigned char get_active_key(int col);
// Internal function used for keyboard()
int keyboardGetColumn(void);
// Internal function used for keyboard()
void keyboardActivate(unsigned int row);
// Inits the keypads GPIO ports and sets the c_buffer to its default values.
void keypad_init(char_buffer* c_buffer);