//#include "stm32f4xx.h"
//#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_rcc.h"
#include "timer.h"
#include "keypad.h"


unsigned char latest_entered_character = 0xFF;
unsigned char prelim_char = 0xFF;
unsigned char keyboard_character;
long entered_key_timestamp = 0;
int new_char_available = 0;

int EXTI_Lines[COLS] = {EXTI_Line0, EXTI_Line1, EXTI_Line2, EXTI_Line3};
int EXTI_PinSources[COLS] = {EXTI_PinSource0, EXTI_PinSource1, EXTI_PinSource2, EXTI_PinSource3};
int EXTI_IRQs[COLS] = {EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn};

void keypadirq_handler() {
	for (int i = 0; i < COLS; i++) {
		print("IRQ HANDLING ");
		print_int(i);
		print("\n");
		TIM_SetCounter(TIM5, 0);
		while (TIM_GetCounter(TIM5) < 250); // Delay for 250 microseconds
		
		if(EXTI_GetITStatus(EXTI_Lines[i]) != RESET) {
			keyboard_character = get_active_key(i);
			print_int(keyboard_character);
			new_char_available = 1;
			
			TIM_SetCounter(TIM5, 0);
			while (TIM_GetCounter(TIM5) < 250 ); // Delay another 250 microseconds
			
			EXTI_ClearITPendingBit(EXTI_Lines[i]);	
		}
	}
}

void keypad_init(char_buffer* c_buffer) {
	// Re-allocate vectortable
	*((unsigned long *)0xE000ED08) = 0x2001C000;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
    TIM_TimeBaseInitTypeDef TIM_timer;
    TIM_timer.TIM_Prescaler = (84 - 1); // set prescaler 83 to get one tick each micro second
    TIM_timer.TIM_Period = 0xFFFFFFFF; // maximum value of prescaler
    TIM_timer.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_timer.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM5, &TIM_timer);
    TIM_Cmd(TIM5, ENABLE);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	RCC_AHB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitTypeDef keypad_In;
	GPIO_InitTypeDef keypad_Out;

	EXTI_InitTypeDef keypadirq;
	NVIC_InitTypeDef NVICkeypad;

	GPIO_StructInit(&keypad_In); // COLUMN 0-3 pin as input
	GPIO_StructInit(&keypad_Out); //ROW 4-7 as output

	keypad_In.GPIO_Pin = GPIO_Pin_0| GPIO_Pin_1| GPIO_Pin_2| GPIO_Pin_3;
	keypad_Out.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_6| GPIO_Pin_7;

	keypad_In.GPIO_PuPd = GPIO_PuPd_DOWN;
	
	keypad_In.GPIO_OType = GPIO_OType_PP;
	keypad_Out.GPIO_OType = GPIO_OType_PP;
	
	keypad_In.GPIO_Mode = GPIO_Mode_IN;
	keypad_Out.GPIO_Mode = GPIO_Mode_OUT;

	GPIO_Init(GPIOE, &keypad_In);
	GPIO_Init(GPIOE, &keypad_Out);

	for (int i = 0; i < COLS; i++) {
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSources[i]);
		keypadirq.EXTI_Line = EXTI_Lines[i];
		keypadirq.EXTI_Mode = EXTI_Mode_Interrupt;
		keypadirq.EXTI_Trigger = EXTI_Trigger_Rising;
		keypadirq.EXTI_LineCmd = ENABLE;
		
		EXTI_Init(&keypadirq);
	
		NVICkeypad.NVIC_IRQChannel = EXTI_IRQs[i];
		NVICkeypad.NVIC_IRQChannelPreemptionPriority = 1;
		NVICkeypad.NVIC_IRQChannelSubPriority = 0;
		NVICkeypad.NVIC_IRQChannelCmd = ENABLE;
		*((int* ) (0x2001C058 + i*4)) = keypadirq_handler;
		
		NVIC_Init(&NVICkeypad);
	}

	// Activate all rows
	for(int row = 1; row <= ROWS; row++) {
		keyboardActivate(row);
	}
	
	for (int i = 0; i < PASSWORD_LENGTH; i++) {
		c_buffer->current_password[i] = 1;
	}
	
	for (int i = 0; i < CHARACTER_BUFFER_LENGTH; i++) {
		c_buffer->entered_characters_buffer[i] = 0xFF;
	}
	
	c_buffer->current_index = 0;
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

unsigned char get_active_key(int col) {
	keyboardActivate(0); // Deactivate keyboard pins
	unsigned const char key[] = {1, 2, 3, 'A', 4, 5, 6, 'B', 7, 8, 9, 'C', 'E', 0, 'F','D'};
	int row;
	char return_key = 0xFF;
	for(row = 1; row <= ROWS; row++) {
		keyboardActivate(row);
		if(keyboardGetColumn()) {
			return_key = (key[ROWS*(row-1) + (col)]);
			break;
		}
	}
	
	for(int row = 1; row <= ROWS; row++) {
		keyboardActivate(row);
	}
	return return_key;
}

char* get_latest_chars_entered(char_buffer* c_buffer, unsigned char amount_of_chars, char* latest_chars) {
	if (amount_of_chars > CHARACTER_BUFFER_LENGTH) {
		print_line("Invalid amount_of_chars value entered");
		return 0;
	}
	int index = c_buffer->current_index;

	for (int i = 0; i < amount_of_chars; i++) {
		latest_chars[i] = c_buffer->entered_characters_buffer[index];
		index--;
		if (index < 0) {
			index = CHARACTER_BUFFER_LENGTH - 1;
		}
	}
	return latest_chars;
}

int check_password(char_buffer* c_buffer) {
	unsigned char size;
	char latest_password[PASSWORD_LENGTH];
	get_latest_chars_entered(c_buffer, PASSWORD_LENGTH, latest_password);
	for(int i = 0; i < PASSWORD_LENGTH; i++){
		if(latest_password[i] != c_buffer->current_password[i]) {
			return 0;
		}
	}
	return 1;
}

void increment_index(char_buffer* c_buffer) {
	c_buffer->current_index++;
	if (c_buffer->current_index > (CHARACTER_BUFFER_LENGTH - 1)) {
		c_buffer->current_index = 0;
	}
}

void add_char_to_buffer(char_buffer* c_buffer, char new_char ) {
	c_buffer->entered_characters_buffer[c_buffer->current_index] = new_char;
	increment_index(c_buffer);
}

void reset_buffer( char_buffer* c_buffer ) {
	for(int i = 0; i < CHARACTER_BUFFER_LENGTH; i++) {
		c_buffer->entered_characters_buffer[i] = '\0';
	}
	c_buffer->current_index = 0;
}


void print_buffer(char_buffer* c_buffer) {
	char ordered_entered_chars[CHARACTER_BUFFER_LENGTH];
	get_latest_chars_entered(c_buffer, CHARACTER_BUFFER_LENGTH, ordered_entered_chars);

	for(int i = 0; i < CHARACTER_BUFFER_LENGTH; i++) {
		if(ordered_entered_chars[i] >= 0 && ordered_entered_chars[i] <= 9)
			print_int(ordered_entered_chars[i]);
		else if (ordered_entered_chars[i] >= 9 && ordered_entered_chars[i] < 0xFF) {
			print(ordered_entered_chars[i]);			
		}
	}
	print("\n");
}

int keypad_update(char_buffer* c_buffer) {
	if (new_char_available) {
		new_char_available = 0;
		if (keyboard_character != 0xFF) {
			add_char_to_buffer(c_buffer, keyboard_character);
			return 1;
		}
	}
	return 0;
}