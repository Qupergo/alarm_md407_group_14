/*
 * 	startup.c
 *
 */
 
 #include "usart.h"
 #include "can.h"
 #include "stm32f4xx_rcc.h"
 #include "timer.h"
 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}

// Frequency for sending data from defect unit in milliseconds
int time_between_send = 10;

void main(void)
{
	rt_info defect_rt_info;
	ls_info defect_ls_info;
	tx_can_msg defect_tx_msg;
	
	timer_init(1);
	// To simulate a defect arbitrary numbers are used being not all ones or zeroes 
	for (int i = 0; i < MAX_UNITS; i++) {
		defect_rt_info.transmit_sequence_num[i] = timer_ms*1;
		defect_rt_info.recieve_sequence_num[i] = timer_ms*2;
		defect_ls_info.is_connected[i] = timer_ms*3;
		defect_ls_info.latest_self_lifesign_timestamp = timer_ms*4;
		defect_ls_info.recieved_lifesigns[i] = timer_ms*5;
	}
	for (int i = 0; i < MAX_RT_FRAMES; i++) {
		defect_rt_info.rt_frames[i].is_used = timer_ms*6;
	}
	
	defect_tx_msg.priority = 1;
	defect_tx_msg.message_type = 1;
	defect_tx_msg.reciever_id = 1;
	for(int i = 0; i < 6; i++){
		defect_tx_msg.content[i] = 'L'; // Literally just take the L
	}


	while (1) {
		if(timer_ms >= time_between_send){
		can_send_message(&defect_rt_info, &defect_ls_info, CAN1, defect_tx_msg);
		}
	}
}

