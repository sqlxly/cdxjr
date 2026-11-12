#ifndef __DEVCONFIG_H__
#define __DEVCONFIG_H__

#include <rtthread.h>
#include "drv_button.h"

#define setbitv(x,y)  x|=(1<<y)   //位读写
#define clrbitv(x,y)  x&=~(1<<y) 
#define getbitv(x,y)  ((x) >> (y)&1)

#define	RELAY_NUM			 7   //7组继电器输出
#define	DCT_NUM				24   //24组电磁铁输出
#define	DCF_NUM				20   //20组24V开闭控制 包括13组气缸开闭 1组发托控制 1组主输送线控制 5组备用
#define SERVO_MAX			 3   //3台伺服驱动器

#define TURN_FORWORD	 0   //定义为1-A通道  定义为0-B通道t

typedef union
{                                
		uint8_t chip1_data;    
		uint8_t chip2_data;   
		uint8_t chip3_data;   
		uint8_t chip4_data;   
		uint8_t chip5_data;   
} InputRegister;

//定义伺服电机状态机
enum eElectricCylinderState
{
  eCylinderIdle  		= 3000U,   	//空闲状态
  eCylinderOpen   	= 3001U,		//开启电机
	eCylinderClose  	= 3002U,		//关闭电机
	eCylinderError 	  = 3003U,		//电机异常
};

// 通过SN74HC165移位寄存器输入的端口定义
enum EnumInputPort {
	// MCU_IN1~MCU_IN8  74HC165-1P 8-ch
	eJJdin1 = 0, 		//1层1列蛋杯开闭状态 
	eJJdin2 = 1, 		//1层2列蛋杯开闭状态
	eJJdin3 = 2, 		//1层3列蛋杯开闭状态
	eJJdin4 = 3, 		//1层4列蛋杯开闭状态
	eJJdin5 = 4, 		//2层1列蛋杯开闭状态
	eJJdin6 = 5, 		//2层2列蛋杯开闭状态
	eJJdin7 = 6, 		//2层3列蛋杯开闭状态
	eJJdin8 = 7, 		//2层4列蛋杯开闭状态
	
	// MCU_IN9~MCU_IN16 74HC165-2P 8-ch
	eJJdin9  = 8,  	//3层1列蛋杯开闭状态
	eJJdin10 = 9,  	//3层2列蛋杯开闭状态
	eJJdin11 = 10, 	//3层3列蛋杯开闭状态
	eJJdin12 = 11, 	//3层4列蛋杯开闭状态
	eJJdin13 = 12, 	//移动缓存蛋杯开闭状态
	eJJdin14 = 13, 	//移动缓存接蛋位置1
	eJJdin15 = 14, 	//移动缓存接蛋位置2
	eJJdin16 = 15, 	//移动缓存接蛋位置3
	
	// MCU_IN17~MCU_IN21 74HC165-3P 5-ch
	eJJdin17 = 16, 	//移动缓存接蛋位置4
	eJJdin18 = 17, 	//移动缓存放蛋位置 即正对接受抓位置
	eJJdin19 = 18, 	//接受抓蛋杯开闭状态
	eJJdin20 = 19, 	//接受抓控制电机转一圈的位置检测接近开关
	eJJdin21 = 20, 	//输送链条主轴旋转一圈的位置检测接近开关 转一圈更换一个容器
	
	// MCU_IN22~MCU_IN29 74HC165-4P 8-ch
	eMFSdin1 = 24, 	//检测托盘缺蛋检测1 
	eMFSdin2 = 25, 	//检测托盘缺蛋检测2
	eMFSdin3 = 26, 	//检测托盘缺蛋检测3
	eMFSdin4 = 27, 	//检测托盘缺蛋检测4
	eMFSdin5 = 28, 	//检测托盘缺蛋检测5
	eMFSdin6 = 29, 	//检测托盘缺蛋检测6
	eMFSdin7 = 30, 	//检测托盘缺蛋检测7
	eFSdin1  = 31, 	//接受抓空蛋杯检测反射传感器 检测接受抓中的蛋是否都放入到托盘
	
