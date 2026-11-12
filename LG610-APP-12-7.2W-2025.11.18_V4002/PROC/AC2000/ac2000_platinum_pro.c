#include "ac2000_platinum_pro.h"
//#include "bsp_clkconfig.h"
#include "data_table.h"
//#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "humiture.h"
#include <time.h>
#include "mynet.h"
extern rt_size_t send_to_ac2000(const void* buffer, rt_size_t size);
//#include "mynet.h"
/**************注意*************************
	AC2000设备信息
	设备颜色：黑色
	设备名称：ROTEM PLATINUM PRO
	设备系统版本：6.18r05
****************************************/

unsigned char *PorLevelsOfVentilationData=NULL;
uint16_t PorLevelsOfVentilationDataLength=0;
int PorRecvFlag;
PorAc2000Params *Ac2000PorParams=NULL;
unsigned int Number_relays=0;//提前获取继电器数量
unsigned int Number_relaystate=0;
unsigned char WaterSum=0;
unsigned char WaterBuffer[4];
extern unsigned char _Findstate;
extern unsigned char ACCESS_DIFFERENT;
const unsigned char Ac2000PorContorTemp[3][AC2000ProCmdLength]={
	 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x43,0x1D,0x01,0x00,0x00,0xFC,0x5F},
	 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x43,0x1D,0x01,0xFC,0x00,0xFC,0x5B}, 	
   {0XFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x43,0x1D,0x01,0xF8,0x01,0x28,0x84} 	
};

const unsigned char Ac2000PorContorHumitidy[1][AC2000ProCmdLength]={
	 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x22,0x01,0x00,0x00,0xA8,0xCE } 	
};
const unsigned char Ac2000PorContorMinMax[1][AC2000ProCmdLength]={
	 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x81,0x20,0x01,0x00,0x00,0x7C,0x20}
};

const unsigned char Ac2000PorContorStaticpressure[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x2A,0x01,0x00,0x00,0x30,0x5E}
};

const unsigned char Ac2000PorContorMode[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x42,0x2B,0x01,0x00,0x00,0x09,0x79}
};
const unsigned char Ac2000PorContorSystem[19][AC2000ProCmdLength]=//系统参数
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x65,0x01,0x00,0x00,0x1C,0x85},//温度曲线
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x62,0x01,0x00,0x00,0x0C,0x72},//cycle heater
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0x66,0x01,0x00,0x00,0x0A,0xB4},//辐射加热器
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0xA6,0x01,0x00,0x00,0x12,0xBE},//variable heater 
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x6C,0x01,0x00,0x00,0x10,0x82},//湿度处理 Humidity Treatment 
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x86,0x01,0x00,0x00,0x0F,0x9B},//CO2处理  Co2TREATMENT BY
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0xE7,0x01,0x00,0x00,0x06,0xF2},//氨气处理
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x42,0x69,0x01,0x00,0x00,0x1A,0xC8},//静压
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x67,0x01,0x00,0x00,0x1C,0x88},//通风等级，隧道
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x6E,0x01,0x00,0x00,0x23,0x99},//CURTAINS 屋顶
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0xF9,0x01,0x00,0x00,0x0C,0x0B},//VCC
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x6A,0x01,0x00,0x00,0x14,0x83},//湿帘COOL PAD 
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x6B,0x01,0x00,0x00,0x10,0x80},//加湿器
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x6F,0x01,0x00,0x00,0x10,0x83},//灯光
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x87,0x01,0x00,0x00,0x14,0x9F},//水和饲料
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x80,0x01,0x00,0x00,0x2C,0xB0},//饲料存量
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xDE,0x01,0x00,0x00,0x20,0x02},// NATURAL ENTRANCE CONDITIONS       WIND & RAIN EFFECT
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0xEF,0x01,0x00,0x00,0x08,0xFB},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x72,0x01,0x00,0x00,0x01,0x77},//报警设置
	
};
const unsigned char Ac2000PorContorCo2processing[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x92,0x01,0x00,0x00,0xD2,0x68}
};
const unsigned char Ac2000PorContorAmmoniatreatment[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x07,0xE1,0x01,0x00,0x00,0xD2,0xBD}
};
const unsigned char Ac2000PorMechVentilationlevel[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x04,0x4C},//注意长度一致
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x42,0x99,0x01,0x00,0x00,0xFE,0xDC},

};
const unsigned char Ac2000PorMechSpeedcontrolfanlevel[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x44,0x01,0x00,0x00,0x04,0x4E},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x44,0x01,0x00,0x00,0xBC,0x07},
};

const unsigned char Ac2000PorMechCurtainlevel[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x37,0x01,0x00,0x00,0x04,0x41},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x37,0x01,0x00,0x00,0x66,0xA1},
};
const unsigned char Ac2000PorMechStirfan[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x1E,0x01,0x00,0x00,0x04,0x28},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x1E,0x01,0x00,0x00,0x44,0x67},
};
const unsigned char Ac2000PorMechstirringfanprogarm[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x3D,0x01,0x00,0x00,0x0F,0x50}
};
const unsigned char Ac2000PorMechsprayer[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x25,0x01,0x00,0x00,0xF0,0x19 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x25,0x01,0xF0,0x00,0x60,0x79 },
};
const unsigned char Ac2000PorMechRefrigerationplate[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x24,0x01,0x00,0x00,0xFE,0x26 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x24,0x01,0xFE,0x00,0x52,0x78 },
};

const unsigned char Ac2000PorMechLightNoFeed[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xCE,0x01,0x00,0x00,0xF0,0x02 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xCE,0x01,0xF0,0x00,0xF0,0xF2 },
};
const unsigned char Ac2000PorMechLightFeed[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xCF,0x01,0x00,0x00,0xF0,0x03 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xCF,0x01,0x00,0x00,0xF0,0x03 },
};

const unsigned char Ac2000PorMechWaterFeed[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xD8,0x01,0x00,0x00,0xEA,0x06 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0xD8,0x01,0xEA,0x00,0xEA,0xF0 },

};
const unsigned char Ac2000ProMechAddSystem[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0x28,0x01,0x00,0x00,0x50,0xBC },
};

const unsigned char Ac2000ProMechWaterOnDemand[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0xA5,0x01,0x00,0x00,0x04,0xAF },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x83,0xA5,0x01,0x00,0x00,0xFE,0x29 },

};

const unsigned char Ac2000ProMechNatural[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x42,0xE0,0x01,0x00,0x00,0x2C,0x51 }
};

const unsigned char Ac2000ProMechFeedScale[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0xC2,0x01,0x00,0x00,0xFE,0xC7 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0xC2,0x01,0xFE,0x00,0xF2,0xB9 },
};

const unsigned char Ac2000ManageProChicken[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x0C,0x01,0x00,0x00,0x30,0x41 }
};

const unsigned char Ac2000ProManageFeed[15][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x00,0x00,0xEE,0x55 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xEE,0x00,0xEE,0x43 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xDC,0x01,0xEE,0x32 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xCA,0x02,0xEE,0x22 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xB8,0x03,0xEE,0x10 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0xA6,0x04,0xEE,0x00-1},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x94,0x05,0xEE,0xEE},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x82,0x06,0xEE,0xDD },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x70,0x07,0xEE,0xCC },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x5E,0x08,0xEE,0xBB },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x4C,0x09,0xEE,0xAA },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x3A,0x0A,0xEE,0x99 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x28,0x0B,0xEE,0x88 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x16,0x0C,0xEE,0x78 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x61,0x01,0x04,0x0D,0x90,0x09 },
};

const unsigned char Ac2000ProMangageTimeDay[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x5D,0x01,0x00,0x00,0x04,0x65 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x5C,0x01,0x00,0x00,0x04,0x64 },
};

const unsigned char Ac2000Chickens[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x41,0x0B,0x01,0x00,0x00,0x02,0x51 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x0D,0x01,0x00,0x00,0x04,0x15 },
};

const unsigned char Ac2000AlarmSetting[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x08,0x3B,0x01,0x00,0x00,0xF0,0x36 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x08,0x3B,0x01,0xF0,0x00,0xF0,0x26 },
};

const unsigned char Ac2000NoSuccessfulSet[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x91,0x01,0x00,0x00,0x06,0x9B }
};

const unsigned char Ac2000MangPassword[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x12,0x01,0x00,0x00,0x0E,0x24 }
};

const unsigned char Ac2000TestSwitch[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x48,0x01,0x00,0x00,0xFD,0x4A },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x48,0x01,0xFD,0x00,0xCF,0x19 },
};
const unsigned char Ac2000TestAnalogsensor[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x7A,0x01,0x00,0x00,0xA8,0x28 },

};
const unsigned char Ac2000TestDialogSensor[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x7B,0x01,0x00,0x00,0x6C,0xEC }
};

const unsigned char Ac2000TestStaticPressure[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x7C,0x01,0x00,0x00,0x02,0x82 }
};

const unsigned char Ac2000Server_TemTemperaturecalibration[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x13,0x01,0x00,0x00,0xFA,0x11 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x13,0x01,0xFA,0x00,0x6E,0x7F },
};
const unsigned char Ac2000ServerHumidtiycalibration[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x01,0x02,0x11,0x01,0x00,0x00,0x1E,0x35 },
};

const unsigned char Ac2000ServerFeedCalibration[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x2F,0x01,0x00,0x00,0x3C,0x71 }
};
const unsigned char Ac2000ServerWaterCalibration[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x30,0x01,0x00,0x00,0x58,0x8D }
};
const unsigned char Ac2000ServerPluginterface[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x46,0x01,0x00,0x00,0x90,0xDA }
};

const unsigned char Ac2000ServerWaterWaterlinenipple[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x8B,0x01,0x00,0x00,0x50,0xDF}
};
const unsigned char Ac2000ServerFeeder[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x89,0x01,0x00,0x00,0x50,0xDE }
};
const unsigned char Ac2000ServerAmmoniaCalibration[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0xE3,0x01,0x00,0x00,0x0C,0xF4 }
};
const unsigned char Ac2000InstallCurtainSetting[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x36,0x01,0x00,0x00,0x04,0x40 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x36,0x01,0x00,0x00,0x0F,0x4B },
};

const unsigned char Ac2000InstallTemperaturedefinition[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x35,0x01,0x00,0x00,0x04,0x3F },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x35,0x01,0x00,0x00,0xF0,0x2B },
};
const unsigned char Ac2000InstallFanAirCapacity[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x04,0x4C},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x08,0x4E},
};
const unsigned char Ac2000InstallChickSizeof[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x43,0x01,0x00,0x00,0x10,0x57}
};

const unsigned char Ac2000InstallCommunication[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x17,0x01,0x00,0x00,0x03,0x1E}
};

const unsigned char Ac2000InstallSetup[1][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x01,0x08,0x0E,0x01,0x00,0x00,0x2B,0x45}
};
const unsigned char Ac2000InstallRelaysensorfind[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x04,0x4C},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x98,0xDE},
};
 const unsigned char Ac2000InstallRelaysensor[2][AC2000ProCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x1C,0x01,0x00,0x00,0x04,0x26 },
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x04,0x1C,0x01,0x00,0x00,0xF2,0x15 },

};
void Ac2000DataProMalloc(void)
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
				rt_kprintf("Ac2000OldDataPro malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000OldDataPro malloc failed\r\n");
			}		
		}
		if(Ac2000OldData==NULL)
		{
			Ac2000OldData=(AC2000_DATA_TYPE *)rt_malloc(sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);	
			if(Ac2000OldData!=NULL)
			{
				rt_memset(Ac2000OldData,0,sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);
				rt_kprintf("Ac2000OldDataPro malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000OldDataPro malloc failed\r\n");
			}	
		}
		
		
		if(Ac2000PorParams==NULL)
		{
			Ac2000PorParams=(PorAc2000Params *)rt_malloc(sizeof(PorAc2000Params));
			if(Ac2000PorParams!=NULL)
			{
				rt_memset(Ac2000PorParams,0,sizeof(PorAc2000Params));
				int len=sizeof(PorAc2000Params);
				rt_kprintf("\nAc2000PorParams size:%d\n",len);
			}
			else
			{
				rt_kprintf("Ac2000PorParams创建失败!\n");
			}
		}
		
}


void CreateAC2000PorNormal(const uint8_t CmdSource[][AC2000ProCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
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
void PorOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000ProCmdLength],uint8_t cmd_num,uint8_t *cmd_index)
{

	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000ProCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,(*cmd_index));   //参数赋值
		(*cmd_index)++;
		PorRecvFlag=(AC2000SendBuff[16]+7);
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);	
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}
}





void RelayCreateAC2000PorNormal(const uint8_t CmdSource[][AC2000ProCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,unsigned short dex,unsigned short prov,unsigned char state,unsigned short prov1)
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
unsigned char s=0;
unsigned char porstate=1;

void relaySend(const uint8_t Cmd[][AC2000ProCmdLength],unsigned short porv,unsigned char difference)
{
	unsigned short Quotient=0;
	unsigned short Morethan=0;
	if(difference==1)
	{
	
	if(s==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000ProCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,0);   //参数赋值
		PorRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//    s=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	else if(s==1)
	{
		Quotient =Number_relays/porv;
		Morethan = Number_relays%porv;
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 0)&&(Morethan==0))
			{
				AirxxxStatus.recv_systeminfo_cmd=2;	
				s=0;		
				PorWrite_data_Empty();
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
			
			}
			else if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,porv,0,porv);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,Morethan,0,Morethan);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				 if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,1,Morethan);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;
					my_log(AC2000SendBuff,18);
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					my_log(AC2000SendBuff,18);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
	 }
 }
	
 else if(difference==0)//发送后不会结束，跳转状态
 {
	if(s==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000ProCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,0);   //参数赋值
		PorRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//    s=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	else if(s==1)
	{
		Quotient =Number_relays/porv;
		Morethan = Number_relays%porv;
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,porv,0,porv);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				_Findstate=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,Morethan,0,Morethan);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				_Findstate=1;
				porstate=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					_Findstate=1;
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,1,Morethan);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					_Findstate=1;
					my_log(AC2000SendBuff,18);
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					my_log(AC2000SendBuff,18);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
	 }
 }
 else if(difference==2)//通风等级计算方式
 {
	 if(s==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000ProCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,0);   //参数赋值
		PorRecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//    s=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(s==1)
	{
		Quotient =Number_relaystate/porv;
		Morethan = Number_relaystate%porv;
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,porv,0,porv);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,Morethan,0,Morethan);
				PorRecvFlag=(AC2000SendBuff[16]+7);

				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);

					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,1,Morethan);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					my_log(AC2000SendBuff,18);
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					my_log(AC2000SendBuff,18);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 }
 else if(difference==3)//没有帧头
 {

		Quotient =Number_relaystate/porv;
		Morethan = Number_relaystate%porv;
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,porv,0,porv);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				_Findstate=0;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000ProCmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,1,Morethan,0,Morethan);
				PorRecvFlag=(AC2000SendBuff[16]+7);
				_Findstate=0;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				s=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					_Findstate=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(porstate>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,1,Morethan);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					_Findstate=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	            
					my_log(AC2000SendBuff,18);
					porstate=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					s=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000ProCmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC2000PorNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_pro_house,1,porstate,porv,0,porv);
					my_log(AC2000SendBuff,18);
					PorRecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					porstate=porstate+1;
					send_to_ac2000((const void*) AC2000SendBuff, AC2000ProCmdLength);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000ProCmdLength,2000,3,&PorRecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 
}






