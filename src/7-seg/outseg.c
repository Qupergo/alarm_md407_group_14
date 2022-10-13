#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

void seg7_init(void){
	
	GPIO_InitTypeDef seg7;
	GPIO_StructInit(&seg7);
	seg7.GPIO_Pin	=   GPIO_Pin_8| GPIO_Pin_9| GPIO_Pin_10| GPIO_Pin_11|
							GPIO_Pin_12| GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	seg7.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_Init(GPIOE, &seg7);
 }
 
void seg7(unsigned char c){
	//Segment-array över hur siffrorna skall synas på skärmen
    unsigned short result[] = {0x3F00, 0x0600, 0x5B00, 0x4F00,
							0x6600, 0x6D00, 0x7D00, 0x0700, 0x7F00, 
							0x6F00, 0x7700, 0x7C00, 0x5800, 0x5E00, 0x8000, 0x7600};
    if(c > 0xFF00){
         GPIO_Write(GPIOE, 0x00);
    }else{
        GPIO_Write(GPIOE, result[c]);
    }	 
}

void trigger_alarm_7seg(){
	GPIO_Write(GPIOE, 0xFF);
	GPIO_Write(GPIOE, 0x00);
	GPIO_Write(GPIOE, 0xFF);
	GPIO_Write(GPIOE, 0x00);
}

void main(void)
{
	seg7_init();
}

