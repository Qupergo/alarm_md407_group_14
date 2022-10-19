/* 
This is meant purely as an example of how the CAN functions should be used.
The code in this file should be used in other place. This file should not be run.
*/
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_can.h"
#include "can.h"
#include "timer.h"

unsigned char SELF_TYPE = TYPE_CENTRAL_UNIT;
unsigned char num_sub_units = 2;
unsigned char self_id;

void main(void)
{
    // For all units
    timer_init();
    can_init(CAN1);
    rx_can_msg rx_msg;
    rt_info _rt_info;
    ls_info _ls_info;

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
    
    // For central unit
    u_info central_unit;
    central_unit.is_used = 1;
    central_unit.type = TYPE_CENTRAL_UNIT;
    central_unit.main_id = 0;
    central_unit.num_sub_units = 0;
    units[0] = central_unit;

    // For non central units
    // Send initial alive message to central unit
    tx_can_msg initial_alive;
    initial_alive.priority = 1;
    initial_alive.message_type = MSGID_NEW_ALIVE;
    initial_alive.content[0] = SELF_TYPE;
    initial_alive.content[1] = num_sub_units;
    initial_alive.reciever_id = 0;
    can_send_message(&_rt_info, &_ls_info, CAN1, initial_alive);
    unsigned char waiting_for_alive_response = 1;

    while (1) {
        can_update(&_rt_info, &_ls_info);
        if (can_receive_message(&_rt_info, &_ls_info, CAN1, &rx_msg)) {
            switch (rx_msg.message_type) {
                // For central unit
                case MSGID_NEW_ALIVE:
                    for (int i = 0; i < MAX_UNITS; i++) {
                        if (!units[i].is_used) {
                            units[i].is_used = 1;
                            units[i].main_id = i;
                            units[i].type = rx_msg.content[0];
                            units[i].num_sub_units = rx_msg.content[1];
                            
                            tx_can_msg tx_msg;
                            tx_msg.reciever_id = i;
                            tx_msg.message_type = MSGID_NEW_ALIVE_RESPONSE;
                            tx_msg.content[0] = rx_msg.content[0];
                            tx_msg.content[1] = i;
                            can_send_message(&_rt_info, &_ls_info, CAN1, tx_msg);
                        }
                    }
                    break;
                // For non central units
                case MSGID_NEW_ALIVE_RESPONSE:
                    if (waiting_for_alive_response) {
                        if (rx_msg.content[0] == SELF_TYPE) {
                            waiting_for_alive_response = 0;
                            self_id = rx_msg.content[1];
                            _rt_info.recieve_sequence_num[rx_msg.sender_id] = 0;
                            _rt_info.transmit_sequence_num[rx_msg.sender_id] = 1;
                        }
                    }
            }
        }
    }
}

