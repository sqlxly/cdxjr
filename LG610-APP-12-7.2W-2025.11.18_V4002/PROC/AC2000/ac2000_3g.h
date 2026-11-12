#ifndef __AC2000_3G_H
#define __AC2000_3G_H

#include "stm32h7xx.h"

#define AC2000_3g_CmdNum 10
#define AC2000CmdLength 18 
#define AC2000HouseNum 64


#define	TEMPERATURE_CURVE_CMD        										1006 
#define	HUMIDITY_TREATM_CMD  										        1007
#define	CO2_HANDLE_CMD               										1008 
#define	MIN_MAX_LEVEL_CMD        									    	1009
#define	STATIC_PRESSURE_CMD          										1010
#define	CONTROL_MODE_CMD             										1011
#define	SYSTEM_INFORAMATION_CMD      										1012
        
#define	DEVICE_LEVELS_OF_VENTILATION_CMD                1013  
#define	DEVICE_VARIABLE_SPEED_FAN_LEVELS_CMD            1014  
#define	DEVICE_VENT_AND_CURTIAN_LEVELS_CMD              1015  
#define	DEVICE_STIR_FAN_LEVELS_CMD             					1016  
#define	DEVICE_STIR_FAN_PROGRAM_CMD             				1017  
#define	DEVICE_FOGGERS_CMD             									1019  
#define	DEVICE_COOL_PAD_CMD             								1018 
#define	DEVICE_LIGHT_CMD             										1020  
#define	DEVICE_WATER_AND_FEED_CMD             				  1021  
#define	DEVICE_EXTRA_SYSTEM_CMD             						1022
#define	DEVICE_WATER_ON_DEMAND_CMD             					1023  

#define	MANAGE_STOCKOFCHICKEN_CMD             					1024  
#define	MANAGE_STOCKOFFEED_CMD             					    1025 
#define	MANAGE_TIMEANDDATA_CMD             					    1026
#define	MANAGE_GROWTHANDHEN_CMD             					  1027
#define	MANAGE_ALARMSET_CMD             					  		1028
#define	MANAGE_ALARMRESET_CMD             					  	1029
#define	MANAGE_PASSWORD_CMD             					      1030

#define	HISTORT_TEMPERATURE_CMD             						1031  
#define	HISTORY_HUMIDITY_TREATM_CMD             			  1032 
#define	HISTORY_CO2_CMD             					   				1033
#define	HISTORY_WATER_CMD             					 			  1034
#define	HISTORY_FEED_CMD             					  				1035
#define	HISTORY_DEAD_CMD             					  				1036
#define	HISTORY_HEATER_CMD             					 		    1037
#define	HISTORY_RADIANT_HEATER_CMD             					1038
#define	HISTORY_ALARM_CMD             					   		  1039
#define	HISTORY_EVENT_MAP_CMD             					    1040
#define	HISTORY_WATCH_CMD             					    	  1041
#define	HISTORY_SET_CMD             					      		1042

#define	TEST_SWITCHRELAY_CMD            					      1043
#define	TEST_ANALOGSENSOR_CMD             					    1044
#define	TEST_DIGITALSENSOR_CMD             					    1045
#define	TEST_STATICPRESSURE_CMD             					 	1046

#define	SERVICE_TEMPERATURE_ADJUST_CMD            			1047
#define	SERVICE_HUMIDITY_ADJUST_CMD             				1048
#define	SERVICE_FEED_ADJUST_CMD             					  1049
#define	SERVICE_WATER_ADJUST_CMD             					 	1050	

#define	INSTALL_NORMALSET_CMD            								1051  //未解析
#define	INSTALL_RELAYSETUP_CMD            							1052  //未解析
#define	INSTALL_ANALOGSENSOR_CMD            						1053  
#define	INSTALL_DIGITALSENSOR_CMD            						1054  
#define	INSTALL_ANALOGOUTPUT_CMD            						1055  
#define	INSTALL_VENTCURTAINSETUP_CMD            				1056  //未解析
#define	INSTALL_TEMPERATUREDEFINE_CMD            				1057
#define	INSTALL_CAPACITYOFFANAIR_CMD            				1058
#define	INSTALL_HENSIZE_CMD            									1059
#define	INSTALL_COMMUNICATION_CMD            						1060