	// MCU_IN30~MCU_IN37 74HC165-5P 8-ch
	eFSdin2 = 32,		//空托检测反射型传感器 检测链条上是否缺失蛋托 
	eFSdin3 = 33, 	//对接自动发托机 用于蛋托检测的反射传感器
	eMMdin  = 34, 	//包装通道安全盖板开关
	eXCdin1 = 35, 	//左行程限位开关
	eXCdin2 = 36, 	//右行程限位开关
	eANdin1 = 37, 	//整机急停按钮 接主控上
	eANdin2 = 38, 	//整机启动按钮 接主控上
	eANdin3 = 39, 	//通道复位按钮
	
	// MCU_IN38~MCU_IN42 74HC165-6P 5-ch
	eANdin4 = 40, 	//通道动作按钮 按一次放一次蛋 长按通道动作2秒则执行清空缓存的操作
	eANdin5 = 41, 	//通道清空按钮 取消
	eANdin6 = 42,		//堆料检测传感器 堆料后延时报警
	eANdin7 = 43,
	eANdin8 = 44,
	
	eUnknow = 48		//未定义
};

// 电磁铁输出控制端口定义
enum EnumDCTOutputPort {
	eDCTout1  = 0,		//主轨道1电磁铁1
	eDCTout2  = 1,		//主轨道1电磁铁2
	eDCTout3  = 2,		//主轨道1电磁铁3
	eDCTout4  = 3,		//主轨道1电磁铁4
	eDCTout5  = 4,		//主轨道1电磁铁5
	eDCTout6  = 5,		//主轨道1电磁铁6
	
	eDCTout7  = 6,		//主轨道2电磁铁1
	eDCTout8  = 7,		//主轨道2电磁铁2
	eDCTout9  = 8,		//主轨道2电磁铁3
	eDCTout10 = 9,		//主轨道2电磁铁4
	eDCTout11 = 10,		//主轨道2电磁铁5
	eDCTout12 = 11,		//主轨道2电磁铁6
	
	eDCTout13 = 12,		//主轨道3电磁铁1
	eDCTout14 = 13,		//主轨道3电磁铁2
	eDCTout15 = 14,		//主轨道3电磁铁3
	eDCTout16 = 15,		//主轨道3电磁铁4
	eDCTout17 = 16,		//主轨道3电磁铁5
	eDCTout18 = 17,		//主轨道3电磁铁6
	
	eDCTout19 = 18,		//主轨道4电磁铁1
	eDCTout20 = 19,		//主轨道4电磁铁2
	eDCTout21 = 20,		//主轨道4电磁铁3
	eDCTout22 = 21,		//主轨道4电磁铁4
	eDCTout23 = 22,		//主轨道4电磁铁5
	eDCTout24 = 23,		//主轨道4电磁铁6
	eDCTunknow = 24
};

// 气缸开闭控制端口定义
enum EnumDCFOutputPort {
	eDCFout1  = 0,	//1层1列蛋杯开闭气缸控制
	eDCFout2  = 1,	//1层2列蛋杯开闭气缸控制
	eDCFout3  = 2,	//1层3列蛋杯开闭气缸控制
	eDCFout4  = 3,	//1层4列蛋杯开闭气缸控制
	
	eDCFout5  = 4,  //2层1列蛋杯开闭气缸控制
	eDCFout6  = 5,	//2层2列蛋杯开闭气缸控制
	eDCFout7  = 6,  //2层3列蛋杯开闭气缸控制
	eDCFout8  = 7,	//2层4列蛋杯开闭气缸控制
	
	eDCFout9  = 8,	//3层1列蛋杯开闭气缸控制
	eDCFout10 = 9,	//3层2列蛋杯开闭气缸控制
	eDCFout11 = 10,	//3层3列蛋杯开闭气缸控制
	eDCFout12 = 11,	//3层4列蛋杯开闭气缸控制
	
	eDCFout13 = 12, //第4层移动缓存蛋杯开闭气缸控制
	
	eXPSSout1 = 13, //斜坡输送电机控制
	eFTDJout2 = 14, //发托机电机控制
	
