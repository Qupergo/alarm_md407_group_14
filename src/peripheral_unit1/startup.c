/*
 * 	startup.c
 *
 */
 
 

#include "door.h"



__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}


#define NumberDoors 2
Door doors[NumberDoors];

void GPIOInit(void){
	GPIO_InitTypeDef GPIOConfig;
	GPIO_StructInit(&GPIOConfig);
	
	GPIOConfig.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4;
	GPIOConfig.GPIO_Mode = GPIO_Mode_IN;
	GPIOConfig.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOD,&GPIOConfig);
	
	GPIOConfig.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIOConfig.GPIO_Mode = GPIO_Mode_OUT;
	GPIOConfig.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD,&GPIOConfig);	
}
 
void CreateDoors (void){
	for (int i = 0; i < NumberDoors; i++){
		Door door = {
			i+1,
			1 << (4 * i),
			1 << ((4 * i) + 1),
			1 << ((4 * i) + 2),
			1 << ((4 * i )+ 3),
			0,
			0,
			4,
			0
		};
		doors[i] = door;
	} 
}

void check_switch(void){
	for (int i = 0; i < NumberDoors ; i++){
		if (!(GPIO_ReadInputDataBit(GPIOD, doors[i].Switch))){
			doors[i].Status_Lokal_alarm = 1;
			GPIO_ResetBits(GPIOD, doors[i].Closed);
		}else{
			doors[i].Status_Lokal_alarm = 0;
			doors[i].Status_Central_alarm = 0;
			doors[i].Timer = 0;
			GPIO_SetBits(GPIOD, doors[i].Closed);
			GPIO_ResetBits(GPIOD, doors[i].Local_alarm);
		}
		
	}
}


void TIM2_irq_handler(void){
	print("\nirq");
	for (int i = 0 ; i< NumberDoors ; i++){
		if ( doors[i].Status_Lokal_alarm | doors[i].Status_Central_alarm){
			doors[i].Timer++;
			print("\nIncremnt");
			if (doors[i].Timer >= 2 * doors[i].Time_threshold){
			print("\nCentral Alarm");
			}else if (doors[i].Timer >= doors[i].Time_threshold){
			GPIO_SetBits(GPIOD, doors[i].Local_alarm);
			doors[i].Status_Central_alarm = 1;
			
		}
		}
	}
	
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

void Tim_Init (void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	*((int *) 0x2001C0B0) =TIM2_irq_handler;
	TIM_TimeBaseInitTypeDef TIM_timer;
	TIM_timer.TIM_Prescaler = (42000 - 1);
	TIM_timer.TIM_Period = (2000-1);
	TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_timer);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef nvic_init;
	nvic_init.NVIC_IRQChannel = TIM2_IRQn;
	nvic_init.NVIC_IRQChannelPreemptionPriority = 0;
	nvic_init.NVIC_IRQChannelSubPriority = 0;
	nvic_init.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic_init);
	
	//*((void (**) (void)) 0x2001C0B0) =TIM2_irq_handler;
}
void main(void)
{
	GPIOInit();
	CreateDoors();
	Tim_Init();
	print("wlecome");
	//
	while (1) {
		check_switch();
		
	}
		
	
}

