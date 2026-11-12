#ifndef __DATA_TABLE_H
#define __DATA_TABLE_H

#include "stm32h7xx.h"
#include <stdbool.h>
#include <rtthread.h>
#include <rtdevice.h>

/****公共变量****/
#define AliyunServiceSysLength       256
#define AliyunServiceDataLength      4096


#define SINGLE_3G_TEST       0        //test
#define SINGLE_PLATINUM_TEST 0        //test
#define SINGLE_PRO_TEST      0        //test
/****公共变量****/
#define AC2000_LINK_RETYRY     5000
#define VERSION_3G             1
#define VERSION_PLATINUM       2
#define VERSION_PLATINUM_PRO   3
#define VERSION_PLUS           4

#define IWDG_EN                1

#define AC2000CmdLength 18 
#define AC2000HouseNum 64

typedef int32_t AC2000_DATA_TYPE;
extern AC2000_DATA_TYPE *Ac2000Data;
extern AC2000_DATA_TYPE *Ac2000OldData;

union CONVERSE{
	uint16_t value;
	uint8_t data[2];
};
typedef struct AC2000
{
	uint8_t    ac2000_3g_link;
	uint8_t    ac2000_pro_link;
	uint8_t    ac2000_platinum_link;
	uint8_t    ac2000_plus_link;
	uint8_t ac2000_3g_house;
	uint8_t ac2000_pro_house;
	uint8_t ac2000_platinum_house;
	uint8_t ac2000_plus_house;
	uint8_t ac2000_type;
	uint16_t ac2000_retry;
}AC2000;

extern AC2000 ac2000_table;
int sum_ac2000(uint8_t *data,int len);
unsigned int server_crc_chk(unsigned char* data, unsigned short int length);
unsigned int crc_chk(unsigned char* data, unsigned short int length);
void AppendCRC(unsigned char *data,unsigned int crcValue);
float FourByteToFloat(unsigned char* byteArry);
float orderFourByteToFloat(unsigned char* byteArry);
void FloatToFourByte(float floatNum, unsigned char* byteArry);
float hexToFloatInverse(unsigned char *content);

void my_log(unsigned char *dataSource,uint16_t len);

extern const unsigned short AC2000WarmCode[];

