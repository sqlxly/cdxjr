#ifndef __DEV_CONFIG_H__
#define __DEV_CONFIG_H__

#include <rtthread.h>

#define setbitv(x,y)  x|=(1<<y)   //位读写
#define clrbitv(x,y)  x&=~(1<<y) 
#define getbitv(x,y)  ((x) >> (y)&1)

#define	RELAY_NUM			 					7   	//7组继电器输出

#define	OUT_NUM									36   	//36组24V开闭控制

#define SORTER_USED_MAX					20		//最多20个分级通道

#define PER_CUP_WIDTH						63.5  //通过测量得到单个蛋杯宽度63.5mm

#define PER_CUP_PLUSE						167  	//定义每个蛋杯的脉冲宽度 就是两个蛋杯接近开关信号之间的脉冲计数值 AB相编码器 转一圈为2*1000P/R

#define DCT_AHEAD_MAX_PLUSE			225		//电磁铁最大提前位置转换成脉冲数 及电动推缸缩回位置转换成脉冲数

#define	PER_EXTERNAL_SYNC_PLUSE 500		//外部设备 如称重 裂纹 脏蛋等信号 每一排鸡蛋 对应主控编码器的脉冲间隔是500

//定义主控状态机
enum MainMotorStatus
{
	eMotorIdle  				= 2000U,  //空闲状态
	eMotorRun   				= 2001U,	//正常运行状态
	eMotorStop  				= 2002U,	//停机状态
	eMotorAlarm 				= 2003U,  //故障报警状态
	eMotorUnknowState 	= 2004U		//未知状态
};

//定义系统运行状态机
enum SystemWorkStatus
{
	eSystemIdle  				= 7000U,  //空闲状态
	eSystemRun   				= 7001U,	//正常运行状态
	eSystemStop  				= 7002U,	//停机状态
	eSystemReady  			= 7003U,	//启动准备状态
	eSystemReset 				= 7004U,  //复位状态
	eSystemAlarm 				= 7005U,  //故障报警状态
	eChannelStep 				= 7006U,	//包装通道手动控制
	ePipelineStep				= 7007U,	//输送线调试 手动控制状态
	eExtensionLineStep	= 7008U,	//分级线调试 手动控制状态
	ePackageEdit				= 7009U,	//包装管理中新建或者编辑包装盒的动作测试控制
	eSystemUnknowState 	= 7010U		//未知状态
};

//定义电动推缸状态机
enum eElectricCylinderState
{
	eCylinderIdle  		= 3000U,   	//空闲状态
	eCylinderOpen   	= 3001U,		//开启电动推缸
	eCylinderClose  	= 3002U,		//关闭电动推缸
	eCylinderError 	  = 3003U,		//电动推缸异常
};

//定义包装通道按钮类型
enum EnumButtonType {
	keyUnknowBtn 	= 30,		//按键状态未知
	keyRunBtn 		= 31,   //运行/暂停 键按下
	keyRunBtnUp		= 32,		//按钮释放
	keyStopBtn 		= 33,   //暂停键按下 急停
	keyResetBtn 	= 34   	//复位键按下
};

//异常报警处理
enum EnumAlarmType {
	alarmUnknow 			= 1000,	//未知报警
	alarmNormal				= 1001,	//正常状态
	alarmStopDevice 	= 1002, //停机报警
	alarmPromptMsg 		= 1003  //提示报警 不停机
};

// 通过SN74HC165移位寄存器输入的端口定义
enum EnumInputIO {
	// MCU_IN1~MCU_IN8  74HC165-1P 8-ch
	eInputIO1 = 4, 	 // 光检位置 整机启动按钮 与其它点位的整机启动按钮并在一起
	eInputIO2 = 5, 	 //	光检位置 整机急停按钮 与其它点位的整机急停按钮并在一起(按下急停开关是否可以把信号传给控制器)
	eInputIO3 = 6, 	 // 通道偏离装置 鸡蛋脱离通道辊条位置 会触碰偏离装置 此后整机停机并显示报警
	eInputIO4 = 7, 	 //	定向转载输送 接蛋检测 确保鸡蛋掉入蛋杯中  对射传感器
	eInputIO5 = 3, 	 // 定向转载输送 链条张紧检测 接近开关
	eInputIO6 = 2, 	 // 蛋杯收缩变距到位检测1	四条主轨道预留4个接近开关
	eInputIO7 = 1, 	 // 蛋杯收缩变距到位检测2
	eInputIO8 = 0, 	 // 蛋杯收缩变距到位检测3

