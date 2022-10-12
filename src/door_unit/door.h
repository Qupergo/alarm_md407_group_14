
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "usart.h"
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
	uint32_t local_alarm_time_threshold_s;
	uint32_t global_alarm_time_thershold_s;
	uint32_t opened_door_timestamp_ms;
} u_door;
