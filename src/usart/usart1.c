/*
 * 	startup.c
 *
 */
#include "usart1.h"

char input_buffer[32];
char buffer_index = 0;
char send_message_flag = 0;
char clear_buffer_flag = 0;


void USART_irq_handler(void) {

	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	if (clear_buffer_flag) {
		clear_buffer_flag = 0;
		clear_input_buffer();
	}

	char input = USART_ReceiveData(USART1);

	// if enter is pressed
	if (input == '\n' || buffer_index == (sizeof(input_buffer) / sizeof(int)) - 1) {
		send_message_flag = 1;
		clear_buffer_flag = 1;
	}
	else {
		USART_Snd(input);
		input_buffer[buffer_index] = input;
		buffer_index++;
	}



}

void USART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); //Connect PA9 to USART1_Tx
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); //Connect PA10 to USART1_Rx
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	*((int*)0x2001C0D4) = USART_irq_handler;
}

void USART_Snd(char c) {
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData(USART1, c);
}

void print(char* s) {
	while (*s != '\0') {
		USART_Snd(*(s++));
	}
}

void print_line(char* s) {
	while (*s != '\0') {
		USART_Snd(*(s++));
	}
	USART_Snd('\n');
}

void print_int(int x) {
	char a[32];
	print(itoa(x, a, 10));

}

void clear_input_buffer(void) {
	for (int i = 0; i < (sizeof(input_buffer) / sizeof(int)); i++) {
		input_buffer[i] = '\0';
	}
}

char* USART1_receive_data(void) {
	send_message_flag = 0;
	buffer_index = 0;
	return input_buffer;

}
