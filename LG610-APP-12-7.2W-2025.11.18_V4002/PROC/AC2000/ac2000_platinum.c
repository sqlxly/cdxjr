#include "ac2000_platinum.h"
#include "data_table.h"
//#include "bsp_clkconfig.h"
//#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "data_table.h"
#include "humiture.h"
#include "mynet.h"
extern rt_size_t send_to_ac2000(const void* buffer, rt_size_t size);
//#include "mynet.h"
/**************注意*************************
	AC2000设备信息
	设备颜色：黄红色
	设备名称：ROTEM PLATINUM XL
	设备系统版本：4.07r14
****************************************/
int PlatinumRecvFlag=0;
PlatinumAc2000Params *Ac2000PlatinumParams=NULL;
unsigned char *PlatinumLevelsOfVentilationData=NULL;
uint16_t PlatinumLevelsOfVentilationDataLength=0;
extern unsigned char Plainum_Reset;
unsigned short Number_relaystatePL=0;
unsigned short Number_relaysPL=0;
extern unsigned char Platinum_Fan_State;
extern unsigned char Platinum_GetFan_;
extern unsigned char Coll_System;
//#if SINGLE_PLATINUM_TETS==1

unsigned char TunnelFan_Buffer[20];   //隧道风机在线记录数组
unsigned char ExhaustFan_Buffer[10];  //排气风机在线记录数组
unsigned char StrFan_Buffer[15];
unsigned char Light_Buffer[4];    //灯光数量数组

unsigned char Water_Buffer[4];
unsigned char Feed_Buffer[4];
unsigned char Heavedragon_Buffer[4];


unsigned char TunnelFan_Sum=0;        //数量
unsigned char ExhaustFan_Sum=0;       //排气风机数量
unsigned char StrFan_Sum=0;           //
unsigned char light_Sum=0;    
unsigned char Water_Sum=0;
unsigned char Feed_Sum=0;
unsigned char Heavedragon_Sum=0;

unsigned char Speedfan_Buffer[4];
unsigned char Frequency_Buffer[4];//变频风机

unsigned char Relay_Buffer[40];




//#if SINGLE_PLATINUM_TETS==1
const unsigned char AC2000PlatinumCmd[2][AC2000CmdLength]=/*命令码表*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},//连接 
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x0B,0x79,0x01,0x00,0x00,0xDC,0x63},
};
//#else
//const unsigned char AC2000PlatinumCmd[AC2000_Platinum_CmdNum][AC2000CmdLength]=/*命令码表*/
//{
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},//连接 
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x0B,0x79,0x01,0x00,0x00,0xDC,0x63},//查询 
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x52,0x01,0x00,0x00,0x1A,0x71},//曲线状态
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x06,0x2f,0x01,0x00,0x00,0x18,0x50},//饲料校准
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xA1,0x01,0x00,0x00,0x3C,0xE1},//饲料箱
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x4B,0x01,0x00,0x00,0x30,0x80},//水和饲料
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x0C,0x11},//报警查询
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0x00,0x00,0xFC,0x49},//查询前1-34个继电器状态
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xFC,0x00,0x2A,0x73},//查询前35-40个继电器状态
//	//{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x50,0x01,0x00,0x00,0x1A,0x6F},//温度和湿度状态
//	//{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x43,0x13,0x01,0x00,0x00,0x48,0xA1},//温度校准
//};
//#endif

const unsigned char AC2000PlatinumContorTemp[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x1D,0x01,0x00,0x00,0xF2,0x17},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x1D,0x01,0xF2,0x00,0x6E,0x85},

};
const unsigned char Ac2000PlatinumContorHumitidy[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x22,0x01,0x00,0x00,0xA8,0xCE}
};
const unsigned char Ac2000PlatinumContorMinMax[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x20,0x01,0x00,0x00,0x2C,0x50 }
};
const unsigned char Ac2000PlatinumContorStaticpressure[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x2A,0x01,0x00,0x00,0x30,0x60 }
};
const unsigned char Ac2000PlatinumContorMode[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x47,0x2B,0x01,0x00,0x00,0x07,0x7D }
};
const unsigned char Ac2000PlatinumSystemparameters[17][AC2000PlatinumCmdLength]=//系统参数
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x65,0x01,0x00,0x00,0x1C,0x86},//温度曲线
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x66,0x01,0x00,0x00,0x0A,0x75},//辐射加热器
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xA6,0x01,0x00,0x00,0x08,0xB2},//VH
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x6C,0x01,0x00,0x00,0x02,0x73},//湿度处理
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x86,0x01,0x00,0x00,0x01,0x8C},//CO2chuli
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x6D,0x01,0x00,0x00,0x06,0x78},//最高最低级别
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x69,0x01,0x00,0x00,0x18,0x7F},//静压
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x67,0x01,0x00,0x00,0x14,0x80},//通风等级
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x08,0x6E,0x01,0x00,0x00,0x1C,0x95},//屋顶卷帘
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x6A,0x01,0x00,0x00,0x10,0x82},//湿帘
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0x6B,0x01,0x00,0x00,0x0A,0x7C},//加湿器
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x6F,0x01,0x00,0x00,0x10,0x87},//灯光
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x87,0x01,0x00,0x00,0x14,0xA0},//水和饲料
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0B,0x79,0x01,0x00,0x00,0xDC,0x63},//饲料存量
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x70,0x01,0x00,0x00,0x01,0x76},//饲料标志
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x71,0x01,0x00,0x00,0x12,0x88},//紧急状态设置
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x72,0x01,0x00,0x00,0x01,0x78},//报警设置
};

const unsigned char Ac2000PlatinumContorCo2processing[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x92,0x01,0x00,0x00,0xD2,0x68}
};

//设备
const unsigned char Ac2000PlatinumMechVentilationlevel[3][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x99,0x01,0x00,0x00,0xF0,0x8D},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x99,0x01,0xF0,0x00,0xF0,0x7D},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x99,0x01,0xE0,0x01,0x10,0x8E}
};

const unsigned char Ac2000PlatinumMechSpeedcontrolfanlevel[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x44,0x01,0x00,0x00,0x7C,0xC8 }
};

const unsigned char Ac2000PlatinumMechStirfan[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x06,0x1E,0x01,0x00,0x00,0xF8,0x1F }
};


const unsigned char Ac2000PlatinumManageProChicken[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x0C,0x01,0x00,0x00,0x24,0x34 }
};

const unsigned char Ac2000PlatinumManageFeed[8][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x00,0x00,0xFC,0x63 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xFC,0x00,0xFC,0x5F },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xF8,0x01,0xFC,0x5C },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xF4,0x02,0xFC,0x59 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xF0,0x03,0xFC,0x56 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xEC,0x04,0xFC,0x53 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xE8,0x05,0xFC,0x50 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xE4,0x06,0x24,0x75 },
};
const unsigned char Ac2000PlatinumMangageTimeDay[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x5D,0x01,0x00,0x00,0x04,0x65 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x5C,0x01,0x00,0x00,0x04,0x65 },

};

const unsigned char Ac2000PlatinumManageChickens[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x0B,0x01,0x00,0x00,0x02,0x11 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x0D,0x01,0x00,0x00,0x04,0x15 },

};
const unsigned char Ac2000PlatinumManageAlarmSetting[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x09,0x3B,0x01,0x00,0x00,0x58,0x9F }
};
const unsigned char Ac2000PlatinumManageAlarmReset[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x72,0x01,0x00,0x00,0x01,0x77 }
};

const unsigned char Ac2000PlatinumManageNoSuccessfulSet[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x91,0x01,0x00,0x00,0x06,0x9B }
};

const unsigned char Ac2000PlatinumMangPassword[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x12,0x01,0x00,0x00,0x0E,0x24 }
};

const unsigned char Ac2000PlatinumTestSwitch[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x48,0x01,0x00,0x00,0xFC,0x49},
  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x48,0x01,0xFC,0x00,0xFC,0x45} 	
};
const unsigned char Ac2000PlatinumTestAnalogsensor[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x7A,0x01,0x00,0x00,0x2C,0xAB}
};
const unsigned char Ac2000PlatinumTestDialogSensor[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x7B,0x01,0x00,0x00,0x08,0x87 }
};
const unsigned char Ac2000PlatinumTestStaticPressure[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x7C,0x01,0x00,0x00,0x02,0x82 }
};
//服务

const unsigned char Ac2000PlatinumServer_TemTemperaturecalibration[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x43,0x13,0x01,0x00,0x00,0x48,0xA1}
};

const unsigned char Ac2000PlatinumServerHumidtiycalibration[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x4C,0x01,0x00,0x00,0x0C,0x5D },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x10,0x01,0x00,0x00,0x06,0x1B }
};
const unsigned char Ac2000PlatinumServerFeedCalibration[1][AC2000PlatinumCmdLength]=
{
  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x06,0x2F,0x01,0x00,0x00,0x18,0x50}
};
const unsigned char Ac2000PlatinumServerWaterCalibration[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x30,0x01,0x00,0x00,0x18,0x50}
};
const unsigned char Ac2000PlatinumServerPluginterface[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x46,0x01,0x00,0x00,0x90,0xDB}
};

const unsigned char Ac2000PlatinumServerWaterWaterlinenipple[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x07,0x8B,0x01,0x00,0x00,0x50,0xE5}
};
const unsigned char Ac2000PlatinumInstallSetup[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x07,0x31,0x01,0x00,0x00,0x0C,0x47}
};
const unsigned char Ac2000PlatinumInstallRelayLayout[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0B,0x32,0x01,0x00,0x00,0xFC,0x3C},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0B,0x32,0x01,0xFC,0x00,0x48,0x84 }
};
const unsigned char Ac2000PlatinumInstallAnalogSensor[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x47,0x33,0x01,0x00,0x00,0x17,0x94}
};
const unsigned char Ac2000PlatinumInstallDialogSensor[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x08,0x34,0x01,0x00,0x00,0x09,0x48 }
};

const unsigned char Ac2000PlatinumInstallAnalogOutput[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x85,0x40,0x01,0x00,0x00,0x50,0x18 }
};

const unsigned char Ac2000PlatinumInstallCurtainSetting[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0x36,0x01,0x00,0x00,0x20,0x5D}
};

const unsigned char Ac2000PlatinumInstallTemperaturedefinition[4][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0A,0x35,0x01,0x00,0x00,0xFC,0x3E},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0A,0x35,0x01,0xFC,0x00,0xFC,0x3A},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0A,0x35,0x01,0xF8,0x01,0xFC,0x37},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x0A,0x35,0x01,0xF4,0x02,0x6C,0xA4 }
};
const unsigned char Ac2000PlatinumInstallFanAirCapacity[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x42,0x01,0x00,0x00,0x78,0xC2}
};
const unsigned char Ac2000PlatinumInstallChickSizeof[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x43,0x01,0x00,0x00,0x10,0x5B}
};
const unsigned char Ac2000PlatinumInstallCommunication[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x17,0x01,0x00,0x00,0x02,0x1D}
};

const unsigned char Ac2000PlatinumMechCurtainlevel[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0x37,0x01,0x00,0x00,0xE0,0x1E},
	{0XFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0x37,0x01,0xE0,0x00,0x20,0x3E}
};
const unsigned char Ac2000PlatinumMechstirringfanprogarm[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x3D,0x01,0x00,0x00,0x0F,0x51}
};
const unsigned char Ac2000PlatinumMechsprayer[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x25,0x01,0x00,0x00,0xF0,0x19},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x25,0x01,0xF0,0x00,0x60,0x69}
}; 

const unsigned char Ac2000PlatinumMechRefrigerationplate[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x24,0x01,0x00,0x00,0xF0,0x18},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x24,0x01,0xF0,0x00,0x60,0x78 }
};

const unsigned char Ac2000PlatinumMechLightNoFeed[3][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x26,0x01,0x00,0x00,0xFA,0x28},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x26,0x01,0xFA,0x00,0xFA,0x22},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x26,0x01,0xF4,0x01,0x0A,0x2D}
};

const unsigned char Ac2000PlatinumMechLightFeed[3][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x95,0x01,0x00,0x00,0xFA,0x97 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x95,0x01,0xFA,0x00,0xFA,0x91 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x95,0x01,0xF4,0x01,0x0A,0x9C }	

};

const unsigned char Ac2000PlatinumMechWaterFeed[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x06,0x27,0x01,0x00,0x00,0xFC,0x2C },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x06,0x27,0x01,0xFC,0x00,0x9C,0xC8 },
};

