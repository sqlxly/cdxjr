#include "mqtt_client_proc.h"
#include "mqtt_client.h"
#include "netTools.h"
#include "font.h"
#include "terminal_proc.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <string.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME    "mqtt.sample"
#define DBG_LEVEL           DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>
//test IEMI: 001900333030510733303739
#define MQTT_URI                "tcp://mqtt.znmc.co:1883"
//#define MQTT_URI                "tcp://10.0.8.227:1883"
//#define MQTT_SUBTOPIC           "/mqtt/test"
//#define MQTT_PUBTOPIC           "/mqtt/test"
#define MQTT_WILLMSG            "Internet Off!"

int is_online = 0;															//MQTT连接状态

/* define MQTT client context */
static mqtt_client client;											//MQTT客户端设备参数

static char mqtt_property_pubtopic[64] = {0};		//设备端上报属性topic

static char mqtt_warn_pubtopic[64] = {0};				//设备端报警事件topic

static char mqtt_back_pubtopic[64] = {0};				//设备端订阅消息回复topic

static char mqtt_subtopic[64] = {0};						//订阅服务端下发参数topic

static int is_started = 0;

static void mqtt_connect_callback(mqtt_client *c)
{
    LOG_D("inter mqtt_connect_callback!");
}
extern unsigned char ari720_module_status;
extern void setNetworkStatus(int status);
static void mqtt_online_callback(mqtt_client *c)
{
	is_online = 1;
	ari720_module_status = 0;
	setNetworkStatus(Normal);//网络正常
    LOG_D("inter mqtt_online_callback!");
}

static void mqtt_offline_callback(mqtt_client *c)
{
	is_online = 0;
	LOG_D("inter mqtt_offline_callback!");
}

static void mqtt_sub_callback(mqtt_client *c, message_data *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub callback: %.*s %.*s",
               msg_data->topic_name->lenstring.len,
               msg_data->topic_name->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
		/* 通知数据处理线程解析数据 */
		_operate_mbus1_mq_send((char *)msg_data->message->payload, strlen(msg_data->message->payload)+1);
}

static void mqtt_sub_default_callback(mqtt_client *c, message_data *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub default callback: %.*s %.*s",
               msg_data->topic_name->lenstring.len,
               msg_data->topic_name->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
}

static int mqtt_start(void)
{
    /* init condata param by using MQTTPacket_connectData_initializer */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    if (is_started)
    {
        return -1;
    }
    /* config MQTT context param */
    {
        client.isconnected = 0;
        client.uri = MQTT_URI;

        /* generate the random client ID */
				rt_snprintf(mqtt_subtopic, 					sizeof(mqtt_subtopic), 					"/sub/device/%s/fw", 		 stm32_uniqueId);		//订阅服务端下发参数topic
				rt_snprintf(mqtt_property_pubtopic, sizeof(mqtt_property_pubtopic), "/up/device/%s/property", stm32_uniqueId);		//设备端上报属性topic
				rt_snprintf(mqtt_warn_pubtopic, 		sizeof(mqtt_warn_pubtopic), 		"/up/device/%s/warn", 		 stm32_uniqueId);		//设备端报警事件topic
				rt_snprintf(mqtt_back_pubtopic, 		sizeof(mqtt_back_pubtopic), 		"/up/device/%s/fw/back",  stm32_uniqueId);		//设备端订阅消息回复topic
			
        /* config connect param */
        rt_memcpy(&client.condata, &condata, sizeof(condata));
        client.condata.clientID.cstring = stm32_uniqueId;
				client.condata.username.cstring = "LG105_USER";
				client.condata.password.cstring = "znmc@device";
        client.condata.keepAliveInterval = 30;
        client.condata.cleansession = 1;

        /* config MQTT will param. */
        client.condata.willFlag = 1;
        client.condata.will.qos = 1;
        client.condata.will.retained = 0;
        client.condata.will.topicName.cstring = mqtt_property_pubtopic;
        client.condata.will.message.cstring = MQTT_WILLMSG;

        /* malloc buffer. */
        client.buf_size = client.readbuf_size = 4096;
        client.buf = rt_calloc(1, client.buf_size);
        client.readbuf = rt_calloc(1, client.readbuf_size);
        if (!(client.buf && client.readbuf))
        {
            LOG_E("no memory for MQTT client buffer!");
            return -1;
        }

        /* set event callback function */
        client.connect_callback = mqtt_connect_callback;
        client.online_callback = mqtt_online_callback;
        client.offline_callback = mqtt_offline_callback;

        /* set subscribe table and event callback */
        client.message_handlers[0].topicFilter = rt_strdup(mqtt_subtopic);
        client.message_handlers[0].callback = mqtt_sub_callback;
        client.message_handlers[0].qos = QOS1;

        /* set default subscribe event callback */
        client.default_message_handlers = mqtt_sub_default_callback;
    }

		{
      int value;
      uint16_t u16Value;
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_CONN_TIMEO, &value);
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_MSG_TIMEO, &value);
      value = 5;
      paho_mqtt_control(&client, MQTT_CTRL_SET_RECONN_INTERVAL, &value);
      value = 30;
      paho_mqtt_control(&client, MQTT_CTRL_SET_KEEPALIVE_INTERVAL, &value);
      u16Value = 3;
      paho_mqtt_control(&client, MQTT_CTRL_SET_KEEPALIVE_COUNT, &u16Value);
    }
		
    /* run mqtt client */
    paho_mqtt_start(&client, 8192, 7);
    is_started = 1;

    return 0;
}

