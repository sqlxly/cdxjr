#include <rtthread.h>
#include <rtdevice.h>
//#include "malloc.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "data_table.h"
#include "humiture.h"
#include "mynet.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ac2000_plus.h"

extern int user_usart_pass(rt_uint8_t *send_buf, int send_len, int read_bufsz);
PlusAc2000Params *Ac2000PlusParams=NULL;
unsigned char *PLUSLevelsOfVentilationData=NULL;
uint16_t PLUSLevelsOfVentilationDataLength=0;

int PlusRecvFlag;
/*温度值表*/
const float tableTemperature[976]={
100.0, 99.6, 99.2, 98.9, 98.5, 98.1, 97.8, 97.4, 97.0,96.7,96.3, 96.0, 95.6, 95.3, 94.9, 94.6, 94.2, 93.9, 93.5, 93.2, 
92.9, 92.5, 92.2, 91.9, 91.5, 91.2, 90.9, 90.5, 90.2, 89.9, 89.6, 89.3, 88.9, 88.6, 88.3, 88.0, 87.7, 87.4, 87.1, 86.7,
86.4, 86.1, 85.8, 85.5, 85.2, 84.9, 84.6, 84.3, 84.1, 83.8, 83.5, 83.2, 82.9, 82.6, 82.3, 82.0, 81.8, 81.5, 81.2, 80.9, 
80.6, 80.4, 80.1, 79.8, 79.6, 79.3, 79.0, 78.7, 78.5, 78.2, 78.0, 77.7, 77.4, 77.2, 76.9, 76.7, 76.4, 76.2, 75.9, 75.6, 
75.4, 75.1, 74.9, 74.7, 74.4, 74.2, 73.9, 73.7, 73.4, 73.2, 73.0, 72.7, 72.5, 72.3, 72.0, 71.8, 71.6, 71.3, 71.1, 70.9, 
70.6, 70.4, 70.2, 70.0, 69.7, 69.5, 69.3, 69.1, 68.9, 68.6, 68.4, 68.2, 68.0, 67.8, 67.6, 67.4, 67.1, 66.9, 66.7, 66.5, 
66.3, 66.1, 65.9, 65.7, 65.5, 65.3, 65.1, 64.9, 64.7, 64.5, 64.3, 64.1, 63.9, 63.7, 63.5, 63.3, 63.1, 62.9, 62.7, 62.6, 
62.4, 62.2, 62.0, 61.8, 61.6, 61.4, 61.3, 61.1, 60.9, 60.7, 60.5, 60.3, 60.2, 60.0, 59.8, 59.6, 59.5, 59.3, 59.1, 58.9,
58.8, 58.6, 58.4, 58.2, 58.1, 57.9, 57.7, 57.6, 57.4, 57.2, 57.1, 56.9, 56.7, 56.6, 56.4, 56.2, 56.1, 55.9, 55.8, 55.6, 
55.4, 55.3, 55.1, 55.0, 54.8, 54.7, 54.5, 54.3, 54.2, 54.0, 53.9, 53.7, 53.6, 53.4, 53.3, 53.1, 53.0, 52.8, 52.7, 52.5,
	
52.4, 52.2, 52.1, 51.9, 51.8, 51.6, 51.5, 51.4, 51.2, 51.1, 50.9, 50.8, 50.6, 50.5, 50.4, 50.2, 50.1, 49.9, 49.8, 49.7, 
49.5, 49.4, 49.3, 49.1, 49.0, 48.8, 48.7, 48.6, 48.4, 48.3, 48.2, 48.0, 47.9, 47.8, 47.7, 47.5, 47.4, 47.3, 47.1, 47.0, 
46.9, 46.7, 46.6, 46.5, 46.4, 46.2, 46.1, 46.0, 45.9, 45.7, 45.6, 45.5, 45.4, 45.2, 45.1, 45.0, 44.9, 44.7, 44.6, 44.5,
44.4, 44.3, 44.1, 44.0, 43.9, 43.8, 43.7, 43.5, 43.4, 43.3, 43.2, 43.1, 42.9, 42.8, 42.7, 42.6, 42.5, 42.4, 42.2, 42.1, 
42.0, 41.9, 41.8, 41.7, 41.6, 41.4, 41.3, 41.2, 41.1, 41.0, 40.9, 40.8, 40.7, 40.5, 40.4, 40.3, 40.2, 40.1, 40.0, 39.9, 
39.8, 39.7, 39.6, 39.4, 39.3, 39.2, 39.1, 39.0, 38.9, 38.8, 38.7, 38.6, 38.5, 38.4, 38.3, 38.2, 38.1, 37.9, 37.8, 37.7, 
37.6, 37.5, 37.4, 37.3, 37.2, 37.1, 37.0, 36.9, 36.8, 36.7, 36.6, 36.5, 36.4, 36.3, 36.2, 36.1, 36.0, 35.9, 35.8, 35.7, 
35.6, 35.5, 35.4, 35.3, 35.2, 35.1, 35.0, 34.9, 34.8, 34.7, 34.6, 34.5, 34.4, 34.3, 34.2, 34.1, 34.0, 33.9, 33.8, 33.7, 
33.6, 33.5, 33.4, 33.3, 33.2, 33.1, 33.0, 32.9, 32.8, 32.7, 32.6, 32.5, 32.4, 32.3, 32.2, 32.1, 32.0, 31.9, 31.8, 31.7, 
31.6, 31.5, 31.4, 31.3, 31.2, 31.2, 31.1, 31.0, 30.9, 30.8, 30.7, 30.6, 30.5, 30.4, 30.3, 30.2, 30.1, 30.0, 29.9, 29.8, 

29.7, 29.6, 29.5, 29.5, 29.4, 29.3, 29.2, 29.1, 29.0, 28.9, 28.8, 28.8, 28.7, 28.6, 28.5, 28.4, 28.3, 28.3, 28.2, 28.1, 
28.0, 27.9, 27.8, 27.7, 27.6, 27.5, 27.4, 27.3, 27.2, 27.2, 27.1, 27.0, 26.9, 26.8, 26.7, 26.6, 26.5, 26.4, 26.3, 26.2, 
26.2, 26.1, 26.0, 25.9, 25.8, 25.7, 25.6, 25.5, 25.4, 25.3, 25.3, 25.2, 25.1, 25.0, 24.9, 24.8, 24.7, 24.6, 24.5, 24.4, 
24.3, 24.3, 24.2, 24.1, 24.0, 23.9, 23.8, 23.7, 23.6, 23.5, 23.4, 23.3, 23.3, 23.2, 23.1, 23.0, 22.9, 22.8, 22.7, 22.6, 
22.5, 22.4, 22.4, 22.3, 22.2, 22.1, 22.0, 21.9, 21.8, 21.7, 21.6, 21.5, 21.4, 21.3, 21.3, 21.2, 21.1, 21.0, 20.9, 20.8, 
20.7, 20.6, 20.5, 20.4, 20.3, 20.3, 20.2, 20.1, 20.0, 19.9, 19.8, 19.7, 19.6, 19.5, 19.4, 19.4, 19.3, 19.2, 19.2, 19.1, 
19.0, 18.9, 18.8, 18.7, 18.6, 18.5, 18.5, 18.4, 18.3, 18.2, 18.1, 18.0, 18.0, 17.9, 17.8, 17.7, 17.6, 17.5, 17.4, 17.3, 
17.3, 17.2, 17.1, 17.0, 16.9, 16.8, 16.7, 16.6, 16.5, 16.5, 16.4, 16.3, 16.2, 16.1, 16.0, 15.9, 15.9, 15.8, 15.7, 15.6, 
15.5, 15.4, 15.3, 15.3, 15.2, 15.1, 15.0, 14.9, 14.8, 14.8, 14.7, 14.6, 14.5, 14.4, 14.3, 14.2, 14.1, 14.0, 13.9, 13.9, 
13.8, 13.7, 13.6, 13.5, 13.4, 13.4, 13.3, 13.2, 13.1, 13.0, 12.9, 12.8, 12.7, 12.7, 12.6, 12.5, 12.4, 12.3, 12.2, 12.1, 

12.0, 12.0, 11.9, 11.8, 11.7, 11.6, 11.5, 11.4, 11.3, 11.2, 11.2, 11.1, 11.0, 10.9, 10.8, 10.7, 10.6, 10.5, 10.5, 10.4, 
10.3, 10.2, 10.1, 10.0, 9.9 , 9.9 , 9.8 , 9.7 , 9.6 , 9.5 , 9.4 , 9.3 , 9.2 , 9.1 , 9.1 , 9.0 , 8.9 , 8.8 , 8.7 , 8.6 , 
8.5 , 8.4 , 8.4 , 8.3 , 8.2 , 8.1 , 8.0 , 7.9 , 7.8 , 7.7 , 7.6 , 7.5 , 7.4 , 7.3 , 7.3 , 7.2 , 7.1 , 7.0 , 6.9 , 6.8 , 
6.7 , 6.6 , 6.5 , 6.4 , 6.4 , 6.3 , 6.2 , 6.1 , 6.0 , 5.9 , 5.8 , 5.7 , 5.6 , 5.5 , 5.4 , 5.4 , 5.3 , 5.2 , 5.1 , 5.0 , 
4.9 , 4.8 , 4.7 , 4.6 , 4.5 , 4.4 , 4.3 , 4.2 , 4.1 , 4.0 , 3.9 , 3.9 , 3.8 , 3.7 , 3.6 , 3.5 , 3.4 , 3.3 , 3.2 , 3.1 , 
3.0 , 2.9 , 2.8 , 2.7 , 2.6 , 2.5 , 2.4 , 2.3 , 2.2 , 2.1 , 2.0 , 1.9 , 1.8 , 1.7 , 1.6 , 1.5 , 1.4 , 1.3 , 1.3 , 1.2 , 
1.1 , 1.0 , 0.9 , 0.8 , 0.7 , 0.6 , 0.5 , 0.4 , 0.3 , 0.2 , 0.1 , -0.1, -0.2, -0.3, -0.4, -0.5, -0.6, -0.7, -0.8, -0.9,
-1.0, -1.1, -1.2, -1.3, -1.4, -1.5, -1.6, -1.7, -1.8, -1.9, -2.0, -2.1, -2.2, -2.3, -2.4, -2.6, -2.7, -2.8, -2.9, -3.0, 
-3.1, -3.2, -3.3, -3.4, -3.5, -3.6, -3.8, -3.9, -4.0, -4.1, -4.2, -4.3, -4.4, -4.5, -4.6, -4.8, -4.9, -5.0, -5.1, -5.2, 
-5.3, -5.4, -5.6, -5.7, -5.8, -5.9, -6.0, -6.1, -6.3, -6.4, -6.5, -6.6, -6.7, -6.9, -7.0, -7.1, -7.2, -7.3, -7.5, -7.6, 

-7.7, -7.8, -8.0, -8.1, -8.2, -8.3, -8.5, -8.6, -8.7, -8.8, -9.0, -9.1, -9.2, -9.4, -9.5, -9.6, -9.7, -9.9,-10.0,-10.1,
-10.3,-10.4,-10.5,-10.7,-10.8,-10.9,-11.1,-11.2,-11.3,-11.5,-11.6,-11.8,-11.9,-12.0,-12.2, -12.3,-12.5,-12.6,-12.7,-12.9,
-13.0,-13.2,-13.3,-13.5,-13.6,-13.7,-13.9,-14.0,-14.2,-14.3,-14.5,-14.6,-14.8,-14.9,-15.1, -15.2,-15.4,-15.6,-15.7,-15.9,
-16.0,-16.2,-16.3,-16.5,-16.6,-16.8,-17.0,-17.1,-17.3,-17.4,-17.6,-17.8,-17.9,-18.1,-18.3, -18.4,-18.6,-18.8,-18.9,-19.1,
-19.3,-19.5,-19.6,-19.8,-20.0,-20.1,-20.3,-20.5,-20.7,-20.9,-21.0,-21.2,-21.4,-21.6,-21.8, -21.9,-22.1,-22.3,-22.5,-22.7,
-22.9,-23.1,-23.2,-23.4,-23.6,-23.8,-24.0,-24.2,-24.4,-24.6,-24.8,-25.0,-25.2,-25.4,-25.6, -25.8,-26.0,-26.2,-26.4,-26.6,
-26.8,-27.0,-27.2,-27.5,-27.7,-27.9,-28.1,-28.3,-28.5,-28.7,-29.0,-29.2,-29.4,-29.6,-29.8, -30.1,-30.3,-30.5,-30.9,-31.0,
-31.2,-31.4,-31.6,-31.8,-32.1,-32.4,-32.6,-32.8,-33.1,-33.3,-33.6,-33.8,-34.0,-34.3,-34.5, -34.8,-35.0,-35.3,-35.5,-35.8,
-36.0,-36.3,-36.6,-36.8,-37.1,-37.3,-37.6,-37.9,-38.1,-38.4,-38.7,-38.9,-39.2,-39.5,-39.8, -40.0
};