const unsigned char Ac2000PlatinumMechWaterOnDemand[2][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xA5,0x01,0x00,0x00,0xF8,0xA1},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xA5,0x01,0xF8,0x00,0x98,0x39 }
};
const unsigned char Ac2000PlatinumMechAddSystem[1][AC2000PlatinumCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x28,0x01,0x00,0x00,0x50,0x7C }
};
void Ac2000DataPlatinumMalloc(void)
{
		if(Ac2000Data==NULL)
		{
			Ac2000Data=(AC2000_DATA_TYPE *)rt_malloc(sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);
			if(Ac2000Data!=NULL)
			{
				rt_memset(Ac2000Data,0,sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);
				Ac2000Data[ADDR_3G_1862]=1;
				Ac2000Data[ADDR_3G_1863]=1;
				Ac2000Data[ADDR_3G_1864]=1;
				Ac2000Data[ADDR_3G_1865]=1;
				rt_kprintf("Ac2000Data malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000Data malloc failed\r\n");
			}		
		}
		if(Ac2000OldData==NULL)
		{
			Ac2000OldData=(AC2000_DATA_TYPE *)rt_malloc(sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);	
			if(Ac2000OldData!=NULL)
			{
				rt_memset(Ac2000OldData,0,sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);
				rt_kprintf("Ac2000OldData3G malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000OldData3G malloc failed\r\n");
			}	
		}
			if(Ac2000PlatinumParams==NULL)
		{
			Ac2000PlatinumParams=(PlatinumAc2000Params *)rt_malloc(sizeof(PlatinumAc2000Params));
			if(Ac2000PlatinumParams!=NULL)
			{
				rt_memset(Ac2000PlatinumParams,0,sizeof(PlatinumAc2000Params));
				int len=sizeof(PlatinumAc2000Params);
				rt_kprintf("PlatinumAc2000Params size:%d\n",len);
			}
			else
			{
				rt_kprintf("PlatinumAc2000Params创建失败!\n");
			}
		}
}

void CreateAC2000PlatinumSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000PlatinumCmd[CmdIndex][8];
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000PlatinumCmd[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000PlatinumCmd[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000PlatinumCmd[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000PlatinumCmd[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000PlatinumCmd[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000PlatinumCmd[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000PlatinumCmd[CmdIndex][17]+HenHouseNum;
	
	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	AC2000_SendBuff[17]=sum;
	
//	rt_kprintf("mcu send platinum:\r\n");
//	for(uint16_t i=0;i<18;i++)
//	{
//		rt_kprintf("%02x ",AC2000_SendBuff[i]);
//	}
//	rt_kprintf("\r\n");
}
void CreateAC2000PlatinumNormal(const uint8_t CmdSource[][AC2000PlatinumCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = CmdSource[CmdIndex][0];
	AC2000_SendBuff[1] = CmdSource[CmdIndex][1];
	AC2000_SendBuff[2] = CmdSource[CmdIndex][2];
	AC2000_SendBuff[3] = CmdSource[CmdIndex][3];
	AC2000_SendBuff[4] = CmdSource[CmdIndex][4];
	AC2000_SendBuff[5] = CmdSource[CmdIndex][5];
	AC2000_SendBuff[6] = CmdSource[CmdIndex][6];
	AC2000_SendBuff[7] = CmdSource[CmdIndex][7];
	AC2000_SendBuff[8] = CmdSource[CmdIndex][8];
	AC2000_SendBuff[9] = CmdSource[CmdIndex][9];
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = CmdSource[CmdIndex][11];
	AC2000_SendBuff[12] = CmdSource[CmdIndex][12];
	AC2000_SendBuff[13] = CmdSource[CmdIndex][13];
	AC2000_SendBuff[14] = CmdSource[CmdIndex][14];
	AC2000_SendBuff[15] = CmdSource[CmdIndex][15];
	AC2000_SendBuff[16] = CmdSource[CmdIndex][16];
	AC2000_SendBuff[17] = CmdSource[CmdIndex][17];
	
	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	AC2000_SendBuff[17]=sum;
	
//	rt_kprintf("mcu send:\r\n");
//	for(uint16_t i=0;i<18;i++)
//	{
//		rt_kprintf("%02x ",AC2000_SendBuff[i]);
//	}
//	rt_kprintf("\r\n");
	
}

void RelayCreateAC2000PLNormal(const uint8_t CmdSource[][AC2000PlatinumCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,unsigned short dex,unsigned short prov,unsigned char state,unsigned short prov1)
{
	 unsigned short cex=0;
	 cex = (unsigned short)((dex-1)*prov);
if(CmdIndex == 0)
{
	AC2000_SendBuff[0] = CmdSource[CmdIndex][0];
	AC2000_SendBuff[1] = CmdSource[CmdIndex][1];
	AC2000_SendBuff[2] = CmdSource[CmdIndex][2];
	AC2000_SendBuff[3] = CmdSource[CmdIndex][3];
	AC2000_SendBuff[4] = CmdSource[CmdIndex][4];
	AC2000_SendBuff[5] = CmdSource[CmdIndex][5];
	AC2000_SendBuff[6] = CmdSource[CmdIndex][6];
	AC2000_SendBuff[7] = CmdSource[CmdIndex][7];
	AC2000_SendBuff[8] = CmdSource[CmdIndex][8];
	AC2000_SendBuff[9] = CmdSource[CmdIndex][9];
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = CmdSource[CmdIndex][11];
	AC2000_SendBuff[12] = CmdSource[CmdIndex][12];
	AC2000_SendBuff[13] = CmdSource[CmdIndex][13];
	AC2000_SendBuff[14] = CmdSource[CmdIndex][14];
	AC2000_SendBuff[15] = CmdSource[CmdIndex][15];
	AC2000_SendBuff[16] = CmdSource[CmdIndex][16];
	AC2000_SendBuff[17] = CmdSource[CmdIndex][17];
}
else if(CmdIndex==1)
{
	AC2000_SendBuff[0] = CmdSource[CmdIndex][0];
	AC2000_SendBuff[1] = CmdSource[CmdIndex][1];
	AC2000_SendBuff[2] = CmdSource[CmdIndex][2];
	AC2000_SendBuff[3] = CmdSource[CmdIndex][3];
	AC2000_SendBuff[4] = CmdSource[CmdIndex][4];
	AC2000_SendBuff[5] = CmdSource[CmdIndex][5];
	AC2000_SendBuff[6] = CmdSource[CmdIndex][6];
	AC2000_SendBuff[7] = CmdSource[CmdIndex][7];
	AC2000_SendBuff[8] = CmdSource[CmdIndex][8];
	AC2000_SendBuff[9] = CmdSource[CmdIndex][9];
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = CmdSource[CmdIndex][11];
	AC2000_SendBuff[12] = CmdSource[CmdIndex][12];
	AC2000_SendBuff[13] = CmdSource[CmdIndex][13];
	AC2000_SendBuff[14] =cex&0xFF;
	AC2000_SendBuff[15] =(cex>>8)&0xFF; 
	if(state==0)
	AC2000_SendBuff[16] = prov;
	else if(state==1)
	AC2000_SendBuff[16] = prov1;
	AC2000_SendBuff[17] = CmdSource[CmdIndex][17];
}

	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	  AC2000_SendBuff[17]=sum;
}



void PlatinumOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000PlatinumCmdLength],uint8_t cmd_num,uint8_t *cmd_index)
{

	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PlatinumNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,(*cmd_index));   //参数赋值
		(*cmd_index)++;
		PlatinumRecvFlag=(AC2000SendBuff[16]+7);
		if(Platinum_GetFan_==0)
		{
			if((*cmd_index)==cmd_num)
			{
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
			}
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);	
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}
}

unsigned char PLSendFlip=0;
unsigned char State_PL=1;
void relaySend_PL(const uint8_t Cmd[][AC2000PlatinumCmdLength],unsigned short porv,unsigned char difference)
{
		unsigned short Quotient=0;
		unsigned short Morethan=0;
	
	if(difference==1)
	{
		if(PLSendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PlatinumNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,0);   //参数赋值
		PlatinumRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//    PLSendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(PLSendFlip==1)
	{
		Quotient =Number_relaysPL/porv;
		Morethan =Number_relaysPL%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient==0)&&(Morethan)==0)//没有数据直接填空发送
			{
				AirxxxStatus.recv_systeminfo_cmd=2;	
				PLSendFlip=0;		
	//			Write_data_Empty();
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
			}
			else if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,porv,0,porv);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
				
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,Morethan,0,Morethan);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);

				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);

					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					State_PL=State_PL+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,1,Morethan);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					my_log(AC2000SendBuff,18);
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_PL=State_PL+1;
					my_log(AC2000SendBuff,18);
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
	}
	}
 else if(difference==2)
 {
	 if(PLSendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PlatinumNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,0);   //参数赋值
		PlatinumRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//    PLSendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(PLSendFlip==1)
	{
		Quotient =Number_relaystatePL/porv;
		Morethan =Number_relaystatePL%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,porv,0,porv);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
				
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,Morethan,0,Morethan);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);

				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);

					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					State_PL=State_PL+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,1,Morethan);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					my_log(AC2000SendBuff,18);
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_PL=State_PL+1;
					my_log(AC2000SendBuff,18);
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 }
  else if(difference==3)//
 {
	 if(PLSendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PlatinumNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,0);   //参数赋值
		PlatinumRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//    PLSendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(PLSendFlip==1)
	{
		Quotient =Number_relaysPL/porv;
		Morethan =Number_relaysPL%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,porv,0,porv);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
	//			EquipmentState=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,Morethan,0,Morethan);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
	//			EquipmentState=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
			//		EquipmentState=1;
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					State_PL=State_PL+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,1,Morethan);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
			//		EquipmentState=1;
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_PL=State_PL+1;
					my_log(AC2000SendBuff,18);
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 }else if(difference==4)//
 {
	 
		Quotient =Number_relaystatePL/porv;
		Morethan =Number_relaystatePL%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,porv,0,porv);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
	//			EquipmentState=0;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,1,Morethan,0,Morethan);
				PlatinumRecvFlag=(AC2000SendBuff[16]+7);
		//		EquipmentState=0;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				PLSendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
				//	EquipmentState=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_PL=1;
		  	//	FullState=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					State_PL=State_PL+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_PL>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,1,Morethan);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
			//		EquipmentState=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_PL=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					my_log(AC2000SendBuff,18);
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					PLSendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000PlatinumCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PLNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_platinum_house,1,State_PL,porv,0,porv);
					PlatinumRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_PL=State_PL+1;
					my_log(AC2000SendBuff,18);
					send_to_ac2000((const void*)AC2000SendBuff, AC2000PlatinumCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000PlatinumCmdLength,2000,3,&PlatinumRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 
}