#if SINGLE_PRO_TEST==1
const unsigned char AC2000ProCmd[AC2000_Pro_CmdNum][AC2000CmdLength]=/*命令码表*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},//连接 
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x01,0x2c,0x01,0x00,0x00,0x16,0x47}
};
#else
//const unsigned char AC2000ProCmd[AC2000_Pro_CmdNum][AC2000CmdLength]=/*命令码表*/
//{
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},//扫描鸡舍号
//  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x0C,0x11},//报警查询
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x04,0x05,0x01,0x00,0x00,0xfe,0x1b},//主屏幕1
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x04,0x05,0x01,0xfc,0x01,0x59,0x63},//主屏幕2
//	//{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x7c,0x01,0x00,0x00,0x02,0x82},//静压AD
//  {0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x01,0x2c,0x01,0x00,0x00,0x16,0x47},//温度和湿度状态
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x01,0x2d,0x01,0x00,0x00,0x20,0x52},//曲线状态
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x02,0x11,0x01,0x00,0x00,0x1e,0x35},//湿度校准
//  {0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x00,0x04,0x16,0x01,0x00,0x00,0x55,0x73},//水和食料
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x13,0x01,0x00,0x00,0xfa,0x11},//读取温度1 24
//	//{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x13,0x01,0xfa,0x00,0x6e,0x7F},//读取温度2 12
//	
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0x00,0x00,0xe6,0x34},//读取继电器 一组 230==23*10
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0x00,0x00,0xfd,0x4b},//读取继电器 二组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xfd,0x00,0xcf,0x1a},//读取继电器 二组 207==23*9
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0x00,0x00,0xfd,0x4b},//读取继电器 三组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xfd,0x00,0xfd,0x48},//读取继电器 三组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xfa,0x01,0xb8,0x00},//读取继电器 三组 184==23*8
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0x00,0x00,0xfd,0x4b},//读取继电器 四组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xfd,0x00,0xfd,0x48},//读取继电器 四组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xfa,0x01,0xfd,0x46},//读取继电器 四组 253==23*11
//	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x48,0x01,0xf7,0x02,0xa1,0xe8},//读取继电器 四组 161==23*7
//};
#endif

const unsigned char AC2000ProCmd[3][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},//扫描鸡舍号
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x01,0x02,0x00,0x01,0x04,0x05,0x01,0x00,0x00,0xFE,0x0B},//主屏幕1
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x07,0x01,0x00,0x00,0x80,0x8A} //报警

};

/*
ff 41 54 54 45 00 00 01 04 00 01 01 11 01 00 00 04 1d 

recv pro:11->
AC2000 PRO CODE:0011 11

01 14 00 11 01 27 1e 00 00 00 1e 

41 54 54 45 04 
*/

void CreateAC2000ProSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = AC2000ProCmd[CmdIndex][0];
	AC2000_SendBuff[1] = AC2000ProCmd[CmdIndex][1];
	AC2000_SendBuff[2] = AC2000ProCmd[CmdIndex][2];
	AC2000_SendBuff[3] = AC2000ProCmd[CmdIndex][3];
	AC2000_SendBuff[4] = AC2000ProCmd[CmdIndex][4];
	AC2000_SendBuff[5] = AC2000ProCmd[CmdIndex][5];
	AC2000_SendBuff[6] = AC2000ProCmd[CmdIndex][6];
	AC2000_SendBuff[7] = AC2000ProCmd[CmdIndex][7];
	AC2000_SendBuff[8] = AC2000ProCmd[CmdIndex][8];
	AC2000_SendBuff[9] = AC2000ProCmd[CmdIndex][9];
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000ProCmd[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000ProCmd[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000ProCmd[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000ProCmd[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000ProCmd[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000ProCmd[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000ProCmd[CmdIndex][17]+HenHouseNum;
	
	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	AC2000_SendBuff[17]=sum;
	
//	rt_kprintf("mcu pro send:\r\n");
//	for(uint16_t i=0;i<18;i++)
//	{
//		rt_kprintf("%02x ",AC2000_SendBuff[i]);
//	}
//	rt_kprintf("\r\n");
}
void connect_ac2000_pro(uint8_t *buff,uint8_t house)
{
	CreateAC2000ProSendBuf(buff,house,0);
}

static unsigned char Parse_MainScreen_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 04 05 01 1f 
	00 bc 04 47 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff ff ff 00 00 00 00 21 00 00 00 
	10 27 8c 23 54 0b 01 00 07 00 00 00 7a 0c 00 00 00 00 00 00 6a 0a 24 0b 24 0b 89 0d c2 f2 00 00 00 00 89 0d 37 0a 10 27 10 27 10 27 01 00 01 01 00 00 00 86 4a 00 07 01 1e ff 00 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2c d3 18 3e e4 38 06 3e 37 3d e7 3d a6 08 c2 3d 16 d4 9c 3d 09 3f 6f 3d 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6a f4 e0 41 21 de e5 41 b6 9c ea 41 20 84 e9 41 20 84 e9 41 ca f7 e8 41 14 55 e9 41 14 55 e9 41 7d e1 e9 41 ae 58 eb 41 96 d0 ec 41 f6 19 ee 41 fb 34 ef 41 af c2 ef 41 83 50 f0 41 57 af f0 41 79 de f0 41 64 3d f1 41 90 c4 
	
	01 14 04 05 01 1f 
	00 bc 04 47 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff ff ff 00 00 00 00 21 00 00 00 
	10 27 8c 23 54 0b 
	01 00 07 00 00 00 92 0b 00 00 00 00 00 00 6a 0a 1b 0b 1b 0b 89 0d c2 f2 00 00 00 00 89 0d 2d 0a 10 27 10 27 10 27 01 00 01 01 00 00 00 86 4a 00 07 01 1e ff 00 0e 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 2c d3 18 3e e4 38 06 3e 37 3d e7 3d a6 08 c2 3d 16 d4 9c 3d 09 3f 6f 3d 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 6a f4 e0 41 21 de e5 41 b6 9c ea 41 20 84 e9 41 20 84 e9 41 ca f7 e8 41 14 55 e9 41 14 55 e9 41 7d e1 e9 41 ae 58 eb 41 96 d0 ec 41 f6 19 ee 41 fb 34 ef 41 af c2 ef 41 83 50 f0 41 57 af f0 41 79 de f0 41 64 3d f1 41 90 bf
	
	01 14 04 05 01 1f 
	00 bc 04 47 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff ff ff 00 00 00 00 21 00 00 00 
	10 27 
	00 00 --当前静压
	54 0b --目标静压
	01 00 
	03 00 --生长日期
	00 00 
	3c 02 --时间
	00 00 00 00 00 00 
	6a 0a --设置温度       26.7
	6b 0b --平均温度       29.2 
	6b 0b --整鸡舍温度     29.2
	93 0d --通道温度       34.8
	c2 f2 --舍外温度       -33.9
	00 00 00 00 
	93 0d --冷风温度       34.8
	a4 0a --屋顶温度       27.2
	10 27 --无
	10 27 --无
	10 27 --无
	01 00 --级别
	01 01 00 
	00 00 --平均湿度
	47 94 00 07 
	01 最小级别
	1e 最大级别
	ff 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 16 d4 9c 3d 09 3f 6f 3d 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 ac a2 0b 42 24 6f 0b 42 24 6f 0b 42 ac a2 0b 42 ac a2 0b 42 24 6f 0b 42 24 6f 0b 42 00 00 c8 42 00 00 c8 42 00 00 c8 42 00 00 c8 42 00 15 
	
	*/
	int16_t postion,TmpInt;

  postion=48;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//当前静压
	Ac2000Data[ADDR_3G_1802] =TmpInt/100;

	postion=postion+2;
	
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//目标静压 
	Ac2000Data[ADDR_3G_1803] =TmpInt/100;

	postion=postion+4;
	
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//日龄
	Ac2000Data[ADDR_3G_1804] =TmpInt;

	
	postion=postion+4;


	postion=postion+8;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);//设置温度

	Ac2000Data[ADDR_3G_1847] =TmpInt;
	postion=postion+2;//平均温度

	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1807] =TmpInt;

	
	postion=postion+2;//整鸡舍温度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1812] =TmpInt;		

	

	postion=postion+2;//通道温度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1849] =TmpInt;		

	
	postion=postion+2;//室外温度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1808] =TmpInt;


  postion=postion+6;//冷风温度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1811] =TmpInt;

	
  postion=postion+2;//屋顶温度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1809] =TmpInt;

	
	postion+=11;
	TmpInt=dataSource[postion];	//通风等级
	Ac2000Data[ADDR_3G_1805] =TmpInt;

	
	postion=postion+2;//平均湿度
	TmpInt = dataSource[postion+1]*256+dataSource[postion];
	if(TmpInt==0x2710) TmpInt=9999;
	Ac2000Data[ADDR_3G_1806] =TmpInt;

	
	
	postion+=6;	//最小级别
	TmpInt = dataSource[postion];
	Ac2000Data[ADDR_3G_1852] =TmpInt;

	
	postion+=1;	//最大级别
	TmpInt = dataSource[postion];
	Ac2000Data[ADDR_3G_1853] =TmpInt;
	
	return postion;
}

static unsigned char Parse_MainScreen2_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 04 05 01 1f 
	00 00 04 
	f4 04 00 09 00 00 02 00 00 00 09 02 00 02 00 00 00 0c 00 00 02 00 00 00 0c 01 00 02 00 00 00 1d 00 00 02 00 00 00 11 00 00 05 00 00 00 00 00 00 00 00 11 01 00 05 00 00 00 00 00 00 00 00 11 02 00 05 00 00 00 00 00 00 00 00 11 03 00 05 00 00 00 00 00 00 00 00 ae 
*/
	int16_t postion,TmpInt=0;
	postion=9;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//CO2
	Ac2000Data[ADDR_PLATINUM_2411] =TmpInt;
	rt_kprintf("CO2=%d  %04X\r\n",TmpInt,TmpInt);

	return postion;
}

//报警解析
static void Parse_Warn_pro(uint8_t *dataSource,uint16_t len)
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
	*/
	unsigned int postion=7;
	unsigned char warnNum=dataSource[postion++];
	if(warnNum>0)
	{
		Ac2000Data[ADDR_3G_1801]=1;//3G版报警状态
	}
	else
	{
		Ac2000Data[ADDR_3G_1801]=0;
	}
	unsigned int alarm_code=0x00;
	for(unsigned char j=0;j<=AC2000_ALARM_NUM_3G;j++)
	{
		AlarmCodePlatinum[ALARM_3G_12000+j][1]=0;
	}
	
	for(unsigned char i = 0; i<warnNum; i++)
	{
		alarm_code=dataSource[postion]*256+dataSource[postion+1];//得到告警类型码
		for(unsigned short j=0;j<AC2000_ALARM_NUM_3G;j++)
		{
			if(alarm_code==AC2000WarmCode[j])
			{
			//	rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				AlarmCodePlatinum[ALARM_3G_12000+j][1]=1;
				break;
			}
			if(j==(AC2000_ALARM_NUM_3G-1))
			{
				if(alarm_code==AC2000WarmCode[AC2000_ALARM_NUM_3G-1])
				{
					AlarmCode3g[0+j][1]=1;;
			//		rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				}
				else
				{
					AlarmCode3g[ALARM_3G_12000][1]=1;
			//		rt_kprintf("未知报警\r\n");
				}
			}
		}
		postion+=2;
	}
}

static void Parse_curve_state_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 01 2d 01 44 
	6a 0a --26.7 目标温度
	54 09 --23.9 低警报温度
	ac 0d --35.0 高警报温度
	ac 0d --35.0 Tunnel High Alarm
	48 71 --29   目标静压
	fb 09 --25.6 加热器温度
	96 0c --32.2 通道温度
	60 09 --
	60 09 --
	01 
	1e 
	00 00 00 00 00 00 00 00 00 00 00 00 
	93 
	*/
	int16_t postion,TmpInt;

	postion=6;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2426] = TmpInt;//目标温度
	rt_kprintf("目标温度=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2429] = TmpInt;//低温警报温度
	rt_kprintf("低温警报温度=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2430] = TmpInt;//高温警报温度
	rt_kprintf("高温警报温度=%d\r\n",TmpInt);
	
	postion=postion+2;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	rt_kprintf("Tunnel High Alarm=%d\r\n",TmpInt);
	
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
	Ac2000Data[ADDR_PLATINUM_2428] = TmpInt;//通道温度
	rt_kprintf("通道温度=%d\r\n",TmpInt);	
	
	postion=postion+6;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2432] = TmpInt;//最低级别

	
	postion=postion+1;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_PLATINUM_2433] = TmpInt;//最高级别
	
	
}