const unsigned char AC2000PlusCmd[AC2000_plus_CmdNum][AC2000PlusCmdLength]=/*命令码表*/
{
/*000 扫描鸡舍号0000_0000_01		*/	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x03,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x01,0x05},
/*001 主屏幕0479_0000_50	*/		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x00,0x04,0x32,0x01,0x00,0x00,0x2C,0x65},
/*002 报警查询0002_0000_0C*/		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x02,0x01,0x00,0x00,0x0C,0x11},
/*003 时间查询0234_0000_02*/		    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x34,0x01,0x00,0x00,0x02,0x3C},
/*系统参数*/					{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x17,0x01,0x00,0x00,0xE8,0x07},
/*温度校准因素*/				{0xff,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x1e,0x01,0x00,0x00,0x1c,0x3f},
/*温度*/					    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x37,0x01,0x00,0x00,0x0C,0x47},
/*测试继电器(实际开闭状态)*/        {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x15,0x01,0x00,0x00,0x38,0x51},
/*测试继电器(设置的默认状态)*/	    {0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x38,0x01,0x00,0x00,0x04,0x40}
};
const unsigned char AC2000PlusControlTemperatureCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x0A,0x01,0x00,0x00,0x8C,0x9C}
};

const unsigned char AC2000PlusMinmax_levelCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x0B,0x01,0x00,0x00,0x28,0x39}
};

const unsigned char AC2000Plushumidity_Co2[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x0C,0x01,0x00,0x00,0x03,0x14}
};

const unsigned char AC2000PlusRefrigeration[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x0D,0x01,0x00,0x00,0x46,0x58}
};

const unsigned char AC2000Plusillumination[2][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x0E,0x01,0x00,0x00,0xFC,0x10},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x0E,0x01,0xFC,0x00,0x62,0x72}
};

const unsigned char AC2000PlusfeedCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x11,0x01,0x00,0x00,0x50,0x65}
};

const unsigned char AC2000PlusAdditional_systemCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x24,0x01,0x00,0x00,0x33,0x5B}
};

const unsigned char AC2000PluspressureCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x1A,0x01,0x00,0x00,0x12,0x31}
};

const unsigned char AC2000PlushumidifierCmd[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x26,0x01,0x00,0x00,0x46,0x71}
};

const unsigned char AC2000PlusManagedeath[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x44,0x01,0x00,0x00,0x04,0x4C}
};

const unsigned char AC2000PlusManagePlusPoultry_metering[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x18,0x01,0x00,0x00,0x04,0x20}
};

const unsigned char AC2000PlusManageFeed_stock[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x36,0x01,0x00,0x00,0x04,0x3E}
};

const unsigned char AC2000PlusManageTime_day[2][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x34,0x01,0x00,0x00,0x02,0x3B},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x10,0x01,0x00,0x00,0x02,0x18}
};

const unsigned char AC2000PlusManagestock[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x19,0x01,0x00,0x00,0x01,0x1E}
};

const unsigned char AC2000PlusManagenewstock[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x35,0x01,0x00,0x00,0x01,0x3A}
};

const unsigned char AC2000PlusManagealarmRest[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x03,0x01,0x00,0x00,0x01,0x07}
};
const unsigned char AC2000PlusManagealarmTest[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x1F,0x01,0x00,0x00,0x04,0x27}
};

const unsigned char AC2000PlusTestTemperatureCmd[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x37,0x01,0x00,0x00,0x0C,0x47}
};

const unsigned char AC2000PlusTestHumidity[1][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x40,0x01,0x00,0x00,0x08,0x4D}
};

const unsigned char AC2000PlusTestRelay[2][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x15,0x01,0x00,0x00,0x38,0x51},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x38,0x01,0x00,0x00,0x04,0x40}
};

const unsigned char AC2000PlusTestDigital[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x41,0x01,0x00,0x00,0x02,0x47}
};
const unsigned char AC2000PlusTestPulse[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x3C,0x01,0x56,0x00,0x02,0x9A}
};

const unsigned char AC2000PlusTestChicken[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x4A,0x01,0x00,0x00,0x04,0x52}
};

const unsigned char AC2000PlusTestVersion[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x01,0x01,0x00,0x00,0x06,0x0A}
};

const unsigned char AC2000PlusCaliHumetidy[2][AC2000PlusCmdLength]=
{
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x23,0x01,0x00,0x00,0x08,0x2F},
	{	0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x40,0x01,0x00,0x00,0x08,0x4D}

};
const unsigned char AC2000PlusCalichick[2][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x21,0x01,0x00,0x00,0x08,0x2E},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x33,0x01,0x00,0x00,0x04,0x3B}
};

const unsigned char AC2000PlusCaliFeed[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x1C,0x01,0x00,0x00,0x04,0x23}
};

const unsigned char AC2000PlusCaliWater[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x1B,0x01,0x00,0x00,0x04,0x23}
};

const unsigned char AC2000PlusCaliCo2[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x28,0x01,0x00,0x00,0x06,0x32}
};
const unsigned char AC2000PlusInstallConguration[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x12,0x01,0x00,0x00,0x1F,0x39}
};
const unsigned char AC2000PlusInstallSensor[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x16,0x01,0x00,0x00,0x13,0x2E}
};

const unsigned char AC2000PlusInstallSystem[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x05,0x17,0x01,0x00,0x00,0xE8,0x07}
};

const unsigned char AC2000PlusInstallWindColling[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x27,0x01,0x00,0x00,0x3D,0x6A}
};