	eDCFunknow = 15
};

// 继电器输出控制端口定义
enum EnumRLYOutputPort {
	eRLYout1 = 0,   //控制发托三相电机
	eRLYout2 = 1,   //控制网链输送电机
	eRLYout3 = 2,   //备用
	eRLYout4 = 3,		//备用
	eRLYout5 = 4,		//备用
	eRLYout6 = 5,		//备用
	eRLYout7 = 6,		//备用
	eRLYunknow = 7
};

//定义主状态
enum EnumMainStateManchine {
	eIdleState		= 1,    //空闲状态
	eRunState 		= 2,   	//运行状态
	eStopState 		= 3,   	//暂停状态
	eControlState = 4,    //手动控制状态
	eEditState		= 5,    //新建包装控制状态
	eUnknowState 	= 6			//未知状态
};

//定义包装通道状态机
enum EnumBranchStateManchine {
	eBranchIdleState 		= 20,
	eResetState 				= 21,   //通道复位	控制伺服电机复位
	eActiveState 				= 22,		//通道动作  控制接受抓与蛋托输送电机动作 按一次完成一次放蛋动作
	eClearState 				= 23,		//通道清空  清空缓存中的所有鸡蛋 全部装入蛋托
	eClearAllState			= 24,		//通道清空  强制清空所有缓存
	eBranchUnknowState 	= 25		//未知状态
};

//定义包装通道按钮类型
enum EnumButtonType {
	keyUnknowBtn 	= 10,		//按键状态未知
	keyResetBtn 	= 13,   //复位键按下
	keyActiveBtn 	= 14,		//通道动作键按下
	keyClearBtn 	= 15,		//通道清空键按下
	keyClearAllBtn = 16		//强制清空所有缓存内的鸡蛋
};

typedef struct
{
	uint8_t sta;					/* 阶梯缓存状态机 */
	uint8_t flag;    			/* 阶梯缓存控制标志: 0 - 阶梯缓存已复位并且蛋杯为空
																						 1 - 阶梯缓存中有鸡蛋
																						 2 - 正在操作气缸打开
																						 3 - 正在操作气缸关闭
																						 4 - 气缸故障
												*/
	uint8_t count;	  		/* 操作气缸打开后需要计时 */
	uint8_t countEx;
} JTHCOperateTypeDef;

typedef struct
{
	uint8_t sta;					/* 移动缓存状态机 */
	uint8_t flag;    			/* 移动缓存控制标志: 0 - 移动缓存已复位并且蛋杯为空
																						 1 - 移动缓存中有鸡蛋
																						 2 - 正在操作气缸打开
																						 3 - 正在操作气缸关闭
																						 4 - 移动缓存故障
												*/
	uint8_t mark; 				/* 移动缓存的位置目标位置 */
	int32_t speed;				/* 电机移动速度 */
	uint8_t count;	  		/* 操作气缸打开后需要计时 */
	uint8_t countEx;
	uint8_t ydhc_rst;			/* 移动缓存复位标志 */
} YDHCOperateTypeDef;

typedef struct
{
	uint8_t sta;				/* 接受抓状态机 */
	uint8_t flag;    		/* 接受抓控制标志: 0 - 接受抓已复位并且蛋杯为空
																				 1 - 接受抓中有鸡蛋
																				 2 - 正在旋转至脱离复位位置 
																				 3 - 正在旋转至复位位置 
																				 4 - 接受抓故障 
											*/
	int32_t speed;			/* 接受抓电机移动速度 */
//	int32_t moveLength; /* 伺服移动多少个脉冲 用于脉冲超限时的故障检测 */
	uint8_t jsz_rst;		/* 接收抓复位标志 */
	uint8_t count;	  	/* 接收抓复位超时计数 */
} JSZOperateTypeDef;

