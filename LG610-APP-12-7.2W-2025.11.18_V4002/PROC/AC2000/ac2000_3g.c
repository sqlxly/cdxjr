#include "ac2000_3g.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "data_table.h"
#include "humiture.h"
#include "mynet.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern int user_usart_pass(rt_uint8_t *send_buf, int send_len, int read_bufsz);
/**************注意*************************
	AC2000设备信息
	设备颜色：黑色
	设备名称：ROTEM 3G
	设备系统版本：9.09R13
****************************************/

G3Ac2000Params *Ac20003GParams=NULL;
unsigned int TimerWater;
unsigned int TimerFeed; 
unsigned int TimerHeater;
unsigned int TimerRadiant_Heater;
unsigned int TimerAlarm;
unsigned int TimerMap;
unsigned int TimerWatch;
int AC2000RecvFlag;
extern unsigned char Set_difference;
unsigned short Number_relaystate3g=0;
unsigned short Number_relays3g=0;
extern unsigned char EquipmentState;//设备状态
extern unsigned char ACCESS_DIFERENT3g;
unsigned char FullState=0;
unsigned char FullDataState=0;//数据查询是不是恰好满数据帧
void Ac2000Data3GMalloc(void)
{
	  rt_kprintf("\r\n");
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
		
		
		if(Ac20003GParams==NULL)
		{
			Ac20003GParams=(G3Ac2000Params *)rt_malloc(sizeof(G3Ac2000Params));
			if(Ac20003GParams!=NULL)
			{
				rt_memset(Ac20003GParams,0,sizeof(G3Ac2000Params));
				int len=sizeof(G3Ac2000Params);
				rt_kprintf("\nAc20003GParams size:%d\n",len);
			}
			else
			{
				rt_kprintf("Ac20003GParams创建失败!\n");
			}
		}
//		if(Ac20003GOldParams==NULL)
//		{
//			Ac20003GOldParams=(G3Ac2000Params *)rt_malloc(sizeof(G3Ac2000Params));
//			if(Ac20003GOldParams!=NULL)
//			{
//				rt_memset(Ac20003GOldParams,0,sizeof(G3Ac2000Params));
//				int len=sizeof(G3Ac2000Params);
//				rt_kprintf("\nAc20003GOldParams size:%d\n",len);
//			}
//			else
//			{
//				rt_kprintf("Ac20003GOldParams创建失败!\n");
//			}
//		}

}

/************************控制********************************/
const unsigned char AC2000Cmd[AC2000_3g_CmdNum][AC2000CmdLength]=/*命令码表*/
{
/*000 扫描鸡舍号0000_0000_01		*/			{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},
/*001 主屏幕0479_0000_50	*/		    		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x04,0x79,0x01,0x00,0x00,0x50,0xd0},
/*002 报警查询0002_0000_0C	*/		  		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x02,0x01,0x00,0x00,0x0C,0x11},
/*003 读取温度*/				                {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x13,0x01,0x00,0x00,0x3C,0x53},
/*004 水和饲料*/					            {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x03,0x4B,0x01,0x00,0x00,0x21,0x72},
/*005 继电器*/                      		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x48,0x01,0x00,0x00,0xF7,0x43},
/*006 继电器*/                      		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x48,0x01,0xF7,0x00,0x85,0xC8},
/*007 曲线状态*/                    		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x52,0x01,0x00,0x00,0x1A,0x70},
/*008 温度和湿度状态*/              		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x50,0x01,0x00,0x00,0x1A,0x6E},
/*系统时间*/									{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x5d,0x01,0x00,0x00,0x04,0x65}
};


const unsigned char AC2000ControlTemperatureCmd[3][AC2000CmdLength]=/*命令码表*/
{
/*000 温度曲线1*/		     								{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1D,0x01,0x00,0x00,0xE0,0x02},
/*001 温度曲线2*/		     								{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1D,0x01,0xE0,0x00,0xE0,0xE2},
/*002 温度曲线3*/		     								{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1D,0x01,0xC0,0x01,0x20,0x03}    
};

const unsigned char AC2000ControlHumidityCmd[1][AC2000CmdLength]=/*命令码表*/
{
/*003 湿度处理*/	   {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x22,0x01,0x00,0x00,0xA8,0xCE}   //最后加1
};
const unsigned char AC2000ControlMaxMinCmd[1][AC2000CmdLength]=/*命令码表*/
{
/*004 最低/最高级别*/{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x1F,0x01,0x00,0x00,0x42,0x65}    
};
const unsigned char AC2000ControlStaticPressureCmd[1][AC2000CmdLength]=/*命令码表*/
{
/*005 静压*/         {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x2A,0x01,0x00,0x00,0x30,0x5E}   
};
const unsigned char AC2000ControlModeCmd[1][AC2000CmdLength]=/*命令码表*/
{
/*006 控制模式*/     {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x2B,0x01,0x00,0x00,0x09,0x39},    
};
const unsigned char AC2000ControlCo2Cmd[1][AC2000CmdLength]=/*命令码表*/
{
/*007 CO2处理*/      {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x92,0x01,0x00,0x00,0xD2,0x68}      
};


const unsigned char AC2000SysInfoServiceCmd[17][AC2000CmdLength]=/*命令码表*/
{
/*000 0x0165 系统参数 温度曲线*/		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x65,0x01,0x00,0x00,0x1C,0x85},
/*001 0x0162 系统参数 CYCLE HEATER*/		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x62,0x01,0x00,0x00,0x0C,0x72},
/*002 0x4166 系统参数 辐射加热器*/		  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x41,0x66,0x01,0x00,0x00,0x0A,0xB4},
/*003 0x03a6 VARIABLE HEATER*/         	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x03,0xA6,0x01,0x00,0x00,0x12,0xBE},
/*004 0x026c 湿度处理*/    						  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x6C,0x01,0x00,0x00,0x07,0x78},
/*005 0x0286 CO2处理*/             			{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x86,0x01,0x00,0x00,0x06,0x91},
/*006 0x0269 静压*/         						{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x69,0x01,0x00,0x00,0x1A,0x88},
/*007 0x0167 通风级别 */          			{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x67,0x01,0x00,0x00,0x14,0x7F},
/*008 0x036e 卷帘和屋顶*/  							{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x03,0x6E,0x01,0x00,0x00,0x24,0x98},
/*009 0x01F9 ENT CURTAIN CALIBRATION*/  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xF9,0x01,0x00,0x00,0x0B,0x08},
/*010 0x026a 湿帘 COOL PAD FLUSH*/      {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x6A,0x01,0x00,0x00,0x14,0x83},
/*011 0x026b 加湿器*/                   {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x6B,0x01,0x00,0x00,0x10,0x80},
/*012 0x016f 灯光*/                     {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x6F,0x01,0x00,0x00,0x10,0x83},
/* 水和饲料*/                           {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x87,0x01,0x00,0x00,0x14,0xA0},
/*饲料存量*/                            {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x80,0x01,0x00,0x00,0x18,0x9D},
/*饲料存量中的料斗标志*/                 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x70,0x01,0x00,0x00,0x01,0x76},
/*013 0x0172 报警设置*/                 {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x72,0x01,0x00,0x00,0x01,0x77}
};

/************************设备********************************/
const unsigned char AC2000_Device_LevelsOfVentilation[2][AC2000CmdLength]=/*设备 通风级别*/
{
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x04,0x4C},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x99,0x01,0x00,0x00,0xfe,0x9c},
		
};

const unsigned char AC2000_Device_VariableSpeedFanLevels[2][AC2000CmdLength]=/*设备 调速风扇级别*/
{
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x44,0x01,0x00,0x00,0x04,0x4E},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x44,0x01,0x00,0x00,0x80,0xC9}
};

const unsigned char AC2000_Device_VentAndCurtianLevels[2][AC2000CmdLength]=/*设备 通风和风帘级别*/
{
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x37,0x01,0x00,0x00,0x04,0x41},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x37,0x01,0x00,0x00,0x66,0xA1}
};

const unsigned char AC2000_Device_StirFanLevels[2][AC2000CmdLength]=/*搅扇级别*/
{
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x1e,0x01,0x00,0x00,0x04,0x28},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1e,0x01,0x00,0x00,0x44,0x67}
};
const unsigned char AC2000_Device_StirFanProgram[1][AC2000CmdLength]=/*搅扇程序*/
{
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x3d,0x01,0x00,0x00,0x0f,0x50}
};
const unsigned char AC2000_Device_Foggers[2][AC2000CmdLength]=/*命令码表*/
{
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x25,0x01,0x00,0x00,0xF0,0x19},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x25,0x01,0xF0,0x00,0x60,0x79}
};
const unsigned char AC2000_Device_CoolPad[2][AC2000CmdLength]=/*命令码表*/
{
	  {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x24,0x01,0x00,0x00,0xFE,0x26},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x24,0x01,0xFE,0x00,0x52,0x78}
};
const unsigned char AC2000_Device_Light[4][AC2000CmdLength]=/*命令码表*/
{
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xCF,0x01,0x00,0x00,0xfc,0xCF},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xCF,0x01,0xFC,0x00,0xFC,0xCB},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xCF,0x01,0xF8,0x01,0xFC,0xC8},
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xCF,0x01,0xF4,0x02,0x90,0x59}	
};
const unsigned char AC2000_Device_WaterAndFeed[5][AC2000CmdLength]=/*命令码表*/
{	
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xd0,0x01,0x00,0x00,0xf0,0xc4},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xd0,0x01,0xf0,0x00,0xf0,0xb4},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xd0,0x01,0xe0,0x01,0xf0,0xa5},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xd0,0x01,0xd0,0x02,0xf0,0x96},
		{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0xd0,0x01,0xc0,0x03,0xf0,0x87}		
};
const unsigned char AC2000_Device_ExtraSystem[1][AC2000CmdLength]=/*命令码表*/
{
		{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x41,0x28,0x01,0x00,0x00,0x50,0xBC}
};
const unsigned char AC2000_Device_WaterOnDemand[2][AC2000CmdLength]=/*命令码表*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x0E,0x01,0xA5,0x01,0x00,0x00,0x04,0xBD},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x0E,0x03,0xA5,0x01,0x00,0x00,0xFE,0xB7},
};

const unsigned char AC2000_ManageStockOfChicken[1][AC2000CmdLength]=/*管理-鸡的存量*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_ManageStockOfFeed[5][AC2000CmdLength]=/*管理-饲料的存量*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x61,0x01,0x00,0x00,0xFA,0x60},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x61,0x01,0xFA,0x00,0xFA,0x5A},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x61,0x01,0xF4,0x00,0xFA,0x55},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x61,0x01,0xEE,0x02,0xFA,0x50},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x61,0x01,0x08,0x03,0x0C,0x5D}
};
const unsigned char AC2000_ManageTimeAndDate[2][AC2000CmdLength]=/*管理-时间和日期*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x5d,0x01,0x00,0x00,0x04,0x65},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x5c,0x01,0x00,0x00,0x04,0x64}
};
const unsigned char AC2000_ManageGrowthRegionAndHen[1][AC2000CmdLength]=/*管理-生长期和鸡群*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0D,0x01,0x00,0x00,0x04,0x15}
};
const unsigned char AC2000_ManageAlarmSet[4][AC2000CmdLength]=/*管理-警报设置*/
{
	//{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x3B,0x01,0x00,0x00,0x04,0x45},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x3B,0x01,0x00,0x00,0xF0,0x30},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x3B,0x01,0xF0,0x00,0xF0,0x20},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x3B,0x01,0xE0,0x01,0xF0,0x11},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x3B,0x01,0xD0,0x02,0x60,0x72}
};
const unsigned char AC2000_ManageAlarmReset[1][AC2000CmdLength]=/*管理-警报重置*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x04,0x01,0x00,0x00,0x0c,0x14}
};
const unsigned char AC2000_ManagePassword[1][AC2000CmdLength]=/*管理-密码*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x0E,0x01,0x12,0x01,0x00,0x00,0x0E,0x31 }
};

//鸡秤
const unsigned char AC2000_ChickSetting[1][AC2000CmdLength]= /*鸡秤基本设置*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x3A,0x01,0x00,0x00,0x0E,0x4D}
};

const unsigned char AC2000_Chicklayout[4][AC2000CmdLength]= /*鸡秤的布局*/
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x1C,0x01,0x00,0x00,0xF2,0x14},
};


const unsigned char AC2000_ChickCurce[3][AC2000CmdLength]= //鸡秤曲线
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x2C,0x01,0x00,0x00,0xEA,0x1B},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x2C,0x01,0xD4,0x01,0xEA,0xF0},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x2C,0x01,0xBE,0x02,0x4E,0x3F},
};

const unsigned char AC2000_ChickConversion[1][AC2000CmdLength]=//鸡秤转换
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x0F,0x00,0x01,0x01,0x93,0x01,0x00,0x00,0x46,0xEB}
};

const unsigned char AC2000_ChickTest[1][AC2000CmdLength]= //鸡秤测试
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x5A,0x01,0x00,0x00,0x0E,0x6D}
};

const unsigned char AC2000_ChickCalibration[1][AC2000CmdLength]= //鸡秤校准
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x11,0x01,0x00,0x00,0x16,0x2C}
};

const unsigned char AC2000_History_Temperature[1][AC2000CmdLength]=/*历史温度*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Humidity_treatm[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_CO2[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Water[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Feed[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Dead[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Heater[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Radiant_Heater[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Alarm[1][AC2000CmdLength]=/*历史*/
{
	
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Event_Map[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Watch[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
const unsigned char AC2000_History_Set[1][AC2000CmdLength]=/*历史*/
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x0c,0x01,0x00,0x00,0x24,0x34}
};
unsigned char AC2000_Get_length[AC2000CmdLength]=/*获取数据长度*/
{
	0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x96,0x01,0x00,0x00,0x0C,0xA7
};
//测试
const unsigned char AC2000_Test_SwitchRelay[2][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x48,0x01,0x00,0x00,0xF7,0x43},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x48,0x01,0xF7,0x00,0x85,0xC8}
};
const unsigned char AC2000_Test_AnalogSensor[2][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x7a,0x01,0x00,0x00,0x04,0x84},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x7a,0x01,0x00,0x00,0x7e,0xfd}
};
const unsigned char AC2000_Test_DigitalSensor[2][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x7b,0x01,0x00,0x00,0x04,0x85},
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x7b,0x01,0x00,0x00,0x12,0x92,}
};
const unsigned char AC2000_Test_StaticPressure[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x7c,0x01,0x00,0x00,0x02,0x82}
};

//服务
const unsigned char AC2000_Service_Temperature[1][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x13,0x01,0x00,0x00,0x3C,0x53}
};
const unsigned char AC2000_Service_Humidity[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x10,0x01,0x00,0x00,0x1e,0x32}
};
const unsigned char AC2000_Service_Feed[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x2f,0x01,0x00,0x00,0x18,0x4b}
};
const unsigned char AC2000_Service_Water[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x30,0x01,0x00,0x00,0x10,0x44}
};

//安装
const unsigned char AC2000_Install_NormalSet[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x04,0x90,0x01,0x00,0x00,0x27,0xbe}
};
const unsigned char AC2000_Install_RelaySetUp[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x04,0x01,0x00,0x00,0x0c,0x14}
};
const unsigned char AC2000_Install_AnalogSensor[2][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x1C,0x01,0x00,0x00,0x04,0x26},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1C,0x01,0x00,0x00,0xF2,0x13},
};
const unsigned char AC2000_Install_DigitalSensor[1][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x03,0x4B,0x01,0x00,0x00,0x21,0x72}
};	
//const unsigned char AC2000_Install_AnalogOutput[1][AC2000CmdLength]=
//{
//	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x02,0x1C,0x01,0xF2,0x00,0xF2,0x05}//模拟输出
//};
const unsigned char AC2000_Install_VentCurtainSetUp[2][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x0E,0x01,0x36,0x01,0x00,0x00,0x04,0x4D},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x0E,0x02,0x36,0x01,0x00,0x00,0x54,0x9D },
};	
const unsigned char AC2000_Install_TemperatureDefine[2][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x00,0x01,0x35,0x01,0x00,0x00,0x04,0x3F},//温度定义
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x35,0x01,0x00,0x00,0x5A,0x93}
};	
const unsigned char AC2000_Install_CapacityOfFanAir[2][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x04,0x00,0x01,0x01,0x42,0x01,0x00,0x00,0x04,0x5A},//查询单位
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x0E,0x01,0x42,0x01,0x00,0x00,0x98,0xEC },//风机空气容量ID

};	
const unsigned char AC2000_Install_HenSize[1][AC2000CmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x43,0x01,0x00,0x00,0x10,0x57}//鸡舍大小
};	
const unsigned char AC2000_Install_Communication[1][AC2000CmdLength]=
{
	{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x01,0x17,0x01,0x00,0x00,0x02,0x1D}//通信
};	

/************************管理********************************/
void CreateAC20003GSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000Cmd[CmdIndex][8];
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000Cmd[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000Cmd[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000Cmd[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000Cmd[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000Cmd[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000Cmd[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000Cmd[CmdIndex][17]+HenHouseNum;
}


void CreateAC20003GControlSysInfoSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000SysInfoServiceCmd[CmdIndex][8];;
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000SysInfoServiceCmd[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000SysInfoServiceCmd[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000SysInfoServiceCmd[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000SysInfoServiceCmd[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000SysInfoServiceCmd[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000SysInfoServiceCmd[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000SysInfoServiceCmd[CmdIndex][17]+HenHouseNum;
	
//	rt_kprintf("mcu send:\r\n");
//	for(uint16_t i=0;i<18;i++)
//	{
//		rt_kprintf("%02x ",AC2000_SendBuff[i]);
//	}
//	rt_kprintf("\r\n");
}

void CreateAC20003GDeviceLevelsOfVentilation(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000_Device_LevelsOfVentilation[CmdIndex][8];;
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000_Device_LevelsOfVentilation[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000_Device_LevelsOfVentilation[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000_Device_LevelsOfVentilation[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000_Device_LevelsOfVentilation[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000_Device_LevelsOfVentilation[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000_Device_LevelsOfVentilation[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000_Device_LevelsOfVentilation[CmdIndex][17]+HenHouseNum;
	
//	rt_kprintf("mcu send:\r\n");
//	for(uint16_t i=0;i<18;i++)
//	{
//		rt_kprintf("%02x ",AC2000_SendBuff[i]);
//	}
//	rt_kprintf("\r\n");
}

void CreateAC20003GVariableSpeedFanLevels(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][8];;
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][15];
	if(CmdIndex==0)
	{
		AC2000_SendBuff[16]=AC2000_Device_VariableSpeedFanLevels[CmdIndex][16];
	}
	else
	{
		AC2000_SendBuff[16] = Ac20003GParams->Ac2000Device.variable_speed_fan_levelsFanNum;
	}
	AC2000_SendBuff[17] = AC2000_Device_VariableSpeedFanLevels[CmdIndex][17]+HenHouseNum;
	
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

void CreateAC20003GNormalOneParam(const uint8_t CmdSource[][AC2000CmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,uint8_t param)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = CmdSource[CmdIndex][8];;
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = CmdSource[CmdIndex][11];
	AC2000_SendBuff[12] = CmdSource[CmdIndex][12];
	AC2000_SendBuff[13] = CmdSource[CmdIndex][13];
	AC2000_SendBuff[14] = CmdSource[CmdIndex][14];
	AC2000_SendBuff[15] = CmdSource[CmdIndex][15];
	if(CmdIndex==0)
	{
		AC2000_SendBuff[16]=CmdSource[CmdIndex][16];
	}
	else
	{
		AC2000_SendBuff[16] = param;
	}
	AC2000_SendBuff[17] = CmdSource[CmdIndex][17]+HenHouseNum;
	
	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	AC2000_SendBuff[17]=sum;

	rt_kprintf("mcu send:\r\n");
	for(uint16_t i=0;i<18;i++)
	{
		rt_kprintf("%02x ",AC2000_SendBuff[i]);
	}
	rt_kprintf("\r\n");
}
void CreateAC20003GNormal(const uint8_t CmdSource[][AC2000CmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
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
void OverLoadSendOneParamSpecialCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t param)
{
	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		if((*cmd_index)==0||(*cmd_index)==1)
		{
			CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,(*cmd_index));
			(*cmd_index)++;
			AC2000RecvFlag=(AC2000SendBuff[16]+7);
		}
		else
		{
			CreateAC20003GNormalOneParam(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,(*cmd_index),param);
			(*cmd_index)++;
			AC2000RecvFlag=23;
		}
		
		rt_kprintf("AC2000RecvFlag: %d \n",AC2000RecvFlag);
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d \n",AC2000SendBuff[11],AC2000SendBuff[12],AC2000RecvFlag);
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);	
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}							
}
void OverLoadSendOneParamCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t param)
{
	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormalOneParam(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,(*cmd_index),param);
		(*cmd_index)++;
		AC2000RecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("AC2000RecvFlag: %d \n",AC2000RecvFlag);
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d \n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);	
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}							
}
void OverLoadSendSpecialCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index)//通风级别
{
	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,(*cmd_index));
		(*cmd_index)++;
		if((*cmd_index)==cmd_num)
		{
			Ac20003GParams->Ac2000Device.LevelsOfVentilationCmdCount=1;
		}
		else
		{
			Ac20003GParams->Ac2000Device.LevelsOfVentilationCmdCount=0;
		}
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d \n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOutSpecial(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);	//取值发送
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}	
}
void OverLoadSendNormalCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t i)
{
	
	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,(*cmd_index));   //参数赋值
		(*cmd_index)++;
		AC2000RecvFlag=(AC2000SendBuff[16]+7);
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);	
//		if(result==1)rt_kprintf("recieve timeout\n");
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}
}
void connect_ac2000_3g(uint8_t *buff,uint8_t house)
{
	CreateAC20003GSendBuf(buff,house,0);
}
void RelayCreateAC20003gNormal(const uint8_t CmdSource[][AC2000CmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,unsigned short dex,unsigned short prov,unsigned char state,unsigned short prov1)
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
unsigned char SendFlip=0;
unsigned char State_3g=1;
void relaySend_3g(const uint8_t Cmd[][AC2000CmdLength],unsigned short porv,unsigned char difference)
{
		unsigned short Quotient=0;
		unsigned short Morethan=0;
	
	if(difference==1)
	{
		if(SendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,0);   //参数赋值
		AC2000RecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
    SendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(SendFlip==1)
	{
		Quotient =Number_relays3g/porv;
		Morethan =Number_relays3g%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient==0)&&(Morethan)==0)//没有数据直接填空发送
			{

				AirxxxStatus.recv_systeminfo_cmd=2;	
				SendFlip=0;		
				Write_data_Empty();
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
			}
			else if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,porv,0,porv);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				FullDataState=1;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,Morethan,0,Morethan);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);

				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					FullDataState=1;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					State_3g=State_3g+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,1,Morethan);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					my_log(AC2000SendBuff,18);
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_3g=State_3g+1;
					my_log(AC2000SendBuff,18);
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,3000,3,&AC2000RecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
	}
	}
 else if(difference==2)
 {
	 if(SendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,0);   //参数赋值
		AC2000RecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
    SendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(SendFlip==1)
	{
		Quotient =Number_relaystate3g/porv;
		Morethan =Number_relaystate3g%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,porv,0,porv);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,Morethan,0,Morethan);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);

				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);

					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					State_3g=State_3g+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,1,Morethan);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					my_log(AC2000SendBuff,18);
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_3g=State_3g+1;
					my_log(AC2000SendBuff,18);
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 }
  else if(difference==3)//
 {
	 if(SendFlip==0)//发送查询帧
	{
		uint8_t AC2000SendBuff[AC2000CmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC20003GNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,0);   //参数赋值
		AC2000RecvFlag=(AC2000SendBuff[16]+7);
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//		int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//    SendFlip=1;		
//		if(result==1)rt_kprintf("recieve timeout\n");
	}else if(SendFlip==1)
	{
		Quotient =Number_relays3g/porv;
		Morethan =Number_relays3g%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,porv,0,porv);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				EquipmentState=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,Morethan,0,Morethan);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				EquipmentState=1;
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					EquipmentState=1;
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					State_3g=State_3g+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,1,Morethan);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					EquipmentState=1;
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_3g=State_3g+1;
					my_log(AC2000SendBuff,18);
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 }else if(difference==4)//
 {
	 
		Quotient =Number_relaystate3g/porv;
		Morethan =Number_relaystate3g%porv;
		rt_kprintf("Quotient%d,Morethan%d",Quotient,Morethan);
		if(Morethan!=0)
		{
			Quotient =Quotient+1;
		}
			if((Quotient == 1)&&(Morethan==0))//只发送一条完整数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,porv,0,porv);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				EquipmentState=0;
				FullState=1;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//			int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
			SendFlip=0;		
//			if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient==1)&&(Morethan!=0))//只发送一条未满数据
			{
				uint8_t AC2000SendBuff[AC2000CmdLength];
				memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
				RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,1,Morethan,0,Morethan);
				AC2000RecvFlag=(AC2000SendBuff[16]+7);
				EquipmentState=0;
				AirxxxStatus.aliyun_service_param1=-1;
				AirxxxStatus.aliyun_service_param2=-1;
				AirxxxStatus.recv_systeminfo_cmd=2;	
				rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
				user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//				int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				SendFlip=0;		
//				if(result==1)rt_kprintf("recieve timeout\n");
			}
			else if((Quotient!=1)&&(Morethan==0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					EquipmentState=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_3g=1;
					FullState=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
				
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					State_3g=State_3g+1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//				
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			}
			else if((Quotient!=1)&&(Morethan!=0))
			{
				if(State_3g>=Quotient)
				{
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,1,Morethan);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					EquipmentState=0;
					AirxxxStatus.aliyun_service_param1=-1;
					AirxxxStatus.aliyun_service_param2=-1;
					AirxxxStatus.recv_systeminfo_cmd=2;	
					State_3g=1;
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					my_log(AC2000SendBuff,18);
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					SendFlip=0;		
//					if(result==1)rt_kprintf("recieve timeout\n");
				}
				else
				{
					
					uint8_t AC2000SendBuff[AC2000CmdLength];
					memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
					RelayCreateAC20003gNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_3g_house,1,State_3g,porv,0,porv);
					AC2000RecvFlag=(AC2000SendBuff[16]+7);
					rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
					State_3g=State_3g+1;
					my_log(AC2000SendBuff,18);
					user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000CmdLength,AC2000RecvFlag);