static unsigned char Parse_MainScreen_Platinum(uint8_t *dataSource,uint16_t len)
{
	/*
	01 17 0b 79 01 9d 
	0f 04 ce 00 90 00 70 03 75 00 00 00 01 00 01 
	01 通风等级  转十进制
	01 00 00 00 00 00 00 00 00 00 00 00 00 00 02 03 02 03 02 03 02 03 02 03 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 04 
	00 舍内湿度1
	ff 舍内湿度2
	00 内部湿度-平均湿度
	64 外部湿度
	01 00 
	18 00 日龄  高低交换后转十进制
	4c 58 时间 第二个字节 除以8取整是 时 第一个字节 除以31取整 再加上 （（第二个字节除以8取余）乘以8） 46 BA 表示 23:18
	e0 00 3b 00 00 00 00 00 00 00 
	6a 0a 目标温度 高低交换后转十进制 后 除以100 取1位小数
	71 0a 
	71 0a 平均温度
	ee 0a 隧道温度-通道温度
	10 27 室外温度
	f0 d8 
	ee 0a 80 0a 10 27 40 0a 2e 0a 5b 0a 52 0a 10 27 10 27 10 27 10 27 10 27 10 27 10 27 10 27 10 27 10 27 10 27      18个温度传感器、两个字节表示一个温度、 高低交换后转十进制
	00 00 00 00 10 27 
	ff ff     CO2  高低交换后转十进制 FF FF 表示没有使用CO2传感器 5E 02 表示606
	80 0a     当前静压 高低交换后转十进制 后 除以100 取整
	00 00 c8 07 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	11 

	*/
	unsigned int postion,TmpInt;

  postion=21;
	Ac2000Data[ADDR_3G_1805]=dataSource[postion];	//通风等级
	rt_kprintf("通风等级=%d\r\n",Ac2000Data[ADDR_3G_1805]);
		

	postion=postion+95;
	if(dataSource[postion]==0xFF)
	{
		Ac2000Data[ADDR_3G_1814] =9999;//湿度传感器1未启用
		rt_kprintf("舍内湿度1=%d\r\n",Ac2000Data[ADDR_3G_1814]);
	}
	else
	{
		Ac2000Data[ADDR_3G_1814]=dataSource[postion];//舍内湿度1
		rt_kprintf("舍内湿度1=%d\r\n",Ac2000Data[ADDR_3G_1814]);
	}
	
	postion=postion+1;
	if(dataSource[postion]==0xFF)
	{
		Ac2000Data[ADDR_3G_1815] =9999;//湿度传感器2未启用
		rt_kprintf("舍内湿度2=%d\r\n",Ac2000Data[ADDR_3G_1815]);
	}
	else
	{
		Ac2000Data[ADDR_3G_1815]=dataSource[postion];//舍内湿度1
		rt_kprintf("舍内湿度2=%d\r\n",Ac2000Data[ADDR_3G_1815]);
	}
	
  postion=postion+1;
	if(dataSource[postion]==0xFF)
	{
		Ac2000Data[ADDR_3G_1806] =9999;//内部湿度-平均湿度未启用
		rt_kprintf("内部湿度-平均湿度=%d\r\n",Ac2000Data[ADDR_3G_1806]);
	}
	else
	{
		Ac2000Data[ADDR_3G_1806]=dataSource[postion];//内部湿度-平均湿度
		rt_kprintf("内部湿度-平均湿度=%d\r\n",Ac2000Data[ADDR_3G_1806]);
	}
	
  postion=postion+1;
	if(dataSource[postion]==0xFF)
	{
		Ac2000Data[ADDR_3G_1816] =9999;//外部湿度
		rt_kprintf("外部湿度=%d\r\n",Ac2000Data[ADDR_3G_1816]);
	}
	else
	{
		Ac2000Data[ADDR_3G_1816]=dataSource[postion];//外部湿度
		rt_kprintf("外部湿度=%d\r\n",Ac2000Data[ADDR_3G_1816]);
	}
	
	postion=postion+3;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//日龄
	Ac2000Data[ADDR_3G_1804] =TmpInt;
	rt_kprintf("日龄=%d\r\n",TmpInt);
	
	postion=postion+2;
	//TmpInt=dataSource[postion]+dataSource[postion+1]*256;//时间
	rt_kprintf("时间=%d:%d\r\n",dataSource[postion+1]/8,(dataSource[postion]/31)+((dataSource[postion+1]%8))*8);
	
	postion=postion+12;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//目标温度
	Ac2000Data[ADDR_3G_1847] =TmpInt;
	rt_kprintf("目标温度=%d\r\n",TmpInt);
	
	postion=postion+4;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//平均温度
	Ac2000Data[ADDR_3G_1807] =TmpInt;
	rt_kprintf("平均温度=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//隧道温度-通道温度
	Ac2000Data[ADDR_3G_1810] =TmpInt;
	rt_kprintf("隧道温度-通道温度=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//室外温度
	Ac2000Data[ADDR_3G_1808] =TmpInt;
	rt_kprintf("室外温度=%d\r\n",TmpInt);
	
	postion=postion+4;
	for(uint8_t i=0;i<6;i++)
	{
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;	
		if(i<9)
		{
			if(TmpInt==0x2710)//不详
			{
				Ac2000Data[ADDR_3G_1821+i] = 9999;
			}
			else
			{
				Ac2000Data[ADDR_3G_1821+i] =TmpInt;
			}
			rt_kprintf("温度传感器=%d  %d\r\n",(i+1),(TmpInt));	
		}	
		postion=postion+2;
	}
	
	postion=postion+30;
	TmpInt=dataSource[postion]+dataSource[postion+1]*256;//CO2
	if(TmpInt==0xFFFF)
	{
		Ac2000Data[ADDR_3G_1813] =9999;
		rt_kprintf("co2 no exist\r\n");
	}
	else
	{
		Ac2000Data[ADDR_3G_1813]=TmpInt;
		rt_kprintf("CO2=%d\r\n",TmpInt);
	}
	
	postion=postion+6;
	TmpInt=(dataSource[postion]+dataSource[postion+1]*256)/100;//当前静压
	Ac2000Data[ADDR_3G_1803]=TmpInt; 
	rt_kprintf("当前静压=%d\r\n",TmpInt);
	
	Ac2000Data[ADDR_PLATINUM_2480] = tiWenCalc((Ac2000Data[ADDR_PLATINUM_2401]/100), Ac2000Data[ADDR_PLATINUM_2409], Ac2000Data[ADDR_PLATINUM_2466]);
	rt_kprintf("体感温度=%d\r\n",Ac2000Data[ADDR_PLATINUM_2480]);
	
	return postion;
}

static void Parse_curve_state_Platinum(uint8_t *dataSource,uint16_t len)
{
	/*
	01 17 02 52 01 6d 
	6a 0a 目标温度
	54 09 低温报警
	ac 0d 高温报警
	d6 4d 目标静压
	fb 09 加热器温度
	96 0c 通道温度
	01 最低级别
	1e 最高级别
	00 00 00 00 00 00 00 00 00 00 00 00 72 
	*/
	unsigned int postion,TmpInt;
	float	TmpFloat;
	
	postion=6;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_PLATINUM_2426] = TmpFloat;//目标温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_PLATINUM_2426] = 0xffff-TmpInt;
	}	
	rt_kprintf("目标温度=%d\r\n",Ac2000Data[ADDR_PLATINUM_2426]);
	
	postion=postion+2;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_PLATINUM_2429] = TmpFloat;//低温警报温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_PLATINUM_2429] = 0xffff-TmpInt;
	}	
	rt_kprintf("低温警报温度=%d\r\n",Ac2000Data[ADDR_PLATINUM_2429]);
	
	postion=postion+2;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_PLATINUM_2430] = TmpFloat;//高温警报温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_PLATINUM_2430] = 0xffff-TmpInt;
	}	
	rt_kprintf("高温警报温度=%d\r\n",Ac2000Data[ADDR_PLATINUM_2430]);
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion])/1000;
	Ac2000Data[ADDR_PLATINUM_2431] = TmpInt;//目标静压
	rt_kprintf("目标静压=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	Ac2000Data[ADDR_PLATINUM_2427] = TmpInt;//加热器温度
	rt_kprintf("加热器温度=%d\r\n",TmpInt);

	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	Ac2000Data[ADDR_PLATINUM_2428] = TmpInt;//隧道温度
	rt_kprintf("隧道温度=%d\r\n",TmpInt);	
	
	postion=postion+2;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2432] = TmpInt;//最低级别
	rt_kprintf("最低级别=%d\r\n",TmpInt);	
	
	postion=postion+1;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2433] = TmpInt;//最高级别
	rt_kprintf("最高级别=%d\r\n",TmpInt);	
	
}
//报警解析
static void Parse_Warn_Platinum(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 00 02 01 18 07 
	03 
	01 00 
	6f 05 
	03 02 
	00 00 
	00 00 
	84 
	
	01 17 00 02 01 1b 07 
	03 
	ce 00 
	90 00 
	75 00 
	00 00 
	00 00 
	dd 
	
	01 17 02 50 01 6b 14 
	0b db 
	0a d8 
	0a 94 
	0b 10 
	27 14 
	0b 10 
	27 10 
	27 10 
	27 ff 
	ff 00 
	00 9c 
	ff 10 
	27 50 
	
	
	*/
	unsigned int postion=7;
	unsigned char warnNum=dataSource[postion++];
	if(warnNum>0)
	{
		Ac2000Data[ADDR_PLATINUM_2421]=1;//3G版报警状态
	}
	else
	{
		Ac2000Data[ADDR_PLATINUM_2421]=0;
	}
	unsigned int alarm_code=0x00;
	for(unsigned char j=0;j<=AC2000_ALARM_NUM_PLATINUM;j++)
	{
		AlarmCodePlatinum[ALARM_PLATINUM_13000+j][1]=0;
	}
	
	for(unsigned char i = 0; i<warnNum; i++)
	{
		alarm_code=dataSource[postion]*256+dataSource[postion+1];//得到告警类型码
		for(unsigned short j=0;j<AC2000_ALARM_NUM_PLATINUM;j++)
		{
			if(alarm_code==AC2000WarmCode[j])
			{
				rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				AlarmCodePlatinum[ALARM_PLATINUM_13000+j][1]=1;
				break;
			}
			if(j==(AC2000_ALARM_NUM_PLATINUM-1))
			{
				if(alarm_code==AC2000WarmCode[AC2000_ALARM_NUM_PLATINUM-1])
				{
					AlarmCodePlatinum[ALARM_PLATINUM_13000+j][1]=1;
					rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				}
				else
				{
					AlarmCodePlatinum[ALARM_PLATINUM_13000][1]=1;
					rt_kprintf("未知报警\r\n");
				}
			}
		}
		postion+=2;
	}
}

static float AC2000ServiceFeedFactor[4];
static void GetAC2000ServiceFeedFactor(unsigned char *dataSource,uint16_t len)
{
	/*
	01 17 06 2f 01 4e 
	01 00 00 00 80 3f 01 00 00 00 80 3f 
	01 00 00 00 80 3f 01 00 00 00 80 3f 
	00 
	01 17 06 2f 01 4e 
	01 00 db 73 e8 3e 02 00 db 73 e8 3e 
	01 00 db 73 e8 3e 00 00 db 73 e8 3e 
	d4 
	01 17 06 2f 01 4e 
	01 00 db 73 68 3f 02 00 db 73 e8 3e 
	01 00 db 73 e8 3e 00 00 db 73 e8 3e 
	55 
	
	01 17 06 2f 01 4e 
	01 00 db 73 68 3f 02 00 e2 33 08 43 
	01 00 db 73 e8 3e 00 00 db 73 e8 3e 
	41 
	
	*/
	unsigned int postion;
	//uint16_t	TmpInt;
	float TmpFloat;

	postion=6;
	
	//TmpInt=dataSource[postion];// Method
	postion=postion+2;
	TmpFloat=FourByteToFloat(&dataSource[postion])*2.2f;//因数1   2/9
	AC2000ServiceFeedFactor[0]=TmpFloat;
		
	postion=postion+4;
	//TmpInt=dataSource[postion];//Method
	postion=postion+2;
	TmpFloat=FourByteToFloat(&dataSource[postion])*2.2f;//因数2
	AC2000ServiceFeedFactor[1]=TmpFloat;

	postion=postion+4;
	//TmpInt=dataSource[postion];//Method
	postion=postion+2;
	TmpFloat=FourByteToFloat(&dataSource[postion])*2.2f;//因数3
	AC2000ServiceFeedFactor[2]=TmpFloat;
	
	postion=postion+4;
	//TmpInt=dataSource[postion];//Method
	postion=postion+2;
	TmpFloat=FourByteToFloat(&dataSource[postion])*2.2f;//因数4
	AC2000ServiceFeedFactor[3]=TmpFloat;

  rt_kprintf("饲料因数=%0.3f,%0.3f,%0.3f,%0.3f\r\n",AC2000ServiceFeedFactor[0],AC2000ServiceFeedFactor[1],AC2000ServiceFeedFactor[2],AC2000ServiceFeedFactor[3]);
}
static void Parse_FeedLeft_Platinum(unsigned char *dataSource,uint16_t len)
{
	/*
	01 17 01 A1 01 BB 
	04 04 04 05 00 00 00 00 00 00 00 00 
	F0 79 
	CF 07 
	CF 07 
	00 00 
	
	B8 0B 
	CF 07 
	B8 0B 
	00 00
	
	11 32 00 00 00 00 00 00 39 FE FF FF 00 00 00 00 19 32 00 00 00 00 00 00 39 FE FF FF 00 00 00 00 7A 
	*/
	unsigned int postion=18;
	unsigned int WData1=dataSource[postion+1]*256+dataSource[postion];	
	postion=26;
	unsigned int WData2=dataSource[postion+1]*256+dataSource[postion];	
	
	if(AC2000ServiceFeedFactor[0]<0)
	{
		AC2000ServiceFeedFactor[0]=1;
	}
	Ac2000Data[ADDR_PLATINUM_2425]=(WData1-WData2)/AC2000ServiceFeedFactor[0];//料塔1余料
	rt_kprintf("料塔1余料=%d\r\n",Ac2000Data[ADDR_PLATINUM_2425]);
}


