#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stdio.h>
#include "timer.h"
#include "keypad.h"


__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void ) 
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ 
volatile(".L1: B .L1\n");				/* never return */
}

void keypad_init(void){	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitTypeDef keypad_In;
	GPIO_InitTypeDef keypad_Out;
	
	GPIO_StructInit(&keypad_In); // COLUMN 0-3 pin as input
	GPIO_StructInit(&keypad_Out); //ROW 4-7 as output
	
	keypad_In.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3;
	keypad_Out.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;
	
	keypad_In.GPIO_PuPd = GPIO_PuPd_DOWN; //PULLDOWN 
	
	keypad_In.GPIO_OType = GPIO_OType_PP; //push/pull
	keypad_Out.GPIO_OType = GPIO_OType_PP; //OPEN DRAIN
	
	keypad_In.GPIO_Mode = GPIO_Mode_IN;
	keypad_Out.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_Init(GPIOE, &keypad_In);
	GPIO_Init(GPIOE, &keypad_Out);
	
}

void buffer_init(void){
	lce_buffer _lce_buffer = {
		.current_password = {' '},
		.entered_characters_buffer = {' '},
		.entered_characters_timer = {' '},
	};
}


void keyboardActivate(unsigned int row){
	//Aktivera angiven rad hos tangentbordet eller deaktivera samtliga
	switch(row){
		case 1: GPIO_SetBits(GPIOE,GPIO_Pin_4); break; //GPIO_SetBits(GPIOD,GPIO_Pin_4)
		case 2: GPIO_SetBits(GPIOE,GPIO_Pin_5); break; //GPIO_SetBits(GPIOD,GPIO_Pin_5)
		case 3: GPIO_SetBits(GPIOE,GPIO_Pin_6); break; //GPIO_SetBits(GPIOD,GPIO_Pin_6)
		case 4: GPIO_SetBits(GPIOE,GPIO_Pin_7); break; //GPIO_SetBits(GPIOD,GPIO_Pin_7)
		case 0: GPIO_Write(GPIOE, 0x0000); break;
	}
}
int keyboardGetColumn(void){
	//Om någon tangent i den aktiverade raden är nedtryckt
	// returneras dess kolumnnummer. Annars 0.	
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)) { return 4; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)) { return 3; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)) { return 2; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)) { return 1; }
	return 0;
	}

char keyboard(void){
	unsigned const char key[] = {'1','2','3','0xA','4','5','6','0xB','7','8','9','0xC','0xE','0','0xF','0xD'};
	int row, col;
	for(row = 1; row < 5; row++){
		keyboardActivate(row);
		if(col = keyboardGetColumn()){
			keyboardActivate(0);
			return (key[4*(row-1) + (col-1)]);
		}
	}
	keyboardActivate(0);
	return 0xFF;
}

void store_password(void)
{
	for(int i = 0; i < PASSWORD_LENGTH; i++){
		unsigned char new_char = keyboard();
		_lce_buffer.current_password[i] = new_char;
		store_timer_to_buffer(i);
	}
}


void store_timer_to_buffer(unsigned int index){
	_lce_buffer.entered_characters_timer[index] ==  timer_ms;
}

void check_password(){

	unsigned char size;
	unsigned char min = get_indexof(_lce_buffer.entered_characters_timer, min_value(_lce_buffer.entered_characters_timer));
	for(int i = min; i < PASSWORD_LENGTH; i++){
		if(_lce_buffer.entered_characters_buffer[i] == current_password[size]) {
			size++;
		}
	} 
	return (size == sizeof(current_password));
}


void add_char_to_buffer( unsigned char new_char ) {
	unsigned int char_added = 0;

	for (int i = 0; i < sizeof(_lce_buffer.entered_characters_buffer); i++) {
		if (_lce_buffer.entered_characters_buffer[i] == '\0') {
			_lce_buffer.entered_characters_buffer[i] = new_char;
			store_timer_to_buffer(i);
			char_added = 1;
		}else
			{
			unsigned index = get_indexof(max_value(_lce_buffer.entered_characters_timer),_lce_buffer.entered_characters_timer);
			_lce_buffer.entered_characters_buffer[index] = new_char;
			char_added = 1;
		}
	}
}

unsigned int check_door_id(u_door* doors){
	unsigned char index_door_id = (min_value(_lce_buffer.entered_characters_timer));
	unsigned char door_id = _lce_buffer.entered_characters_buffer[index_door_id];
	switch (door_id){
		case '1': return 1;break;
		case '2': return 2;break;
		case '3': return 3;break;
		case '4': return 4;break;
	}
}


// save when a msg is entered by using timer in an array
// if array is full, no place to add char, swap the oldest element with the newest one and update timer for this new element
//

void main(void){
	buffer_init();
	timer_init();
	keypad_init();
	while(1){
		char new_char = keyboard();
		if (new_char != 0xFF) {
			add_char_to_buffer(new_char);
		}
	}
}


