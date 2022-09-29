#include <stdbool.h>

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_usart.h"

#include "can.h"
#include "usart.h"

/* 
    Connected devices should be saved in a pointer struct containing their type (Distance sensor, Vibration sensor, Door)
    and their unique ID decided by the central unit. They should also each have a tiemr
*/

// CAN message ID's
#define CAN_ID_STATUS_REQUEST 0x100
#define CAN_ID_STATUS_UPDATE 0x200
#define CAN_ID_ALARM_TRIGGER 0x300
#define CAN_ID_ALARM_RESET 0x400
#define CAN_SELF_ID 0x10

// CAN message lengths
#define CAN_LENGTH_STATUS_REQUEST 0x01
#define CAN_LENGTH_STATUS_UPDATE 0x01
#define CAN_LENGTH_ALARM_TRIGGER 0x01
#define CAN_LENGTH_ALARM_RESET 0x01

// CAN message structs
CanTxMsg status_request = {
    .StdId = CAN_ID_STATUS_REQUEST,
    .DLC = CAN_LENGTH_STATUS_REQUEST,
    .RTR = CAN_RTR_Data,
    .IDE = CAN_Id_Extended
};

CanTxMsg alarm_trigger = {
    .StdId = CAN_ID_ALARM_TRIGGER,
    .DLC = CAN_LENGTH_ALARM_TRIGGER,
    .RTR = CAN_RTR_Data,
    .IDE = CAN_Id_Extended
};

CanTxMsg alarm_reset = {
    .StdId = CAN_ID_ALARM_RESET,
    .DLC = CAN_LENGTH_ALARM_RESET,
    .RTR = CAN_RTR_Data,
    .IDE = CAN_Id_Extended
};

// Connected devices struct
typedef struct {
    char id;
    char type;
    bool alarm;
} device;

// Connected devices array
device devices[10];

// Number of connected devices
int device_count = 0;

// Alarm status
bool alarm_status = false;

// Alarm trigger
void trigger_alarm(){
    // Send alarm trigger message to all connected devices
    for(int i = 0; i < device_count; i++){
        can_send_message(CAN1, &alarm_trigger);
    }
    // Set alarm status to true
    alarm_status = true;
}

// Alarm reset
void reset_alarm(){
    // Send alarm reset message to all connected devices
    for(int i = 0; i < device_count; i++){
        can_send_message(CAN1, &alarm_reset);
    }
    // Set alarm status to false
    alarm_status = false;
}

// Add device to connected devices array
void add_device(char type){
    // Add device to array
    devices[device_count].id = device_count;
    devices[device_count].type = type;
    devices[device_count].alarm = false;
    device_count++;
}

// Remove device from connected devices array
void remove_device(char id){
    // Find device in array
    for(int i = 0; i < device_count; i++){
        if(devices[i].id == id){
            // Remove device from array
            for(int j = i; j < device_count; j++){
                devices[j] = devices[j+1];
            }
            device_count--;
            return;
        }
    }
}

// Check if device is connected
bool is_connected(char id){
    // Find device in array
    for(int i = 0; i < device_count; i++){
        if(devices[i].id == id){
            return true;
        }
    }
    return false;
}

bool valid_message(char sender_id, char reciever_id, char message_type, char device_type) {
    // TODO: This should be in can.c
    // Should check that the sender is not yourself, that the reciever is correct
    // That the message type makes sense for the the device type
    return true;
}

void main(void) {
    // Their id is chosen by the central unit once they connect and send a message which the central unit picks up
    // The central unit should send a request to all connected devices for a status update every 30 milliseconds
    // If one does not respond within 60 milliseconds an alarm should be triggered
    // device_type
    // 0 = Central unit
    // 1 = Distance sensor
    // 2 = Vibration sensor
    // 3 = Door

    // Initialize CAN
    can_init(CAN1);

    // Initialize CAN message structs
    // First field of data should be the device type
    status_request.Data[0] = 0x0;
    alarm_trigger.Data[0] = 0x0;
    alarm_reset.Data[0] = 0x0;

    // Initialize connected devices array
    for(int i = 0; i < 10; i++){
        devices[i].id = i;
        devices[i].type = 0;
        devices[i].alarm = false;
    }

    // Initialize variables
    char id;
    char type;
    char data;
    int counter = 0;
    int alarm_counter = 0;
    bool alarm_triggered = false;

    CanRxMsg incoming_msg;

    // Main loop
    while(1){
        // Check if there are any messages in the CAN queue
        if(can_receive_message(CAN1, &incoming_msg)){
            // Get id and type from message
            id = incoming_msg.StdId;
            char sender_id = id & 0x00F;
            char reciever_id = (id & 0x0F0) >> 4;
            char message_type = (id & 0xF00) >> 8;

            char device_type = incoming_msg.Data[0];

            // Check if device is connected
            if (sender_id == 0) {
                add_device(device_type);
            }
            else if (valid_message(sender_id, reciever_id, message_type, device_type)) {
                // Handle message depending on type
                // message_type
                // 1 = Status Request
                // 2 = Alarm Trigger
                // 3 = Alarm reset
                // TODO: More types
            }
        }
        // Check if counter is at 30 milliseconds
        // TODO: Have device specific counters that actually count milliseconds
        if(counter == 30) {
            // Reset counter
            counter = 0;
            // Send status request to all connected devices
            for(int i = 0; i < device_count; i++){
                // Reset id field
                status_request.StdId &= CAN_ID_STATUS_REQUEST;
                // Set recieving and sending device ids
                status_request.StdId |= devices[i].id | CAN_SELF_ID;
                can_send_message(CAN1, &status_request);
            }
        }
        // Check if counter is at 60 milliseconds
        if(counter == 60){
            // Reset counter
            counter = 0;
            // Check if alarm should be triggered
            if(alarm_counter > 0){
                trigger_alarm();
            }
        }
        // Increment counter
        counter++;
    }
}