static void Parse_th_state_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 01 2c 01 43 
	17 0b --平均温度
	c2 f2 --舍外温度
	2d 0a --屋顶温度   
	89 0d --隧道温度
	10 27 --风冷温度
	17 0b --整鸡舍温度
	10 27 --第一鸡群温度
	10 27 --第二鸡群温度
	10 27 --第三鸡群温度
	ff ff 
	9c ff 
	3a 
	*/
	int16_t postion,TmpInt;
	postion=6;
	
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("平均温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("舍外温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("屋顶温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("隧道温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("风冷温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);

	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("第一鸡群温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("第二鸡群温度=%d\r\n",TmpInt);	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	rt_kprintf("第三鸡群温度=%d\r\n",TmpInt);	
	postion=postion+2;
	
		
}
// 继电器状态
static void Parse_RelyTest_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	05 14 02 48 01 64 
	05 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 
	05 01 00 01 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	0b 00 00 01 00 00 00 00 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	0c 00 00 01 00 00 00 00 03 00 00 00 00 00 00 00 00 00 00 00 01 00 00 
	0b 01 00 01 00 00 00 00 04 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	0c 01 00 01 00 00 00 00 05 00 00 00 00 00 00 00 00 00 00 00 01 00 00 
	10 01 00 01 00 00 00 00 06 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	0f 00 00 01 00 00 00 00 07 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	0f 01 00 01 00 00 00 00 08 00 00 00 00 00 00 00 00 00 00 00 00 01 00 
	0e 00 00 01 00 00 00 00 09 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	05 0a 00 01 00 00 00 00 0a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	ce 
	*/
	unsigned int postion=0;
	postion=14;
	int x,y=0;
	int m=(len-7)/23;
	for(uint8_t i=0;i<m;i++)
	{
			x=dataSource[postion]+1;//继电器序号
			postion=postion+12;
			
			y=dataSource[postion];//继电器状态
			
			postion=postion+2;
			if(dataSource[postion]==1)//反向
			{
				y=!y;
			}
			if(x<=30)
			{
				Ac2000Data[ADDR_PLATINUM_2434+x-1]=y;
			}
			else
			{
				Ac2000Data[ADDR_PLATINUM_2551+x-30-1]=y;
			}
		
			postion=postion+9;
	}
}

static void Parse_AskTmpature_pro(uint8_t *dataSource,uint16_t len)
{
	/*
	05 14 01 13 01 2e 
	01 00 00 04 dd 9c 58 45 00 00 
	01 01 00 04 00 40 1c 46 00 00 
	01 02 00 04 00 40 1c 46 00 00 
	01 03 00 04 00 40 1c 46 00 00 
	01 04 00 04 00 40 1c 46 00 00 
	01 05 00 04 00 40 1c 46 00 00 
	01 06 00 04 00 40 1c 46 00 00 
	01 07 00 04 00 40 1c 46 00 00 
	01 08 00 04 00 40 1c 46 00 00 
	01 09 00 04 00 40 1c 46 00 00 
	01 0a 00 04 00 40 1c 46 00 00 
	01 0b 00 04 00 40 1c 46 00 00 
	01 0c 00 04 00 40 1c 46 00 00 
	01 0d 00 04 00 40 1c 46 00 00 
	01 0e 00 04 00 40 1c 46 00 00 
	01 0f 00 04 00 40 1c 46 00 00 
	01 10 00 04 00 40 1c 46 00 00 
	01 11 00 04 00 40 1c 46 00 00 
	01 12 00 04 00 40 1c 46 00 00 
	01 13 00 04 00 40 1c 46 00 00 
	01 14 00 04 00 40 1c 46 00 00 
	01 15 00 04 00 40 1c 46 00 00 
	01 16 00 04 00 40 1c 46 00 00 
	01 17 00 04 00 40 1c 46 00 00 
	01 18 00 04 00 40 1c 46 00 00 
	ef 
	
	*/
	unsigned int postion;
	float	TmpFloat;
	uint32_t TmpInt;
	uint8_t TmpInt_index;
	postion = 7;
	int m=(len-7)/10;
	for(uint8_t j=0;j<m;j++)
	{	
		TmpInt_index=dataSource[postion]+1;//序号
		postion=postion+3;
    TmpInt=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
		if(TmpInt==0x461c4000)
		{
			TmpFloat = 9999;
			if(TmpInt_index<=9)
			{
				Ac2000Data[ADDR_PLATINUM_2412+TmpInt_index-1]=TmpFloat;
				//rt_kprintf("wendu %d=%f    no exist\r\n",TmpInt_index,TmpFloat);
			}
			else
			{
				break;
			}
		}
		else
		{
			TmpFloat=FourByteToFloat(&dataSource[postion]);
			if(TmpInt_index<=9)
			{
				Ac2000Data[ADDR_PLATINUM_2412+TmpInt_index-1]=TmpFloat;
				//rt_kprintf("wendu %d=%f  \r\n",TmpInt_index,TmpFloat);
			}
			else
			{
				break;
			}
		}
		postion=postion+7;	
	}
	
	for(uint8_t j=0;j<9;j++)
	{
		rt_kprintf("ADDR_PLATINUM_2412 + %d :%d \r\n",j,Ac2000Data[ADDR_PLATINUM_2412+j]);
	}
}
//static void Parse_AskTmpature2_pro(uint8_t *dataSource,uint16_t len)
//{
//	/*
//	05 14 01 13 01 2e 
//	01 19 00 04 00 40 1c 46 00 00 
//	01 1a 00 04 00 40 1c 46 00 00 
//	01 1b 00 04 00 40 1c 46 00 00 
//	01 1c 00 04 00 40 1c 46 00 00 
//	01 1d 00 04 00 40 1c 46 00 00 
//	01 1e 00 04 00 40 1c 46 00 00 
//	01 1f 00 04 00 40 1c 46 00 00 
//	01 20 00 04 00 40 1c 46 00 00 
//	01 21 00 04 00 40 1c 46 00 00 
//	01 22 00 04 00 40 1c 46 00 00 
//	01 23 00 04 00 40 1c 46 00 00 
//	77 

//	*/
//	unsigned int postion;
//	float	TmpFloat;
//	uint32_t TmpInt;
//	uint8_t TmpInt_index;
//	postion = 7;
//	int m=(len-7)/10;
//	for(uint8_t j=0;j<m;j++)
//	{	
//		TmpInt_index=dataSource[postion]+1;//序号
//		postion=postion+3;	
//		TmpInt=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
//		if(TmpInt==0x461c4000)
//		{
//			TmpFloat = 9999;
//			if(TmpInt_index<=9)
//			{
//				Ac2000Data[ADDR_PLATINUM_2412+TmpInt_index-1]=TmpFloat;
//				rt_kprintf("wendu %d=%f    no exist\r\n",TmpInt_index,TmpFloat);
//			}
//		}
//		else
//		{
//			TmpFloat=FourByteToFloat(&dataSource[postion]);
//			if(TmpInt_index<=9)
//			{
//				Ac2000Data[ADDR_PLATINUM_2412+TmpInt_index-1]=TmpFloat;
//				rt_kprintf("wendu %d=%f     \r\n",TmpInt_index,TmpFloat);
//			}
//		}
//		postion=postion+7;
//	}
//}
//水和食料解析
static void Parse_FeedWater_pro(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 04 16 01 30 
	00 00 00 00 00 00 00 00 
	42 00 00 00 
	98 00 00 00 
	00 00 00 00 
	00 00 00 00 
	10 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 ff ff 00 00 00 00 00 00 ff ff ff ff 00 00 00 e2 
	*/
	unsigned int WData=dataSource[17]*256*256*256+dataSource[16]*256*256+dataSource[15]*256+dataSource[14];	
	Ac2000Data[ADDR_PLATINUM_2422]=WData*10;//水表1
	rt_kprintf("水表1=%d\r\n",WData);//水表1
	
	WData=dataSource[21]*256*256*256+dataSource[20]*256*256+dataSource[19]*256+dataSource[18];
	Ac2000Data[ADDR_PLATINUM_2423]=WData*10;//水表2
	rt_kprintf("水表2=%d\r\n",WData);//水表2
	
	
	WData=(dataSource[33]*256*256*256+dataSource[32]*256*256+dataSource[31]*256+dataSource[30])*2.2f;	
	Ac2000Data[ADDR_PLATINUM_2424]=WData;//饲料1
	rt_kprintf("饲料1=%d\r\n",WData);//饲料1
	
	WData=(dataSource[37]*256*256*256+dataSource[36]*256*256+dataSource[35]*256+dataSource[34])*2.2f;
	rt_kprintf("饲料2=%d\r\n",WData);//饲料2
	

	
}
static void Parse_Pressure_pro(unsigned char *dataSource,uint16_t len)
{
	int16_t TmpInt=(dataSource[7]*256+dataSource[6]);
	Ac2000Data[ADDR_PLATINUM_2404]=(TmpInt+5)/10;
	
}

static void Parse_humiture_pro(unsigned char *dataSource,uint16_t len)
{
	/*
		01 14 02 11 01 29 
		02 00 00 04 00 00 80 bf 00 00 
		02 01 00 04 00 00 80 bf 00 00 
		03 00 00 04 00 00 80 bf 00 00 
		d1 
	*/
	unsigned int postion=0;
	unsigned int TmpInt;
	float TmpFloat;
	postion=6;
	int length=(len-7)/10;
	for(uint16_t i=0;i<length;i++)
	{
		if(dataSource[postion]==0x02)//湿度传感器-内部
		{
			postion=postion+1;
			TmpInt=dataSource[postion]+1;//序号
			
			postion=postion+3;
			TmpFloat=FourByteToFloat(&dataSource[postion]);//湿度值
			
			postion=postion+6;
			if(TmpInt==1||TmpInt==2) 
			{
				Ac2000Data[ADDR_PLATINUM_2407+TmpInt-1]=TmpFloat;
			}
			rt_kprintf("内部湿度%d: %0.1f\r\n",TmpInt,TmpFloat);
		}
		else if(dataSource[postion]==0x03)//湿度传感器-外部
		{       
			postion=postion+1;
			TmpInt=dataSource[postion]+1;//序号
			
			postion=postion+3;
			TmpFloat=FourByteToFloat(&dataSource[postion]);//湿度值
			
			postion=postion+6;
			if(TmpInt==1) 
			{
				Ac2000Data[ADDR_PLATINUM_2410+TmpInt-1]=TmpFloat;
			}
			rt_kprintf("外部湿度%d: %0.1f\r\n",TmpInt,TmpFloat);
		}
		else
		{
			postion=postion+1;
			postion=postion+3;
			postion=postion+6;		
		}
	}
}

//控制
void Por_Coll_Temperaturecalibration(unsigned char *dataSource,uint16_t len)
{
	 if(len==259)
	 {
	
		if(PorLevelsOfVentilationData==NULL) 
		{
			PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*5);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*5);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*4)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*5);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		if(len>6)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
		rt_memset(SysData,0,AliyunServiceSysLength*3);
		postion= 34;
		for(i=0;i<15;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;//日期
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion+2;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//目标温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//加热温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//隧道温度
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
		
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//低的报警
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//高的报警
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);//TUNNEL HIGH ALARM
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+12;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorAc2000ControlTemperatureCurve,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorAc2000ControlTemperatureCurve));
		strcat(Ac2000PorParams->Ac2000PorControl.PorAc2000ControlTemperatureCurve,SysData);	

	  rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROTEMPERATURECURVE_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}

void Por_Coll_Humidity(unsigned char *dataSource,uint16_t len)
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
		rt_memset(Ac2000PorParams->Ac2000PorControl.Porshumidity_treatm,0,sizeof(Ac2000PorParams->Ac2000PorControl.Porshumidity_treatm));
		strcat(Ac2000PorParams->Ac2000PorControl.Porshumidity_treatm,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
	  if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROHUMIDITYTREATM_CMD;
		}
}
void Por_Coll_Min_Max(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		int Hour;
	  int Minute;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =8;
		for(i=0;i<30;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日期
			Hour = TmpInt/60;
			Minute = TmpInt%60;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d",Hour);
			strcat(SysData,tmp);
			strcat(SysData,":");
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d,",Minute);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpInt = dataSource[postion];//最低级别
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+1;
			TmpInt = dataSource[postion];//最高级别
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.Pormin_max_level,0,sizeof(Ac2000PorParams->Ac2000PorControl.Pormin_max_level));
		strcat(Ac2000PorParams->Ac2000PorControl.Pormin_max_level,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
	  if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROMINMAX_CMD;
		}
}

void Por_Coll_Static_Pressure(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
	
	  unsigned char i;
	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		float TmpFloat;
		postion =10;
		for(i=0;i<5;i++)
		{
			TmpFloat = FourByteToFloat(&dataSource[postion]);//舎外 在低温下的静压
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion +8;
		}
	  postion = 14;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//隧道目标静压
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 30;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//隧道低静压报警
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 38;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//隧道高静压报警
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 46;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//隧道静压带
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 6;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//屋顶目标静压
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 6;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//屋顶目标静压
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 50;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//屋顶静压带
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorStaticpressure,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorStaticpressure));
		strcat(Ac2000PorParams->Ac2000PorControl.PorStaticpressure,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
	  if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROSTATICPRESSURE_CMD;
		}
}

void Por_Coll_Static_Mode(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;

	  char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 10;
		TmpInt = dataSource[postion];//整个鸡舍
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	  postion = 13;
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
		postion = 14;
		TmpInt = dataSource[postion];//Heat Cycle
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 12;
		TmpInt = dataSource[postion];//Analog Heat
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorControlmode,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorControlmode));
		strcat(Ac2000PorParams->Ac2000PorControl.PorControlmode,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
	  if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROMODE_CMD;
		}
		
}

void Por_Coll_System_temperaturecali(unsigned char *dataSource,uint16_t len)
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
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyTemperature_curve,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyTemperature_curve));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyTemperature_curve,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
}


void Por_Coll_System_Cycleheater(unsigned char  *dataSource,uint16_t len)
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
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cycle_Heater,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cycle_Heater));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cycle_Heater,SysData);	
		rt_kprintf("温度曲线-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		
		
		
		
	
}

void Por_Coll_System_Radiant_heater(unsigned char  *dataSource,uint16_t len)
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
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyRadiant_heater,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyRadiant_heater));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyRadiant_heater,SysData);	
		rt_kprintf("辐射加热器-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_VariableHeater(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 14;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Variable_heater,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Variable_heater));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Variable_heater,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Humidity_Treatment(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 14;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =10;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion-4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Humidity_treatment,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Humidity_treatment));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Humidity_treatment,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Co2_Treatment(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 14;
	
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion =10;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion = postion-4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Co2_dealwith,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Co2_dealwith));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Co2_dealwith,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Ammonia(unsigned char *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ammonia_Treatment,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ammonia_Treatment));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ammonia_Treatment,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Staticpressure(unsigned char *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 23;//阵风延时
	  for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+1;
		}
		postion = 20;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
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
		postion = postion+8;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =26;
		for(i=0;i<5;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion= postion+1;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Static_pressure,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Static_pressure));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Static_pressure,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_ventilation_tunnel(unsigned char *dataSource,uint16_t len)
{
	
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 14;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 10;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 12;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = 7;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 16;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
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
		postion = 17;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 26;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ventilation_level_tunnel,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ventilation_level_tunnel));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ventilation_level_tunnel,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Curtains_roof(unsigned char *dataSource,uint16_t len)//Curtains 跟屋顶
{
	
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		int Hour;
	  int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		char *SysDatar=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysDatar,0,AliyunServiceSysLength);
		postion = 27;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 39;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtains_Vents,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtains_Vents));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtains_Vents,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		postion = 15;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysDatar,tmp);
		postion = 38;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysDatar,tmp);
		postion = 31;//时间
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysDatar,tmp);
		strcat(SysDatar,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysDatar,tmp);
		
		
		postion = 33;//时间
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysDatar,tmp);
		strcat(SysDatar,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysDatar,tmp);
		
		postion = 19;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysDatar,tmp);
		
		if(SysDatar!=NULL) SysDatar[strlen(SysDatar)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.roof,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.roof));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.roof,SysDatar);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysDatar);
		rt_free(SysDatar);
}

void Por_Coll_System_Vent_Curtains(unsigned char *dataSource,uint16_t len)
{
	
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		int Hour;
	  int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 10;
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
	
		postion = postion+2;
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
	  
		postion =postion+2;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =6;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtain_Calibraation,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtain_Calibraation));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtain_Calibraation,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_humidifier(unsigned char *dataSource,uint16_t len)
{

		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 


		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		TmpFloat =  FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat =  FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =18;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 14;
		TmpFloat =  FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.humidifier,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.humidifier));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.humidifier,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}

