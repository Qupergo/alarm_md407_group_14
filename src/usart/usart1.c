/*
 * 	startup.c
 *
 */
#include "usart1.h"
#include "misc.h"
#include "buffer.h"



// every time a key is pressed the program will jump to USART_irq_handler
void USART_irq_handler(void) {
	USART_ClearITPendingBit(USART1, USART_IT_RXNE); 
	char input = USART_ReceiveData(USART1);
	new_buffer_char = input - 48; // Offset by 48 to give actual integer value when writing numbers instead of ascii character value
	new_char_available = 1;
	
	USART_Snd(input);
}



void USART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);


	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1); //Connect PA9 to USART1_Tx
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); //Connect PA10 to USART1_Rx


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	USART_InitStructure.USART_BaudRate = 124400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Cmd(USART1, ENABLE);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitTypeDef nvic_init;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	nvic_init.NVIC_IRQChannel = USART1_IRQn;        //Configration of NVIC for USART1
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0x00;
	nvic_init.NVIC_IRQChannelSubPriority = 0x00;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);

	*((void (**) (void)) 0x2001C0D4) = USART_irq_handler;
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