enum MQTT_ID_ATTRIBUTE
{
	ID_AVER_TEMP=2001,//平均温度
	ID_OUTSIDE_TEMP,  //室外温度
	ID_ROOF_TEMP=2006,     //屋顶温度
	ID_TUNNEL_TEMP,   //隧道温度
	ID_FELT_AIR_TEMP,//体感温度
	ID_TEMP1,        //温度1
	ID_TEMP2,        //温度2
	ID_TEMP3,        //温度3
	ID_TEMP4,        //温度4
	ID_TEMP5,        //温度5
	ID_TEMP6,        //温度6
	ID_TEMP7,        //温度7
	ID_TEMP8,        //温度8
	ID_TARG_TEMP=2053,    //目标温度
	ID_HEAT_TEMP,    //加热温度
	ID_LOW_ALARM_TEMP=2056, //低温报警温度
	ID_HIGH_ALARM_TEMP, //高温报警温度
	ID_TARG_TEMP_LAG, //目标温度滞后
	ID_HEAT_TEMP_LAG, //加热温度滞后
	ID_TARG_PRESSURE=2064, //目标压力
	ID_CURRENT_NEGATIVE_PRESSURE, //当前负压
	ID_MINI_VENT_LEVE,         //最小通风级别
	ID_MAXI_VENT_LEVE,         //最大通风级别
	ID_CURRENT_VENT_LEVE,         //当前通风级别
	ID_CURRENT_AGE,         //当前日龄
	ID_CURRENT_NUM_STOCK=2072,         //当前存栏数
	ID_TODAY_DEAD_NUM,         //今日死淘数
	ID_SYSTEM_VERSION,         //系统版本
	ID_NET_INTENSITY,         //网络强度
	ID_SYSTEM_TIME=2076,         //系统时间
	ID_CONTROLLER_ALARM=2078,         //控制器报警
	ID_WATER_ALARM_STATE,         //缺水报警状态
	ID_AVER_HUM=2083,         //平均湿度
	ID_HUM1,                  //湿度1
	ID_HUM2,                  //湿度2
	ID_CO2_1=2090,            //CO2 1
	ID_CO2_2,            //CO2 2
	ID_AMMONIA1=2096,         //氨气1
	ID_OXYGEN1=2102,         //氧气1
	ID_DUST1=2108,         //粉尘1
	ID_CROSS_WIND_SPEED=2114,         //平均截面风速
	ID_VERTICAL_WIND_SPEED, //纵向风速
	ID_VARIABLE_SPEED_FAN_PERCENT1=2178, //变速风机1百分比
	ID_VARIABLE_SPEED_FAN_PERCENT2, //变速风机2百分比
	ID_WATERLINE_STATUS1=2198, //水线1状态
	ID_WATERLINE_STATUS2, //水线2状态
	ID_WATERLINE_STATUS3, //水线3状态
	ID_WATERLINE_STATUS4, //水线4状态
	ID_WATERLINE_STATUS5, //水线5状态
	ID_WATERLINE_STATUS6, //水线6状态
	ID_WATERLINE_STATUS7, //水线7状态
	ID_WATERLINE_STATUS8, //水线8状态
	ID_WATERLINE_STATUS9, //水线9状态
	ID_WATERLINE_STATUS10, //水线10状态
	ID_WATERLINE_STATUS11, //水线11状态
	ID_WATERLINE_STATUS12, //水线12状态
	ID_WATERLINE_STATUS13, //水线13状态
	ID_WATERLINE_STATUS14, //水线14状态
	ID_WATERLINE_STATUS15, //水线15状态
	ID_WATERLINE_STATUS16, //水线16状态
	ID_WATERLINE_STATUS17, //水线17状态
	ID_WATERLINE_STATUS18, //水线18状态
	ID_WATERLINE_STATUS19, //水线19状态
	ID_WATERLINE_STATUS20, //水线20状态
	ID_WATERLINE_STATUS21, //水线21状态
	ID_WATERLINE_STATUS22, //水线22状态
	ID_WATERLINE_STATUS23, //水线23状态
	ID_WATERLINE_STATUS24, //水线24状态
	ID_WATERLINE_STATUS25, //水线25状态
	ID_WATERLINE_STATUS26, //水线26状态
	ID_WATERLINE_STATUS27, //水线27状态
	ID_WATERLINE_STATUS28, //水线28状态
	ID_WATERLINE_STATUS29, //水线29状态
	ID_WATERLINE_STATUS30, //水线30状态
	ID_WATERLINE_STATUS31, //水线31状态
	ID_WATERLINE_STATUS32, //水线32状态
	ID_WATERLINE_STATUS33, //水线33状态
	ID_WATERLINE_STATUS34, //水线34状态
	ID_WATERLINE_STATUS35, //水线35状态
	ID_WATERLINE_STATUS36, //水线36状态
	ID_WATERLINE_STATUS37, //水线37状态
	ID_WATERLINE_STATUS38, //水线38状态
	ID_WATERLINE_STATUS39, //水线39状态
	ID_WATERLINE_STATUS40, //水线40状态
	ID_WATERLINE_STATUS41, //水线41状态
	ID_WATERLINE_STATUS42, //水线42状态
	ID_WATERLINE_STATUS43, //水线43状态
	ID_WATERLINE_STATUS44, //水线44状态
	ID_WATERLINE_STATUS45, //水线45状态
	ID_WATERLINE_STATUS46, //水线46状态
	ID_WATERLINE_STATUS47, //水线47状态
	ID_WATERLINE_STATUS48, //水线48状态
	ID_WATERLINE_STATUS49, //水线49状态
	ID_WATERLINE_STATUS50, //水线50状态
	ID_WATERLINE_STATUS51, //水线51状态
	ID_WATERLINE_STATUS52, //水线52状态
	ID_WATERLINE_STATUS53, //水线53状态
	ID_WATERLINE_STATUS54, //水线54状态
	ID_WATERLINE_STATUS55, //水线55状态
	ID_WATERLINE_STATUS56, //水线56状态
	ID_WATERLINE_STATUS57, //水线57状态
	ID_WATERLINE_STATUS58, //水线58状态
	ID_WATERLINE_STATUS59, //水线59状态
	ID_WATERLINE_STATUS60, //水线60状态
	ID_WATERLINE_STATUS61, //水线61状态
	ID_WATERLINE_STATUS62, //水线62状态
	ID_WATERLINE_STATUS63, //水线63状态
	ID_WATERLINE_STATUS64, //水线64状态
	ID_ILLUMINATION_PERCENT1=2302, //照明1亮度百分比
	ID_ILLUMINATION_PERCENT2, //照明2亮度百分比
	ID_RELAY_STATUS1=2312, //继电器状态1
	ID_RELAY_STATUS2, //继电器状态2
	ID_RELAY_STATUS3, //继电器状态3
	ID_RELAY_STATUS4, //继电器状态4
	ID_RELAY_STATUS5, //继电器状态5
	ID_RELAY_STATUS6, //继电器状态6
	ID_RELAY_STATUS7, //继电器状态7
	ID_RELAY_STATUS8, //继电器状态8
	ID_RELAY_STATUS9, //继电器状态9
	ID_RELAY_STATUS10, //继电器状态10
	ID_RELAY_STATUS11, //继电器状态11
	ID_RELAY_STATUS12, //继电器状态12
	ID_RELAY_STATUS13, //继电器状态13
	ID_RELAY_STATUS14, //继电器状态14
	ID_RELAY_STATUS15, //继电器状态15
	ID_RELAY_STATUS16, //继电器状态16
	ID_RELAY_STATUS17, //继电器状态17
	ID_RELAY_STATUS18, //继电器状态18
	ID_RELAY_STATUS19, //继电器状态19
	ID_RELAY_STATUS20, //继电器状态20
	ID_RELAY_STATUS21, //继电器状态21
	ID_RELAY_STATUS22, //继电器状态22
	ID_RELAY_STATUS23, //继电器状态23
	ID_RELAY_STATUS24, //继电器状态24
	ID_RELAY_STATUS25, //继电器状态25
	ID_RELAY_STATUS26, //继电器状态26
	ID_RELAY_STATUS27, //继电器状态27
	ID_RELAY_STATUS28, //继电器状态28
	ID_RELAY_STATUS29, //继电器状态29
	ID_RELAY_STATUS30, //继电器状态30
	ID_RELAY_STATUS31, //继电器状态31
	ID_RELAY_STATUS32, //继电器状态32
	ID_RELAY_STATUS33, //继电器状态33
	ID_RELAY_STATUS34, //继电器状态34
	ID_RELAY_STATUS35, //继电器状态35
	ID_RELAY_STATUS36, //继电器状态36
	ID_RELAY_STATUS37, //继电器状态37
	ID_RELAY_STATUS38, //继电器状态38
	ID_RELAY_STATUS39, //继电器状态39
	ID_RELAY_STATUS40, //继电器状态40
	ID_SMALL_WINDOW_DEGREE_OPEN1=2376, //小窗1开度
	ID_SMALL_WINDOW_DEGREE_OPEN2, //小窗2开度
	ID_AIR_PLATE_DEGREE_OPEN1=2388, //风板1开度
	ID_AIR_PLATE_DEGREE_OPEN2, //风板2开度
	ID_AIR_PLATE_DEGREE_OPEN3, //风板3开度
	ID_AIR_PLATE_DEGREE_OPEN4, //风板4开度
	ID_AIR_PLATE_DEGREE_OPEN5, //风板5开度
	ID_AIR_PLATE_DEGREE_OPEN6, //风板6开度
	ID_AIR_PLATE_DEGREE_OPEN7, //风板7开度
	ID_AIR_PLATE_DEGREE_OPEN8, //风板8开度
	ID_TUYERE_OPEN1=2400, //风口1开度
	ID_TUYERE_OPEN2, //风口2开度
	ID_TUYERE_OPEN3, //风口3开度
	ID_TUYERE_OPEN4, //风口4开度
	ID_TUYERE_OPEN5, //风口5开度
	ID_TUYERE_OPEN6, //风口6开度
	ID_WATER_METER_TODAY_DOSAGE1=2448, //水表1今日用量
	ID_WATER_METER_TODAY_DOSAGE2, //水表2今日用量
	ID_WATER_METER_TODAY_DOSAGE3, //水表3今日用量
	ID_WATER_METER_TODAY_DOSAGE4, //水表4今日用量
	ID_CURRENT_MATERIAL_WEIGHT_TOWER1=2486, //料塔1当前料重
	ID_FEED_TODAY_TOWER1, //料塔1今日上料
	ID_CONSUMES_MATERIAL_TODAY_TOWER1, //料塔1今日耗料
	ID_TOWER1_WEIGHT_SENSOR1, //料塔1传感器1料重
	ID_TOWER1_WEIGHT_SENSOR2, //料塔1传感器2料重
	ID_TOWER1_WEIGHT_SENSOR3, //料塔1传感器3料重
	ID_TOWER1_WEIGHT_SENSOR4, //料塔1传感器4料重
	ID_TOWER1_WEIGHT_SENSOR5, //料塔1传感器5料重
	ID_TOWER1_WEIGHT_SENSOR6, //料塔1传感器6料重
	ID_TOWER1_WEIGHT_SENSOR7, //料塔1传感器7料重
	ID_TOWER1_WEIGHT_SENSOR8, //料塔1传感器8料重
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME1, //料塔1今日耗料时间1
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD1, //料塔1今日耗料时段1-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME2, //料塔1今日耗料时间2
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD2, //料塔1今日耗料时段2-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME3, //料塔1今日耗料时间3
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD3, //料塔1今日耗料时段3-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME4, //料塔1今日耗料时间4
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD4, //料塔1今日耗料时段4-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME5, //料塔1今日耗料时间5
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD5, //料塔1今日耗料时段5-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME6, //料塔1今日耗料时间6
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD6, //料塔1今日耗料时段6-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME7, //料塔1今日耗料时间7
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD7, //料塔1今日耗料时段7-用量
	ID_FEED_TOWER1_TODAY_CONSUMPTION_TIME8, //料塔1今日耗料时间8
	ID_FEED_TOWER1_TODAY_CONSUMPTION_PERIOD8, //料塔1今日耗料时段8-用量
	ID_CURRENT_MATERIAL_WEIGHT_TOWER2=2530, //料塔2当前料重
	ID_FEED_TODAY_TOWER2, //料塔2今日上料
	ID_CONSUMES_MATERIAL_TODAY_TOWER2, //料塔2今日耗料
	ID_TOWER2_WEIGHT_SENSOR1, //料塔2传感器1料重
	ID_TOWER2_WEIGHT_SENSOR2, //料塔2传感器2料重
	ID_TOWER2_WEIGHT_SENSOR3, //料塔2传感器3料重
	ID_TOWER2_WEIGHT_SENSOR4, //料塔2传感器4料重
	ID_TOWER2_WEIGHT_SENSOR5, //料塔2传感器5料重
	ID_TOWER2_WEIGHT_SENSOR6, //料塔2传感器6料重
	ID_TOWER2_WEIGHT_SENSOR7, //料塔2传感器7料重
	ID_TOWER2_WEIGHT_SENSOR8, //料塔2传感器8料重
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME1, //料塔2今日耗料时间1
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD1, //料塔2今日耗料时段1-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME2, //料塔2今日耗料时间2
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD2, //料塔2今日耗料时段2-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME3, //料塔2今日耗料时间3
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD3, //料塔2今日耗料时段3-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME4, //料塔2今日耗料时间4
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD4, //料塔2今日耗料时段4-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME5, //料塔2今日耗料时间5
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD5, //料塔2今日耗料时段5-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME6, //料塔2今日耗料时间6
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD6, //料塔2今日耗料时段6-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME7, //料塔2今日耗料时间7
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD7, //料塔2今日耗料时段7-用量
	ID_FEED_TOWER2_TODAY_CONSUMPTION_TIME8, //料塔2今日耗料时间8
	ID_FEED_TOWER2_TODAY_CONSUMPTION_PERIOD8, //料塔2今日耗料时段8-用量
};