const unsigned char AC2000PlusInstallVenti[2][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x13,0x01,0x00,0x00,0xF0,0x09},
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x03,0x13,0x01,0xF0,0x00,0x50,0x59}
};
const unsigned char AC2000PlusInstallRollerBlind[1][AC2000PlusCmdLength]=
{
	{0xFF,0x41,0x54,0x54,0x45,0x00,0x00,0x00,0x02,0x00,0x01,0x02,0x14,0x01,0x00,0x00,0x64,0x7D}
};

void PlusOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000PlusCmdLength],uint8_t cmd_num,uint8_t *cmd_index)
{

	if((*cmd_index)<cmd_num)
	{
		uint8_t AC2000SendBuff[AC2000PlusCmdLength];
		memset(AC2000SendBuff,0,sizeof(AC2000SendBuff));
		CreateAC2000PlusNormal(Cmd,AC2000SendBuff,ac2000_table.ac2000_plus_house,(*cmd_index));   //参数赋值
		(*cmd_index)++;
		PlusRecvFlag=(AC2000SendBuff[16]+7);
		if((*cmd_index)==cmd_num)
		{
			AirxxxStatus.aliyun_service_param1=-1;
			AirxxxStatus.aliyun_service_param2=-1;
			AirxxxStatus.recv_systeminfo_cmd=2;	
		}
		rt_kprintf("SEND_CMD:%02X%02X  %d\n",AC2000SendBuff[11],AC2000SendBuff[12],(AC2000SendBuff[16]+7));
		user_usart_pass((rt_uint8_t*)AC2000SendBuff, AC2000PlusCmdLength, PlusRecvFlag);
	}
	if((*cmd_index)>=cmd_num)
	{
		(*cmd_index)=0;								
	}
}


void CreateAC2000PlusNormal(const uint8_t CmdSource[][AC2000PlusCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
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


void CreateAC2000PlusSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex)
{
	AC2000_SendBuff[0] = 0xff;
	AC2000_SendBuff[1] = 0x41;
	AC2000_SendBuff[2] = 0x54;
	AC2000_SendBuff[3] = 0x54;
	AC2000_SendBuff[4] = 0x45;
	AC2000_SendBuff[5] = 0x00;
	AC2000_SendBuff[6] = 0x00;
	AC2000_SendBuff[7] = 0x00;
	AC2000_SendBuff[8] = AC2000PlusCmd[CmdIndex][8];
	AC2000_SendBuff[9] = 0x00;
	AC2000_SendBuff[10] = HenHouseNum;
	AC2000_SendBuff[11] = AC2000PlusCmd[CmdIndex][11];
	AC2000_SendBuff[12] = AC2000PlusCmd[CmdIndex][12];
	AC2000_SendBuff[13] = AC2000PlusCmd[CmdIndex][13];
	AC2000_SendBuff[14] = AC2000PlusCmd[CmdIndex][14];
	AC2000_SendBuff[15] = AC2000PlusCmd[CmdIndex][15];
	AC2000_SendBuff[16] = AC2000PlusCmd[CmdIndex][16];
	AC2000_SendBuff[17] = AC2000PlusCmd[CmdIndex][17]+HenHouseNum;
	uint8_t sum=0;
	for(uint16_t i=5;i<=16;i++)
	{
		sum=sum+AC2000_SendBuff[i];
	}
	AC2000_SendBuff[17]=sum;	

}


void Ac2000DataPlusMalloc(void)
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
				rt_kprintf("Ac2000DataPLUS malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000DataPLUS malloc failed\r\n");
			}		
		}
		if(Ac2000OldData==NULL)
		{
			Ac2000OldData=(AC2000_DATA_TYPE *)rt_malloc(sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);	
			if(Ac2000OldData!=NULL)
			{
				rt_memset(Ac2000OldData,0,sizeof(AC2000_DATA_TYPE)*BASE_DATA_ID_NUM_3G);
				rt_kprintf("Ac2000OldDataPLUS malloc sucess\r\n");
			}
			else
			{
				rt_kprintf("Ac2000OldDataPLUS malloc failed\r\n");
			}	
		}
		
		if(Ac2000PlusParams==NULL)
		{
			Ac2000PlusParams=(PlusAc2000Params *)rt_malloc(sizeof(PlusAc2000Params));
			if(Ac2000PlusParams!=NULL)
			{
				rt_memset(Ac2000PlusParams,0,sizeof(PlusAc2000Params));
				int len=sizeof(PlusAc2000Params);
				rt_kprintf("\nAc20003GParams size:%d\n",len);
			}
			else
			{
				rt_kprintf("Ac20003GParams创建失败!\n");
			}
		}
		
}

void Parse_MainPlus(uint8_t *dataSource,uint16_t len)//主屏幕解析函数
{
	unsigned int  postion = 6;
	unsigned int TempInit =0;
	TempInit = dataSource[postion+1];
	Ac2000Data[ADDR_1136] = TempInit;
	rt_kprintf("当前通风等级 = %d\r\n",Ac2000Data[ADDR_3G_1805]);
	postion += 6;
	
	TempInit = dataSource[postion]*256+dataSource[postion+1];
	Ac2000Data[ADDR_1135] = TempInit;
	rt_kprintf("当前日龄 = %d\r\n",Ac2000Data[ADDR_3G_1804]);
	postion += 1;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1133] = TempInit;
	rt_kprintf("当前静压 = %d\r\n",Ac2000Data[ADDR_3G_1802]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1134] = (TempInit/100)*10+(TempInit%10);  //算出十位和个位
	rt_kprintf("目标静压 = %d\r\n",Ac2000Data[ADDR_3G_1803]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1138] = TempInit;  
	rt_kprintf("平均温度 = %d\r\n",Ac2000Data[ADDR_3G_1807]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1198] = TempInit;  
	rt_kprintf("设置温度 = %d\r\n",Ac2000Data[ADDR_3G_1847]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1199] = TempInit;  
	rt_kprintf("加热温度 = %d\r\n",Ac2000Data[ADDR_3G_1848]);
	postion += 2;
	
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1282] = TempInit;  
	rt_kprintf("制冷温度 = %d\r\n",Ac2000Data[ADDR_3G_1932]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1139] = TempInit ; 
	rt_kprintf("室外温度 = %d\r\n",Ac2000Data[ADDR_3G_1808]);
	postion += 4;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1147] = TempInit/10;  
	rt_kprintf("外部湿度 = %d\r\n",Ac2000Data[ADDR_3G_1816]);
	postion += 12;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1209] = TempInit;  
	rt_kprintf("今日水量 = %d\r\n",Ac2000Data[ADDR_3G_1858]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	Ac2000Data[ADDR_1142] = TempInit;  
	rt_kprintf("冷风温度 = %d\r\n",Ac2000Data[ADDR_3G_1811]);
	postion += 2;
	
	TempInit = dataSource[postion+1]*256+dataSource[postion+2];
	if (TempInit == 65535) rt_kprintf("CO2 = %d\r\n",0);
	else
	{
		Ac2000Data[ADDR_3G_1813] = TempInit;  
		rt_kprintf("CO2 = %d\r\n",Ac2000Data[ADDR_1144]);
	}
}


void Plusday_time(uint8_t *dataSource,uint16_t len)
{

	unsigned int  postion = 6;
	unsigned int TempInit =0;
	
	TempInit = dataSource[postion]*256+dataSource[postion+1];//时间
	Ac2000Data[ADDR_1266]=TempInit;
	rt_kprintf("本次运行时间 = %d\r\n",Ac2000Data[ADDR_3G_1915]);
}

void SystemParameters(uint8_t *dataSource,uint16_t len)
{
	char tmp[8];
	int postion ;
	unsigned char i;
	float TmpFloat;
	postion =6;
	for(i=0;i<58;i++)
	{
		if(7==(i+1))//高温报警，高于目标温度的值（度）
		{
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			Ac2000Data[ADDR_1202] = ((int)(TmpFloat*100)+Ac2000Data[ADDR_1198]);
			sprintf(tmp,"%0.1f,",TmpFloat);
			//Ac2000Data[ADDR_3G_1851]ADDR_1202
			rt_kprintf("高温报警，高于目标温度的值%d（度）\r\n",(int)(TmpFloat*100));
		}
		if(11==(i+1))//低温报警，低于加热温度的值（度）
		{
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			Ac2000Data[ADDR_1201] = (Ac2000Data[ADDR_1198]-(int)(TmpFloat*100));//Ac2000Data[ADDR_3G_1850]
			sprintf(tmp,"%0.1f,",TmpFloat);
			rt_kprintf("低温报警，低于加热温度的值%d（度）\r\n",(int)(TmpFloat*100));
		}
		postion = postion+4;
	}
}

#include <stdio.h>
#define DBG_BUFF_MAX_LEN          256
/* debug print : support float double */
int dbg_printf(const char *fmt, ...)
{
    va_list args;
    static char rt_log_buf[DBG_BUFF_MAX_LEN] = { 0 };

    va_start(args, fmt);
    int length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);

    rt_kputs(rt_log_buf);

    return length;
}