	// MCU_IN9~MCU_IN16 74HC165-2P 8-ch
	eInputIO9  = 12, // 蛋杯收缩变距到位检测4
	eInputIO10 = 13, // 主轨道组件 链条张紧检测
	eInputIO11 = 14, // 主轨道组件 主蛋杯接蛋检测1 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中
	eInputIO12 = 15, // 主轨道组件 主蛋杯接蛋检测2 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中
	eInputIO13 = 11, // 主轨道组件 主蛋杯接蛋检测3 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中
	eInputIO14 = 10, // 主轨道组件 主蛋杯接蛋检测4 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中
	eInputIO15 = 9,  // 主轨道组件 1号蛋杯组检测 主轨道蛋杯循环一圈 刷新一次蛋杯标定数据
	eInputIO16 = 8,  // 主轨道组件 主蛋杯标定接近开关 每个蛋杯一个脉冲

	// MCU_IN17~MCU_IN21 74HC165-3P 8-ch
	eInputIO17 = 20, // 主轨道组件 异常蛋统计检测2
	eInputIO18 = 21, // 主轨道组件 异常蛋统计检测3
	eInputIO19 = 22, // 主轨道组件 异常蛋统计检测4
	eInputIO20 = 23, // 主轨道组件 蛋杯闭合到位检测1
	eInputIO21 = 19, // 主轨道组件 蛋杯闭合到位检测2
	eInputIO22 = 18, //	主轨道组件 蛋杯闭合到位检测3
	eInputIO23 = 17, // 主轨道组件 蛋杯闭合到位检测4
	eInputIO24 = 16, // 主轨道组件 异常蛋统计检测1 共4个漫反射开关

	// MCU_IN22~MCU_IN29 74HC165-4P 8-ch
	eInputIO25 = 28, // 主电机链条同步检测
	eInputIO26 = 29, // 从电机链条同步检测
	eInputIO27 = 30, // 裂纹输送A 安全门开关
	eInputIO28 = 31, // 裂纹输送B 安全门开关
	eInputIO29 = 27, // 主控复位按钮信号
	eInputIO30 = 26, // 主控转载托盘复位原点信号
	eInputIO31 = 25, // 抛弃堆蛋检测
	eInputIO32 = 24, //

	// MCU_IN30~MCU_IN37 74HC165-5P 8-ch
	eInputIO33 = 36, //
	eInputIO34 = 37, //
	eInputIO35 = 38, //
	eInputIO36 = 39, //
	eInputIO37 = 35, //
	eInputIO38 = 34, //
	eInputIO39 = 33, // 电动推缸缩回限位
	eInputIO40 = 32, // 电动推缸伸出限位

	eInputUnknow = 50 //未定义
};

// IO输出控制端口定义
enum EnumOutputIO {
	//Output1 group
	eOutputIO1  = 40,		// 主轨道组件 喷码机1选择1
	eOutputIO2  = 41,		// 主轨道组件 喷码机1选择2
	eOutputIO3  = 42,		// 主轨道组件 喷码机1选择3
	eOutputIO4  = 43,		// 主轨道组件 喷码机1选择4
	eOutputIO5  = 44,		// 主轨道组件 喷码机2选择1
	eOutputIO6  = 45,		// 主轨道组件 喷码机2选择2
	eOutputIO7  = 46,		// 主轨道组件 喷码机2选择3
	eOutputIO8  = 47,		// 主轨道组件 喷码机2选择4
	eOutputIO9  = 32,		// 主轨道组件 喷码机3选择1
	eOutputIO10 = 33,		// 主轨道组件 喷码机3选择2
	eOutputIO11 = 34,		// 主轨道组件 喷码机3选择3
	eOutputIO12 = 35,		// 主轨道组件 喷码机3选择4
	                    
	//Output2 group     
	eOutputIO13 = 36,		// 主轨道组件 喷码机4选择1
	eOutputIO14 = 37,		// 主轨道组件 喷码机4选择2
	eOutputIO15 = 38,		// 主轨道组件 喷码机4选择3
	eOutputIO16 = 39,		// 主轨道组件 喷码机4选择4
	eOutputIO17 = 24,		// 主轨道组件 喷码机喷码使能 可同时使能4台喷码机
	eOutputIO18 = 25,		// 漏液蛋踢出气缸1
	eOutputIO19 = 26,		// 漏液蛋踢出气缸2
	eOutputIO20 = 27,		// 漏液蛋踢出气缸3
	eOutputIO21 = 28,		// 漏液蛋踢出气缸4
	eOutputIO22 = 29,		// 漏液蛋踢出气缸5
	eOutputIO23 = 30,		// 漏液蛋踢出气缸6
	eOutputIO24 = 31,		// 漏液蛋踢出气缸7