//水和食料解析
static void Parse_FeedWater_Platinum(unsigned char *dataSource,uint16_t len)
{
		/*
	01 14 03 4B 01 64 
	00 00 00 00 
	00 00 00 00 82 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 82 
	01 14 03 4B 01 64 00 00 00 00 00 00 00 00 82 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 82 
	
	01 17 02 4B 01 66 
	01 00 01 FF FF FF FF FF FF FF FF FF FF FF FF 00 
	78 37 00 00 水1
	00 00 00 00 水2
	00 00 00 00 水3
	00 00 00 00 水4
	05 BA 01 00 喂料器1今日耗料
	00 00 00 00 喂料器2今日耗料
	C6 11 00 00 喂料器3今日耗料
	00 00 00 00 喂料器4今日耗料
	3C
	*/
	unsigned int postion=22;
	unsigned int WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	Ac2000Data[ADDR_PLATINUM_2422]=WData;//水表1
	rt_kprintf("水表1=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	Ac2000Data[ADDR_PLATINUM_2423]=WData;//水表2
	rt_kprintf("水表2=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	rt_kprintf("水表3=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	rt_kprintf("水表4=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	Ac2000Data[ADDR_PLATINUM_2424]=WData;//饲料1 今日耗料
	rt_kprintf("饲料1=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	rt_kprintf("饲料2=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	rt_kprintf("饲料3=%d\r\n",WData);
	
	postion=postion+4;
	WData=dataSource[postion+3]*256*256*256+dataSource[postion+2]*256*256+dataSource[postion+1]*256+dataSource[postion];	
	rt_kprintf("饲料4=%d\r\n",WData);
}


// 前34继电器状态
static void Parse_RelyTest1_Platinum(uint8_t *dataSource,uint16_t len)
{
	/*
	01 17 02 48 01 63 
	
	00 00 00 00 00 00 00 00 00 00 00 00 00 00
	
	32 00 00 00 00 00 00 00 00 00 00 00 00 00 
	32 00 00 00 00 00 00 00 00 00 00 00 00 00 
	32 00 00 00 00 00 00 00 00 00 00 00 00 00 
	32 00 00 00 00 00 00 00 00 00 00 00 00 00 
	33 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	2b 
	4表示没有继电器
	0表示手动关闭
	1表示手动开启
	2表示自动模式
	3表示自动打开
	*/
	/*
	4 NO_RELAY
	3 AUTO_OPEN
	2 AUTO_CLOSE
	1 HAND_OPEN
	0 FORCE_CLOSE
	*/
	unsigned int postion=0;
	postion=20;
	int x,y=0;
	for(uint8_t i=0;i<15;i++)
	{
		y=(dataSource[postion]&0x00F0)>>4;
		x=dataSource[postion]&0x000F;
		if(x==1||x==3)
		{
			x=1;
		}
		else if(x==0||x==2)
		{
			x=0;
		}
		
		if(y==1||y==3)
		{
			y=1;
		}
		else if(y==0||y==2)
		{
			y=0;
		}	
		Ac2000Data[ADDR_PLATINUM_2434+(i*2)]=x;
		Ac2000Data[ADDR_PLATINUM_2434+(i*2+1)]=y;
		postion=postion+14;
	}
	for(uint8_t i=0;i<30;i++)
	{
		rt_kprintf("继电器%d 状态=%d  \n",(i+2434),Ac2000Data[ADDR_PLATINUM_2434+i]);
	}
	
	for(uint8_t i=0;i<2;i++)
	{
		y=(dataSource[postion]&0x00F0)>>4;
		x=dataSource[postion]&0x000F;
		if(x==1||x==3)
		{
			x=1;
		}
		else if(x==0||x==2)
		{
			x=0;
		}
		
		if(y==1||y==3)
		{
			y=1;
		}
		else if(y==0||y==2)
		{
			y=0;
		}	
		Ac2000Data[ADDR_PLATINUM_2551+(i*2)]=x;
		Ac2000Data[ADDR_PLATINUM_2551+(i*2+1)]=y;
		postion=postion+14;
	}
	
	for(uint8_t i=0;i<4;i++)
	{
		rt_kprintf("继电器%d 状态=%d  \n",(i+2551),Ac2000Data[ADDR_PLATINUM_2551+i]);
	}
}
// 接着6路继电器状态
static void Parse_RelyTest2_Platinum(uint8_t *dataSource,uint16_t len)
{
	/*
	01 17 02 48 01 63 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	44 00 00 00 00 00 00 00 00 00 00 00 00 00 
	cc 
	4表示没有继电器
	0表示手动关闭
	1表示手动开启
	2表示自动模式
	3表示自动打开
	*/
	/*
	4 NO_RELAY
	3 AUTO_OPEN
	2 AUTO_CLOSE
	1 HAND_OPEN
	0 FORCE_CLOSE
	*/
	unsigned int postion=0;
	postion=6;
	int x,y=0;
	for(uint8_t i=0;i<3;i++)
	{
		y=(dataSource[postion]&0x00F0)>>4;
		x=dataSource[postion]&0x000F;
		if(x==1||x==3)
		{
			x=1;
		}
		else if(x==0||x==2)
		{
			x=0;
		}
		
		if(y==1||y==3)
		{
			y=1;
		}
		else if(y==0||y==2)
		{
			y=0;
		}	
		Ac2000Data[ADDR_PLATINUM_2555+(i*2)]=x;
		Ac2000Data[ADDR_PLATINUM_2555+(i*2+1)]=y;
		postion=postion+14;
	}
	for(uint8_t i=0;i<6;i++)
	{
		rt_kprintf("继电器%d 状态=%d  \n",(2555+i),Ac2000Data[ADDR_PLATINUM_2555+i]);
	}
}

void Platinum_Coll_Temperaturecalibration(uint8_t *dataSource,uint16_t len)
{
	if(len==249)
	 {
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PlatinumLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(len >6)
	{
		rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		unsigned char i=0;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
	  rt_memset(SysData,0,AliyunServiceSysLength*3);
		postion= 22;
		for(i=0;i<15;i++)
		{
			TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;//日期
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion+2;
			TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);//目标温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);//加热温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);//隧道温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
		
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);//低的报警
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);//高的报警
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+4;

		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumAc2000ControlTemperatureCurve,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumAc2000ControlTemperatureCurve));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumAc2000ControlTemperatureCurve,SysData);	

	  rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_kprintf("温度曲线:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROTEMPERATURECURVE_CMD;
		}
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
	
	}
		
}
void Platinum_Coll_Humidity(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		
	  unsigned char i;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =14;
		for(i=0;i<20;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日期
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//湿度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//delay
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//duration
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinumshumidity_treatm,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinumshumidity_treatm));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinumshumidity_treatm,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROHUMIDITYTREATM_CMD;
		}
		
}
void Platinum_Coll_Min_Max(uint8_t *dataSource,uint16_t len)
{
	
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		int Hour;
		int Minute;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =12;
		for(i=0;i<10;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
			
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
			
				postion =postion-2;
			
				TmpInt = dataSource[postion];//最低级别
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+1;
				TmpInt = dataSource[postion];//最高级别
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion= postion+5;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinummin_max_level,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinummin_max_level));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.Platinummin_max_level,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROMINMAX_CMD;
		}
		
}

void Platinum_Coll_Static_Pressure(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =10;
		for(unsigned char i=0;i<5;i++)
		{
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+8;
		}
		postion =14;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =30;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =38;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion= 46;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 6;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);;
		postion =50;
		TmpInt = (int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumStaticpressure,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumStaticpressure));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumStaticpressure,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROSTATICPRESSURE_CMD;
		}
		
}

void Platinum_Coll_ControlMode(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 10;
		TmpInt = dataSource[postion];//整个鸡舍
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	  postion = 12;
		TmpInt = dataSource[postion];//是否空鸡舍模式
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 7;
		TmpInt = dataSource[postion];//温度曲线是否关闭
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 11;
		TmpInt = dataSource[postion];//最低最高控制
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumControlmode,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumControlmode));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumControlmode,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROMODE_CMD;
		}
	
}
void Platinum_Coll_System_temperaturecali(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
	  float TmpFloat; 
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 10;
		for(i=0;i<4;i++)
		{
			TmpFloat = FourByteToFloat(&dataSource[postion]);//温度曲线修正值
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		postion = 8;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//降温系数百分比
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = 26;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//快速降温限制
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);

		postion = 7;
		TmpInt = dataSource[postion];//最小通风低于加热温度的值
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = 30;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//非养殖区与温度的差值
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyTemperature_curve,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyTemperature_curve));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyTemperature_curve,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Platinum_Coll_System_Radiant_heater(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	
	  float TmpFloat; 
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		
		postion =8;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 6;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyRadiant_heater,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyRadiant_heater));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyRadiant_heater,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Platinum_Coll_System_VariableHeater(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Variable_heater,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Variable_heater));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Variable_heater,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);

}

void Platinum_Coll_System_Humidity_treatment(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion +1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Humidity_treatment,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Humidity_treatment));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Humidity_treatment,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);

}
void Platinum_Coll_Co2Mode(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Co2_dealwith,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Co2_dealwith));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Co2_dealwith,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);


}
void Platinum_Coll_Min_Max_level(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =8;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Min_Max_Level,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Min_Max_Level));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Min_Max_Level,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);

}
void Platinum_Coll_System_Staticpressure(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
	  unsigned char i;
		postion = 10;//阵风延时
	  for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+1;
		}
		postion = 18;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = 22;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion = 20;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = 13;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion =14;
		for(i=0;i<4;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+1;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Static_pressure,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Static_pressure));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Static_pressure,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Platinum_Level_Ventilation(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+1;
		}
		

		postion = 18;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion =10;
		for(i=0;i<4;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Ventilation_level,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Ventilation_level));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Ventilation_level,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		

}
void Platinum_Coll_System_Shutter_Roof(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		int Hour;
	  int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =7;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =18;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=9;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=11;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =12;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =14;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=26;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion =10;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=22;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		
		postion=postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		
		postion=30;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.roof,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.roof));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.roof,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Platinum_Coll_System_WetCurtain(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		int Hour;
	  int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =10;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=8;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		postion=6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=18;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=7;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Wet_curtain,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Wet_curtain));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Wet_curtain,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
}

void Platinum_Coll_System_humidifier(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =8;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=7;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.humidifier,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.humidifier));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.humidifier,SysData);	

	  rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Platinum_Coll_System_light(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion= postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =16;
		for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
		}
		postion=8;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=9;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=12;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=14;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=10;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.light,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.light));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.light,SysData);	
		rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		
}

void Platinum_Coll_System_Water_Feed(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Water_feed,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Water_feed));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Water_feed,SysData);	
		rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Platinum_Coll_System_Waterstock(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 210;
		for(unsigned char i=0;i<4;i++)
		{
			TmpInt = (int)(FourByteToFloat(&dataSource[postion])*2.203);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Feed_stock,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Feed_stock));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Feed_stock,SysData);	
		rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Platinum_Coll_System_Activate_funnel(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Activate_funnel,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Activate_funnel));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Activate_funnel,SysData);	
		rt_kprintf("湿度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Platinum_Coll_System_EmerGency_Setting(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 20;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 16;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 8;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =6;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Emergency_Settings,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Emergency_Settings));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Emergency_Settings,SysData);	
		rt_kprintf("紧急状态设置-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Platinum_Coll_Alarmset(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(Plainum_Reset==1)
		{
			Plainum_Reset=0;
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmreset,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmreset));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmreset,SysData);	  

			rt_free(SysData);
		
			if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGEALARMRESET_CMD;
		}
		}
		else 
		{
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Alarm_system,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Alarm_system));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Alarm_system,SysData);	  
			rt_kprintf("报警状态设置-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
			rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROSYSTEMMODE_CMD;
		}
		}
		Plainum_Reset=0;
		Coll_System=0;
}
void Platinum_Coll_Co2Treatment(uint8_t *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =16;
		for(unsigned char i=0;i<20;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumCo2processing,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumCo2processing));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumCo2processing,SysData);	  
		rt_kprintf("报警状态设置-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumCONTROCO2PROCESSING_CMD;
		}
}

void Platinum_Equipment_Ventilationlevel(uint8_t *dataSource,uint16_t len)
{
	if(len==247)
	{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PlatinumLevelsOfVentilationDataLength<=256*4)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(len>6)
	{
		rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		unsigned char i;
		unsigned char Number = 0;
		unsigned char MAKE=0;
		char tmp[8];
		int postion ;
		int TmpInt;
		int IntPostion;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*5);
		rt_memset(SysData,0,AliyunServiceSysLength*5);
		postion =16;
		for(i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			
			TmpFloat=FourByteToFloat(&PlatinumLevelsOfVentilationData[postion+12]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			//隧道风机判断
	
			//排气风机判断
			for(unsigned char j=0;j<2;j++)
			{
				TmpInt=PlatinumLevelsOfVentilationData[postion+8+j];
				for(unsigned char u=0;u<4;u++)
				{
					Number++;
					for(unsigned char s=0;s<10;s++)
					{
						if(Number==ExhaustFan_Buffer[s])
						{
							MAKE=1;
						}
					}
					if(MAKE==1)
					{
						IntPostion = (TmpInt>>(u*2))&0x03;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",IntPostion);
						strcat(SysData,tmp);
						MAKE=0;
					}else
					{
						strcat(SysData,",");
					}
				}
			}
			
			TmpInt=PlatinumLevelsOfVentilationData[postion+10];
			for(unsigned char j=0;j<2;j++)
			{
					Number++;
					for(unsigned char s=0;s<10;s++)
					{
						if(Number==ExhaustFan_Buffer[s])
						{
							MAKE=1;
						}
					}
					if(MAKE==1)
					{
						IntPostion = (TmpInt>>(j*2))&0x03;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",IntPostion);
						strcat(SysData,tmp);
						MAKE=0;
					}else
					{
						strcat(SysData,",");
					}
			}
			Number=0;
			
					for(unsigned char j=0;j<4;j++)
			{
				TmpInt=PlatinumLevelsOfVentilationData[postion+4+j];
				for(unsigned char u=0;u<4;u++)
				{
					Number++;
					for(unsigned char s=0;s<20;s++)
					{
						if(Number==TunnelFan_Buffer[s])
						{
							MAKE=1;
						}
					}
					if(MAKE==1)
					{
						IntPostion = (TmpInt>>(u*2))&0x03;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",IntPostion);
						strcat(SysData,tmp);
						MAKE=0;
					}else
					{
						strcat(SysData,",");
					}
				}
			}
			//上面只判断填充了前16个隧道风机还剩下4个
			for(unsigned char k=0;k<4;k++)
			{
				Number++;
				for(unsigned char s=0;s<20;s++)
				{
					if(Number==TunnelFan_Buffer[s])
					{
						MAKE=1;
					}
				}
				if(MAKE==1)
				{
				
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",0);
					strcat(SysData,tmp);
					MAKE=0;
				}else
				{
					strcat(SysData,",");
				}
			
			}
			
			Number = 0;
			
			
			
			postion =postion+16;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumVentilationlevel,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumVentilationlevel));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumVentilationlevel,SysData);	  

		rt_free(SysData);
		Platinum_Fan_State=0;
		Platinum_GetFan_=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMVENTILATIONLEVEL_CMD;
		}
		
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;

		}
}