#include "NTCCalculation.h"
float TempCoefficientArray[6];//温度系数值
void PlusTemperature(uint8_t *dataSource,uint16_t len)
{
	char tmp[8];
	int postion ;
	int Revicedata=0;
	float TmpFloat;
	unsigned char j;
	postion =6;
	for(j=0;j<6;j++)
	{
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",j+1);
		Revicedata = dataSource[postion]*256+dataSource[postion+1];
		if((Revicedata <= 0x03F3)&&(Revicedata>=0x0024))
		{
			TmpFloat = (tableTemperature[Revicedata-0x0024]*100);//对应查表
//			TmpFloat = (calculationGetNtc(Revicedata)*100.0f);//估值查表
		}
		else if(Revicedata<0x0024) //短路 大于100度
		{
			TmpFloat = -99999-TempCoefficientArray[j];
		}
		else if(Revicedata>0x03F3) //开路 小于-40度
		{
			TmpFloat = 99999-TempCoefficientArray[j];
		}
		dbg_printf("温度%d：%f（度）\r\n",j+1,TmpFloat);
		Ac2000Data[ADDR_3G_1821+j] = 	TmpFloat;
		postion =postion+2;
	}
}

/*温度校准系数*/
void PlusTemperatureCoefficient(uint8_t *dataSource,uint16_t len)
{
	union{
	 float TmpFloat;
	 unsigned char farray[4];
	}utemp;
	int postion ;
	unsigned char j;
	postion =6;
	postion +=4;

	for(j=0;j<6;j++)
	{
		utemp.farray[0] = dataSource[postion+3];
		utemp.farray[1] = dataSource[postion+2];
		utemp.farray[2] = dataSource[postion+1];
		utemp.farray[3] = dataSource[postion];
		rt_kprintf("utemp.farray：0x%X,0x%X,0x%X,0x%X\n",utemp.farray[0],utemp.farray[1],utemp.farray[2],utemp.farray[3]);
		TempCoefficientArray[j] = utemp.TmpFloat;
		dbg_printf("温度%d系数：%f\r\n",j+1,utemp.TmpFloat);
		postion +=4;
	}
}
unsigned char relay_state[20]={0};
void PlusRelay(uint8_t *dataSource,uint16_t len)
{
	int postion ;
	unsigned char TmpInt;
	postion=6;

	/*继电器实际状态 命令0x0138*/
	if(len==11)
	{
		TmpInt = dataSource[postion];//序号
//		Ac2000Data[ADDR_3G_1827] = (TmpInt&0x08);
//		Ac2000Data[ADDR_3G_1827+1] = (TmpInt&0x04);
//		Ac2000Data[ADDR_3G_1827+2] = (TmpInt&0x02);
//		Ac2000Data[ADDR_3G_1827+3] = (TmpInt&0x01);
//		
//		TmpInt = dataSource[postion+1];//序号
//		Ac2000Data[ADDR_3G_1827+4] = (TmpInt&0x80);
//		Ac2000Data[ADDR_3G_1827+5] = (TmpInt&0x40);
//		Ac2000Data[ADDR_3G_1827+6] = (TmpInt&0x20);
//		Ac2000Data[ADDR_3G_1827+7] = (TmpInt&0x10);
//		Ac2000Data[ADDR_3G_1827+8] = (TmpInt&0x08);
//		Ac2000Data[ADDR_3G_1827+9] = (TmpInt&0x04);
//		Ac2000Data[ADDR_3G_1827+10] = (TmpInt&0x02);
//		Ac2000Data[ADDR_3G_1827+11] = (TmpInt&0x01);
//	
//		TmpInt = dataSource[postion+2];//序号
//		Ac2000Data[ADDR_3G_1827+12] = (TmpInt&0x80);
//		Ac2000Data[ADDR_3G_1827+13] = (TmpInt&0x40);
//		Ac2000Data[ADDR_3G_1827+14] = (TmpInt&0x20);
//		Ac2000Data[ADDR_3G_1827+15] = (TmpInt&0x10);
//		Ac2000Data[ADDR_3G_1827+16] = (TmpInt&0x08);
//		Ac2000Data[ADDR_3G_1827+17] = (TmpInt&0x04);
//		Ac2000Data[ADDR_3G_1827+18] = (TmpInt&0x02);
//		Ac2000Data[ADDR_3G_1827+19] = (TmpInt&0x01);
		relay_state[0] = (TmpInt&0x08);
		relay_state[1] = (TmpInt&0x04);
		relay_state[2] = (TmpInt&0x02);
		relay_state[3] = (TmpInt&0x01);
		
		TmpInt = dataSource[postion+1];//序号
		relay_state[4] = (TmpInt&0x80);
		relay_state[5] = (TmpInt&0x40);
		relay_state[6] = (TmpInt&0x20);
		relay_state[7] = (TmpInt&0x10);
		relay_state[8] = (TmpInt&0x08);
		relay_state[9] = (TmpInt&0x04);
		relay_state[10] = (TmpInt&0x02);
		relay_state[11] = (TmpInt&0x01);
		
		TmpInt = dataSource[postion+2];//序号
		relay_state[12] = (TmpInt&0x80);
		relay_state[13] = (TmpInt&0x40);
		relay_state[14] = (TmpInt&0x20);
		relay_state[15] = (TmpInt&0x10);
		relay_state[16] = (TmpInt&0x08);
		relay_state[17] = (TmpInt&0x04);
		relay_state[18] = (TmpInt&0x02);
		relay_state[19] = (TmpInt&0x01);
	}
	else if(63==len)
	{
		for(int j=0;j<30;j++)
		{
			TmpInt = dataSource[postion];//序号
			TmpInt=dataSource[postion+1];
			Ac2000Data[ADDR_3G_1827+j]=TmpInt;
			postion=postion+2;//22
			rt_kprintf("继电器 %d 状态=%d\r\n",j,Ac2000Data[ADDR_3G_1827+j]);
		}
	}
}

void Parse_WarnPlus(uint8_t *dataSource,uint16_t len)
{
	unsigned int  postion = 6;

	
	unsigned char warnNum=dataSource[postion];
	if(warnNum>0)
	{
		Ac2000Data[ADDR_1132]=1;//3G版报警状态是否存在报警
	}
	else
	{
		Ac2000Data[ADDR_1132]=0;
	}
	unsigned int alarm_code=0x00;
	for(unsigned char j=0;j<=AC2000_ALARM_NUM_3G;j++)
	{
		AlarmCode3g[ALARM_3G_12000+j][1]=0;
	}
	postion +=2;
	for(unsigned char i = 0; i<warnNum; i++)
	{
		alarm_code=dataSource[postion+1]*256+dataSource[postion];//得到告警类型码
		for(u16 j=0;j<AC2000_ALARM_NUM_3G;j++)
		{
			if(alarm_code==AC2000WarmCode[j])
			{
				rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				AlarmCode3g[ALARM_3G_12000+j][1]=1;
				break;
			}
			if(j==(AC2000_ALARM_NUM_3G-1))
			{
				if(alarm_code==AC2000WarmCode[AC2000_ALARM_NUM_3G-1])
				{
					AlarmCode3g[0+j][1]=1;
					rt_kprintf("告警类型码=%04X\r\n",AC2000WarmCode[j]);
				}
				else
				{
					AlarmCode3g[ALARM_3G_12000][1]=1;
					rt_kprintf("未知报警\r\n");
				}
			}
		}
		postion+=2;
	}

}

void temperature_plus(uint8_t *dataSource,uint16_t len)
{
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	float	TmpFloat;
	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	postion = 6;
	
	
	for(unsigned char i=0;i<10;i++)
	{
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//日
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		postion=postion+2;
		
	
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//加热
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion = postion+4;
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//加热
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion = postion+4;
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//制冷
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve,0,sizeof(Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve));
	strcat(Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve,SysData);	
	rt_kprintf("rt_kprintf. PlusAc2000ControlTemperatureCurve:%d\n",sizeof(Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve));
	rt_kprintf("温度1-.PlusAc2000ControlTemperatureCurve:%s\n",SysData);
	rt_kprintf("温度2-.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSTEMPERATURE_CURVE_CMD;
	}
	
}


void Minmax_level_plus(uint8_t *dataSource,uint16_t len)//最小最大级别
{
	
	unsigned int postion;
	int	TmpInt;
	char tmp[8];
	

	char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
	rt_memset(SysData,0,AliyunServiceSysLength);
	postion = 6;
	for(unsigned char i=0;i<10;i++)
	{
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//日
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpInt = dataSource[postion];//最小
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		
		TmpInt = dataSource[postion];//最大
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		
	}
	
	if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	rt_memset(Ac2000PlusParams->PAc2000Control.Plusmin_max_level,0,sizeof(Ac2000PlusParams->PAc2000Control.Plusmin_max_level));
	strcat(Ac2000PlusParams->PAc2000Control.Plusmin_max_level,SysData);	
	rt_kprintf("最小最大级别-.Plusmin_max_level:%s\n",SysData);
	rt_free(SysData);
	if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSMIN_MAX_LEVEL_CMD;
	}

}


void humidity_Co2(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
	
	
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 7;
	
		TmpInt = dataSource[postion];//Co2
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
	
		TmpInt = dataSource[postion];//湿度
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Control.Plushumidity_treatm,0,sizeof(Ac2000PlusParams->PAc2000Control.Plushumidity_treatm));
		strcat(Ac2000PlusParams->PAc2000Control.Plushumidity_treatm,SysData);	
		rt_kprintf("湿度CO2-.Plushumidity_treatm:%s\n",SysData);
		rt_free(SysData);
		
			if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSHUMIDITY_TREATM_CMD;
	}

}