	//Output3 group
	eOutputIO25 = 16,		// 漏液蛋踢出气缸8
	eOutputIO26 = 17,		// 漏液蛋踢出气缸9
	eOutputIO27 = 18,		// 漏液蛋踢出气缸10
	eOutputIO28 = 19,		// 漏液蛋踢出气缸11
	eOutputIO29 = 20,		// 漏液蛋踢出气缸12
	eOutputIO30 = 21,		// 脏蛋踢出气缸1
	eOutputIO31 = 22,		// 脏蛋踢出气缸2
	eOutputIO32 = 23,		// 脏蛋踢出气缸3
	eOutputIO33 = 8,		// 脏蛋踢出气缸4
	eOutputIO34 = 9,		// 脏蛋踢出气缸5
	eOutputIO35 = 10,		// 脏蛋踢出气缸6
	eOutputIO36 = 11,		// 脏蛋踢出气缸7

	eOutputIO37 = 12,		// 脏蛋踢出气缸8
	eOutputIO38 = 13,		// 脏蛋踢出气缸9
	eOutputIO39 = 14,		// 脏蛋踢出气缸10
	eOutputIO40 = 15,		// 脏蛋踢出气缸11
	eOutputIO41 = 0,		// 脏蛋踢出气缸12
	eOutputIO42 = 1,		// 预留7路输出
	eOutputIO43 = 2,
	eOutputIO44 = 3,
	eOutputIO45 = 4,
	eOutputIO46 = 5,
	eOutputIO47 = 6,
	eOutputIO48 = 7,
	
	eOutputUnknow = 0xff	// 未定义
};

// 继电器输出控制端口定义
enum EnumRelayIO {
	//Output1 group
	eRelayIO1 = 0,		// 光检位置 自动控制照明灯管通电断电 (手动)
	eRelayIO2 = 1,		// 紫外消毒 自动控制消毒灯管通断电 有启停延时
	eRelayIO3 = 2,		// 喷油位置	1个电磁阀控制气源动力的通断  1.整机启动时 油雾机先抽气 油泵再启动; 2.整机停止工作超过3分钟后 油泵再停止 再过15秒后油雾机停止 如果延时时间内再次启动机器就持续工作
	eRelayIO4 = 3,		// 喷油位置	1台油雾机收集多余油雾	
	eRelayIO5 = 4,		// 抛弃通道输送滚条电机
	eRelayIO6 = 5,		// 开机后继电器6先闭合 给变频器从机上电
	eRelayIO7 = 6,		// 等待变频器从机启动完成 再给主机上电(继电器7)
	
	eRelayUnknow = 8	// 未定义
};

// 声光报警灯控制枚举变量
enum EnumLedAndBeep {
	eYellow = 10,		// 黄色
	eGreen = 11,		// 绿色
	eRed 		= 12,		// 红色
	eUnknowColor = 100	// 未定义颜色
};

typedef struct {
	uint16_t baseAddr;
	uint16_t len;
} MBReceiveType;


/*** 定义Ethercat主站发送给单个从机的数据通信协议 ***/

enum EnumMasterCommand {
	eMasterRunState 				= 0x5501,	//自动运行命令 自动控制设备动作
	eMasterStopState				= 0x5502, //设备停止命令
	eMasterControlState			= 0x5503,	//手动控制命令
	eMasterEditState				= 0x5504,	//新建包装控制命令
	eMasterSndParamState1   = 0x5505, //设置参数1的命令 主控设置包装通道参数
	eMasterSndParamState2		= 0x5506,	//设置参数2的命令 主控设置包装通道参数
	eMasterSndParamState3   = 0x5507, //设置参数3的命令 主控设置包装通道参数
	eMasterSndUnknow				= 0x55ff	//未知命令
};