void Platinum_Equipment_SpeedFan(uint8_t *dataSource,uint16_t len)
{
		unsigned char i=0;
	  unsigned char pyroad=0;
		char tmp[8];
		int postion ;
		int TmpInt;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =10;
		for(i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			
		  for(unsigned char j=0;j<4;j++)
			{
				for(unsigned char s=0;s<4;s++)
				{
					if((j+1)==(Speedfan_Buffer[s]))
					{
						pyroad = 1;
						break;
					}
				}
				if(pyroad==1)
				{
					TmpInt = dataSource[postion];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
					pyroad=0;
				}
				else{
					strcat(SysData,",");
				}
				postion =postion+1;
			}
		}
		rt_memset(Speedfan_Buffer,0,4);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumSpeedcontrolfanlevel,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumSpeedcontrolfanlevel));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumSpeedcontrolfanlevel,SysData);	  

		rt_free(SysData);
		Platinum_Fan_State=0;
		Platinum_GetFan_=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMSPEEDCONTROFAN_CMD;
		}
		
		
}
void Platinum_Equipment_Ventilation_curtain(uint8_t *dataSource,uint16_t len)
{
		if(len == 231)
		{
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
			if(PlatinumLevelsOfVentilationDataLength<=256*2)
			{
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			}
			else
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
				PlatinumLevelsOfVentilationDataLength=0;
			}
		}
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			unsigned char WindcurtainBuffer[4][30];//二维存储数据数组
			unsigned char VentilationW[30];
			uint16_t postion;
			unsigned char i=0;
			unsigned char sum=0;
			int	TmpInt;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			postion =0;
			for(unsigned char i=0;i<4;i++)
			{
				if(PlatinumLevelsOfVentilationData[postion]!=0)
				{
					for(unsigned char j=0;j<30;j++)
					{
						WindcurtainBuffer[i][j]=PlatinumLevelsOfVentilationData[postion+2+j];
					}
				}
				else
				{
					for(unsigned char j=0;j<30;j++)
					{
						WindcurtainBuffer[i][j]=0xff;
					}
				}
				postion =postion +32;
			}
			postion =postion +32;
			if(PlatinumLevelsOfVentilationData[postion]!=0)
			{
				for(unsigned char j=0;j<30;j++)
				{
					VentilationW[j]=PlatinumLevelsOfVentilationData[postion+2+j];
				}
			}
			else
			{
				for(unsigned char j=0;j<30;j++)
				{
					VentilationW[j]=0xff;
				}
			}
		
			for(unsigned char s=0;s<30;s++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",s+1);
				strcat(SysData,tmp);
				
				for(unsigned char a=0;a<4;a++)
				{
					if(WindcurtainBuffer[a][s]==0xff)
					{
						strcat(SysData,",");
					}
					else
					{
						TmpInt = WindcurtainBuffer[a][s];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
					}
					
				}
				if(VentilationW[s]==0xff)
				{
					strcat(SysData,",");
				}
				else
				{
					TmpInt = VentilationW[s];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
				}
			}
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumCurtainlevel,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumCurtainlevel));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumCurtainlevel,SysData);	  

		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMCURTAINLEVEL_CMD;
		}
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
		}
}

void Platinum_Equipment_Stirfanleve(uint8_t *dataSource,uint16_t len)
{
			uint16_t postion;
			unsigned char i=0;
			unsigned char sta=0;
			int	TmpInt;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			postion = 14;
			unsigned char number=0; 
			for(i=0;i<30;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);	
				
				for(unsigned char j=0;j<4;j++)
				{
					TmpInt = dataSource[postion+j];
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1)==(Frequency_Buffer[s]-8))
						{
							sta=1;
						}
					}
					if(sta==1)
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);	
						sta=0;
					}
					else
					{
						strcat(SysData,",");
	
					}
				}
				
				for(unsigned char s=0;s<3;s++)
				{
					TmpInt = dataSource[postion+s+4];
					for(unsigned char j=0;j<4;j++)
					{
						number++;
						for(unsigned char x=0;x<15;x++)
						{
							if(number==StrFan_Buffer[x])
							{
								sta=1;
							}
						}
						if(sta==1)
						{
						
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",(TmpInt>>(j*2))&(0x03));
							strcat(SysData,tmp);
							sta=0;
						}
						else
						{
						strcat(SysData,",");
						}
					}
				}
				
				for(unsigned char s=0;s<3;s++)
				{
					TmpInt = dataSource[postion+7];
					number++;
					for(unsigned char x=0;x<15;x++)
					{
						if(number==StrFan_Buffer[x])
						{
							sta=1;
						}
					}
					if(sta==1)
					{
						
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",(TmpInt>>(s*2))&(0x03));
						strcat(SysData,tmp);
						sta=0;
					}
					else
					{
						strcat(SysData,",");
					}
				
				}
				number=0;
				postion =postion+8;
			
			}
		rt_memset(Speedfan_Buffer,0,4);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanlevel,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanlevel));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanlevel,SysData);	  

		rt_free(SysData);
		Platinum_Fan_State=0;
		Platinum_GetFan_=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMSTIRRINGFANLEVEL_CMD;
		}
		
}
void Platinum_Equipment_StirfanProgram(uint8_t *dataSource,uint16_t len)
{
			uint16_t postion;
			unsigned char i=0;
			int	TmpInt;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			postion =6;
			for(i=0;i<15;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
				
				TmpInt = dataSource[postion];
				
				for(unsigned char j=0;j<5;j++)
				{
					if(TmpInt&(1<<j))
					{
						strcat(SysData,"1,");
					}
					else
					{
						strcat(SysData,"0,");
					}
				}
				postion =postion+1;
			}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanprogram,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanprogram));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanprogram,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMSTIRRINGFANPROGRAM_CMD;
		}
			

}
void Platinum_Equipment_spray(uint8_t *dataSource,uint16_t len)
{
	 if(len == 247)
	 {
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*2)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			uint16_t postion;
			unsigned char i=0;
			int	TmpInt;
			char tmp[10];
			float TmpFloat;
			int Hour;
			int Minute;
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			postion =16;
		  for(unsigned char i=0;i<20;i++)
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion+2;//结束时间
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion+2;//湿度至
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//目标差值
				TmpFloat=FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;
			}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumsprayer,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumsprayer));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumsprayer,SysData);	  

			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMSPRAYER_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
			
			
		}

}

void Platinum_Equipment_Coolingplate(uint8_t *dataSource,uint16_t len)
{
	 if(len == 247)
	 {
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*2)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
		rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		postion = 16;	//日期
		int Hour = 0;
		int Minute = 0;
		for(uint8_t i=0;i<20;i++)
		{
		
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion+2;//结束时间
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion+2;//湿度至
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//隧道差值
				TmpFloat=FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;

		
	}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumRefrigerationplate,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumRefrigerationplate));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumRefrigerationplate,SysData);	  

			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMREFRIGERATION_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
	
	
}
}

void Platinum_Equipment_NoFeedLight(uint8_t *dataSource,uint16_t len)
{
	if(len == 257)
	 {
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*4)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			uint16_t postion;
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*5);
			rt_memset(SysData,0,AliyunServiceSysLength*5);
			int	TmpInt;
			char tmp[8];
			float TmpFloat;
			postion = 10;	//日期
			int Hour = 0;
			unsigned char  TAG=0;
			int Minute = 0;
			for(unsigned char i =0;i<50;i++)
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+2;
				
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion =postion+2;
				TmpInt=PlatinumLevelsOfVentilationData[postion];
				for(unsigned char j=0;j<4;j++)
				{
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1) == Light_Buffer[s])
						{
							TAG=1;
						}
					}
					if(TAG==1)
					{
						if(TmpInt&(1<<j))
						{
							strcat(SysData,"1,");
						}
						else
						{
							strcat(SysData,"0,");
						}
						TAG=0;
					}
					else
					{
						strcat(SysData,",");
					}
				 }
				postion =postion+6;
			  }
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed,SysData);	  
			Platinum_Fan_State=0;
			Platinum_GetFan_=0;
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMLIGHT_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
			
		 }
}
void Platinum_Equipment_FeedLight(uint8_t *dataSource,uint16_t len)
{
		if(len == 257)
		{
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*4)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			uint16_t postion;
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*5);
			rt_memset(SysData,0,AliyunServiceSysLength*5);
			int	TmpInt;
			char tmp[8];
			float TmpFloat;
			postion = 10;	//日期
			int Hour = 0;
			unsigned char  TAG=0;
			int Minute = 0;
			for(unsigned char i =0;i<50;i++)
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+2;
				
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion =postion+2;
				TmpInt=PlatinumLevelsOfVentilationData[postion];
				for(unsigned char j=0;j<4;j++)
				{
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1) == Light_Buffer[s])
						{
							TAG=1;
						}
					}
					if(TAG==1)
					{
						if(TmpInt&(1<<j))
						{
							strcat(SysData,"1,");
						}
						else
						{
							strcat(SysData,"0,");
						}
						TAG=0;
					}
					else
					{
						strcat(SysData,",");
					}
				 }
				
				 postion =postion+6;
			  }
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed,SysData);	  
			Platinum_Fan_State=0;
			Platinum_GetFan_=0;
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMLIGHT_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
			
		 }

}

