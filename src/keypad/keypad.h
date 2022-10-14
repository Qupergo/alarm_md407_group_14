#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include <stdio.h>

#define PASSWORD_LENGTH 4
#define CHARACTER_BUFFER_LENGTH 8

//volatile static unsigned char current_password[PASSWORD_LENGTH] = {0x11, 0x11, 0x11, 0x11};
//volatile unsigned char latest_characters_entered[CHARACTER_BUFFER_LENGTH];


typedef struct characters_entered{
	unsigned char current_password[PASSWORD_LENGTH];
	unsigned char entered_characters_buffer[CHARACTER_BUFFER_LENGTH];
	unsigned char entered_characters_timer[CHARACTER_BUFFER_LENGTH];
}lce_buffer;

// user enters a PASSWORD_LENGTH digit code and the code is stored for later usage
void store_password();


//Check if the password enterd mathces the one stored by the user
void check_password();


//Initiates keypad settings
void keypad_init(void);

//initiates 7 segement unit
void seg7_init(void);


// Trigger the alarm in a 7 segement unit
void trigger_alarm_7seg();

unsigned char min_value(unsigned char ch[]){
	unsigned char min = ch[0];
	for(int i = 0 ; i < sizeof(ch); i++){
		if(ch[i] < min) {
			min = ch[i];
		}	
	}return min;
}

unsigned int get_indexof(unsigned char ch[], unsigned char elem){
	for(int i = 0 ; i < sizeof(ch); i++){
		if(ch[i] == elem){ return i; }
		}
}

unsigned char max_value(unsigned char ch[]){
	unsigned char max = ch[0];
	for(int i = 0 ; i < sizeof(ch); i++){
		if(max < ch[i]) {
			max = ch[i];
		}	
	}return max;
}