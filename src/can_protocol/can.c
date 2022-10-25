#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_can.h"
#include "can.h"
#include "timer.h"

unsigned short self_id;
unsigned char waiting_for_alive_response = 0;

// Configures selected CAN interface for incoming messages
void can_init(CAN_TypeDef * CANx, int is_central_unit) {
    // enable clocks
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE); 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // configure GPIOB for CAN communication
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_CAN1);     
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_CAN1); 
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_CAN1);     
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_CAN1);

    GPIO_InitTypeDef gpiob_init;
    GPIO_StructInit(&gpiob_init);

    gpiob_init.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_6;
    gpiob_init.GPIO_Mode = GPIO_Mode_AF;
    gpiob_init.GPIO_OType = GPIO_OType_PP;
    gpiob_init.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpiob_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpiob_init);  

    gpiob_init.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_5;
    gpiob_init.GPIO_Mode = GPIO_Mode_AF;
    gpiob_init.GPIO_OType = GPIO_Mode_IN;
    gpiob_init.GPIO_PuPd = GPIO_PuPd_UP;    
    gpiob_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpiob_init);

    // configure CAN
    CAN_InitTypeDef can_init;
    CAN_StructInit(&can_init);

    // 42 MHz clock on APB1
    // prescaler = 7 => time quanta tq = 1/6 us
    // bit time = tq*(SJW + BS1 + BS2)
    // see figure 346 in F407 - Reference Manual.pdf
    can_init.CAN_BS1 = CAN_BS1_3tq; 
    can_init.CAN_BS2 = CAN_BS2_4tq; 
    can_init.CAN_Prescaler = 7; // baudrate 750kbps

    // initialize CAN settings
    if (CAN_Init(CANx, &can_init) == CAN_InitStatus_Success)
        USART_Snd_StrLn("CAN init successful!");
    else
        USART_Snd_StrLn("CAN Init failed!");

    // config CAN-filter
    CAN_FilterInitTypeDef can_filter_init;

    // default filter - accept all to CAN_FIFO
    can_filter_init.CAN_FilterNumber = CANx == CAN1 ? 0 : 14; // 0..13 for CAN1, 14..27 for CAN2
    can_filter_init.CAN_FilterFIFOAssignment = CAN_FIFO0;
    can_filter_init.CAN_FilterMode = CAN_FilterMode_IdMask;
    can_filter_init.CAN_FilterScale = CAN_FilterScale_16bit;
    can_filter_init.CAN_FilterIdHigh = 0x0000;
    can_filter_init.CAN_FilterIdLow = 0x0000;
    can_filter_init.CAN_FilterMaskIdHigh = 0x0000;
    can_filter_init.CAN_FilterMaskIdLow = 0x0000;
    can_filter_init.CAN_FilterFIFOAssignment = 0;
    can_filter_init.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&can_filter_init);
	
	if (is_central_unit) {
		self_id = 0;
	}
	else {
		// 7 is the largest number available for the id, this is to prevent it from colliding with other units
		// And most importantly to keep it from colliding with the central unit.
		self_id = 7;
	}
}
// Gets a pending message from a specified can peripheral.
// The message is then decoded and acks are sent as required.
// Returns 1 if a message was read, 0 otherwise.
int can_receive_message(rt_info* _rt_info, ls_info* _ls_info, CAN_TypeDef* CANx, rx_can_msg* rx_msg) {
    if (CAN_MessagePending(CANx, CAN_FIFO0) < 1) { return 0; }
    CanRxMsg can_msg;

    CAN_Receive(CANx, CAN_FIFO0, &can_msg);

    // Decode the sender_id, reciever_id and the type of message from the StdId field
    unsigned char message_type = (can_msg.StdId & 0b01111000000) >> 6;
    unsigned char sender_id = (can_msg.StdId & 0b00000111000) >> 3;
    unsigned char reciever_id = can_msg.StdId & 0b00000000111;

    // Get the last two chars in the data which correspond to the sequence number
    unsigned short sequence_n = can_msg.Data[6] << 8;
    sequence_n |= can_msg.Data[7];


    // Invalid message type was sent
    if (message_type > MAX_MSGID) { 
        if (DEBUG) {
            USART_Snd_StrLn("Invalid message type"); 
        }
        return 0; 
    }

    // If a unit recieved a message that is supposed to be from itself
    // Then a replay attack may be happening, start the alarm
    if (sender_id == reciever_id) {
        // Trigger alarm
    }

    // If the central unit recieves a message from a unit
    // That has not sent its first alive message
    // Then it could be a hostile unit, just ignore the message
    if (self_id == 0) {
        if (!units[sender_id].is_used && message_type != MSGID_NEW_ALIVE && message_type != MSGID_ACK) {
            if (DEBUG) {
    			USART_Snd_StrLn("Unrecognised unit message recieved");
            }
            return 0;
        }
    }

    // If a non central unit recieves a message from anything other than the central unit, just throw it away
    // They cant confirm if a unit is hostile or not because they dont have unit info
    // So they should only accept messages from the central unit
    // If a unit tries to impersonate the central unit, an alarm will be started
    if (self_id != 0) {
        if (sender_id != 0) {
            if (DEBUG) {
    			USART_Snd_StrLn("Non central unit recieved message from non central unit");
            }
            return 0;
        }
    }

    // Save all data into our own recieve message struct
    rx_msg->message_type = message_type;
    rx_msg->sender_id = sender_id;
    int content_length = sizeof(rx_msg->content);
    for (int i = 0; i < content_length; i++) {
        rx_msg->content[i] = can_msg.Data[i];
    }
	
	if (self_id != 7) {
		// If the unit has not been initialized then it cant check if a message was meant for itself or not
		// Message is not meant for this unit
		if (reciever_id != self_id) { 
            if (DEBUG) {
                USART_Snd_StrLn("Message not meant for this unit");
            }
            return 0; 
        }

	}

    // If we recieve an ack for a message
    // Then we know that the retransmit frame will no longer be needed
    // Set is used for that frame to 0
    if (message_type == MSGID_ACK) {
		
        for (int i = 0; i < MAX_RT_FRAMES; i++) {
            if (_rt_info->rt_frames[i].sequence_n == sequence_n) {
				
                _rt_info->rt_frames[i].is_used = 0;
				break;
            }
        }
    }

    // Lifesigns and acks should not be acked
    else if (!(message_type == MSGID_LIFESIGN)) {
        if (DEBUG) {
    		USART_Snd_StrLn("Recieved non ack, non lifesign message");
        }
        tx_can_msg ack;

        ack.message_type = MSGID_ACK;
        ack.priority = 1;
        ack.reciever_id = sender_id;

        if (_rt_info->recieve_sequence_num[sender_id] + 1 == sequence_n) {
            // The message has the correct sequence number, update seq number and 
            // send back an ack
            _rt_info->recieve_sequence_num[sender_id] = sequence_n;
            can_send_message(_rt_info, _ls_info, CANx, ack);
            if (DEBUG) {
    			USART_Snd_StrLn("Sent ack for message");
            }
            return 1;
        }
        else if ((_rt_info->recieve_sequence_num[sender_id] + 1) > sequence_n) {
            if (DEBUG) {
	    		USART_Snd_StrLn("recieved duplicate message, sending ack");
            }
            // The message is a duplicate, ack might have been lost, send a new ack
            can_send_message(_rt_info, _ls_info, CANx, ack);
        }
        return 0;        
    }
	else {
		// Lifesigns should be recieved
		return 1;
	}
}
// Send a message over CAN and save the message in the retransmission buffer
void can_send_message(rt_info* _rt_info, ls_info* _ls_info, CAN_TypeDef* CANx, tx_can_msg tx_msg) {
    unsigned short sequence_n = _rt_info->transmit_sequence_num[tx_msg.reciever_id];
    int length = sizeof(tx_msg.content) + sizeof(sequence_n);

    // The StdId field contains the following information in its 11 bits of storage
    // Bit 10: Priority
    // Bit 9-6: Type of message
    // Bit 5-3: Sender ID
    // Bit 2-0: Reciever ID
    CanTxMsg outgoing = {
        .StdId = (tx_msg.priority << 10) + (tx_msg.message_type << 6) + (self_id << 3) + tx_msg.reciever_id,
        .DLC = length,
        .RTR = CAN_RTR_Data,
        .IDE = CAN_ID_STD,
    };

    // The first 6 bytes of data should be the content
    int content_length = sizeof(tx_msg.content);
    for (int i = 0; i < content_length; i++) {
        outgoing.Data[i] = tx_msg.content[i];
    }
    
    // The next 2 should be the sequence number
    // Which should be the latest sequence number saved in the context
    outgoing.Data[6] = (sequence_n >> 8) & 0xff;
    outgoing.Data[7] = sequence_n & 0xff;

    // send the message over the selected CAN peripheral
    char mailbox = CAN_Transmit(CANx, &outgoing);

    // check if the transmission buffer was full
    if (mailbox == CAN_TxStatus_NoMailBox) {
        if (DEBUG) {
            USART_Snd_StrLn("CAN TxBuf full!");
        }
    }

    _rt_info->transmit_sequence_num[tx_msg.reciever_id]++;

    // Don't ack lifesigns as they are sent repeteadly, also don't ack other acks because it creates a infinite loop.   
    if (tx_msg.message_type != MSGID_ACK && tx_msg.message_type != MSGID_LIFESIGN) {

        // Save the transmitted message in the retransmission buffer
        // If the buffer is full then the message is not saved
        for (int i = 0; i < MAX_RT_FRAMES; i++) {
            if (!_rt_info->rt_frames[i].is_used) {
                _rt_info->rt_frames[i].is_used = 1;
                _rt_info->rt_frames[i].CANx = CANx;
                _rt_info->rt_frames[i].msg = outgoing;
                _rt_info->rt_frames[i].send_timestamp = timer_ms;
                _rt_info->rt_frames[i].sequence_n = sequence_n;
            }
        }
    }
}

