#include "can.h"
#include "string.h"
#include "keypad.h"
#include "timer.h"
#include "usart1.h"


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
	print_line("\nSelect one option from the following");
	print_line("1 Set a new password");
	print_line("2 Enable door alarm");
	print_line("3 Disable door alarm"); 
	print_line("4 Set a time threshold for door");
	print_line("5 Calibrate distance sensor");
	print_line("6 Adjust sensitivity for distance sensor");
	print_line("7 Restart alarm");
	print_line("0 To exit");
	print("Option: ");
}

void reset_self_unit( void ) {
	self_id = 0;

}

int unit_id_exists_with_type(int id, enum unit_type_id unit_type) {
	for (int i = 0; i < MAX_UNITS; i++)	{
		if (units[i].is_used) {
			if (units[i].main_id == id && units[i].type == unit_type) {
				return 1;
			}
		}
	}
	return 0;
}

u_info* get_units_of_type(enum unit_type_id unit_type) {
	u_info matching_units[MAX_UNITS];

	for (int i = 0; i < MAX_UNITS; i++) {
		if (units[i].type == unit_type) {
			matching_units[i] = units[i];
		}
	}
}

u_info get_unit_by_id(int id) {
	for (int i = 0; i < MAX_UNITS; i++) {
		if (units[i].main_id == id) {
			return units[i];
		}
	}
}

void print_all_doors() {
	print_line("Currently available door units:");
	for (int i = 0; i < MAX_UNITS; i++) {
		if (units[i].is_used && units[i].type == TYPE_DOOR_UNIT) {
			print("Door unit ");
			print_int(units[i].main_id);
			print_line("has the following doors connected:");
			for (int j = 0; j < units[i].num_sub_units; j++) {
				print("Door connected to GPIO pin ");
				print_int(j);
				print("\n");
			}
		}
	}
}

