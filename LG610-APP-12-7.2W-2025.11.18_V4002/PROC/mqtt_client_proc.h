#ifndef __MQTT_CLIENT_PROC_H__
#define __MQTT_CLIENT_PROC_H__

#include <rtthread.h>
#include <rtdevice.h>
#include "common.h"

typedef struct {
	int cmd;							//MQTT订阅消息的命令码
	int mbAddr;						//根据IEMI解析设备的modbus地址
	char msgId[30];				//消息ID
	char *data;						//其它数据
}PayloadMsg;

int mqtt_update_property(char *payload, rt_uint16_t len);

int mqtt_update_warn(char *payload, rt_uint16_t len);

void mqtt_property_back_pubtopic(int cmd, char *msgId, rt_uint16_t state);

PayloadMsg payload_msg_analyse(char *payload, GWDataType *gwType, int len);
void mqtt_parameter_back_pubtopic(int cmd, char *msgId, char *payload);
#endif
