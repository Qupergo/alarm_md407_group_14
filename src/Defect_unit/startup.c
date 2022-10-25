/*
 * 	startup.c
 *
 */
 
 #include "defect_unit.h"

 
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}



void main(void)
{
	int timestamp_of_last_send = 0;
	
	timer_init();
	can_init(CAN1,0);
	
	while (1) {
		if((TIME_BETWEEN_SEND_MS + timestamp_of_last_send) <= timer_ms) {
			timestamp_of_last_send = timer_ms; // Update timestamp to current send
			
			// send message with standard id and specify the length of the data
			CanTxMsg outgoing = {
			.StdId = 5, // Id is 3 bits meaning the highest possible would be 7
			.DLC = 8, // 8 is the greatest amount, do not change volume using this, change with frequency instead
			.RTR = CAN_RTR_DATA,
			.IDE = CAN_Id_Standard
			};

			// fill the data field
			for(int i = 0; i < 8; i++){
			outgoing.Data[i] = 0;
			}
			CAN_Transmit(CAN1, &outgoing);
		}
		else{
			
		}
	}
	
}