// Executes a specified option, function should be called every time a new character is recieved
// This is checked with buffer_update which returns 1 whenever a new character is entered
// Returns if the command is done executing or not.
int execute_option(char_buffer* c_buffer, rt_info* _rt_info, int option, int* chars_entered) {
	int done_with_option = 0;
	switch (option) {
		case 1: // Set a new password
			if (*chars_entered == 0) {
				print("\nEnter the new password (");
				print_int(PASSWORD_LENGTH);
				print(" characters): ");
			}
			else if (*chars_entered == PASSWORD_LENGTH) {
				char new_password[PASSWORD_LENGTH];
				get_latest_chars_entered(c_buffer, PASSWORD_LENGTH, new_password);
				for(int i = 0; i < PASSWORD_LENGTH; i++) {
					c_buffer->current_password[i] = new_password[i];
				}
				done_with_option = 1;
				print("\nSuccessfully updated password to ");
				for (int i = 0; i < PASSWORD_LENGTH; i++) {
					print_int(c_buffer->current_password[i]);
				}
				print("\n");
			}
			break;
		// Only difference is type of message so do these at same time
		case 2: // Enable door alarm
		case 3: // Disable door alarm
			if (*chars_entered == 0) {
				print_all_doors();
				if (option == 2) {
					print_line("Enable door alarm for which door?");
				}
				else if (option == 3) {
					print_line("Disable door alarm for which door?");	
				}
				print("Enter door unit id (or 0 to exit): ");
			}
			else if (*chars_entered == 1) {
				print("\n");
				print("Enter GPIO pin of connected door (or 0 to exit): ");
			}
			else if (*chars_entered == 2) {
				char door_values[2];
				get_latest_chars_entered(c_buffer, 2, door_values);
				char door_id = door_values[1];
				char door_unit_id = door_values[0];
				
				if (door_unit_id == 0 || door_id == 0) {
					print_line("0 Entered, exiting...");

					done_with_option = 1;
					break;
				}
				if (!unit_id_exists_with_type(door_unit_id, TYPE_DOOR_UNIT)) {
					print("A door unit with the id ");
					print_int(door_unit_id);
					print_line(" does not exist.\nPlease try again");
					*chars_entered = 0;
					break;
				}
				else if (door_unit_id > get_unit_by_id(door_unit_id).num_sub_units) {
					print("A door with id ");
					print_int(door_id);
					print(" connected to door unit with id ");
					print_int(door_unit_id);
					print_line(" does not exist.\nPlease try again");
					*chars_entered = 1;
					break;
				}

				tx_can_msg msg_alarm;
				if (option == 2) {
					msg_alarm.message_type = MSGID_START_ALARM;
				}
				else if (option == 3) {
					msg_alarm.message_type = MSGID_STOP_ALARM;
				}
				msg_alarm.reciever_id = door_unit_id; // Set to a correct id
				msg_alarm.priority = 0; // priority is max
				msg_alarm.content[0] = door_id;
				// send usart msg with info ?
				done_with_option = 1;
			}
			break;
		case 4:  // Set new time threshold
			if (*chars_entered == 0) {
				print_all_doors();
				print_line("Set new time threshold for which door?");
				print("Enter door unit id (or 0 to exit): ");
			}
			else if (*chars_entered == 1) {
				print("\n");
				print("Enter new time threshold value in seconds (when value is entered, write 'D'): ");
			}
			if (*chars_entered > 2) {
				char latest_char[1];
				get_latest_chars_entered(c_buffer, *chars_entered, latest_char);
				if (latest_char == 0xD) {
					char door_values[*chars_entered];
					get_latest_chars_entered(c_buffer, *chars_entered, door_values);
					char door_unit_id = door_values[0];

					if (door_unit_id == 0) {
						print_line("0 Entered, time threshold change");
						done_with_option = 1;
						break;
					}

					if (!unit_id_exists_with_type(door_unit_id, TYPE_DOOR_UNIT)) {
						print("A door unit with the id ");
						print_int(door_unit_id);
						print_line(" does not exist.\nPlease try again");
						*chars_entered = 0;
						break;
					}

					tx_can_msg set_new_threshold;
					set_new_threshold.message_type = MSGID_SET_DOOR_ALARM_TIME_THRESHOLD;
					set_new_threshold.reciever_id = TYPE_DOOR_UNIT;
					set_new_threshold.priority = 1;
					int i = 1; // Don't include the door id at pos 0.
					while (set_new_threshold.content[i] <= 9) {
						if (i >= CONTENT_LENGTH) {
							break;
						}
						set_new_threshold.content[i] = door_values[i];
						i++;
					}
					done_with_option = 1;
				}
			}
			break;
		case 5: // Calibrate distance alarm
			if (*chars_entered == 0) {
				print_line("Currently connected sensor units:");
				for (int i = 0; i < MAX_UNITS; i++) {
					if (units[i].type == TYPE_SENSOR_UNIT) {
						print("Sensor unit with ID: ");
						print_int(units[i].main_id);
						print("\n");
					}
				}
				print_line("Recalibrate which sensor unit?");
				print("Enter sensor unit id (or 0 to exit): ");
			}
			else if (*chars_entered == 1) {
				print("\n");
				char* sensor_id;
				get_latest_chars_entered(c_buffer, 1, sensor_id);
				if (sensor_id == 0) {
					print_line("0 Entered, exiting calibration");
					done_with_option = 1;
					break;
				}

				if (!unit_id_exists_with_type(sensor_id, TYPE_SENSOR_UNIT)) {
					print("A sensor unit with the id ");
					print_int(sensor_id);
					print_line(" does not exist.\nPlease try again");
					*chars_entered = 0;
					break;
				}

				tx_can_msg msg_recalibrate = {
					.message_type = MSGID_RECALIBERATE,
					.reciever_id = sensor_id,
				};
				can_send_message(_rt_info, CAN1, msg_recalibrate);
				done_with_option = 1;
			}
			break;
		case 6: // Set sensitivity for distance sensor
			if (*chars_entered == 0) {
				print_line("Currently connected sensor units:");
				for (int i = 0; i < MAX_UNITS; i++) {
					if (units[i].type == TYPE_SENSOR_UNIT) {
						print("Sensor unit with ID: ");
						print_int(units[i].main_id);
						print("\n");
					}
				}
				print_line("Set sensitivty for which sensor unit's distance sensor?");
				print("Enter sensor unit id: ");
			}
			else if (*chars_entered == 1) {
				print("\n");
				print_line("What should the sensitivity be?");
				print("Distance threshold (in cm) (or 0 to exit): ");
			}
			else if (*chars_entered > 2) {
				char latest_char[1];
				get_latest_chars_entered(c_buffer, *chars_entered, latest_char);
				if (latest_char == 0xD) {
					char sensor_values[*chars_entered];
					get_latest_chars_entered(c_buffer, chars_entered, sensor_values);
					char sensor_id = sensor_values[0];
					if (sensor_id == 0) {
						print_line("0 Entered, exiting sensitivity change");
						done_with_option = 1;
						break;
					}

					if (!unit_id_exists_with_type(sensor_id, TYPE_SENSOR_UNIT)) {
						print("A sensor unit with the id ");
						print_int(sensor_id);
						print_line(" does not exist.\nPlease try again");
						*chars_entered = 0;
						break;
					}

					tx_can_msg msg_update_threshold = {
						.message_type = MSGID_SENSOR_DISTANCE_THRESHOLD,
						.reciever_id = sensor_id, // Sensor id
					};
					int i = 1; // Don't include the sensor id at pos 0.
					while (msg_update_threshold.content[i] <= 9) {
						if (i >= CONTENT_LENGTH) {
							break;
						}
						msg_update_threshold.content[i] = sensor_values[i];
						i++;
					}

					can_send_message(_rt_info, CAN1, msg_update_threshold);
					done_with_option = 1;
				}
			}
			break;
		case 7:
			print_line("Reseting all units...");
			reset_self_unit();
			// Start at 1 to avoid sending to central unit
			for (int i = 1; i < MAX_UNITS; i++) {
				if (units[i].is_used) {
					tx_can_msg msg_reset_unit = {
						.priority = 1,
						.message_type = MSGID_RESET_UNIT,
						.reciever_id = units[i].main_id,
					};

					can_send_message(_rt_info, CAN1, msg_reset_unit);

					if (units[i].type == TYPE_DOOR_UNIT) {
						print("Door unit with id ");
					}
					else if (units[i].type == TYPE_SENSOR_UNIT) {
						print("Sensor unit with id ");
					}
					print_int(units[i].main_id);
					print_line(" has been sent a reset message");
				}
			}
			print_line("Reset is finished.");
			done_with_option = 1;
			break;
		case 0:
			print_line("Option 0 entered, exiting menu...");
			done_with_option = 1;
			break;
		default:
			print("Option:");
			print_int(option);
			print_line(" is invalid. Please enter the password again\nThen enter a valid option.");
			done_with_option = 1;
			break;
	}
	return done_with_option;
}