void Por_Coll_System_Wetcurtain_Cool(unsigned char *dataSource,uint16_t len)
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
		char *SysDatac=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysDatac,0,AliyunServiceSysLength);
		postion = 14;
		TmpFloat = FourByteToFloat(&dataSource[postion]);//温度带
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 7;
		for(i=0;i<4;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+1;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wet_curtain,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wet_curtain));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wet_curtain,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
		postion = 12;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//Cool
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysDatac,tmp);
		strcat(SysDatac,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysDatac,tmp);
		
		postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysDatac,tmp);
		
		if(SysDatac!=NULL) SysDatac[strlen(SysDatac)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cool_pad_Plush,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cool_pad_Plush));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cool_pad_Plush,SysDatac);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysDatac);
		rt_free(SysDatac);
		
}

void Por_Coll_System_light(unsigned char *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		
		postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
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
			postion =postion+2;
		}
		postion = 8;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+3;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 10;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.light,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.light));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.light,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);

}


void Por_Coll_System_Feed_stock(unsigned char *dataSource,uint16_t len)
{
	
			unsigned char i;
			char tmp[8];
			int postion ;
			int TmpInt;
			float TmpFloat; 


			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			rt_memset(SysData,0,AliyunServiceSysLength);
			postion = 12;
			for(i=0;i<4;i++)
			{
				TmpFloat = FourByteToFloat(&dataSource[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion =postion+10;
			}
			postion =47;
			TmpInt = dataSource[postion]+1;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Feed_stock,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Feed_stock));
			strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Feed_stock,SysData);	
			rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
			rt_free(SysData);
			
		
	
}
void Por_Coll_System_feed_water(unsigned char *dataSource,uint16_t len)
{
		
		char tmp[8];
		int postion ;
		int TmpInt;

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
	  postion = 6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+3;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_feed,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_feed));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_feed,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
}
void Por_Coll_System_Nec_Wre(unsigned char *dataSource,uint16_t len)
{		
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		char *SysDataw=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysDataw,0,AliyunServiceSysLength);
		postion = 34;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 22;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =30;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 26;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =35;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 6;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Natural_entrance_conditions,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Natural_entrance_conditions));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Natural_entrance_conditions,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		for(i=0;i<4;i++)
		{
			TmpFloat = FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysDataw,tmp);
			postion =postion+4;
		}
		if(SysDataw!=NULL) SysDataw[strlen(SysDataw)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wind_Rain_Effect,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wind_Rain_Effect));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wind_Rain_Effect,SysDataw);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysDataw);
		rt_free(SysDataw);
		
}
void Por_Coll_System_Wod(unsigned char *dataSource,uint16_t len)
{
	
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat; 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpFloat= FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_Demand,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_Demand));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_Demand,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		
}
void Por_Coll_System_Alarmsystem(unsigned char *dataSource,uint16_t len)
{
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
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Alarm_system,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Alarm_system));
		strcat(Ac2000PorParams->Ac2000PorControl.PorContorSystem.Alarm_system,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROSYSTEMMODE_CMD;
		}
}

void Por_Coll_Co2Mode(unsigned char *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 16;
		for(i=0;i<20;i++)
		{	
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//开始值
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//结束值
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//延时(秒)
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//持续(秒)
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
		}
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PorParams->Ac2000PorControl.PorCo2processing,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorCo2processing));
			strcat(Ac2000PorParams->Ac2000PorControl.PorCo2processing,SysData);	
			rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORCONTROCO2PROCESSING_CMD;
			}
}

void Por_Coll_Ammoniatreatment(unsigned char *dataSource,uint16_t len)
{
		unsigned char i;
		char tmp[8];
		int postion ;
		int TmpInt;
 
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 16;
		for(i=0;i<20;i++)
		{	
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//开始值
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//结束值
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//延时(秒)
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			TmpInt = dataSource[postion]+dataSource[postion+1]*256;//持续(秒)
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorControl.PorAmmoniaTreatment,0,sizeof(Ac2000PorParams->Ac2000PorControl.PorAmmoniaTreatment));
		strcat(Ac2000PorParams->Ac2000PorControl.PorAmmoniaTreatment,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORCONTROAMMONIATREATMENT_CMD;
		}
}

//设备
void PorWrite_data_Empty(void)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	rt_memset(SysData,0,AliyunServiceSysLength*12);
	char tmp[8];
	if(AirxxxStatus.aliyun_service_param2==PORMECHANISMSPEEDCONTROFAN_CMD)//变频风机数据为空
	{
		for(unsigned char i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(unsigned char j=0;j<50;j++)
			{
				strcat(SysData,",");
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel,SysData);	
		rt_free(SysData);
		//rt_kprintf("device variable_speed_fan_levels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSPEEDCONTROFAN_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==PORMECHANISMCURTAINLEVEL_CMD)//小窗卷帘级别
	{
		for(unsigned char i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(unsigned char j=0;j<8;j++)
			{
				strcat(SysData,",");
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_VentAndCurtianLevels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMCURTAINLEVEL_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==PORMECHANISMSTIRRINGFANLEVEL_CMD)//搅拌风机级别
	{
		for(unsigned char i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(unsigned char j=0;j<15;j++)
			{
				strcat(SysData,",");
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_StirFanLevels:%s\n",Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSTIRRINGFANLEVEL_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==PORINSTALL_CURTAINSETTING_CMD)//安装风帘设置
	{
		for(unsigned char i=0;i<48;i++)
		{
			strcat(SysData,",");
		
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting,SysData);
		//rt_kprintf("device Ac2000SInstall.VentCurtainSetUp:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_CURTAINSETTING_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==PORINSTALL_FANAIRCAPACITY_CMD)//风机空气容量
	{
		for(unsigned char i=0;i<60;i++)
		{
			strcat(SysData,",");
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,SysData);
		rt_kprintf("device Ac2000SInstall.CapacityOfFanAir:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_FANAIRCAPACITY_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==PORINSTALL_TEMPERATUREDE_CMD)//温度定义
	{
		for(unsigned short i=0;i<504;i++)
		{
			strcat(SysData,",");
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition,SysData);
		//rt_kprintf("device Ac2000SInstall.TemperatureDefine:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_TEMPERATUREDE_CMD;
		}	
	
	}

}


void Por_Mechanism_Ventilationlevel(unsigned char *dataSource,uint16_t len)
{
		
	 if(len==261)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		 rt_kprintf("差值:""666");
		 
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}

		uint8_t tunnel=0;
		uint8_t Exhaust=0;
		uint8_t k=0;
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		unsigned char TfanStatus[30];
		unsigned char EfanStatus[20];
		unsigned char T[30];
		unsigned char E[20];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*20);
	  rt_memset(SysData,0,AliyunServiceSysLength*20);
		
	  //	rt_memset(Ac20003GParams->Ac2000Device.levels_of_ventilation,0,sizeof(Ac20003GParams->Ac2000Device.levels_of_ventilation));
		//rt_kprintf("LevelsOfVentilationDataLength:%d\n",LevelsOfVentilationDataLength);
  	//my_log(LevelsOfVentilationData,LevelsOfVentilationDataLength);
		uint8_t  s_level=1;
		
		postion=12;	//差值
		for(uint8_t i=0;i<30;i++)
		{
		//	if(postion<(LevelsOfVentilationDataLength-14)) 
		//	{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",s_level);
				strcat(SysData,tmp);
				s_level++;
				
				TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				//rt_kprintf("差值:%f  ",TmpFloat);
				
				postion=postion+4;
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				//rt_kprintf("开启:%d  ",TmpInt);
			
			
				
				postion=postion+4;
				memset(TfanStatus,0,sizeof(TfanStatus));
				memset(EfanStatus,0,sizeof(EfanStatus));	
				memset(T,0,sizeof(T));
				memset(E,0,sizeof(E));	
				for(uint8_t n=0;n<60;n++)
				{
					if(PorLevelsOfVentilationData[postion]==5||PorLevelsOfVentilationData[postion]==6)//5:隧道风机 6:排气风机
					{
						switch(PorLevelsOfVentilationData[postion])//5:隧道风机 6:排气风机
						{
							case 5:
							{
								 k=PorLevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 T[tunnel]=k;
								 TfanStatus[tunnel]=PorLevelsOfVentilationData[postion+4];
								postion=postion+6;
								tunnel+=1;
							}break;
							case 6:
							{
								 k=PorLevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 E[Exhaust]=k;
								EfanStatus[Exhaust]=PorLevelsOfVentilationData[postion+4];
								postion=postion+6;
								Exhaust+=1;
							}break;
							default:break;
						}
					}
					else
					{
						break;
					}
				}	
			
				if(tunnel ==0)//无数据全部空白的
				{
					for(unsigned char s=0;s<30;s++)
					{
						strcat(SysData,",");
					}
				
				}
				else if(tunnel==1)//判断数据是否只有一个，如果只有一个那么需要前后进行空白填充
				{
					for(unsigned char s=0;s<T[tunnel-1];s++)
					{
						strcat(SysData,",");
					}
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TfanStatus[tunnel-1]);
					strcat(SysData,tmp);
					for(unsigned char s=T[tunnel-1] ;s<29;s++)
					{
						strcat(SysData,",");
					}
				}
				else
				{
					for(unsigned char s=0;s<T[0];s++)
					{
						strcat(SysData,",");
					}
					
					for(uint8_t n=0;n<tunnel-1;n++)//中间数据空白填充
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TfanStatus[n]);
						strcat(SysData,tmp);
							if((T[n+1]-T[n])>1)
							{
								for(unsigned char a=0;a<(T[n+1]-T[n]-1);a++)
								{
									strcat(SysData,",");
								}
							
							}
					}
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TfanStatus[tunnel-1]);
						strcat(SysData,tmp);
						for(unsigned char s=T[tunnel-1];s<29;s++)
						{
							strcat(SysData,",");
						}
					
						
				}
				
				tunnel=0;
		
				
	
				
				if(Exhaust ==0)//无数据全部空白的
				{
					for(unsigned char s=0;s<20;s++)
					{
						strcat(SysData,",");
					}
				
				}
				else if(Exhaust==1)//判断数据是否只有一个，如果只有一个那么需要前后进行空白填充
				{
					for(unsigned char s=0;s<E[Exhaust-1];s++)
					{
						strcat(SysData,",");
					}
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",EfanStatus[Exhaust-1]);
					strcat(SysData,tmp);
					for(unsigned char s=E[Exhaust-1] ;s<19;s++)
					{
						strcat(SysData,",");
					}
				}
				else
				{
					for(unsigned char s=0;s<E[0];s++)
					{
						strcat(SysData,",");
					}
					
					for(uint8_t n=0;n<Exhaust-1;n++)//中间数据空白填充
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",EfanStatus[n]);
						strcat(SysData,tmp);
							if((E[n+1]-E[n])>1)
							{
								for(unsigned char a=0;a<(E[n+1]-E[n]-1);a++)
								{
									strcat(SysData,",");
								}
							
							}
					}
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",EfanStatus[Exhaust-1]);
						strcat(SysData,tmp);
						for(unsigned char s=E[Exhaust-1];s<19;s++)
						{
							strcat(SysData,",");
						}
				}
				Exhaust=0;
				
				
				
						
				

		}

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorVentilationlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorVentilationlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorVentilationlevel,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMVENTILATIONLEVEL_CMD;
		}
		
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}
void Por_Mechanism_Inverterfan(unsigned char *dataSource,uint16_t len)
{
	 if(len==195)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
			uint16_t postion;
			int	TmpInt;

			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
			rt_memset(SysData,0,AliyunServiceSysLength*6);
			unsigned char Aisle1[30][30];
			unsigned char Roof1[30][30];
			unsigned char A[30];
			unsigned char R[30];
			unsigned char a=0;
			unsigned char b=0;
			unsigned char s=PorLevelsOfVentilationDataLength/64;

		
			postion =0;
			for(unsigned char i=0;i<s;i++)
			{
				if(PorLevelsOfVentilationData[postion]==0x05)//通道风扇
				{
					A[a]=PorLevelsOfVentilationData[postion+1];//存储风机编号
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[PorLevelsOfVentilationData[postion+1]][j]=PorLevelsOfVentilationData[postion+4+(j*2)];//存入多维数组中
					}
					a=a+1;//在的风机个数
				}
				else if(PorLevelsOfVentilationData[postion]==0x06)//排气风扇
				{
					R[b]=PorLevelsOfVentilationData[postion+1];//保存风帘编号,判断差距
					for(unsigned char z=0;z<30;z++)
					{
						Roof1[PorLevelsOfVentilationData[postion+1]][z]=PorLevelsOfVentilationData[postion+4+(z*2)];
					}
					b=b+1;
				}
				postion =postion+64;
			}
		
		
			
			
			if(a==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<30;i++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[i][j]=0xff;
					}
				}
			}
			else if(a==1)//有一个数据
			{
	
					for(unsigned char u=0;u<A[0];u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}

					for(unsigned char u=A[0]+1;u<30;u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}
			}
			else 
			{
				for(unsigned char u=0;u<A[0];u++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[u][j]=0xff;
					}
				}
				
				for(unsigned char n=0;n<a-1;n++)//中间数据空白填充
				{
						if((A[n+1]-A[n])>1)
						{
							for(unsigned char i=A[n]+1;i<A[n+1];i++)
							{
								for(unsigned char j=0;j<30;j++)
								{
									Aisle1[i][j]=0xff;
									
								}
							}
							
						}
				}
				for(unsigned char u=A[a-1]+1;u<30;u++)
				{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
				}
				
			}
			//排气风机
			if(b==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<20;i++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Roof1[i][j]=0xff;
					}
				}
			}
			else if(b==1)//有一个数据
			{
	
					for(unsigned char u=0;u<R[0];u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
					}

					for(unsigned char u=R[0]+1;u<20;u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
					}
				
			
			}
			else 
			{
				for(unsigned char u=0;u<R[0];u++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Roof1[u][j]=0xff;
					}
				}
				
				for(uint8_t n=0;n<b-1;n++)//中间数据空白填充
				{
						if((R[n+1]-R[n])>1)
						{
							for(unsigned char i=R[n]+1;i<R[n+1];i++)
							{
								for(unsigned char j=0;j<30;j++)
								{
									Roof1[i][j]=0xff;
									
								}
							}
							
						}
				}
				for(unsigned char u=R[b-1]+1;u<20;u++)
				{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
				}
				
			}
			
			
			
			
			
			
			for(unsigned char i=0;i<30;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
				for(unsigned char j=0;j<30;j++)
				{
					TmpInt = Aisle1[j][i];
					if(TmpInt==0xff)
					{
					
						strcat(SysData,",");
					}
					else
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
					}
				}
				for(unsigned char z=0;z<20;z++)
					{
					TmpInt=Roof1[z][i];
					if(TmpInt==0xff)
					{
					
						strcat(SysData,",");
					}
					else
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);

					}

				}
			
			}
			
			
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel,SysData);	
	//	rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSPEEDCONTROFAN_CMD;
		}
			if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	
	}
}
void Por_Mechanism_Ventilation_curtain_rating(unsigned char *dataSource,uint16_t len)
{
	if(len==261)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*6)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		 
		 if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*6)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
		
			uint16_t postion;
			int	TmpInt;

			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
			rt_memset(SysData,0,AliyunServiceSysLength*6);
			unsigned char Aisle1[4][30];
			unsigned char Roof1[4][30];
			unsigned char A[4];
			unsigned char R[4];
			unsigned char a=0;
			unsigned char b=0;
			unsigned char s=PorLevelsOfVentilationDataLength/34;
			postion =0;
			for(unsigned char i=0;i<s;i++)
			{
				if(PorLevelsOfVentilationData[postion]==0x0D)
				{
					A[a]=PorLevelsOfVentilationData[postion+1];
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[PorLevelsOfVentilationData[postion+1]][j]=PorLevelsOfVentilationData[postion+4+j];
					}
					a=a+1;
				}
				else if(PorLevelsOfVentilationData[postion]==0x0F)
				{
					R[b]=PorLevelsOfVentilationData[postion+1];//保存风帘编号,判断差距
					for(unsigned char z=0;z<30;z++)
					{
						Roof1[PorLevelsOfVentilationData[postion+1]][z]=PorLevelsOfVentilationData[postion+4+z];
					}
					b=b+1;
				}
				postion =postion+34;
			}
		
				if(a==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<4;i++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[i][j]=0xff;
					}
				}
			}
			else if(a==1)//有一个数据
			{
	
					for(unsigned char u=0;u<A[0];u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}

					for(unsigned char u=A[0]+1;u<4;u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}
			}
			else 
			{
				for(unsigned char u=0;u<A[0];u++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[u][j]=0xff;
					}
				}
				
				for(unsigned char n=0;n<a-1;n++)//中间数据空白填充
				{
						if((A[n+1]-A[n])>1)
						{
							for(unsigned char i=A[n]+1;i<A[n+1];i++)
							{
								for(unsigned char j=0;j<30;j++)
								{
									Aisle1[i][j]=0xff;
									
								}
							}
							
						}
				}
				for(unsigned char u=A[a-1]+1;u<4;u++)
				{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
				}
				
			}

			if(b==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<4;i++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Roof1[i][j]=0xff;
					}
				}
			}
			else if(b==1)//有一个数据
			{
	
					for(unsigned char u=0;u<R[0];u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
					}

					for(unsigned char u=R[0]+1;u<4;u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
					}
				
			
			}
			else 
			{
				for(unsigned char u=0;u<R[0];u++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Roof1[u][j]=0xff;
					}
				}
				
				for(uint8_t n=0;n<b-1;n++)//中间数据空白填充
				{
						if((R[n+1]-R[n])>1)
						{
							for(unsigned char i=R[n]+1;i<R[n+1];i++)
							{
								for(unsigned char j=0;j<30;j++)
								{
									Roof1[i][j]=0xff;
									
								}
							}
							
						}
				}
				for(unsigned char u=R[b-1]+1;u<4;u++)
				{
						for(unsigned char j=0;j<30;j++)
						{
							Roof1[u][j]=0xff;
						}
				}
				
			}
			
			
			
			
			
			
			for(unsigned char i=0;i<30;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
				
				for(unsigned char z=0;z<4;z++)
					{
					TmpInt=Roof1[z][i];
					if(TmpInt==0xff)
					{
					
						strcat(SysData,",");
					}
					else
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);

					}

				}
				
				for(unsigned char j=0;j<4;j++)
				{
					TmpInt = Aisle1[j][i];
					if(TmpInt==0xff)
					{
					
						strcat(SysData,",");
					}
					else
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
					}
				}
				
			
			}
			
		
	

		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMCURTAINLEVEL_CMD;
		}
	}
}