void Refrigeration_Plus(uint8_t *dataSource,uint16_t len)
{
	  unsigned int postion;
		int	TmpInt;
		char tmp[8];
	  int Hour;
	  int Minute;
		float	TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(unsigned char i=0; i<5; i++)
	 {
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//开始时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
	  postion = postion+2;
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//结束时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//隧道差值
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//至湿度
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/10.0);//开(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/10.0);//关(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+2;
	 }
	 
	 if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
	 rt_memset(Ac2000PlusParams->PAc2000Control.PlusRefrigeration,0,sizeof(Ac2000PlusParams->PAc2000Control.PlusRefrigeration));
	 strcat(Ac2000PlusParams->PAc2000Control.PlusRefrigeration,SysData);	
	 rt_kprintf("制冷-.PlusRefrigeration:%s\n",SysData);
	 rt_free(SysData);
	 
			if(AirxxxStatus.recv_systeminfo_cmd==2)
	{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSREFRIGERATION_CMD;
	}
}

void 	Illumination_plus(uint8_t *dataSource,uint16_t len)
{
	if(len == 259)
	{
	if(PLUSLevelsOfVentilationData==NULL) 
	{
		PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
		if(PLUSLevelsOfVentilationData!=NULL)
		{
			rt_memset(PLUSLevelsOfVentilationData,0,256*3);
			PLUSLevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(PLUSLevelsOfVentilationDataLength<=256*2)
	{
		rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
	}
	else
	{
		rt_memset(PLUSLevelsOfVentilationData,0,256*3);
		PLUSLevelsOfVentilationDataLength=0;
	}
	}
	else
	{
		if(len>6)
		{
	
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
	
		rt_kprintf("PLUSLevelsOfVentilationDataLength:%s->\n",PLUSLevelsOfVentilationData);
		rt_kprintf("PLUSLevelsOfVentilationDataLength:%d->\n",PLUSLevelsOfVentilationDataLength);
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
	  int Hour;
	  int Minute;
	
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 0;
		
		for(unsigned char i=0;i<50;i++)
		{
			if(postion<(PLUSLevelsOfVentilationDataLength-1)) 
			{
				TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];//日
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion = postion+2;
			
				TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];//从时间
				Hour = TmpInt/60;
				Minute = TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",Minute);
				strcat(SysData,tmp);
				postion = postion+2;
			
				TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];//至时间
				Hour = TmpInt/60;
				Minute = TmpInt%60;
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d",Hour);
				strcat(SysData,tmp);
				strcat(SysData,":");
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",Minute);
				strcat(SysData,tmp);
				postion = postion+2;
		
				TmpInt = PLUSLevelsOfVentilationData[postion];//强度
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion = postion+1;
			}
		}
		
		
		 if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlusParams->PAc2000Control.Plusillumination,0,sizeof(Ac2000PlusParams->PAc2000Control.Plusillumination));
			strcat(Ac2000PlusParams->PAc2000Control.Plusillumination,SysData);	
			rt_kprintf("照明1-.PlusRefrigeration:%s\n",Ac2000PlusParams->PAc2000Control.Plusillumination);
			rt_kprintf("照明2-.PlusRefrigeration:%s\n",SysData);
			rt_free(SysData);
			if(PLUSLevelsOfVentilationData!=NULL) 
			{
				rt_free(PLUSLevelsOfVentilationData);
				PLUSLevelsOfVentilationData=NULL;
			}
			PLUSLevelsOfVentilationDataLength=0;
			
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSILLUMINATION_CMD;
		}
			
		}
	}
}

void ConFeed(uint8_t *dataSource,uint16_t len)
{
	  unsigned int postion;
		int	TmpInt;
		char tmp[8];
	  int Hour;
	  int Minute;
		float	TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(unsigned i=0;i<10;i++)
	{

		 
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//开始时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
	  postion = postion+2;
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//结束时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//量/鸡
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
	}
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Control.PlusFeed,0,sizeof(Ac2000PlusParams->PAc2000Control.PlusFeed));
		strcat(Ac2000PlusParams->PAc2000Control.PlusFeed,SysData);	
		rt_kprintf("饲料-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSFEED_CMD;
		}
}

void Additional_system(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
	  int Hour;
	  int Minute;
		float	TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(unsigned char i=0; i<3; i++)
	 { 

		 
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//开始时间
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
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//结束时间
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
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//开始温度
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//结束温度
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
		
		TmpInt = (dataSource[postion]*256+dataSource[postion+1]);//开(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
	
		TmpInt = (dataSource[postion]*256+dataSource[postion+1]);//关(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpInt = dataSource[postion];//传感器
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
	 }
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Control.PlusAdditional_system,0,sizeof(Ac2000PlusParams->PAc2000Control.PlusAdditional_system));
		strcat(Ac2000PlusParams->PAc2000Control.PlusAdditional_system,SysData);	
		rt_kprintf("附加系统-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSADDITIONAL_CMD;
		}
}

void pressure(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		char tmp[8];
		double TmpFloat;
		float Float;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 16;
	
		Float = orderFourByteToFloat(&dataSource[postion]);//舎外低温
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",Float);
		strcat(SysData,tmp);
		postion = 8;
		
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/1000.00);//压力低温
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 20;
		
		Float = orderFourByteToFloat(&dataSource[postion]);//舎外高温
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",Float);
		strcat(SysData,tmp);
		postion = 6;
	
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/1000.00);//压力高温
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 10;
	
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/1000.00);//低压报警
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 12;
		
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/1000.00);//高压报警
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		postion =14;
	
		TmpFloat = (float)((dataSource[postion]*256+dataSource[postion+1])/1000.00);//滞后
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Control.Plusstatic_pressure,0,sizeof(Ac2000PlusParams->PAc2000Control.Plusstatic_pressure));
		strcat(Ac2000PlusParams->PAc2000Control.Plusstatic_pressure,SysData);	
		rt_kprintf("压力-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSSTATICPRESSURE_CMD;
		}
}

void humidifier(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
	  int Hour;
	  int Minute;
		float	TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		for(unsigned char i=0; i<5; i++)
	 { 
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//开始时间
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
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//结束时间
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
		
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//目标差值
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+4;
	
		
		TmpInt = (dataSource[postion]*256+dataSource[postion+1]);//湿度百分比
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
	
		TmpFloat = (dataSource[postion]*256+dataSource[postion+1])/10.0;//开(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+2;
		
		TmpFloat = (dataSource[postion]*256+dataSource[postion+1])/10.0;//关(分)
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = postion+2;
	 }
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Control.Plushumidifier,0,sizeof(Ac2000PlusParams->PAc2000Control.Plushumidifier));
		strcat(Ac2000PlusParams->PAc2000Control.Plushumidifier,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=PLUSHUMIDIFIER_CMD;
		}
}


void Manage_death(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];//添加死亡
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = dataSource[postion];//每日死亡
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = dataSource[postion];//添加淘汰
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = dataSource[postion];//每日淘汰
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusChicken_Death,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusChicken_Death));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusChicken_Death,SysData);	
		rt_kprintf("死亡量-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGEDEATH_CMD;
		}
}


void Manage_PlusPoultry_metering(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =9;
		TmpInt = dataSource[postion];//初始鸡数
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		TmpInt = dataSource[postion]-18;//更新鸡数
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusPoultry_metering,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusPoultry_metering));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusPoultry_metering,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGEPOUITRY_CMD;
		}
}


void Manage_Plusfeed_stock(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		float TmpFloat;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion = 6;
		strcat(SysData,"0,");
		TmpFloat = orderFourByteToFloat(&dataSource[postion]);//饲料供应
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusFeed_stock,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusFeed_stock));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusFeed_stock,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGEFEEDSTOCK_CMD;
		}

	
}

unsigned char STAETTYPE=0;
void Manage_Plusday_time(uint8_t *dataSource,uint16_t len)
{
	if(PLUSLevelsOfVentilationData==NULL) 
	{
		PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256*12);
		if(PLUSLevelsOfVentilationData!=NULL)
		{
			rt_memset(PLUSLevelsOfVentilationData,0,256*12);
			PLUSLevelsOfVentilationDataLength=0;
		}
		else
		{
			rt_kprintf("LevelsOfVentilationData malloc failure\n");
		}
	}
	if(PLUSLevelsOfVentilationDataLength<=256*11)
	{
		rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
		STAETTYPE++;
	}
	else
	{
		STAETTYPE=0;
		rt_memset(PLUSLevelsOfVentilationData,0,256*12);
		PLUSLevelsOfVentilationDataLength=0;
	}
	
	if(STAETTYPE>=2)
	{
		unsigned int postion;
		int TmpInt;
		int Hour;
		int Minute;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];//时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
	  postion = postion+2;
		
		TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];//日龄
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusTime_day,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusTime_day));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusTime_day,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		if(PLUSLevelsOfVentilationData!=NULL) 
		{
			rt_free(PLUSLevelsOfVentilationData);
			PLUSLevelsOfVentilationData=NULL;
		}
			PLUSLevelsOfVentilationDataLength=0;
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGETIMEDAY_CMD;
		}
		
		STAETTYPE=0;
	}
		
}