//					int result=Usart_SendArrayTimeOut(AC2000SendBuff,AC2000CmdLength,2000,3,&AC2000RecvFlag);
//					if(result==1)rt_kprintf("recieve timeout\n");
				
				}
			
			}
		}
 
}



void Length_acquisition3g(unsigned char *dataSource,uint16_t len)
{
	Number_relays3g = dataSource[6]+dataSource[7]*256;
	Number_relaystate3g = (((((Number_relays3g/8)*6)+8)*30)+12);//只用于计算下发询问通风等级的帧数量
	rt_kprintf("%d\n",Number_relays3g);

}


static unsigned char Parse_MainScreen(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 04 79 01 93 
	0f 04 cc 01 0e 00 03 02 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	a0 09 01 00 08 00 00 00 e3 b5 00 00 00 00 00 00 6a 0a 90 06 90 06 90 06 10 27 00 00 00 00 10 27 10 27 
	01 00 01 01 00 ff 00 01 11 00 00 05 32 
	*/
	unsigned int postion,TmpInt;
	float	TmpFloat;

	postion=38;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion])/100;
	Ac2000Data[ADDR_3G_1802] = ((unsigned char)(TmpInt&0x00ff))+((unsigned char)((TmpInt&0xff00)>>8));//当前静压

	postion=40;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion])/100;
	Ac2000Data[ADDR_3G_1803] = ((unsigned char)(TmpInt&0x00ff))+((unsigned char)((TmpInt&0xff00)>>8));//目标静压

	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	Ac2000Data[ADDR_3G_1804] = ((unsigned char)(TmpInt&0x00ff))+((unsigned char)((TmpInt&0xff00)>>8));//日龄

	postion=56;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1847] = TmpFloat;//目标温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1847] = 0xffff-TmpInt;
	}	
	if(Ac2000Data[ADDR_3G_1847]==0x2710) Ac2000Data[ADDR_3G_1847]=9999;
	
	postion+=4;
	TmpFloat = (float)((float)(dataSource[postion+1]*256+dataSource[postion]));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1807] = TmpFloat;//平均温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1807] = TmpInt  = 0xffff-TmpInt;
	}		
	if(Ac2000Data[ADDR_3G_1807]==0x2710) Ac2000Data[ADDR_3G_1807]=9999;
	
	postion+=2;
	TmpFloat = (float)((float)(dataSource[postion+1]*256+dataSource[postion]));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1849] = TmpFloat;//通道温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1849] = TmpInt  = 0xffff-TmpInt;
	}		
	if(Ac2000Data[ADDR_3G_1849]==0x2710) Ac2000Data[ADDR_3G_1849]=9999;
	
	postion+=2;	
	TmpFloat=(float)((float)(dataSource[postion+1]*256+dataSource[postion]));	
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1808] =  TmpFloat;//室外温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1808] = TmpInt  = 0xffff-TmpInt;
	}
	if(Ac2000Data[ADDR_3G_1808]==0x2710) Ac2000Data[ADDR_3G_1808]=9999;
	
	postion+=8;
	TmpFloat=(float)((float)(dataSource[postion+1]*256+dataSource[postion]));	
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1809] =  TmpFloat;//屋顶温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1809] = TmpInt  = 0xffff-TmpInt;
	}
	if(Ac2000Data[ADDR_3G_1809]==0x2710) Ac2000Data[ADDR_3G_1809]=9999;
	
	postion+=5;
	Ac2000Data[ADDR_3G_1805]=dataSource[postion];	//通风等级
	
	postion+=2;	
	TmpFloat = dataSource[postion];
	if(TmpFloat==0xFF)
	{
		Ac2000Data[ADDR_3G_1806] =9999;//湿度传感器未启用
	}
	else
	{	
		Ac2000Data[ADDR_3G_1806] =TmpFloat;//内部湿度就是平均湿度
	}
	
	/*体感温度*/
	Ac2000Data[ADDR_3G_1861] = tiWenCalc((Ac2000Data[ADDR_3G_1807]/100), Ac2000Data[ADDR_3G_1806], Ac2000Data[ADDR_3G_1856]);
	
	return postion;
}


//报警解析
static void Parse_Warn(uint8_t *dataSource,uint16_t len)
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
		AlarmCode3g[ALARM_3G_12000+j][1]=0;
	}
	
	for(unsigned char i = 0; i<warnNum; i++)
	{
		alarm_code=dataSource[postion]*256+dataSource[postion+1];//得到告警类型码
		for(unsigned short j=0;j<AC2000_ALARM_NUM_3G;j++)
		{
			if(alarm_code==AC2000WarmCode[j])
			{
				//rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				AlarmCode3g[ALARM_3G_12000+j][1]=1;
				break;
			}
			if(j==(AC2000_ALARM_NUM_3G-1))
			{
				if(alarm_code==AC2000WarmCode[AC2000_ALARM_NUM_3G-1])
				{
					AlarmCode3g[ALARM_3G_12000+j][1]=1;
					//rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				}
				else
				{
					AlarmCode3g[ALARM_3G_12000][1]=1;
					//rt_kprintf("未知报警\r\n");
				}
			}
		}
		postion+=2;
	}
}

static void Parse_curve_state(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 01 52 01 69 6a 0a 54 09 ac 0d 84 6a fb 09 96 0c 01 1e 00 00 00 00 00 00 00 00 00 00 00 00 3d
	*/
	unsigned int postion,TmpInt;
	float	TmpFloat;
	
	postion=6;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1847] = TmpFloat;//目标温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1847] = 0xffff-TmpInt;
	}	
	
	postion=postion+2;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1850] = TmpFloat;//低温警报温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1850] = 0xffff-TmpInt;
	}
	
	postion=postion+2;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1851] = TmpFloat;//高温警报温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1851] = 0xffff-TmpInt;
	}	
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion])/1000;
	Ac2000Data[ADDR_3G_1803] = TmpInt;//目标静压
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	Ac2000Data[ADDR_3G_1848] = TmpInt;//加热器温度

	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	Ac2000Data[ADDR_3G_1810] = TmpInt;//隧道温度
	
	postion=postion+2;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_3G_1852] = TmpInt;//最低级别
	
	postion=postion+1;
	TmpInt=dataSource[postion];
	Ac2000Data[ADDR_3G_1853] = TmpInt;//最高级别
}

// 继电器状态
static void Parse_RelyTest1(uint8_t *dataSource,uint16_t len)
{
	/*
	0f 00 00 01 00 5f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 0f 01 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 01 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 0f 02 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 02 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 05 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 06 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 07 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 07 01 00 01 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 07 02 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ff ff 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 94 

	*/
	unsigned int postion=0;
	unsigned int data=0;
	postion=22;
	
  if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1827]=data;postion=postion+19;//22
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1828]=data;postion=postion+19;//41
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1829]=data;postion=postion+19;//60
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1830]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1831]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1832]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1833]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1834]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1835]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1836]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1837]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1838]=data;postion=postion+19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1839]=data;postion=postion+19;
//	for(uint8_t i=0;i<13;i++)
//	{
//		rt_kprintf("继电器 %d 状态=%d\r\n",(i+1),Ac2000Data[ADDR_1827+i]);
//	}
}
static void Parse_RelyTest2(uint8_t *dataSource,uint16_t len)
{
	unsigned int data=0;
	unsigned int postion=6;
	postion=postion+16;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1840]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1841]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1842]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1843]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1844]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1845]=data;postion+=19;
	if(dataSource[postion]==0x00) {data = 0x00;}else {data = 0x01;}Ac2000Data[ADDR_3G_1846]=data;postion+=19;
//	for(uint8_t i=0;i<7;i++)                                     
//	{
//		rt_kprintf("继电器 %d 状态=%d\r\n",(i+14),Ac2000Data[ADDR_1840+i]);
//	}
	
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		char AliyunServiceData[128];
		char tmp[4];
		memset(AliyunServiceData,0,sizeof(AliyunServiceData));
		for(uint8_t i=0;i<13;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",(i+1));
			strcat(AliyunServiceData,tmp);//序号
			
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",Ac2000Data[ADDR_3G_1827+i]);
			strcat(AliyunServiceData,tmp);//状态
		}
		for(uint8_t i=0;i<7;i++)                                     
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",(i+14));
			strcat(AliyunServiceData,tmp);//序号
			
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",Ac2000Data[ADDR_3G_1840+i]);
			strcat(AliyunServiceData,tmp);
		}
		
		AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Test.SwitchRelay,0,sizeof(Ac20003GParams->Ac2000Test.SwitchRelay));
		strcat(Ac20003GParams->Ac2000Test.SwitchRelay,AliyunServiceData);
		//rt_kprintf("SwitchRelay:%s\r\n",Ac20003GParams->Ac2000Test.SwitchRelay);
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=TEST_SWITCHRELAY_CMD;
	}
}

static void SystemTime(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 5d 01 74 
	da e8 a6 43 
	ab 
	*/
	unsigned int postion;
	int TmpInt;
	postion=6;
	TmpInt=(int)FourByteToFloat(&dataSource[postion]);
	Ac2000Data[ADDR_1266] = TmpInt;
}

static void Parse_AskTmpature(uint8_t *dataSource,uint16_t len)
{
	/*
	01 14 01 13 01 2a 
	01 00 00 04 52 b1 cc 44 00 00 
	01 01 00 04 a5 32 d3 44 00 00 
	01 02 00 04 52 b1 cc 44 00 00 
	01 03 00 04 60 49 d6 44 9c ff 
	01 04 00 04 00 40 1c 46 00 00 
	01 05 00 04 00 40 1c 46 00 00 
	e3 
	*/
		unsigned int postion;
	    uint16_t tmpdata[6];
		unsigned char FloatData[4],j,OffAddress;
		unsigned int TmpInt,TmpInt1;
		unsigned short TmpInt2;
		float	TmpFloat;
		OffAddress = 10;
		for(j=0;j<6;j++)
		{
			postion=OffAddress+j*10;
			FloatData[0]= dataSource[postion];
			postion++;		
			FloatData[1]= dataSource[postion];
			postion++;		
			FloatData[2]= dataSource[postion];
			postion++;		
			FloatData[3]= dataSource[postion];
			TmpFloat = *(float*)FloatData;
      
			tmpdata[j]=dataSource[postion+1]+dataSource[postion+2]*256;//因素
			//rt_kprintf("tmpdata %d =%d\r\n",j,tmpdata[j]);
			
			TmpInt = 0x00000000;
			TmpInt = FloatData[3]<<24;
			
			TmpInt1 = 0x00000000;
			TmpInt1 = FloatData[2]<<16;
			
			TmpInt = TmpInt+TmpInt1;
			
			TmpInt1 = 0x00000000;
			TmpInt1 = FloatData[1]<<8;
			
			TmpInt = TmpInt+TmpInt1;
			TmpInt = TmpInt+FloatData[0];
			
			if((FloatData[3]&0x80)==0x00)
			{
				TmpFloat = TmpFloat;
				if(TmpInt==0x461c4000)
				{
					//TmpFloat = 9999;
				}
				Ac2000Data[ADDR_3G_1821+j] = 	TmpFloat;//传感器
				//rt_kprintf("ADDR_1821+%d=%d\r\n",j,Ac2000Data[ADDR_1821+j]);
			}
			else
			{
				FloatData[3] = FloatData[3]&0x7f;
				TmpFloat = *(float *)FloatData;
				TmpFloat = TmpFloat;
				TmpInt = (unsigned int)TmpFloat;
				TmpInt2 = (unsigned short)(TmpInt&0x0000ffff);
				TmpInt2 = 0xffff-TmpInt2;
				Ac2000Data[ADDR_3G_1821+j] = TmpInt2;//传感器
				//rt_kprintf("ADDR_1821+%d=%d\r\n",j,Ac2000Data[ADDR_1821+j]);
			}			
		}
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			char AliyunServiceData[128];
			
			char tmp[8];
			memset(AliyunServiceData,0,sizeof(AliyunServiceData));
			for(uint8_t i=0;i<6;i++)
			{
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",(i+1));
				strcat(AliyunServiceData,tmp);
				
				memset(tmp,0,sizeof(tmp));
				if(Ac2000Data[ADDR_3G_1821+i]<10000.0)
				{
				sprintf(tmp,"%0.1f,",(Ac2000Data[ADDR_3G_1821+i]/100.0f));
				strcat(AliyunServiceData,tmp);
				}
				else
				{
				strcat(AliyunServiceData,"---,");
				}					
				//rt_kprintf("tmpdata %d =%d\r\n",j,tmpdata[j]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",(tmpdata[i]/1000.0f));
				strcat(AliyunServiceData,tmp);
			}

			AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
			rt_memset(Ac20003GParams->Ac2000Service.Temperature,0,sizeof(Ac20003GParams->Ac2000Service.Temperature));
			strcat(Ac20003GParams->Ac2000Service.Temperature,AliyunServiceData);
		
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=SERVICE_TEMPERATURE_ADJUST_CMD;
		}
}
//水和食料解析
static void Parse_FeedWater(unsigned char *dataSource,uint16_t len)
{
		/*
	01 14 03 4B 01 64 
	00 00 00 00 
	00 00 00 00 82 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 82 
	01 14 03 4B 01 64 
	00 00 00 00 00 00 00 00 
	
	82 00 00 00 
	00 00 00 00 
	
	00 00 00 00 
	00 00 00 00 
	
	00 00 00 00 
	00 00 00 00 
	
	00 
	82 
	*/

	unsigned int WData=dataSource[17]*256*256*256+dataSource[16]*256*256+dataSource[15]*256+dataSource[14];	
	Ac2000Data[ADDR_3G_1817]=WData;//水表1
	WData=dataSource[21]*256*256*256+dataSource[20]*256*256+dataSource[19]*256+dataSource[18];
	Ac2000Data[ADDR_3G_1818]=WData;//水表2
	//rt_kprintf("水表1=%d\r\n",Ac2000Data[ADDR_1817]);
	//rt_kprintf("水表2=%d\r\n",Ac2000Data[ADDR_1818]);
	
	WData=dataSource[33]*256*256*256+dataSource[32]*256*256+dataSource[31]*256+dataSource[30];	
	Ac2000Data[ADDR_3G_1819]=WData;//饲料1
	WData=dataSource[37]*256*256*256+dataSource[36]*256*256+dataSource[35]*256+dataSource[34];
	Ac2000Data[ADDR_3G_1820]=WData;//饲料2
	//rt_kprintf("饲料1=%d\r\n",Ac2000Data[ADDR_1819]);
	//rt_kprintf("饲料2=%d\r\n",Ac2000Data[ADDR_1820]);
}
static void Parse_humiture(unsigned char *dataSource,uint16_t len)
{
	/*
		01 14 01 50 01 67 97 0A 10 27 10 27 97 0A 10 27 97 0A 10 27 10 27 10 27 0A 00 98 21 C8 19 C8 19 38 
	*/
	unsigned int postion=0;
	unsigned int TmpInt;
	
	postion=6;
	float TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1807] = TmpFloat;//平均温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt/10;
		Ac2000Data[ADDR_3G_1807] = 0xffff-TmpInt;
	}	
	//rt_kprintf("平均温度=%d\r\n",Ac2000Data[ADDR_1807]);
	
	postion=postion+2;
	TmpFloat=(float)(((float)(dataSource[postion+1]*256+dataSource[postion])));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1808] = TmpFloat;//外部温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1808] = 0xffff-TmpInt;
	}	
	//rt_kprintf("外部温度=%d\r\n",Ac2000Data[ADDR_1808]);
	
	postion+=2;
	TmpFloat=(float)((float)(dataSource[postion+1]*256+dataSource[postion]));	
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1809] =  TmpFloat;//屋顶温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1809] = TmpInt  = 0xffff-TmpInt;
	}
	//rt_kprintf("屋顶温度=%d\r\n",Ac2000Data[ADDR_1809]);
	
  postion+=2;
	TmpFloat = (float)((float)(dataSource[postion+1]*256+dataSource[postion]));
	if((dataSource[postion+1]&0x80)==0x00)
	{
		Ac2000Data[ADDR_3G_1810] = TmpFloat;//隧道温度
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1810] = TmpInt  = 0xffff-TmpInt;
	}		
	//rt_kprintf("隧道温度=%d\r\n",Ac2000Data[ADDR_1810]);
	
	postion+=2;
	TmpFloat=(float)((float)(dataSource[postion+1]*256+dataSource[postion]));	
	if((dataSource[postion+1]&0x80)==0x00)
	{
		if(TmpFloat!=0X2710)
		{
			Ac2000Data[ADDR_3G_1811] =  TmpFloat;//冷风温度
		}
		else
		{
			Ac2000Data[ADDR_3G_1811]=9999;
		}
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1811] = TmpInt  = 0xffff-TmpInt;
	}
	//rt_kprintf("冷风温度=%d\r\n",Ac2000Data[ADDR_1811]);
	
	postion+=2;
	TmpFloat=(float)((float)(dataSource[postion+1]*256+dataSource[postion]));	
	if((dataSource[postion+1]&0x80)==0x00)
	{
		if(TmpFloat!=0X2710)
		{
			Ac2000Data[ADDR_3G_1812] =  TmpFloat;//满鸡舍温度
		}
		else
		{
			Ac2000Data[ADDR_3G_1812]=9999;
		}
	}
	else
	{
		TmpInt = 0xffff-(dataSource[postion+1]*256+dataSource[postion]);
		TmpInt++;
		TmpInt  = TmpInt;
		Ac2000Data[ADDR_3G_1812] = TmpInt  = 0xffff-TmpInt;
	}
	//rt_kprintf("满鸡舍温度=%d\r\n",Ac2000Data[ADDR_1812]);
	

	
	postion=24;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	if(TmpInt!=0xFFFF)
	{
		Ac2000Data[ADDR_3G_1813] = TmpInt;//CO2浓度
	}
	else
	{
		Ac2000Data[ADDR_3G_1813]=9999;//CO2浓度未启用
	}
	//rt_kprintf("CO2浓度=%d\r\n",Ac2000Data[ADDR_1813]);	
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	if(TmpInt!=0xFF9C)
	{
		Ac2000Data[ADDR_3G_1814] = TmpInt/100;//湿度1
	}
	else
	{
		Ac2000Data[ADDR_3G_1814]=9999;//湿度1未启用
	}
	//rt_kprintf("湿度1=%d\r\n",Ac2000Data[ADDR_1814]);	
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	if(TmpInt!=0xFF9C)
	{
		Ac2000Data[ADDR_3G_1815] = TmpInt/100;//湿度2
	}
	else
	{
		Ac2000Data[ADDR_3G_1815]=9999;//湿度2未启用
	}
	//rt_kprintf("湿度2=%d\r\n",Ac2000Data[ADDR_1815]);	
	
	postion=postion+2;
	TmpInt=(dataSource[postion+1]*256+dataSource[postion]);
	if(TmpInt!=0xFF9C)
	{
		Ac2000Data[ADDR_3G_1816] = TmpInt/100;//外部湿度
	}
	else
	{
		Ac2000Data[ADDR_3G_1816]=9999;//外部湿度未启用
	}
	//rt_kprintf("外部湿度=%d\r\n",Ac2000Data[ADDR_1815]);	
	
}