// Checks if timeouts have been reached for retransmissions and lifesigns and take action accordingly.
// Should run in the main loop of each startup.c script.
void can_update(rt_info* _rt_info, ls_info* _ls_info) {
    // Check if ack timeout has been reached for any of the messages sent
    for (int i = 0; i < MAX_RT_FRAMES; i++) {
        if (_rt_info->rt_frames[i].is_used) {
            if ((_rt_info->rt_frames[i].send_timestamp + ACK_TIMEOUT_MS) <= timer_ms) {
                // Ack has not been recieved
                // Retransmit the message
                char mailbox = CAN_Transmit(_rt_info->rt_frames[i].CANx, &_rt_info->rt_frames[i].msg);
                // check if the transmission buffer was full
                if (mailbox == CAN_TxStatus_NoMailBox) {
                    if (DEBUG) {
                        USART_Snd_StrLn("Can buffer full while attempting retransmit, trying again...");
                    }
                }
            }
        }
    }

    // If the current unit is not the central unit, send out lifesigns according to the LIEFSIGN_FREQUENCY_MS
    if (self_id != 0) {
        if ((_ls_info->latest_self_lifesign_timestamp + LIFESIGN_FREQUENCY_MS) <= timer_ms) {
            _ls_info->latest_self_lifesign_timestamp = timer_ms;
            tx_can_msg lifesign;
            lifesign.priority = 1;
            lifesign.message_type = MSGID_LIFESIGN;
            lifesign.reciever_id = 0;
            can_send_message(_rt_info, _ls_info, CAN1, lifesign);
        } 
    }
	// Otherwise the unit is the central unit
    else {
        // Check if any of the units have not sent a lifesign after double the time they were given
        for(int i = 0; i < MAX_UNITS; i++) {
            if (_ls_info->is_connected[i]) {
                if ((_ls_info->recieved_lifesigns[i] + LIFESIGN_FREQUENCY_MS * 2) <= timer_ms) {
                    // A connected device is no longer sending a lifesign
                    // It might have been disconnected, to prevent unauthorized access
                    // Start the alarm
                }
            }
        }
    }
}