void main(void) {
	char_buffer c_buffer;
	keypad_init(&c_buffer);
	timer_init();
	USART1_Init();
	
	rx_can_msg rx_msg;
    rt_info _rt_info;
    ls_info _ls_info;
	u_info central_unit;
	
	can_init(&_rt_info, &_ls_info, CAN1, 1);

    central_unit.is_used = 1;
    central_unit.type = TYPE_CENTRAL_UNIT;
    central_unit.main_id = 0;
    central_unit.num_sub_units = 0;
    units[0] = central_unit;

	int choosing_menu_option = 0;
	char option;
	int executing_menu_option = 0;
	int chars_entered_for_option[1];
	*chars_entered_for_option = 0;

	new_buffer_char = 0xFF;
	new_char_available = 0;

	print("\nEnter the password to open the menu (Default password is '1111'): ");

	while (1) {
		if (buffer_update(&c_buffer)) {
			if (choosing_menu_option) {
				get_latest_chars_entered(&c_buffer, 1, &option);
				choosing_menu_option = 0;
				executing_menu_option = 1;
				execute_option(&c_buffer, &_rt_info, option, chars_entered_for_option);
			}
			else if (executing_menu_option) {
				*chars_entered_for_option += 1;
				if (execute_option(&c_buffer, &_rt_info, option, chars_entered_for_option)) {
					executing_menu_option = 0;
					*chars_entered_for_option = 0;
					print("\nEnter the password to open the menu\nEnter password: ");

				}
			}
			else if (check_password(&c_buffer)) {
				reset_buffer();
				print_menu_options();
				// Set choosing menu option to 1 so that the next time the keypad updates
				choosing_menu_option = 1;
			}
			else {
				
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