void Manage_PlusFlock(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];//禽群数
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusFlock,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusFlock));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusFlock,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGESTOCK_CMD;
		}
}

void Manage_PlusNewFlock(uint8_t *dataSource,uint16_t len)
{
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];//是否清楚历史
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusNew_Flock,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusNew_Flock));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusNew_Flock,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGENEWSTOCK_CMD;
		}
}

void Manage_PlusAlarm_reset(uint8_t *dataSource,uint16_t len)
{
	
		unsigned int postion;
		int	TmpInt;
		char tmp[8];
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];//是否清楚历史
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusAlarm_reset,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusAlarm_reset));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusAlarm_reset,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGEALARMTES_CMD;
		}

}

void Manage_PlusTest(uint8_t *dataSource,uint16_t len)
{
		int TmpInt;
		int Hour;
		int Minute;
		char tmp[8];
		int postion ;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//时间
		Hour = TmpInt/60;
		Minute = TmpInt%60;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",Hour);
		strcat(SysData,tmp);
		strcat(SysData,":");
	  memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",Minute);
		strcat(SysData,tmp);
	  postion = postion+2;
		
		TmpInt = dataSource[postion]*256+dataSource[postion+1];//日龄
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Manage.PlusAlarm_test,0,sizeof(Ac2000PlusParams->PAc2000Manage.PlusAlarm_test));
		strcat(Ac2000PlusParams->PAc2000Manage.PlusAlarm_test,SysData);	
		rt_kprintf("加湿器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=MANAGEALARMTEST_CMD;
		}


}

unsigned char TypeTemperature;
void Testtemperature(uint8_t *dataSource,uint16_t len)
{
	
		
		char tmp[8];
		int postion ;
	  int Revicedata=0;
		float TmpFloat;
		unsigned char j;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(j=0;j<6;j++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",j+1);
			strcat(SysData,tmp);
			
			Revicedata = dataSource[postion]*256+dataSource[postion+1];
			if(Revicedata <= 0x3FA)
			{
				if(Revicedata<=781)//温度大于0
				{
					TmpFloat = (float)(Revicedata*(-0.09273)+70.29818);//根据两点一线得公式再套入计算
					//TmpFloat = ((float)((float)((781-Revicedata)/11.8+0.2)))*100;
				}
				else //温度小于0
				{
					TmpFloat = (float)(Revicedata*(-0.21379)+177.00828);
					//TmpFloat =0-((float)((float)((Revicedata-781)/6.2)-1.0))*100;
					
				}
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%0.1f,",TmpFloat);
				strcat(SysData,tmp);
				strcat(SysData,tmp);
				
			}
			else //不在
			{
				strcat(SysData,"***,");
				strcat(SysData,"***,");
				TmpFloat = 9999;
			}
			Ac2000Data[ADDR_3G_1821+j] = 	TmpFloat;
			postion =postion+2;
		}
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		if(TypeTemperature==1)
		{
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTesttemperature,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTesttemperature));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTesttemperature,SysData);	
		rt_kprintf("测试温度-.PlusRefrigeration:%s\n",SysData);
	
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTTEMPERAURE_CMD;
		}
		}
		else if(TypeTemperature==2)
		{
			rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_temperature,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_temperature));
			strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_temperature,SysData);	
			rt_kprintf("测试温度-.PlusRefrigeration:%s\n",SysData);
			
			
			if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CALIBRATIONTEMPERAURE_CMD;
		}
		}
			rt_free(SysData);
		  TypeTemperature=0;
}
unsigned char moniandshidu;
unsigned char distinguish=0;
void Testhumidity(uint8_t *dataSource,uint16_t len)
{
	char tmp[8];
	int postion ;
	int Doublepostion;
	int Revicedata=0;
	float TmpFloat;
	unsigned char j;
		
	postion = 2;
		if(dataSource[postion]==0x01&&dataSource[postion+1]==0x23)
		{
		if(PLUSLevelsOfVentilationData==NULL) 
		{
			PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PLUSLevelsOfVentilationData!=NULL)
			{
				rt_memset(PLUSLevelsOfVentilationData,0,256);
				PLUSLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PLUSLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
			distinguish = 1;
		}
		else
		{
			distinguish = 0;
			rt_memset(PLUSLevelsOfVentilationData,0,256);
			PLUSLevelsOfVentilationDataLength=0;
		}
		}
		else
		{
			char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
			rt_memset(SysData,0,AliyunServiceSysLength);
			if(distinguish==1)
			{
			if(PLUSLevelsOfVentilationDataLength<=256*11)
			{
				rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
				PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
				distinguish = 0;
			}
				postion =0;
				Doublepostion=0;
				for(j=0;j<4;j++)
				{
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%d,",j+1);
						strcat(SysData,tmp);
						Revicedata = PLUSLevelsOfVentilationData[Doublepostion+8]*256+PLUSLevelsOfVentilationData[Doublepostion+8+1];
						if(Revicedata == 0)//传感器不在
						{
							TmpFloat = 9999;
							strcat(SysData,"***,");
						}
						else
						{		
							TmpFloat = (float)(Revicedata*0.16287+0.02);
							memset(tmp,0,sizeof(tmp));
							sprintf(tmp,"%0.1f,",TmpFloat);
							strcat(SysData,tmp);
						}
						  Doublepostion += 2;
						
						Revicedata = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];
						TmpFloat = (float)(Revicedata*0.16287+0.02);
						memset(tmp,0,sizeof(tmp));
						sprintf(tmp,"%0.1f,",TmpFloat);
						strcat(SysData,tmp);
						postion += 2;
						
						
				}
				
				if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		    rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_humidityCo2,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_humidityCo2));
		  	strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_humidityCo2,SysData);	
			 if(AirxxxStatus.recv_systeminfo_cmd==2)
			 {
				 AirxxxStatus.recv_systeminfo_cmd=3;
				 AirxxxStatus.aliyun_service_param_reply=CALIBRATTONHUMIDITY;
			 }
			 if(PLUSLevelsOfVentilationData!=NULL) 
			{
				rt_free(PLUSLevelsOfVentilationData);
				PLUSLevelsOfVentilationData=NULL;
			}
				PLUSLevelsOfVentilationDataLength=0;
			}
		else
		{
		postion =6;
		
		
		if(moniandshidu==1)
		{
			for(j=0;j<4;j++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",j+1);
			strcat(SysData,tmp);
			Revicedata = dataSource[postion]*256+dataSource[postion+1];
		  TmpFloat = (float)(Revicedata*0.16287+0.02);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			Ac2000Data[ADDR_3G_1814+j] = TmpFloat;//传感器
			rt_kprintf("ADDR_3GP_3114+%d=%.1f\r\n",j,TmpFloat);
			postion += 2;
	  }
		
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			rt_memset(Ac2000PlusParams->PAc2000Test.PlusTesthumidityCo2,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTesthumidityCo2));
			strcat(Ac2000PlusParams->PAc2000Test.PlusTesthumidityCo2,SysData);	
			if(AirxxxStatus.recv_systeminfo_cmd==2)
			{
				AirxxxStatus.recv_systeminfo_cmd=3;
				AirxxxStatus.aliyun_service_param_reply=TESTHUMIDITY_CO2_CMD;
			}
		}
		else if(moniandshidu==2)
		{
			
			for(j=0;j<4;j++)
		 {
		
			Revicedata = dataSource[postion]*256+dataSource[postion+1];
			TmpFloat = (float)(Revicedata*0.16287+0.02);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			
			Ac2000Data[ADDR_3G_1814+j] = TmpFloat;//传感器
			rt_kprintf("ADDR_3GP_3114+%d=%.1f\r\n",j,TmpFloat);
			postion += 2;
	   }
		
			if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
			
			rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestsimulation,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestsimulation));
			strcat(Ac2000PlusParams->PAc2000Test.PlusTestsimulation,SysData);	
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTSIMULATION_CMD;
		}
		}
		rt_kprintf("测试温度-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		moniandshidu=0;
	}
	}
}
unsigned char Relay;
void TestRelay(uint8_t *dataSource,uint16_t len)
{
	if(len==63)
	{
		if(PLUSLevelsOfVentilationData==NULL) 
		{
			PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PLUSLevelsOfVentilationData!=NULL)
			{
				rt_memset(PLUSLevelsOfVentilationData,0,256);
				PLUSLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PLUSLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);

		}
		else
		{
			rt_memset(PLUSLevelsOfVentilationData,0,256);
			PLUSLevelsOfVentilationDataLength=0;
		}
	}
	else if(len==11)
	{
		rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
		PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
		char tmp[8];
		int postion ;

		int TmpInt;
		unsigned char j;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion=0;
	
		if(Relay == 1)
		{
			
			for(j=0;j<30;j++)
		{
			
			TmpInt = PLUSLevelsOfVentilationData[postion];//序号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			TmpInt=PLUSLevelsOfVentilationData[postion+1];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			Ac2000Data[ADDR_3G_1827+j]=TmpInt;
			postion=postion+2;//22
			rt_kprintf("继电器 %d 状态=%d\r\n",j,Ac2000Data[ADDR_3G_1827+j]);
	
		}	
			
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestRelay,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestRelay));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTestRelay,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTRELAY_CMD;
		}
		Relay=0;
	 }
	else if(Relay ==2)
	{
		for(j=0;j<30;j++)
		{
			
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",j+1);
			strcat(SysData,tmp);
			
			TmpInt = PLUSLevelsOfVentilationData[postion];//序号
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			
			TmpInt=PLUSLevelsOfVentilationData[postion+1];
		  memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			Ac2000Data[ADDR_3G_1827+j]=TmpInt;
			postion=postion+2;//22
			rt_kprintf("继电器 %d 状态=%d\r\n",j,Ac2000Data[ADDR_3G_1827+j]);
		}
		
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallRelay_layout,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallRelay_layout));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallRelay_layout,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLRELAY_CMD;
		}
		Relay=0;
	}
	
		if(PLUSLevelsOfVentilationData!=NULL) 
		{
			rt_free(PLUSLevelsOfVentilationData);
			PLUSLevelsOfVentilationData=NULL;
		}
			PLUSLevelsOfVentilationDataLength=0;
	}
}
void Testdigital(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));//数字1
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));//数字2
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestdigital,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestdigital));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTestdigital,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTDIGITAL_CMD;
		}
		
	
}

