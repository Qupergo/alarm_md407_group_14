#include <stdint.h>
#include "stm32f4xx_can.h"

#define MAX_UNITS 10
#define MAX_RT_FRAMES 30
#define ACK_TIMEOUT_MS 1000
#define LIFESIGN_FREQUENCY_MS 500
#define IS_CENTRAL_UNIT 1


// All possible message types and their corresponding ids
// Lower number is higher priority
enum message_id {
    START_ALARM_TYPE_ID,
    STOP_ALARM_TYPE_ID,
    NEW_ALIVE_TYPE_ID,
    NEW_ALIVE_RESPONSE_TYPE_ID,
    LIFESIGN_TYPE_ID,
    ACK_TYPE_ID,
    PING_TYPE_ID,
    PONG_TYPE_ID,
    NUM_MESSAGE_TYPES,
};

// All possible unit types and their corresponding ids
enum unit_type_id {
    CENTRAL_UNIT_TYPE_ID,
    DOOR_UNIT_TYPE_ID,
    SENSOR_UNIT_TYPE_ID,
};

// Recieved messages through CAN
typedef struct rx_can_message {
    unsigned char message_type;
    unsigned char sender_id;
    unsigned char content[6];
} rx_can_msg;

// Transmitted messages through CAN
typedef struct tx_can_message {
    unsigned char priority;
    unsigned char message_type;
    unsigned char reciever_id;
    unsigned char content[6];
} tx_can_msg;

// Saved frames for eventual retransmit
typedef struct retransmission_frame {
    unsigned char is_used;
    CAN_TypeDef* CANx;
    CanTxMsg msg;
    unsigned long int send_timestamp;
    unsigned short sequence_n;
} rt_frame;

// Information stored in the central unit for each unit connected to it.
typedef struct unit_info {
    unsigned char is_used;
    unsigned char main_id;
    unsigned char type;
    unsigned char num_sub_units;
} u_info;

u_info units[MAX_UNITS];

// The information needed in each unit for retransmits to function.
typedef struct retransmission_info {
    unsigned short transmit_sequence_num[MAX_UNITS];
    unsigned short recieve_sequence_num[MAX_UNITS];
    rt_frame rt_frames[MAX_RT_FRAMES];
} rt_info;

// The lifesign information.
typedef struct lifesign_info {
    unsigned int recieved_lifesigns[MAX_UNITS];
    unsigned char is_connected[MAX_UNITS];
    unsigned int latest_self_lifesign_timestamp;
} ls_info;

// Configures selected CAN interface for incoming messages
void can_init(CAN_TypeDef * CANx);
// Gets a pending message from a specified can peripheral.
// The message is then decoded and acks are sent as required.
// Returns true if a message was read, false otherwise.
int can_receive_message(rt_info* _rt_info, ls_info* _ls_info, CAN_TypeDef* CANx, rx_can_msg* rx_msg);
// Send a message over CAN and save the message in the retransmission buffer
void can_send_message(rt_info* _rt_info, ls_info* _ls_info, CAN_TypeDef* CANx, tx_can_msg tx_msg);
// Checks if timeouts have been reached for retransmissions and lifesigns and takes action accordingly.
// Should run in the main loop of each startup.c script.
void can_update(rt_info* _rt_info, ls_info* _ls_info);