void temperature_curve1_2(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 02 1d 01 35 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	01 00 56 55     d5 41 c7 71 26.7    cc 41 8e e3 25.6   00 42 72 1c 32.2    bf 41 00 00 23.9   0c 42 00 00  35.0   c0 41 00 00   c0 41
	日              目标            加热            隧道            低的报警       高的报警
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 98
	*/
	unsigned int postion;
	int	TmpInt;
	float	TmpFloat;
	//long Tmplong;
	char tmp[8];
    static uint8_t i=0;
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	
	
	postion=36;
	for(uint8_t n=0;n<7;n++)
	{
		TmpInt=dataSource[postion+1]*256+dataSource[postion];//日
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(AliyunServiceData,tmp);
		
		for(uint8_t m=0;m<5;m++)
		{
			postion=postion+4;
			TmpFloat=hexToFloatInverse(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);//转float类型的
			strcat(AliyunServiceData,tmp);
		}
		postion=postion+6;
		//rt_kprintf("AliyunServiceData:%s\n",AliyunServiceData);
	}
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	if(i%2==0) // 2
	{
		rt_memset(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve1,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve1));
		strcat(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve1,AliyunServiceData);
	}
	else // 0 1
	{
		rt_memset(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve2,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve2));
		strcat(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve2,AliyunServiceData);
	}
	i++;
	
	/*
	231byte
01 14 02 1d 01 35 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
01 00 56 55 d5 41 c7 71 cc 41 8e e3 00 42 72 1c bf 41 00 00 0c 42 00 00 c0 41 00 00 c0 41 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 98 

231byte
01 14 02 1d 01 35
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00


*/
}
void temperature_curve3(unsigned char *dataSource,uint16_t len)
{
	/*
	39 byte
	01 14 02 1d 01 35 
	00 00 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  */
	unsigned int postion;
	int	TmpInt;
	float	TmpFloat;
	//long Tmplong;
	char tmp[8];
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	
	postion=8;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//日
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	for(uint8_t m=0;m<5;m++)
	{
		postion=postion+4;
		TmpFloat=hexToFloatInverse(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(AliyunServiceData,tmp);
	}
	
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve3,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve3));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve3,AliyunServiceData);

	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=TEMPERATURE_CURVE_CMD;
	}
	
}

void humidity_treatm_function(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 22 01 39 
	00 00 00 00 00 00 00 00 
	
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 00 00 00 00 00 00 00 
	00 
  */
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	
	postion=14;
	for(uint8_t n=0;n<20;n++)
	{
		for(uint8_t m=0;m<4;m++)
		{
			TmpInt=dataSource[postion+1]*256+dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(AliyunServiceData,tmp);
			postion=postion+2;
		}
	}
	
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.humidity_treatm,0,sizeof(Ac20003GParams->Ac2000Control.humidity_treatm));
	strcat(Ac20003GParams->Ac2000Control.humidity_treatm,AliyunServiceData);
	
	rt_kprintf("DATA:%s\n",Ac20003GParams->Ac2000Control.humidity_treatm);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=HUMIDITY_TREATM_CMD;
		
	}
}
void Min_max_level(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 1F 01 36 
	00 00 00 00 00 00 
	
	01 00 01 14 1E 00 
	02 00 00 13 1D 00 
	03 00 00 12 1C 00 
	04 00 00 11 1B 00 
	05 00 00 10 1A 00 
	06 00 00 0F 19 00 
	07 00 00 0E 18 00 
	08 00 00 0D 17 00 
	09 00 00 0C 16 00 
	0A 00 00 0B 15 00 
	
	D2
	*/
	unsigned int postion;
	int	TmpInt;
//	float	TmpFloat;
	//long Tmplong;
	char tmp[8];
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	postion=12;
	for(uint8_t m=0;m<10;m++)
	{
		TmpInt=dataSource[postion+1]*256+dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(AliyunServiceData,tmp);
		postion=postion+3;
		
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(AliyunServiceData,tmp);
		postion=postion+1;
		
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(AliyunServiceData,tmp);
		postion=postion+2;
	}

	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.min_max_level,0,sizeof(Ac20003GParams->Ac2000Control.min_max_level));
	strcat(Ac20003GParams->Ac2000Control.min_max_level,AliyunServiceData);
	//rt_kprintf("DATA:%s\n",Ac20003GParams->Ac2000Control.min_max_level);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=MIN_MAX_LEVEL_CMD;
	}
}
void static_pressure_function(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 2A 01 41 00 00 
	40 41 00 00 E8 41 00 00 98 41 00 00 A0 41 00 00 A0 41 00 00 A0 40 00 00 C0 40 00 00 1C 42 00 00 20 42 00 00 20 41 00 00 10 41 00 00 00 41 
	D8 01 14 00 02 01 18 1F 05 02 00 70 03 70 04 70 05 70 06 F8
	*/
	unsigned int postion;
	//int	TmpInt;
	float	TmpFloat;
	char tmp[8];
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	
	postion=12;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//最小通风-舍外 在低温下的静压
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=20;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//最小通风-舍外 在高温下的静压
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=28;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//最小通风-舍外  低静压报警
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=36;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//最小通风-舍外  高静压报警
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=44;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//最小通风-舍外 静压带
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=16;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//隧道 目标静压
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=32;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//隧道  低静压报警
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=40;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//隧道 高静压报警
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=48;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//隧道 静压带
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=8;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//屋顶 目标静压
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
	
	postion=52;
	TmpFloat=hexToFloatInverse(&dataSource[postion]);//屋顶 静压带
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(AliyunServiceData,tmp);
		
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.static_pressure,0,sizeof(Ac20003GParams->Ac2000Control.static_pressure));
	strcat(Ac20003GParams->Ac2000Control.static_pressure,AliyunServiceData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=STATIC_PRESSURE_CMD;
	}
}
void Control_mode(unsigned char *dataSource,uint16_t len)
{
/*
01 14 02 2B 01 43 
00 01 00 00 01 00 01 01 01 
05
*/
	unsigned int postion;
	int	TmpInt;
//	float	TmpFloat;
	char tmp[8];
	char AliyunServiceData[256];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));
	
	postion=10;
	TmpInt=dataSource[postion];//鸡舍模式
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);

	postion=13;
	TmpInt=dataSource[postion];//空鸡舍模式
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	postion=7;
	TmpInt=dataSource[postion];//温度曲线开启或关闭
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	postion=11;
	TmpInt=dataSource[postion];//最低最高级别控制
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	postion=14;
	TmpInt=dataSource[postion];//Heat cycle
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	postion=12;
	TmpInt=dataSource[postion];//Analog heat mode
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(AliyunServiceData,tmp);
	
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.control_mode,0,sizeof(Ac20003GParams->Ac2000Control.control_mode));
	strcat(Ac20003GParams->Ac2000Control.control_mode,AliyunServiceData);
	
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=CONTROL_MODE_CMD;
	}	
}






void Co2_Handle(unsigned char *dataSource,uint16_t len)
{
/*
01 14 01 92 01 A9 
00 00 64 00 00 00 00 00 00 00
	
01 00 88 13 F4 01 0F 27 0F 27 
02 00 24 13 EA 01 0E 27 B8 22 
03 00 C0 12 E0 01 0D 27 B7 22 
04 00 5C 12 D6 01 0C 27 B6 22 
05 00 F8 11 CC 01 0B 27 B5 22 
06 00 94 11 C2 01 0A 27 B4 22 
07 00 30 11 B8 01 09 27 B3 22 
08 00 CC 10 AE 01 08 27 B2 22 
09 00 68 10 A4 01 07 27 B1 22 
0A 00 04 10 9A 01 06 27 B0 22 
0B 00 A0 0F 90 01 05 27 AF 22 
0C 00 3C 0F 86 01 04 27 AE 22 
0D 00 D8 0E 7C 01 03 27 AC 22 
0E 00 74 0E 72 01 02 27 AB 22 
0F 00 10 0E 68 01 01 27 AA 22 
10 00 AC 0D 5E 01 00 27 A9 22 
11 00 48 0D 54 01 FF 26 A8 22 
12 00 E4 0C 4A 01 FE 26 A7 22 
13 00 80 0C 40 01 FD 26 A6 22 
E7 03 1C 0C 36 01 FC 26 A5 22 
86

*/	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	char AliyunServiceData[512];
	memset(AliyunServiceData,0,sizeof(AliyunServiceData));

	postion=16;
	for(uint8_t n=0;n<20;n++)
	{
		for(uint8_t m=0;m<5;m++)
		{
			TmpInt=dataSource[postion+1]*256+dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(AliyunServiceData,tmp);
			postion=postion+2;
		}
	}
	AliyunServiceData[strlen(AliyunServiceData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.CO2_Handle,0,sizeof(Ac20003GParams->Ac2000Control.CO2_Handle));
	strcat(Ac20003GParams->Ac2000Control.CO2_Handle,AliyunServiceData);
	rt_kprintf("CO2_Handle:%s\n",AliyunServiceData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=CO2_HANDLE_CMD;
	}
}




void system_inforamation_tmperature(unsigned char *dataSource,uint16_t len)//温度曲线
{
/*
0	 01 14 01 65 01 7c 04 
7	 02                         最小通风低于加热温度的值:平均（02） （无）00   区域（01）
8	 0f                         降温系数（%）
	 00                         
10 9a 99 99 3f    1.2         温度曲线修正值
14 9a 99 d9 3f    1.700000    设置温度改变提醒（差值）
18 9a 99 19 3f    0.600000    目标温度带
22 9a 99 19 3f    0.600000    加热器温度带
26 9a 99 d9 3f    1.700000    快速降温限制（度））
30 00 00 5c c2    -55.000000  非养殖区与加热温度的差：
ea 
*/
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	
	
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
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
	
	postion=22;
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	postion=8;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	postion=26;
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);

	postion=7;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=30;
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_tmperature_curvel,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_tmperature_curvel));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_tmperature_curvel,SysData);
	
	rt_kprintf("系统参数-温度曲线:%s\n",SysData);
	rt_free(SysData);
}

void System_inforamation_cycle_heater(unsigned char *dataSource,uint16_t len)//CYCLE HEATER
{
/*
01 14 01 62 01 79 
33 33 33 3f        Low Heat Differential
cd cc 8c 40        High Heat Differential
e7 03              Minimum ON Time
00 00        
27 
*/
	
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;

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
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_cycle_heater,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_cycle_heater));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_cycle_heater,SysData);
	rt_kprintf("系统参数-CYCLE HEATER:%s\n",SysData);
	rt_free(SysData);
}