typedef struct
{
	uint8_t sta;				/* 输送通道状态机 */
	uint8_t flag;    		/* 输送通道控制标志: 0 - 托盘移动到下一个放蛋位置 蛋托为空 可以放入鸡蛋
																				   1 - 托盘中有鸡蛋
																				   2 - 正在旋转至下一个放蛋位置
																				   4 - 输送通道故障 
											*/
	uint8_t type;				/* 蛋格类型 如5x6 2x5等 */
	int32_t speed;			/* 输送通道电机移动速度 */
	uint8_t sstd_rst;		/* 输送通道复位标志 */
	uint8_t autoTrayFlag; /* 到达发托位置 需要自动发托机发送蛋托 */
	uint8_t autoTrayEnable; /* 使能自动发托机 */
} SSTDOperateTypeDef;

typedef struct {
	uint8_t staBak;					//记录输入端口前一次的电平状态 用于脉冲上升沿检测
	int32_t zero;						//检测到脉冲上升沿 记录伺服电机起始位置
	int32_t distance;				//根据起始位置和当前位置 计算出相对距离
} ServoStateInfo;	

typedef struct {
	Button_t btn;						//定义按钮结构体
	uint8_t disable;				//是否禁用此按钮 例如在运行状态下禁用复位、清空、通道动作等按钮
	uint8_t btnWait;  			//按钮预执行状态 例如运行状态下按暂停 需要等待通道逻辑执行完成再停机
} ButtonOperateTypeDef;

/*** 定义Ethercat主站发送给单个从站的数据协议 ***/
enum EnumMasterCommand {
	eMasterRunState 			= 0x5501,		//自动运行命令 自动控制设备动作
	eMasterStopState			= 0x5502,   //设备停止命令
	eMasterControlState		= 0x5503,		//手动控制命令
	eMasterEditState			= 0x5504,		//新建包装控制命令
	eMasterSndParamState1 = 0x5505,   //设置参数1的命令 主控设置包装通道参数
	eMasterSndParamState2	= 0x5506,		//设置参数2的命令 主控设置包装通道参数
	eMasterSndParamState3 = 0x5507,		//设置参数3的命令 主控设置包装通道参数
	eMasterSndUnknow			= 0x55ff		//未知命令
};

//启动状态下发送的数据结构
typedef struct {
	uint16_t cmd;								//命令码 0x5501
	uint8_t dctLine1	:6;				//1排电磁铁开关控制 6bits
	uint8_t dbFull1		:2;				//1排电磁铁下面的蛋杯都放满了(0为空 1为有蛋未满 2为放满) 发送高电平脉冲信号 
	uint8_t dctLine2	:6;				//2排电磁铁开关控制 6bits
	uint8_t dbFull2		:2;				//2排电磁铁下面的蛋杯都放满了(0为空 1为有蛋未满 2为放满) 发送高电平脉冲信号
	uint8_t dctLine3	:6;				//3排电磁铁开关控制 6bits
	uint8_t dbFull3		:2;				//3排电磁铁下面的蛋杯都放满了(0为空 1为有蛋未满 2为放满) 发送高电平脉冲信号
	uint8_t dctLine4	:6;				//4排电磁铁开关控制 6bits
	uint8_t dbFull4		:2;				//4排电磁铁下面的蛋杯都放满了(0为空 1为有蛋未满 2为放满) 发送高电平脉冲信号
	uint8_t speed;							//分级机运行速度
} AcceptMasterMessageWork;

//停机状态下发送的数据结构
typedef struct {
	uint16_t cmd;								//命令码 0x5502
	uint8_t alarmConfirm :1;		//报警确认后 清除报警信息
	uint8_t	bak					 :7;		//备用
} AcceptMasterMessageStop;

