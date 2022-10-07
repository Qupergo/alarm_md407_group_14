/*
 * 	startup.c
 *
 */
 
 #include <stdio.h>
 #include <usart.h>
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup ( void )
{
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
}



void CAN_send(char* p)
{
	//Function placeholder for CAN-protocol function
	print("hello world");
}
 typedef struct
{
	int priority;		// Priority bit 10
	int type;			// Message type bits 9-6
	int sender;			// Sender ID bits 5-3
	int receiver;		// Receiver ID  bits 3-0

} CAN_msg;

void delayms(int i)
{
	//delay from TIM
}

void main(void)
{
	// If only send data via a function for send this will be fine, 
	// just need to check so it's the right amout of bits sent in the array
	char jumble[] = {'D','e','f','e','c','t',' ','B','o','a','r','d'};
//	int i = 8;
	while(1)
	{
		CAN_send(jumble);
//		delayms(3);
	}
	
	// If send data by using a CAN.struct
	CAN_msg mumble;
	mumble.priority	= 0b10000000000;
	mumble.type		= 0b01111000000;
	mumble.sender	= 0b00000111000;
	mumble.receiver	= 0b00000000111;
	
	// Send mumble
	// Waiting for CAN protocol to finish
}