void System_inforamation_radiant_heater(unsigned char *dataSource,uint16_t len)//辐射加热器
{
/*
01 14 41 66 01 bd 
1e 00              Minimum ON Time
66 66 a6 3f        低档辐射加热器-与加热温度的差值
9a 99 19 3f        高档辐射加热器-差值（低于低辐射的设定值）
5a 
*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	
	postion=8;
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	postion=12;
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
			
	postion=6;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_radiant_heater,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_radiant_heater));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_radiant_heater,SysData);
	rt_kprintf("系统参数-辐射加热:%s\n",SysData);
	rt_free(SysData);
}

void System_inforamation_variable_heater(unsigned char *dataSource,uint16_t len)//VARIABLE HEATER
{
/*
01 14 03 a6 01 bf 
9a 99 19 3f       0.6 //Low Diff. Below Heat Temperature
cd cc 8c 40       4.4 //High Diff. Below Heat Temperature
e4 38 0e 3f e4 38 8e 40 0f 00 52 
*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	char tmp[8];
	float	TmpFloat;
	
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
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_variable_heater,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_variable_heater));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_variable_heater,SysData);	
	rt_kprintf("系统参数-variable_heater:%s\n",SysData);
	rt_free(SysData);
}

void System_inforamation_humidity_treatment(unsigned char *dataSource,uint16_t len)//湿度处理
{
 /*
	01 14 02 6c 01 84 
	05     湿度带（%）
	01     低于加热温度时是否湿度处理  00-	否 01-	是
	01 00  HUMIDITY TREATMENT BY       03 05—通道风扇  01 00---级别   00 00---无
	ff ff  通道风扇数字号倒序转十进制
	00 05 
	*/
	
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	//float	TmpFloat;
	
	postion=6;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=7;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
			
	postion=8;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_humidity_treatment,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_humidity_treatment));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_humidity_treatment,SysData);	
	rt_kprintf("系统参数-humidity_treatment:%s\n",SysData);
	rt_free(SysData);
	
}
void System_inforamation_C02_treatment(unsigned char *dataSource,uint16_t len)//CO2处理
{
 /*
	01 14 02 86 01 9E 
	
	00           00-	否 01-	是
	01 00        03 05—通道风扇  01 00---级别  00 00---无
	00 00        通道风扇号数 倒序转十进制

	00 01 
	*/
	
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	//float	TmpFloat;
	
	postion=6;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=7;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
			
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_C02_treatment,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_C02_treatment));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_C02_treatment,SysData);	
	rt_kprintf("系统参数-CO2处理:%s\n",SysData);
	rt_free(SysData);
	
}
void System_inforamation_static_pressure(unsigned char *dataSource,uint16_t len)//静压
{
	/*
	01 14 02 69 01 81 
	66 66 5E C1   低温-进气（与目标的差）
	33 33 B3 C0   高温进气（与目标的差）
	00 FF FF 00 
	3C 00         紧急静压延迟（秒）
	00            最小通风使用 00-小窗  01-隧道幕帘
	1E 1E 
	0A            阵风延迟时间（秒）
	01            隧道通风下是否负压控制 00-否 01-是
	01            隧道过渡模式 00-否 01-是
	64            紧急静压下的卷帘位置% 
	01            低静压报警-最低级别（最小通风）
	02            低静压报警-最低级别（隧道）
	0A            建立静压时间（秒）
	03            屋顶通风提前打开时间（秒）
	00 BA
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	
	postion=23;//阵风延迟时间（秒）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

  //隧道通风下是否负压控制 00-否 01-是
	postion=24;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	//隧道过渡模式 00-否 01-是
	postion=25;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=20;
	TmpInt=dataSource[postion];//最小通风使用 00-小窗  01-隧道幕帘
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=6;
	TmpFloat=FourByteToFloat(&dataSource[postion]);//低温-进气（与目标的差）
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
		
	postion=10;
	TmpFloat=FourByteToFloat(&dataSource[postion]);//高温进气（与目标的差）
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	postion=18;
	TmpInt=dataSource[postion+1]*256+dataSource[postion];//紧急静压延迟（秒）
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

//紧急静压下的卷帘位置% 
	postion=26;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
//低静压报警-最低级别（最小通风）
	postion=27;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
//低静压报警-最低级别（隧道）
	postion=28;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
//建立静压时间（秒）
	postion=29;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
//屋顶通风提前打开时间（秒）
	postion=23;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_static_pressure,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_static_pressure));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_static_pressure,SysData);	
	rt_kprintf("系统参数-静压:%s\n",SysData);
	rt_free(SysData);
}
void System_inforamation_ventilation_level(unsigned char *dataSource,uint16_t len)//通风级别
{
	/*
	01 14 01 67 01 7E 
	1E              最高通风级别
	0A 							起始隧道级别
	14 							最大风机数，转换至隧道模式的值
	00 							                                     不确定是倒序还是顺序
	B4 00 					提高级别的延迟时间（秒）
	3C 00 					降低级别的延迟时间
	14 00 					风机循环时间（秒，0-手动）
	06 00 					隧道退出延迟
	
	CD CC 8C 3F 		隧道模式退出-低于隧道温度的温度差
	00 00 A0 40 		隧道退出-舍外温度高于隧道的差值
	8A
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	
	postion=6;
	for(uint8_t i=0;i<3;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion++;
	}

	postion=18;
	for(uint8_t i=0;i<2;i++)
	{
		TmpFloat=FourByteToFloat(&dataSource[postion]);;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=postion+4;
	}
	
	postion=10;
	for(uint8_t i=0;i<4;i++)
	{
		TmpInt=dataSource[postion+1]*256+dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion=postion+2;
	}
	

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_ventilation_level,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_ventilation_level));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_ventilation_level,SysData);	
	rt_kprintf("系统参数-通风级别:%s\n",SysData);
	rt_free(SysData);
}
void System_inforamation_rolling_shutter_and_roof(unsigned char *dataSource,uint16_t len)//卷帘和屋顶
{
	/*
	01 14 03 6e 01 87 e4 38 8e 3e 00 01 05 0a 00 9a 99 d9 3f 33 33 17 42 00 ff ff 00 01 00 1c 00 1e 00 00 00 00 00 14 05 01 00 01 56 
  01 14 03 6e 01 87 e4 38 8e 3e 00 01 05 0a 00 9a 99 d9 3f 33 33 17 42 00 ff ff 00 02 00 1c 00 1e 00 00 00 00 00 14 05 01 00 03 59 
	01 14 03 6e 01 87 e4 38 8e 3e 00 01 05 0a 00 9a 99 d9 3f 33 33 17 42 00 ff ff 00 02 00 0a 00 1e 00 00 00 00 00 14 05 01 01 03 48 

0		01 14 03 6e 01 87 e4 38 8e 3e 00 01 05 0a 00 
15	9a 99 d9 3f 运行的最小屋顶温度
19	33 33 17 42 关闭屋顶的最高温度
23	00 ff ff 00 
27	02 00       第二个小窗的运行起始日 
29	1c 00       停止风机，卷帘移动（合计%）
		1e 00 
33	00 00       运行开始
35	00 00       运行结束
37	14 05 01 
40	00          屋顶运行直至级别
41	03          第二小窗运行的起始级别
		59 
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	int Hour;
	int Minute;
	postion=29;//停止风机，卷帘移动（合计%）
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=27;//  第二个小窗的运行起始日
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=41;//第二小窗运行的起始级别
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=15;//运行的最小屋顶温度 
	TmpFloat=FourByteToFloat(&dataSource[postion]);;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	postion=40;//屋顶运行直至级别 
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	postion=33;//运行开始 运行结束
	for(uint8_t i=0;i<2;i++)
	{
		TmpInt=dataSource[postion+1]*256+dataSource[postion];
		memset(tmp,0,sizeof(tmp));
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
	}
	
	postion=19;//关闭屋顶的最高温度
	TmpFloat=FourByteToFloat(&dataSource[postion]);;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	postion=postion+2;
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_rolling_shutter_and_roof,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_rolling_shutter_and_roof));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_rolling_shutter_and_roof,SysData);	
	
	rt_kprintf("系统参数 卷帘和屋顶：%s\n",SysData);
	rt_free(SysData);
}
void System_inforamation_curtain_calibration(unsigned char *dataSource,uint16_t len)//VENT CURTAIN CALIBRATION
{
	/*
	01 14 01 F9 01 10   
	00 00 C0 40  低于这个温度关闭（外）浮点计算
	0A 00        从时间 
	50 00        至时间
	05           步骤数量
	01 					 电源开启通风校准 00—否 01—是
	0A           接近界限 %
	6A
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	int Hour;
	int Minute;
	postion=10;//From Time
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Hour = TmpInt/60;
	Minute = TmpInt%60; 
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d",Hour);
  strcat(SysData,tmp);
	strcat(SysData,":");
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d,",Minute);
	strcat(SysData,tmp);

	postion=12;//To Time 
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Hour = TmpInt/60;
	Minute = TmpInt%60; 
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d",Hour);
	strcat(SysData,tmp);
  strcat(SysData,":");
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d,",Minute);
	strcat(SysData,tmp);
	
	postion=14;//Number Of Steps
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=15;//Power Up Vents Calibration
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=16;//Proximity To Edge %
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	postion=6;//Close Below This Temp. (Out)
	TmpFloat=FourByteToFloat(&dataSource[postion]);;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_curtain_calibration,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_curtain_calibration));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_curtain_calibration,SysData);
	
	rt_kprintf("系统参数-VENT CURTAIN CALIBRATION:%s\n",SysData);
	rt_free(SysData);	
}
void System_inforamation_wet_curtain_cool_pad_flush(unsigned char *dataSource,uint16_t len)//湿帘 COOL PAD FLUSH
{
	/*
	01 14 02 6a 01 82 
	00            湿帘冲洗持续时间（分钟）
	01            Cool Pad-1 Minimum Level To Operate
	01 						Cool Pad-2 Minimum Level To Operate
	01 						Cool Pad-3 Minimum Level To Operate
	01 						Cool Pad-4 Minimum Level To Operate
	00 00         冲洗湿帘的时间：
	00 
	cd cc 8c 3f   温度带
	00 00 80 40   湿度带（%）
	00 00 00 00   Diff Between Cool Pads Stage
	28  
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	int Hour;
	int Minute;
	postion=14;//温度带 湿度带（%）  Diff Between Cool Pads Stage
	for(uint8_t i=0;i<3;i++)
	{
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=postion+4;
	}

	postion=7;//Cool Pad-1 Minimum Level To Operate
	for(uint8_t i=0;i<4;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion++;
	}
	
	postion=12;//冲洗湿帘的时间：
	TmpInt=dataSource[postion+1]*256+dataSource[postion];
	Hour = TmpInt/60;
	Minute = TmpInt%60; 
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d",Hour);
	strcat(SysData,tmp);
	strcat(SysData,":");
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%02d,",Minute);
	strcat(SysData,tmp);
	
	

	postion=6;//湿帘冲洗持续时间（分钟）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_wet_curtain_cool_pad_flush,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_wet_curtain_cool_pad_flush));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_wet_curtain_cool_pad_flush,SysData);
	rt_kprintf("系统参数-湿帘 COOL PAD FLUSH:%s\n",SysData);
	rt_free(SysData);	
}

void System_inforamation_Humidifier(unsigned char *dataSource,uint16_t len)//加湿器
{
	/*
	01 14 02 6B 01 83 
	CD CC 8C 3F      温度带
	00 00 A0 40      湿度带（%）
	CD CC 0C 40      Diff between foggers stage
	06               启动运行的最低级别
	1E   						 启动运行的最高级别
	00               Enable rotation 00—否 01—是
	00 4D
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	
	postion=6;//温度带  湿度带（%）
	for(uint8_t i=0;i<2;i++)
	{
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion=postion+4;
	}
	
	postion=18;//启动运行的最低级别
	for(uint8_t i=0;i<3;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion++;
	}
	
	postion=14;//Diff between foggers stage
	TmpFloat=FourByteToFloat(&dataSource[postion]);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);


	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_Humidifier,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_Humidifier));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_Humidifier,SysData);
	rt_kprintf("系统参数 加湿器：%s\n",SysData);
	rt_free(SysData);		
}
void System_inforamation_lighting(unsigned char *dataSource,uint16_t len)//灯光
{
	/*
	01 14 01 6F 01 86 
	0A    日出时间（分钟）
	14    日落时间（分钟）
	32    峰值增加量（%）
	01    信号灯是 00—none 01—灯光1  02—灯光2  03—灯光3  04—灯光4
	01    灯光传感器激活 00—否 01—是
	00 
	9A 02 喂料前的信号（秒）
	2B 02 喂料时的信号（秒）
	E7 03 允许某天的峰值 
	78 03 峰值循环（分钟）
	09 03 峰值持续时间（分钟
	8C
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	
	postion=6;//日出时间（分钟）
	for(uint8_t i=0;i<2;i++)
	{
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion++;
	}
	postion=16;//允许某天的峰值 
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=18;//峰值循环（分钟）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=20;//峰值持续时间（分钟
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);	
	
	postion=8;//峰值增加量（%）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=9;//信号灯是 00—none 01—灯光1  02—灯光2  03—灯光3  04—灯光4;/
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=12;//喂料前的信号（秒）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=14;//喂料时的信号（秒）
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
		
	postion=10;//灯光传感器激活 00—否 01—是
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_lighting,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_lighting));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_lighting,SysData);
	rt_kprintf("系统参数-灯光:%s\n",SysData);
	rt_free(SysData);	
	
}
void system_inforamation_food_water(unsigned char *dataSource,uint16_t len)//水和饲料
{

	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	
	postion=6;
	TmpInt=dataSource[postion]+1;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_water,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_water));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_water,SysData);
	rt_kprintf("系统参数-灯光:%s\n",SysData);
	rt_free(SysData);	

}

void system_inforamation_food_left(unsigned char *dataSource,uint16_t len)//饲料存量
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[8];

	float TmpFloat;
	postion = 12;
	TmpFloat=(FourByteToFloat(&dataSource[postion])*2.203f);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	postion =22;
	TmpFloat=(FourByteToFloat(&dataSource[postion])*2.203f);
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%0.1f,",TmpFloat);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_left,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_left));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_left,SysData);
	rt_kprintf("系统参数-灯光:%s\n",SysData);
	rt_free(SysData);	
	
	

}
void system_inforamation_food_leftli(unsigned char *dataSource,uint16_t len)
{
			//饲料余量中的料斗区别
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		postion =6; 
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
    if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_leftlog,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_leftlog));
		strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_leftlog,SysData);
		rt_kprintf("系统参数-灯光:%s\n",SysData);
		rt_free(SysData);	

}

void System_inforamation_alarm_set(unsigned char *dataSource,uint16_t len)//报警设置
{
	/*
	01 14 01 72 01 89 
	01 
	01  
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];

	postion=6;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_alarm_set,0,sizeof(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_alarm_set));
	strcat(Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_alarm_set,SysData);
	rt_kprintf("系统参数-报警设置:%s\n",SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=SYSTEM_INFORAMATION_CMD;
	}
	rt_free(SysData);
}



/************************设备********************************/

unsigned char *LevelsOfVentilationData=NULL;
uint16_t LevelsOfVentilationDataLength=0;



void Write_data_Empty(void)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	rt_memset(SysData,0,AliyunServiceSysLength*12);
	char tmp[8];
	if(AirxxxStatus.aliyun_service_param2==DEVICE_VARIABLE_SPEED_FAN_LEVELS_CMD)//变频风机数据为空
	{
		for(unsigned char i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(unsigned char j=0;j<40;j++)
			{
				strcat(SysData,",");
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.variable_speed_fan_levels,0,sizeof(Ac20003GParams->Ac2000Device.variable_speed_fan_levels));
		strcat(Ac20003GParams->Ac2000Device.variable_speed_fan_levels,SysData);	
		rt_free(SysData);
		//rt_kprintf("device variable_speed_fan_levels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_VARIABLE_SPEED_FAN_LEVELS_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==DEVICE_VENT_AND_CURTIAN_LEVELS_CMD)//小窗卷帘级别
	{
		for(unsigned char i=0;i<30;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(unsigned char j=0;j<5;j++)
			{
				strcat(SysData,",");
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels,0,sizeof(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels));
		strcat(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_VentAndCurtianLevels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_VENT_AND_CURTIAN_LEVELS_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==DEVICE_STIR_FAN_LEVELS_CMD)//搅拌风机级别
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
		rt_memset(Ac20003GParams->Ac2000Device.stir_fan_levels,0,sizeof(Ac20003GParams->Ac2000Device.stir_fan_levels));
		strcat(Ac20003GParams->Ac2000Device.stir_fan_levels,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_StirFanLevels:%s\n",Ac20003GParams->Ac2000Device.stir_fan_levels);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_STIR_FAN_LEVELS_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==INSTALL_VENTCURTAINSETUP_CMD)
	{
		for(unsigned char i=0;i<24;i++)
		{
			strcat(SysData,",");
		
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.VentCurtainSetUp,0,sizeof(Ac20003GParams->Ac2000Install.VentCurtainSetUp));
		strcat(Ac20003GParams->Ac2000Install.VentCurtainSetUp,SysData);
		//rt_kprintf("device Ac2000SInstall.VentCurtainSetUp:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_VENTCURTAINSETUP_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==INSTALL_CAPACITYOFFANAIR_CMD)//风机空气容量
	{
		for(unsigned char i=0;i<60;i++)
		{
			strcat(SysData,",");
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.CapacityOfFanAir,0,sizeof(Ac20003GParams->Ac2000Install.CapacityOfFanAir));
		strcat(Ac20003GParams->Ac2000Install.CapacityOfFanAir,SysData);
		rt_kprintf("device Ac2000SInstall.CapacityOfFanAir:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_CAPACITYOFFANAIR_CMD;
		}
	}
	else if(AirxxxStatus.aliyun_service_param2==INSTALL_TEMPERATUREDEFINE_CMD)//温度定义
	{
		for(unsigned char i=0;i<70;i++)
		{
			strcat(SysData,",");
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.TemperatureDefine,0,sizeof(Ac20003GParams->Ac2000Install.TemperatureDefine));
		strcat(Ac20003GParams->Ac2000Install.TemperatureDefine,SysData);
		//rt_kprintf("device Ac2000SInstall.TemperatureDefine:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_TEMPERATUREDEFINE_CMD;
		}	
	
	}

}
	

static void Device_LevelsOfVentilationLast(unsigned char *dataSource,uint16_t len)
{

	
	
	if(len==261)
	{
		rt_kprintf("LevelsOfVentilationDataLength:%d\n",len);
	if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*20);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*19)
	{
		rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
		LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
	}
	else
	{
		rt_memset(LevelsOfVentilationData,0,256*20);
		LevelsOfVentilationDataLength=0;
	}
	if(FullDataState==1)
	{
		uint8_t tunnel=0;
		uint8_t Exhaust=0;
		uint8_t k=0;
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		uint32_t TfanStatus[20];
		uint32_t EfanStatus[20];
		unsigned char T[20];
		unsigned char E[20];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*20);
	  rt_memset(SysData,0,AliyunServiceSysLength*20);
		

		rt_kprintf("LevelsOfVentilationData:%s\n",LevelsOfVentilationData);
	
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
				
				TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				//rt_kprintf("差值:%f  ",TmpFloat);
				
				postion=postion+4;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				//rt_kprintf("开启:%d  ",TmpInt);
			
				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				//rt_kprintf("关闭:%d\n",TmpInt);
				
				postion=postion+2;
				memset(TfanStatus,0,sizeof(uint32_t)*20);
				memset(EfanStatus,0,sizeof(uint32_t)*20);	
				memset(T,0,sizeof(T));
				memset(E,0,sizeof(E));
				for(uint8_t n=0;n<40;n++)
				{
					if(LevelsOfVentilationData[postion]==5||LevelsOfVentilationData[postion]==6)//5:隧道风机 6:排气风机
					{
						switch(LevelsOfVentilationData[postion])//5:隧道风机 6:排气风机
						{
							case 5:
							{
								 k=LevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 T[tunnel]=k;
								 TfanStatus[tunnel]=LevelsOfVentilationData[postion+4];
								postion=postion+6;
								tunnel+=1;
							}break;
							case 6:
							{
								 k=LevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 E[Exhaust]=k;
								EfanStatus[Exhaust]=LevelsOfVentilationData[postion+4];
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
					for(unsigned char s=0;s<20;s++)
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
					for(unsigned char s=T[tunnel-1] ;s<19;s++)
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
						for(unsigned char s=T[tunnel-1];s<19;s++)
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
			//}
		}


		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.levels_of_ventilation,0,sizeof(Ac20003GParams->Ac2000Device.levels_of_ventilation));
		strcat(Ac20003GParams->Ac2000Device.levels_of_ventilation,SysData);
		rt_kprintf("Device- LevelOfVentilation:%s\n",Ac20003GParams->Ac2000Device.levels_of_ventilation);
		
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_LEVELS_OF_VENTILATION_CMD;
		}
			
		FullDataState=0;
		
	}
	
	
	}
	else if(len>6)
	 {
		rt_kprintf("LevelsOfVentilationDataLength:%d\n",len);
		if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*20);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*19)
	{
		rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
		LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
	}
	else
	{
		rt_memset(LevelsOfVentilationData,0,256*20);
		LevelsOfVentilationDataLength=0;
	}

		uint8_t tunnel=0;
		uint8_t Exhaust=0;
		uint8_t k=0;
		uint16_t postion;
		int	TmpInt;
		float TmpFloat;
		char tmp[10];
		uint32_t TfanStatus[20];
		uint32_t EfanStatus[20];
		unsigned char T[20];
		unsigned char E[20];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*20);
	  rt_memset(SysData,0,AliyunServiceSysLength*20);
		

		rt_kprintf("LevelsOfVentilationData:%s\n",LevelsOfVentilationData);
	
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
				
				TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				//rt_kprintf("差值:%f  ",TmpFloat);
				
				postion=postion+4;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				//rt_kprintf("开启:%d  ",TmpInt);
			
				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				//rt_kprintf("关闭:%d\n",TmpInt);
				
				postion=postion+2;
				memset(TfanStatus,0,sizeof(uint32_t)*20);
				memset(EfanStatus,0,sizeof(uint32_t)*20);	
				memset(T,0,sizeof(T));
				memset(E,0,sizeof(E));
				for(uint8_t n=0;n<40;n++)
				{
					if(LevelsOfVentilationData[postion]==5||LevelsOfVentilationData[postion]==6)//5:隧道风机 6:排气风机
					{
						switch(LevelsOfVentilationData[postion])//5:隧道风机 6:排气风机
						{
							case 5:
							{
								 k=LevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 T[tunnel]=k;
								 TfanStatus[tunnel]=LevelsOfVentilationData[postion+4];
								postion=postion+6;
								tunnel+=1;
							}break;
							case 6:
							{
								 k=LevelsOfVentilationData[postion+1];//风机编号 实际需加1
								 E[Exhaust]=k;
								EfanStatus[Exhaust]=LevelsOfVentilationData[postion+4];
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
					for(unsigned char s=0;s<20;s++)
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
					for(unsigned char s=T[tunnel-1] ;s<19;s++)
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
						for(unsigned char s=T[tunnel-1];s<19;s++)
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
			//}
		}


		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.levels_of_ventilation,0,sizeof(Ac20003GParams->Ac2000Device.levels_of_ventilation));
		strcat(Ac20003GParams->Ac2000Device.levels_of_ventilation,SysData);
		rt_kprintf("Device- LevelOfVentilation:%s\n",Ac20003GParams->Ac2000Device.levels_of_ventilation);
		
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_LEVELS_OF_VENTILATION_CMD;
		}
		
		Ac20003GParams->Ac2000Device.LevelsOfVentilationCmdCount=0;
	}
//	rt_kprintf("LevelsOfVentilationDataLength: err%d\n",len);
}


static void Device_VariableSpeedFanLevels(unsigned char *dataSource,uint16_t len)
{


		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[10];
		unsigned char Aisle1[20][30];
		unsigned char Roof1[20][30];
		unsigned char A[20];
		unsigned char R[20];
		unsigned char a=0;
		unsigned char b=0;
		postion=6;
		for(uint8_t i=0;i<2;i++)
		{
			if(dataSource[postion]==0x05)//通道风扇
				{
					A[a]=dataSource[postion+1];//存储风机编号
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[dataSource[postion+1]][j]=dataSource[postion+4+(j*2)];//存入多维数组中
					}
					a=a+1;//在的风机个数
				}
				else if(dataSource[postion]==0x06)//排气风扇
				{
					R[b]=dataSource[postion+1];//保存风帘编号,判断差距
					for(unsigned char z=0;z<30;z++)
					{
						Roof1[dataSource[postion+1]][z]=dataSource[postion+4+(z*2)];
					}
					b=b+1;
				}
				postion =postion+64;
		}
		
	rt_kprintf("aaa%d,bbb%d",a,b);	
		
	if(a==0)//说明一组数据都没有
			{
				for(unsigned char i=0;i<20;i++)
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

					for(unsigned char u=A[0]+1;u<20;u++)
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
				for(unsigned char u=A[a-1]+1;u<20;u++)
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
				for(unsigned char j=0;j<20;j++)
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
		rt_memset(Ac20003GParams->Ac2000Device.variable_speed_fan_levels,0,sizeof(Ac20003GParams->Ac2000Device.variable_speed_fan_levels));
		strcat(Ac20003GParams->Ac2000Device.variable_speed_fan_levels,SysData);	
		rt_free(SysData);
		//rt_kprintf("device variable_speed_fan_levels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_VARIABLE_SPEED_FAN_LEVELS_CMD;
		}
		

}

static void Device_VentAndCurtianLevels(unsigned char *dataSource,uint16_t len)//通风和风帘级别
{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned char Aisle1[30];
		unsigned char Roof1[4][30];

		unsigned char R[4];
		unsigned char a=0;
		unsigned char b=0;
		unsigned char s=len/34;
		postion =6;
		for(unsigned char i=0;i<s;i++)
		{
			if(dataSource[postion]==0x0B)
			{
		
				for(unsigned char j=0;j<30;j++)
				{
					Aisle1[j]=dataSource[postion+4+j];
				}
				a=a+1;
			}
			else if(dataSource[postion]==0x0D)
			{
				R[b]=dataSource[postion+1];
				for(unsigned char z=0;z<30;z++)
				{
					Roof1[dataSource[postion+1]][z]=dataSource[postion+4+z];
				}
				b=b+1;
			}
				postion =postion+34;
		}
		
			if(a==0)//说明一组数据都没有
			{
					for(unsigned char j=0;j<30;j++)
					{
						Aisle1[j]=0xff;
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
				
		
				
					TmpInt = Aisle1[i];
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
				
			
			}
			
			
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels,0,sizeof(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels));
		strcat(Ac20003GParams->Ac2000Device.vent_and_curtain_Levels,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_VentAndCurtianLevels:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_VENT_AND_CURTIAN_LEVELS_CMD;
		}
	
}