void Platinum_Equipment_WaterFeed(uint8_t *dataSource,uint16_t len)
{
	  if(len == 259)
	  {
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*4)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			uint16_t postion;
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*5);
			rt_memset(SysData,0,AliyunServiceSysLength*5);
			int	TmpInt;
			char tmp[8];
			float TmpFloat;
			postion = 8;	
			int Hour = 0;
			unsigned char  TAG=0;
			int Minute = 0;
			for(unsigned char i=0;i<50;i++)
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+2;
				
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion =postion+2;
				TmpInt=PlatinumLevelsOfVentilationData[postion];
				for(unsigned char j=0;j<4;j++)
				{
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1) == Water_Buffer[s])
						{
							TAG=1;
						}
					}
					if(TAG==1)
					{
						if(TmpInt&(1<<j))
						{
							strcat(SysData,"1,");
						}
						else
						{
							strcat(SysData,"0,");
						}
						TAG=0;
					}
					else
					{
						strcat(SysData,",");
					}
				 }
				 postion =postion+1;
				 TmpInt=PlatinumLevelsOfVentilationData[postion];
				 for(unsigned char j=0;j<4;j++)
				 {
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1) == Feed_Buffer[s])
						{
							TAG=1;
						}
					}
					if(TAG==1)
					{
						if(TmpInt&(1<<j))
						{
							strcat(SysData,"1,");
						}
						else
						{
							strcat(SysData,"0,");
						}
						TAG=0;
					}
					else
					{
						strcat(SysData,",");
					}
				 }
				 
				 postion =postion+1;
				 TmpInt=PlatinumLevelsOfVentilationData[postion];
				 for(unsigned char j=0;j<4;j++)
			 	 {
					for(unsigned char s=0;s<4;s++)
					{
						if((j+1) == Heavedragon_Buffer[s])
						{
							TAG=1;
						}
					}
					if(TAG==1)
					{
						if(TmpInt&(1<<j))
						{
							strcat(SysData,"1,");
						}
						else
						{
							strcat(SysData,"0,");
						}
						TAG=0;
					}
					else
					{
						strcat(SysData,",");
					}
				 }
				 
				 postion =postion+2;
				 
			}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterfeed ,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterfeed));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterfeed,SysData);	  
			Platinum_Fan_State=0;
			Platinum_GetFan_=0;
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMWATERFEED_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
			
		}
}
void Platinum_Equipment_AddSystem(uint8_t *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float TmpFloat;
	uint16_t x1[4];
	uint16_t x2[4];
	float    x3[4];
	float    x4[4];
	uint16_t x5[4];
	uint16_t x6[4];
	uint16_t x7[4];
	uint16_t x8[4];
	
	postion=6;	
	for(uint8_t i=0;i<4;i++)
	{
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;//系统X开始时间
		x1[i]=TmpInt;
		
		postion=postion+2;//系统X结束时间
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		x2[i]=TmpInt;
		
		postion=postion+2;//系统X 开始湿度
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		x5[i]=TmpInt;
		
		postion=postion+2;//系统X 结束湿度
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		x6[i]=TmpInt;
					
		postion=postion+2;//系统X 开启（秒）
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		x7[i]=TmpInt;
		
		postion=postion+2;//系统X 关闭（秒）
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		x8[i]=TmpInt;
		
		postion=postion+2;//开始温度
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		x3[i]=TmpFloat;
		
		postion=postion+4;//结束温度
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		x4[i]=TmpFloat;		
		
		postion=postion+4;	
	}
	for(uint8_t i=0;i<8;i++)
	{
		memset(tmp,0,sizeof(tmp));
		switch(i)
		{
			case 0:
			{
				strcat(SysData,"001,");
				sprintf(tmp,"%02d:%02d,",(x1[0]/60),(x1[0]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x1[1]/60),(x1[1]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x1[2]/60),(x1[2]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x1[3]/60),(x1[3]%60));
				strcat(SysData,tmp);
			}break;
			case 1:
			{
				strcat(SysData,"002,");
				sprintf(tmp,"%02d:%02d,",(x2[0]/60),(x2[0]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x2[1]/60),(x2[1]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x2[2]/60),(x2[2]%60));
				strcat(SysData,tmp);
				sprintf(tmp,"%02d:%02d,",(x2[3]/60),(x2[3]%60));
				strcat(SysData,tmp);
			}break;
			case 2:
			{
				strcat(SysData,"003,");
				sprintf(tmp,"%0.1f,",x3[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x3[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x3[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x3[3]);
				strcat(SysData,tmp);
			}break;
			case 3:
			{
				strcat(SysData,"004,");
				sprintf(tmp,"%0.1f,",x4[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x4[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x4[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%0.1f,",x4[3]);
				strcat(SysData,tmp);
			}break;
			
			case 4:
			{
				strcat(SysData,"005,");
				sprintf(tmp,"%d,",x5[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x5[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x5[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x5[3]);
				strcat(SysData,tmp);
			}break;
			case 5:
			{
				strcat(SysData,"006,");
				sprintf(tmp,"%d,",x6[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x6[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x6[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x6[3]);
				strcat(SysData,tmp);
			}break;
			case 6:
			{
				strcat(SysData,"007,");
				sprintf(tmp,"%d,",x7[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x7[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x7[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x7[3]);
				strcat(SysData,tmp);
			}break;
			case 7:
			{
				strcat(SysData,"008,");
				sprintf(tmp,"%d,",x8[0]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x8[1]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x8[2]);
				strcat(SysData,tmp);
				sprintf(tmp,"%d,",x8[3]);
				strcat(SysData,tmp);
			}break;
			default:break;
		}	
	}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumAdditionalsystem ,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumAdditionalsystem));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumAdditionalsystem,SysData);	  
			
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMADDITIONALSYSTEM_CMD;
			}
		
	
}

void Platinum_Equipment_Water_Demand(uint8_t *dataSource,uint16_t len)
{
	 if(len == 255)
	  {
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
				if(PlatinumLevelsOfVentilationData!=NULL)
				{
					rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
					PlatinumLevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("LevelsOfVentilationData malloc failure\n");
				}
		 }
		 if(PlatinumLevelsOfVentilationDataLength<=256*2)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
		 else
		 {
				rt_memset(PlatinumLevelsOfVentilationData,0,256*3);
				PlatinumLevelsOfVentilationDataLength=0;
		 }
	 }
		else if(len>6)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			uint16_t postion;
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			int	TmpInt;
			char tmp[8];
			float TmpFloat;
			postion = 0;	
			int Hour = 0;
			int Minute = 0;
			for(unsigned char i=0;i<50;i++)
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+4;
				
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;//开始时间
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				postion =postion+2;
				
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;//结束时间
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion-4;
				memset(tmp,0,sizeof(tmp));//WOD
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+6;
				
			}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterOnDemand ,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterOnDemand));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterOnDemand,SysData);	  
		
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumMECHANISMWATERONDEMAND_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
			
		}
}


void Platinum_Manage_chicken_stock(uint8_t *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;
		int Sum[3];
		int chick[9];
		unsigned char sum=0;
		int	TmpInt;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
	
		for(i=0;i<9;i++)
		{
			strcat(SysData,"0,");
		}
		postion = 30;
		for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
			Sum[i]=TmpInt;
		}
		postion=6;
		for(i=0;i<6;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			chick[i]=TmpInt;
		}
		for(unsigned char j=0;j<3;j++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
			chick[j+6]=TmpInt;
		}
		TmpInt = Sum[0]-chick[0]-chick[3]-chick[6];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		TmpInt = Sum[1]-chick[1]-chick[4]-chick[7];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		TmpInt = Sum[2]-chick[2]-chick[5]-chick[8];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageChickenstock,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageChickenstock));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageChickenstock,SysData);	  

		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGECHICKENSTOCK_CMD;
		}
}

void Platinum_Manage_Feed_Stock(uint8_t *dataSource,uint16_t len)
{
	if(len==259)
	{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*10);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*10);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PlatinumLevelsOfVentilationDataLength<=256*9)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256*10);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(len>6)
	{
		if(PlatinumLevelsOfVentilationDataLength<=256*10)
		 {
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		 }
 		char *SysData=(char *)rt_malloc(AliyunServiceSysLength * 3);
		rt_memset(SysData,0,AliyunServiceSysLength*3);
		unsigned int postion;
		int TmpInt;
		int Year;
		int month;
		int day;
		char tmp[8];
		float TmpFloat;
		char i=0;
		postion=18;
		for(unsigned char i=0;i<100;i++)
		{
		if(postion<PlatinumLevelsOfVentilationDataLength)
		{
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",i);
		strcat(SysData,tmp);
		
		TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
		if(TmpInt!=0)
		{
		day =TmpInt%32;//天
		Year =((TmpInt/32)-320)/16;
		month =((TmpInt/32)-320)%16;
		
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%2d",day);
		strcat(SysData,tmp);
		strcat(SysData,"-");
		
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%2d",month);
		strcat(SysData,tmp);
		strcat(SysData,"-");
			
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%2d,",Year+2000);
		strcat(SysData,tmp);
		}else
		{
			strcat(SysData,"00");
			strcat(SysData,"-");
			strcat(SysData,"00");
			strcat(SysData,"-");
			strcat(SysData,"00,");
		
		}
		
    postion+=2;
		TmpInt = (int)(FourByteToFloat(&PlatinumLevelsOfVentilationData[postion])*2.203);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PlatinumLevelsOfVentilationData[postion])*2.203);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PlatinumLevelsOfVentilationData[postion])*2.203);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PlatinumLevelsOfVentilationData[postion])*2.203);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion= postion+4;
		
		 
		}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageFeedstock,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageFeedstock));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageFeedstock,SysData);	  

		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGEFEEDSTOCK_CMD;
		}
		
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
	}
}
unsigned char PlTimeDayState=0;
void Platinum_ManageTimeAndDate1(uint8_t *dataSource,uint16_t len)
{
	if(PlTimeDayState == 0)
	{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		}
		if(PlatinumLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			PlTimeDayState=1;
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		unsigned char i=0;
		int Year;
		int month;
		int Hour;
		int Minute;
		int day;
		unsigned char time=0;
		unsigned char sum=0;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		TmpInt =(int)FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);

		Hour = TmpInt/60;
		Minute=TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		
		postion =postion+4;
		TmpInt = PlatinumLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,"-");
		postion=postion+1;
		TmpInt = PlatinumLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,"-");
		postion = postion+1;
		TmpInt = PlatinumLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt+2000);
		strcat(SysData,tmp);
		
		postion = postion+1;
		TmpInt = PlatinumLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageTimedate,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageTimedate));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageTimedate,SysData);	  

		rt_free(SysData);
		PlTimeDayState=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGETOMEDATE_CMD;
		}
		
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
}
}
unsigned char GrowingState=0;
void Platinum_ManageGrowingData(uint8_t *dataSource,uint16_t len)
{
	if(GrowingState == 0)
	{
			if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PlatinumLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			GrowingState=1;
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(GrowingState==1)
	{
		if(PlatinumLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256);
			PlatinumLevelsOfVentilationDataLength=0;
		}
		rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
		TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageGrowthperiodflock,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageGrowthperiodflock));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageGrowthperiodflock,SysData);	  

		rt_free(SysData);
		GrowingState=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGEGROWTHPERIODFLOCK_CMD;
		}
		
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
	}



}

void Platinum_ManageAlarmSet(uint8_t *dataSource,uint16_t len)
{
		uint16_t postion;
		int	TmpInt;
  	float TmpFloat;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 20;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 86;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
	  postion = 78;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =22;
		int Hour;
		int Minute;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute=TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		postion =7;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =24;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =8;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =50;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmsettings,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmsettings));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmsettings,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGEALARMSETTING_CMD;
		}
		
}
void Platinum_ManageNoSaveSuccess(uint8_t *dataSource,uint16_t len)
{
		unsigned char i ;
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<6;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+1;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageSavesuccessfulsettings,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageSavesuccessfulsettings));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageSavesuccessfulsettings,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGESAVESUCCESS_CMD;
		}
}
void Platinum_ManagePassWord(uint8_t *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[32];
	int TmpInt;
	
	postion=6;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=8;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=10;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=12;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=14;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=16;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=18;
	TmpInt=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManagepassword,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManagepassword));
	strcat(Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManagepassword,SysData);	  

	rt_free(SysData);

	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PlatinumMANAGEPASSWORD_CMD;
	}
}

void Platinum_TestRelay(uint8_t *dataSource,uint16_t len)
{
	static unsigned char Relay=0;
	if(Relay==0)
	{
		if(len==259)
		{
			if(PlatinumLevelsOfVentilationData==NULL) 
			{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
			}
			if(PlatinumLevelsOfVentilationDataLength<=256*4)
			{
				rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
				Relay=1;
			}
			else
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*5);
				PlatinumLevelsOfVentilationDataLength=0;
			}
		}
	}
	else if(Relay==1)
	{
		if(len==259)
		{
			Relay=0;
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			int	TmpInt;
			int postion;
			float TmpFloat;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*10);
			rt_memset(SysData,0,AliyunServiceSysLength*10);
			postion =14;
			int x,y=0;
			for(unsigned char i=0;i<20;i++)
			{
				y=(PlatinumLevelsOfVentilationData[postion]&0x00F0)>>4;
				x=PlatinumLevelsOfVentilationData[postion]&0x000F;
				
				
				
				
				if(x==1||x==3)
				{
					x=1;
				}
				else if(x==0||x==2)
				{
					x=0;
				}
				if(y==1||y==3)
				{
					y=1;
				}
				else if(y==0||y==2)
				{
					y=0;
				}
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",Relay_Buffer[i*2]);
				strcat(SysData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",x);
				strcat(SysData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",Relay_Buffer[(i*2)+1]);
				strcat(SysData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",y);
				strcat(SysData,tmp);
				
				postion=postion+14;
			}
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestswitch,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestswitch));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestswitch,SysData);	  
			Platinum_Fan_State=0;
			Platinum_GetFan_=0;
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumTESTSWITCH_CMD;
			}
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	
}
void Platinum_TestAnalogsensor(uint8_t *dataSource,uint16_t len)
{
		int	TmpInt;
		int postion;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*10);
	  rt_memset(SysData,0,AliyunServiceSysLength*10);
		postion =6;
		unsigned char i;
		for(i=0;i<11;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		
	 if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	 rt_memset(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestAnalogsensor,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestAnalogsensor));
	 strcat(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestAnalogsensor,SysData);	  

	 rt_free(SysData);

	 if(AirxxxStatus.recv_systeminfo_cmd==2)
	 {
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PlatinumTESTANALOGSENSOR_CMD;
	 }
		
}
void Platinum_TestDialogsensor(uint8_t *dataSource,uint16_t len)
{
		int	TmpInt;
		int postion;
		
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*10);
	  rt_memset(SysData,0,AliyunServiceSysLength*10);
		postion =6;
		unsigned char i;
		for(i=0;i<8;i++)
		{

			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestDigitalsensor,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestDigitalsensor));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestDigitalsensor,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumTESTDIGITALSENSOR_CMD;
		}
}
void Platinum_TestStaticPressure(uint8_t *dataSource,uint16_t len)
{
		int	TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*10);
	  rt_memset(SysData,0,AliyunServiceSysLength*10);
		postion =6;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestStaticpressure,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestStaticpressure));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestStaticpressure,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumTESTSTATICPRESSURE_CMD;
		}
}

void Platinum_ServiceTemperaturecalibration(uint8_t *dataSource,uint16_t len)
{
		int	TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*10);
	  rt_memset(SysData,0,AliyunServiceSysLength*10);
		postion = 10;
		for(unsigned char i=0;i<9;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			
			TmpInt = FourByteToFloat(&dataSource[postion]);
			if(TmpInt<1)
			{
				strcat(SysData,"------,");
			}
			TmpInt = FourByteToFloat(&dataSource[postion+36]);
			
			if(TmpInt<1)
			{
				strcat(SysData,"------,");
			}
			postion= postion+4;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerTemperaturecalibration,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerTemperaturecalibration));
		strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerTemperaturecalibration,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICETEMPERATURE_CMD;
		}
		
	
	
}