//测试校准时 手动控制从站的数据结构
typedef struct {
	uint16_t cmd;								//命令码 0x5503 / 0x5504
	uint8_t dctLine1	:6;				//1排电磁铁点动开关控制 6bits
	uint8_t	bak1			:2;
	uint8_t dctLine2	:6;				//2排电磁铁点动开关控制 6bits
	uint8_t	bak2			:2;
	uint8_t dctLine3	:6;				//3排电磁铁点动开关控制 6bits
	uint8_t	bak3			:2;
	uint8_t dctLine4	:6;				//4排电磁铁点动开关控制 6bits
	uint8_t	bak4			:2;
	
	uint16_t jthcCtrl	:12;			//阶梯缓存气缸控制 3层x4列 = 12bits
	uint16_t toOrigin	:1;				//新建包装--回原点
	uint16_t toNext		:1;				//新建包装--下一步
	uint16_t jthcBak	:2;				
	
	uint16_t ftjAct		:2;				//发托机点动开关
	uint16_t dtssAct	:1;				//蛋托输送 运行1/停止0
	uint16_t dtssTurn	:2;				//蛋托输送 正转1/反转2/无效0
	uint16_t jszAct		:1;				//接收爪 运行1/停止0
	uint16_t jszTurn	:2;				//接收爪 正转1/反转2/无效0
	uint16_t ydhcAct  :1;				//移动缓存 运行1/停止0
	uint16_t ydhcTurn	:2;				//移动缓存 正转1/反转2/无效0
	uint16_t ydhcFlex	:2;				//移动缓存 伸出1/收缩2/无效0
	uint16_t xpssAct	:2;				//斜坡输送电机开关控制
	uint16_t bak			:1;				//备用
} AcceptMasterControl;
//从站接收配置参数1
typedef struct {
	uint16_t cmd;								//命令码 0x5505
	uint8_t row;								//包装盒行数
	uint8_t col;								//包装盒列数
	uint16_t autoTray;					//自动派托位置
	uint16_t trayCheck; 				//蛋托检测位置(包装盒检测位置)
	uint16_t transportStart;		//输送启动位置(接受爪上升到何处位置时链条开始启动)
	uint16_t packBoxPost1;			//1排落蛋位置
	uint16_t packBoxPost2;			//2排落蛋位置
} AcceptMasterMessageParam1;
//从站接收配置参数2
typedef struct {
	uint16_t cmd;								//命令码 0x5506
	uint16_t packBoxPost3;			//3排落蛋位置
	uint16_t packBoxPost4;			//4排落蛋位置
	uint16_t packBoxPost5;			//5排落蛋位置
	uint16_t packBoxPost6;			//6排落蛋位置
	uint16_t packBoxPost7;			//7排落蛋位置
	uint16_t packBoxPost8;			//8排落蛋位置
	uint16_t chainLinkCnt;			//占用装托链条格数
} AcceptMasterMessageParam2;

//发送配置参数3到从机
typedef struct {
	uint16_t cmd;									//命令码 0x5507
	uint16_t bak1;
	uint16_t bak2;
	uint16_t bak3;
	uint16_t bak4;
	uint16_t bak5;
	uint16_t bak6;
	uint16_t alarmConfirm		:1;		//报警确认按钮 或者按开机启动一键确认
	uint16_t bak						:15;
} AcceptMasterMessageParam3;

/*** 定义Ethercat主站接收单个从站的数据协议 ***/
typedef struct {
	uint16_t alarmCache;									//缓存蛋杯开闭故障 三层缓存 每层4个传感器 共12个传感器

	uint16_t alarmMoveServoConenct	:1;		//移动缓存伺服通信故障
	uint16_t alarmMoveServodevice		:1;		//移动缓存伺服设备故障
	uint16_t alarmMoveCupError			:1;		//移动缓存蛋杯开闭故障
	uint16_t alarmAcceptServoConnect:1;		//接收抓伺服通信故障
	uint16_t alarmAcceptServoDevice	:1;		//接收抓伺服设备故障
	uint16_t alarmAcceptCupError		:1;		//接收抓弹夹中有鸡蛋未放掉
	uint16_t alarmConveyServoConnect:1;		//蛋托输送伺服通信故障
	uint16_t alarmConveyServoDevice	:1;		//蛋托输送伺服设备故障
	uint16_t alarmAutoDispatchError	:1;		//自动发托机故障
	uint16_t alarmTrayMiss					:1;		//蛋托缺失
	uint16_t alarmTrayStacke				:1;		//蛋托堆叠
	uint16_t alarmBak								:5;		//备用
}AlarmMarkTypeDef;