void Testpulse(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;

			TmpInt = dataSource[postion]*256+dataSource[postion];
			memset(tmp,0,sizeof(tmp));//数字1
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
	

		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestpulse,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestpulse));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTestpulse,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTPULSE_CMD;
		}
		
}
void TestChicken_scale(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		if(dataSource[postion]==0xFF&&dataSource[postion+1]==0xFF)
		{
			strcat(SysData,"***,");
		}
		else
		{
			TmpInt = dataSource[postion]*256+dataSource[postion];
			memset(tmp,0,sizeof(tmp));//1
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
		postion = postion+2;
			if(dataSource[postion]==0xFF&&dataSource[postion+1]==0xFF)
		{
			strcat(SysData,"***,");
		}
		else
		{
			TmpInt = dataSource[postion]*256+dataSource[postion];
			memset(tmp,0,sizeof(tmp));//1
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestChicken_scale,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestChicken_scale));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTestChicken_scale,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTCHICKEN_CMD;
		}
		
}



void TestVersion(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =9;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,".");
		postion = postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d",TmpInt);
		strcat(SysData,tmp);
		strcat(SysData,"r");
		postion = postion+1;
		TmpInt = dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%02d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Test.PlusTestversion,0,sizeof(Ac2000PlusParams->PAc2000Test.PlusTestversion));
		strcat(Ac2000PlusParams->PAc2000Test.PlusTestversion,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=TESTVERSION_CMD;
		}
		
}



void Calibrationchicken(uint8_t *dataSource,uint16_t len)
{
	if(len==15)
	{
		if(PLUSLevelsOfVentilationData==NULL) 
		{
			PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256);
			if(PLUSLevelsOfVentilationData!=NULL)
			{
				rt_memset(PLUSLevelsOfVentilationData,0,256);
				PLUSLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PLUSLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
	
		}
		else
		{

			rt_memset(PLUSLevelsOfVentilationData,0,256);
			PLUSLevelsOfVentilationDataLength=0;
		}
	}
	
	else
	{
		if(PLUSLevelsOfVentilationDataLength<=256)
		{
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
	
		}
		else
		{

			rt_memset(PLUSLevelsOfVentilationData,0,256);
			PLUSLevelsOfVentilationDataLength=0;
		}
		
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =0;
		strcat(SysData,"1,");
		TmpFloat = FourByteToFloat(&PLUSLevelsOfVentilationData[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion =8;
		TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];
		TmpFloat = ((float)TmpInt)/1000.00f;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 4;
		strcat(SysData,"2,");
		
		TmpFloat = FourByteToFloat(&PLUSLevelsOfVentilationData[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion = 10;
		TmpInt = PLUSLevelsOfVentilationData[postion]*256+PLUSLevelsOfVentilationData[postion+1];
		TmpFloat = ((float)TmpInt)/1000.00f;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.2f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Chicken_scale,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Chicken_scale));
		strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Chicken_scale,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CALIBRATIONCHICKEN_CMD;
		}
		if(PLUSLevelsOfVentilationData!=NULL) 
		{
			rt_free(PLUSLevelsOfVentilationData);
			PLUSLevelsOfVentilationData=NULL;
		}
		PLUSLevelsOfVentilationDataLength=0;
	}
	
}

void CalibrationFeed(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion]*256+dataSource[postion+1];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
	
		TmpInt = dataSource[postion]*256+dataSource[postion+1];
		TmpFloat = ((float)TmpInt)/1000.00f;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Feed_pulse_ratio,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Feed_pulse_ratio));
		strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Feed_pulse_ratio,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CALIBRATIONFEED_CMD;
		}
	
	
}
void CalibrationWater(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpFloat = FourByteToFloat(&dataSource[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Water_pulse_ratio,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Water_pulse_ratio));
		strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Water_pulse_ratio,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CALIBRATIONWATER_CMD;
		}
	
	
}

void CalibrationCo2(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
		
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt = dataSource[postion]*256+dataSource[postion+1];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion]*256+dataSource[postion+1];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =postion+2;
		TmpInt = dataSource[postion]*256+dataSource[postion+1];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
	
	
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Co2,0,sizeof(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Co2));
		strcat(Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Co2,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=CALIBRATIONCO2_CMD;
		}
	
}

void InstallConfiguration(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =14;
		TmpInt =dataSource[postion]; //空鸡舍
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
    postion = 6;
		TmpInt =dataSource[postion]; //摄氏度/华氏度
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 12;
		TmpInt =dataSource[postion]; //通风级别
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 7;
		TmpInt =dataSource[postion]; //模拟输出1
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 8;
		TmpInt =dataSource[postion]; //模拟输出2
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =11;
		TmpInt =dataSource[postion]; //起始隧道级别
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =13;
		TmpInt =dataSource[postion]; //加热区
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =9;
		TmpInt =dataSource[postion]; //曲线
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion =17;
		for(i=0;i<10;i++)
		{
			TmpInt =dataSource[postion]*256+dataSource[postion+1]; //曲线
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion+2;
		}
		postion = 15;
		TmpInt =dataSource[postion]; //长度单位
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 16;
		TmpInt =dataSource[postion]; //风量单位
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallConfiguration,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallConfiguration));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallConfiguration,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLCONFIGURATION_CMD;
		}
}

void InstallSensor(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<9;i++)
		{	
			TmpInt =dataSource[postion];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion+1;
		}
		postion = 18;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+2;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = postion+1;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallSensor_layout,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallSensor_layout));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallSensor_layout,SysData);	
		rt_kprintf("传感器布局-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLSENSOR_CMD;
		}
}

void InstallSystem(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
	  unsigned char i;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<12;i++)
		{ 	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		for(i=0;i<12;i++)
		{	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		for(i=0;i<12;i++)
		{	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		for(i=0;i<12;i++)
		{	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		
		for(i=0;i<12;i++)
		{	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		
		for(i=0;i<10;i++)
		{	
			TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%0.1f,",TmpFloat);
			strcat(SysData,tmp);
			postion = postion+4;
		}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallSystem_parameters,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallSystem_parameters));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallSystem_parameters,SysData);	
		rt_kprintf("继电器-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLSYSTEM_CMD;
		}
}


void InstallWindcooling(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		TmpInt =dataSource[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		postion = 7;
		TmpInt = dataSource[postion]*256*256*256+dataSource[postion+1]*256*256+dataSource[postion+2]*256+dataSource[postion+3];
		TmpFloat = (float)TmpInt/10000.0f;
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		postion= postion+4;
		for(i=0;i<12;i++)
		{
			TmpInt = dataSource[postion]*256*256*256+dataSource[postion+1]*256*256+dataSource[postion+2]*256+dataSource[postion+3];
			TmpInt = TmpInt/1000;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",TmpInt);
			strcat(SysData,tmp);
			postion = postion + 4;
		}
     TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
		 TmpFloat =  TmpFloat;
		 memset(tmp,0,sizeof(tmp));
		 sprintf(tmp,"%0.1f,",TmpFloat);
		 strcat(SysData,tmp);
		 postion = postion+4;
		 TmpFloat =  orderFourByteToFloat(&dataSource[postion]);
		 TmpFloat =  TmpFloat;
		 memset(tmp,0,sizeof(tmp));
		 sprintf(tmp,"%0.2f,",TmpFloat);
		 strcat(SysData,tmp);
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallWind_cooling,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallWind_cooling));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallWind_cooling,SysData);	
		rt_kprintf("风冷-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLWINDCOLLING_CMD;
		}
		
}


