
#include <rtthread.h>

#ifndef _MAIL_BOX_H__
#define _MAIL_BOX_H__

#define MESSAGE_DAT_LEN 64

typedef struct 
{
	uint8_t addr;     //包装通道设备地址
	uint8_t cmd;      //控制命令
	uint16_t val;     //命令值
} packge_mail_box_type;

extern struct rt_messagequeue masseg_modbus_cmd;
extern struct rt_messagequeue send_control_state_cmd;
#endif  	// _SCREEN_CHAT_H_