//接收到单个从站的状态信息
typedef struct {
	uint16_t cmd;											//命令响应 根据主机发送的命令码 返回命令码 格式0xAAxx 其中xx与发送命令的最后两位相同
	uint16_t jthcSta		:12;					//阶梯缓存蛋杯状态 3层x4列 = 12bits
	uint16_t ftjPost		: 1;					//发托机到位状态
	uint16_t dtss_dt		: 1;     			//蛋托输送 蛋托检测
	uint16_t dtss_dl		: 1;     			//蛋托输送 堆料检测
	uint16_t dtss_lw		: 1;     			//蛋托输送 零位检测

	uint16_t jsz_dw			: 1;					//接受抓 接受抓到位
	uint16_t jsz_dj			: 1;					//接受抓 蛋夹关闭
	uint16_t jsz_db			: 1;					//接受抓 蛋杯检测
	uint16_t ydhc_dj		:	1;					//移动缓存 蛋夹关闭
	uint16_t ydhc_lw		:	1;					//移动缓存 零位检测
	uint16_t ydhc_xw1		:	1;					//移动缓存 限位开关1
	uint16_t ydhc_xw2		:	1;					//移动缓存 限位开关2
	uint16_t ydhc_jdw1	:	1;					//移动缓存 1号接蛋位检测
	uint16_t ydhc_jdw2	:	1;					//移动缓存 2号接蛋位检测
	uint16_t ydhc_jdw3	:	1;					//移动缓存 3号接蛋位检测
	uint16_t ydhc_jdw4	:	1;					//移动缓存 4号接蛋位检测
	uint16_t jthc_gd1		: 1;					//轨道1下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd2		: 1;					//轨道2下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd3		: 1;					//轨道3下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd4		: 1;					//轨道4下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t clearFlag	: 1;					//通道清空标志

	uint16_t 	sstdServoPost;					//蛋托输送私服电机位置
	uint16_t 	 jszServoPost;					//接受抓私服电机位置
	
	AlarmMarkTypeDef 	alarm;					//添加故障报警信息
} SlaveSendMessage;

typedef struct {
	uint8_t row;											//包装盒行数
	uint8_t col;											//包装盒列数
	uint16_t autoTray;								//自动派托位置
	uint16_t trayCheck; 							//蛋托检测位置(包装盒检测位置)
	uint16_t transportStart;					//输送启动位置(接受爪上升到何处位置时链条开始启动)
	uint16_t packBoxPost1;						//1排落蛋位置
	uint16_t packBoxPost2;						//2排落蛋位置
	uint16_t packBoxPost3;						//3排落蛋位置
	uint16_t packBoxPost4;						//4排落蛋位置
	uint16_t packBoxPost5;						//5排落蛋位置
	uint16_t packBoxPost6;						//6排落蛋位置
	uint16_t packBoxPost7;						//7排落蛋位置
	uint16_t packBoxPost8;						//8排落蛋位置
	uint16_t chainLinkCnt;						//占用装托链条格数
} PackageChannelWorkParam;

// 包装通道设备操作相关参数
typedef struct {
	enum EnumButtonType keyBtn;
		
	uint8_t inputData[6];

	uint32_t dcfState;
	
	JTHCOperateTypeDef jthcOP[13];
	
	YDHCOperateTypeDef ydhcOP;
	
	JSZOperateTypeDef	 jszOP;
	
	SSTDOperateTypeDef sstdOP;
	
	PackageChannelWorkParam workParam;
	
} ChanbelOperateTypeDef;

typedef struct {
	enum eElectricCylinderState eState;
	uint8_t uStateEx;
} CylinderStateTypeDef;

extern enum EnumMainStateManchine mainState;
extern enum EnumMainStateManchine mainStateNext;
extern enum EnumBranchStateManchine branchState;

extern CylinderStateTypeDef cylinderState[SERVO_MAX];
extern AcceptMasterControl mCtrl;

extern ServoStateInfo sstd_servo;
extern ServoStateInfo jsz_servo;

extern AlarmMarkTypeDef alarmMark;					//包装通道报警标志位

uint8_t get_input_port_sta(uint8_t *iData, enum EnumInputPort e);

void clear_alarm_mark(void);

#endif