void InstallVentilationlevel(uint8_t *dataSource,uint16_t len)
{
	rt_kprintf("InstallVentilationlevel:%d\n",len);
 if(len==247)
	{
			if(PLUSLevelsOfVentilationData!=NULL) 
	   {
	  	 rt_free(PLUSLevelsOfVentilationData);
	  	 PLUSLevelsOfVentilationData=NULL;
    	}
   	PLUSLevelsOfVentilationDataLength=0;
		if(PLUSLevelsOfVentilationData==NULL) 
		{
			PLUSLevelsOfVentilationData=(unsigned char *)rt_malloc(256*3);
			if(PLUSLevelsOfVentilationData!=NULL)
			{
				rt_memset(PLUSLevelsOfVentilationData,0,256*3);
				PLUSLevelsOfVentilationDataLength=0;
			}
			else
			{
				rt_kprintf("LevelsOfVentilationData malloc failure\n");
			}
		 }
		if(PLUSLevelsOfVentilationDataLength<=256*2)
		{
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
	
		}
		else
		{

			rt_memset(PLUSLevelsOfVentilationData,0,256*3);
			PLUSLevelsOfVentilationDataLength=0;
		}
	
	}
	else if(len==87)
	{
	
			rt_memcpy(&PLUSLevelsOfVentilationData[PLUSLevelsOfVentilationDataLength],&dataSource[6],len-7);
			PLUSLevelsOfVentilationDataLength=PLUSLevelsOfVentilationDataLength+(len-7);
			rt_kprintf("TmpInt- PLUSLevelsOfVentilationData:%d\n",PLUSLevelsOfVentilationData[319]);
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		unsigned char t;
		unsigned char j=0;
		unsigned char x;
	  float TmpFloat;
		char *SysData=(char *)rt_malloc(AliyunServiceSysLength*3);
		rt_memset(SysData,0,AliyunServiceSysLength*3);
		postion =0;
		for(t=0;t<20;t++)
		{
			j=0;
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",t+1);
			strcat(SysData,tmp);
			
			
			TmpInt = PLUSLevelsOfVentilationData[postion]<<8|PLUSLevelsOfVentilationData[postion+1];
		  for(i=0;i<12;i++)
		 {
			
			if((TmpInt&(1<<i))!=0)
			{
				
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",i+1);
				strcat(SysData,tmp);
			}
			else
			{
				j++;
			}
		}
	
		
		
		for(x=0;x<j;x++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",0);
			strcat(SysData,tmp);
		}


		
		
		
		postion =postion+2;
		TmpFloat = orderFourByteToFloat(&PLUSLevelsOfVentilationData[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion =postion+4;
		TmpFloat = orderFourByteToFloat(&PLUSLevelsOfVentilationData[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion =postion+4;
		TmpFloat = orderFourByteToFloat(&PLUSLevelsOfVentilationData[postion]);
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%0.1f,",TmpFloat);
		strcat(SysData,tmp);
		
		postion = postion+4;
		TmpInt = PLUSLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion = postion+1;
		TmpInt = PLUSLevelsOfVentilationData[postion];
		memset(tmp,0,sizeof(tmp));
		sprintf(tmp,"%d,",TmpInt);
		strcat(SysData,tmp);
		
		postion =postion+1;
		
		
	}
		
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallVentilation_level,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallVentilation_level));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallVentilation_level,SysData);	

	  rt_kprintf("风冷-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLVENTI_CMD;
		}
	
	}
}

void InstallRoller_blind(uint8_t *dataSource,uint16_t len)
{
		char tmp[8];
		int postion ;
		int TmpInt;
	  unsigned char i;
		
		unsigned char j=0;

		char *SysData=(char *)rt_malloc(AliyunServiceSysLength);
		rt_memset(SysData,0,AliyunServiceSysLength);
		postion =6;
		for(i=0;i<20;i++)
		{
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d,",i+1);
			strcat(SysData,tmp);
			for(j=0;j<5;j++)
			{
				TmpInt = dataSource[postion];
				memset(tmp,0,sizeof(tmp));
				sprintf(tmp,"%d,",TmpInt);
				strcat(SysData,tmp);
				postion =postion+1;
			}	
		}
		if(SysData!=NULL) SysData[strlen(SysData)-1]=0x00;
		rt_memset(Ac2000PlusParams->PAc2000Install.PlusInstallRoller_blind,0,sizeof(Ac2000PlusParams->PAc2000Install.PlusInstallRoller_blind));
		strcat(Ac2000PlusParams->PAc2000Install.PlusInstallRoller_blind,SysData);	
		rt_kprintf("风冷-.PlusRefrigeration:%s\n",SysData);
		rt_free(SysData);
		
		if(AirxxxStatus.recv_systeminfo_cmd==2)
		{
			AirxxxStatus.recv_systeminfo_cmd=3;
			AirxxxStatus.aliyun_service_param_reply=INSTALLROLLERBLIND_CMD;
		}

}


int Ac2000PlusDataParse(uint8_t *data,u16 Counter)
{
	uint8_t result=0;
	uint16_t cmd_code=data[2]*256+data[3];
	rt_kprintf("recv plus normal data:  %04X  %d\r\n\r\n",cmd_code,Counter);
	//my_log(data,Counter);
	if(Ac2000Data!=NULL)
	{
		switch(cmd_code)
		{
			case 0x0432://主屏
		{
			Parse_MainPlus(data,Counter);
		}break;
		
			case 0x0002://报警查询
		{
			Parse_WarnPlus(data,Counter);
		}break;
	//控制
			case 0x030A: //温度
		{
			temperature_plus(data,Counter);
		}break;
		
			case 0x030B://最小最大级别
		{
			Minmax_level_plus(data,Counter);
			
		}break;
			case 0x020C://湿度Co2
		{
			if(Counter == 10)  humidity_Co2(data,Counter);
		}break;
			case 0x020D://制冷
		{
			if(Counter == 77)  Refrigeration_Plus(data,Counter);
		}break;
			case 0x030E://照明
		{
			Illumination_plus(data,Counter);
		}break;
			case 0x0111://饲料
		{
			ConFeed(data,Counter);
		}break;
			case 0x0124://附加系统
		{
			Additional_system(data,Counter);
		}break;
			case 0x021A://压力
		{
			if(Counter == 25) pressure(data,Counter);
		}break;
		
			case 0x0226://加湿器
		{
			if(Counter == 77) humidifier(data,Counter);
		}break;
		//管理
			case 0x0144://死亡量
		{
			if(Counter == 11)  Manage_death(data,Counter);
		}break;
			case 0x0118://鸡存量
		{
			if(Counter == 11)  Manage_PlusPoultry_metering(data,Counter);
			
		}break;
			case 0x0136://饲料存量
		{
			if(Counter == 11) Manage_Plusfeed_stock(data,Counter);
		}break;
			case 0x0234://时间
		{
			if(Counter ==9)  Plusday_time(data,Counter);
		}break;
		case 0x0310://时间与日期
		{
			if(Counter ==9)  Manage_Plusday_time(data,Counter);
		}break;
		case 0x0119://禽群数
		{
			if(Counter ==8)  Manage_PlusFlock(data,Counter);
		}break;
		case 0x0135://新鸡群
		{
			if(Counter ==8)  Manage_PlusNewFlock(data,Counter);
		}break;
		case 0x0003://报警重置
		{
			if(Counter == 8)  Manage_PlusAlarm_reset(data,Counter);
		}break;
		case 0x011F://报警测试
		{
			if(Counter == 11) Manage_PlusTest(data,Counter);
		}break;
		//测试
		case 0x0137:
		{
			if(Counter == 19) PlusTemperature(data,Counter);
//			if(Counter == 19) Testtemperature(data,Counter);
		}break;
		case 0x0240:
		{
			if(Counter == 15) Testhumidity(data,Counter);
		}break;
		case 0x0115:
		{
//			TestRelay(data,Counter);//继电器
			PlusRelay(data,Counter);//继电器
		}break;
		case 0x0138:
		{
//			TestRelay(data,Counter);//继电器
			PlusRelay(data,Counter);//继电器
		}break;
		case 0x0141:
		{
			Testdigital(data,Counter);//数字输入
		}break;
		case 0x033C:
		{
			if(Counter ==9) Testpulse(data,Counter);
		}break;
		case 0x014A:
		{
			if(Counter ==11) TestChicken_scale(data,Counter);
		}break;
		case 0x0001:
		{
			if(Counter ==13) TestVersion(data,Counter);
		}break;
		//校准
		case 0x011e://温度校准系数
			PlusTemperatureCoefficient(data,Counter);
			break;
		case 0x0123://湿度
		{
			if(Counter ==15) Testhumidity(data,Counter);
		}break;
		case 0x0221://鸡秤
		{
			Calibrationchicken(data,Counter);
		}break;
		case 0x0133:
		{
			Calibrationchicken(data,Counter);
		}break;		
		case 0x011C:
		{
			if(Counter==11)
			CalibrationFeed(data,Counter);
		}break;
		case 0x011B:
		{
			if(Counter ==11)
			CalibrationWater(data,Counter);
		}break;
		case 0x0128:
		{
			if(Counter ==13)
			CalibrationCo2(data,Counter);
			
		}break;
		//安装
		case 0x0512:
		{
			if(Counter == 38)
				InstallConfiguration(data,Counter);
			
		}break;
		case 0x0313:
		{
			InstallVentilationlevel(data,Counter);
		
		}break;
		case 0x0216:
		{
			if(Counter == 26)
				InstallSensor(data,Counter);
		}break;
		case 0x0517:
		{
			SystemParameters(data,Counter);
			//InstallSystem(data,Counter);
		}break;
		case 0x0327:
		{
			if(Counter ==68)
				InstallWindcooling(data,Counter);
		}break;
		case 0x0214:
		{
			if(Counter == 107)
				InstallRoller_blind(data,Counter);
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



