unsigned char HumidityState=0;
void Platinum_ServiceHumiditycalibration(uint8_t *dataSource,uint16_t len)
{
		if(HumidityState==0)
		{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		}
		if(PlatinumLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			HumidityState=1;                                                                                          
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256);
			PlatinumLevelsOfVentilationDataLength=0;
		}
		}
		else if(HumidityState==1)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			
			int	TmpInt;
			int HumBuff[3];
			int YsBuff[3];
			int postion;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			rt_memset(SysData,0,AliyunServiceSysLength);
			postion =0;
			for(unsigned char i=0;i<3;i++)
			{
				TmpInt = (int)FourByteToFloat(&PlatinumLevelsOfVentilationData[postion]);
				HumBuff[i] = TmpInt;
			  postion =postion+4;
			}
			for(unsigned char j=0;j<3;j++)
			{
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				YsBuff[j] = TmpInt;
				postion =postion+2;
			}
			HumidityState=0;
			for(unsigned char s=0;s<3;s++)
			{

				strcat(SysData,"00");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",s+1);
				strcat(SysData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",HumBuff[s]);
				strcat(SysData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",YsBuff[s]);
				strcat(SysData,tmp);
			
			}
			
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerHumiditycalibration,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerHumiditycalibration));
		strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerHumiditycalibration,SysData);	  

		rt_free(SysData);

		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICEHUMIDITY_CMD;
		}
		
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
		PlatinumLevelsOfVentilationDataLength=0;
		
		}
}
void Platinum_ServiceFeedcalibration(uint8_t *dataSource,uint16_t len)
{
			float TmpFloat;
			int postion;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			rt_memset(SysData,0,AliyunServiceSysLength);
			postion =8;
			strcat(SysData,"001,");
			
			for(unsigned char i=0;i<4;i++)
			{
				TmpFloat = FourByteToFloat(&PlatinumLevelsOfVentilationData[postion])*2.203;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion =postion+6;
			}
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeedcalibration,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeedcalibration));
			strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeedcalibration,SysData);	  

			rt_free(SysData);

			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICEFEED_CMD;
			}
}
void Platinum_ServiceWatercalibraton(uint8_t *dataSource,uint16_t len)
{
			float TmpFloat;
			int postion;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			rt_memset(SysData,0,AliyunServiceSysLength);
			postion =10;
			for(unsigned char i=0;i<5;i++)
			{
				TmpFloat = FourByteToFloat(&dataSource[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.3f,",TmpFloat);
				strcat(SysData,tmp);
				postion =postion+4;
			}
			postion = 6;
			TmpFloat = FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.3f,",TmpFloat);
			strcat(SysData,tmp);
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWatercalibration,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWatercalibration));
			strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWatercalibration,SysData);	  
			rt_kprintf("PlatinumSerWatercalibration:%s\n",SysData);
			rt_free(SysData);

			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICEWATER_CMD;
			}

}

void Platinum_ServiceSetpluginterface(uint8_t *dataSource,uint16_t len)
{
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		strcat(SysData,"1,");
		strcat(SysData,"Setting #1,");
		strcat(SysData,"2,");
		strcat(SysData,"Setting #2,");
		strcat(SysData,"3,");
		strcat(SysData,"Setting #3,");
		strcat(SysData,"4,");
		strcat(SysData,"Setting #4,");

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerSetpluginterface,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerSetpluginterface));
		strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerSetpluginterface,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICESETPLUG_CMD;
		}
}
void Platinum_ServiceWaterlinenippleflushing(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		unsigned int StartTimeBuffer[20];
		unsigned int StartMinuteBuffer[20];
		unsigned int StartStateBuffer[20];
		int Hour;
		int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(unsigned char i=0;i<20;i++)
		{
			StartTimeBuffer[i]=dataSource[postion]+dataSource[postion+1]*256;
			postion =postion +2;
			
		}
		for(unsigned char j=0;j<20;j++)
		{
			StartMinuteBuffer[j]=dataSource[postion];
			postion =postion +1;
		}
		for(unsigned char s=0;s<20;s++)
		{
			StartStateBuffer[s] = dataSource[postion];
			postion =postion+1;
		}
		for(unsigned char y=0;y<20;y++)
		{
			TmpInt=StartTimeBuffer[y];
			Hour = TmpInt/60;
			Minute=TmpInt%60;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d",Hour);
			strcat(SysData,tmp);
			strcat(SysData,":");
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d,",Minute);
			strcat(SysData,tmp);
			
			TmpInt=StartMinuteBuffer[y];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			TmpInt=StartStateBuffer[y];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWaterlinenipplerinse,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWaterlinenipplerinse));
		strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWaterlinenipplerinse,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICEWATERLINENIPPLERINSE_CMD;
		}
		
}

void Platinum_ServiceFeederwaterfeeder(uint8_t *dataSource,uint16_t len)
{
//		float TmpFloat;
//		int TmpInt;
//		int postion;
//		char tmp[10];
//		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
//		rt_memset(SysData,0,AliyunServiceSysLength);
//		postion = 6;
//		for(unsigned char i=0;i<10;i++)
//		{
//			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
//			memset(tmp,0,sizeof(tmp));
//			sprintf(tmp,"%d,",TmpInt);
//			strcat(SysData,tmp);
//			postion = postion+4;
//			//根据变量判断后面的继电器的类型跟个数
//			if(FeedWaterState==1)//只有水
//			{
//				TmpFloat=(float)(dataSource[postion]+dataSource[postion+1]*256)/10.0;
//				memset(tmp,0,sizeof(tmp));
//				sprintf(tmp,"%0.1f,",TmpFloat);
//				strcat(SysData,tmp);
//				
//				strcat(SysData,",");
//				postion =postion+2;
//			}
//			else if(FeedWaterState==2)
//			{
//				strcat(SysData,",");
//				
//				TmpFloat=(float)(dataSource[postion]+dataSource[postion+1]*256)/10.0;
//				memset(tmp,0,sizeof(tmp));
//				sprintf(tmp,"%0.1f,",TmpFloat);
//				strcat(SysData,tmp);
//				postion =postion+2;
//			}
//			else if(FeedWaterState==3)
//			{
//				TmpFloat=(float)(dataSource[postion]+dataSource[postion+1]*256)/10.0;
//				memset(tmp,0,sizeof(tmp));
//				sprintf(tmp,"%0.1f,",TmpFloat);
//				strcat(SysData,tmp);
//				postion =postion+2;
//				TmpFloat=(float)(dataSource[postion]+dataSource[postion+1]*256)/10.0;
//				memset(tmp,0,sizeof(tmp));
//				sprintf(tmp,"%0.1f,",TmpFloat);
//				strcat(SysData,tmp);
//				postion =postion+2;
//			
//			}	
//		}
//		
//		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
//		rt_memset(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeeder,0,sizeof(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeeder));
//		strcat(Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeeder,SysData);	  
//		rt_free(SysData);
//		if(AirxxxStatus.recv_systeminfo_cmd==2)
//		{
//			AirxxxStatus.recv_systeminfo_cmd=3;
//			AirxxxStatus.aliyun_service_param_reply=PlatinumSERVICEENNDER_CMD;
//		}
}
void Platinum_Install_Set(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =8;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 15;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 11;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallSetup,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallSetup));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallSetup,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_SETUP_CMD;
		}
}

void Platinum_Install_Relay_Layout(uint8_t *dataSource,uint16_t len)
{
	if(len==259)
	{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*2);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*2);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		}
		if(PlatinumLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			HumidityState=1;                                                                                          
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256*2);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(len>=6)
	{
	
			rt_memset(TunnelFan_Buffer,0,sizeof(TunnelFan_Buffer));//清零
			rt_memset(ExhaustFan_Buffer,0,sizeof(ExhaustFan_Buffer));
			rt_memset(Light_Buffer,0,sizeof(Light_Buffer));
			rt_memset(Water_Buffer,0,sizeof(Water_Buffer));
			rt_memset(Feed_Buffer,0,sizeof(Feed_Buffer));
			rt_memset(Heavedragon_Buffer,0,sizeof(Heavedragon_Buffer));
		
			rt_memset(Relay_Buffer,0,sizeof(Relay_Buffer));
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			int TmpInt;
			int postion;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
			rt_memset(SysData,0,AliyunServiceSysLength*2);
			postion =4;
			for(unsigned char i=0;i<80;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
				TmpInt=PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				if(i<40)//把前40个继电器保存进数组，供测试部分继电器打包发送
				{
					Relay_Buffer[i]=TmpInt;
				}
				
				
				if(TmpInt>=65&&TmpInt<=84)//获取隧道风机编号跟数量
				{
					rt_kprintf("TunnelFan_Buffer %d\n",TmpInt);
					TunnelFan_Buffer[TunnelFan_Sum]=TmpInt-64;//隧道风机
					TunnelFan_Sum++;
				}
				if(TmpInt>=85&&TmpInt<=94)//获取排气风机编号跟数量
				{
					rt_kprintf("ExhaustFan_Buffer %d\n",TmpInt);
					ExhaustFan_Buffer[ExhaustFan_Sum]= TmpInt-84;//排气风机
					ExhaustFan_Sum++;
				}
				if(TmpInt>=95&&TmpInt<=109)
				{
					StrFan_Buffer[StrFan_Sum] = TmpInt-94;
					StrFan_Sum++;
				}
				if(TmpInt>=148&&TmpInt<=151)
				{
					Light_Buffer[light_Sum]=TmpInt-147;
					light_Sum++;
				}
				if(TmpInt>=140&&TmpInt<=143)
				{
					Water_Buffer[Water_Sum]=TmpInt-139;
					Water_Sum++;
				}
				if(TmpInt>=144&&TmpInt<=147)
				{
					Feed_Buffer[Feed_Sum]=TmpInt-143;
					Feed_Sum++;
				}
				if(TmpInt>=152&&TmpInt<=155)
				{
					Heavedragon_Buffer[Heavedragon_Sum]=TmpInt-151;
					Heavedragon_Sum++;
				}
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion = postion +4;
			}
			TunnelFan_Sum=0;
			ExhaustFan_Sum=0;
			StrFan_Sum=0;
			Water_Sum=0;
			Feed_Sum=0;
			Heavedragon_Sum=0;
			light_Sum=0;
		if(Platinum_GetFan_==0)
		{
		rt_kprintf("123\n");
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallRelaylayout,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallRelaylayout));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallRelaylayout,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_RELAYLAYOUT_CMD;
		}
		}
		else if(Platinum_GetFan_==2)
		{
			Platinum_GetFan_=3;
			rt_kprintf("Platinum_GetFan_ %d\n",Platinum_GetFan_);
			rt_free(SysData);
		}
		else 
		{
			Platinum_Fan_State=1;
			Platinum_GetFan_=0;
			rt_kprintf("Platinum_Fan_State %d\n",Platinum_Fan_State);
			rt_free(SysData);
		}
		
	
		if(PlatinumLevelsOfVentilationData!=NULL) 
		{
			rt_free(PlatinumLevelsOfVentilationData);
			PlatinumLevelsOfVentilationData=NULL;
		}
			PlatinumLevelsOfVentilationDataLength=0;
	}
	
}

void Platinum_Install_Analogsensor(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion =7;
		for(unsigned char i=0;i<11;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+1;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogsensor,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogsensor));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogsensor,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_ANALOGSENSOR_CMD;
		}

}

void Platinum_Install_Digitalsensor(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion =7;
		for(unsigned char i=0;i<8;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+1;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallDigitalsensor,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallDigitalsensor));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallDigitalsensor,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL__DIGITALSENSOR_CMD;
		}
}

void Platinum_Install_Analogoutput(uint8_t *dataSource,uint16_t len)
{
		float TmpFloat;
		int TmpInt;
		int postion;
		char tmp[10];
		unsigned char j=0;
		unsigned char s=0;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		memset(Frequency_Buffer,0,4);
		postion = 6;
		for(unsigned char i=0;i<8;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			
			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			
			if(TmpInt>=9&&TmpInt<=12)
			{
				rt_kprintf("TmpInt%d\n",TmpInt);
				Frequency_Buffer[s]=TmpInt;
				s++;
			}
			if(TmpInt>=1&&TmpInt<=4)
			{
				
				Speedfan_Buffer[j]=TmpInt;
				j++;
			}
			
			TmpFloat = FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+4;
			TmpFloat = FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+4;
			
		}
		if(Platinum_GetFan_==0)
		{
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogoutput,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogoutput));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogoutput,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_ANALOGOUTPUT_CMD;
		}
		}
		else
		{
			Platinum_GetFan_=0;
			Platinum_Fan_State=1;
			rt_free(SysData);
			
		}
}
void Platinum_Install_windcurtainset(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion = 6;
		for(unsigned char i=0;i<16;i++)
		{
			if((i==0)||(i==2)||(i==4)||(i==6)||(i==8)||(i==10)||(i==12)||(i==14))
			{
				strcat(SysData,"00");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i);
				strcat(SysData,tmp);
			}
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCurtainsetting,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCurtainsetting));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCurtainsetting,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_CURTAINSETTING_CMD;
		}
}

