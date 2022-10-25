#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"


void USART_irq_handler(void);
void USART1_Init(void);
void USART_Snd(char c);
void USART_Snd_Str(char* s);
void USART_Snd_StrLn(char* s);
void USART_Snd_Int(int x);
void clear_input_buffer(void);
char* USART1_receive_data(void);