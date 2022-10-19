#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"

char input_buffer[32];
char buffer_index = 0;
char send_message_flag = 0;
char clear_buffer_flag = 0;

void USART_irq_handler(void);
void USART1_Init(void);
void USART_Snd(char c);
void USART_Snd_Str(char* s);
void USART_Snd_StrLn(char* s);
void USART_Snd_Int(int x);
void clear_input_buffer(void);
char* USART1_receive_data(void);