void Platinum_Install_Temperaturedefined(uint8_t *dataSource,uint16_t len)
{
		if(len==259)
	{
		if(PlatinumLevelsOfVentilationData==NULL) 
		{
				PlatinumLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PlatinumLevelsOfVentilationData!=NULL)
			{
				rt_memset(PlatinumLevelsOfVentilationData,0,256*6);
				PlatinumLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		}
		if(PlatinumLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			HumidityState=1;                                                                                          
		}
		else
		{
			rt_memset(PlatinumLevelsOfVentilationData,0,256*6);
			PlatinumLevelsOfVentilationDataLength=0;
		}
	}
	else if(len>=6)
	{
			rt_memcpy(&PlatinumLevelsOfVentilationData[PlatinumLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PlatinumLevelsOfVentilationDataLength=PlatinumLevelsOfVentilationDataLength+(len-7);
			int TmpInt;
			int postion;
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
			rt_memset(SysData,0,AliyunServiceSysLength*6);
			postion =6;
			for(unsigned char i=0;i<100;i++)
			{
				TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				if(TmpInt!=0)
				{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
					
				postion =postion+2;
				TmpInt = PlatinumLevelsOfVentilationData[postion]+PlatinumLevelsOfVentilationData[postion+1]*256;
				for(unsigned char j=0;j<9;j++)
				{
					if(TmpInt&(1<<j))
					{
						strcat(SysData,"1,");
					}
					else
					{
						strcat(SysData,"0,");
					}
				}
				
				
					postion =postion+4;
				}
				else
				{
					for(unsigned char j=0;j<10;j++)
					{
						strcat(SysData,",");
					}
					postion= postion+6;
				}
			}
			
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallTemperaturedefinition,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallTemperaturedefinition));
			strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallTemperaturedefinition,SysData);	  

			rt_free(SysData);

			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_TEMPERATUREDE_CMD;
			}
		
			if(PlatinumLevelsOfVentilationData!=NULL) 
			{
				rt_free(PlatinumLevelsOfVentilationData);
				PlatinumLevelsOfVentilationData=NULL;
			}
			PlatinumLevelsOfVentilationDataLength=0;
		
	}
}

void Platinum_Install_Fan_capacity(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion = 74;
		for(unsigned char i=0;i<10;i++)
		{
			TmpInt = FourByteToFloat(&dataSource[postion])/1.699;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+4;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallFanaircapacity,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallFanaircapacity));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallFanaircapacity,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_FANAIRCAPACITY_CMD;
		}

}
void Platinum_Install_Henhousesize(uint8_t *dataSource,uint16_t len)
{
		float TmpFloat;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion = 6;
		for(unsigned char i=0;i<4;i++)
		{
			TmpFloat = FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCoopsize,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCoopsize));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCoopsize,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_COOPSIZE_CMD;
		}
		
}

void Platinum_Install_Communication(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int postion;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion = 6;

		TmpInt=dataSource[postion];
		switch(TmpInt)
		{
			case 0:strcat(SysData,"1200,");break;
			case 1:strcat(SysData,"2400,");break;
			case 2:strcat(SysData,"4800,");break;
			case 3:strcat(SysData,"9600,");break;
			case 4:strcat(SysData,"19200,");break;
			case 5:strcat(SysData,"38400,");break;
			case 6:strcat(SysData,"57600,");break;
			case 7:strcat(SysData,"115200,");break;
			default:strcat(SysData,"0,");break;
		}
	
		postion=7;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCommunication,0,sizeof(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCommunication));
		strcat(Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCommunication,SysData);	  
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PlatinumINSTALL_COMMUNICATION_CMD;
		}
}



int Ac2000DataParsePlatinum(uint8_t *data,unsigned short Counter)
{
	uint8_t result=0;
	uint16_t cmd_code=data[2]*256+data[3];
	rt_kprintf("AC2000 PLATINUM_CODE:%04X %d\r\n",cmd_code,Counter);
	//my_log(data,Counter);
  if(Ac2000Data!=NULL)
	{
		switch(cmd_code)
		{
			case 0x0B79://查询主屏
			{
				if(Coll_System==0)
				{
					if(Counter==227)Parse_MainScreen_Platinum(data,Counter);//ok
				}
				else
				{
					Platinum_Coll_System_Waterstock(data,Counter);
				}
			}break;
//		case 0x0252://曲线状态
//		{
//			if(Counter==33)Parse_curve_state_Platinum(data,Counter);//ok
//		}break;
//		case 0x062F://饲料校准
//		{
//			if(Counter==31)GetAC2000ServiceFeedFactor(data,Counter);
//		}break;
//		case 0x01A1://饲料箱
//		{
//			if(Counter==67)Parse_FeedLeft_Platinum(data,Counter);
//		}break;
//		case 0x024B://水和饲料
//		{
//			if(Counter==55)Parse_FeedWater_Platinum(data,Counter);
//		}break;
//		case 0x0002://报警查询
//		{
//			Parse_Warn_Platinum(data,Counter);
//		}break;
//		case 0x0248://继电器状态
//		{
//			if(Counter==259)
//			{
//				Parse_RelyTest1_Platinum(data,Counter);
//			}
//			else if(Counter==49)
//			{
//				Parse_RelyTest2_Platinum(data,Counter);
//			}
//		}break;
			case 0x051D:
			{
				Platinum_Coll_Temperaturecalibration(data,Counter);
			}break;
			case 0x0122:
			{
				Platinum_Coll_Humidity(data,Counter);
			}break;
			case 0x0120:
			{
				Platinum_Coll_Min_Max(data,Counter);
			}break;
			case 0x032A:
			{
				Platinum_Coll_Static_Pressure(data,Counter);
			}break;
			case 0x472B:
			{
				Platinum_Coll_ControlMode(data,Counter);
			}break;
			case 0x0265:
			{
				Platinum_Coll_System_temperaturecali(data,Counter);//温度曲线
			}break;
			case 0x0266:
			{
				Platinum_Coll_System_Radiant_heater(data,Counter);//辐射加热器
				
			}break;
			case 0x01A6:
			{
				Platinum_Coll_System_VariableHeater(data,Counter);//VH
			}break;
			case 0x026C:
			{
				Platinum_Coll_System_Humidity_treatment(data,Counter);//湿度处理
			}break;
			case 0x0286:
			{
				Platinum_Coll_Co2Mode(data,Counter);//Co2处理
			}break;
			case 0x026D:
			{
				Platinum_Coll_Min_Max_level(data,Counter);//最低最高级别
			}break;
			case 0x0569:
			{
				Platinum_Coll_System_Staticpressure(data,Counter);//静压处理
			}break;
			case 0x0267:
			{
				Platinum_Level_Ventilation(data,Counter);//通风等级
			}break;
			case 0x086E:
			{
				Platinum_Coll_System_Shutter_Roof(data,Counter);//卷帘和屋顶
			}break;
			case 0x056A:
			{
				Platinum_Coll_System_WetCurtain(data,Counter);//湿帘
			}break;
			case 0x046B:
			{
				Platinum_Coll_System_humidifier(data,Counter);//加湿器
			}break;
			case 0x056F:
			{
				Platinum_Coll_System_light(data,Counter);//灯光
			}break;
			case 0x0287:
			{
				Platinum_Coll_System_Water_Feed(data,Counter);
			}break;
			case 0x0270:
			{
				Platinum_Coll_System_Activate_funnel(data,Counter);
			}break;
			case 0x0271:
			{
				Platinum_Coll_System_EmerGency_Setting(data,Counter);//
			}break;
		
			case 0x0272:
			{
				Platinum_Coll_Alarmset(data,Counter);//报警设置
			}break;
			case 0x0192:
			{
				Platinum_Coll_Co2Treatment(data,Counter);//CO2处理
			}break;
			//设备
			case 0x0199:
			{
				Platinum_Equipment_Ventilationlevel(data,Counter);
			}break;
			case 0x0544:
			{
				Platinum_Equipment_SpeedFan(data,Counter);
			}break;
			case 0x0437:
			{
				Platinum_Equipment_Ventilation_curtain(data,Counter);
			}break;
			
			case 0x061E:
			{
				Platinum_Equipment_Stirfanleve(data,Counter);
			}break;
			
			case 0x023D:
			{
				Platinum_Equipment_StirfanProgram(data,Counter);
			}break;			
			case 0x0125:
			{
				Platinum_Equipment_spray(data,Counter);
			}break;
			case 0x0124:
			{
				Platinum_Equipment_Coolingplate(data,Counter);
			}break;
			
			case 0x0526:
			{
				Platinum_Equipment_NoFeedLight(data,Counter);
			}break;
			
			case 0x0595:
			{
				Platinum_Equipment_FeedLight(data,Counter);
			}break;
			case 0x0627:
			{
				Platinum_Equipment_WaterFeed(data,Counter);
			}break;
			case 0x0128:
			{
				Platinum_Equipment_AddSystem(data,Counter);
			}break;
			case 0x01A5:
			{
				Platinum_Equipment_Water_Demand(data,Counter);
			}break;
			//管理
			case 0x010C://鸡的存量
			{
				Platinum_Manage_chicken_stock(data,Counter);
			}break;
			case 0x0361:
			{
				Platinum_Manage_Feed_Stock(data,Counter);
			}break;
			case (0x015d): 
			{
				if(Counter==11)Platinum_ManageTimeAndDate1(data,Counter); //管理--时间和日期
			}break;
			case (0x025c):
			{
				if(Counter==11)Platinum_ManageTimeAndDate1(data,Counter);//管理--时间和日期
			}break;
			case 0x010B:
			{
				if(Counter==9)Platinum_ManageGrowingData(data,Counter);
			}break;
			case 0x010D:
			{
				if(Counter == 11)Platinum_ManageGrowingData(data,Counter);
			}break;
			case 0x093B:
			{
				if(Counter == 95)Platinum_ManageAlarmSet(data,Counter);//警报设置
			}break;
			case 0x0191:
			{
				if(Counter==13) Platinum_ManageNoSaveSuccess(data,Counter);
			}break;
			case 0x0112:
			{
				if(Counter == 21) Platinum_ManagePassWord(data,Counter);
			}break;
			//测试
			case 0x0248:
			{
				Platinum_TestRelay(data,Counter);
			}break;
			case 0x027A:
			{
				Platinum_TestAnalogsensor(data,Counter);
			}break;
			case 0x017B:
			{
				Platinum_TestDialogsensor(data,Counter);
			}break;
			case 0x017C:
			{
				Platinum_TestStaticPressure(data,Counter);
			}break;
			//服务
			case 0x4313:
			{
				Platinum_ServiceTemperaturecalibration(data,Counter);
			}break;
			case 0x0210:
			{
				Platinum_ServiceHumiditycalibration(data,Counter);
			}break;
			case 0x024C:
			{
				Platinum_ServiceHumiditycalibration(data,Counter);
			}break;
			case 0x062F:
			{
				Platinum_ServiceFeedcalibration(data,Counter);
			}break;
			case 0x0530:
			{
				Platinum_ServiceWatercalibraton(data,Counter);
			}break;
			case 0x0246:
			{
				Platinum_ServiceSetpluginterface(data,Counter);
			}break;
			case 0x078B:
			{
				Platinum_ServiceWaterlinenippleflushing(data,Counter);//水线乳头冲洗
			}break;
			case 0x0789:
			{
				Platinum_ServiceFeederwaterfeeder(data,Counter);//喂水器喂料器
			}break;
			//安装
			case 0x0731:
			{
				Platinum_Install_Set(data,Counter);
			}break;
			case 0x0B32:
			{
				Platinum_Install_Relay_Layout(data,Counter);
			}break;
			case 0x4733:
			{
				Platinum_Install_Analogsensor(data,Counter);
			}break;
			case 0x0834:
			{
				Platinum_Install_Digitalsensor(data,Counter);
			}break;
			case 0x8540:
			{
				Platinum_Install_Analogoutput(data,Counter);
			}break;
			case 0x0436:
			{
				Platinum_Install_windcurtainset(data,Counter);
			}break;
			case 0x0A35:
			{
				Platinum_Install_Temperaturedefined(data,Counter);
			}break;
			case 0X0542:
			{
				Platinum_Install_Fan_capacity(data,Counter);
			}break;
			case 0x0543:
			{
				Platinum_Install_Henhousesize(data,Counter);
			}break;
			case 0x0117:
			{
				Platinum_Install_Communication(data,Counter);
			}break;
			default:
				result=1;
				rt_kprintf("unknowed->");
			//	my_log(data,Counter);
			break;
	}
}
	return result;
}

