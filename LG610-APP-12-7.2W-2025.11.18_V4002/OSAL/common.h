#ifndef __COMMON_H__
#define __COMMON_H__

#include <rtthread.h>
#include <rtdevice.h>

#define VERSION    "V1.0.0"   		//系统版本
#define VERSION_FLOAT    1.00  		//系统版本
#define DATETIME   "2022/08/10"   //版本时间

#define ALARM_TYPE_LEN			60

#define RTU_TIMEOUT		500
#define RTU_DELAY_MS	20
#define UID_BASEADDR 		UID_BASE
typedef enum MQTTdataType{
  eUpdate 			= 1,							//上报所有数据
  eRefresh 	 	  = 2,							//上报数据变化量  
} MQTTDataType;
typedef enum NetworkStatus{
	Checking = 0,//检测中
	ModuleAbnormal	= 1,				//模块通信异常
	SIMAbnormal  =2,                                //SIM卡损坏，或未插卡
	GSMNetworkRejected=3,                         //GSM网络注册失败
	GPRSNetworkRejected = 4,//GPRS网络注册失败
	SingalStrength = 5,//信号强度检测失败
	GPRSConnect  = 6,//不能连接GPRS
	LocalAddr = 7,//获取本地地址失败
	NetworkInit = 8,//网络初始化失败
	ServerConnetFailure =9,                         //服务器通讯失败
	DeviceNotRegistered = 10,                        //设备未注册
	WiredNetworkException = 11,                      //有线网络异常
	Normal	= 12,				//正常
}networkStatus;
typedef struct{
	rt_uint16_t type;							// 节点设备类型
	rt_uint16_t agreeLen;					// 协议长度 
	rt_uint16_t dataLen;					// 数据长度
	rt_uint16_t agreement[100];		// 节点设备协议数据
}DEVDataType;

typedef struct GWDataType{
//	rt_uint8_t devNum;					// 设备序号 即modbus地址
	rt_uint16_t type;						// 节点设备类型
//	rt_uint8_t isOnline;				// 是否在线
	rt_uint16_t iemi[6];				// 24位设备IEMI编码
//	rt_uint8_t alarmBit[8]; 		// 64种报警状态 通过读取modbus线圈状态获得 8*8bit=64bit
	rt_uint16_t data[120];			// 读取modbus输入寄存器并备份 用于设备数据变化时的比较
}GWDataType;

typedef struct{
	rt_uint8_t displayMode;   //显示屏显示方式 1 正向  2逆向
	rt_uint8_t slave_type; //1 3G  2 Platinum 3 Platinum_pro 4 plus
	rt_uint8_t netMode;       //网络方式	 1--4G   0--有线
	rt_uint8_t ipMode;				//IP获取方式 1--手动 0--自动
	rt_uint8_t ip[4];					//IP地址
	rt_uint8_t nm[4];				  //子网掩码
	rt_uint8_t gw[4];					//网关
}TNetworkInfo;

extern char stm32_uniqueId[33];
extern char qrcodeData[900];

void setDisplayMode(rt_uint8_t mode);
void setSlaveType(rt_uint8_t mode);
//联网相关参数设置
void setGetIpMode(rt_uint8_t mode);
void setNetworkMode(rt_uint8_t mode);
void setNetworkDynamicInfo(TNetworkInfo dInfo);
void setIpAddress(rt_uint8_t index, rt_uint8_t v);
void setNmAddress(rt_uint8_t index, rt_uint8_t v);
void setGwAddress(rt_uint8_t index, rt_uint8_t v);
TNetworkInfo *getTNetworInfo(void);
rt_uint8_t getTNetworInfo_net_mode(void);
rt_uint8_t getTNetworInfo_ip_mode(void);
rt_uint8_t getTNetworInfo_ac2000_type(void);
rt_uint8_t getTNetworInfo_display_mode(void);
void readStm32UniqueID(void) ;

rt_err_t flash_write_device_config(TNetworkInfo *buf);
rt_err_t flash_read_device_config(TNetworkInfo *buf);
rt_err_t factory_reset_device_config(void);
#endif
