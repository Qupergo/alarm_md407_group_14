/*
 * 	startup.c
 *
 */
 
 #include "usart.h"
 #include "can.h"
 #include "stm32f4xx_rcc.h"
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

// timer_ms set to 0 for now, change when implementing timer.c
int timer_ms = 0;

void main(void)
{
	// For all units
	rt_info _defect_rt_info;
	ls_info _defect_ls_info;
	tx_can_msg _defect_tx_msg;
	
	// To simulate a defect arbitrary numbers are used being not all ones or zeroes 
	for (int i = 0; i < MAX_UNITS; i++) {
		_defect_rt_info.transmit_sequence_num[i] = timer_ms*1;
		_defect_rt_info.recieve_sequence_num[i] = timer_ms*2;
		_defect_ls_info.is_connected[i] = timer_ms*3;
		_defect_ls_info.latest_self_lifesign_timestamp = timer_ms*4;
		_defect_ls_info.recieved_lifesigns[i] = timer_ms*5;
	}
	for (int i = 0; i < MAX_RT_FRAMES; i++) {
		_defect_rt_info.rt_frames[i].is_used = timer_ms*6;
	}
	
	_defect_tx_msg.priority = 'A';
	_defect_tx_msg.message_type = 'B';
	_defect_tx_msg.reciever_id = 'C';
	for(int i = 0; i < 6; i++){
		_defect_tx_msg.content[i] = 'L'; // Literally just take the L
	}
	
	// For non central units
	// Send initial alive message to central unit
	// Imagine this not working on a defect board
	/*
	tx_can_msg initial_alive;
	initial_alive.priority = 1;
	initial_alive.message_type = NEW_ALIVE_TYPE_ID;
	initial_alive.content[0] = SELF_TYPE;
	initial_alive.content[1] = num_sub_units;
	initial_alive.reciever_id = 0;
	can_send_message(&_rt_info, &_ls_info, CAN1, initial_alive);
	unsigned char waiting_for_alive_response = 1;
	 */
	
	

	while (1) {
		can_send_message(_defect_rt_info, _defect_ls_info, CAN1, txmsg)
		can_update(&_rt_info, &_ls_info);
		if (can_receive_message(&_rt_info, &_ls_info, CAN1, &rx_msg)) {
			switch (rx_msg.message_type) {
				// For non central units
				case NEW_ALIVE_RESPONSE_TYPE_ID:
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