void network_mode_init(void)
{
	char ipAddr[20];
	char gwAddr[20];
	char maskAddr[20];
	TNetworkInfo *info;

	info = getTNetworInfo();

	lan_or_4G_select(info->netMode);
	
	if(info->netMode == 0){		
		if(info->ipMode == 1){  
			sprintf(ipAddr, "%d.%d.%d.%d", info->ip[0], info->ip[1], info->ip[2], info->ip[3]);
			sprintf(maskAddr, "%d.%d.%d.%d", info->nm[0], info->nm[1], info->nm[2], info->nm[3]);
			sprintf(gwAddr, "%d.%d.%d.%d", info->gw[0], info->gw[1], info->gw[2], info->gw[3]);
			netdev_work_mode_select(1, 3, "%s%s%s", ipAddr, maskAddr, gwAddr);
		}else{
			netdev_work_mode_select(0, 0, "%s%s%s", "0.0.0.0", "0.0.0.0", "0.0.0.0");
		}
	}
}

int get_mqtt_client_connected(void)
{
	return client.isconnected;
}

int mqtt_publish(const char *topic, void *json, int len)
{
    if (is_started == 0)
    {
        return -1;
    }

		if (is_online == 0)
    {
        return -1;
    }
		
    return paho_mqtt_publish(&client, QOS1, topic, json, len);
}

int mqtt_update_property(char *payload, rt_uint16_t len)
{
//	rt_kprintf("property: %s\n", payload);
	return mqtt_publish(mqtt_property_pubtopic, payload, len);
}

int mqtt_update_warn(char *payload, uint16_t len)
{
//	rt_kprintf("warn: %s\n", payload);
	return mqtt_publish(mqtt_warn_pubtopic, payload, len);
}

void mqtt_property_back_pubtopic(int cmd, char *msgId, rt_uint16_t state)
{
	char *payload = rt_malloc(128);

	rt_memset(payload, '\0', 128);
	sprintf(payload, "%d$%s$%d", cmd, msgId, state);
//	rt_kprintf("%s %s\n", mqtt_back_pubtopic, pstr);
	mqtt_publish(mqtt_back_pubtopic, payload, strlen(payload));
	rt_free(payload);
}

void mqtt_parameter_back_pubtopic(int cmd, char *msgId, char *data)
{
	char *payload = rt_malloc(1024);
	sprintf(payload, "%d$%s$%s", cmd, msgId, data);
	rt_kprintf("%s %s\n", mqtt_back_pubtopic, payload);
	mqtt_publish(mqtt_back_pubtopic, payload, strlen(payload));
	rt_free(payload);
}

PayloadMsg payload_msg_analyse(char *payload, GWDataType *gwType, int len)
{
	char *pStr, str[30], i;
	char *result = RT_NULL;
	PayloadMsg msg;
	GWDataType *pGw;
	
	pStr = (char *)payload;
	//解析命令关键字
	result = strtok( pStr, "$" );
	if(result == NULL) return msg;
	msg.cmd = atoi(result);
	//解析随机包序列号
	result = strtok( NULL, "$" );
	if(result == NULL) return msg;
	else if(strlen(result) >= 20) return msg;
	sprintf(msg.msgId, result, strlen(result));
	//分割出数据字段
	result = strtok( NULL, "$" );
	msg.data = result;
	//解析modbus地址
	if((msg.cmd == 2)||(msg.cmd == 5)){
		msg.mbAddr = -1;
		pGw = gwType;
		for(i=0; i<len; i++){
			sprintf(str, "%04x%04x%04x%04x%04x%04x", pGw->iemi[1], pGw->iemi[0], pGw->iemi[3], pGw->iemi[2], pGw->iemi[5], pGw->iemi[4]); 
			if(strstr(msg.data, str) != RT_NULL){
				msg.mbAddr = i;
				break;
			}
			pGw++;
		}
		rt_kprintf("msg.mbAddr = %d\n", msg.mbAddr);
	}

	return msg;
}

void mqtt_paho_pthread(void)
{
	/* 初始化网络配置 选择有线还是无线网络 */
	network_mode_init();
	
	/* 初始化MQTT任务 创建TOPIC 订阅消息 */
	mqtt_start();
}
