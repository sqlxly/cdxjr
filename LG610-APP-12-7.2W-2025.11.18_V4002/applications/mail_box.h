
#include <rtthread.h>

#ifndef _MAIL_BOX_H__
#define _MAIL_BOX_H__

#define MESSAGE_DAT_LEN 64
#define AI_MESSAGE_DAT_LEN 100
typedef struct 
{
	uint8_t addr;     //包装通道设备地址
	uint8_t cmd;      //控制命令
	uint16_t val;     //命令值
} packge_mail_box_type;

extern struct rt_messagequeue masseg_p1;
extern struct rt_messagequeue masseg_p2;
extern struct rt_messagequeue masseg_p3;
extern struct rt_messagequeue masseg_p4;
extern struct rt_messagequeue masseg_p5;
extern struct rt_messagequeue masseg_p6;
extern struct rt_messagequeue masseg_t1;
extern struct rt_messagequeue masseg_throw1;
extern struct rt_messagequeue masseg_throw2;
extern struct rt_messagequeue masseg_weight;
extern struct rt_messagequeue masseg_ai;
void get_screen_mb(struct rt_mailbox *s_mb);

#endif  	// _SCREEN_CHAT_H_
