__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_can.h"
#include "can.h"

unsigned char SELF_TYPE = TYPE_CENTRAL_UNIT;
unsigned char num_sub_units = 0;
unsigned char self_id;

void trigger_alarm() {
    //TODO
}

void main(void)
{
    keypad_init();
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

    u_info central_unit;
    central_unit.is_used = 1;
    central_unit.type = TYPE_CENTRAL_UNIT;
    central_unit.main_id = 0;
    central_unit.num_sub_units = 0;
    units[0] = central_unit;

    char password_entered = 0;

    while (1) {
        keypad_update();
        if (password_entered) {
            // Turn off alarm if it is on   
        }
        else if (check_password()) {
            password_entered = 1;
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
                            can_send_message(&_rt_info, &_ls_info, CAN1, alive_response);
                        }
                    }
                    break;
            }
        }
    }
}