static void Device_StirFanLevels(unsigned char *dataSource,uint16_t len)//搅扇级别
{
		if(len==245)
		{
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*4);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*4);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*3)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*4);
			LevelsOfVentilationDataLength=0;
		}
		if(FullDataState==1)
		{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*4);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		char tmp[8];
		postion =0;
		unsigned char Aisle1[15][30];
		unsigned char A[15];
	  int TmpInt; 
		unsigned char a=0;
		unsigned char s=LevelsOfVentilationDataLength/34;
		for(unsigned char i=0;i<s;i++)
		{
			if(LevelsOfVentilationData[postion]==0x07)
			{
				A[a]=LevelsOfVentilationData[postion+1];
				for(unsigned char j=0;j<30;j++)
				{
					Aisle1[LevelsOfVentilationData[postion+1]][j]=LevelsOfVentilationData[postion+4+j];
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
		rt_memset(Ac20003GParams->Ac2000Device.stir_fan_levels,0,sizeof(Ac20003GParams->Ac2000Device.stir_fan_levels));
		strcat(Ac20003GParams->Ac2000Device.stir_fan_levels,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_StirFanLevels:%s\n",Ac20003GParams->Ac2000Device.stir_fan_levels);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_STIR_FAN_LEVELS_CMD;
		}
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		FullDataState=0;
		}
 }
	else if(len>6)
	{
		if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*4);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*4);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*3)
	{
		rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
		LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
	}
	else
	{
		rt_memset(LevelsOfVentilationData,0,256*4);
		LevelsOfVentilationDataLength=0;
	}

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*4);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		char tmp[8];
		postion =0;
		unsigned char Aisle1[15][30];
		unsigned char A[15];
	  int TmpInt; 
		unsigned char a=0;
		unsigned char s=LevelsOfVentilationDataLength/34;
		for(unsigned char i=0;i<s;i++)
		{
			if(LevelsOfVentilationData[postion]==0x07)
			{
				A[a]=LevelsOfVentilationData[postion+1];
				for(unsigned char j=0;j<30;j++)
				{
					Aisle1[LevelsOfVentilationData[postion+1]][j]=LevelsOfVentilationData[postion+4+j];
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
		rt_memset(Ac20003GParams->Ac2000Device.stir_fan_levels,0,sizeof(Ac20003GParams->Ac2000Device.stir_fan_levels));
		strcat(Ac20003GParams->Ac2000Device.stir_fan_levels,SysData);	
		rt_free(SysData);
		rt_kprintf("Device_StirFanLevels:%s\n",Ac20003GParams->Ac2000Device.stir_fan_levels);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_STIR_FAN_LEVELS_CMD;
		}
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
	}
}
static void Device_StirFanProgram(unsigned char *dataSource,uint16_t len)
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
	
		/*switch(TmpInt)
		{
			
			case 0x00:sprintf(tmp,"%s","0,0,0,0,0,");strcat(SysData,tmp);break;
			case 0x01:sprintf(tmp,"%s","1,0,0,0,0,");strcat(SysData,tmp);break;
			case 0x02:sprintf(tmp,"%s","0,1,0,0,0,");strcat(SysData,tmp);break;
			case 0x04:sprintf(tmp,"%s","0,0,1,0,0,");strcat(SysData,tmp);break;
			case 0x08:sprintf(tmp,"%s","0,0,0,1,0,");strcat(SysData,tmp);break;
			case 0x10:sprintf(tmp,"%s","0,0,0,0,1,");strcat(SysData,tmp);break;
			default:break;
		}
		
	}
 */
		postion++;
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Device.stir_fan_program,0,sizeof(Ac20003GParams->Ac2000Device.stir_fan_program));
	strcat(Ac20003GParams->Ac2000Device.stir_fan_program,SysData);	
	rt_kprintf("Device_VentAndCurtianLevels:%s\n",SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=DEVICE_STIR_FAN_PROGRAM_CMD;
	}

}
static void Device_Foggers(unsigned char *dataSource,uint16_t len)
{
	if(len==247)
	{
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*12);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("Device_Foggers malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		if(len>6)
		{
			/*
			01 14 01 25 01 3c 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 

			01 14 01 25 01 3c 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
			00 
			*/
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		int Hour;
		int Minute;
		postion=16;	//日期
		for(uint8_t i=0;i<20;i++)
		{
			if(postion<(LevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
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
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
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
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//目标差值
				TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.foggers,0,sizeof(Ac20003GParams->Ac2000Device.foggers));
		strcat(Ac20003GParams->Ac2000Device.foggers,SysData);	
		rt_kprintf("foggers:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_FOGGERS_CMD;
		}
	}
}

static void Device_CoolPad(unsigned char *dataSource,uint16_t len)//加湿器
{
	if(len==261)
	{
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*12);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("Device_CoolPad malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		if(len>6)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		postion = 16;	//日期
		int Hour = 0;
		int Minute = 0;
		for(uint8_t i=0;i<20;i++)
		{
			if(postion<(LevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日期
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开始时间
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
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
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
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
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//开（秒）
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//关（秒）
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//隧道差值
				TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion=postion+4;
			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.cool_pad,0,sizeof(Ac20003GParams->Ac2000Device.cool_pad));
		strcat(Ac20003GParams->Ac2000Device.cool_pad,SysData);	
		//rt_kprintf("cool_pad:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_COOL_PAD_CMD;
		}
	}
}
static void Device_Light(unsigned char *dataSource,uint16_t len)
{
	if(len==247)
	{
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*12);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("Device_CoolPad malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		
		if(FullState==1)
		{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
		rt_memset(SysData,0,AliyunServiceSysLength*12);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned char _RelaySum=0;
		postion=0;	
		_RelaySum = ((LevelsOfVentilationDataLength/50)/6)-1;
		for(uint8_t i=0;i<50;i++)
		{
			if(postion<(LevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//FROM TIME
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d:%02d,",(TmpInt/60),TmpInt%60);
				strcat(SysData,tmp);
				postion =postion+4;
				if(_RelaySum==0)
			{
				for(unsigned char relay=0;relay<4;relay++)
				{
					strcat(SysData,",");
				}
			}
			else if(_RelaySum==1)
			{
				for(unsigned char s=0;s<LevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = LevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=LevelsOfVentilationData[postion+1]+1;ss<4;ss++)
				{
					strcat(SysData,",");
				}
				postion =postion+6;
			}
			else
			{
				for(unsigned char s=0;s<LevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = LevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=0;ss<_RelaySum-1;ss++)
				{
					for(unsigned char t=LevelsOfVentilationData[postion+1]+1;t<LevelsOfVentilationData[postion+7];t++)
					{
						strcat(SysData,",");
					}
					postion =postion+6;
					TmpInt = LevelsOfVentilationData[postion+4];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
 				}
				for(unsigned char j=LevelsOfVentilationData[postion+1]+1;j<4;j++)
				{
					strcat(SysData,",");
				}
				
				postion =postion+6;
			}

		
		}
	}
		FullState=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.light,0,sizeof(Ac20003GParams->Ac2000Device.light));
		strcat(Ac20003GParams->Ac2000Device.light,SysData);	
		//rt_kprintf("light:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_LIGHT_CMD;
		}
	}	
}
else if(len>6)
	{
	if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*12);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("Device_CoolPad malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
		rt_memset(SysData,0,AliyunServiceSysLength*12);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned char _RelaySum=0;
		postion=0;	
		_RelaySum = ((LevelsOfVentilationDataLength/50)/6)-1;
		for(uint8_t i=0;i<50;i++)
		{
			if(postion<(LevelsOfVentilationDataLength-1)) 
			{
				memset(tmp,0,sizeof(tmp));//日
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;//FROM TIME
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d:%02d,",(TmpInt/60),TmpInt%60);
				strcat(SysData,tmp);
				postion =postion+4;
				if(_RelaySum==0)
			{
				for(unsigned char relay=0;relay<4;relay++)
				{
					strcat(SysData,",");
				}
			}
			else if(_RelaySum==1)
			{
				for(unsigned char s=0;s<LevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = LevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=LevelsOfVentilationData[postion+1]+1;ss<4;ss++)
				{
					strcat(SysData,",");
				}
				postion =postion+6;
			}
			else
			{
				for(unsigned char s=0;s<LevelsOfVentilationData[postion+1];s++)
				{
					strcat(SysData,",");
				}
				TmpInt = LevelsOfVentilationData[postion+4];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				for(unsigned char ss=0;ss<_RelaySum-1;ss++)
				{
					for(unsigned char t=LevelsOfVentilationData[postion+1]+1;t<LevelsOfVentilationData[postion+7];t++)
					{
						strcat(SysData,",");
					}
					postion =postion+6;
					TmpInt = LevelsOfVentilationData[postion+4];
					memset(tmp,0,sizeof(tmp));
					sprintf(tmp,"%d,",TmpInt);
					strcat(SysData,tmp);
 				}
				for(unsigned char j=LevelsOfVentilationData[postion+1]+1;j<4;j++)
				{
					strcat(SysData,",");
				}
				
				postion =postion+6;
			}

			}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.light,0,sizeof(Ac20003GParams->Ac2000Device.light));
		strcat(Ac20003GParams->Ac2000Device.light,SysData);	
		//rt_kprintf("light:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_LIGHT_CMD;
		}
	}

}
static void Device_WaterAndFeed(unsigned char *dataSource,uint16_t len)
{
	if(len==259)
	{
	if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		
		if(FullState==1)
		{
			uint16_t postion;
		unsigned int sum=0;
		unsigned int time=0;
		int	TmpInt;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
	
		sum = ((LevelsOfVentilationDataLength/50)/6)-1;  //总共有多少组继电器
		//计算总共有多少个继电器
		postion=0;	
		for(uint8_t i=0;i<50;i++)
		{
		
				TmpInt=LevelsOfVentilationData[postion];//日
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);

				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
				//rt_kprintf("TmpInt:%d",TmpInt);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d:%02d,",(TmpInt/60),(TmpInt%60));
				strcat(SysData,tmp);
				postion=postion+4;
				
				if((LevelsOfVentilationData[postion]==0x17)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在水1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x17)&&(time<sum))//存在水2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time++;
						}
						else
						{
								strcat(SysData,"*,");
								
						}
						
					}
					else if(LevelsOfVentilationData[postion]==1)//存在水2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];//日
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time++;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
				}
				
				
				
				
				if((LevelsOfVentilationData[postion]==0x18)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在饲料1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x18)&&(time<sum))//存在饲料2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time++;
						}
						else
						{
								strcat(SysData,"*,");
						}
					}
					else if(LevelsOfVentilationData[postion]==1)//存在饲料2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time++;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
				}
				
				
				if((LevelsOfVentilationData[postion]==0x19)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在蛟龙1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x19)&&(time<sum))//存在蛟龙2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time=0;
						}
						else
						{
								strcat(SysData,"*,");
								time=0;
						}
					}
					else if(LevelsOfVentilationData[postion]==1)//存在蛟龙2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time=0;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
					time=0;
				}
			
		}	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.water_and_feed,0,sizeof(Ac20003GParams->Ac2000Device.water_and_feed));
		strcat(Ac20003GParams->Ac2000Device.water_and_feed,SysData);
		//rt_kprintf("Device- water_and_feed:%s\n",Ac20003GParams->Ac2000Device.water_and_feed);
		
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_WATER_AND_FEED_CMD;
		}
		
				FullState=0;
		
		
		}
}
	else if(len>6)
	{
		if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	

		uint16_t postion;
		unsigned int sum=0;
		unsigned int time=0;
		int	TmpInt;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*12);
	  rt_memset(SysData,0,AliyunServiceSysLength);
	
		sum = ((LevelsOfVentilationDataLength/50)/6)-1;  //总共有多少组继电器
		//计算总共有多少个继电器
		postion=0;	
		for(uint8_t i=0;i<50;i++)
		{
		
				TmpInt=LevelsOfVentilationData[postion];//日
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);

				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
				//rt_kprintf("TmpInt:%d",TmpInt);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d:%02d,",(TmpInt/60),(TmpInt%60));
				strcat(SysData,tmp);
				postion=postion+4;
				
				if((LevelsOfVentilationData[postion]==0x17)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在水1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x17)&&(time<sum))//存在水2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time++;
						}
						else
						{
								strcat(SysData,"*,");
								
						}
						
					}
					else if(LevelsOfVentilationData[postion]==1)//存在水2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];//日
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time++;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
				}
				
				
				
				
				if((LevelsOfVentilationData[postion]==0x18)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在饲料1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x18)&&(time<sum))//存在饲料2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time++;
						}
						else
						{
								strcat(SysData,"*,");
						}
					}
					else if(LevelsOfVentilationData[postion]==1)//存在饲料2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time++;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
				}
				
				
				if((LevelsOfVentilationData[postion]==0x19)&&(time<sum))
				{
					postion= postion+1;
					if(LevelsOfVentilationData[postion]==0)//存在蛟龙1
					{
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion= postion+2;
						time++;
						if((LevelsOfVentilationData[postion]==0x19)&&(time<sum))//存在蛟龙2
						{
								postion=postion+4;
								TmpInt=LevelsOfVentilationData[postion];
								memset(tmp,0,sizeof(tmp));
								sprintf(tmp,"%d,",TmpInt);
								strcat(SysData,tmp);
								postion = postion+2;
								time=0;
						}
						else
						{
								strcat(SysData,"*,");
								time=0;
						}
					}
					else if(LevelsOfVentilationData[postion]==1)//存在蛟龙2
					{
						strcat(SysData,"*,");
						postion= postion+3;
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion = postion+2;
						time=0;
					}
				}
				else
				{
					strcat(SysData,"*,");
					strcat(SysData,"*,");
					time=0;
				}
			
		}	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.water_and_feed,0,sizeof(Ac20003GParams->Ac2000Device.water_and_feed));
		strcat(Ac20003GParams->Ac2000Device.water_and_feed,SysData);
		//rt_kprintf("Device- water_and_feed:%s\n",Ac20003GParams->Ac2000Device.water_and_feed);
		
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_WATER_AND_FEED_CMD;
		}
		
		Ac20003GParams->Ac2000Device.LevelsOfVentilationCmdCount=0;
	}
}	

static void Device_ExtraSystem(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 41 28 01 7f 
	3c 00 开始时间01:00
	78 00 结束时间02:00
	01 00 开始湿度
	01 00 结束湿度
	64 00 开启（秒）
	64 00 关闭（秒）
	cd cc cc 3e  开始温度
	00 00 00 3f  结束温度
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 3f 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 3f 
	00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 3f 
	1d 
	*/
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
	rt_memset(Ac20003GParams->Ac2000Device.extra_system,0,sizeof(Ac20003GParams->Ac2000Device.extra_system));
	strcat(Ac20003GParams->Ac2000Device.extra_system,SysData);	
	//rt_kprintf("DEVICE_EXTRA_SYSTEM_CMD:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=DEVICE_EXTRA_SYSTEM_CMD;
	}
}


void Device_WaterOnDemand(unsigned char *dataSource,uint16_t len)
{
	if(len==261)
	{
	if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*3);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(LevelsOfVentilationDataLength<=256*2)
	{
		rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
		LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
	}
	else
	{
		rt_memset(LevelsOfVentilationData,0,256*3);
		LevelsOfVentilationDataLength=0;
	}
	}
	else if(len>6)
	{
		if(LevelsOfVentilationData==NULL) 
		{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*3);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
		}
		if(LevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*3);
			LevelsOfVentilationDataLength=0;
		}
		
		uint16_t postion;
		int	TmpInt;
		char tmp[10];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
	  rt_memset(SysData,0,AliyunServiceSysLength*3);
		postion=0;	
		for(uint8_t i=0;i<50;i++)
		{
		
				TmpInt=LevelsOfVentilationData[postion];//日
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);

				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02d:%02d,",(TmpInt/60),(TmpInt%60));
				strcat(SysData,tmp);
	
				postion=postion+2;
				TmpInt=LevelsOfVentilationData[postion];//继电器是否打开
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;				
			
		}

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Device.water_on_demand ,0,sizeof(Ac20003GParams->Ac2000Device.water_on_demand));
		strcat(Ac20003GParams->Ac2000Device.water_on_demand,SysData);
		rt_kprintf("Device- water_on_demand:%s\n",Ac20003GParams->Ac2000Device.water_on_demand);
		
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=DEVICE_WATER_ON_DEMAND_CMD;
		}
		

	}

	
	}
	



static void ManageStockOfChicken(unsigned char *dataSource,uint16_t len)
{
/*
	01 14 01 0c 01 23 
	02 00          合计死亡的公鸡
	02 00          合计死亡的母鸡
	04 00          合计死亡的鸡
	03 00          合计剔除的公鸡
	04 00          合计剔除的母鸡
	07 00          合计剔除的鸡
	05 00 00 00    合计移除的公鸡
	06 00 00 00    合计移除的母鸡
	0b 00 00 00    合计移除的鸡
	10 27 00 00    放置的公鸡
	10 27 00 00    放置的母鸡
	20 4e 00 00    放置的鸡
	08 
*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	long TmpLong;
	char tmp[8];
	long  tmp1,tmp2,tmp3;
	strcat(SysData,"0,0,0,0,0,0,0,0,0,");
	
	postion=30;//放置的公鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp1=TmpLong;
	
	postion=34;//放置的母鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp2=TmpLong;
	
	postion=38;//放置的鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp3=TmpLong;
		
	postion=6;//合计死亡的公鸡
	memset(tmp,0,sizeof(tmp));
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp1=tmp1-TmpLong;
	
	postion=8;//合计死亡的母鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp2=tmp2-TmpLong;
	
	postion=10;//合计死亡的鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp3=tmp3-TmpLong;
	
	postion=12;//合计剔除的公鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp1=tmp1-TmpLong;
	
	postion=14;//合计剔除的母鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp2=tmp2-TmpLong;
	
	postion=16;//合计剔除的鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp3=tmp3-TmpLong;
	
	postion=18;//合计移除的公鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp1=tmp1-TmpLong;
	
	postion=22;//合计移除的母鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp2=tmp2-TmpLong;
	
	postion=26;//合计移除的鸡
	TmpLong=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",TmpLong);
	strcat(SysData,tmp);
	tmp3=tmp3-TmpLong;
	
	//公鸡数
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",tmp1);
	strcat(SysData,tmp);
	
	//母鸡数
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",tmp2);
	strcat(SysData,tmp);
	
	//鸡数
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%ld,",tmp3);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Manage.StockOfChicken,0,sizeof(Ac20003GParams->Ac2000Manage.StockOfChicken));
	strcat(Ac20003GParams->Ac2000Manage.StockOfChicken,SysData);	
	rt_kprintf("MANAGE_STOCKOFCHICKEN_CMD:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=MANAGE_STOCKOFCHICKEN_CMD;
	}
}


static void ManageStockOfFeed(unsigned char *dataSource,uint16_t len)
{
	if(len==257)
	{
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*20);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("Device_Foggers malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*20);
			LevelsOfVentilationDataLength=0;
		}
	}
	else
	{
		if(len>6)
		{
			if(LevelsOfVentilationDataLength<=256*19)
		 {
				rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
				LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		 }
  	}
		my_log(LevelsOfVentilationData,LevelsOfVentilationDataLength);
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength * 4);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int TmpInt;
		int Year;
		int month;
		int day;
		char tmp[8];
		postion=10;
		for(unsigned char i=0;i<100;i++)
		{
		if(postion<LevelsOfVentilationDataLength)
		{
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",i);
		strcat(SysData,tmp);
		
		TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
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
		TmpInt = (int)(FourByteToFloat(&LevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion+=4;
		TmpInt = (int)(FourByteToFloat(&LevelsOfVentilationData[postion])*2.203f);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		
		postion+=4;
		 
		}
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Manage.StockOfFeed,0,sizeof(Ac20003GParams->Ac2000Manage.StockOfFeed));
		strcat(Ac20003GParams->Ac2000Manage.StockOfFeed,SysData);	
		rt_kprintf("StockOfFeed:%s\n",SysData);
		rt_free(SysData);
		
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGE_STOCKOFFEED_CMD;
		}
	}
}
static void ManageTimeAndDate1(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 5d 01 74 
	da e8 a6 43 
	ab 
	*/
	//my_log(dataSource, len);
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int TmpInt;
	int Hour;
	int Minute;
	char tmp[32];
	postion=6;
	TmpInt=(int)FourByteToFloat(&dataSource[postion]);

	Hour = TmpInt/60;
	Minute = TmpInt%60;
	
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%2d",Hour);
	strcat(SysData,tmp);
	strcat(SysData,":");
	
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%2d,",Minute);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Manage.TimeAndDate1,0,sizeof(Ac20003GParams->Ac2000Manage.TimeAndDate1));
	strcat(Ac20003GParams->Ac2000Manage.TimeAndDate1,SysData);	
	rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",SysData);

	rt_free(SysData);
}
static void ManageTimeAndDate2(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;

	int TmpInt;
	char tmp[32];
	postion=6;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%2d",TmpInt);
	strcat(SysData,tmp);
	strcat(SysData,"-");
	
	postion=7;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%2d",TmpInt);
	strcat(SysData,tmp);
	strcat(SysData,"-");

	postion=8;
	TmpInt=dataSource[postion]+2000;
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	postion=9;
	TmpInt=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpInt);
	strcat(SysData,tmp);
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	
	strcat(Ac20003GParams->Ac2000Manage.TimeAndDate2,SysData);	
	rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",Ac20003GParams->Ac2000Manage.TimeAndDate2);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=MANAGE_TIMEANDDATA_CMD;
	}
}
static void ManageGrowthAndHen(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	uint16_t TmpLong;
	char tmp[32];

	/*
	01 14 01 0D 01 24 
	01 00  鸡群编号
	00 00 
	01
	*/
	
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",Ac2000Data[ADDR_3G_1804]);
	strcat(SysData,tmp);
	
	postion=6;
	TmpLong=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);
	
	postion=7;
	TmpLong=dataSource[postion];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	strcat(SysData,tmp);

	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Manage.GrowthAndHen,0,sizeof(Ac20003GParams->Ac2000Manage.GrowthAndHen));
	strcat(Ac20003GParams->Ac2000Manage.GrowthAndHen,SysData);	
	rt_kprintf("GrowthAndHen:%s\n",Ac20003GParams->Ac2000Manage.GrowthAndHen);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=MANAGE_GROWTHANDHEN_CMD;
	}
	rt_free(SysData);
}
static void ManagePasswordSet(unsigned char *dataSource,uint16_t len)
{
	/*
	 01 14 01 12 01 29 
	 00 00   9A 07   00 00   00 00   00 00   00 00   00 00 
	 A1 	
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[32];
	uint16_t TmpLong;
	
	postion=6;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	postion=8;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	
	postion=10;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	postion=12;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	
	postion=14;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	
	postion=16;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	
	postion=18;
	TmpLong=dataSource[postion]*256+dataSource[postion+1];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%d,",TmpLong);
	if(TmpLong==0)
	{
		strcat(SysData,"------,");
	
	}
	else
	{
	strcat(SysData,tmp);
	}
	
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac20003GParams->Ac2000Manage.Password,0,sizeof(Ac20003GParams->Ac2000Manage.Password));
	strcat(Ac20003GParams->Ac2000Manage.Password,SysData);	
	rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",Ac20003GParams->Ac2000Manage.Password);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=MANAGE_PASSWORD_CMD;
	}
	

}
static void ManageAlarmSet(unsigned char *dataSource,uint16_t len)
{
	/*	
		01 00 00 00 00 00 00 00 00 00 70 42 00 00 01 00 
		02 00 00 00 00 00 00 00 00 00 f0 41 00 00 01 00 
		03 00 00 00 ff ff ff ff 00 00 00 00 01 00 01 00  
		04 00 00 00 00 00 00 00 00 00 00 00 04 00 01 00  
		05 00 00 00 00 00 00 00 00 00 48 42 04 00 01 00 
		06 00 00 00 00 00 00 00 e4 38 0e 3f 05 00 01 00 
		07 00 00 00 00 00 00 00 e4 38 0e 3f 05 00 01 00 
		08 00 00 00 ff ff ff ff 00 00 00 00 01 00 01 00 
		09 00 00 00 00 00 00 00 00 00 34 44 03 00 01 00 
		0a 00 00 00 00 00 00 00 00 00 00 00 02 00 01 00 
		0b 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 
		0c 00 00 00 ff ff ff ff 00 00 00 00 01 00 01 00 
		0d 00 00 00 00 00 00 00 00 00 70 41 00 00 01 00 
		0e 00 00 00 00 00 00 00 00 00 00 00 02 00 01 00 
		12 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		13 00 00 00 00 00 00 00 00 00 f0 43 03 00 00 00 
		14 00 00 00 00 00 00 00 00 00 96 44 03 00 00 00 
		15 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		15 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 
		16 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		17 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 
		18 00 00 00 00 00 00 00 00 00 80 3f 00 00 00 00 
		19 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		1a 00 00 00 00 00 00 00 00 00 48 42 00 00 00 00 
		1b 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		1c 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		1d 00 00 00 00 00 00 00 00 00 20 41 00 00 00 00 
		1e 00 00 00 00 00 00 00 00 00 20 41 00 00 00 00 
		1f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		20 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00  
		21 00 00 00 00 00 00 00 00 00 b4 43 03 00 00 00 
		22 00 00 00 00 00 00 00 00 00 a5 44 03 00 00 00 
		23 00 00 00 00 00 00 00 00 00 00 00 02 00 00 00 
		24 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		25 00 00 00 00 00 00 00 00 00 70 42 00 00 00 00 
		26 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		27 00 00 00 00 00 00 00 00 00 00 00 03 00 00 00 
		28 00 00 00 01 00 00 00 00 00 00 00 03 00 00 00 
		29 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		2a 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
		2a 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 
		2b 00 00 00 00 00 00 00 00 00 70 42 00 00 00 00 
		2c 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		2d 00 00 00 00 00 00 00 00 00 48 42 04 00 00 00 
		2e 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00  
		2f 00 00 00 00 00 00 00 00 40 9c 45 00 00 00 00 
		30 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		31 00 00 00 00 00 00 00 00 00 c8 42 00 00 00 00 
		32 00 00 00 ff ff ff ff 00 00 00 00 01 00 00 00 
		33 00 00 00 00 00 00 00 00 50 43 47 00 00 00 00 
		34 00 00 00 00 00 00 00 00 50 c3 48 00 00 00 00 
	*/
	if(LevelsOfVentilationData==NULL) 
	{
		LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
		if(LevelsOfVentilationData!=NULL)
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(len>6)
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);	
		}
		else 
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	}
	if(len<247)
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		float TmpFloat;
		char tmp[32];
		int	TmpInt;
		postion=0;
		for(uint8_t i=0;i<15;i++)
		{
			TmpInt=LevelsOfVentilationData[postion];//序号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			//strcat(SysData,tmp);
			postion=postion+4;	
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0xFF&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0xFF)//没有启用
			{
				postion = postion+12;
			}
			else
			{
				postion=postion+4;
				TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				postion = postion+8;
			}
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Manage.AlarmSet,0,sizeof(Ac20003GParams->Ac2000Manage.AlarmSet));
		strcat(Ac20003GParams->Ac2000Manage.AlarmSet,SysData);	
		rt_kprintf("ManageAlarmSet:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGE_ALARMSET_CMD;
		}
	}
}
//static void ManageAlarmReset(unsigned char *dataSource,uint16_t len)
//{
//	/*
//	01 14 01 04 01 1b 
//	b7 1b a3 42 
//	c4 20 a3 42 
//	b7 1b a3 42 
//	37 
//	*/
//}
//static void ManagePassword(unsigned char *dataSource,uint16_t len)
//{

