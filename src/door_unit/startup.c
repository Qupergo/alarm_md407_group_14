__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

#include "door.h"
#include "timer.h"
#include "can.h"

#define DEBUG 1

unsigned char SELF_TYPE = TYPE_DOOR_UNIT;
unsigned char num_sub_units = NUMBER_DOORS;
unsigned char self_id;
int send_alarm = 0;

u_door doors[NUMBER_DOORS];

void init_GPIO( void ) {
	GPIO_InitTypeDef GPIOConfig;
	GPIO_StructInit(&GPIOConfig);
	
	GPIOConfig.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4;
	GPIOConfig.GPIO_Mode = GPIO_Mode_IN;
	GPIOConfig.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOD,&GPIOConfig);
	
	GPIOConfig.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIOConfig.GPIO_Mode = GPIO_Mode_OUT;
	GPIOConfig.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD,&GPIOConfig);	
}

void init_doors( void ) {
	for (int i = 0; i < NUMBER_DOORS; i++){
		u_door door = {
			.id = i + 1,
			.connected_switch = 1 << (4 * i),
			.is_closed_led_pin = 1 << ((4 * i) + 1),
			.local_alarm_led_pin = 1 << ((4 * i) + 2),
			.is_locked_led_pin = 1 << ((4 * i )+ 3),
			.status_central_alarm = 0,
			.status_local_alarm = 0,
			.door_is_locked = 0,
			.local_alarm_time_threshold_s = DEFAULT_LOCAL_TIME_THRESHOLD,
			.global_alarm_time_threshold_s = DEFAULT_GLOBAL_TIME_THRESHOLD,
			.opened_door_timestamp_ms = 0,
		};
		doors[i] = door;
	} 
}

// Checks if a door has been opened and sets the timestamp of the opening and
// Turns on the is_closed_led_pin
void check_if_door_opened(int door_index) {
	if (!(GPIO_ReadInputDataBit(GPIOD, doors[door_index].connected_switch))) {
		if (!doors[door_index].door_is_open) {
			doors[door_index].door_is_open = 1;
			doors[door_index].opened_door_timestamp_ms = timer_ms;
			GPIO_ResetBits(GPIOD, doors[door_index].is_closed_led_pin);
		}
	}
	else {
		doors[door_index].status_local_alarm = 0;
		doors[door_index].door_is_open = 0;
		doors[door_index].opened_door_timestamp_ms = 0;
		GPIO_SetBits(GPIOD, doors[door_index].is_closed_led_pin);
		GPIO_ResetBits(GPIOD, doors[door_index].local_alarm_led_pin);
	}	
}

void stop_local_alarm(int door_index) {
	GPIO_SetBits(GPIOD, doors[door_index].is_closed_led_pin);
	GPIO_ResetBits(GPIOD, doors[door_index].local_alarm_led_pin);
	doors[door_index].status_local_alarm = 0;
}

void start_local_alarm(int door_index) {
	GPIO_SetBits(GPIOD, doors[door_index].local_alarm_led_pin);
	doors[door_index].status_local_alarm = 1;
}

void stop_global_alarm(int index) {
	doors[index].status_central_alarm = 0;
	send_alarm = 0;
}

void start_global_alarm(int index) {
	doors[index].status_central_alarm = 1;
	send_alarm = 1;
}

void lock_door(int door_index) {
	GPIO_SetBits(GPIOD, doors[door_index].is_locked_led_pin);
	doors[door_index].door_is_locked = 1;
}

void unlock_door(int door_index) {
		GPIO_ResetBits(GPIOD, doors[door_index].is_locked_led_pin);
		doors[door_index].door_is_locked = 0;
}

void door_update( void ) {
	for (int i = 0 ; i < NUMBER_DOORS ; i++) {
		check_if_door_opened(i);
		if (doors[i].door_is_open) {
			// When the time for the local alarm has passed start the local alarm.
			// After global_alarm_time_threshold has gone by after the local alarm started, start the global alarm.
			if ((doors[i].opened_door_timestamp_ms 
				+ doors[i].local_alarm_time_threshold_s * 1000
				+ doors[i].global_alarm_time_threshold_s * 1000) <= timer_ms) {
					print_line("global alarm threshold reached");
					doors[i].status_central_alarm = 1;
					send_alarm = 1;
			}
			else if ((doors[i].opened_door_timestamp_ms + doors[i].local_alarm_time_threshold_s * 1000) <= timer_ms) {
				start_local_alarm(i);
				print("timestamp for door timed out, starting local alarm on door ");
				print_int(i);
				print("\n");
			}
		}
	}
}