/****3G版变量****/
enum AC2000_3G_DATA
{
	ADDR_3G_1801=0	,//3G版报警状态
	ADDR_3G_1802	,//当前静压
	ADDR_3G_1803	,//目标静压
	ADDR_3G_1804	,//日龄
	ADDR_3G_1805	,//通风等级
	ADDR_3G_1806	,//平均湿度
	ADDR_3G_1807	,//平均温度
	ADDR_3G_1808	,//室外温度
	ADDR_3G_1809	,//屋顶温度
	ADDR_3G_1810	,//隧道温度
	ADDR_3G_1811	,//冷风温度
	ADDR_3G_1812	,//满鸡舍温度
	ADDR_3G_1813	,//CO2
	ADDR_3G_1814	,//湿度1
	ADDR_3G_1815	,//湿度2
	ADDR_3G_1816	,//外部湿度
	ADDR_3G_1817	,//水表1
	ADDR_3G_1818	,//水表2
	ADDR_3G_1819	,//饲料1
	ADDR_3G_1820	,//饲料2
	ADDR_3G_1821	,//温度1
	ADDR_3G_1822	,//温度2
	ADDR_3G_1823	,//温度3
	ADDR_3G_1824	,//温度4
	ADDR_3G_1825	,//温度5
	ADDR_3G_1826	,//温度6
	ADDR_3G_1827	,//继电器状态1
	ADDR_3G_1828	,//继电器状态2
	ADDR_3G_1829	,//继电器状态3
	ADDR_3G_1830	,//继电器状态4
	ADDR_3G_1831	,//继电器状态5
	ADDR_3G_1832	,//继电器状态6
	ADDR_3G_1833	,//继电器状态7
	ADDR_3G_1834	,//继电器状态8
	ADDR_3G_1835	,//继电器状态9
	ADDR_3G_1836	,//继电器状态10
	ADDR_3G_1837	,//继电器状态11
	ADDR_3G_1838	,//继电器状态12
	ADDR_3G_1839	,//继电器状态13
	ADDR_3G_1840	,//继电器状态14
	ADDR_3G_1841	,//继电器状态15
	ADDR_3G_1842	,//继电器状态16
	ADDR_3G_1843	,//继电器状态17
	ADDR_3G_1844	,//继电器状态18
	ADDR_3G_1845	,//继电器状态19
	ADDR_3G_1846	,//继电器状态20
	ADDR_3G_1847	,//目标温度
	ADDR_3G_1848	,//加热温度
	ADDR_3G_1849	,//通道温度(目标隧道)
	ADDR_3G_1850	,//低温报警温度
	ADDR_3G_1851	,//高温报警温度
	ADDR_3G_1852	,//最小通风级别
	ADDR_3G_1853	,//最大通风级别
	ADDR_3G_1854	,//扩展温度
	ADDR_3G_1855	,//扩展湿度
	ADDR_3G_1856	,//风速
	ADDR_3G_1857	,//水表读数
	ADDR_3G_1858	,//水今日用量
	ADDR_3G_1859	,//CO2
	ADDR_3G_1860	,//亮度
	ADDR_3G_1861	,//体感温度
	ADDR_3G_1862	,//开关量1-状态
	ADDR_3G_1863	,//开关量2-状态
	ADDR_3G_1864	,//开关量3-状态
	ADDR_3G_1865	,//开关量4-状态
	ADDR_3G_1866	,//饮水监测状态 有无报警
	ADDR_3G_1867	,//水线1-状态
	ADDR_3G_1868	,//水线2-状态
	ADDR_3G_1869	,//水线3-状态
	ADDR_3G_1870	,//水线4-状态
	ADDR_3G_1871	,//水线5-状态
	ADDR_3G_1872	,//水线6-状态
	ADDR_3G_1873	,//水线7-状态
	ADDR_3G_1874	,//水线8-状态
	ADDR_3G_1875	,//水线9-状态
	ADDR_3G_1876	,//水线10-状态
	ADDR_3G_1877	,//水线11-状态
	ADDR_3G_1878	,//水线12-状态
	ADDR_3G_1879	,//水线13-状态
	ADDR_3G_1880	,//水线14-状态
	ADDR_3G_1881	,//水线15-状态
	ADDR_3G_1882	,//水线16-状态
	ADDR_3G_1883	,//水线17-状态
	ADDR_3G_1884	,//水线18-状态
	ADDR_3G_1885	,//水线19-状态
	ADDR_3G_1886	,//水线20-状态
	ADDR_3G_1887	,//水线21-状态
	ADDR_3G_1888	,//水线22-状态
	ADDR_3G_1889	,//水线23-状态
	ADDR_3G_1890	,//水线24-状态
	ADDR_3G_1891	,//水线25-状态
	ADDR_3G_1892	,//水线26-状态
	ADDR_3G_1893	,//水线27-状态
	ADDR_3G_1894	,//水线28-状态
	ADDR_3G_1895	,//水线29-状态
	ADDR_3G_1896	,//水线30-状态
	ADDR_3G_1897	,//水线31-状态
	ADDR_3G_1898	,//水线32-状态
	ADDR_3G_1899	,//水线33-状态
	ADDR_3G_1900	,//水线34-状态
	ADDR_3G_1901	,//水线35-状态
	ADDR_3G_1902	,//水线36-状态
	ADDR_3G_1903	,//水线37-状态
	ADDR_3G_1904	,//水线38-状态
	ADDR_3G_1905	,//水线39-状态
	ADDR_3G_1906	,//水线40-状态
	ADDR_3G_1907	,//水线41-状态
	ADDR_3G_1908	,//水线42-状态
	ADDR_3G_1909	,//水线43-状态
	ADDR_3G_1910	,//水线44-状态
	ADDR_3G_1911	,//水线45-状态
	ADDR_3G_1912	,//水线46-状态
	ADDR_3G_1913	,//水线47-状态
	ADDR_3G_1914	,//水线48-状态
	ADDR_3G_1915	,//本次运行时间
	ADDR_3G_1916	,//信号强度
	ADDR_3G_1917	,//PC连接状态
	ADDR_3G_1918	,//温湿度传感器状态
	ADDR_3G_1919	,//风速传感器状态
	ADDR_3G_1920	,//水表传感器状态
	ADDR_3G_1921	,//CO2传感器状态
	ADDR_3G_1922	,//光照传感器状态
	ADDR_3G_1923	,//缺水报警连接状态
	ADDR_3G_1924	,//缺水报警延时
	ADDR_3G_1925	,//缺水报警监测
	ADDR_3G_1926	,//缺水报警采集盒通信
	ADDR_3G_1927	,//料塔当前料重
	ADDR_3G_1928	,//料塔今日用量
	ADDR_3G_1929	,//料塔今日上量
	ADDR_3G_1930	,//料塔累计用量
	ADDR_3G_1931	,//称重系统连接状态
	ADDR_3G_1932  ,//制冷温度
};