//}
//鸡秤布局
static void ChickLayout(unsigned char *dataSource,uint16_t len)
{
	/*
	舎内        数据      器械
	1			?         ？
	
	2           ?         ?
	*/
	
	
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[32];
	int	TmpInt;
	postion = len-1;//判断最后一个字节
	if(dataSource[postion] == 0x16)//两个鸡秤都没有 器械0 0 
	{
		TmpInt = 0;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		TmpInt = 0;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	}
	else if(dataSource[postion] == 0x18)// 器械 1 0 
	{
		TmpInt = 1;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		TmpInt = 0;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	}
	else if(dataSource[postion] == 0x1a) //器械 0 2
	{
		TmpInt = 0;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		TmpInt = 2;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	}
	else if(dataSource[postion] == 0x1b)
	{
		postion = 162; 
		if(dataSource[postion] == 0x00) //器械1 2
		{
			TmpInt = 1;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			TmpInt = 2;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
		else if(dataSource[postion] == 0x01)//器械 2 1
		{	
			TmpInt = 2;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			TmpInt = 1;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	memset(Ac20003GParams->Ac2000ChickenPound.layout,0,sizeof(Ac20003GParams->Ac2000ChickenPound.layout));
	strcat(Ac20003GParams->Ac2000ChickenPound.layout,SysData);	
	rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",Ac20003GParams->Ac2000ChickenPound.layout);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=CHICK_LAYOUT;
	}
	
	
}

static void ChickBasicSetting(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 3A 01 51
	01 14 02 
	14 0A 14 0A 14 0A 71 00 00 00 00 
	E2 	
	*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[32];
	int	TmpInt;
	int hour,minute;
	if(Set_difference == 1)//判断是基本设置
	{		
		postion = 6;  //鸡秤模式
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 7; //整齐度范围
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 8;//曲线选择器
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		memset(Ac20003GParams->Ac2000ChickenPound.Setting,0,sizeof(Ac20003GParams->Ac2000ChickenPound.Setting));
		strcat(Ac20003GParams->Ac2000ChickenPound.Setting,SysData);	
		rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",Ac20003GParams->Ac2000ChickenPound.Setting);
		rt_free(SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CHICK_BASICSETTING;
		}
	}
	else if(Set_difference == 2)//判断是鸡磅设置
	{
		postion = 11;  //
		TmpInt = dataSource[postion]; //Female Range upper
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion = 12;  //
		TmpInt = dataSource[postion]; //Female Range lower
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion = 13;  //
		TmpInt = dataSource[postion]; //male Range Upper
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion = 14;  //
		TmpInt = dataSource[postion]; //Female Range lower
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion = 16;  
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//start time
		hour = TmpInt/60;
		minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%4d,",(hour*100+minute));
		strcat(SysData,tmp);
	
		postion = 18;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//stop time
		hour = TmpInt/60;
		minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%4d,",(hour*100+minute));
		strcat(SysData,tmp);
	    if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		strcat(Ac20003GParams->Ac2000ChickenPound.Setting,SysData);	
		rt_kprintf("MANAGE_TIMEANDDATA_CMD:%s\n",Ac20003GParams->Ac2000ChickenPound.Setting);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
		AirxxxStatus.recv_systeminfo_cmd=3;
		AirxxxStatus.aliyun_service_param_reply=CHICK_SETTING;
	}
	}
}
unsigned char state;
unsigned char Curve_difference =0;
static void ChickCurse(unsigned char *dataSource,uint16_t len)//曲线数据
{

	if(len==241)
	{
		if(Curve_difference == 0)
		{
			
			
			if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*12);
					LevelsOfVentilationDataLength=0;
				}
			else
			{
				rt_kprintf("Device_CoolPad malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],60);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+60;
			Curve_difference =1;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	  }
	  else if(Curve_difference == 1)
	  {
		 rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[78],len-80);
		 LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-80);
		 Curve_difference =2;
	  }
	}
	else if(len == 85)
	{
		if((LevelsOfVentilationDataLength<=256*11)&&(Curve_difference ==2))
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		unsigned int frequency;
		postion =0;
		for(frequency =0; frequency<30; frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			strcat(SysData," ,");
			postion+=2;
		}
		postion = 60;
		for(frequency=0; frequency<62; frequency++)
		{
			TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.2f,",(TmpFloat*2.203f));
			strcat(SysData,tmp);
			postion+=4;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000ChickenPound.Curse,0,sizeof(Ac20003GParams->Ac2000ChickenPound.Curse));
		strcat(Ac20003GParams->Ac2000ChickenPound.Curse,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CHICK_CURVE;
		}
	}
}

static void Chickhistory(unsigned char *dataSource,uint16_t len)
{
	
	
	
}

static void ChickConversion(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float TmpFloat;
	unsigned int frequency;
	postion = 8;
	for(frequency=0;frequency<5;frequency++)
	{
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日期
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日期
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		if((dataSource[postion] ==0xFF)&&(dataSource[postion+1]==0xFF)&&(dataSource[postion+2]==0xFF)&&(dataSource[postion+3]==0xFF))
		{
			strcat(SysData,tmp);
			postion+=4;
		}
			TmpFloat=FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.2f,",TmpFloat);
			strcat(SysData,tmp);
		postion+=4;
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000ChickenPound.Conversion,0,sizeof(Ac20003GParams->Ac2000ChickenPound.Conversion));
		strcat(Ac20003GParams->Ac2000ChickenPound.Conversion,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CHICK_CONVERSION;
		}
}

static void ChickTest(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	postion =6;
	if(dataSource[postion]==0x10)//存在
	{
		if(dataSource[postion+1]==0x01)//第一个是鸡舍二号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",2);
			strcat(SysData,tmp);
		}
		else//第一个存在的是鸡舍一号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",1);
			strcat(SysData,tmp);
		}
		postion = 10;
		TmpInt = dataSource[postion]+dataSource[postion+1];//A/D
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	}
	postion =13;
	if(dataSource[postion]==0x10)//存在
	{
		if(dataSource[postion+1]==0x01)//第二个是鸡舍二号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",2);
			strcat(SysData,tmp);
		}
		else//第二个存在的是鸡舍一号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",1);
			strcat(SysData,tmp);
		}
		postion = 17;
		TmpInt = dataSource[postion]+dataSource[postion+1];//A/D
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000ChickenPound.Test,0,sizeof(Ac20003GParams->Ac2000ChickenPound.Test));
		strcat(Ac20003GParams->Ac2000ChickenPound.Test,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CHICK_TEST;
		}
}

static void Chickcalibration(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	char tmp[8];
	postion =6;
	float TmpFloat;
	if(dataSource[postion]==0x10)//存在
	{
		if(dataSource[postion+1]==0x01)//第一个是鸡舍二号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",2);
			strcat(SysData,tmp);
		}
		else//第一个存在的是鸡舍一号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",1);
			strcat(SysData,tmp);
		}
		postion = 12;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",(TmpFloat/2.203f));
		strcat(SysData,tmp);
	}
		postion =17;
	if(dataSource[postion]==0x10)//存在
	{
		if(dataSource[postion+1]==0x01)//第二个是鸡舍二号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",2);
			strcat(SysData,tmp);
		}
		else//第二个存在的是鸡舍一号
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",1);
			strcat(SysData,tmp);
		}
		postion = 23;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",(TmpFloat/2.203f));
		strcat(SysData,tmp);
	}
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000ChickenPound.calibration,0,sizeof(Ac20003GParams->Ac2000ChickenPound.calibration));
		strcat(Ac20003GParams->Ac2000ChickenPound.calibration,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CHICK_CALIBRATION;
		}
}

static void AC2000ReciverFlog(unsigned char *dataSource,uint16_t len,unsigned char flog)
{
	unsigned char data=0;
	data = dataSource[14];//接收到的总共数据有多少组
	switch(flog)
	{
		case TEMPERATURE:
		{
			
		}break;
		case HUMIDITY:
		{
		
		}break;
		case CO2:
		{
		
		}break;
		case WATER:
		{
			if(data%5==0)
			{
				TimerWater = data/5;
			}
			else
			{
				TimerWater = data/5+1;
			}
		}break;
		case FEED:
		{
			if(data%10==0)
			{
				TimerFeed = data/10;
			}
			else
			{
				TimerFeed = data/10+1;
			}
		}break;
		case DEAD:
		{
			
		}break;
		case HEATER:
		{
			if(data%3==0)
			{
				TimerHeater = data/3;
			}
			else
			{
				TimerHeater = data/3+1;
			}
		}break;
		case RADIANT_HEATER:
		{
			if(data%3==0)
			{
				TimerHeater = data/3;
			}
			else
			{
				TimerHeater = data/3+1;
			}
		
		}break;
		case ALARM:
		{
			if(data%16==0)
			{
				TimerAlarm = data/16;
			}
			else
			{
				TimerAlarm = data/16+1;
			}
		}break;
		case EVENT_MAP:
		{
			if(data%16==0)
			{
				TimerMap = data/16;
			}
			else
			{
				TimerMap = data/16+1;
			}
		
			
		}break;
		case WATCH:
		{
			if(data%4==0)
			{
				TimerWatch = data/4;
			}
			else
			{
				TimerWatch = data/4+1;
			}
		
		}break;
		case SETT:
		{
			
		}break;
	
	}
}

static void AC2000HistoryTemperature(unsigned char *dataSource,uint16_t len)
{
	
/*
FF 41 54 54 45 00 00 00 0F 00 01 01 53 01 00 00 32 97 	
01 14 01 53 01 6A 
01 00 01 00 0F 0A 4A 08 1E 0A   0A0F-平均值  084A--最低值  081E最高值
02 00 02 00 A6 08 2D 08 27 0A   
03 00 03 00 15 09 82 08 A7 0A 
04 00 04 00 E9 09 8D 08 2D 0A 
05 00 05 00 52 09 C4 07 BB 0C 
C9 	
*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	unsigned char cycle=0;
	int	TmpInt;
	char tmp[8];
	float TmpFloat;
	postion =6;
	for(cycle=0;cycle<5;cycle++)
	{
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=4;
		
		TmpFloat = (float)((dataSource[postion]+dataSource[postion+1]*256)/100.0);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f",TmpFloat);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpFloat = (float)((dataSource[postion]+dataSource[postion+1]*256)/100.0);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f",TmpFloat);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpFloat = (float)((dataSource[postion]+dataSource[postion+1]*256)/100.0);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f",TmpFloat);
		strcat(SysData,tmp);
		postion+=2;
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Temperature,0,sizeof(Ac20003GParams->Ac2000History.Temperature));
		strcat(Ac20003GParams->Ac2000History.Temperature,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORT_TEMPERATURE_CMD;
		}
}
static void AC2000HistoryHumidity_treatm(unsigned char *dataSource,uint16_t len)
{
/*
	Data: FF 41 54 54 45 00 00 00 0F 00 01 01 54 01 00 00 3C A2 	
Data: 01 14 01 54 01 6B 
01 00 01 00 00 00 E8 03 00 00  
02 00 02 00 00 00 E8 03 00 00 
03 00 03 00 00 00 E8 03 00 00 
04 00 04 00 00 00 E8 03 00 00 
05 00 05 00 00 00 E8 03 00 00 
06 00 06 00 00 00 E8 03 00 00
AC
*/
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	unsigned char cycle=0;
	int	TmpInt;
	char tmp[8];
	postion =6;
	for(cycle=0;cycle<5;cycle++)
	{
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=4;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//平均
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//最低
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//最高
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Humidity_treatm,0,sizeof(Ac20003GParams->Ac2000History.Humidity_treatm));
		strcat(Ac20003GParams->Ac2000History.Humidity_treatm,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_HUMIDITY_TREATM_CMD;
		}
		

	
}
static void AC2000HistoryCO2(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	unsigned int postion;
	unsigned char cycle=0;
	int	TmpInt;
	char tmp[8];
	postion =6;
	for(cycle=0;cycle<5;cycle++)
	{
		TmpInt = dataSource[postion]+dataSource[postion+1]*256;//日
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=4;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//平均
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//最低
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
		
		TmpInt = (dataSource[postion]+dataSource[postion+1]*256);//最高
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		postion+=2;
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.CO2,0,sizeof(Ac20003GParams->Ac2000History.CO2));
		strcat(Ac20003GParams->Ac2000History.CO2,SysData);	
		rt_kprintf("Curse:%s\n",SysData);
		rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_CO2_CMD;
		}
	
}


static void AC2000HistoryWater(unsigned char *dataSource,uint16_t len)
{
/*水
Data: FF 41 54 54 45 00 00 00 0F 00 01 01 7E 01 00 00 FA 8A 	
Data: 01 14 01 7E 01 95 	
01 00 01 00 FF 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
02 00 02 00 00 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
03 00 03 00 00 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
04 00 04 00 00 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
05 00 05 00 00 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
09 

Data: FF 41 54 54 45 00 00 00 0F 00 01 01 7E 01 05 00 64 F9 	
Data: 01 14 01 7E 01 95 
06 00 06 00 00 00 FF 00 FF 00 FF 00 FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
07 00 07 00 00 00 00 00 00 00 FF 00 FF 00 22 01 00 00 22 01 00 00 00 00 00 00 00 00 00 00 44 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 44 02 00 00 
E6
*/
	static unsigned int ReciverWater=0;
	if(ReciverWater<TimerWater-1)
	{
		if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*12);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverWater++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency;
		postion =0;
		Maxfrequency = AliyunServiceSysLength/50;//最大重复次数
		for(frequency=0;frequency<Maxfrequency;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10;//合计喝水量百分比差值
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
				
			}
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//水量百分1差别
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//水量百分2差别
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//每日水量1
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//每日水量2
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=12;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//合计每日喝水量
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//制冷板
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//喷雾器
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//冲洗用的水
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10;//总用水量
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Water,0,sizeof(Ac20003GParams->Ac2000History.Water));
		strcat(Ac20003GParams->Ac2000History.Water,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_WATER_CMD;
		}
		
	
	}
	
	
	
	
	
	
	
 	
}

static void AC2000HistoryFeed(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverFeed=0;
	if(ReciverFeed<TimerFeed-1)
	{
		if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*12);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverFeed++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency;
		postion =0;
		Maxfrequency = AliyunServiceSysLength/22;//最大重复次数
		for(frequency=0;frequency<Maxfrequency;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10)*2.203;//合计百分差比
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10)*2.203;//百分一差比
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10)*2.203;//百分2差比
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=2;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10)*2.203;//每日合计量
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10)*2.203;//每日饲料1
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
			if(LevelsOfVentilationData[postion]==0xFF&&LevelsOfVentilationData[postion+1]==0&&LevelsOfVentilationData[postion+2]==0xFF&&LevelsOfVentilationData[postion+3]==0)//不详
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt =((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256+LevelsOfVentilationData[postion+2]*256*256+LevelsOfVentilationData[postion+3]*256*256*256)/10)*2.203;//每日饲料2
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			postion+=4;
		}
	}
}

static void AC2000HistoryDead(unsigned char *dataSource,uint16_t len)
{
	if(len==247)
	{
		if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*12);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
	}
	else if(len==47)
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		unsigned int frequency;
		postion =0;
		for(frequency=0;frequency<7;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=4;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//公鸡死亡的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//母鸡死亡的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//今日死亡的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//公鸡剔除的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//母鸡剔除的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//今日剔除的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//公鸡死亡
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//公鸡死亡
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//母鸡死亡
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//母鸡死亡
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//今日死亡
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=2;
			TmpFloat = (float)((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/100.00);//公鸡百分比
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.2f",TmpFloat);
			strcat(SysData,tmp);
			postion+=2;
			TmpFloat = (float)((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/100.00);//公鸡百分比
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.2f",TmpFloat);
			strcat(SysData,tmp);
			postion+=2;
			TmpFloat = (float)((LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/100.00);//鸡百分比
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.2f",TmpFloat);
			strcat(SysData,tmp);
			postion+=2;
			
			TmpInt = (LevelsOfVentilationData[postion]+(LevelsOfVentilationData[postion+1]*256)+(LevelsOfVentilationData[postion+2]*256*256)+(LevelsOfVentilationData[postion+3]*256*256*256));//公鸡更新的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=4;
			TmpInt = (LevelsOfVentilationData[postion]+(LevelsOfVentilationData[postion+1]*256)+(LevelsOfVentilationData[postion+2]*256*256)+(LevelsOfVentilationData[postion+3]*256*256*256));//母鸡更新的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=4;
			TmpInt = (LevelsOfVentilationData[postion]+(LevelsOfVentilationData[postion+1]*256)+(LevelsOfVentilationData[postion+2]*256*256)+(LevelsOfVentilationData[postion+3]*256*256*256));//鸡总更新的
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion+=4;
			
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Dead,0,sizeof(Ac20003GParams->Ac2000History.Dead));
		strcat(Ac20003GParams->Ac2000History.Dead,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_DEAD_CMD;
		}
		
	}
	
}

static void AC2000HistoryHeater(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverHeater=0;
	if(ReciverHeater<TimerHeater-1)
	{
		if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*12);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*12);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverHeater++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency=0;
		unsigned int cycle;
		unsigned int Hour;
		unsigned int Minute;
		postion =0;
		Maxfrequency = AliyunServiceSysLength/68;
		for(frequency=0; frequency<Maxfrequency; frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			for(cycle=0;cycle<16;cycle++)
			{
				TmpInt = (LevelsOfVentilationData[postion]+(LevelsOfVentilationData[postion+1]*256)+(LevelsOfVentilationData[postion+2]*256*256)+(LevelsOfVentilationData[postion+3]*256*256*256));//公鸡更新的
				Hour = TmpInt/60;//小时
				Minute = TmpInt%60;//分
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",(Hour*100+Minute));//例如4：30  就为430 后两位为分
				strcat(SysData,tmp);
				postion +=2;
			}
			
		}
		ReciverHeater=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Heater,0,sizeof(Ac20003GParams->Ac2000History.Heater));
		strcat(Ac20003GParams->Ac2000History.Heater,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_HEATER_CMD;
		}
	}
}

static void AC2000HistoryRadiant_Heater(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverRadiantHeater=0;
	if(ReciverRadiantHeater<TimerRadiant_Heater-1)
	{
		if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*19);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*19);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverRadiantHeater++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency=0;
		unsigned int cycle;
		unsigned int Hour;
		unsigned int Minute;
		postion =0;
		Maxfrequency = AliyunServiceSysLength/132;
		for(frequency=0; frequency<Maxfrequency; frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			for(cycle=0;cycle<32;cycle++)
			{
				TmpInt = (LevelsOfVentilationData[postion]+(LevelsOfVentilationData[postion+1]*256)+(LevelsOfVentilationData[postion+2]*256*256)+(LevelsOfVentilationData[postion+3]*256*256*256));//公鸡更新的
				Hour = TmpInt/60;//小时
				Minute = TmpInt%60;//分
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",(Hour*100+Minute));//例如4：30  就为430 后两位为分
				strcat(SysData,tmp);
				postion +=2;
			}
			
		}
		ReciverRadiantHeater=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Radiant_Heater,0,sizeof(Ac20003GParams->Ac2000History.Radiant_Heater));
		strcat(Ac20003GParams->Ac2000History.Radiant_Heater,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_RADIANT_HEATER_CMD;
		}
	}
}

static void AC2000HistoryAlarm(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverAlarm;
	if(ReciverAlarm<TimerAlarm)
	{
		
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*19);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*19);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverAlarm++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency=0;
		unsigned int Hour;
		unsigned int Minute;
		unsigned int second;
		postion =0;
		Maxfrequency = LevelsOfVentilationDataLength/12;
		for(frequency=0;frequency<Maxfrequency;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]*256+LevelsOfVentilationData[postion+1];//报警编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=4;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
			second = TmpInt%32;//秒
			Minute = (TmpInt/32)%64; //分
			Hour = ((TmpInt)/32)/64; //时
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%6d",(Hour*10000+Minute*100+second));
			strcat(SysData,tmp);
		}
		
		ReciverAlarm=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Alarm,0,sizeof(Ac20003GParams->Ac2000History.Alarm));
		strcat(Ac20003GParams->Ac2000History.Alarm,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_ALARM_CMD;
		}
		
	}
	
}

static void AC2000HistoryEvent_Map(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverMap;
	if(ReciverMap<TimerMap)
	{
		
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*19);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*19);
			LevelsOfVentilationDataLength=0;
		}
		
		ReciverMap++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		unsigned int frequency;
		unsigned int Maxfrequency=0;
		unsigned int Hour;
		unsigned int Minute;
		unsigned int second;
		postion =0;
		Maxfrequency = LevelsOfVentilationDataLength/12;
		for(frequency=0;frequency<Maxfrequency;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]*256+LevelsOfVentilationData[postion+1];//事件编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=4;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
			second = TmpInt%32;//秒
			Minute = (TmpInt/32)%64; //分
			Hour = ((TmpInt)/32)/64; //时
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%6d",(Hour*10000+Minute*100+second));
			strcat(SysData,tmp);
		}
		
		ReciverMap=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Event_Map,0,sizeof(Ac20003GParams->Ac2000History.Event_Map));
		strcat(Ac20003GParams->Ac2000History.Event_Map,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_EVENT_MAP_CMD;
		}
		
	}
	
}

static void AC2000HistoryWatch(unsigned char *dataSource,uint16_t len)
{
	static unsigned int ReciverWatch;
	if(ReciverWatch<TimerWatch)
	{
		
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*60);
			if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*19);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("Device_CoolPad malloc failure\n");
				}
			}
		if(LevelsOfVentilationDataLength<=256*59)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*59);
			LevelsOfVentilationDataLength=0;
		}
		ReciverWatch++;
	}
	else
	{
		if(LevelsOfVentilationDataLength<=256*11)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+len-7;
		}
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		float TmpFloat;
		
		unsigned int frequency;
		unsigned int Maxfrequency=0;
		unsigned int Hour;
		unsigned int Minute;
		postion =4;
		Maxfrequency = LevelsOfVentilationDataLength/54;
		for(frequency=0;frequency<Maxfrequency;frequency++)
		{
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//日
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",TmpInt);
			strcat(SysData,tmp);
			postion +=2;
			TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//时间
			Minute = (TmpInt/32)%64; //分钟
			Hour = (TmpInt/32)/64; //小时
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",(Hour*10+Minute));
			strcat(SysData,tmp);
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//平均温度
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//鸡舍温度最低
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//鸡舍温度平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//鸡舍温度最高
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度一平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度二平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度三平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度四平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度五平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//温度六平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//屋顶最小
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//屋顶平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//屋顶最大
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//外部最低
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//外部平均
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x10)&&(LevelsOfVentilationData[postion+1]!=0x27))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//外部最大
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0xF6)&&(LevelsOfVentilationData[postion+1]!=0xFF))
			{
				TmpInt = -1;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//Humidity in 1 min
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0xF6)&&(LevelsOfVentilationData[postion+1]!=0xFF))
			{
				TmpInt = -1;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//Humidity in 1 Avg
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
			if((LevelsOfVentilationData[postion]!=0x00)&&(LevelsOfVentilationData[postion+1]!=0x00))
			{
				strcat(SysData,",");
			}
			else
			{
				TmpInt = LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256;//Humidity in 1 Max
				TmpFloat = (float)(TmpInt/100.0);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f",TmpFloat);
				strcat(SysData,tmp);
			}
			postion +=2;
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10;//水
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			postion +=2;
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256)/10*2.203;//饲料
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
			postion +=2;
				TmpInt = (LevelsOfVentilationData[postion]+LevelsOfVentilationData[postion+1]*256);//水平
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",TmpInt);
				strcat(SysData,tmp);
		}
		ReciverWatch=0;
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Watch,0,sizeof(Ac20003GParams->Ac2000History.Watch));
		strcat(Ac20003GParams->Ac2000History.Watch,SysData);	
		//rt_kprintf("ManageAlarmSet:%s\n",Ac20003GParams->Ac2000Manage.AlarmSet);
		rt_free(SysData);
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}
		LevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_WATCH_CMD;
		}
	}
}
static void AC2000HistorySet(unsigned char *dataSource,uint16_t len)
{
	if(len==103)
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		postion =0;
		for(uint8_t i=0;i<48;i++)
		{
			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000History.Set,0,sizeof(Ac20003GParams->Ac2000History.Set));
		strcat(Ac20003GParams->Ac2000History.Set,SysData);	
		rt_free(SysData);		
		//rt_kprintf("device Ac2000Test.AnalogSensor:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=HISTORY_SET_CMD;
		}
	}
}


