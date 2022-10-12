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

void init_doors(void){
	for (int i = 0; i < NUMBER_DOORS; i++){
		u_door door = {
			.id = i + 1,
			.connected_switch = 1 << (4 * i),
			.is_closed_led_pin = 1 << ((4 * i) + 1),
			.local_alarm_led_pin = 1 << ((4 * i) + 2),
			.is_locked_led_pin = 1 << ((4 * i )+ 3),
			.status_central_alarm = 0,
			.status_local_alarm = 0,
			.local_alarm_time_threshold_s = DEFAULT_LOCAL_TIME_THRESHOLD,
			.global_alarm_time_thershold_s = DEFAULT_GLOBAL_TIME_THRESHOLD,
			.opened_door_timestamp_ms = 0,
		};
		doors[i] = door;
	} 
}

// Checks if a door has been opened and sets the timestamp of the opening and
// Turns on the is_closed_led_pin
void check_if_door_opened(int door_index){
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

// Start global alarm by sending CAN message to central unit.
void start_global_alarm( void ) {

}

void start_local_alarm( int door_index ) {
	GPIO_SetBits(GPIOD, doors[door_index].local_alarm_led_pin);
	doors[door_index].status_local_alarm = 1;
}

void door_update( void ) {
	for (int i = 0 ; i < NUMBER_DOORS ; i++) {
		check_if_door_opened(i);
		if (doors[i].door_is_open) {
			// When the time for the local alarm has passed start the local alarm.
			// After global_alarm_time_threshold has gone by after the local alarm started, start the global alarm.
			if ((doors[i].opened_door_timestamp_ms 
				+ doors[i].local_alarm_time_threshold_s
				+ doors[i].global_alarm_time_thershold_s * 1000) <= timer_ms) {
				start_global_alarm();
			}
			else if ((doors[i].opened_door_timestamp_ms + doors[i].local_alarm_time_threshold_s * 1000) <= timer_ms) {
				start_local_alarm(i);
			}
		}
	}
}

void main(void) {
	init_GPIO();
	init_doors();
	timer_init();

	while (1) {
		door_update();
	}
}

