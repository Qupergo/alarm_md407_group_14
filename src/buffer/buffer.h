#define PASSWORD_LENGTH 4
#define CHARACTER_BUFFER_LENGTH 16

typedef struct latest_characters_entered {
	char current_password[PASSWORD_LENGTH];
	unsigned char entered_characters_buffer[CHARACTER_BUFFER_LENGTH];
	unsigned char current_index;
} char_buffer;

char new_buffer_char;
int new_char_available;

// Add a new char to the char_buffer 
void add_char_to_buffer(char_buffer* c_buffer, char new_char);
// Should be periodically called to see if a char should be added to the buffer
// Returns 1 when a new key has been entered
int buffer_update(char_buffer* c_buffer);
// Increments the current index in the buffer
void increment_index(char_buffer* c_buffer);
// Compares the PASSWORD_LENGTH latest characters to the current password
int check_password(char_buffer* c_buffer);
// Get the N latest chars entered
char* get_latest_chars_entered(char_buffer* c_buffer, unsigned char amount_of_chars, char* latest_chars);