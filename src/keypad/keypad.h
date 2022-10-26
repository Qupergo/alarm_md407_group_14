#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"

#define ROWS 4
#define COLS 4
#define PASSWORD_LENGTH 4
#define CHARACTER_BUFFER_LENGTH 8

typedef struct latest_characters_entered {
	char current_password[PASSWORD_LENGTH];
	unsigned char entered_characters_buffer[CHARACTER_BUFFER_LENGTH];
	unsigned char current_index;
} char_buffer;

// Add a new char to the char_buffer 
void add_char_to_buffer(char_buffer* c_buffer, char new_char);
// Should be periodically called to see if a key is being pressed and adds it to the buffer
// Returns 1 when a new key has been entered
int keypad_update(char_buffer* c_buffer);
// Increments the current index in the buffer
void increment_index(char_buffer* c_buffer);
// Compares the PASSWORD_LENGTH latest characters to the current password
int check_password(char_buffer* c_buffer);
// Get the N latest chars entered
char* get_latest_chars_entered(char_buffer* c_buffer, unsigned char amount_of_chars, char* latest_chars);
// Returns a char pressed on the keypad
unsigned char get_active_key(int col);
// Internal function used for keyboard()
int keyboardGetColumn(void);
// Internal function used for keyboard()
void keyboardActivate(unsigned int row);
// Inits the keypads GPIO ports and sets the c_buffer to its default values.
void keypad_init(char_buffer* c_buffer);