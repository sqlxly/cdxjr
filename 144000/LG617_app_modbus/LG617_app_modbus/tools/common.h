#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>

#define AT(x)  							__attribute__((at(x)))

#define	APP_VERSION  				1023
	
#define WEIGH_CHANNEL	 			6			//6个HX71708称重采集芯片

#define MOVE_OUT_PLUSE 			8 

#define UDP_SND_PLUSE 			13

#define ENCODER_SPEED_MAX		40

#define ENCODER_SPEED_MIN		25

#define WEIGH_CLEAR_LEVEL		30.0f

#define UDP_MSG_HEAD 				0x33445566

#define REQ_UDP_MSG_HEAD 		0x11223344

#define SRAM_SAFE_CODE 			0x12345678

#define UDP_MSG_ID_LEN			60000

#define MODBUS_TCP_BASEADDR 8000

#define setbitv(x,y)  x|=(1<<y)   //位读写
#define clrbitv(x,y)  x&=~(1<<y) 
#define getbitv(x,y)  ((x) >> (y)&1)

typedef struct {
	uint8_t sensitivity_1; 
	uint8_t sensitivity_2; 
	uint8_t sensitivity_3; 
	uint16_t addCnt;
}CrackParam;

typedef struct {
	uint32_t safeCode;
	uint16_t errTimes[6][32];
} Bkup_Sram_Alarm;
	
typedef struct {
	uint16_t baseAddr;
	uint16_t len;
} MBReceiveType;

typedef struct
{
	uint16_t command;
	uint16_t mesg_id;   
	uint8_t type[6];   	//图片显示鸡蛋三种状态
  uint16_t weight[6]; //鸡蛋重量
	uint8_t level[6];   //裂纹等级
}Egg_Quality_Info;

typedef struct
{
	uint32_t head;      //消息头 0x33445566
	uint16_t msg_len;   //数据长度
	uint16_t mesg_id;   //消息id
  uint16_t weight[6]; //鸡蛋重量
	uint8_t level[6];   //裂纹等级
	uint8_t alarm[22];  //报警信息
	uint32_t CRC32;     //CRC异或校验
}Send_Mesg_Info;

typedef struct
{
	uint32_t head;   		//消息头 UDP_MSG_HEAD
	uint16_t msg_len;		//数据长度
	uint16_t mesg_id;		//消息id
	uint8_t  result; 		//返回值 1成功 0失败
	uint32_t CRC32;  		//CRC异或校验
}Replay_Mesg_Info;

typedef struct
{
	uint32_t ZeroAD[6];   			//零位校准时 零位AD值
	uint32_t autoPeelAD[6];			//自动去皮值 将输入重量转换成对应AD值
	uint32_t addPeelAlarmAD[6]; //累计去皮报警值 将输入重量转换成对应AD值
	uint32_t weighThreshold[6];	//鸡蛋称重起始阀值 超过此值才认为是鸡蛋 将输入重量转换成对应AD值
	double linearK[6];					//称重线性参数 线性方程斜率值k
}Weigh_Linear_Info;

struct AlarmType
{
	uint8_t errWeigh			:1;  //称重传感器异常
	uint8_t errAdc				:1;	 //称重数据采集芯片故障
	uint8_t	errPeel				:1;  //当前皮重超过累计去皮报警值
	uint8_t errCan				:1;  //CAN总线通信故障
	uint8_t errCrack			:1;	 //破口检测传感器损坏率过高
	uint8_t warnCrack			:1;  //破口检测传感器有部分损坏 但没超过设定的损坏率 只用作提示信息 可以不触发报警
	uint8_t bak						:2;  //未使用
};

struct AlarmInfo{
	struct AlarmType type[6];
	uint8_t proximitySensorErr;		//破口检测单元接近开关故障
	uint8_t encoderErr;						//破口检测单元编码器故障
};

void BackupSRAM_config(void);

void BackupSRAM_clear(void);

/* eeprom */
rt_err_t save_weigh_calibration(void);

rt_err_t read_weigh_calibration(void);

Weigh_Linear_Info *get_weigh_calibration(void);

struct AlarmInfo getAlarmInfo(void);

#ifdef __cplusplus
}
#endif

#endif