enum AC2000_3G_ALARM
{
	ALARM_3G_11515=0	,//AC2000通信失败，请检查线路连接。
	ALARM_3G_11433	,//水线一列-1层(1路) 缺水报警
	ALARM_3G_11434	,//水线一列-2层(2路) 缺水报警
	ALARM_3G_11435	,//水线一列-3层(3路) 缺水报警
	ALARM_3G_11436	,//水线一列-4层(4路) 缺水报警
	ALARM_3G_11437	,//水线一列-5层(5路) 缺水报警
	ALARM_3G_11438	,//水线一列-6层(6路) 缺水报警
	ALARM_3G_11439	,//水线一列-7层(7路) 缺水报警
	ALARM_3G_11440	,//水线一列-8层(8路) 缺水报警
	ALARM_3G_11441	,//水线二列-1层(9路) 缺水报警
	ALARM_3G_11442	,//水线二列-2层(10路) 缺水报警
	ALARM_3G_11443	,//水线二列-3层(11路) 缺水报警
	ALARM_3G_11444	,//水线二列-4层(12路) 缺水报警
	ALARM_3G_11445	,//水线二列-5层(13路) 缺水报警
	ALARM_3G_11446	,//水线二列-6层(14路) 缺水报警
	ALARM_3G_11447	,//水线二列-7层(15路) 缺水报警
	ALARM_3G_11448	,//水线二列-8层(16路) 缺水报警
	ALARM_3G_11449	,//水线三列-1层(17路) 缺水报警
	ALARM_3G_11450	,//水线三列-2层(18路) 缺水报警
	ALARM_3G_11451	,//水线三列-3层(19路) 缺水报警
	ALARM_3G_11452	,//水线三列-4层(20路) 缺水报警
	ALARM_3G_11453	,//水线三列-5层(21路) 缺水报警
	ALARM_3G_11454	,//水线三列-6层(22路) 缺水报警
	ALARM_3G_11455	,//水线三列-7层(23路) 缺水报警
	ALARM_3G_11456	,//水线三列-8层(24路) 缺水报警
	ALARM_3G_11457	,//水线四列-1层(25路) 缺水报警
	ALARM_3G_11458	,//水线四列-2层(26路) 缺水报警
	ALARM_3G_11459	,//水线四列-3层(27路) 缺水报警
	ALARM_3G_11460	,//水线四列-4层(28路) 缺水报警
	ALARM_3G_11461	,//水线四列-5层(29路) 缺水报警
	ALARM_3G_11462	,//水线四列-6层(30路) 缺水报警
	ALARM_3G_11463	,//水线四列-7层(31路) 缺水报警
	ALARM_3G_11464	,//水线四列-8层(32路) 缺水报警
	ALARM_3G_11465	,//水线五列-1层(33路) 缺水报警
	ALARM_3G_11466	,//水线五列-2层(34路) 缺水报警
	ALARM_3G_11467	,//水线五列-3层(35路) 缺水报警
	ALARM_3G_11468	,//水线五列-4层(36路) 缺水报警
	ALARM_3G_11469	,//水线五列-5层(37路) 缺水报警
	ALARM_3G_11470	,//水线五列-6层(38路) 缺水报警
	ALARM_3G_11471	,//水线五列-7层(39路) 缺水报警
	ALARM_3G_11472	,//水线五列-8层(40路) 缺水报警
	ALARM_3G_11473	,//水线六列-1层(41路) 缺水报警
	ALARM_3G_11474	,//水线六列-2层(42路) 缺水报警
	ALARM_3G_11475	,//水线六列-3层(43路) 缺水报警
	ALARM_3G_11476	,//水线六列-4层(44路) 缺水报警
	ALARM_3G_11477	,//水线六列-5层(45路) 缺水报警
	ALARM_3G_11478	,//水线六列-6层(46路) 缺水报警
	ALARM_3G_11479	,//水线六列-7层(47路) 缺水报警
	ALARM_3G_11480	,//水线六列-8层(48路) 缺水报警
	ALARM_3G_11518	,//风机动力电箱停电报警
	ALARM_3G_11519	,//环控电箱停电报警
	ALARM_3G_11520	,//XXX电箱停电报警
	ALARM_3G_11521	,//IO 4 断开报警
	ALARM_3G_11487	,//扩展温度传感器1 通信故障报警
	ALARM_3G_11492	,//水表1 通信故障报警
	ALARM_3G_11493	,//智能缺水报警器 通信故障报警
	ALARM_3G_11500	,//二氧化碳传感器1 通信故障报警
	ALARM_3G_11497	,//风速传感器1 通信故障报警
	ALARM_3G_11510	,//亮度传感器1 通信故障报警
				