void Por_Mechanism_Stirfan(unsigned char *dataSource,uint16_t len)
{
			if(len==245)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		 
		 if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
		
			uint16_t postion;
			int	TmpInt;
			
			char tmp[10];
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
			rt_memset(SysData,0,AliyunServiceSysLength*6);
			postion =0;
			unsigned char Aisle1[15][30];
			unsigned char A[15];
			unsigned char a=0;
			unsigned char s=PorLevelsOfVentilationDataLength/34;
			for(unsigned char i=0;i<s;i++)
			{
				if(PorLevelsOfVentilationData[postion]==0x07)
				{
					A[a]=PorLevelsOfVentilationData[postion+1];
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[PorLevelsOfVentilationData[postion+1]][j]=PorLevelsOfVentilationData[postion+4+j];
					
					}
					a=a+1;
				}
			 postion =postion+34;
			}
					
		if(a==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<15;i++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[i][j]=0xff;
					}
				}
			}
			else if(a==1)//有一个数据
			{
	
					for(unsigned char u=0;u<A[0];u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}

					for(unsigned char u=A[0]+1;u<15;u++)
					{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
					}
			}
			else 
			{
				for(unsigned char u=0;u<A[0];u++)
				{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[u][j]=0xff;
					}
				}
				
				for(unsigned char n=0;n<a-1;n++)//中间数据空白填充
				{
						if((A[n+1]-A[n])>1)
						{
							for(unsigned char i=A[n]+1;i<A[n+1];i++)
							{
								for(unsigned char j=0;j<30;j++)
								{
									Aisle1[i][j]=0xff;
									
								}
							}
							
						}
				}
				for(unsigned char u=A[a-1]+1;u<15;u++)
				{
						for(unsigned char j=0;j<30;j++)
						{
							Aisle1[u][j]=0xff;
						}
				}
				
			}
		
		
		
		for(unsigned char i=0;i<30;i++)
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
				for(unsigned char j=0;j<15;j++)
				{
					TmpInt = Aisle1[j][i];
					if(TmpInt==0xff)
					{
					
						strcat(SysData,",");
					}
					else
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
					}
				}
			
			}
		
		
		
		
		
		
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel));
			strcat(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel,SysData);	
			rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
			rt_free(SysData);
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSTIRRINGFANLEVEL_CMD;
			}
	}
}

void Por_Mechanism_Stirring_fanprogram(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;

	char tmp[8];

	my_log(dataSource,len);
	postion=6;
	for(uint8_t j=0;j<15;j++)
	{
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",(j+1));
		strcat(SysData,tmp);
		
		
		
		for(uint8_t i=0;i<5;i++)
		{
			if(((dataSource[postion])&(1<<i))!=0)//提取前五位数据
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%s","1,");
				strcat(SysData,tmp);
				

			}
			else
			{
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%s","0,");
				strcat(SysData,tmp);
	
			}
		}
	
	
		postion++;
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanprogram,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanprogram));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorStirringfanprogram,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSTIRRINGFANPROGRAM_CMD;
		}
	
}

void Por_Mechanism_sprayer(unsigned char *dataSource,uint16_t len)
{
	if(len==247)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		
		
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
	

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		int Hour;
		int Minute;
		postion=16;	//日期
		for(uint8_t i=0;i<20;i++)
		{
			if(postion<(PorLevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//目标差值
				TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;
			}
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.Porsprayer,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.Porsprayer));
		strcat(Ac2000PorParams->Ac2000PorMechanism.Porsprayer,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMSPRAYER_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		}
}

void Por_Mechanism_Refrigeratedversion(unsigned char *dataSource,uint16_t len)
{
	if(len==261)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		int Hour;
		int Minute;
		postion=16;	//日期
		for(uint8_t i=0;i<20;i++)
		{
			if(postion<(PorLevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//目标差值
				TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;
			}
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorRefrigerationplate,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorRefrigerationplate));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorRefrigerationplate,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMREFRIGERATION_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		}

}

void Por_Mechanism_light_Nofeed(unsigned char *dataSource,uint16_t len)
{
	if(len==259)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
		uint16_t postion;
		unsigned char i=0;
		int	TmpInt;
		int Hour;
		int Minute;

		unsigned char _RelaySum=0;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
	  rt_memset(SysData,0,AliyunServiceSysLength*6);
		postion = 0;
		_RelaySum = ((PorLevelsOfVentilationDataLength/50)/6)-1;
		for(i=0;i<50;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
			memset(tmp,0,sizeof(tmp));//日期
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;//开始时间
			TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
			if(_RelaySum==0)
			{
				for(unsigned char relay=0;relay<10;relay++)
				{
					strcat(SysData,",");
				}
			}
			else if(_RelaySum==1)
			{
				for(unsigned char s=0;s<PorLevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = PorLevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=PorLevelsOfVentilationData[postion+1]+1;ss<10;ss++)
				{
					strcat(SysData,",");
				}
				postion =postion+6;
			}
			else
			{
				for(unsigned char s=0;s<PorLevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = PorLevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=0;ss<_RelaySum-1;ss++)
				{
					for(unsigned char t=PorLevelsOfVentilationData[postion+1]+1;t<PorLevelsOfVentilationData[postion+7];t++)
					{
						strcat(SysData,",");
					}
					postion =postion+6;
					TmpInt = PorLevelsOfVentilationData[postion+4];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
 				}
				for(unsigned char j=PorLevelsOfVentilationData[postion+1]+1;j<10;j++)
				{
					strcat(SysData,",");
				}
				
				postion =postion+6;
			}

		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.Porlight_Nofeed,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.Porlight_Nofeed));
		strcat(Ac2000PorParams->Ac2000PorMechanism.Porlight_Nofeed,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMLIGHT_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}

void Por_Mechanism_light_feed(unsigned char *dataSource,uint16_t len)
{
	if(len==259)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	 else if(len >6)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*6);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*6);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*5)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*6);
			PorLevelsOfVentilationDataLength=0;
		}
		uint16_t postion;
		unsigned char i=0;
		int	TmpInt;
		int Hour;
		int Minute;

		char tmp[10];
		unsigned char _RelaySum=0;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*6);
	  rt_memset(SysData,0,AliyunServiceSysLength*6);
		postion = 0;
		 _RelaySum = ((PorLevelsOfVentilationDataLength/50)/6)-1;
			for(i=0;i<50;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
			memset(tmp,0,sizeof(tmp));//日期
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			
			postion=postion+2;//开始时间
			TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
			if(_RelaySum==0)
			{
				for(unsigned char relay=0;relay<10;relay++)
				{
					strcat(SysData,",");
				}
			}
			else if(_RelaySum==1)
			{
				for(unsigned char s=0;s<PorLevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = PorLevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=PorLevelsOfVentilationData[postion+1]+1;ss<10;ss++)
				{
					strcat(SysData,",");
				}
				postion =postion+6;
			}
			else
			{
				for(unsigned char s=0;s<PorLevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = PorLevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=0;ss<_RelaySum-1;ss++)
				{
					for(unsigned char t=PorLevelsOfVentilationData[postion+1]+1;t<PorLevelsOfVentilationData[postion+7];t++)
					{
						strcat(SysData,",");
					}
					postion =postion+6;
					TmpInt = PorLevelsOfVentilationData[postion+4];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
 				}
				for(unsigned char j=PorLevelsOfVentilationData[postion+1]+1;j<10;j++)
				{
					strcat(SysData,",");
				}
				
				postion =postion+6;
			}

		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.Porlight_feed,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.Porlight_feed));
		strcat(Ac2000PorParams->Ac2000PorMechanism.Porlight_feed,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMLIGHT_FEED_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}

}

void Por_Mechanism_water_feed(unsigned char *dataSource,uint16_t len)
{
	if(len==247)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else if(len >6)
	{
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}
		

		uint8_t WSum=0;
		uint8_t FSum=0;
		uint8_t MTSum=0;
		uint8_t k=0;
		uint16_t postion;
		int	TmpInt;

		char tmp[10];
		unsigned char WStatus[4];
		unsigned char FStatus[4];
		unsigned char MtStatus[4];
		unsigned char W[4];
		unsigned char F[4];
		unsigned char MT[4];
		unsigned char Number=0;
		int Hour;
		int Minute;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*20);
	  rt_memset(SysData,0,AliyunServiceSysLength*20);
		Number = ((PorLevelsOfVentilationDataLength/50)/6)-1;//每一排有多少个继电器开启
		rt_kprintf("Number%d\n",Number);
		postion=0;	//差值
		for(uint8_t i=0;i<50;i++)
		{
				TmpInt=PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+2;
			
			
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;//开始时间
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				postion=postion+2;
			
				TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;//开始时间
				Hour = TmpInt/60;
				Minute=TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d,",Minute);
				strcat(SysData,tmp);
				
				postion=postion+2;
				memset(WStatus,0,sizeof(WStatus));
				memset(FStatus,0,sizeof(FStatus));
				memset(MtStatus,0,sizeof(MtStatus));	
				memset(W,0,sizeof(W));	
				memset(F,0,sizeof(F));	
				memset(MT,0,sizeof(MT));	
				
				for(uint8_t n=0;n<Number;n++)
				{
					if(PorLevelsOfVentilationData[postion]==0x17)
					{
							k=PorLevelsOfVentilationData[postion+1];
							W[WSum]=k;
							WStatus[WSum]=PorLevelsOfVentilationData[postion+4];
							postion=postion+6;
							WSum=WSum+1;
					}
					else if(PorLevelsOfVentilationData[postion]==0x18)
					{
							k=PorLevelsOfVentilationData[postion+1];
							F[FSum]=k;
							FStatus[FSum]=PorLevelsOfVentilationData[postion+4];
							postion=postion+6;
							FSum=FSum+1;
					}
					else if(PorLevelsOfVentilationData[postion]==0x11)//5:隧道风机 6:排气风机
					{
							k=PorLevelsOfVentilationData[postion+1];//风机编号 实际需加1
							MT[MTSum]=k;
							MtStatus[MTSum]=PorLevelsOfVentilationData[postion+4];
							postion=postion+6;
							MTSum=MTSum+1;
					}
				}	
			
				if(WSum ==0)//无数据全部空白的
				{
					for(unsigned char s=0;s<4;s++)
					{
						strcat(SysData,",");
					}
				
				}
				else if(WSum==1)//判断数据是否只有一个，如果只有一个那么需要前后进行空白填充
				{
					for(unsigned char s=0;s<W[WSum-1];s++)
					{
						strcat(SysData,",");
					}
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",WStatus[WSum-1]);
					strcat(SysData,tmp);
					for(unsigned char s=W[WSum-1] ;s<3;s++)
					{
						strcat(SysData,",");
					}
				}
				else
				{
					for(unsigned char s=0;s<W[0];s++)
					{
						strcat(SysData,",");
					}
					
					for(uint8_t n=0;n<WSum-1;n++)//中间数据空白填充
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",WStatus[n]);
						strcat(SysData,tmp);
							if((W[n+1]-W[n])>1)
							{
								for(unsigned char a=0;a<(W[n+1]-W[n]-1);a++)
								{
									strcat(SysData,",");
								}
							
							}
					}
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",WStatus[WSum-1]);
						strcat(SysData,tmp);
						for(unsigned char s=W[WSum-1];s<3;s++)
						{
							strcat(SysData,",");
						}
					
						
				}
				
				WSum=0;
		
				
	
				
				if(FSum ==0)//无数据全部空白的
				{
					for(unsigned char s=0;s<4;s++)
					{
						strcat(SysData,",");
					}
				
				}
				else if(FSum==1)//判断数据是否只有一个，如果只有一个那么需要前后进行空白填充
				{
					for(unsigned char s=0;s<F[FSum-1];s++)
					{
						strcat(SysData,",");
					}
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",FStatus[FSum-1]);
					strcat(SysData,tmp);
					for(unsigned char s=F[FSum-1] ;s<3;s++)
					{
						strcat(SysData,",");
					}
				}
				else
				{
					for(unsigned char s=0;s<F[0];s++)
					{
						strcat(SysData,",");
					}
					
					for(uint8_t n=0;n<FSum-1;n++)//中间数据空白填充
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",FStatus[n]);
						strcat(SysData,tmp);
							if((F[n+1]-F[n])>1)
							{
								for(unsigned char a=0;a<(F[n+1]-F[n]-1);a++)
								{
									strcat(SysData,",");
								}
							
							}
					}
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",FStatus[FSum-1]);
						strcat(SysData,tmp);
						for(unsigned char s=F[FSum-1];s<3;s++)
						{
							strcat(SysData,",");
						}
					
						
				}
				FSum=0;
				
				
				
				if(MTSum ==0)//无数据全部空白的
				{
					for(unsigned char s=0;s<4;s++)
					{
						strcat(SysData,",");
					}
				
				}
				else if(MTSum==1)//判断数据是否只有一个，如果只有一个那么需要前后进行空白填充
				{
					for(unsigned char s=0;s<MT[MTSum-1];s++)
					{
						strcat(SysData,",");
					}
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",MtStatus[MTSum-1]);
					strcat(SysData,tmp);
					for(unsigned char s=MT[MTSum-1] ;s<3;s++)
					{
						strcat(SysData,",");
					}
				}
				else
				{
					for(unsigned char s=0;s<MT[0];s++)
					{
						strcat(SysData,",");
					}
					
					for(uint8_t n=0;n<MTSum-1;n++)//中间数据空白填充
					{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",MtStatus[n]);
						strcat(SysData,tmp);
							if((MT[n+1]-MT[n])>1)
							{
								for(unsigned char a=0;a<(MT[n+1]-MT[n]-1);a++)
								{
									strcat(SysData,",");
								}
							
							}
					}
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",MtStatus[MTSum-1]);
						strcat(SysData,tmp);
						for(unsigned char s=MT[MTSum-1];s<3;s++)
						{
							strcat(SysData,",");
						}
					
						
				}
				MTSum=0;
				
				
	}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorWaterfeed,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorWaterfeed));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorWaterfeed,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMWATERFEED_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
}
}
void Pro_Mechanism_Additionalsystem(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;


		int	TmpInt;
		int Hour;
		int Minute;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;

		for(i=0;i<4;i++)
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;//开始时间
			Hour = TmpInt/60;
			Minute=TmpInt%60;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d",Hour);
			strcat(SysData,tmp);
			strcat(SysData,":");
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d,",Minute);
			strcat(SysData,tmp);
			postion = postion+20;
		}
		postion = 8;
		for(i=0;i<4;i++)
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;//结束时间
			Hour = TmpInt/60;
			Minute=TmpInt%60;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d",Hour);
			strcat(SysData,tmp);
			strcat(SysData,":");
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d,",Minute);
			strcat(SysData,tmp);
			postion = postion+20;
		}
		postion =18;
		for(i=0;i<4;i++)//开始温度
		{
			TmpFloat =FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		postion =22;
		for(i=0;i<4;i++)//结束温度
		{
			TmpFloat =FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		
		postion = 10;
		for(i=0;i<4;i++)//开始湿度
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		postion = 12;
		for(i=0;i<4;i++)//结束湿度
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		postion = 14;
		for(i=0;i<4;i++)//开启
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		postion = 16;
		for(i=0;i<4;i++)//关闭
		{
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+20;
		}
		
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorAdditionalsystem,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorAdditionalsystem));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorAdditionalsystem,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMADDITIONALSYSTEM_CMD;
		}
}

