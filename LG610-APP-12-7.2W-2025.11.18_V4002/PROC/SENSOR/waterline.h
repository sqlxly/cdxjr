#ifndef __WATERLINE_H
#define __WATERLINE_H

#include "stm32h7xx.h"
#include <stdbool.h>

#define EN_WATERLINE_SENSOR 1
#define WATERLINE_CMD_LENGTH 6
#define WATERLINE_RECV_LENGTH  55
#define stateBinValue 1
#define WATERLINE_LINK_RETRY  55
typedef struct waterline
{
	bool waterline_mount;
	uint8_t waterline_link;
	uint8_t waterline_retry;
	uint8_t waterline_alarm_delay;//报警延时
	uint8_t waterline_alarm_on;//报警开启
	uint8_t waterline_communication;		//缺水报警采集盒通信失败 8048
	uint8_t waterline_alarm_state;	    //水线缺水报警           8049
}WATERLINE;

extern WATERLINE waterline_table;
extern const uint8_t waterline_cmd[WATERLINE_CMD_LENGTH];

void waterline_process(uint8_t *dataBuff,uint16_t length);


#endif