//测试
//static void AC2000TestSwitchRelay(unsigned char *dataSource,uint16_t len)
//{

//}
static void AC2000TestAnalogSensor(unsigned char *dataSource,uint16_t len)
{
/*
	01 14 00 7a 01 90 7e 00 00 00 7e 

	01 14 02 7a 01 92 
	00 00 00 00 
	c7 01 5a d6 2e 45 00 00 00 00 01 00 00 00 
	fe 03 00 00 00 00 00 3c 1c c6 02 00 00 00 
	b8 01 d1 83 37 45 00 00 00 00 03 00 00 00 
	fe 03 00 00 00 00 00 3c 1c c6 04 00 00 00 
	bb 03 00 00 00 00 00 3c 1c c6 05 00 00 00 
	fe 03 00 00 00 00 00 3c 1c c6 06 00 00 00 
	fe 03 00 00 00 00 00 3c 1c c6 07 00 00 00 
	fe 03 00 00 00 00 00 3c 1c c6 08 00 00 00 
	06 00 00 00 80 bf 00 00 00 00 d4 
	*/
	if(len==11)
	{
		/*
		01 14 00 44 01 5a 
		00 00 00 00 
		00 
		*/
		Ac20003GParams->Ac2000Test.AnalogSensorNum=dataSource[6];
	}
	else 
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		//int mode=0;
		float TmpFloat;
		
//		switch(Ac20003GParams->Ac2000Test.AnalogSensorNum)
//		{
//			case 0x7e:mode=1;break;
//			default:break;
//		}
		postion=10;
		for(uint8_t i=0;i<9;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",(i+1));
			strcat(SysData,tmp);
			
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+2;
			
			TmpFloat=FourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			if(TmpFloat<10000.0f)
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
			postion=postion+12;
		}
	  if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Test.AnalogSensor,0,sizeof(Ac20003GParams->Ac2000Test.AnalogSensor));
		strcat(Ac20003GParams->Ac2000Test.AnalogSensor,SysData);
		rt_free(SysData);		
		//rt_kprintf("device Ac2000Test.AnalogSensor:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TEST_ANALOGSENSOR_CMD;
		}
	}	
}
static void AC2000TestDigitalSensor(unsigned char *dataSource,uint16_t len)
{
/*
	01 14 00 7b 01 91 12 00 00 00 12 

	01 14 02 7b 01 93 
	00 00 00 00 01 
	00 01 00 00    water1
	01 00 00 00 01 
	08 00 00 00    water2
	0c   
	
	01 14 02 7b 01 93 
	00 00 00 00 01 
	00 00 00 00 
	01 00 00 00 01 
	00 00 00 00 
	03 
	
	*/
	if(len==11)
	{
		Ac20003GParams->Ac2000Test.DigitalSensorNum=dataSource[6];
	}
	else if(len==25)
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		long	TmpInt;
		int	TmpIntX;
		char tmp[8];
		//int mode=0;

//		switch(Ac20003GParams->Ac2000Test.AnalogSensorNum)
//		{
//			case 0x7e:mode=1;break;
//			default:break;
//		}
		
		postion=6;
		TmpIntX=dataSource[postion];//水表1序号
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpIntX);
		strcat(SysData,tmp);
		
		postion=10;
		TmpIntX=dataSource[postion];//水表1状态
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpIntX);
		strcat(SysData,tmp);
		
		postion=11;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld,",TmpInt);
		strcat(SysData,tmp);

		postion=15;
		TmpIntX=dataSource[postion];//水表1序号
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpIntX);
		strcat(SysData,tmp);
		
		postion=19;
		TmpIntX=dataSource[postion];//水表1状态
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpIntX);
		strcat(SysData,tmp);
		
		postion=20;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256+dataSource[postion+2]*256*256+dataSource[postion+3]*256*256*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%ld,",TmpInt);
		strcat(SysData,tmp);
		
	  if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Test.DigitalSensor,0,sizeof(Ac20003GParams->Ac2000Test.DigitalSensor));
		strcat(Ac20003GParams->Ac2000Test.DigitalSensor,SysData);
		rt_free(SysData);		
		//rt_kprintf("device Ac2000Test.DigitalSensor:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TEST_DIGITALSENSOR_CMD;
		}
	}
}
static void AC2000TestStaticPressure(unsigned char *dataSource,uint16_t len)
{
/*
	01 14 01 7c 01 93 
	00 00 
	00 
*/
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		uint16_t	TmpInt;
		char tmp[8];

		postion=6;
		TmpInt=dataSource[postion]+dataSource[postion+1]*256;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Test.StaticPressure,0,sizeof(Ac20003GParams->Ac2000Test.StaticPressure));
		strcat(Ac20003GParams->Ac2000Test.StaticPressure,SysData);
		rt_free(SysData);		
		//rt_kprintf("device Ac2000Test.StaticPressure:%s\n",SysData);
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TEST_STATICPRESSURE_CMD;
		}
	}
}

//static void AC2000ServiceTemperature(unsigned char *dataSource,uint16_t len)
//{
//}
static void AC2000ServiceHumidity(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 10 01 27 
	02 00 00 04 00 00 80 bf 00 00  湿度传感器-内部1
	02 01 00 04 18 f0 9f e5 00 00  湿度传感器-内部2
	03 00 00 04 18 f0 9f e5 00 00  湿度传感器-外部
	6b 
	*/
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		uint16_t	TmpInt;
		float TmpFloat;
		char tmp[8];
	
		postion=6;
		for(uint8_t i=0;i<3;i++)
		{
			TmpInt=dataSource[postion];//湿度传感器类型
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			postion=postion+1;
			TmpInt=dataSource[postion]+1;//编号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			postion=postion+3;
			if(dataSource[postion]==0x18&&dataSource[postion+1]==0xf0&&dataSource[postion+2]==0x9f&&dataSource[postion+3]==0xe5)
			{
				//不详
				strcat(SysData,"---,");
			}
			else
			{
				TmpFloat=FourByteToFloat(&dataSource[postion]);
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
			}
			
			postion=postion+4;
			TmpInt=dataSource[postion]+dataSource[postion+1]*256;//因数
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			postion=postion+2;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Service.Humidity,0,sizeof(Ac20003GParams->Ac2000Service.Humidity));
		strcat(Ac20003GParams->Ac2000Service.Humidity,SysData);
		rt_kprintf("AC2000ServiceHumidity:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=SERVICE_HUMIDITY_ADJUST_CMD;
		}
	}
}
static void AC2000ServiceFeed(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 2F 01 46 
	
	01 E0 00 00 80 3F 01 20 00 00 80 3F 01 08 00 00 80 3F 01 02 00 00 80 3F 0A 	

	*/
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		uint16_t	TmpInt;
		float TmpFloat;
		char tmp[8];
	
		postion=6;
		TmpInt=dataSource[postion];// Method
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion=8;
		TmpFloat=FourByteToFloat(&dataSource[postion])*2.203f;//因数
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion=12;
		TmpInt=dataSource[postion];//Method
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
				
		postion=14;
		TmpFloat=FourByteToFloat(&dataSource[postion])*2.203f;//因数
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Service.Feed,0,sizeof(Ac20003GParams->Ac2000Service.Feed));
		strcat(Ac20003GParams->Ac2000Service.Feed,SysData);
		//rt_kprintf("device Ac2000Service.Feed:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=SERVICE_FEED_ADJUST_CMD;
		}
	}

}
static void AC2000ServiceWater(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 01 30 01 47 
	cd cc cc 3d 
	cd cc cc 3d 
	cd cc cc 3d 
	cd cc cc 3d 
	88 
	
	*/
	{
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		float TmpFloat;
		char tmp[8];
	
		postion=6;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion=postion+4;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion=postion+4;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
				
		postion=postion+4;
		TmpFloat=FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Service.Water,0,sizeof(Ac20003GParams->Ac2000Service.Water));
		strcat(Ac20003GParams->Ac2000Service.Water,SysData);
		//rt_kprintf("device Ac2000Service.Water:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=SERVICE_WATER_ADJUST_CMD;
		}
	}
}
	
//安装
static void AC2000InstallNormalSet(unsigned char *dataSource,uint16_t len)
{
	/*
	01 14 04 90 01 AA 
	00 00 C6 42 56 55 D5 3F 00 00 48 42 00 00 C4 09 
	01       //温度单位：01-法摄氏度F 00-摄氏度C
	00       //重量单位：00—	KG 01—	磅       
	00       //长度单位：00—	公制 01—	非公制
	00       //风机空气容量： 00---立方米/小时 01---CFM
	02       //静压单位：00—	毫巴 01—	英寸水柱 02—	帕斯卡 03—	厘米水柱 04—	毫米水柱 05—	无
	00 
	01       //生长区    01-1   02-2  03-3 04-4 
	01       //最小通风  00-是  01-否
	00       //自然通风  00-否  01-是
	01       //隧道      00-否  01-是
	00 00 00 00 00 01 
	05       // 00-1min 01-5min 02-10min 03-20min 04-30min 05—1H  06-2H  07-3H  08-4H  09-6H  0A-8H  0B-12H  0C-天
	00 00 02 00 01 00 
	2D 
	*/
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		uint16_t	TmpInt;
		int k;
		char tmp[8];
	
		postion=22;
		for(uint8_t k=0;k<5;k++)
		{
			if(k==0)
			{
				TmpInt=dataSource[postion];
				if(TmpInt==1)
				{
						strcat(SysData,"F,");
				}
				else 
				{
						strcat(SysData,"C,");
				}
					postion=postion+1;
			}
			else
			{
				TmpInt=dataSource[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion=postion+1;
			}
		}
		
		postion=postion+1;
		TmpInt=dataSource[postion];
		for(uint8_t k=0;k<4;k++)
		{
			TmpInt=dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion=postion+1;
		}
		
		postion=postion+6;
		TmpInt=dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.NormalSet,0,sizeof(Ac20003GParams->Ac2000Install.NormalSet));
		strcat(Ac20003GParams->Ac2000Install.NormalSet,SysData);
		//rt_kprintf("device Ac2000Install.NormalSet:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_NORMALSET_CMD;
		}
	
}
static void AC2000InstallRelaySetUp(unsigned char *dataSource,uint16_t len)
{
	/*
ff 41 54 54 45 00 00 00 02 00 01 01 04 01 00 00 0c 15 
ff 41 54 54 45 00 00 00 02 00 01 00 02 01 00 00 0c 12 
ff 41 54 54 45 00 00 00 02 00 01 00 02 01 00 00 0c 12 
ff 41 54 54 45 00 00 00 02 00 01 01 04 01 00 00 0c 15 
ff 41 54 54 45 00 00 00 02 00 01 01 04 01 00 00 0c 15 

ff 41 54 54 45 00 00 00 04 00 01 01 1c 01 00 00 04 27 
ff 41 54 54 45 00 00 00 02 00 01 02 1c 01 00 00 f2 14 
ff 41 54 54 45 00 00 00 02 00 01 02 1c 01 f2 00 c6 da 

ff 41 54 54 45 00 00 00 04 00 01 01 79 01 00 00 04 84 
ff 41 54 54 45 00 00 00 04 00 01 01 42 01 00 00 04 4d 
ff 41 54 54 45 00 00 00 02 00 01 01 42 01 00 00 18 5f 
ff 41 54 54 45 00 00 00 02 00 01 02 69 01 00 00 1a 89 
ff 41 54 54 45 00 00 00 02 00 01 01 5c 01 00 00 04 65




01 14 01 04 01 1b 4e 88 57 42 d1 89 70 42 d1 89 70 42 87 
01 14 00 02 01 18 1f 05 70 02 70 04 70 05 70 06 03 01 f9 
01 14 00 02 01 18 1f 05 70 02 70 04 70 05 70 06 03 01 f9 
01 14 01 04 01 1b 4e 88 57 42 d1 89 70 42 93 9e 70 42 5e 
01 14 01 04 01 1b 4e 88 57 42 d1 89 70 42 93 9e 70 42 5e
 
01 14 00 1c 01 32 b8 01 00 00 b9 

01 14 02 1c 01 34 01 00 00 04 00 00 80 bf 00 00 80 bf 00 00 00 00 00 00 00 00 00 00 01 01 00 04 00 00 80 bf 00 00 80 bf 01 00 00 00 00 00 00 00 00 00 01 02 00 04 00 00 80 bf 00 00 80 bf 02 00 00 00 00 00 00 00 00 00 01 03 00 04 00 00 80 bf 00 00 80 bf 03 00 00 00 00 00 00 00 00 00 01 04 00 04 00 00 80 bf 00 00 80 bf 04 00 00 00 00 00 00 00 00 00 01 05 00 04 00 00 80 bf 00 00 80 bf 05 00 00 00 00 00 00 00 00 00 09 00 00 02 00 00 80 bf 00 00 80 bf 00 00 01 00 00 00 00 00 00 00 09 01 00 02 00 00 80 bf 00 00 80 bf 01 00 01 00 00 00 00 00 00 00 2a 00 00 03 00 00 00 00 00 00 20 41 00 00 01 00 00 00 00 00 00 00 2c 00 00 03 00 00 00 00 00 00 20 41 01 00 01 00 00 00 00 00 00 00 01 00 00 01 00 00 80 bf 00 00 80 bf 00 00 02 00 00 00 00 00 01 01 eb 

01 14 02 1c 01 34 05 00 00 01 00 00 80 bf 00 00 80 bf 01 00 02 00 00 00 00 00 01 00 05 01 00 01 00 00 80 bf 00 00 80 bf 02 00 02 00 00 00 00 00 01 00 06 00 00 01 00 00 80 bf 00 00 80 bf 03 00 02 00 00 00 00 00 01 00 09 00 00 01 00 00 80 bf 00 00 80 bf 04 00 02 00 00 00 00 00 01 00 0a 00 00 01 00 00 80 bf 00 00 80 bf 05 00 02 00 00 00 00 00 01 00 0d 00 00 01 00 00 80 bf 00 00 80 bf 06 00 02 00 00 00 00 00 01 00 19 00 00 01 00 00 80 bf 00 00 80 bf 07 00 02 00 00 00 00 00 01 00 17 00 00 01 00 00 80 bf 00 00 80 bf 08 00 02 00 00 00 00 00 01 00 18 00 00 01 00 00 80 bf 00 00 80 bf 09 00 02 00 00 00 00 00 01 00 38

01 14 00 79 01 8f ce 00 00 00 ce 
01 14 00 42 01 58 18 00 00 00 18 
01 14 01 42 01 59 05 00 00 01 00 bc 84 47 05 01 00 01 00 bc 84 47 06 00 00 01 00 bc 04 47 29
01 14 02 69 01 81 e4 38 5e c1 1d c7 b1 c0 00 ff ff 00 3c 00 00 1e 1e 0a 01 00 64 00 00 0a 00 00 7f 
01 14 01 5c 01 73 10 03 0d 06 26

	*/
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		char tmp[32];
	
		postion=10;
		//TmpInt=dataSource[postion]+dataSource[postion+1]*256;
	  memset(tmp,0,sizeof(tmp));
		for(uint8_t i=0;i<20;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,%02X,%d,%d,",(i+1),dataSource[postion],(i+1),0);
			strcat(SysData,tmp);
			postion=postion+1;
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.RelaySetUp,0,sizeof(Ac20003GParams->Ac2000Install.RelaySetUp));
		strcat(Ac20003GParams->Ac2000Install.RelaySetUp,SysData);
		rt_kprintf("device Ac2000Install.NormalSet:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_RELAYSETUP_CMD;
		}
		
}
static void AC2000InstallAnalogSensor(unsigned char *dataSource,uint16_t len)
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
		if(LevelsOfVentilationData==NULL) 
		{
			LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
			if(LevelsOfVentilationData!=NULL)
			{
				rt_memset(LevelsOfVentilationData,0,256*20);
				LevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("AnalogSensor malloc failure\n");
			}
		}
		if(LevelsOfVentilationDataLength<=256*19)
		{
			rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
			LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
		}
		else
		{
			rt_memset(LevelsOfVentilationData,0,256*20);
			LevelsOfVentilationDataLength=0;
		}

	}
	else if(len>6)
		{
			if(LevelsOfVentilationData==NULL) 
			{
				LevelsOfVentilationData=(unsigned char *)rt_malloc(256*20);
				if(LevelsOfVentilationData!=NULL)
				{
					rt_memset(LevelsOfVentilationData,0,256*20);
					LevelsOfVentilationDataLength=0;
				}
				else
				{
					rt_kprintf("AnalogSensor malloc failure\n");
				}
			}
			if(LevelsOfVentilationDataLength<=256*19)
			{
				rt_memcpy(&LevelsOfVentilationData[LevelsOfVentilationDataLength],&dataSource[6],len-7);
				LevelsOfVentilationDataLength=LevelsOfVentilationDataLength+(len-7);
			}
			else
			{
				rt_memset(LevelsOfVentilationData,0,256*20);
				LevelsOfVentilationDataLength=0;
			}
		

	
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData1=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData2=(char *)rt_malloc(AliyunServiceSysLength);
			char *SysData3=(char *)rt_malloc(AliyunServiceSysLength);
		  rt_memset(SysData,0,AliyunServiceSysLength);
			rt_memset(SysData1,0,AliyunServiceSysLength);
			rt_memset(SysData2,0,AliyunServiceSysLength);
			rt_memset(SysData3,0,AliyunServiceSysLength);
			unsigned char lightSum=0;
			unsigned char WaterandFeed=0;
			unsigned int postion;
			float	TmpFloat;
			int	TmpInt;
			char tmp[8];
			postion=0;
			for(int i=0;i<9;i++)
			{
				if(postion<(LevelsOfVentilationDataLength-22)) 
				{
					if(LevelsOfVentilationData[postion+3]==0x04)
					{
						TmpInt=LevelsOfVentilationData[postion];//传感器类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion=postion+1;						
		      	
						TmpInt=LevelsOfVentilationData[postion]+1;//器械编号
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData,tmp);
						postion=postion+21;						
					}
					else
					{
						break;						
					}
				}
				else
				{
					break;
				}
			}
			if(Ac20003GParams->Ac2000Install.AnalogSensorAndOutput==1)
			{
				
			Ac20003GParams->Ac2000Install.AnalogSensorAndOutput=0;
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac20003GParams->Ac2000Install.AnalogSensor,0,sizeof(Ac20003GParams->Ac2000Install.AnalogSensor));
			strcat(Ac20003GParams->Ac2000Install.AnalogSensor,SysData);
			rt_kprintf("Ac2000SInstall.AnalogSensor:%s\n",Ac20003GParams->Ac2000Install.AnalogSensor);
			
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=INSTALL_ANALOGSENSOR_CMD;
			}
		 }
			rt_free(SysData);	

     for(int i=0;i<2;i++)//数字传感器
			{

				if(postion<(LevelsOfVentilationDataLength-22)) 
				{
					if(LevelsOfVentilationData[postion+3]==0x02)
					{

						TmpInt=LevelsOfVentilationData[postion];//数据
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData1,tmp);
	
					  postion=postion+1;//器械
						TmpInt=LevelsOfVentilationData[postion]+1;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData1,tmp);	
					  postion=postion+21;
					}
					else
					{
						//postion=postion+22;	
						break;
					}
				}
				else
				{
					break;
				}
			}
			if(Ac20003GParams->Ac2000Install.AnalogSensorAndOutput==2)
			{
				Ac20003GParams->Ac2000Install.AnalogSensorAndOutput=0;
				if(SysData1!=NULL) SysData1[strlen(SysData1)-1]=0x00;
				rt_memset(Ac20003GParams->Ac2000Install.DigitalSensor,0,sizeof(Ac20003GParams->Ac2000Install.DigitalSensor));
				strcat(Ac20003GParams->Ac2000Install.DigitalSensor,SysData1);
				rt_kprintf("Ac2000SInstall.DigitalSensor:%s\n",SysData1);
					
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=INSTALL_DIGITALSENSOR_CMD;
			}
			}
			
			rt_free(SysData1);
	
			for(int i=0;i<2;i++)
			{
				if(postion<(LevelsOfVentilationDataLength-22)) 
				{
					if(LevelsOfVentilationData[postion+3]==0x03)
					{
						TmpInt=LevelsOfVentilationData[postion];//类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData2,tmp);
	
					  postion=postion+1;//编号
						TmpInt=LevelsOfVentilationData[postion]+1;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData2,tmp);
						
						postion=postion+3;//最小电压-输出
						TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%0.1f,",TmpFloat);
						strcat(SysData2,tmp);

						postion=postion+4;//最高电压-输出
						TmpFloat=FourByteToFloat(&LevelsOfVentilationData[postion]);
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%0.1f,",TmpFloat);
						strcat(SysData2,tmp);
	
					  postion=postion+14;
					}
					else
					{
						//postion=postion+22;		
						break;
					}
				}
				else
				{
					break;
				}
			}

			if(Ac20003GParams->Ac2000Install.AnalogSensorAndOutput==3)
			{
				Ac20003GParams->Ac2000Install.AnalogSensorAndOutput=0;
			if(SysData2!=NULL) SysData2[strlen(SysData2)-1]=0x00;
			rt_memset(Ac20003GParams->Ac2000Install.AnalogOutput,0,sizeof(Ac20003GParams->Ac2000Install.AnalogOutput));
			strcat(Ac20003GParams->Ac2000Install.AnalogOutput,SysData2);
			rt_kprintf("Ac2000SInstall.AnalogOutput:%s\n",SysData2);
				
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=INSTALL_ANALOGOUTPUT_CMD;
			}
			}
			
			rt_free(SysData2);	
		
			for(int i=0;i<20;i++)
			{
				if(postion<=(LevelsOfVentilationDataLength-22)) 
				{
					if(LevelsOfVentilationData[postion+3]==0x01)
					{
						
						memset(tmp,0,sizeof(tmp));//继电器号
						sprintf(tmp,"%d,",i+1);
						strcat(SysData3,tmp);
						
						TmpInt=LevelsOfVentilationData[postion];//类型
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						
						if((TmpInt==0x18)||(TmpInt==0x17)||(TmpInt==0x19))
						{
							WaterandFeed=WaterandFeed+1;
						}
						if(TmpInt==0x16)
						{
							
							lightSum=lightSum+1;
						}
	
					  postion=postion+1;//器械
						TmpInt=LevelsOfVentilationData[postion]+1;
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						postion=postion+20;
						
						TmpInt=LevelsOfVentilationData[postion];
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",TmpInt);
						strcat(SysData3,tmp);
						postion=postion+1;
					}
					else
					{
						//postion=postion+22;		
						break;
					}
				}
				else
				{
					break;
				}
			
			
			}
			if(ACCESS_DIFERENT3g==1)
				Number_relaystate3g = ((lightSum+1)*6)*50;
			else if(ACCESS_DIFERENT3g==2)
			Number_relaystate3g = ((WaterandFeed+1)*6)*50;
			
			rt_kprintf("WATER%d Number%d",WaterandFeed,Number_relaystate3g);
			if(Ac20003GParams->Ac2000Install.AnalogSensorAndOutput==4)
			{
				Ac20003GParams->Ac2000Install.AnalogSensorAndOutput=0;
			if(SysData3!=NULL) SysData3[strlen(SysData3)-1]=0x00;
			rt_memset(Ac20003GParams->Ac2000Install.RelaySetUp,0,sizeof(Ac20003GParams->Ac2000Install.RelaySetUp));
			strcat(Ac20003GParams->Ac2000Install.RelaySetUp,SysData3);
			rt_kprintf("Ac2000SInstall.AnalogOutput:%s\n",SysData3);
				
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=INSTALL_RELAYSETUP_CMD;
			}
			}
			
			rt_free(SysData3);	
			
		LevelsOfVentilationDataLength=0;
		if(LevelsOfVentilationData!=NULL) 
		{
			rt_free(LevelsOfVentilationData);
			LevelsOfVentilationData=NULL;
		}	
	}

}

 void AC2000InstallVentCurtainSetUp(unsigned char *dataSource,uint16_t len)
{

	/*
	01 14 02 36 01 4e 
	0f 00 00 01 00 ff ff 00 3c 00 3c 00 00 00 
	0f 01 00 01 00 ff ff 00 3c 00 3c 00 00 00 
	0f 02 00 01 00 ff ff 00 3c 00 3c 00 00 00 
	95 
	*/
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		unsigned int TmpInt;
		char tmp[8];
	  postion=6;
	  unsigned char sum;
		sum = (len-6)/14;
		for(uint8_t j=0;j<sum;j++)
		{
				TmpInt=dataSource[postion]+1;//风帘号,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d--",TmpInt);
				strcat(SysData,tmp);
				postion =postion+1;
				TmpInt=dataSource[postion]+1;//风帘号,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion = postion+4;
			if(dataSource[postion]==0xFF&&dataSource[postion+1]==0xFF)
			{
				strcat(SysData,"N,");
				
			}
			else
			{
				strcat(SysData,"Y,");
			}
			
				postion=postion+3;				
				TmpInt=dataSource[postion]+dataSource[postion+1]*256;//打开,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				
				postion=postion+2;	
				TmpInt=dataSource[postion]+dataSource[postion+1]*256;//关闭,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion=postion+4;
				//rt_kprintf("1:%s\n",tmp);
		
		}

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.VentCurtainSetUp,0,sizeof(Ac20003GParams->Ac2000Install.VentCurtainSetUp));
		strcat(Ac20003GParams->Ac2000Install.VentCurtainSetUp,SysData);
		//rt_kprintf("device Ac2000SInstall.VentCurtainSetUp:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_VENTCURTAINSETUP_CMD;
		}
}

