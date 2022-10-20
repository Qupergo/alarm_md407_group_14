#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
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


void keypad_init(void) {	
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

void keyboardActivate(unsigned int row) {
	//Aktivera angiven rad hos tangentbordet eller deaktivera samtliga
	switch(row){
		case 1: GPIO_SetBits(GPIOE,GPIO_Pin_4); break; //GPIO_SetBits(GPIOD,GPIO_Pin_4)
		case 2: GPIO_SetBits(GPIOE,GPIO_Pin_5); break; //GPIO_SetBits(GPIOD,GPIO_Pin_5)
		case 3: GPIO_SetBits(GPIOE,GPIO_Pin_6); break; //GPIO_SetBits(GPIOD,GPIO_Pin_6)
		case 4: GPIO_SetBits(GPIOE,GPIO_Pin_7); break; //GPIO_SetBits(GPIOD,GPIO_Pin_7)
		case 0: GPIO_Write(GPIOE, 0x0000); break;
	}
}

int keyboardGetColumn(void) {
	//Om någon tangent i den aktiverade raden är nedtryckt
	// returneras dess kolumnnummer. Annars 0.	
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3)) { return 4; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_2)) { return 3; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1)) { return 2; }
	if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)) { return 1; }
	return 0;
	}

unsigned char keyboard(void) {
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

char* get_latest_chars_entered(unsigned char amount_of_chars) {
	if (amount_of_chars > CHARACTER_BUFFER_LENGTH) {
		print("Invalid amount_of_chars value entered");
		return 0;
	}
	char* latest_chars;
	int index = latest_char_buffer.current_index;

	for (int i = 0; i < amount_of_chars; i++) {
		*latest_chars = latest_char_buffer.entered_characters_buffer[index];
		latest_chars++;
		index--;
		if (index < 0) {
			index = CHARACTER_BUFFER_LENGTH;
		}
	}
	return latest_chars;
}

int check_password() {
	unsigned char size;
	char* latest_password;
	latest_password = get_latest_chars_entered(PASSWORD_LENGTH);
	for(int i = 0; i < PASSWORD_LENGTH; i++){
		if(latest_password[i] != latest_char_buffer.current_password[i]) {
			return 0;
		}
	}
	return 1;
}

void increment_index() {
	latest_char_buffer.current_index++;
	if (latest_char_buffer.current_index > (CHARACTER_BUFFER_LENGTH - 1)) {
		latest_char_buffer.current_index = 0;
	}
}

void add_char_to_buffer( char new_char ) {
	increment_index();
	latest_char_buffer.entered_characters_buffer[latest_char_buffer.current_index] = new_char;
}

void reset_buffer( void ) {
	for(int i = 0; i < CHARACTER_BUFFER_LENGTH; i++) {
		latest_char_buffer.entered_characters_buffer[i] = '\0';
	}
	latest_char_buffer.current_index = 0;
}

int keypad_update() {
	unsigned char new_char = keyboard();
	// A new character has been entered, add it to the buffer if it is a new char
	if (new_char != latest_entered_character) {
		// If no button is pressed, keyboard returns 0xFF
		// This means a character has been let go, don't add 0xFF to the buffer but update latest_entered_character
		latest_entered_character = new_char;
		if (new_char != 0xFF) {
			add_char_to_buffer(new_char);
			return 1;
		}
	}
	return 0
}