#define CHICK_BASICSETTING                                     1062 //基本设置
#define CHICK_LAYOUT                                           1063 //布局
#define CHICK_SETTING                                          1064 //鸡舍设置
#define CHICK_CURVE                                            1065 //曲线
#define CHICK_HISTORY                                          1066 //历史
#define CHICK_CONVERSION                                       1067 //转换
#define CHICK_TEST                                             1068 //测试
#define CHICK_CALIBRATION                                      1069 //校准

#define	CMD_START              1006  
#define	CMD_STOP               1110

#define GET_PROPERTY_PARAMS       30

#define LOW_PARAMS_LEN            128
#define NORMAL_PARAMS_LEN         256
#define HIGH_PARAMS_LEN           512

#define SEND_PC_ENABLE         1


#define Temperature_curve                                 10000
#define Temperature_curve_correction_value                10001
#define Temperature_change_reminder                       10002
#define Target_temperature_zone                           10003
#define Heater_temperature_zone                           10004               
#define Cooling_coefficient                               10005  
#define Lower_than_heating_temperature                    10006
#define Heating_temperature_difference                    10007

#define Cycle_Herter                                      20000
#define Low_Heat_Differential                             20001
#define High_Heat_Differential                            20002
#define Minimum_On_Time                                   20003

#define Radiant_heater                                    30000
#define Low_radiant_heater_difference                     













enum ReciverTimer //判断接收的是属于哪部分的数据
{
	TEMPERATURE=1,
	HUMIDITY,
	CO2,
	WATER,
	FEED,
	DEAD,
	HEATER,
	RADIANT_HEATER,
	ALARM,
	EVENT_MAP,
	WATCH,
	SETT,
};

typedef struct AC2000CONTROLSYSTEMPARAMS
{
	char system_inforamation_tmperature_curvel[LOW_PARAMS_LEN];     			//温度曲线
	char system_inforamation_cycle_heater[LOW_PARAMS_LEN];  							//CYCLE HEATER
	char system_inforamation_radiant_heater[LOW_PARAMS_LEN];  		 				//辐射加热器
	char system_inforamation_variable_heater[LOW_PARAMS_LEN];							//VARIABLE HEATER
	char system_inforamation_humidity_treatment[LOW_PARAMS_LEN];					//湿度处
	char system_inforamation_C02_treatment[LOW_PARAMS_LEN];								//CO2处理
	char system_inforamation_static_pressure[LOW_PARAMS_LEN];							//静压
	char system_inforamation_ventilation_level[LOW_PARAMS_LEN];						//通风级别
	char system_inforamation_rolling_shutter_and_roof[LOW_PARAMS_LEN];		//卷帘和屋顶
	char system_inforamation_curtain_calibration[LOW_PARAMS_LEN];					//VENT CURTAIN CALIBRATION
	char system_inforamation_wet_curtain_cool_pad_flush[LOW_PARAMS_LEN];	//湿帘 COOL PAD FLUSH
	char system_inforamation_Humidifier[LOW_PARAMS_LEN];									//加湿器
	char system_inforamation_lighting[LOW_PARAMS_LEN];										//灯光
	char system_inforamation_food_water[LOW_PARAMS_LEN];									//水和饲料
	char system_inforamation_food_left[LOW_PARAMS_LEN];										//饲料存量
	char system_inforamation_food_leftlog[LOW_PARAMS_LEN];	
	char system_inforamation_alarm_set[LOW_PARAMS_LEN];										//报警设置	
}G3Ac2000ContyrolSystemParams;

typedef struct G3AC2000CONTROLTEMPERATURECURVE
{
	char temperature_curve1[NORMAL_PARAMS_LEN];
	char temperature_curve2[NORMAL_PARAMS_LEN];
	char temperature_curve3[NORMAL_PARAMS_LEN];
}G3Ac2000ControlTemperatureCurve;

typedef struct G3AC2000CONTROL
{
	G3Ac2000ControlTemperatureCurve Ac2000ControlTemperatureCurve;
	char humidity_treatm[NORMAL_PARAMS_LEN];
	char min_max_level[NORMAL_PARAMS_LEN];
	char static_pressure[NORMAL_PARAMS_LEN];
	char control_mode[NORMAL_PARAMS_LEN];
	char CO2_Handle[NORMAL_PARAMS_LEN];
	G3Ac2000ContyrolSystemParams Ac2000ContyrolSystemParams;
}G3Ac2000Control;

