#include <stdbool.h>

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "can.h"
#include "usart.h"



// configures selected CAN interface for incoming messages
void can_init(CAN_TypeDef * CANx){
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
        print("\nCAN init successful!");
    else
        print("\nCAN Init failed!");

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
}

void can_send_message(CAN_TypeDef * CANx, CanTxMsg* outgoing){
    // transmit the message
    char mailbox = CAN_Transmit(CANx, outgoing);

    // check if the transmission buffer is full
    if (mailbox == CAN_TxStatus_NoMailBox) {
        print("CAN TxBuf full!");
    }
}

bool can_receive_message(CAN_TypeDef * CANx, CanRxMsg* message){
    // check if there are pending messages in the queue
    if(CAN_MessagePending(CANx, CAN_FIFO0) < 1){
        return false;
    }
    // retrieve the message and copy it to message struct argument
    CAN_Receive(CANx, CAN_FIFO0, message);
    return true;
}


void can_send_data(CAN_TypeDef * CANx, char *data, char length){
    CanTxMsg outgoing = {
        .StdId = 0,
        .DLC = length, // specify the length
        .RTR = CAN_RTR_Data,
        .IDE = CAN_Id_Extended
    };

    // fill the data field
    for(int i = 0; i < length; i++){
        outgoing.Data[i] = data[i];
    }

    // send the message over the selected CAN peripheral
    can_send_message(CANx, &outgoing);
}

void can_print_data(CanRxMsg* msg){
    // get the id and the length of the message
    uint32_t id = msg->StdId;
    char length = msg->DLC;

    // print the message data
    char msg_data[9];
    for(int i = 0; i < length; i++){
        msg_data[i] = msg->Data[i];
    }
    
    // terminate the string
    msg_data[length] = '\0';
    print(msg_data);
}
