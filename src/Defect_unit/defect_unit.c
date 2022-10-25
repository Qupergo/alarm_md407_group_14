#include "defect_unit.h"

/*
void imitation_attack(void){
	int data[6];
	can_receive_message(&_rt_info, &_ls_info, CAN1, &_rx_msg);
	unsigned short sequence_n = _rx_msg.content[6] << 8;
    sequence_n |= _rx_msg.content[7];
	
	if(_rx_msg.message_type == MSGID_NEW_ALIVE_RESPONSE) {
		if((_rx_msg.content[0]) == TYPE_DOOR_UNIT){
			
			int id_for_door = _rx_msg.sender_id;
			_tx_msg.priority = 0;
			_tx_msg.message_type = MSGID_UNLOCK_DOOR;
			_tx_msg.reciever_id = id_for_door;
			_tx_msg.content
			_tx_msg.content |= sequence_n;
		}
	}
	can_send_message(&_rt_info, &_ls_info, CAN1, _tx_msg);
}

void replay_attack(void){
	
}
*/