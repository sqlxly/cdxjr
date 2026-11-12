#ifndef __MYNET_H
#define __MYNET_H

#include "stm32H7xx.h"
#include <stdbool.h>

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "stdlib.h"
#include "string.h"
#include <ctype.h>
#include <stdio.h>
#include "data_table.h"

//#define BSP_MQTT
#define TCP_BUFF_SIZE        512
#define OFFICAL_VERSION  1
#define TEST_ID          (10000035ul)
#define TEST_IMEI        "867587022907958"

#define ERROR_WENDU       (-20)

#define ALIYUN_PROPERTY_VERSION    "1.0.0"
#define ALIYUN_EVENT_VERSION       "1.0.0"
#define ALIYUN_SERVICE_VERSION     "1.0.0"

#define ALIYUN_PROPERTY_METHOD          "thing.event.property.post"
#define ALIYUN_EVENT_METHOD             "thing.event.sj.post"
#define ALIYUN_SERVICE_METHOD           "thing.service.fw"

#define ALIYUN_PROTERTY_OUTPUT_PARAM "sx"

#define ALIYUN_EVENT_OUTPUT_PARAM1    "t"
#define ALIYUN_EVENT_OUTPUT_PARAM2    "v"
#define ALIYUN_EVENT_OUTPUT_PARAM2_EXTRA1    "v1"
#define ALIYUN_EVENT_OUTPUT_PARAM2_EXTRA2    "v2"
#define ALIYUN_EVENT_OUTPUT_PARAM2_EXTRA3    "v3"

#define ALIYUN_SERVICE_INPUT_PARAM1   "t"
#define ALIYUN_SERVICE_INPUT_PARAM2   "v"
#define ALIYUN_SERVICE_OUTPUT_PARAM1  "t"
#define ALIYUN_SERVICE_OUTPUT_PARAM2  "v"
#define ALIYUN_SERVICE_OUTPUT_PARAM2_EXTRA1  "v1"
#define ALIYUN_SERVICE_OUTPUT_PARAM2_EXTRA2  "v2"
#define ALIYUN_SERVICE_OUTPUT_PARAM2_EXTRA3  "v3"

#define AliyunEventValue    3 //(1:获取系统参数2:获取最新属性信息) 
#define AliyunPropertyValue 2 //(获取最新属性信息) 
#define AliyunSystemParam   1 //(获取系统参数) 

#define AliyunSyncSuccess  "1"
#define AliyunSyncFailed   "0" 

#define AIRXXX_PRODUCT_KEY  "a1frsqwfoix"
#define AIRXXX_PRODUCE_SECRET "6CTIjuAoX09GqZwv"


typedef struct AIRXXXSTATE
{
	bool flash_flag;
	bool airxxx_link;
	bool ota_flag;
	bool ota_recv;
	uint8_t aliyun_eth_or_lte;//0:init 1:eth 2:lte
	uint8_t run_mode;
	uint8_t init_flag;// 1:网线已连接   2:get ip sucess    3:get device_name sucess    4:get device_secret sucess    5: 关闭4G模块    6:连接MQTT 
	uint8_t airxxx_rssi;
	uint8_t airxxx_net_mode;//2 3 4G
	uint8_t recv_flag;
	int recv_systeminfo_cmd;//  0 -->收到服务器指令 ——> 1 发送串口查询系统参数指令 --> 2 收到AC2000串口数据并解析 --> 3 返回服务器系统参数数据 --> 0
	int recv_property_cmd;  //  0 -->收到服务器指令 ——> 1 中转 --> 2 返回服务器产品属性数据 --> 3 返回服务器应答 --> 0
	int aliyun_service_param1;//参数t
	int aliyun_service_param2;//参数v
	int aliyun_service_param_reply;
	int ota_firmware_version;
	uint32_t ota_firmware_total_package;
	uint32_t ota_firmware_size;
	uint32_t ota_firmware_reclen;
	int32_t ota_firmware_send_index;
	int32_t ota_firmware_recv_index;
	char airxxx_lat[20];
	char airxxx_lng[20];
	char airxxx_lac[20];
	char airxxx_ci[20];
	char airxxx_imei[20];
	char airxxx_iccid[20];
	char airxxx_net_reg[20];
	char airxxx_phone_number[20];
	char airxxx_time[32];
	char product_key[36];
	char product_secret[36];
	char device_name[36];
	char device_secret[36];
	char device_token[36];
	char ota_firmware_md5[64];
	char airxxx_location[128];
	char product_key_encrypt[128];
	char product_secret_encrypt[128];
	char device_secret_encrypt[128];
	uint8_t ota_firmware_data[2048];
}AIRXXXSTATUS;



extern AIRXXXSTATUS AirxxxStatus;

int get_alarm_num(int version);
void PackageDataToServer(int index,char *data);



#endif