void Por_Mechanism_WaterOnDemand(unsigned char *dataSource,uint16_t len)
{
	if(len==261)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*12);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*12);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else if(len >6)
	{
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*12);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*12);
			PorLevelsOfVentilationDataLength=0;
		}
		uint16_t postion;
		unsigned char i=0;

		
		int	TmpInt;
		int Hour;
		int Minute;
	
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 0;
		for(i=0;i<50;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			postion =postion+2;
			TmpInt=PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;//开始时间
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
			if(WaterSum==0)
			{
				for(unsigned char w=0;w<4;w++)
				{
					strcat(SysData,",");
				}
			}
			else if(WaterSum==1)
			{
				for(unsigned char j=0;j<WaterBuffer[WaterSum-1];j++)
				{
					strcat(SysData,",");
				}
				
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char j=WaterBuffer[WaterSum-1];j<3;j++)
				{
					strcat(SysData,",");
				}
			}
			else 
			{
				for(unsigned char j=0;j<WaterBuffer[0];j++)
				{
					strcat(SysData,",");
				}
				
				for(unsigned char j=0;j<WaterSum-1;j++)
				{
					TmpInt = PorLevelsOfVentilationData[postion];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
					for(unsigned char s=1;s<WaterBuffer[j+1]-WaterBuffer[j];s++)
					{
							strcat(SysData,",");
					}
					postion =postion+2;
				}
					TmpInt = PorLevelsOfVentilationData[postion];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
					postion =postion+2;
				for(unsigned char j=WaterBuffer[WaterSum-1];j<3;j++)
				{
					strcat(SysData,",");
				}
			}
			
			
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorWaterOnDemand,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorWaterOnDemand));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorWaterOnDemand,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMWATERONDEMAND_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		
		
	}
}

void Por_Mechanism_Natural(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;


	
		int	TmpInt;
		int Hour;
		int Minute;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 26;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;//开始时间
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
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;//开始时间
		Hour = TmpInt/60;
		Minute=TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",Minute);
		strcat(SysData,tmp);
		postion =6;
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
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =18;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =22;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =34;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =44;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =42;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 45;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 46;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =48;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorNaturalProgram,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorNaturalProgram));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorNaturalProgram,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMNATURALPROGRAM_CMD;
		}
}
void Pro_Mechanism_Feed_Scale(unsigned char *dataSource,uint16_t len)
{
	
	if(len==261)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else if(len >6)
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		unsigned char i=0;
	

		int	TmpInt;
		
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 16;
		for(i=0;i<20;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion+20]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);

			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
			TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
			TmpInt =10156*FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;

		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorMechanism.PorFeedScaleProgram,0,sizeof(Ac2000PorParams->Ac2000PorMechanism.PorFeedScaleProgram));
		strcat(Ac2000PorParams->Ac2000PorMechanism.PorFeedScaleProgram,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMECHANISMFEEDSCALEPROGRAM_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		
	
	}

}

void Por_Management_ChickenSum(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;
		int Sum[3];
		int chick[9];
	
		int	TmpInt;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<9;i++)
		{
			strcat(SysData,"0,");
		}
		postion = postion+36;
		for(i=0;i<3;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
			Sum[i]=TmpInt;
		}
		
		postion =6;
		for(i=0;i<9;i++)
		{
			TmpInt = dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
			chick[i]=TmpInt;
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
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageChickenstock,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageChickenstock));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageChickenstock,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGECHICKENSTOCK_CMD;
		}
		
		
}
void Por_management_Feed(unsigned char *dataSource,uint16_t len)
{
	if(len==245)
	 {
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*30);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*30);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*29)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*30);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else if(len >6)
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
	
		int Year;
		int month;
		int day;


		int	TmpInt;

		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*30);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion= 40;
		for(unsigned char i=0;i<99;i++)
		{
		if(postion<PorLevelsOfVentilationDataLength)
		{
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",i+1);
		strcat(SysData,tmp);
		
		TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
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
		TmpInt = (int)(FourByteToFloat(&PorLevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PorLevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PorLevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&PorLevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion+=20;
		 
		}
	}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageFeedstock,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageFeedstock));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageFeedstock,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGEFEEDSTOCK_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}
unsigned char TimeDayState=0;
void Por_MangTimeDay(unsigned char *dataSource,uint16_t len)
{
	if(TimeDayState == 0)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
			TimeDayState=1;
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
	

		int Hour;
		int Minute;


		int	TmpInt;
		
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		TmpInt =(int)FourByteToFloat(&PorLevelsOfVentilationData[postion]);

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
		TmpInt = PorLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,"-");
		postion=postion+1;
		TmpInt = PorLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,"-");
		postion = postion+1;
		TmpInt = PorLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt+2000);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = PorLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageTimedate,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageTimedate));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageTimedate,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGETOMEDATE_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		TimeDayState=0;
	}

}
unsigned char DayOldState=0;
void Por_MangDayoldflock(unsigned char *dataSource,uint16_t len)
{
		if(DayOldState == 0)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
			DayOldState=1;
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		int	TmpInt;

		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
		TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = PorLevelsOfVentilationData[postion]+PorLevelsOfVentilationData[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageGrowthperiodflock,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageGrowthperiodflock));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageGrowthperiodflock,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGEGROWTHPERIODFLOCK_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		DayOldState=0;
}
}

unsigned char AlarmSystemState=0;
void Por_MangAlarm_system(unsigned char *dataSource,uint16_t len)
{
	 if(AlarmSystemState == 0)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
			AlarmSystemState=1;
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		unsigned char i=0;
		int Hour;
		int Minute;
		
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =4;
		for(i=0;i<20;i++)
		{
			if(PorLevelsOfVentilationData[postion]!=0xff)
			{
				if(i==8)
				{
					TmpInt = (int)FourByteToFloat(&PorLevelsOfVentilationData[postion+4]);
					Hour = TmpInt/60;
					Minute=TmpInt%60;
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%02d",Hour);
					strcat(SysData,tmp);
					strcat(SysData,":");
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%02d,",Minute);
					strcat(SysData,tmp);
				}
				else
				{
					TmpFloat = FourByteToFloat(&PorLevelsOfVentilationData[postion+4]);
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%0.1f,",TmpFloat);
					strcat(SysData,tmp);
				
				}
					postion =postion+16;
			}
			else
			{
				if(i==8)
				{
					
					strcat(SysData,"00");
					strcat(SysData,":");
					strcat(SysData,"00,");
				
				}
				postion =postion+16;
				
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageAlarmsettings,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageAlarmsettings));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageAlarmsettings,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGEALARMSETTING_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		AlarmSystemState=0;
}

}

void Pro_Mang_Nosuccessfulsettingssaved(unsigned char *dataSource,uint16_t len)
{
		unsigned char i ;
		uint16_t postion;
		int	TmpInt;
	
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
		rt_memset(Ac2000PorParams->Ac2000PorManage.PorManageSavesuccessfulsettings,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManageSavesuccessfulsettings));
		strcat(Ac2000PorParams->Ac2000PorManage.PorManageSavesuccessfulsettings,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORMANAGESAVESUCCESS_CMD;
		}
}

void Pro_MangPassword(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[32];
	uint16_t TmpLong;
	
	postion=6;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=8;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=10;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=12;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=14;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=16;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=18;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PorParams->Ac2000PorManage.PorManagepassword,0,sizeof(Ac2000PorParams->Ac2000PorManage.PorManagepassword));
	strcat(Ac2000PorParams->Ac2000PorManage.PorManagepassword,SysData);	
	rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PORMANAGEPASSWORD_CMD;
	}

}

void Pro_TestSwitchrelay(unsigned char *dataSource,uint16_t len)
{
	 if(len == 260)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		unsigned char i=0;

		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<20;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion+1;
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+20;
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorTest.PorTestswitch,0,sizeof(Ac2000PorParams->Ac2000PorTest.PorTestswitch));
		strcat(Ac2000PorParams->Ac2000PorTest.PorTestswitch,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORTESTSWITCH_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}

void Pro_TestAnalogsensor(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
  float TmpFloat;
	char tmp[32];
	unsigned int TmpInt;
	postion=6;
	unsigned char i ;
	for(i=0;i<12;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		if(TmpFloat<9999.0)
		{
		if(i<8)
		{
			sprintf(tmp,"%0.1f,",(TmpFloat/100));
		}
		else
		{
			sprintf(tmp,"%0.1f,",TmpFloat);
		}
		}
		else
		{
			sprintf(tmp,"***,");
		}
			strcat(SysData,tmp);
			postion =postion+8;
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PorParams->Ac2000PorTest.PorTestAnalogsensor,0,sizeof(Ac2000PorParams->Ac2000PorTest.PorTestAnalogsensor));
	strcat(Ac2000PorParams->Ac2000PorTest.PorTestAnalogsensor,SysData);	
	rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PORTESTANALOGSENSOR_CMD;
	}
}

void Pro_TestDialogSenSor(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;

	char tmp[32];
	unsigned int TmpInt;
	postion=6;
	unsigned char i ;
	for(i=0;i<12;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion =postion+4;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		
		TmpInt=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+4;
		
		
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PorParams->Ac2000PorTest.PorTestDigitalsensor,0,sizeof(Ac2000PorParams->Ac2000PorTest.PorTestDigitalsensor));
	strcat(Ac2000PorParams->Ac2000PorTest.PorTestDigitalsensor,SysData);	
	rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PORTESTDIGITALSENSOR_CMD;
	}

}

void Pro_TestStaticpressure(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;

	char tmp[32];
	unsigned int TmpInt;
	postion=6;

	TmpInt=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PorParams->Ac2000PorTest.PorTestStaticpressure,0,sizeof(Ac2000PorParams->Ac2000PorTest.PorTestStaticpressure));
	strcat(Ac2000PorParams->Ac2000PorTest.PorTestStaticpressure,SysData);	
	rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=PORTESTSTATICPRESSURE_CMD;
	}
}

void Pro_Service_Temperaturecalibration(unsigned char *dataSource,uint16_t len)
{
	if(len == 257)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		uint16_t postion;
		unsigned char i;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =1;
		for(i=0;i<36;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+3;
			
			TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			if(TmpFloat<10000.0)
			{
				sprintf(tmp,"%0.1f,",(TmpFloat/100));
			}
			else
			{
				sprintf(tmp,"***,");
			}
			strcat(SysData,tmp);
			postion =postion+4;
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+3;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerTemperaturecalibration,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerTemperaturecalibration));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerTemperaturecalibration,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICETEMPERATURE_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}

void Pro_Server_Humiditycalibration(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;

		
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*1);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<3;i++)
		{
		TmpInt= dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		if(TmpFloat>-100.0)
		{
			sprintf(tmp,"%0.1f,",(TmpFloat));
		}
		else
		{
			sprintf(tmp,"***,");
		}
		strcat(SysData,tmp);
		postion =postion+4;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerHumiditycalibration,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerHumiditycalibration));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerHumiditycalibration,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEHUMIDITY_CMD;
		}
}

void Pro_Server_FeedCalibration(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;

	
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		strcat(SysData,"999,");
		for(i=0;i<4;i++)
		{
			TmpFloat=FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.3f,",(TmpFloat));
			strcat(SysData,tmp);
			postion =postion+6;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerFeedcalibration,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerFeedcalibration));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerFeedcalibration,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEFEED_CMD;
		}
}

