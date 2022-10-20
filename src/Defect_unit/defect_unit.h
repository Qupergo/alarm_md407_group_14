#include "can.h"

#define TIME_BETWEEN_SEND_MS 10

rt_info _rt_info;
ls_info _ls_info;
rx_can_msg _rx_msg;
tx_can_msg _tx_msg;
void imitation_attack(void);
void replay_attack(void);