	ALARM_3G_12000	,//控制器 异常报警 报警
	ALARM_3G_12001	,//控制器 开关改变了 报警
	ALARM_3G_12002	,//控制器 温度传感器1断开了 报警
	ALARM_3G_12003	,//控制器 温度传感器2断开了 报警
	ALARM_3G_12004	,//控制器 温度传感器3断开了 报警
	ALARM_3G_12005	,//控制器 温度传感器4断开了 报警
	ALARM_3G_12006	,//控制器 温度传感器5断开了 报警
	ALARM_3G_12007	,//控制器 温度传感器6断开了 报警
	ALARM_3G_12008	,//控制器 温度传感器7断开了 报警
	ALARM_3G_12009	,//控制器 温度传感器8断开了 报警
	ALARM_3G_12010	,//控制器 温度传感器9断开了 报警
	ALARM_3G_12011	,//控制器 温度传感器10断开了 报警
	ALARM_3G_12012	,//控制器 传感器1定义错误 报警
	ALARM_3G_12013	,//控制器 传感器2定义错误 报警
	ALARM_3G_12014	,//控制器 传感器3定义错误 报警
	ALARM_3G_12015	,//控制器 传感器4定义错误 报警
	ALARM_3G_12016	,//控制器 传感器5定义错误 报警
	ALARM_3G_12017	,//控制器 传感器6定义错误 报警
	ALARM_3G_12018	,//控制器 传感器7定义错误 报警
	ALARM_3G_12019	,//控制器 传感器8定义错误 报警
	ALARM_3G_12020	,//控制器 传感器9定义错误 报警
	ALARM_3G_12021	,//控制器 传感器10定义错误 报警
	ALARM_3G_12022	,//控制器 平均传感器故障 报警
	ALARM_3G_12023	,//控制器 温度传感器没有定义 报警
	ALARM_3G_12024	,//控制器 模拟输入卡故障 报警
	ALARM_3G_12025	,//控制器 模拟输出卡故障 报警
	ALARM_3G_12026	,//控制器 数字输入卡故障 报警
	ALARM_3G_12027	,//控制器 高温 报警
	ALARM_3G_12028	,//控制器 传感器1高温 报警
	ALARM_3G_12029	,//控制器 传感器2高温 报警
	ALARM_3G_12030	,//控制器 传感器3高温 报警
	ALARM_3G_12031	,//控制器 传感器4高温 报警
	ALARM_3G_12032	,//控制器 传感器5高温 报警
	ALARM_3G_12033	,//控制器 传感器6高温 报警
	ALARM_3G_12034	,//控制器 传感器7高温 报警
	ALARM_3G_12035	,//控制器 传感器8高温 报警
	ALARM_3G_12036	,//控制器 传感器9高温 报警
	ALARM_3G_12037	,//控制器 低温 报警
	ALARM_3G_12038	,//控制器 传感器1低温  报警
	ALARM_3G_12039	,//控制器 传感器2低温 报警
	ALARM_3G_12040	,//控制器 传感器3低温 报警
	ALARM_3G_12041	,//控制器 传感器4低温 报警
	ALARM_3G_12042	,//控制器 传感器5低温 报警
	ALARM_3G_12043	,//控制器 传感器6低温 报警
	ALARM_3G_12044	,//控制器 传感器7低温 报警
	ALARM_3G_12045	,//控制器 传感器8低温 报警
	ALARM_3G_12046	,//控制器 传感器9低温 报警
	ALARM_3G_12047	,//控制器 舍外温度错误 报警
	ALARM_3G_12048	,//控制器 低于最小气量 报警
	ALARM_3G_12049	,//控制器 低压 报警
	ALARM_3G_12050	,//控制器 CO2通信失败 报警
	ALARM_3G_12051	,//控制器 湿度传感器1故障 报警
	ALARM_3G_12052	,//控制器 备用报警1 报警
	ALARM_3G_12053	,//控制器 低水压 报警
	ALARM_3G_12054	,//控制器 线路断流器故障 报警
	ALARM_3G_12055	,//控制器 报警测试 报警
	ALARM_3G_12056	,//控制器 高压 报警
	ALARM_3G_12057	,//控制器 紧急状态压力 报警
	ALARM_3G_12058	,//控制器 湿度传感器2故障 报警
				
	ALARM_3G_13069	,//料塔称重通信故障
};


enum AC2000_3G_DATA_MAP
{
	ADDR_1132=0	,//3G版报警状态
	ADDR_1133	,//当前静压
	ADDR_1134	,//目标静压
	ADDR_1135	,//日龄
	ADDR_1136	,//通风等级
	ADDR_1137	,//平均湿度
	ADDR_1138 ,//平均温度
	ADDR_1139	,//室外温度
	ADDR_1140	,//屋顶温度
	ADDR_1141	,//隧道温度
	ADDR_1142	,//冷风温度
	ADDR_1143	,//满鸡舍温度
	ADDR_1144	,//CO2
	ADDR_1145	,//湿度1
	ADDR_1146	,//湿度2
	ADDR_1147	,//外部湿度
	ADDR_1148	,//水表1
	ADDR_1149	,//水表2
	ADDR_1150	,//饲料1
	ADDR_1151	,//饲料2
	ADDR_1152	,//温度1
	ADDR_1153	,//温度2
	ADDR_1154	,//温度3
	ADDR_1155	,//温度4
	ADDR_1156	,//温度5
	ADDR_1157	,//温度6
	ADDR_1158	,//继电器状态1
	ADDR_1159	,//继电器状态2
	ADDR_1160	,//继电器状态3
	ADDR_1161	,//继电器状态4
	ADDR_1162	,//继电器状态5
	ADDR_1163	,//继电器状态6
	ADDR_1164	,//继电器状态7
	ADDR_1165	,//继电器状态8
	ADDR_1166	,//继电器状态9
	ADDR_1167	,//继电器状态10
	ADDR_1168	,//继电器状态11
	ADDR_1169	,//继电器状态12
	ADDR_1170	,//继电器状态13
	ADDR_1171	,//继电器状态14
	ADDR_1172	,//继电器状态15
	ADDR_1173	,//继电器状态16
	ADDR_1174	,//继电器状态17
	ADDR_1175	,//继电器状态18
	ADDR_1176	,//继电器状态19
	ADDR_1177	,//继电器状态20
	ADDR_1198	,//目标温度
	ADDR_1199	,//加热温度
	ADDR_1200	,//通道温度(目标隧道)
	ADDR_1201	,//低温报警温度
	ADDR_1202	,//高温报警温度
	ADDR_1203	,//最小通风级别
	ADDR_1204	,//最大通风级别
	ADDR_1205	,//扩展温度
	ADDR_1206	,//扩展湿度
	ADDR_1207	,//风速
	ADDR_1208	,//水表读数
	ADDR_1209	,//水今日用量
	ADDR_1210	,//CO2
	ADDR_1211	,//亮度
	ADDR_1212	,//体感温度
	ADDR_1213	,//开关量1-状态
	ADDR_1214	,//开关量2-状态
	ADDR_1215	,//开关量3-状态
	ADDR_1216	,//开关量4-状态
	ADDR_1217	,//饮水监测状态
	ADDR_1218	,//水线1-状态
	ADDR_1219	,//水线2-状态
	ADDR_1220	,//水线3-状态
	ADDR_1221	,//水线4-状态
	ADDR_1222	,//水线5-状态
	ADDR_1223	,//水线6-状态
	ADDR_1224	,//水线7-状态
	ADDR_1225	,//水线8-状态
	ADDR_1226	,//水线9-状态
	ADDR_1227	,//水线10-状态
	ADDR_1228	,//水线11-状态
	ADDR_1229	,//水线12-状态
	ADDR_1230	,//水线13-状态
	ADDR_1231	,//水线14-状态
	ADDR_1232	,//水线15-状态
	ADDR_1233	,//水线16-状态
	ADDR_1234	,//水线17-状态
	ADDR_1235	,//水线18-状态
	ADDR_1236	,//水线19-状态
	ADDR_1237	,//水线20-状态
	ADDR_1238	,//水线21-状态
	ADDR_1239	,//水线22-状态
	ADDR_1240	,//水线23-状态
	ADDR_1241	,//水线24-状态
	ADDR_1242	,//水线25-状态
	ADDR_1243	,//水线26-状态
	ADDR_1244	,//水线27-状态
	ADDR_1245	,//水线28-状态
	ADDR_1246	,//水线29-状态
	ADDR_1247	,//水线30-状态
	ADDR_1248	,//水线31-状态
	ADDR_1249	,//水线32-状态
	ADDR_1250	,//水线33-状态
	ADDR_1251	,//水线34-状态
	ADDR_1252	,//水线35-状态
	ADDR_1253	,//水线36-状态
	ADDR_1254	,//水线37-状态
	ADDR_1255	,//水线38-状态
	ADDR_1256	,//水线39-状态
	ADDR_1257	,//水线40-状态
	ADDR_1258	,//水线41-状态
	ADDR_1259	,//水线42-状态
	ADDR_1260	,//水线43-状态
	ADDR_1261	,//水线44-状态
	ADDR_1262	,//水线45-状态
	ADDR_1263	,//水线46-状态
	ADDR_1264	,//水线47-状态
	ADDR_1265	,//水线48-状态
	ADDR_1266	,//本次运行时间
	ADDR_1267	,//信号强度
	ADDR_1268	,//PC连接状态
	ADDR_1269	,//温湿度传感器状态
	ADDR_1270	,//风速传感器状态
	ADDR_1271	,//水表传感器状态
	ADDR_1272	,//CO2传感器状态
	ADDR_1273	,//光照传感器状态
	ADDR_1274	,//缺水报警连接状态
	ADDR_1275	,//缺水报警延时
	ADDR_1276	,//缺水报警监测
	ADDR_1277	,//缺水报警采集盒通信
	ADDR_1278	,//料塔当前料重
	ADDR_1279	,//料塔今日用量
	ADDR_1280	,//料塔今日上量
	ADDR_1281	,//料塔累计用量
	ADDR_1282 ,//制冷温度
	ADDR_1283	,//称重系统连接状态
};