void Pro_Server_WaterCalibration(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;


		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		
		postion = 10;
	  for(i=0;i<21;i++)
		{
			TmpFloat=FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.3f,",(TmpFloat));
			strcat(SysData,tmp);
			postion =postion+4;
		}
		postion =6;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.3f,",(TmpFloat));
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerWatercalibration,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerWatercalibration));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerWatercalibration,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEWATER_CMD;
		}
}

void Pro_Server_Pluginterface(unsigned char *dataSource,uint16_t len)
{
		
	

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		strcat(SysData,"1,");
		strcat(SysData,"Setting #1,");
		strcat(SysData,"2,");
		strcat(SysData,"Setting #2,");
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerSetpluginterface,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerSetpluginterface));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerSetpluginterface,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICESETPLUG_CMD;
		}

}

void Pro_Server_Waterlinenipplerinse(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;

		char Hour;
		char Minute;
		unsigned int TmpInt;


		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*1);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		int starttime[20];
	  int startMineute[20];
	  int state[20];
		for(i=0;i<20;i++)
		{
			starttime[i]=dataSource[postion]+dataSource[postion+1]*256;
			postion = postion+2;
			
		}
		postion =46;
		for(i=0;i<20;i++)
		{
			startMineute[i]=dataSource[postion+i];
		}
		postion = 66;
		for(i=0;i<20;i++)
		{
			state[i]=dataSource[postion+i];
		}
		
		for(i=0;i<20;i++)
		{
			TmpInt=starttime[i];//开始时间
			Hour = TmpInt/60;
			Minute=TmpInt%60;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d",Hour);
			strcat(SysData,tmp);
			strcat(SysData,":");
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%02d,",Minute);
			strcat(SysData,tmp);

			
			TmpInt = startMineute[i];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			TmpInt = state[i];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerWaterlinenipplerinse,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerWaterlinenipplerinse));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerWaterlinenipplerinse,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEWATERLINENIPPLERINSE_CMD;
		}
}

void Pro_Server_Waterfeeder(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned char i=0;

		
		unsigned int TmpInt;

		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(i=0;i<10;i++)
		{
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpInt = dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpFloat =(dataSource[postion]+dataSource[postion+1]*256)/10.0;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+2;
			TmpFloat =(dataSource[postion]+dataSource[postion+1]*256)/10.0;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion =postion+2;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerFeeder,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerFeeder));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerFeeder,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEENNDER_CMD;
		}
}

void Pro_Server_AmmoniaCalibration(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;




		unsigned int TmpInt;


		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
	
		postion =10;
		TmpInt =(dataSource[postion]+dataSource[postion+1]*256);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+4;
		TmpInt =(dataSource[postion]+dataSource[postion+1]*256);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 6;
		TmpInt=(int)FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",(TmpInt));
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000Porservice.PorSerAmmoniaCalibration,0,sizeof(Ac2000PorParams->Ac2000Porservice.PorSerAmmoniaCalibration));
		strcat(Ac2000PorParams->Ac2000Porservice.PorSerAmmoniaCalibration,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORSERVICEAMMONIACALIBRATION_CMD;
		}
}

void Pro_Install_Setting(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned int TmpInt;


		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);

		strcat(SysData,"Precision mode,");
		postion =  20;  //温度单位
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  24; //静压单位
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  40;//26风速
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  23;//风机空气容量单位
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  22;//长度单位
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  21;//重量单位
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  26;//生长区
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  27;//是否最小动力
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  28;//14自然通风
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  29;//隧道
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =  36;//历史分辨率
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallSetup,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallSetup));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallSetup,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_SETUP_CMD;
		}
		
	
}



void Pro_Install_Relaysensor(unsigned char *dataSource,uint16_t len)
{
	
	/*
	01 14 02 1C 01 34 
	01 00 00 04 00 00 80 BF 00 00 80 BF 00 00 00 00 00 00 00 00 00 00 
	01 01 00 04 00 00 80 BF 00 00 80 BF 01 00 00 00 00 00 00 00 00 00 
	01 02 00 04 00 00 80 BF 00 00 80 BF 02 00 00 00 00 00 00 00 00 00 
	06 00 00 04 00 00 80 BF 00 00 80 BF 03 00 00 00 00 00 00 00 00 00 
	05 00 00 04 00 00 80 BF 00 00 80 BF 04 00 00 00 00 00 00 00 00 00 
	04 00 00 04 00 00 80 BF 00 00 80 BF 05 00 00 00 00 00 00 00 00 00 
	02 00 00 04 00 00 80 BF 00 00 80 BF 06 00 00 00 00 00 00 00 00 00 
	02 01 00 04 00 00 80 BF 00 00 80 BF 07 00 00 00 00 00 00 00 00 00 
	03 00 00 04 00 00 80 BF 00 00 80 BF 08 00 00 00 00 00 00 00 00 00 
	05 00 00 03 00 00 00 00 00 00 20 41 00 00 01 00 00 00 00 00 00 00 
	17 01 00 01 00 00 80 BF 00 00 80 BF 00 00 02 00 00 00 00 00 01 00 
	D7
	*/
	if(len==249)
	{
		//rt_kprintf("LevelsOfVentilationDataLength:%d\n",LevelsOfVentilationDataLength);
		if(PorLevelsOfVentilationData==NULL) 
		{
			PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("AnalogSensor malloc failure\n");
			}
		}
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}

	}
	else
	{
		//rt_kprintf("LevelsOfVentilationDataLength:%d\n",LevelsOfVentilationDataLength);
		if(len>6)
		{
				if(PorLevelsOfVentilationData==NULL) 
		{
			PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*20);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("AnalogSensor malloc failure\n");
			}
		}
		if(PorLevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*20);
			PorLevelsOfVentilationDataLength=0;
		}
		
	/*
	模拟传感器数据
		01—温度传感器
		06—断路器
		05—测光仪
		04—CO2传感器
		02—舍内温度传感器
		03—舍外温度传感器
		07—电位计
		08—W.O.D压力
		00—无
	数字传感器数据
		09—水表
		0A—制冷板水表
		0B—喷雾器水表
		0C—饲料计量
		0D—蛟龙超时
		0E--额外报警
		0F—水压低
		*/
    //rt_kprintf(" LevelsOfVentilationDataLength:%d\n",LevelsOfVentilationDataLength);

 
			unsigned char lightSum=0;
			unsigned char WaterandFeed=0;

			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData1=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData2=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData3=(char *)rt_malloc(AliyunServiceSysLength);
		  rt_memset(SysData,0,AliyunServiceSysLength);
			rt_memset(SysData1,0,AliyunServiceSysLength);
			rt_memset(SysData2,0,AliyunServiceSysLength);
			rt_memset(SysData3,0,AliyunServiceSysLength);
			rt_memset(WaterBuffer,0,sizeof(WaterBuffer));
			unsigned int postion;
			float	TmpFloat;
			int	TmpInt;
			char tmp[8];

			unsigned char s;unsigned char a;unsigned char b;
			postion=0;
			WaterSum=0;
			for(int i=0;i<12;i++)
			{
					if(postion<(PorLevelsOfVentilationDataLength-22)) 
				{
					if(PorLevelsOfVentilationData[postion+3]==0x04)
					{
						if(i!=0)
						{
						 s=(PorLevelsOfVentilationData[postion+12])-(PorLevelsOfVentilationData[postion-10]);
						for(unsigned char j=1;j<s;j++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+j+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData,tmp);
							strcat(SysData,"***,");
							strcat(SysData,"***,");
						}
						}
						else
						{
							b=PorLevelsOfVentilationData[+12];
							for(unsigned char c=0;c<b;c++)
							{
								
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",c+1);
								strcat(SysData,tmp);
								strcat(SysData,"***,");
								strcat(SysData,"***,");
							
							}
						}
						TmpInt=PorLevelsOfVentilationData[postion+12]+1;//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
												
						
						TmpInt=PorLevelsOfVentilationData[postion];//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
					
		      	
						TmpInt=PorLevelsOfVentilationData[postion+1]+1;//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion=postion+22;						
					}
					else
					{
						unsigned char data=12-PorLevelsOfVentilationData[postion-10];
						for(a=1;a<data;a++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+a+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData,tmp);
							strcat(SysData,"***,");
							strcat(SysData,"***,");
						}
						break;						
					}
				}
				else
				{
					break;
				}
			}
			if(Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput==1)
			{
				
			Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput=0;
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogsensor,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogsensor));
			strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogsensor,SysData);	
			rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
			
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORINSTALL_ANALOGSENSOR_CMD;
			}
		
		 }
			rt_free(SysData);	

     for(int i=0;i<12;i++)//数字传感器
			{
			if(postion<(PorLevelsOfVentilationDataLength-22)) 
				{
					if(PorLevelsOfVentilationData[postion+3]==0x02)
					{
						if(i!=0)
						{
						 s=(PorLevelsOfVentilationData[postion+12])-(PorLevelsOfVentilationData[postion-10]);
						for(unsigned char j=1;j<s;j++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+j+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData1,tmp);
							strcat(SysData1,"***,");
							strcat(SysData1,"***,");
						}
						}
						else
						{
							b=PorLevelsOfVentilationData[+12];
							for(unsigned char c=0;c<b;c++)
							{
								
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",c+1);
								strcat(SysData1,tmp);
								strcat(SysData1,"***,");
								strcat(SysData1,"***,");
							
							}
						}
						TmpInt=PorLevelsOfVentilationData[postion+12]+1;//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData1,tmp);
												
						
						TmpInt=PorLevelsOfVentilationData[postion];//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData1,tmp);
					
		      	
						TmpInt=PorLevelsOfVentilationData[postion+1]+1;//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData1,tmp);
						postion=postion+22;						
					}
					else
					{
						unsigned char data=12-PorLevelsOfVentilationData[postion-10];
						for(a=1;a<data;a++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+a+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData1,tmp);
							strcat(SysData1,"***,");
							strcat(SysData1,"***,");
						}
						break;						
					}
				}
				else
				{
					break;
				}
			}
			if(Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput==2)
			{
				Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput=0;
				if(SysData1!=NULL) SysData1[strlen(SysData1)-1]=0x00;
				rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallDigitalsensor,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallDigitalsensor));
				strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallDigitalsensor,SysData1);
				rt_kprintf("Ac2000SInstall.DigitalSensor:%s\n",SysData1);
					
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORINSTALL__DIGITALSENSOR_CMD;
			}
			}
			
			rt_free(SysData1);
	
			for(int i=0;i<10;i++)
			{
			
					if(PorLevelsOfVentilationData[postion+3]==0x03)
					{
						if(i!=0)
						{
						 s=(PorLevelsOfVentilationData[postion+12])-(PorLevelsOfVentilationData[postion-10]);
						for(unsigned char j=1;j<s;j++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+j+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData2,tmp);
							strcat(SysData2,"***,");
							strcat(SysData2,"***,");
							strcat(SysData2,"***,");
							TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion+8]);
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%0.1f,",TmpFloat);
							strcat(SysData2,tmp);
						}
						}
						else
						{
							b=PorLevelsOfVentilationData[+12];
							for(unsigned char c=0;c<b;c++)
							{
								
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",c+1);
								strcat(SysData2,tmp);
								strcat(SysData2,"***,");
								strcat(SysData2,"***,");
								strcat(SysData2,"***,");
								TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion+8]);
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%0.1f,",TmpFloat);
								strcat(SysData2,tmp);
							}
						}
						TmpInt=PorLevelsOfVentilationData[postion+12]+1;//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData2,tmp);
												
						
						TmpInt=PorLevelsOfVentilationData[postion];//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData2,tmp);
					
		      	
						TmpInt=PorLevelsOfVentilationData[postion+1]+1;//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData2,tmp);
						
						TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion+4]);
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%0.1f,",TmpFloat);
						strcat(SysData2,tmp);
						
						TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion+8]);
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%0.1f,",TmpFloat);
						strcat(SysData2,tmp);
						postion=postion+22;						
					}
					else
					{
						unsigned char data=10-PorLevelsOfVentilationData[postion-10];
						for(a=1;a<data;a++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+a+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData2,tmp);
							strcat(SysData2,"***,");
							strcat(SysData2,"***,");
							strcat(SysData2,"***,");
							TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion-14]);
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%0.1f,",TmpFloat);
							strcat(SysData2,tmp);
						}
						break;						
					}
				
			}
if(Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput==3)
			{
				Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput=0;
				if(SysData2!=NULL) SysData2[strlen(SysData2)-1]=0x00;
				rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogoutput,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogoutput));
				strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogoutput,SysData2);
				rt_kprintf("Ac2000SInstall.AnalogOutput:%s\n",SysData2);
				
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORINSTALL_ANALOGOUTPUT_CMD;
			}
			}
			
			rt_free(SysData2);	
		
			for(int i=0;i<30;i++)
			{
				if(PorLevelsOfVentilationData[postion+3]==0x01)
					{
						
						if(i!=0)
						{
						 s=(PorLevelsOfVentilationData[postion+12])-(PorLevelsOfVentilationData[postion-10]);
						for(unsigned char j=1;j<s;j++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+j+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData3,tmp);
							strcat(SysData3,"***,");
							strcat(SysData3,"***,");
							strcat(SysData3,"***,");
						}
						}
						else
						{
							b=PorLevelsOfVentilationData[+12];
							for(unsigned char c=0;c<b;c++)
							{
								
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",c+1);
								strcat(SysData3,tmp);
								strcat(SysData3,"***,");
								strcat(SysData3,"***,");
								strcat(SysData3,"***,");
							}
						}
						TmpInt=PorLevelsOfVentilationData[postion+12]+1;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
												
						
						TmpInt=PorLevelsOfVentilationData[postion];//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						if(TmpInt==0x16)
						{
							lightSum=lightSum+1;
						}
						else if((TmpInt==0x18)||(TmpInt==0x17))
						{
							WaterandFeed=WaterandFeed+1;
						}
		      	if(TmpInt==0x26)
						{
							WaterBuffer[WaterSum]=PorLevelsOfVentilationData[postion+1];
							WaterSum =WaterSum+1;
						}
						TmpInt=PorLevelsOfVentilationData[postion+1]+1;//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						
						TmpInt=PorLevelsOfVentilationData[postion+21];//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						postion=postion+22;						
					}
					else
					{
						
						unsigned char data=20-PorLevelsOfVentilationData[postion-10];
						for(a=1;a<data;a++)
						{
							TmpInt=PorLevelsOfVentilationData[postion-10]+a+1;
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%d,",TmpInt);
							strcat(SysData3,tmp);
							strcat(SysData3,"***,");
							strcat(SysData3,"***,");
							strcat(SysData3,"***,");
						}
						break;						
					}
			
			
			}
			rt_kprintf("WaterSum%d\n",WaterSum);
			rt_kprintf("WaterSum%d\n",WaterBuffer[0]);
			rt_kprintf("WaterSum%d\n",WaterBuffer[1]);
			if(ACCESS_DIFFERENT==1)
			{
				rt_kprintf("light feed%d\n",lightSum);
				Number_relaystate = ((lightSum+1)*6)*50;//这个是根据灯光继电器个数，来算出在设备部分查询灯光数据回复的数据总数，根据总数才能发送查询帧
			}else if(ACCESS_DIFFERENT==2)
			{
				rt_kprintf("WaterandFeed%d\n",WaterandFeed);
				Number_relaystate = ((WaterandFeed+5)*6)*50;
			}
			
			if(Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput==4)
			{
			Ac2000PorParams->Ac2000PorInstall.AnalogSensorAndOutput=0;
			if(SysData3!=NULL) SysData3[strlen(SysData3)-1]=0x00;
			rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallRelaylayout,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallRelaylayout));
			strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallRelaylayout,SysData3);
			rt_kprintf("Ac2000SInstall.AnalogOutput:%s\n",SysData3);
				
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=PORINSTALL_RELAYLAYOUT_CMD;
			}
			}
			
			rt_free(SysData3);	
			
			PorLevelsOfVentilationDataLength=0;
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}	
	}
}


}
   