enum EnumMasterCommandRsp {
	eMasterRunStateRsp 				= 0xAA01,	//自动运行命令应答
	eMasterStopStateRsp				= 0xAA02, //设备停止命令应答
	eMasterControlStateRsp		= 0xAA03,	//手动控制命令应答
	eMasterEditStateRsp				= 0xAA04,	//新建包装控制命令应答
	eMasterSndParamState1Rsp 	= 0xAA05, //设置参数1的命令应答
	eMasterSndParamState2Rsp	= 0xAA06,	//设置参数2的命令应答
	eMasterSndParamState3Rsp	= 0xAA07,	//设置参数2的命令应答
	eMasterSndUnknowRsp				= 0xAAff	//未知应答
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
} MasterSendMessageWork;

//停机状态下发送的数据结构
typedef struct {
	uint16_t cmd;									//命令码 0x5502
	uint16_t bak1;
	uint16_t bak2;
	uint16_t bak3;
	uint16_t bak4;
	uint16_t bak5;
	uint16_t bak6;
	uint16_t alarmConfirm  :1;		//报警确认后 清除报警信息
	uint16_t bak					 :15;		//备用
} MasterSendMessageStop;

//测试校准时 手动控制从机的数据结构
typedef struct {
	uint16_t cmd;								//命令码 0x5503 / 0x5504
	uint8_t dctLine1	:6;				//1排电磁铁点动开关控制 6bits
	uint8_t	bak1		  :2;
	uint8_t dctLine2	:6;				//2排电磁铁点动开关控制 6bits
	uint8_t	bak2		  :2;
	uint8_t dctLine3	:6;				//3排电磁铁点动开关控制 6bits
	uint8_t	bak3		  :2;
	uint8_t dctLine4	:6;				//4排电磁铁点动开关控制 6bits
	uint8_t	bak4		  :2;
	
	uint16_t jthcCtrl	:12;			//阶梯缓存气缸控制 3层x4列 = 12bits
	uint16_t toOrigin	:1;				//新建包装--回原点
	uint16_t toNext		:1;				//新建包装--下一步
	uint16_t jthcBak	:2;				//备用			
	
	uint16_t ftjAct		:2;				//发托机点动开关
	uint16_t dtssAct	:1;				//蛋托输送 运行1/停止0
	uint16_t dtssTurn	:2;				//蛋托输送 正转1/反转2/无效0
	uint16_t jszAct		:1;				//接收爪 运行1/停止0
	uint16_t jszTurn	:2;				//接收爪 正转1/反转2/无效0
	uint16_t ydhcAct  :1;				//移动缓存 运行1/停止0
	uint16_t ydhcTurn	:2;				//移动缓存 正转1/反转2/无效0
	uint16_t ydhcFlex	:2;				//移动缓存 伸出1/收缩2/无效0
	uint16_t xpssAct	:2;				//斜坡输送电机开关控制
} MasterSendMessageControl;	

//发送配置参数1到从机
typedef struct {
	uint16_t cmd;								//命令码 0x5505
	uint8_t row;								//包装盒行数
	uint8_t col;								//包装盒列数
	uint16_t autoTray;					//自动派托位置
	uint16_t trayCheck; 				//蛋托检测位置(包装盒检测位置)
	uint16_t transportStart;		//输送启动位置(接受爪上升到何处位置时链条开始启动)
	uint16_t packBoxPost1;			//1排落蛋位置
	uint16_t packBoxPost2;			//2排落蛋位置
} MasterSendMessageParam1;
//发送配置参数2到从机
typedef struct {
	uint16_t cmd;								//命令码 0x5506
	uint16_t packBoxPost3;			//3排落蛋位置
	uint16_t packBoxPost4;			//4排落蛋位置
	uint16_t packBoxPost5;			//5排落蛋位置
	uint16_t packBoxPost6;			//6排落蛋位置
	uint16_t packBoxPost7;			//7排落蛋位置
	uint16_t packBoxPost8;			//8排落蛋位置
	uint16_t chainLinkCnt;			//占用装托链条格数
} MasterSendMessageParam2;

