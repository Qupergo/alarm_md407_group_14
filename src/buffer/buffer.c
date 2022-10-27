#include "buffer.h"


char* get_latest_chars_entered(char_buffer* c_buffer, unsigned char amount_of_chars, char* latest_chars) {
	int index = c_buffer->current_index - 1;

	for (int i = 0; i < amount_of_chars; i++) {
		latest_chars[i] = c_buffer->entered_characters_buffer[index];
		index--;
		if (index < 0) {
			index = CHARACTER_BUFFER_LENGTH - 1;
		}
	}

	// Reverse the array as we check it backwards.
	int temp;
	for (int i = 0; i < amount_of_chars/2; i++) {
        temp = latest_chars[i];
        latest_chars[i] = latest_chars[amount_of_chars-i-1];
        latest_chars[amount_of_chars-i-1] = temp;
    }

	return latest_chars;
}

int check_password(char_buffer* c_buffer) {
	unsigned char size;
	char latest_password[PASSWORD_LENGTH];
	get_latest_chars_entered(c_buffer, PASSWORD_LENGTH, latest_password);
	for(int i = 0; i < PASSWORD_LENGTH; i++) {
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


int buffer_update(char_buffer* c_buffer) {
	if (new_char_available) {
		new_char_available = 0;
		if (new_buffer_char != 0xFF) {
			add_char_to_buffer(c_buffer, new_buffer_char);
			return 1;
		}
	}
	return 0;
}
