#ifndef USER_ALARM_H
#define USER_ALARM_H

#include <rtthread.h>

typedef enum alarmTypeEnum {
	/*主控报警类型*/
	eATV930ConnectError			= 59,   //ATV930变频器通讯故障
	eATV930DeviceError  		= 60,	 	//ATV930变频器设备故障
	eMotorSyncError					= 61,		//主从电机同步位置误差过大
	eCylinderConnectError		= 62,		//电动推缸伺服通讯故障
	eCylinderDeviceError		= 63,		//电动推缸伺服设备故障
	eCylinderSyncError			= 64,		//电动推缸同步位置误差过大
	eObritCupCloseError			= 65, 	//主轨道蛋杯闭合异常
	eObritEggsPostionError	= 66,		//主轨道蛋杯鸡蛋位置异常
	eWeighAConnectError			= 67,		//称重控制器A通讯故障
	eWeighBConnectError			= 68,		//称重控制器B通讯故障
	eWeighADeviceError			= 69,		//称重控制器A设备故障
	eWeighBDeviceError			= 70,		//称重控制器B设备故障
	eCrackADeviceError			= 71,		//裂纹检测控制器A设备故障
	eCrackBDeviceError			= 72,		//裂纹检测控制器B设备故障
	eDirtyAConnectError			= 73,		//脏蛋检测控制器A通讯故障
	eDirtyBConnectError			= 74,		//脏蛋检测控制器B通讯故障
	eDirtyADeviceError			=	75,		//脏蛋检测控制器A设备故障
	eDirtyBDeviceError			= 76,		//脏蛋检测控制器B设备故障
	eArraysDeviceError			= 77,		//光检通道排列异常

	/*包装通道报警类型*/
	ePackageConnectError					 = 129,	//包装通道1~20通讯故障
	ePackageLayerCache1Error			 = 130,	//包装通道1~20第1层缓存蛋杯故障
	ePackageLayerCache2Error			 = 131,	//包装通道1~20第2层缓存蛋杯故障
	ePackageLayerCache3Error			 = 132,	//包装通道1~20第3层缓存蛋杯故障
	ePackageMoveCacheConnectError	 = 133,	//包装通道1~20移动缓存伺服通讯故障
	ePackageMoveCacheDeviceError	 = 134,	//包装通道1~20移动缓存伺服设备故障
	ePackageMoveCacheCupCloseError = 135,	//包装通道1~20移动缓存蛋杯开闭故障
	ePackageAcceptConnectError		 = 136,	//包装通道1~20接受抓伺服通讯故障
	ePackageAcceptDeviceError			 = 137,	//包装通道1~20接受抓伺服设备故障
	ePackageAcceptClearError			 = 138,	//包装通道1~20接受抓蛋夹中有鸡蛋未放掉
	ePackageConveyConnectError		 = 139,	//包装通道1~20蛋托输送伺服通讯故障
	ePackageConveyDeviceError			 = 140,	//包装通道1~20蛋托输送伺服设备故障
	ePackageAutoDispatchError			 = 141,	//包装通道1~20自动发托机故障
	ePackageTrayMissError					 = 142,	//包装通道1~20蛋托缺失
	ePackageTrayStackedError			 = 143,	//包装通道1~20蛋托堆叠
	ePackageTargetCompleteInfo		 = 144, //包装通道1~20完成产量(只提示不停机)
	
	/*清洗烘干集蛋桌等设备报警类型*/
	//---待添加
	
	/*分级机清洗润滑系统报警类型*/
	//---待添加
	eAlarmTypeUndefine						 = 150		//未定义报警类型
} AlarmTypeDef;

//报警级别定义
typedef enum {
		ALARM_LEVEL_NULL		 = 0,	 //无报警
    ALARM_LEVEL_INFO     = 1,  //提示级别
    ALARM_LEVEL_WARNING  = 2,  //警告级别
    ALARM_LEVEL_ERROR    = 3,  //错误级别
    ALARM_LEVEL_CRITICAL = 4   //致命级别
} alarm_level_t;

//报警动作处理
typedef enum {
    ALARM_ACTION_NONE = 0,         //无动作
    ALARM_ACTION_NOTIFY = 1,       //提示
    ALARM_ACTION_SHUTDOWN = 2      //停机
} alarm_action_t;

typedef struct {
	alarm_action_t action;
	uint8_t isAlarmFlag;
//	uint8_t timeout; //填充报警信息后 需要等一下再触发
} alarm_state_t;

//包装通道故障报警信息掩膜  用位标识是否报警 位号表示通道位置
typedef struct {
	uint32_t	packageConnectErrorMask;		//包装通道1~20通讯故障报警状态掩膜
	uint32_t	cache1ErrorMask;						//包装通道1~20第1层缓存蛋杯故障状态掩膜
	uint32_t	cache2ErrorMask;						//包装通道1~20第2层缓存蛋杯故障状态掩膜
	uint32_t	cache3ErrorMask;						//包装通道1~20第3层缓存蛋杯故障状态掩膜
	uint32_t	moveCacheConnectErrorMask;	//包装通道1~20移动缓存伺服通讯故障状态掩膜
	uint32_t	moveCacheDeviceErrorMask;		//包装通道1~20移动缓存伺服设备故障状态掩膜
	uint32_t	moveCacheCupErrorMask;			//包装通道1~20移动缓存蛋杯开闭故障状态掩膜
	uint32_t	acceptConnectErrorMask;			//包装通道1~20接受抓伺服通讯故障状态掩膜
	uint32_t	acceptDeviceErrorMask;			//包装通道1~20接受抓伺服设备故障状态掩膜
	uint32_t	acceptClearErrorMask;				//包装通道1~20接受抓蛋夹中有鸡蛋未放掉状态掩膜
	uint32_t	conveyConnectErrorMask;		 	//包装通道1~20蛋托输送伺服通讯故障状态掩膜
	uint32_t	conveyDeviceErrorMask;			//包装通道1~20蛋托输送伺服设备故障状态掩膜
	uint32_t	autoDispatchErrorMask;			//包装通道1~20自动发托机故障状态掩膜
	uint32_t	trayMissErrorMask;			 		//包装通道1~20蛋托缺失状态掩膜
	uint32_t	trayStackedErrorMask;				//包装通道1~20蛋托堆叠状态掩膜
	uint32_t	targetCompleteInfoMask; 		//包装通道1~20完成产量(只提示不停机)状态掩膜
} PackageAlarmMaskTypeDef;

extern void trigger_alarm(alarm_level_t level, AlarmTypeDef type, char *msg);

extern alarm_state_t get_alarm_action(void);

extern void system_alarm_process(void);			//报警触发和报警确认处理函数
	 
#endif
