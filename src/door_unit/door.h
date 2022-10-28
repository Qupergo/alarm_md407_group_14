#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "usart1.h"
#include "misc.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_exti.h"
#include <stdint.h>


#define NUMBER_DOORS 2
#define DEFAULT_LOCAL_TIME_THRESHOLD 4
#define DEFAULT_GLOBAL_TIME_THRESHOLD 8

// Struct for the door unit, includes addresses to all pins connected to the door, id for each door and
// thresholds for starting local and global alarms.
typedef struct DoorUnit {
	uint8_t id;
	uint16_t connected_switch;
	uint16_t is_closed_led_pin;
	uint16_t local_alarm_led_pin;
	uint16_t is_locked_led_pin;
	uint8_t status_central_alarm;
	uint8_t status_local_alarm;
	uint8_t door_is_open;
	uint8_t door_is_locked;
	uint32_t local_alarm_time_threshold_s;
	uint32_t global_alarm_time_threshold_s;
	uint32_t opened_door_timestamp_ms;
} u_door;

// Sets gpio pins correctly for doors
void init_GPIO( void );
// Initialises the door structs with initial values
void init_doors( void );
// Checks if a door has been opened
// If it has, then it records a timestamp for the event
// Which is later used in door_update to start alarms
void check_if_door_opened(int door_index);
// Stops a local alarm in a door
void stop_local_alarm(int door_index);
// Starts a local alarm in a door
void start_local_alarm(int door_index);
// Sets the global alarm status to 0
void stop_global_alarm();
// Sets the global alarm status to 1
void start_global_alarm();
// Used to lock a door, sets its status to locked
// And turns on a led to show that it is locked
void lock_door(int door_index);
// Unlocks a door, sets its locked status to 0
// And turns off the led to show it is unlocked if it is on
void unlock_door(int door_index);
// Checks if an alarm needs to be called by comparing the timestamp
// From check_if_door_opened to the alarm threshold times
void door_update( void );