#define BASE_DATA_ID_NUM_3G   (ADDR_1283+1)
#define BASE_DATA_ID_3G       1132
#define BASE_ALARM_ID_NUM_3G  (ALARM_3G_13069+1)
#define BASE_ALARM_ID_3G      12000
#define ALARM_NUM_3G               (ALARM_3G_13069-ALARM_3G_11515+1)   //总报警数量
#define AC2000_ALARM_NUM_3G        (ALARM_3G_12058-ALARM_3G_12000+1)   //AC2000报警数量

extern const AC2000_DATA_TYPE id_value_dot_range_3g[][2];
extern uint16_t AlarmCode3g[ALARM_NUM_3G][3];


/*****PLATINUM版变量****/
enum AC2000_PLATINUM_DATA
{
	ADDR_PLATINUM_2401	,//	平均温度
	ADDR_PLATINUM_2402	,//	室外温度
	ADDR_PLATINUM_2403	,//	通风级别
	ADDR_PLATINUM_2404	,//	当前静压
	ADDR_PLATINUM_2405	,//	日龄
	ADDR_PLATINUM_2406	,//	隧道温度/通道温度
	ADDR_PLATINUM_2407	,//	湿度1
	ADDR_PLATINUM_2408	,//	湿度2
	ADDR_PLATINUM_2409	,//	平均湿度
	ADDR_PLATINUM_2410	,//	外部湿度
	ADDR_PLATINUM_2411	,//	CO2
	ADDR_PLATINUM_2412	,//	传感器1
	ADDR_PLATINUM_2413	,//	传感器2
	ADDR_PLATINUM_2414	,//	传感器3
	ADDR_PLATINUM_2415	,//	传感器4
	ADDR_PLATINUM_2416	,//	传感器5
	ADDR_PLATINUM_2417	,//	传感器6
	ADDR_PLATINUM_2418	,//	传感器7
	ADDR_PLATINUM_2419	,//	传感器8
	ADDR_PLATINUM_2420	,//	传感器9
	ADDR_PLATINUM_2421	,//	AC2000报警状态
	ADDR_PLATINUM_2422	,//	水表1
	ADDR_PLATINUM_2423	,//	水表2
	ADDR_PLATINUM_2424	,//	当天喂料1
	ADDR_PLATINUM_2425	,//	料塔1余料
	ADDR_PLATINUM_2426	,//	目标温度
	ADDR_PLATINUM_2427	,//	加热温度
	ADDR_PLATINUM_2428	,//	隧道温度
	ADDR_PLATINUM_2429	,//	低温报警温度
	ADDR_PLATINUM_2430	,//	高温报警温度
	ADDR_PLATINUM_2431	,//	目标静压
	ADDR_PLATINUM_2432	,//	最小通风级别
	ADDR_PLATINUM_2433	,//	最大通风级别
	ADDR_PLATINUM_2434	,//	继电器状态1
	ADDR_PLATINUM_2435	,//	继电器状态2
	ADDR_PLATINUM_2436	,//	继电器状态3
	ADDR_PLATINUM_2437	,//	继电器状态4
	ADDR_PLATINUM_2438	,//	继电器状态5
	ADDR_PLATINUM_2439	,//	继电器状态6
	ADDR_PLATINUM_2440	,//	继电器状态7
	ADDR_PLATINUM_2441	,//	继电器状态8
	ADDR_PLATINUM_2442	,//	继电器状态9
	ADDR_PLATINUM_2443	,//	继电器状态10
	ADDR_PLATINUM_2444	,//	继电器状态11
	ADDR_PLATINUM_2445	,//	继电器状态12
	ADDR_PLATINUM_2446	,//	继电器状态13
	ADDR_PLATINUM_2447	,//	继电器状态14
	ADDR_PLATINUM_2448	,//	继电器状态15
	ADDR_PLATINUM_2449	,//	继电器状态16
	ADDR_PLATINUM_2450	,//	继电器状态17
	ADDR_PLATINUM_2451	,//	继电器状态18
	ADDR_PLATINUM_2452	,//	继电器状态19
	ADDR_PLATINUM_2453	,//	继电器状态20
	ADDR_PLATINUM_2454	,//	继电器状态21
	ADDR_PLATINUM_2455	,//	继电器状态22
	ADDR_PLATINUM_2456	,//	继电器状态23
	ADDR_PLATINUM_2457	,//	继电器状态24
	ADDR_PLATINUM_2458	,//	继电器状态25
	ADDR_PLATINUM_2459	,//	继电器状态26
	ADDR_PLATINUM_2460	,//	继电器状态27
	ADDR_PLATINUM_2461	,//	继电器状态28
	ADDR_PLATINUM_2462	,//	继电器状态29
	ADDR_PLATINUM_2463	,//	继电器状态30
	ADDR_PLATINUM_2464	,//	扩展温度
	ADDR_PLATINUM_2465	,//	扩展湿度
	ADDR_PLATINUM_2466	,//	风速
	ADDR_PLATINUM_2467	,//	水表读数
	ADDR_PLATINUM_2468	,//	水今日用量
	ADDR_PLATINUM_2469	,//	CO2
	ADDR_PLATINUM_2470	,//	亮度
	ADDR_PLATINUM_2471	,//	粉尘PM2.5
	ADDR_PLATINUM_2472	,//	粉尘PM10
	ADDR_PLATINUM_2473	,//	电表1读数
	ADDR_PLATINUM_2474	,//	电表1今日用量
	ADDR_PLATINUM_2475	,//	氨气
	ADDR_PLATINUM_2476	,//	料塔当前料重
	ADDR_PLATINUM_2477	,//	料塔今日用料
	ADDR_PLATINUM_2478	,//	料塔今日上料
	ADDR_PLATINUM_2479	,//	料塔累计用料
	ADDR_PLATINUM_2480	,//	体感温度
	ADDR_PLATINUM_2481	,//	开关量1-状态
	ADDR_PLATINUM_2482	,//	开关量2-状态
	ADDR_PLATINUM_2483	,//	开关量3-状态
	ADDR_PLATINUM_2484	,//	开关量4-状态
	ADDR_PLATINUM_2485	,//	饮水监测状态
	ADDR_PLATINUM_2486	,//	水线1-状态
	ADDR_PLATINUM_2487	,//	水线2-状态
	ADDR_PLATINUM_2488	,//	水线3-状态
	ADDR_PLATINUM_2489	,//	水线4-状态
	ADDR_PLATINUM_2490	,//	水线5-状态
	ADDR_PLATINUM_2491	,//	水线6-状态
	ADDR_PLATINUM_2492	,//	水线7-状态
	ADDR_PLATINUM_2493	,//	水线8-状态
	ADDR_PLATINUM_2494	,//	水线9-状态
	ADDR_PLATINUM_2495	,//	水线10-状态
	ADDR_PLATINUM_2496	,//	水线11-状态
	ADDR_PLATINUM_2497	,//	水线12-状态
	ADDR_PLATINUM_2498	,//	水线13-状态
	ADDR_PLATINUM_2499	,//	水线14-状态
	ADDR_PLATINUM_2500	,//	水线15-状态
	ADDR_PLATINUM_2501	,//	水线16-状态
	ADDR_PLATINUM_2502	,//	水线17-状态
	ADDR_PLATINUM_2503	,//	水线18-状态
	ADDR_PLATINUM_2504	,//	水线19-状态
	ADDR_PLATINUM_2505	,//	水线20-状态
	ADDR_PLATINUM_2506	,//	水线21-状态
	ADDR_PLATINUM_2507	,//	水线22-状态
	ADDR_PLATINUM_2508	,//	水线23-状态
	ADDR_PLATINUM_2509	,//	水线24-状态
	ADDR_PLATINUM_2510	,//	水线25-状态
	ADDR_PLATINUM_2511	,//	水线26-状态
	ADDR_PLATINUM_2512	,//	水线27-状态
	ADDR_PLATINUM_2513	,//	水线28-状态
	ADDR_PLATINUM_2514	,//	水线29-状态
	ADDR_PLATINUM_2515	,//	水线30-状态
	ADDR_PLATINUM_2516	,//	水线31-状态
	ADDR_PLATINUM_2517	,//	水线32-状态
	ADDR_PLATINUM_2518	,//	水线33-状态
	ADDR_PLATINUM_2519	,//	水线34-状态
	ADDR_PLATINUM_2520	,//	水线35-状态
	ADDR_PLATINUM_2521	,//	水线36-状态
	ADDR_PLATINUM_2522	,//	水线37-状态
	ADDR_PLATINUM_2523	,//	水线38-状态
	ADDR_PLATINUM_2524	,//	水线39-状态
	ADDR_PLATINUM_2525	,//	水线40-状态
	ADDR_PLATINUM_2526	,//	水线41-状态
	ADDR_PLATINUM_2527	,//	水线42-状态
	ADDR_PLATINUM_2528	,//	水线43-状态
	ADDR_PLATINUM_2529	,//	水线44-状态
	ADDR_PLATINUM_2530	,//	水线45-状态
	ADDR_PLATINUM_2531	,//	水线46-状态
	ADDR_PLATINUM_2532	,//	水线47-状态
	ADDR_PLATINUM_2533	,//	水线48-状态
	ADDR_PLATINUM_2534	,//	缺水报警延时
	ADDR_PLATINUM_2535	,//	缺水报警监测
	ADDR_PLATINUM_2536	,//	缺水报警采集盒通信
	ADDR_PLATINUM_2537	,//	本次运行时间
	ADDR_PLATINUM_2538	,//	信号强度
	ADDR_PLATINUM_2539	,//	PC连接状态
	ADDR_PLATINUM_2540	,//	温湿度传感器状态
	ADDR_PLATINUM_2541	,//	风速传感器状态
	ADDR_PLATINUM_2542	,//	水表传感器状态
	ADDR_PLATINUM_2543	,//	CO2传感器状态
	ADDR_PLATINUM_2544	,//	光照传感器状态
	ADDR_PLATINUM_2545	,//	粉尘传感器状态
	ADDR_PLATINUM_2546	,//	电表传感器状态
	ADDR_PLATINUM_2547	,//	氨气传感器状态
	ADDR_PLATINUM_2548	,//	称重传感器状态
	ADDR_PLATINUM_2549	,//	缺水传感器状态
	ADDR_PLATINUM_2550  ,//空
	ADDR_PLATINUM_2551  ,//继电器状态31
	ADDR_PLATINUM_2552  ,//继电器状态32
	ADDR_PLATINUM_2553  ,//继电器状态33
	ADDR_PLATINUM_2554  ,//继电器状态34
	ADDR_PLATINUM_2555  ,//继电器状态35
	ADDR_PLATINUM_2556  ,//继电器状态36
	ADDR_PLATINUM_2557  ,//继电器状态37
	ADDR_PLATINUM_2558  ,//继电器状态38
	ADDR_PLATINUM_2559  ,//继电器状态39
	ADDR_PLATINUM_2560  ,//继电器状态40
};


