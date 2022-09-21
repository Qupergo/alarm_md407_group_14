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
void app_init(void){
	
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	GPIO_InitTypeDef keypad_In;
	GPIO_StructInit(&keypad_In); // COLOM 0-3 pin as input
	keypad_In.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3;
	keypad_In.GPIO_Mode = GPIO_Mode_IN;
	keypad_In.GPIO_PuPd = GPIO_PuPd_DOWN; //PULLDOWN ?
	keypad_In.GPIO_OType = GPIO_OType_PP; //push/pull

	GPIO_Init(GPIOD, &keypad_In);
	
	GPIO_InitTypeDef keypad_Out;	
	GPIO_StructInit(&keypad_Out); //ROW
	keypad_Out.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	keypad_Out.GPIO_Mode = GPIO_Mode_OUT;
	keypad_Out.GPIO_PuPd = GPIO_PuPd_NOPULL; //
	keypad_Out.GPIO_OType = GPIO_OType_OD; //PUSH PULL
	//keypad_Out.GPIO_Speed = GPIO_Medium_Speed;
	
	GPIO_Init(GPIOD, &keypad_Out);


	GPIO_InitTypeDef out7seg;
	GPIO_StructInit(&out7seg);
	out7seg.GPIO_Pin	=   GPIO_Pin_8| GPIO_Pin_9| GPIO_Pin_10| GPIO_Pin_11|
							GPIO_Pin_12| GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	out7seg.GPIO_Mode	= GPIO_Mode_OUT;
	GPIO_ReadOutputDataBit(GPIOD, out7seg.GPIO_Pin);
	GPIO_Init(GPIOD, &out7seg);
	GPIO_SetBits(GPIOD, out7seg.GPIO_Pin); // Set pin to high level....idk why

 
 }
void keyboardActivate(unsigned int row){
	//Aktivera angiven rad hos tangentbordet eller deaktivera samtliga
	switch(row){
		case 1: GPIO_SetBits(GPIOD,GPIO_Pin_4); break; //GPIO_SetBits(GPIOD,GPIO_Pin_4)
		case 2: GPIO_SetBits(GPIOD,GPIO_Pin_5); break; //GPIO_SetBits(GPIOD,GPIO_Pin_5)
		case 3: GPIO_SetBits(GPIOD,GPIO_Pin_6); break; //GPIO_SetBits(GPIOD,GPIO_Pin_6)
		case 4: GPIO_SetBits(GPIOD,GPIO_Pin_7); break; //GPIO_SetBits(GPIOD,GPIO_Pin_7)
		case 0: GPIO_Write(GPIOD, 0x0000); break;
	}
}

int keyboardGetColumn(void){
	//Om någon tangent i den aktiverade raden är nedtryckt
	// returneras dess kolumnnummer. Annars 0.	
	unsigned char c;
	c = GPIO_ReadInputData(GPIOD);
	if(c & 0x0008){return 4;}
	if(c & 0x0004){return 3;}
	if(c & 0x0002){return 2;}
	if(c & 0x0001){return 1;}
	return 0;
}
unsigned char keyboard(void){
	unsigned char key[] = {1,2,3,0xA,4,5,6,0xB,7,8,9,0xC,0xE,0,0xF,0xD};
	int row, col;
	for(row=1; row<=4; row++){
		keyboardActivate(row);
		if(col = keyboardGetColumn()){
			keyboardActivate(0);
			return key[4*(row-1) + (col-1)]; //
		}
	}
	keyboardActivate(0);
	return 0xFF;
}
void out7seg(unsigned char c){
    
	//Segment-array över hur siffrorna skall synas på skärmen
    unsigned short result[] = {0x3F00, 0x0600, 0x5B00, 0x4F00,
							0x6600, 0x6D00, 0x7D00, 0x0700, 0x7F00, 
							0x6F00, 0x7700, 0x7C00, 0x5800, 0x5E00, 0x8000, 0x7600};
    if(c > 0xFF00){
         GPIO_Write(GPIOD, 0x00);
    }else{
        GPIO_Write(GPIOD, result[c]);
    }
	 
}

unsigned int converter(unsigned char c){
	switch (c){
		case '1': print("1"); break;
		case '2': print("2"); break;
		case '3': print("3"); break;
		case '4': print("4"); break;
		case '5': print("5"); break;
		case '6': print("6"); break;
		case '7': print("7"); break;
		case '8': print("8"); break;
		}
}
void main(void){
	app_init();
	while(1){
	//out7seg(keyboard());
	print(converter(keyboard()));
	}
}
