#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_tim.h"


char input_buffer[32];
char buffer_index ;
char send_message_flag ;
char clear_buffer_flag;

void USART_irq_handler(void);
void USART1_Init(void);
void USART_Snd(char c);
void print(char* s);
void print_line(char* s);
void print_int(int x);
void clear_input_buffer(void);
char USART1_receive_data(void);