//发送配置参数3到从机
typedef struct {
	uint16_t cmd;									//命令码 0x5507
	uint16_t alarmConfirm		:1;		//报警确认按钮 或者按开机启动一键确认
	uint16_t bak						:15;
} MasterSendMessageParam3;

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
	uint16_t cmd;										//命令响应 根据主机发送的命令码 返回命令码 格式0xAAxx 其中xx与发送命令的最后两位相同
	uint16_t jthcSta		:12;				//阶梯缓存气缸状态 3层x4列 = 12bits
	uint16_t ftjPost		: 1;				//发托机到位状态
	uint16_t dtss_dt		: 1;     		//蛋托输送 蛋托检测
	uint16_t dtss_dl		: 1;     		//蛋托输送 堆料检测
	uint16_t dtss_lw		: 1;     		//蛋托输送 零位检测
		
	uint16_t jsz_dw			: 1;				//接受抓 接受抓到位
	uint16_t jsz_dj			: 1;				//接受抓 蛋夹关闭
	uint16_t jsz_db			: 1;				//接受抓 蛋杯检测
	uint16_t ydhc_dj		:	1;				//移动缓存 蛋夹关闭
	uint16_t ydhc_lw		:	1;				//移动缓存 零位检测
	uint16_t ydhc_xw1		:	1;				//移动缓存 限位开关1
	uint16_t ydhc_xw2		:	1;				//移动缓存 限位开关2
	uint16_t ydhc_jdw1	:	1;				//移动缓存 1号接蛋位检测
	uint16_t ydhc_jdw2	:	1;				//移动缓存 2号接蛋位检测
	uint16_t ydhc_jdw3	:	1;				//移动缓存 3号接蛋位检测
	uint16_t ydhc_jdw4	:	1;				//移动缓存 4号接蛋位检测
	uint16_t jthc_gd1		: 1;				//轨道1下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd2		: 1;				//轨道2下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd3		: 1;				//轨道3下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t jthc_gd4		: 1;				//轨道4下面的第一层阶梯缓存状态 空或者有蛋
	uint16_t clearFlag	: 1;				//通道清空标志
	
	uint16_t 	sstdServoPost;				//蛋托输送私服电机位置 转换成位置百分比后扩大10倍 
	uint16_t 	 jszServoPost;				//接受抓私服电机位置 伺服电机默认转一圈是100000PUU, 把数据缩小10倍传输
	
	AlarmMarkTypeDef 	alarm;				//添加故障报警信息
} AcceptSlaveMessage;

typedef struct {
	enum eElectricCylinderState eState;	//电动推缸状态
	uint8_t ctrlMode;										//电动推缸控制模式 1电动推缸手动控制 2电动推缸复位 3根据主电机速度自动匹配电动推缸位置
	uint8_t uStateEx;										//状态扩展 标识伺服启动过程
	int32_t interpolationSpeed;					//电动推缸速度
	int32_t interpolationPosition;			//电动推缸移动位置
	int32_t zeroPostion;								//伺服电机复位后的原点位置
	uint8_t runDirection;								//标记电机当前运转方向 正1 反2
} CylinderCtrlTypeDef;

struct MotorETA
{
    uint16_t bit0			   			 :1;  //通电准备就绪
    uint16_t bit1              :1;	//已通电
    uint16_t bit2              :1;	//运行被允许
    uint16_t bit3              :1;	//故障 检测到错误
    uint16_t bit4              :1;	//已加电
    uint16_t bit5              :1;	//快速停车
    uint16_t bit6              :1;	//通电被禁止
    uint16_t bit7              :1;	//警告被激活
    uint16_t bit8              :1;	//bak
    uint16_t bit9              :1;	//远程
    uint16_t bit10             :1;	//达到目标速度
    uint16_t bit11             :1;	//内部限值有效
    uint16_t bit12             :1;	//bak
    uint16_t bit13             :1;	//bak
    uint16_t bit14             :1;	//通过STOP键停车
    uint16_t bit15             :1;	//转动方向
};

typedef struct {
	/* 读取到的电机数据 */
	struct MotorETA m_ETA;	//主电机状态字
	int16_t m_RFRD;					//主电机输出转速
	int16_t m_LCR;					//主电机电流

	struct MotorETA s_ETA;	//从电机状态字
	int16_t s_RFRD;					//从电机输出转速
	int16_t s_LCR;					//从电机电流
	
	/* 需要写入电机的参数 */
	int16_t m_CMD;					//命令控制字
	int16_t m_LFRD;					//电机目标转速 转每分钟
	int16_t s_CMD;					//命令控制字
	int16_t s_LFRD;					//电机目标转速 转每分钟
	int16_t ACC;						//加速斜坡 单位0.1s
	int16_t DEC;						//减速斜坡 单位0.1s
} MotorParamTypeDef;

typedef struct {
	enum MainMotorStatus eState;//电机工作状态
	uint8_t takeTurn;						//电机启停状态切换 用于分级机调试时 主从电机的点动或者持续运行
	uint8_t stateEx;						//状态扩展 标识主从电机启动流程
	int16_t targetLFRD;					//电机目标速度
	uint8_t speedSmooth;				//平滑加减速 1--需要平滑调速 0--直接按照目标速度启动
	uint8_t motorState;					//电机状态 0--停止  1--运行
	uint16_t stopTimeout;				//电机暂停时需要计时  超时后需要关闭电机使能
}MotorOperateTypeDef;

