#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#define PASSWORD_LENGTH 4
#define CHARACTER_BUFFER_LENGTH 8

typedef struct latest_characters_entered {
	unsigned char current_password[PASSWORD_LENGTH];
	unsigned char entered_characters_buffer[CHARACTER_BUFFER_LENGTH];
	unsigned char current_index;
} char_buffer;

char_buffer latest_char_buffer;
// Start with password 0000

//latest_char_buffer.current_password = {'0','0','0','0'};

// Add a new char to the char_buffer 
void add_char_to_buffer(char new_char);
// Should be periodically called to see if a key is being pressed and adds it to the buffer 
int keypad_update();
// Increments the current index in the buffer
void increment_index();
// Compares the PASSWORD_LENGTH latest characters to the current password
int check_password();
// Get the N latest chars entered
char* get_latest_chars_entered(unsigned char amount_of_chars);
// Returns a char pressed on the keypad
unsigned char keyboard(void);
// Internal function used for keyboard()
int keyboardGetColumn(void);
// Internal function used for keyboard()
void keyboardActivate(unsigned int row);
// Inits the keypad
void keypad_init(void);