static void AC2000InstallTemperatureDefine(unsigned char *dataSource,uint16_t len)
{
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		char tmp[8];
		int mode=0;
		mode =(len-6)/10;
		
		postion=6;
		for(uint8_t j=0;j<mode;j++)
		{
			
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%02X%02X%02X%02X,",dataSource[postion],dataSource[postion+1],dataSource[postion+2],dataSource[postion+3]);
				strcat(SysData,tmp);
				postion=postion+4;
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%01d,%01d,%01d,%01d,%01d,%01d,",dataSource[postion],dataSource[postion+1],dataSource[postion+2],dataSource[postion+3],dataSource[postion+4],dataSource[postion+5]);
				strcat(SysData,tmp);
				postion=postion+6;
			
	
		}

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.TemperatureDefine,0,sizeof(Ac20003GParams->Ac2000Install.TemperatureDefine));
		strcat(Ac20003GParams->Ac2000Install.TemperatureDefine,SysData);
		//rt_kprintf("device Ac2000SInstall.TemperatureDefine:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_TEMPERATUREDEFINE_CMD;
		}	
	
}
static void AC2000InstallCapacityOfFanAir(unsigned char *dataSource,uint16_t len)
{
/*
	02 14 01 42 01 5a 
	05 00 00 01 00 bc 84 47 
	06 00 00 01 00 bc 04 47 
	9b



		01 14 01 42 01 59
		
		05 00 00 01    00 00 40 40 单位是立方英尺的时候等于浮点数
		06 00 00 01    00 00 40 40 单位是立方英尺的时候等于浮点数
		
		1a
	*/
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		unsigned int postion;
		unsigned int TmpInt;
		float	TmpFloat;
		char tmp[8];
		int mode=0;
		postion=6;
		mode = (len-6)/8;
		for(uint8_t j=0;j<mode;j++)
		{
			
				TmpInt=dataSource[postion];//风机类型,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion=postion+1;
				//rt_kprintf("1:%s\n",tmp);
				
				TmpInt=dataSource[postion]+1;//风机号,
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion=postion+3;
				//rt_kprintf("2:%s\n",tmp);
				TmpFloat=FourByteToFloat(&dataSource[postion])/1.699f;//立方英尺
				memset(tmp,0,sizeof(tmp));
				//rt_kprintf("CapacityOfFanAirUint:%d\n",Ac20003GParams->Ac2000Install.CapacityOfFanAirUint);
				if(Ac20003GParams->Ac2000Install.CapacityOfFanAirUint==0x5709)//立方英尺
				{
					sprintf(tmp,"%0.1f,",TmpFloat);
				}
				else if(Ac20003GParams->Ac2000Install.CapacityOfFanAirUint==0x5b9d)//立方米/小时
				{
					sprintf(tmp,"%0.1f,",TmpFloat+2);
				}	
				else
				{
					sprintf(tmp,"%0.1f,",TmpFloat);
				}	
				strcat(SysData,tmp);
				postion=postion+4;
		
			}
	

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac20003GParams->Ac2000Install.CapacityOfFanAir,0,sizeof(Ac20003GParams->Ac2000Install.CapacityOfFanAir));
		strcat(Ac20003GParams->Ac2000Install.CapacityOfFanAir,SysData);
		rt_kprintf("device Ac2000SInstall.CapacityOfFanAir:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_CAPACITYOFFANAIR_CMD;
		}
	}


static void AC2000InstallHenSize(unsigned char *dataSource,uint16_t len)
{
/*
	01 14 01 43 01 5A 
	00 00 60 40 鸡舍长高度
	00 00 20 40 鸡舍短高度
	00 00 60 41 鸡舍宽度 
	00 00 28 42 鸡舍面积 
	0B
*/
	if(len==23)	
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
		rt_memset(Ac20003GParams->Ac2000Install.HenSize,0,sizeof(Ac20003GParams->Ac2000Install.HenSize));
		strcat(Ac20003GParams->Ac2000Install.HenSize,SysData);
		rt_kprintf("device Ac2000SInstall.HenSize:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_HENSIZE_CMD;
		}
	}
}
static void AC2000InstallCommunication(unsigned char *dataSource,uint16_t len)
{
	/*
	ff 41 54 54 45 00 00 00 02 00 01 01 17 01 00 00 02 1e 
	01 14 01 17 01 2e 
	04    00-1200 01-2400 02-4800 03-9600 04-19200 05-38400 06-57600 07-115200
	01    鸡舍标号
	05 
	*/
	if(len==9)	
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
		rt_memset(Ac20003GParams->Ac2000Install.Communication,0,sizeof(Ac20003GParams->Ac2000Install.Communication));
		strcat(Ac20003GParams->Ac2000Install.Communication,SysData);
		//rt_kprintf("device Ac2000SInstall.Communication:%s\n",SysData);
		rt_free(SysData);		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALL_COMMUNICATION_CMD;
		}
	}
}

int Ac20003gDataParseCache(uint8_t *data,unsigned short Counter)
{
	uint8_t result=0;
	uint16_t cmd_code=data[2]*256+data[3];
	rt_kprintf("recv 3g normal data:  %04X  %d\r\n\r\n",cmd_code,Counter);
	if(Ac2000Data!=NULL)
	{
		switch (cmd_code)
		{
			case (0x0000):
			{
				if(Counter==8)
				{
					ac2000_table.ac2000_3g_house=data[0];
				}
				else
				{
				//	my_log(data,Counter);
				}
			}break;
			case (0x0479) ://主屏
			{
				if(Counter==87)Parse_MainScreen(data,Counter);
			}break;
			case (0x0002) ://报警查询
			{
				if(Counter==19)Parse_Warn(data,Counter);
			}break;
			case (0x0113) :
			{
				if(Counter==67)Parse_AskTmpature(data,Counter);//
			}break;
			case (0x034B) :
			{
				if(Counter==40)Parse_FeedWater(data,Counter);//水线
			}break;
			case (0x0148):
			{
				if(Counter==254)
				{
					Parse_RelyTest1(data,Counter);//15*6
				}
				else if(Counter==140)
				{
					Parse_RelyTest2(data,Counter);//测试开关继电器
				}
				else
				{
					//my_log(data,Counter);
				}
			}break;
			case (0x0152):
			{
				if(Counter==33)Parse_curve_state(data,Counter);
			}break;
			case (0x0150):
			{
				if(Counter==33)Parse_humiture(data,Counter);
			}break;
			case (0x021D)://温度曲线
			{
				if(Counter==231)
				{
					temperature_curve1_2(data,Counter);//15*6
				}
				else if(Counter==39)
				{
					temperature_curve3(data,Counter);
				}
				else
				{
				//	my_log(data,Counter);
				}
			}break;
			case (0x0122):
			{
				if(Counter==175)humidity_treatm_function(data,Counter);//20*4 湿度处理
			}break;
			case (0x011F):// 控制 -最低高级级别
			{
				if(Counter==73)
				{
					Min_max_level(data,Counter);
				}
				else
				{
			//		my_log(data,Counter);
				}
			}break;
			case (0x012A):
			{
				if(Counter==55)static_pressure_function(data,Counter);//20*4
			}break;
			case (0x022B)://控制模式
			{
				if(Counter==16)Control_mode(data,Counter);//20*4
			}break;
			case (0x0192)://co2处理
			{
				if(Counter==217)Co2_Handle(data,Counter);//20*4
			}break;
			case (0x0165):
			{
				if(Counter==35)system_inforamation_tmperature(data,Counter);//系统参数 温度曲线
			}break;
			case (0x0162):
			{
				if(Counter==19)System_inforamation_cycle_heater(data,Counter);//系统参数 CYCLE HEATER
			}break;
			case (0x4166):
			{
				if(Counter==17)System_inforamation_radiant_heater(data,Counter);//系统参数 辐射加热器
			}break;
			case (0x03a6):
			{
				if(Counter==25)System_inforamation_variable_heater(data,Counter);//VARIABLE HEATER
			}break;
			case (0x026c):
			{
				if(Counter==14)System_inforamation_humidity_treatment(data,Counter);//湿度处理
			}break;
			case (0x0286):
			{
				if(Counter==13)System_inforamation_C02_treatment(data,Counter);//CO2处理
			}break;
			case (0x0269):
			{
				if(Counter==33)System_inforamation_static_pressure(data,Counter);//静压
			}break;
			case (0x0167):
			{
				if(Counter==27)System_inforamation_ventilation_level(data,Counter);///通风级别 
			}break;
			case (0x036e):
			{
				if(Counter==43)System_inforamation_rolling_shutter_and_roof(data,Counter);//卷帘和屋顶
			}break;
			case (0x01F9):
			{
				if(Counter==18)System_inforamation_curtain_calibration(data,Counter);//VENT CURTAIN CALIBRATION
			}break;
			case (0x026a):
			{
				if(Counter==27)System_inforamation_wet_curtain_cool_pad_flush(data,Counter);//湿帘 COOL PAD FLUSH
			}break;
			case (0x026b):
			{
				if(Counter==23)System_inforamation_Humidifier(data,Counter);//加湿器
			}break;
			case (0x016f):
			{
				if(Counter==23)System_inforamation_lighting(data,Counter);//灯光
			}break;
			case (0x0187):
			{
				if(Counter == 27)system_inforamation_food_water(data,Counter);//水和饲料
			}break;
			case (0x0180):
			{
				if(Counter == 31) system_inforamation_food_left(data,Counter); //饲料存量
			}break;
			case (0x0172):
			{
				if(Counter==8)System_inforamation_alarm_set(data,Counter);//报警设置
			}break;
			case (0x0170):
			{
				if(Counter==8) system_inforamation_food_leftli(data,Counter);//
			}break;
			//设备
			case (0x0299):
			{
					Device_LevelsOfVentilationLast(data,Counter);//设备--通风级别
				}break;
			case (0x0044):
			{
				Length_acquisition3g(data,Counter);//设备--调速风扇级别
			}break;
			case (0x0244):
			{
				Device_VariableSpeedFanLevels(data,Counter);//设备--调速风扇级别
			}break;
			case (0x0037):
			{
				if(Counter==11)Length_acquisition3g(data,Counter);//设备--通风和风帘级别
			}break;
			case (0x0137):
			{
				Device_VentAndCurtianLevels(data,Counter);//设备--通风和风帘级别
			}break;
			case (0x001e):
			{
				if(Counter==11)Length_acquisition3g(data,Counter);//设备--开启搅拌个数
			}break;
			case (0x021e):
			{
				Device_StirFanLevels(data,Counter);//设备--搅拌状态
			}break;
			case (0x013d):
			{
				if(Counter==22)Device_StirFanProgram(data,Counter);//设备--搅拌程序
			}break;
			case (0x0125):
			{
				Device_Foggers(data,Counter);	//设备--喷雾器加湿器
			}break;	
			case (0x0124):
			{
				Device_CoolPad(data,Counter);//设备--制冷板 湿帘
			}break;
			case (0x01cf):
			{
				Device_Light(data,Counter);//设备--灯光
			}break;
			case(0x01d0):
			{
				Device_WaterAndFeed(data,Counter);//设备--水和饲料
			}break;
			case (0x4128):
			{
				if(Counter==87)Device_ExtraSystem(data,Counter);//设备--附加系统
			}break;	
			case (0x00A5):
			{
				Length_acquisition3g(data,Counter);
			}break;
			case (0x03a5):
			{
				Device_WaterOnDemand(data,Counter);//设备--water on demand
			}break;
			
			//管理
			case (0x010C):
			{
				if(Counter==43)ManageStockOfChicken(data,Counter);//管理 --鸡的存量
			}break;
			
			case(0x0261):
			{
					ManageStockOfFeed(data,Counter);//管理--饲料
			}break;
			
			case (0x015d): 
			{
//				if(Counter==11)ManageTimeAndDate1(data,Counter); //管理--时间和日期
				if(Counter==11)SystemTime(data,Counter); //管理--时间和日期
			}break;
			case (0x015c):
			{
				if(Counter==11)ManageTimeAndDate2(data,Counter);//管理--时间和日期
			}break;
			case (0x010D):
			{
				if(Counter==11)ManageGrowthAndHen(data,Counter);//管理--生长期和鸡群
			}break;
			case (0x023B):
			{
				ManageAlarmSet(data,Counter);//管理警报设置
			}break;
			case(0x0112)://管理--密码重置
			{
				if(Counter == 21)ManagePasswordSet(data,Counter);
			}break;
			
			
            //鸡磅
			case(0x013A)://基本设置 根鸡磅的基本设置是同一数据帧
			{
				if(Counter == 21) ChickBasicSetting(data,Counter);
			}break;
			case(0x012C):
			{
				if(Counter == 249) ChickLayout(data,Counter); //鸡磅布局
				else if((Counter == 241)||(Counter == 85)) ChickCurse(data,Counter);//鸡磅曲线
			}break;
			case(0x0160):
			{
				if(Counter == 259) Chickhistory(data,Counter);
			}break;
			case(0x0193):
			{
				if(Counter == 77) ChickConversion(data,Counter); //鸡磅转换
			}break;
			case(0x015A):
			{
				if(Counter == 21) ChickTest(data,Counter);//鸡磅测试
			}break;
			case(0x0111):
			{
				if(Counter == 29) Chickcalibration(data,Counter);
			}break;
		
			//历史
			case(0x0153): //历史温度
			{
				if(Counter == 57) AC2000HistoryTemperature(data,Counter);
			
			}break;
			case(0x0154)://历史湿度
			{
				if(Counter == 67)  AC2000HistoryHumidity_treatm(data,Counter);
			}break;
			case(0x0194):
			{
				if(Counter == 67)  AC2000HistoryCO2(data,Counter);//历史Co2
			}break;
			case(0x0155):
			{
				AC2000HistoryFeed(data,Counter);//历史饲料
			}break;
			case(0x017E):
			{
				AC2000HistoryWater(data,Counter);//历史水
			}break;
			case(0x0158):
			{
				AC2000HistoryDead(data,Counter);//历史死亡
			}break;
			case(0x0159):
			{
				AC2000HistoryHeater(data,Counter);//历史加热器
			}break;
			case(0x0147):
			{
				AC2000HistoryRadiant_Heater(data,Counter);//历史辐射加热器
			}break;
			case(0x0157):
			{
				AC2000HistoryAlarm(data,Counter);
			}break;
			case(0x015B):
			{
				AC2000HistoryEvent_Map(data,Counter);
			}break;
			case(0x01AF):
			{
				AC2000HistoryWatch(data,Counter);
			}break;
			case(0x0145):
			{
				AC2000HistorySet(data,Counter);
			}break;
			case(0x014A):
			{
				//if(Counter==19) AC2000ReciverFlog(data,Counter,Types);
			}
			//测试
			case (0x007a):
			{
				AC2000TestAnalogSensor(data,Counter);
			}break;
			case (0x027a):
			{
				AC2000TestAnalogSensor(data,Counter);
			}break;
			case (0x007b):
			{
				AC2000TestDigitalSensor(data,Counter);
			}break;
			case (0x027b):
			{
				AC2000TestDigitalSensor(data,Counter);
			}break;
			case (0x017c)://静压
			{
				if(Counter==9)AC2000TestStaticPressure(data,Counter);
			}break;
			
			//服务
			//温度在主屏解析
			
			case (0x0110)://湿度 
			{
				if(Counter==37)AC2000ServiceHumidity(data,Counter);
			}break;
			case (0x012f)://feed 
			{
				if(Counter==31)AC2000ServiceFeed(data,Counter);
			}break;
			case (0x0130)://water 
			{
				if(Counter==23)AC2000ServiceWater(data,Counter);
			}break;
			
			//安装
			case (0x0490)://set 
			{ 
				if(Counter==46)AC2000InstallNormalSet(data,Counter);
			}break;
	//		case (0x0104): 
	//		{	
	//			AC2000InstallRelaySetUp(data,Counter);
	//		}break;
			case (0x001C):
			{
				Length_acquisition3g(data,Counter);
			}break;
			case (0x021c):// 
			{
					AC2000InstallAnalogSensor(data,Counter);
			}break;
			case (0x0036):// 
			{
				Length_acquisition3g(data,Counter);
			}break;		
			case (0x0236):// 
			{
				AC2000InstallVentCurtainSetUp(data,Counter);
			}break;	
			case (0x0035):// 
			{
				Length_acquisition3g(data,Counter);	
			}break;	
			case (0x0135):// 温度曲线
			{
				AC2000InstallTemperatureDefine(data,Counter);	
			}break;			
			case (0x0042):
			{
				Length_acquisition3g(data,Counter);
			}break;
			case (0x0142): 
			{
				AC2000InstallCapacityOfFanAir(data,Counter);
			}break;	
			case (0x0143):
			{
				AC2000InstallHenSize(data,Counter);
			}break;
			case (0x0117):
			{
				AC2000InstallCommunication(data,Counter);
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


int Ac20003gDataParseMultiCache(uint8_t *data,unsigned short len)
{
	int result=0;;
	uint16_t DataIndex[100][2];
	uint16_t DataCount=0;
	//rt_kprintf("Ac20003gDataParseMultiCache:%d\r\n",len);
	for(uint16_t i=0;i<(len-1);i++)
	{
		if(data[i]==ac2000_table.ac2000_3g_house&&data[i+1]==0x14)
		{
			if(i==0)
			{
				DataIndex[DataCount][0]=i;
			}
			else
			{
				DataIndex[DataCount][0]=i;   // 首
				DataIndex[DataCount-1][1]=i-1;//尾
			}
			DataCount++;
		}
	}
	DataIndex[DataCount-1][1]=len-1;
	for(uint8_t i=0;i<DataCount;i++)
	{
		int length=DataIndex[i][1]-DataIndex[i][0]+1;
		if(Ac20003gDataParseCache(&data[DataIndex[i][0]],length)==1)
		{
			result=result+1;
		}
	}
	DataCount=0;
	return result;
}
