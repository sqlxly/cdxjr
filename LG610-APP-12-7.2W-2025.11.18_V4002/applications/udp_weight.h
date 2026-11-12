
#include <rtthread.h>


#ifndef _UDP_WEIGHT_H__
#define _UDP_WEIGHT_H__


typedef struct 
{
	uint8_t alarm_weight :1;  
	uint8_t alarm_adc :1;
	uint8_t alarm_pee :1;
	uint8_t alarm_can :1;
	uint8_t alarm_crack :1;
	uint8_t warn_crack :1;
	uint8_t back :2;
}alarm_info;

typedef struct 
{
	alarm_info ch[6];
	uint8_t senser_err;
	uint8_t encode_err;
}weight_alarm_info;
typedef struct 
{
	uint32_t head;  
	uint16_t msg_len;
	uint16_t mesg_id;
	uint16_t weight[6];
	uint8_t level[6];
	uint8_t alarm[22];
	uint32_t CRC32;
}send_mesg_info;

void get_deviceB_weight_info(send_mesg_info *dat);
#define UDP_READ_MESSAGE_HEAD 0x11223344
#define UDP_MESSAGE_HEAD 0x33445566

#endif   