typedef struct G3AC2000DEVICE
{
	uint8_t LevelsOfVentilationCmdCount;    //设备 通风级别 当前指令
	uint8_t variable_speed_fan_levelsFanNum;//设备 调速风扇级别   通道风扇数量
	uint8_t vent_and_curtain_LevelsFanNum;  //设备 通风和风帘级别 通道启用数量
	uint8_t stir_fan_levelsFanNum;          //设备 搅扇级别       通道启用数量
	char variable_speed_fan_levels[NORMAL_PARAMS_LEN];
	char vent_and_curtain_Levels[NORMAL_PARAMS_LEN];
	char stir_fan_levels[NORMAL_PARAMS_LEN];
	char stir_fan_program[NORMAL_PARAMS_LEN];
	char foggers[NORMAL_PARAMS_LEN];
	char cool_pad[NORMAL_PARAMS_LEN];
	char light[NORMAL_PARAMS_LEN];
	char water_and_feed[NORMAL_PARAMS_LEN];
	char extra_system[NORMAL_PARAMS_LEN];
	char water_on_demand[NORMAL_PARAMS_LEN];
	char levels_of_ventilation[HIGH_PARAMS_LEN];
	
}G3Ac2000Device;

typedef struct G3AC2000MANAGE
{
	char StockOfChicken[LOW_PARAMS_LEN];//鸡的存量
	char StockOfFeed[HIGH_PARAMS_LEN];   //饲料的存量
	char TimeAndDate1[LOW_PARAMS_LEN];   //时间和日期
	char TimeAndDate2[LOW_PARAMS_LEN];
	char GrowthAndHen[LOW_PARAMS_LEN];  //生长期和鸡群
	char AlarmSet[LOW_PARAMS_LEN];      //警报设置
	char AlarmReset[LOW_PARAMS_LEN];    //警报重置
	char Password[LOW_PARAMS_LEN];      //密码
}G3Ac2000Manage;

typedef struct G3AC2000CHICKENPOUND
{
	char test[NORMAL_PARAMS_LEN]; //测试
	char Setting[LOW_PARAMS_LEN]; //基本设置
	char layout[LOW_PARAMS_LEN];  //布局
	char Curse[HIGH_PARAMS_LEN];  //曲线
	char Conversion[LOW_PARAMS_LEN];
	char Test[LOW_PARAMS_LEN];
	char calibration[LOW_PARAMS_LEN];
}G3Ac2000ChickenPound;

typedef struct G3AC2000HISTORY
{
	char Temperature[NORMAL_PARAMS_LEN];
	char Humidity_treatm[NORMAL_PARAMS_LEN];
	char CO2[NORMAL_PARAMS_LEN];
	char Water[NORMAL_PARAMS_LEN];
	char Feed[NORMAL_PARAMS_LEN];
	char Dead[NORMAL_PARAMS_LEN];
	char Heater[NORMAL_PARAMS_LEN];
	char Radiant_Heater[NORMAL_PARAMS_LEN];
	char Alarm[NORMAL_PARAMS_LEN];
	char Event_Map[NORMAL_PARAMS_LEN];
	char Watch[NORMAL_PARAMS_LEN];
	char Set[NORMAL_PARAMS_LEN];
}G3Ac2000History;

typedef struct G3AC2000TEST
{
	uint8_t AnalogSensorNum;
	uint8_t DigitalSensorNum;
	char SwitchRelay[LOW_PARAMS_LEN];
	char AnalogSensor[LOW_PARAMS_LEN];
	char DigitalSensor[LOW_PARAMS_LEN];
	char StaticPressure[LOW_PARAMS_LEN];
}G3Ac2000Test;

typedef struct G3AC2000SERVICE
{
	char Temperature[LOW_PARAMS_LEN];
	char Humidity[LOW_PARAMS_LEN];
	char Feed[LOW_PARAMS_LEN];
	char Water[LOW_PARAMS_LEN];
}G3Ac2000Service;

typedef struct G3AC2000SINSTALL
{
	uint8_t CapacityOfFanAirNum;
	uint8_t VentCurtainNum;
	uint8_t TemperatureDefineNum;
	uint8_t AnalogSensorAndOutput;
	uint16_t CapacityOfFanAirUint;
	char NormalSet[LOW_PARAMS_LEN];    //设置
	char RelaySetUp[NORMAL_PARAMS_LEN];   //继电器布局
	char AnalogSensor[LOW_PARAMS_LEN]; //模拟传感器
	char DigitalSensor[LOW_PARAMS_LEN]; //数字传感器
	char AnalogOutput[LOW_PARAMS_LEN];  //模拟输出
	char VentCurtainSetUp[LOW_PARAMS_LEN]; //
	char TemperatureDefine[NORMAL_PARAMS_LEN];//温度定义
	char CapacityOfFanAir[LOW_PARAMS_LEN]; //风扇空气容量
	char HenSize[LOW_PARAMS_LEN]; //鸡舍大小
	char Communication[LOW_PARAMS_LEN]; //通信
}G3Ac2000Install;