// 电机状态结构体
typedef struct {
//  int32_t raw_encoder_count;   // 原始增量编码器累计脉冲数
	int32_t absolute_position;   // 经过校准的绝对位置(脉冲数)
	int32_t last_home_count;     // 上一次触发接近开关时的编码器计数值
	int8_t home_detected;        // 是否检测到过参考点
	double current_freq_hz;      // 当前运行频率
} MotorInformation;

//主控设备操作相关参数
typedef struct {
	enum SystemWorkStatus eSysState;//系统工作状态

	enum EnumButtonType keyBtn; 	 	//按钮状态

//	enum EnumAlarmType alarm;		 		//异常报警状态

	uint8_t sysStateEx;							//执行启动延时的状态机
	
	uint8_t mainMotorResetFlag;			//主电机上电复位完成标志
	
	uint8_t slaveMotorResetFlag;			//主电机上电复位完成标志

	uint8_t servoMotorResetFlag;		//电动推缸伺服电机复位完成标志

	uint8_t inputData[6];			 			//读取的输入端口电平

	uint8_t outputData[6];			 		//写入到输出端口的电平状态

	uint8_t relayData;				 			//继电器输出端口

	int16_t masterSetSpeed;					//当前系统设定运行速度
	
} MasterOperateTypeDef;

// 主轨道踢蛋和喷码位置触发的编码器脉冲记录
typedef struct {
	uint8_t state;						//踢蛋流程的状态机
	uint8_t act;							//蛋杯到位后 用于踢蛋开始的触发信号
	uint16_t orbitCupOffset;	//电动推缸相对于主轨道蛋杯偏移蛋杯个数
	int32_t pushrotStart;			//踢蛋开始时电动缸位置脉冲
	int32_t orbitStart;				//踢蛋开始时蛋杯偏移位置脉冲
	int32_t cPerCupPluse;  		//定义每个蛋杯的脉冲宽度 就是两个蛋杯接近开关信号之间的脉冲计数值
	int32_t currentEncoder; 	//当前蛋杯相对蛋杯开始位置移动了多少个脉冲(归一化数据)
	int32_t encoderCnt;				//每个蛋杯编码器脉冲计数
	int32_t tickEncoderAdd;		//踢蛋开始位置的编码器累加值 用于判断踢蛋结束位置
	int32_t encoderRepostAdd;	//转载缓存脉冲信号对应的编码器计数
	int32_t tagPulseOffset;		//触发喷码机喷码的编码器位置
	int32_t tagRecoverOffset;	//结束喷码机喷码的编码器位置
} TickEggStateDef;

//生产数据 包括产量 速度 成品率 单个通道信息等
typedef struct {
	uint32_t eggsTotal;   //生产总数
	uint16_t workSpeed;   //生产速度
	uint16_t yieldRate;		//成品率
	uint8_t packageState[SORTER_USED_MAX];	//通道状态
} ProductionDataType;

extern MasterOperateTypeDef masterOperate; 	//系统运行状态全局变量

extern ProductionDataType proData;

extern uint8_t alarmConfirm;							//报警确认按钮 按下则清除所有当前报警标志
extern uint8_t cupDetectFlag;							//主轨道蛋杯闭合异常信号
extern uint8_t malpositionDetectFlag;			//主轨道蛋杯鸡蛋位置异常
extern uint8_t arrayDetectFlag;						//排列通道排列异常

extern uint8_t	alarmConfirmBtn;					//报警确认按钮标志
extern uint16_t sorterUsedNumber;					//当前安装的包装通道数量

extern uint8_t get_input_io_sta(uint8_t *iData, enum EnumInputIO e);

extern void set_output_io_sta(uint8_t *oData, enum EnumOutputIO e, int8_t sta);

extern rt_bool_t name_is_empty(uint16_t *nm, uint8_t len);								//判断名字是否为空

extern rt_bool_t compare_names(uint16_t *fn, uint16_t *en, uint8_t len);  //用于比较两个数据是否相同  根据名称比较或查找对应参数时使用

extern rt_bool_t compare_range(int16_t rg1, int16_t rg2, int16_t real);		//判断数据是否在某个区间内

extern void led_and_beep_ctrl(enum EnumLedAndBeep color, uint8_t sta);

#endif