void Lengthacquisition(unsigned char *dataSource,uint16_t len)
{
	Number_relays = dataSource[6]+dataSource[7]*256;
	Number_relaystate = (((((Number_relays/8)*6)+8)*30)+12);//只用于计算下发询问通风等级的帧数量
	rt_kprintf("%d\n",Number_relays);
}


void Pro_Install_Curtainsetting(unsigned char *dataSource,uint16_t len)
{
		uint16_t postion;
		unsigned int TmpInt;


		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
		postion =6;
		for(unsigned char i=0;i<6;i++)
		{
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+5;
		if((dataSource[postion]==0xff)&&(dataSource[postion+1]==0xff))
		{
			strcat(SysData,"0,");
		}
		else
		{
			strcat(SysData,"Potentionmeter,");
		}
		postion = postion+3;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
	}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_CURTAINSETTING_CMD;
		}
		
}

void Por_Temperaturedefinition_sum(unsigned char *dataSource,uint16_t len)
{
	Number_relays = dataSource[6]+dataSource[7]*256;
}
void Pro_Install_Temperaturedefinition(unsigned char *dataSource,uint16_t len)
{
		if(len == 247)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
		uint16_t postion;
		unsigned char i=0;
		unsigned char k=0;

	
		int	TmpInt;
	
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		for(i=0;i<14;i++)
		{
			TmpInt = PorLevelsOfVentilationData[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion =postion+4;
			for(k=0;k<36;k++)
			{
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion = postion+1;
				
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_TEMPERATUREDE_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		
	}
}

void Pro_Install_Fanaircapacity(unsigned char *dataSource,uint16_t len)
{
	
	
		uint16_t postion;
		int TmpInt;
		float TmpFloat;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*2);
		rt_memset(SysData,0,AliyunServiceSysLength*2);
		postion =0;
		if(len == 255)
	{
			if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
		
		if(PorLevelsOfVentilationDataLength==Number_relays)
		{
			for(unsigned char i=0;i<40;i++)
		{
			if(postion<PorLevelsOfVentilationDataLength-8)
			{
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%2d ",TmpInt);
				strcat(SysData,tmp);
				postion =postion+1;
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp," %2d,",TmpInt);
				strcat(SysData,tmp);
				
				postion = postion+3;
				TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",(TmpFloat));
				strcat(SysData,tmp);
				postion =postion+4;
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_FANAIRCAPACITY_CMD;
		}
			if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
		}
		
		
		
	}
	else
	{
		if(PorLevelsOfVentilationData==NULL) 
		{
				PorLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PorLevelsOfVentilationData!=NULL)
			{
				rt_memset(PorLevelsOfVentilationData,0,256*3);
				PorLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PorLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PorLevelsOfVentilationData[PorLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PorLevelsOfVentilationDataLength=PorLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PorLevelsOfVentilationData,0,256*3);
			PorLevelsOfVentilationDataLength=0;
		}
		
		for(unsigned char i=0;i<40;i++)
		{
			if(postion<PorLevelsOfVentilationDataLength-8)
			{
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%2d ",TmpInt);
				strcat(SysData,tmp);
				postion =postion+1;
				TmpInt = PorLevelsOfVentilationData[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp," %2d,",TmpInt);
				strcat(SysData,tmp);
				
				postion = postion+3;
				TmpFloat=FourByteToFloat(&PorLevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",(TmpFloat));
				strcat(SysData,tmp);
				postion =postion+4;
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_FANAIRCAPACITY_CMD;
		}
		if(PorLevelsOfVentilationData!=NULL) 
		{
			rt_free(PorLevelsOfVentilationData);
			PorLevelsOfVentilationData=NULL;
		}
		PorLevelsOfVentilationDataLength=0;
	}
}
void Pro_InstallHenSize(unsigned char *dataSource,uint16_t len)
{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		float	TmpFloat;
		char tmp[8];
	
		postion=6;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion=10;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);

		postion=14;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion=18;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallCoopsize,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallCoopsize));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallCoopsize,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_COOPSIZE_CMD;
		}
}

void Pro_InstallCommunication(unsigned char *dataSource,uint16_t len)
{

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		uint16_t	TmpInt;
		char tmp[8];
		postion=6;
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
		rt_memset(Ac2000PorParams->Ac2000PorInstall.PorInstallCommunication,0,sizeof(Ac2000PorParams->Ac2000PorInstall.PorInstallCommunication));
		strcat(Ac2000PorParams->Ac2000PorInstall.PorInstallCommunication,SysData);	
		rt_kprintf("V H-.PorAc2000ControlTemperatureCurve:%s\n",SysData);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PORINSTALL_COMMUNICATION_CMD;
		}
	
}
int Ac2000DataProParse(uint8_t *data,unsigned short Counter)
{
	uint8_t result=0;
	uint16_t cmd_code;
	cmd_code=data[2]*256+data[3];
	rt_kprintf("recv por normal data:  %04X  %d\r\n\r\n",cmd_code,Counter);
//	my_log(data,Counter);
	if(Ac2000Data!=NULL)
	{
		switch(cmd_code)
		{
	    case 0x0405://主屏
			{
				if(Counter==261)
				{
					Parse_MainScreen_pro(data,Counter);
				}
				else if(Counter==96)
				{
					Parse_MainScreen2_pro(data,Counter);
				}
			}break;
			case 0x0007:
			{
					Parse_Warn_pro(data,Counter);
			}break;
			
			//控制
			case 0x431D:
			{
				Por_Coll_Temperaturecalibration(data,Counter);
			};break;//温度校准
			case 0x0122:
			{
				Por_Coll_Humidity(data,Counter);//湿度曲线
			}break;
			case 0x8120:
			{
				Por_Coll_Min_Max(data,Counter);//最小最大级别
			}break;
			case 0x012A:
			{
				Por_Coll_Static_Pressure(data,Counter);//静压
			}break;
			case 0x422B:
			{
				if(Counter == 16)
				Por_Coll_Static_Mode(data,Counter);//控制模式
			}break;
			case 0x0165:
			{
				if(Counter == 35)//系统参数温度
				Por_Coll_System_temperaturecali(data,Counter);
			}break;
			case 0x0162:
			{
				if(Counter == 19)//系统参数 cycle_heater
				Por_Coll_System_Cycleheater(data,Counter);
			}break;
			case 0x4166:
			{
				if(Counter == 17)//系统参数 辐射加热器
				Por_Coll_System_Radiant_heater(data,Counter);
			}break;
			case 0x03a6:
			{
				if(Counter == 25)//V H
				Por_Coll_System_VariableHeater(data,Counter);
			}break;
			case 0x036c:
			{
				if(Counter == 23)
				Por_Coll_System_Humidity_Treatment(data,Counter);//湿度
			}break;
			case 0x0386:
			{
				if(Counter == 22)
				Por_Coll_System_Co2_Treatment(data,Counter);//Co2处理
			}break;
			case 0x02e7:
			{
				if(Counter == 13)
				Por_Coll_System_Ammonia(data,Counter);//氨处理
			}break;
			case 0x4269:
			{
				if(Counter == 33)
				Por_Coll_System_Staticpressure(data,Counter);//静压
			}break;
			case 0x0267:
			{
				if(Counter == 35)
				Por_Coll_System_ventilation_tunnel(data,Counter);//通风等级与隧道
			}break;
			case 0x056E:
			{
				if(Counter == 42)
				Por_Coll_System_Curtains_roof(data,Counter);//Curtains与屋顶
			}break;
			case 0x03f9:
			{
				if(Counter == 19)
				Por_Coll_System_Vent_Curtains(data,Counter);//Curtains calibration
			}break;
			case 0x026A:
			{
				if(Counter == 27)
				Por_Coll_System_Wetcurtain_Cool(data,Counter);//湿帘 Cool
			}break;
			case 0x026b:
			{
				if(Counter==23)
				Por_Coll_System_humidifier(data,Counter);//加湿器
			}break;
			case 0x016F:
			{
				if(Counter==23)
				Por_Coll_System_light(data,Counter);//灯光
			}break;
			case 0x0180:
			{
				if(Counter == 51)
				Por_Coll_System_Feed_stock(data,Counter);//饲料存量
			}break;
			case 0x0187:
			{
				if(Counter == 27)
				Por_Coll_System_feed_water(data,Counter);//水和饲料
			}break;
			case 0x01DE:
			{
				if(Counter == 39)
				Por_Coll_System_Nec_Wre(data,Counter); // NATURAL ENTRANCE CONDITIONS   WIND RAIN EFFECT
			}break;
			case 0x001EF:
			{
				if(Counter == 15)
				Por_Coll_System_Wod(data,Counter);//w o d
			}break;
			case 0x0172:
			{
				if(Counter == 8)
				Por_Coll_System_Alarmsystem(data,Counter);//报警设置
			}break;
			case 0x0192:
			{
				if(Counter == 217)
				Por_Coll_Co2Mode(data,Counter);
			}break;
			case 0x07E1:
			{
				if(Counter == 217)
				Por_Coll_Ammoniatreatment(data,Counter);
			}break;
			//机件
			case 0x4299://通风等级
			{
				Por_Mechanism_Ventilationlevel(data,Counter);
			}break;
			case 0x0044:
			{
				Lengthacquisition(data,Counter);
			}break;
			case 0x0344://调速风扇级别
			{
				Por_Mechanism_Inverterfan(data,Counter);
			}break;
			case 0x0037:
			{
				Lengthacquisition(data,Counter);
			}break;
			case 0x0137://风帘级别
			{
				
				Por_Mechanism_Ventilation_curtain_rating(data,Counter);
			}break;
			case 0x001E:
			{
				Lengthacquisition(data,Counter);
			}break;
			case 0x021E://搅扇等级
			{
				
				Por_Mechanism_Stirfan(data,Counter);
			}break;
			case 0x013D://搅扇程序
			{
				if(Counter == 22)
				Por_Mechanism_Stirring_fanprogram(data,Counter);
			}break;
			case 0x0125://喷雾器
			{
				Por_Mechanism_sprayer(data,Counter);
			}break;
			case 0x0124://制冷版
			{
				Por_Mechanism_Refrigeratedversion(data,Counter);
			}break;
			case 0x41CE://无饲料灯光???
			{
				Por_Mechanism_light_Nofeed(data,Counter);
			}break;
			case 0x41CF://有饲料灯光???
			{
				Por_Mechanism_light_feed(data,Counter);
			}break;
			case 0x41D8:
			{
				Por_Mechanism_water_feed(data,Counter);
			}break;
			case 0x4128://附加系统
			{
				Pro_Mechanism_Additionalsystem(data,Counter);
			}break;
			case 0x00A5:
			{
				Lengthacquisition(data,Counter);
			}break;
			case 0x83A5://water on demand
			{
				Por_Mechanism_WaterOnDemand(data,Counter);
			}break;
			case 0x42E0:
			{
				Por_Mechanism_Natural(data,Counter);
			}break;
			case 0x04C2:
			{
				Pro_Mechanism_Feed_Scale(data,Counter);
			}break;
			
			//管理
			case 0x020C:
			{
				Por_Management_ChickenSum(data,Counter);
			}break;
			case 0x0361:
			{
				Por_management_Feed(data,Counter);
			}break;
			case 0x015D://时间和日期
			{
				Por_MangTimeDay(data,Counter);
			}break;
			case 0x015C:
			{
				Por_MangTimeDay(data,Counter);
			}break;
			case 0x0410B:
			{
				Por_MangDayoldflock(data,Counter);
			}break;
			case 0x010D:
			{
				Por_MangDayoldflock(data,Counter);
			}break;
			case 0x083B:
			{
				Por_MangAlarm_system(data,Counter);
			}break;
			case 0x0191://没有成功的设置
			{
				Pro_Mang_Nosuccessfulsettingssaved(data,Counter);
			}break;
			case 0x0112:
			{
				Pro_MangPassword(data,Counter);
			}break;
			//测试
			case 0x0248:
			{
				Pro_TestSwitchrelay(data,Counter);
			}break;
			case 0x027A://模拟传感器
			{
				Pro_TestAnalogsensor(data,Counter);
			}break;
			case 0x027B:
			{
				Pro_TestDialogSenSor(data,Counter);
			}break;
			case 0x017C:
			{
				Pro_TestStaticpressure(data,Counter);
			}break;
			
			//服务
			case 0x0113://温度校准
			{
				Pro_Service_Temperaturecalibration(data,Counter);
			}break;
			case 0x0211:    //湿度校准
			{
				Pro_Server_Humiditycalibration(data,Counter);
			}break;
			case 0x032f:
			{
				Pro_Server_FeedCalibration(data,Counter);
			}break;
			case 0x0230:
			{
				Pro_Server_WaterCalibration(data,Counter);
			}break;
			case 0x0146:
			{
				Pro_Server_Pluginterface(data,Counter);
			}break;
			case 0x018B:
			{
				Pro_Server_Waterlinenipplerinse(data,Counter);
			}break;
			case 0x0189:
			{
				Pro_Server_Waterfeeder(data,Counter);
			}break;
			case 0x02E3:
			{
				Pro_Server_AmmoniaCalibration(data,Counter);
			}break;
			//安装
			case 0x080E: //设置
			{
				Pro_Install_Setting(data,Counter);
			}break;
			case 0x001C:
			{
				Lengthacquisition(data,Counter);//这个函数用于提前或者数据的总长度
			}break;
			case 0x041C:
			{
				Pro_Install_Relaysensor(data,Counter);
			}break;
			case 0x0036:
			{
				Lengthacquisition(data,Counter);//长度获取
			}break;
			case 0x0336://风帘设置
			{
				Pro_Install_Curtainsetting(data,Counter);
			}break;
			case 0x0035:
			{
				Por_Temperaturedefinition_sum(data,Counter);
			}break;
			case 0x0335:
			{
				Pro_Install_Temperaturedefinition(data,Counter);
			}break;
			case 0x0042:
			{
				Lengthacquisition(data,Counter);
			}break;
			case 0x0142:
			{
				Pro_Install_Fanaircapacity(data,Counter);
			}break;
			case 0x0143:
			{
				Pro_InstallHenSize(data,Counter);
			}break;
			case 0x0117:
			{
				Pro_InstallCommunication(data,Counter);
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