void main(void) {
	init_GPIO();
	init_doors();
    timer_init();
    rx_can_msg rx_msg;
    rt_info _rt_info;
    ls_info _ls_info;
    can_init(&_rt_info, &_ls_info, CAN1, 0);
  
    for (int i = 0; i < MAX_UNITS; i++) {
        _rt_info.transmit_sequence_num[i] = 1;
        _rt_info.recieve_sequence_num[i] = 0;
        _ls_info.is_connected[i] = 0;
        _ls_info.latest_self_lifesign_timestamp = 0;
        _ls_info.recieved_lifesigns[i] = 0;
    }
    for (int i = 0; i < MAX_RT_FRAMES; i++) {
        _rt_info.rt_frames[i].is_used = 0;
    }

	// Send initial alive message to central unit
    tx_can_msg initial_alive;
    initial_alive.priority = 1;
    initial_alive.message_type = MSGID_NEW_ALIVE;
    initial_alive.content[0] = SELF_TYPE;
    initial_alive.content[1] = num_sub_units;
    initial_alive.reciever_id = 0;
	initial_alive.sequence_n = _rt_info.transmit_sequence_num[0];
    can_send_message(&_rt_info, CAN1, initial_alive);
    unsigned char waiting_for_alive_response = 1;

	while (1) {
		door_update();
		if (send_alarm) {
			for (int i = 0; i < NUMBER_DOORS; i++) {
				if (doors[i].status_central_alarm) {
					tx_can_msg msg_alarm = {
						.priority = 0,
						.message_type = MSGID_START_ALARM,
						.reciever_id = 0,
						.content = doors[i].id,
						.sequence_n = _rt_info.transmit_sequence_num[i],
					};
					can_send_message(&_rt_info, CAN1, msg_alarm);
				}
			}
			send_alarm = 0; // Reset send alarm
		}


        if (!waiting_for_alive_response) {
            can_update(&_rt_info, &_ls_info);
        }
		if (can_receive_message(&_rt_info, &_ls_info, CAN1, &rx_msg)) {
            switch (rx_msg.message_type) {
                case MSGID_NEW_ALIVE_RESPONSE:
                    if (waiting_for_alive_response) {
						print_line("Door unit recieved alive response");
                        if (rx_msg.content[0] == SELF_TYPE) {
                            waiting_for_alive_response = 0;
                            self_id = rx_msg.content[1];
							can_init_filter(self_id);
                            _rt_info.recieve_sequence_num[rx_msg.sender_id] = 0;
                            _rt_info.transmit_sequence_num[rx_msg.sender_id] = 1;
                        }
                    }
					break;
				case MSGID_UNLOCK_DOOR:
					char door_id = rx_msg.content[0]
					unlock_door(door_id);
					break;
				case MSGID_LOCK_DOOR:
					char door_id = rx_msg.content[0]
					lock_door(door_id);
					break;

				case MSGID_SET_DOOR_ALARM_TIME_THRESHOLD:;
					char door_id = rx_msg.content[0]; 
					char new_threshold = rx_msg.content[1];
					char change_local_alarm = rx_msg.content[2];
					if (change_local_alarm) {
						doors[door_id - 1].local_alarm_time_threshold_s = new_threshold;
						if(DEBUG)
							print_line("changed local alarm thresh");
					}
					else {
						doors[door_id - 1].global_alarm_time_threshold_s = new_threshold;
						if(DEBUG)
							print("changed global alarm thresh");
					}
					break;
					
				case MSGID_START_ALARM:;
					char door_id = rx_msg.content[0]
					start_global_alarm(door_id);
					break;
				case MSGID_STOP_ALARM:;
					char door_id = rx_msg.content[0]
					stop_global_alarm(door_id);
					break;
			}
		}
	}
}