typedef struct AC2000PARAMS
{
	G3Ac2000Control 			Ac2000Control;
	G3Ac2000Device 				Ac2000Device;
	G3Ac2000Manage  			Ac2000Manage;
	G3Ac2000ChickenPound  Ac2000ChickenPound;
	G3Ac2000History  			Ac2000History;
	G3Ac2000Test  				Ac2000Test;
	G3Ac2000Service  			Ac2000Service;
	G3Ac2000Install  		  Ac2000Install;
}G3Ac2000Params;

extern G3Ac2000Params *Ac20003GParams;

extern int AC2000RecvFlag;
extern unsigned char Set_difference;
extern unsigned char Types;

extern const unsigned char AC2000ControlTemperatureCmd[3][AC2000CmdLength];
extern const unsigned char AC2000ControlHumidityCmd[1][AC2000CmdLength];
extern const unsigned char AC2000ControlMaxMinCmd[1][AC2000CmdLength];
extern const unsigned char AC2000ControlStaticPressureCmd[1][AC2000CmdLength];
extern const unsigned char AC2000ControlModeCmd[1][AC2000CmdLength];
extern const unsigned char AC2000ControlCo2Cmd[1][AC2000CmdLength];

extern const unsigned char AC2000SysInfoServiceCmd[17][AC2000CmdLength];
extern const unsigned char AC2000_Device_LevelsOfVentilation[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_VariableSpeedFanLevels[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_VentAndCurtianLevels[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_StirFanLevels[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_StirFanProgram[1][AC2000CmdLength];
extern const unsigned char AC2000_Device_Foggers[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_CoolPad[2][AC2000CmdLength];
extern const unsigned char AC2000_Device_Light[4][AC2000CmdLength];
extern const unsigned char AC2000_Device_WaterAndFeed[5][AC2000CmdLength];
extern const unsigned char AC2000_Device_ExtraSystem[1][AC2000CmdLength];
extern const unsigned char AC2000_Device_WaterOnDemand[2][AC2000CmdLength];


extern const unsigned char AC2000_ManageStockOfChicken[1][AC2000CmdLength];/*管理-鸡的存量*/
extern const unsigned char AC2000_ManageStockOfFeed[5][AC2000CmdLength];/*管理-饲料的存量*/
extern const unsigned char AC2000_ManageTimeAndDate[2][AC2000CmdLength];/*管理-时间和日期*/
extern const unsigned char AC2000_ManageGrowthRegionAndHen[1][AC2000CmdLength];/*管理-生长期和鸡群*/
extern const unsigned char AC2000_ManageAlarmSet[4][AC2000CmdLength];/*管理-警报设置*/
extern const unsigned char AC2000_ManageAlarmReset[1][AC2000CmdLength];/*管理-警报重置*/
extern const unsigned char AC2000_ManagePassword[1][AC2000CmdLength];/*管理-密码*/

extern const unsigned char AC2000_History_Temperature[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Humidity_treatm[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_CO2[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Water[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Feed[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Dead[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Heater[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Radiant_Heater[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Alarm[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Event_Map[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Watch[1][AC2000CmdLength];/*历史*/
extern const unsigned char AC2000_History_Set[1][AC2000CmdLength];/*历史*/

extern const unsigned char AC2000_Test_SwitchRelay[2][AC2000CmdLength];
extern const unsigned char AC2000_Test_AnalogSensor[2][AC2000CmdLength];
extern const unsigned char AC2000_Test_DigitalSensor[2][AC2000CmdLength];
extern const unsigned char AC2000_Test_StaticPressure[1][AC2000CmdLength];

extern const unsigned char AC2000_Service_Temperature[1][AC2000CmdLength];
extern const unsigned char AC2000_Service_Humidity[1][AC2000CmdLength];
extern const unsigned char AC2000_Service_Feed[1][AC2000CmdLength];
extern const unsigned char AC2000_Service_Water[1][AC2000CmdLength];

extern const unsigned char AC2000_Install_NormalSet[1][AC2000CmdLength];
extern const unsigned char AC2000_Install_RelaySetUp[1][AC2000CmdLength];
extern const unsigned char AC2000_Install_AnalogSensor[2][AC2000CmdLength];
extern const unsigned char AC2000_Install_DigitalSensor[1][AC2000CmdLength];
//extern const unsigned char AC2000_Install_AnalogOutput[1][AC2000CmdLength];
extern const unsigned char AC2000_Install_VentCurtainSetUp[2][AC2000CmdLength];
extern const unsigned char AC2000_Install_TemperatureDefine[2][AC2000CmdLength];
extern const unsigned char AC2000_Install_CapacityOfFanAir[2][AC2000CmdLength];
extern const unsigned char AC2000_Install_HenSize[1][AC2000CmdLength];
extern const unsigned char AC2000_Install_Communication[1][AC2000CmdLength];
//鸡秤
extern const unsigned char AC2000_ChickSetting[1][AC2000CmdLength];
extern const unsigned char AC2000_Chicklayout[4][AC2000CmdLength];//鸡秤布局
extern const unsigned char AC2000_ChickCurce[3][AC2000CmdLength];
extern const unsigned char AC2000_ChickConversion[1][AC2000CmdLength];
extern const unsigned char AC2000_ChickTest[1][AC2000CmdLength];
extern const unsigned char AC2000_ChickCalibration[1][AC2000CmdLength];


extern void CreateAC20003GSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void CreateAC20003GControlSysInfoSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void CreateAC20003GDeviceLevelsOfVentilation(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void CreateAC20003GVariableSpeedFanLevels(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void CreateAC20003GNormal(const uint8_t CmdSource[][AC2000CmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void CreateAC20003GNormalOneParam(const uint8_t CmdSource[][AC2000CmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,uint8_t param);
extern void OverLoadSendSpecialCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index);
extern void connect_ac2000_3g(uint8_t *buff,uint8_t house);


extern void Ac20003gDataParse(uint8_t *data,unsigned short Counter);
extern int Ac20003gDataParseCache(uint8_t *data,unsigned short Counter);
extern int Ac20003gDataParseMultiCache(uint8_t *data,unsigned short len);
extern void Ac2000Data3GMalloc(void);
void OverLoadSendOneParamSpecialCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t param);
extern void OverLoadSendOneParamCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t param);
extern void OverLoadSendNormalCmd(const uint8_t Cmd[][AC2000CmdLength],uint8_t cmd_num,uint8_t *cmd_index,uint8_t i);
	
extern void system_inforamation_tmperature(unsigned char *dataSource,uint16_t len);     							//温度曲线
extern void system_inforamation_cycle_heater(unsigned char *dataSource,uint16_t len);  							//CYCLE HEATER
extern void system_inforamation_radiant_heater(unsigned char *dataSource,uint16_t len);  		 				//辐射加热器
extern void system_inforamation_variable_heater(unsigned char *dataSource,uint16_t len);							//VARIABLE HEATER
extern void system_inforamation_humidity_treatment(unsigned char *dataSource,uint16_t len);					//湿度处
extern void system_inforamation_C02_treatment(unsigned char *dataSource,uint16_t len);								//CO2处理
extern void system_inforamation_static_pressure(unsigned char *dataSource,uint16_t len);							//静压
extern void system_inforamation_ventilation_level(unsigned char *dataSource,uint16_t len);						//通风级别
extern void system_inforamation_rolling_shutter_and_roof(unsigned char *dataSource,uint16_t len);		//卷帘和屋顶
extern void system_inforamation_curtain_calibration(unsigned char *dataSource,uint16_t len);					//VENT CURTAIN CALIBRATION
extern void system_inforamation_wet_curtain_cool_pad_flush(unsigned char *dataSource,uint16_t len);	//湿帘 COOL PAD FLUSH
extern void system_inforamation_Humidifier(unsigned char *dataSource,uint16_t len);									//加湿器
extern void system_inforamation_lighting(unsigned char *dataSource,uint16_t len);										//灯光
extern void system_inforamation_food_water(unsigned char *dataSource,uint16_t len);									//水和饲料
extern void system_inforamation_food_left(unsigned char *dataSource,uint16_t len);										//饲料存量
extern void system_inforamation_alarm_set(unsigned char *dataSource,uint16_t len);										//报警设置
extern void System_inforamation_waterandfeed(unsigned char *dataSource,uint16_t len);                //水和饲料
extern void system_inforamation_food_leftli(unsigned char *dataSource,uint16_t len);

void relaySend_3g(const uint8_t Cmd[][AC2000CmdLength],unsigned short porv,unsigned char difference);
void Write_data_Empty(void);
#endif