enum AC2000_PLATINUM_ALARM
{
	ALARM_PLATINUM_13000=0,//	控制器 异常报警 报警
	ALARM_PLATINUM_13001,//	控制器 开关改变了 报警
	ALARM_PLATINUM_13002,//	控制器 温度传感器1断开了 报警
	ALARM_PLATINUM_13003,//	控制器 温度传感器2断开了 报警
	ALARM_PLATINUM_13004,//	控制器 温度传感器3断开了 报警
	ALARM_PLATINUM_13005,//	控制器 温度传感器4断开了 报警
	ALARM_PLATINUM_13006,//	控制器 温度传感器5断开了 报警
	ALARM_PLATINUM_13007,//	控制器 温度传感器6断开了 报警
	ALARM_PLATINUM_13008,//	控制器 温度传感器7断开了 报警
	ALARM_PLATINUM_13009,//	控制器 温度传感器8断开了 报警
	ALARM_PLATINUM_13010,//	控制器 温度传感器9断开了 报警
	ALARM_PLATINUM_13011,//	控制器 温度传感器10断开了 报警
	ALARM_PLATINUM_13012,//	控制器 传感器1定义错误 报警
	ALARM_PLATINUM_13013,//	控制器 传感器2定义错误 报警
	ALARM_PLATINUM_13014,//	控制器 传感器3定义错误 报警
	ALARM_PLATINUM_13015,//	控制器 传感器4定义错误 报警
	ALARM_PLATINUM_13016,//	控制器 传感器5定义错误 报警
	ALARM_PLATINUM_13017,//	控制器 传感器6定义错误 报警
	ALARM_PLATINUM_13018,//	控制器 传感器7定义错误 报警
	ALARM_PLATINUM_13019,//	控制器 传感器8定义错误 报警
	ALARM_PLATINUM_13020,//	控制器 传感器9定义错误 报警
	ALARM_PLATINUM_13021,//	控制器 传感器10定义错误 报警
	ALARM_PLATINUM_13022,//	控制器 平均传感器故障 报警
	ALARM_PLATINUM_13023,//	控制器 温度传感器没有定义 报警
	ALARM_PLATINUM_13024,//	控制器 模拟输入卡故障 报警
	ALARM_PLATINUM_13025,//	控制器 模拟输出卡故障 报警
	ALARM_PLATINUM_13026,//	控制器 数字输入卡故障 报警
	ALARM_PLATINUM_13027,//	控制器 高温 报警
	ALARM_PLATINUM_13028,//	控制器 传感器1高温 报警
	ALARM_PLATINUM_13029,//	控制器 传感器2高温 报警
	ALARM_PLATINUM_13030,//	控制器 传感器3高温 报警
	ALARM_PLATINUM_13031,//	控制器 传感器4高温 报警
	ALARM_PLATINUM_13032,//	控制器 传感器5高温 报警
	ALARM_PLATINUM_13033,//	控制器 传感器6高温 报警
	ALARM_PLATINUM_13034,//	控制器 传感器7高温 报警
	ALARM_PLATINUM_13035,//	控制器 传感器8高温 报警
	ALARM_PLATINUM_13036,//	控制器 传感器9高温 报警
	ALARM_PLATINUM_13037,//	控制器 低温 报警
	ALARM_PLATINUM_13038,//	控制器 传感器1低温  报警
	ALARM_PLATINUM_13039,//	控制器 传感器2低温 报警
	ALARM_PLATINUM_13040,//	控制器 传感器3低温 报警
	ALARM_PLATINUM_13041,//	控制器 传感器4低温 报警
	ALARM_PLATINUM_13042,//	控制器 传感器5低温 报警
	ALARM_PLATINUM_13043,//	控制器 传感器6低温 报警
	ALARM_PLATINUM_13044,//	控制器 传感器7低温 报警
	ALARM_PLATINUM_13045,//	控制器 传感器8低温 报警
	ALARM_PLATINUM_13046,//	控制器 传感器9低温 报警
	ALARM_PLATINUM_13047,//	控制器 舍外温度错误 报警
	ALARM_PLATINUM_13048,//	控制器 低于最小气量 报警
	ALARM_PLATINUM_13049,//	控制器 低压 报警
	ALARM_PLATINUM_13050,//	控制器 CO2通信失败 报警
	ALARM_PLATINUM_13051,//	控制器 湿度传感器1故障 报警
	ALARM_PLATINUM_13052,//	控制器 备用报警1 报警
	ALARM_PLATINUM_13053,//	控制器 低水压 报警
	ALARM_PLATINUM_13054,//	控制器 线路断流器故障 报警
	ALARM_PLATINUM_13055,//	控制器 报警测试 报警
	ALARM_PLATINUM_13056,//	控制器 高压 报警
	ALARM_PLATINUM_13057,//	控制器 紧急状态压力 报警
	ALARM_PLATINUM_13058,//	控制器 湿度传感器2故障 报警
	ALARM_PLATINUM_13059,//	AC2000控制器通通信故障
	ALARM_PLATINUM_11518,//	IN1 风机动力电箱停电报警
	ALARM_PLATINUM_11519,//	IN2 环控电箱停电报警
	ALARM_PLATINUM_11520,//	IN3 断开报警
	ALARM_PLATINUM_11521,//	IN4 断开报警
	ALARM_PLATINUM_13060,//	温湿度传感器通信失败
	ALARM_PLATINUM_13062,//	风速传感器通信失败
	ALARM_PLATINUM_13061,//	水表传感器通信失败
	ALARM_PLATINUM_13063,//	CO2传感器通信失败
	ALARM_PLATINUM_13065,//	光照传感器通信失败
	ALARM_PLATINUM_13064,//	粉尘传感器通信失败
	ALARM_PLATINUM_13066,//	电表传感器通信失败
	ALARM_PLATINUM_11503,//	氨气传感器通信失败
	ALARM_PLATINUM_13069,//	称重传感器通信失败
	ALARM_PLATINUM_13068,//	缺水传感器通信失败
	ALARM_PLATINUM_13201,//	水线 1路 缺水报警
	ALARM_PLATINUM_13202,//	水线 2路 缺水报警
	ALARM_PLATINUM_13203,//	水线 3路 缺水报警
	ALARM_PLATINUM_13204,//	水线 4路 缺水报警
	ALARM_PLATINUM_13205,//	水线 5路 缺水报警
	ALARM_PLATINUM_13206,//	水线 6路 缺水报警
	ALARM_PLATINUM_13207,//	水线 7路 缺水报警
	ALARM_PLATINUM_13208,//	水线 8路 缺水报警
	ALARM_PLATINUM_13209,//	水线 9路 缺水报警
	ALARM_PLATINUM_13210,//	水线 10路 缺水报警
	ALARM_PLATINUM_13211,//	水线 11路 缺水报警
	ALARM_PLATINUM_13212,//	水线 12路 缺水报警
	ALARM_PLATINUM_13213,//	水线 13路 缺水报警
	ALARM_PLATINUM_13214,//	水线 14路 缺水报警
	ALARM_PLATINUM_13215,//	水线 15路 缺水报警
	ALARM_PLATINUM_13216,//	水线 16路 缺水报警
	ALARM_PLATINUM_13217,//	水线 17路 缺水报警
	ALARM_PLATINUM_13218,//	水线 18路 缺水报警
	ALARM_PLATINUM_13219,//	水线 19路 缺水报警
	ALARM_PLATINUM_13220,//	水线 20路 缺水报警
	ALARM_PLATINUM_13221,//	水线 21路 缺水报警
	ALARM_PLATINUM_13222,//	水线 22路 缺水报警
	ALARM_PLATINUM_13223,//	水线 23路 缺水报警
	ALARM_PLATINUM_13224,//	水线 24路 缺水报警
	ALARM_PLATINUM_13225,//	水线 25路 缺水报警
	ALARM_PLATINUM_13226,//	水线 26路 缺水报警
	ALARM_PLATINUM_13227,//	水线 27路 缺水报警
	ALARM_PLATINUM_13228,//	水线 28路 缺水报警
	ALARM_PLATINUM_13229,//	水线 29路 缺水报警
	ALARM_PLATINUM_13230,//	水线 30路 缺水报警
	ALARM_PLATINUM_13231,//	水线 31路 缺水报警
	ALARM_PLATINUM_13232,//	水线 32路 缺水报警
	ALARM_PLATINUM_13233,//	水线 33路 缺水报警
	ALARM_PLATINUM_13234,//	水线 34路 缺水报警
	ALARM_PLATINUM_13235,//	水线 35路 缺水报警
	ALARM_PLATINUM_13236,//	水线 36路 缺水报警
	ALARM_PLATINUM_13237,//	水线 37路 缺水报警
	ALARM_PLATINUM_13238,//	水线 38路 缺水报警
	ALARM_PLATINUM_13239,//	水线 39路 缺水报警
	ALARM_PLATINUM_13240,//	水线 40路 缺水报警
	ALARM_PLATINUM_13241,//	水线 41路 缺水报警
	ALARM_PLATINUM_13242,//	水线 42路 缺水报警
	ALARM_PLATINUM_13243,//	水线 43路 缺水报警
	ALARM_PLATINUM_13244,//	水线 44路 缺水报警
	ALARM_PLATINUM_13245,//	水线 45路 缺水报警
	ALARM_PLATINUM_13246,//	水线 46路 缺水报警
	ALARM_PLATINUM_13247,//	水线 47路 缺水报警
	ALARM_PLATINUM_13248 //	水线 48路 缺水报警
};

#define BASE_DATA_ID_NUM_PLATINUM   (ADDR_PLATINUM_2560+1)
#define BASE_DATA_ID_PLATINUM       2401

#define BASE_ALARM_ID_NUM_PLATINUM  (ALARM_PLATINUM_13248+1)
#define BASE_ALARM_ID_PLATINUM      112000
#define ALARM_NUM_PLATINUM          (ALARM_PLATINUM_13248-ALARM_PLATINUM_13000+1)   //总报警数量
#define AC2000_ALARM_NUM_PLATINUM   (ALARM_PLATINUM_13059-ALARM_PLATINUM_13000+1)   //AC2000报警数量



extern const AC2000_DATA_TYPE id_value_dot_range_platinum[][2];
extern uint16_t AlarmCodePlatinum[ALARM_NUM_PLATINUM][3];

#endif
