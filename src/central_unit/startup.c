#include "can.h"
#include "string.h"
#include "keypad.h"
#include "timer.h"


unsigned char SELF_TYPE = TYPE_CENTRAL_UNIT;
unsigned char num_sub_units = 0;
unsigned char self_id;


__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )

{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}


void print_menu_options( void ) {
	print_line("Select one option from the following");
	print_line("1 Set a new password");
	print_line("2 View alarm status for all units");
	print_line("3 Enable door alarm");
	print_line("4 Disable door alarm"); 
	print_line("5 Set a time threshold for door, start 0 for global and 1 for local threshold, enter door ID, followed by threshold value");
	print_line("6 Calibrate distance sensor");
	print_line("7 Adjust sensitivity for distance sensor");
	print_line("8 Restart alarm");
}

u_info* get_unit_of_type(enum unit_type_id unit_type) {
	u_info matching_units[MAX_UNITS];

	for (int i = 0; i < MAX_UNITS; i++) {
		if (units[i].type == unit_type) {
			matching_units[i] = units[i];
		}
	}
}

// Returns the amount of chars neeeded to execute the command.
int execute_option(rt_info* _rt_info, char_buffer* c_buffer, int option, int chars_entered) {
	switch (option) {
		case 1: // Set a new password
			if (chars_entered == PASSWORD_LENGTH) {
				for(int i = 0; i < PASSWORD_LENGTH; i++) {
					c_buffer->current_password[i] = get_latest_chars_entered(c_buffer, PASSWORD_LENGTH)[i];
				}
				return 1;
			}
			break;
		case 2: // View alarm status
			for(int i = 1; i < MAX_UNITS; i++){ // antigen visa info eller skicka can.msg med förfråga om status
				continue;
			}
			return 1;
		case 3: // Enable door alarm
			if (chars_entered == 1) {
				char door_id = *get_latest_chars_entered(c_buffer,1);
				tx_can_msg alarm_message_on;
				alarm_message_on.message_type = MSGID_START_ALARM;
				alarm_message_on.reciever_id = TYPE_DOOR_UNIT; // Set to a correct id
				alarm_message_on.priority = 0; // priority is max
				alarm_message_on.content[0] = door_id;
				// send usart msg with info ?
				return 1;
			}
			break;
		case 4: // Disable door alarm
			if (chars_entered == 1) {
				char door_id = *get_latest_chars_entered(c_buffer, 1);
				tx_can_msg alarm_message_off;
				alarm_message_off.message_type = MSGID_STOP_ALARM;
				alarm_message_off.reciever_id = TYPE_DOOR_UNIT; // Set to a correct id
				alarm_message_off.priority = 0; // priority is max
				alarm_message_off.content[0] = door_id;
				return 1;
			}
			break;
		case 5:  // Set new time threshold
			if (chars_entered > 1) {
				char latest_char = *get_latest_chars_entered(c_buffer, 1);
				if (latest_char == 0xD) {
					char* new_threshold = get_latest_chars_entered(c_buffer, chars_entered);
					tx_can_msg set_new_threshold;
					set_new_threshold.message_type = MSGID_SET_DOOR_ALARM_TIME_THRESHOLD;
					set_new_threshold.reciever_id = TYPE_DOOR_UNIT;
					set_new_threshold.priority = 1;
					int i = 0;
					while (*new_threshold != 0xD) {
						set_new_threshold.content[i] = new_threshold[i];
						i++;
					}
					return 1;
				}
			}
			break;
		case 6: // caliberate distance alarm
			for (int i = 0; i < MAX_UNITS; i++) {
				if (units[i].type == TYPE_SENSOR_UNIT) {
					tx_can_msg msg_recalibrate = {
						.message_type = MSGID_RECALIBERATE,
						.reciever_id = units[i].main_id,
					};
					can_send_message(_rt_info, CAN1, msg_recalibrate);
				}
			}
			return 1;
		case 7: // set sensitivity for distance sensor
			if (chars_entered == 1) {
				unsigned char sensitivity_value = *get_latest_chars_entered(c_buffer, 1);

				for (int i = 0; i < MAX_UNITS; i++) {
					if (units[i].type == TYPE_SENSOR_UNIT) {
						tx_can_msg reset_sensitivity = {
							.message_type = MSGID_SENSOR_DISTANCE_THRESHOLD,
							.reciever_id = units[i].main_id,
							.content[0] = sensitivity_value
						};
						can_send_message(_rt_info, CAN1, reset_sensitivity);
					}
				}
				return 1;
			}
			break;
		case 8:
			// Start at 1 to avoid sending to central unit
			for (int i = 1; i < MAX_UNITS; i++) {
				if (units[i].is_used) {
					tx_can_msg msg_reset_unit = {
						.priority = 1,
						.message_type = MSGID_RESET_UNIT,
						.reciever_id = units[i].main_id,
					};
					can_send_message(_rt_info, CAN1, msg_reset_unit);
				}
			}
			
			return 1;
	}
}

void main(void)
{
	keypad_init();
	rx_can_msg rx_msg;
    rt_info _rt_info;
    ls_info _ls_info;
	u_info central_unit;
	char_buffer c_buffer = {
		.current_password = {'1', '1', '1', '1'},
	};
	
	can_init(&_rt_info, &_ls_info, CAN1, 1);

    central_unit.is_used = 1;
    central_unit.type = TYPE_CENTRAL_UNIT;
    central_unit.main_id = 0;
    central_unit.num_sub_units = 0;
    units[0] = central_unit;

	int choosing_menu_option = 0;
	char option;
	int executing_menu_option = 0;
	int chars_entered_for_option = 0;

	while (1) {
		if (keypad_update(&c_buffer)) {
			if (choosing_menu_option) {
				option = *get_latest_chars_entered(&c_buffer, 1);
				choosing_menu_option = 0;
				executing_menu_option = 1;
			}
			else if (executing_menu_option) {
				chars_entered_for_option++;
				if (execute_option(&_rt_info, &c_buffer, option, chars_entered_for_option)) {
					executing_menu_option = 0;
					chars_entered_for_option = 0;
				}
			}
			else if (check_password(&c_buffer)) {
				reset_buffer();
				print_menu_options();
				choosing_menu_option = 1;
			}
		}

        can_update(&_rt_info, &_ls_info);
        if (can_receive_message(&_rt_info, &_ls_info, CAN1, &rx_msg)) {
            switch (rx_msg.message_type) {
                case MSGID_LIFESIGN:
                    _ls_info.recieved_lifesigns[rx_msg.sender_id] = timer_ms;
                    _ls_info.is_connected[rx_msg.sender_id] = 1;
                    break;

                case MSGID_NEW_ALIVE:
                    for (int i = 0; i < MAX_UNITS; i++) {
                        if (!units[i].is_used) {
                            units[i].is_used = 1;
                            units[i].main_id = i;
                            units[i].type = rx_msg.content[0];
                            units[i].num_sub_units = rx_msg.content[1];
                            
                            tx_can_msg alive_response;
                            alive_response.reciever_id = i;
                            alive_response.message_type = MSGID_NEW_ALIVE_RESPONSE;
                            alive_response.content[0] = rx_msg.content[0];
                            alive_response.content[1] = i;
                            can_send_message(&_rt_info, CAN1, alive_response);
                        }
                    }
                    break;
					
				case MSGID_START_ALARM: ;
					unsigned char alarm_sender = rx_msg.sender_id;
					unsigned char time_passed_since_alarm = timer_ms; // maybe as global variable ?
					print_line("ALARM");	 
			}
        }
    }
}
