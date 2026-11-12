#ifndef __AC2000_PLUS_H
#define __AC2000_PLUS_H

#include "stm32h7xx.h"

typedef uint16_t u16;


extern int PlusRecvFlag;
#define AC2000_plus_CmdNum 9
#define AC2000PlusCmdLength 18 


#define PLOW_PARAMS_LEN            128
#define PNORMAL_PARAMS_LEN         256
#define PHIGH_PARAMS_LEN           512




#define	PLUSTEMPERATURE_CURVE_CMD        										1063 
#define	PLUSMIN_MAX_LEVEL_CMD   										        1064
#define	PLUSHUMIDITY_TREATM_CMD    									    	  1065
#define PLUSREFRIGERATION_CMD                               1066
#define	PLUSILLUMINATION_CMD          										  1067
#define	PLUSFEED_CMD             										        1068
#define	PLUSADDITIONAL_CMD      										        1069
#define PLUSSTATICPRESSURE_CMD                              1070
#define PLUSHUMIDIFIER_CMD                                  1071 


#define MANAGEDEATH_CMD                                     1072
#define MANAGEPOUITRY_CMD                                   1073
#define MANAGEFEEDSTOCK_CMD                                 1074
#define MANAGETIMEDAY_CMD                                   1075
#define MANAGESTOCK_CMD                                     1076
#define MANAGENEWSTOCK_CMD                                  1077
#define MANAGEALARMTES_CMD                                  1078
#define MANAGEALARMTEST_CMD                                 1079

#define TESTTEMPERAURE_CMD                                  1080
#define TESTHUMIDITY_CO2_CMD                                1081
#define TESTRELAY_CMD                                       1082
#define TESTDIGITAL_CMD                                     1083
#define TESTSIMULATION_CMD                                  1084
#define TESTPULSE_CMD                                       1085
#define TESTCHICKEN_CMD                                     1086
#define TESTVERSION_CMD                                     1087


#define CALIBRATIONTEMPERAURE_CMD                           1088 
#define CALIBRATTONHUMIDITY                                 1089
#define CALIBRATIONCHICKEN_CMD                              1090
#define CALIBRATIONFEED_CMD                                 1091
#define CALIBRATIONWATER_CMD                                1092
#define CALIBRATIONCO2_CMD                                  1093

#define INSTALLCONFIGURATION_CMD                            1094
#define INSTALLVENTI_CMD                                    1095
#define INSTALLROLLERBLIND_CMD                              1096
#define INSTALLRELAY_CMD                                    1097
#define INSTALLSENSOR_CMD                                   1098
#define INSTALLSYSTEM_CMD                                   1099
#define INSTALLWINDCOLLING_CMD                              1100



extern const unsigned char AC2000PlusControlTemperatureCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusMinmax_levelCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000Plushumidity_Co2[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusRefrigeration[1][AC2000PlusCmdLength];
extern const unsigned char AC2000Plusillumination[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusfeedCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusAdditional_systemCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PluspressureCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlushumidifierCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagedeath[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagePlusPoultry_metering[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManageFeed_stock[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManageTime_day[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagestock[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagenewstock[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagealarmRest[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusManagealarmTest[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestTemperatureCmd[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestHumidity[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestRelay[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestDigital[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestPulse[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestChicken[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusTestVersion[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusCaliHumetidy[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusCalichick[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusCaliFeed[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusCaliWater[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusCaliCo2[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallConguration[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallSensor[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallSystem[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallWindColling[1][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallVenti[2][AC2000PlusCmdLength];
extern const unsigned char AC2000PlusInstallRollerBlind[1][AC2000PlusCmdLength];

void PlusOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000PlusCmdLength],uint8_t cmd_num,uint8_t *cmd_index);
void CreateAC2000PlusNormal(const uint8_t CmdSource[][AC2000PlusCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);





void CreateAC2000PlusSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
void Parse_MainPlus(uint8_t *dataSource,uint16_t len);//主屏幕解析函数
void Ac2000DataPlusMalloc(void);
void Parse_WarnPlus(uint8_t *dataSource,uint16_t len);
void temperature_plus(uint8_t *dataSource,uint16_t len);
void Minmax_level_plus(uint8_t *dataSource,uint16_t len);
void humidity_Co2(uint8_t *dataSource,uint16_t len);
void Refrigeration_Plus(uint8_t *dataSource,uint16_t len);
void ConFeed(uint8_t *dataSource,uint16_t len);
void Additional_system(uint8_t *dataSource,uint16_t len);
void pressure(uint8_t *dataSource,uint16_t len);
void Manage_death(uint8_t *dataSource,uint16_t len);
void Manage_PlusPoultry_metering(uint8_t *dataSource,uint16_t len);
void Manage_Plusfeed_stock(uint8_t *dataSource,uint16_t len);
void Manage_Plusday_time(uint8_t *dataSource,uint16_t len);
void Manage_PlusFlock(uint8_t *dataSource,uint16_t len);
void Manage_PlusNewFlock(uint8_t *dataSource,uint16_t len);
void Manage_PlusAlarm_reset(uint8_t *dataSource,uint16_t len);
void Manage_PlusTest(uint8_t *dataSource,uint16_t len);
void Testtemperature(uint8_t *dataSource,uint16_t len);
void Testhumidity(uint8_t *dataSource,uint16_t len);
void TestRelay(uint8_t *dataSource,uint16_t len);
void Testpulse(uint8_t *dataSource,uint16_t len);
void TestChicken_scale(uint8_t *dataSource,uint16_t len);
void TestVersion(uint8_t *dataSource,uint16_t len);
void Calibrationchicken(uint8_t *dataSource,uint16_t len);
void CalibrationFeed(uint8_t *dataSource,uint16_t len);
void CalibrationWater(uint8_t *dataSource,uint16_t len);
void CalibrationCo2(uint8_t *dataSource,uint16_t len);
void InstallConfiguration(uint8_t *dataSource,uint16_t len);
void InstallSensor(uint8_t *dataSource,uint16_t len);
void InstallWindcooling(uint8_t *dataSource,uint16_t len);
void InstallVentilationlevel(uint8_t *dataSource,uint16_t len);
void InstallSystem(uint8_t *dataSource,uint16_t len);
void InstallRoller_blind(uint8_t *dataSource,uint16_t len);

int Ac2000PlusDataParse(uint8_t *data,u16 Counter);


typedef struct PlusAC2000CONTROL  //控制大区数据
{
  char PlusAc2000ControlTemperatureCurve[PNORMAL_PARAMS_LEN];//温度
	char Plushumidity_treatm[PNORMAL_PARAMS_LEN]; //湿度-Co2处理
	char Plusmin_max_level[PNORMAL_PARAMS_LEN];   //最高最低级别
	char PlusRefrigeration[PNORMAL_PARAMS_LEN];   //制冷
	char Plusillumination[PHIGH_PARAMS_LEN];   //照明
	char PlusFeed[PNORMAL_PARAMS_LEN];           //饲料
	char PlusAdditional_system[PNORMAL_PARAMS_LEN]; //附加系统
	char Plusstatic_pressure[PNORMAL_PARAMS_LEN]; //压力
	char Plushumidifier[PNORMAL_PARAMS_LEN];    //加湿器
  //
}PlusAc2000Control;

typedef struct PlusAC2000MANAGEMENT  //控制大区数据
{
	char PlusChicken_Death[PNORMAL_PARAMS_LEN];//鸡死亡
	char PlusPoultry_metering[PNORMAL_PARAMS_LEN];//禽计量
	char PlusFeed_stock[PNORMAL_PARAMS_LEN]; //饲料存量
	char PlusTime_day[PNORMAL_PARAMS_LEN]; //时间和日期
	char PlusFlock[PNORMAL_PARAMS_LEN];//禽群数
	char PlusNew_Flock[PNORMAL_PARAMS_LEN];//新鸡群
	char PlusAlarm_reset[PNORMAL_PARAMS_LEN];//报警重置
	char PlusAlarm_test[PNORMAL_PARAMS_LEN];//报警测试
}PlusAc2000Manage;

typedef struct PlusAC2000TEST//测试大区
{
	char PlusTesttemperature[PNORMAL_PARAMS_LEN];//温度
	char PlusTesthumidityCo2[PNORMAL_PARAMS_LEN];//湿度
	char PlusTestRelay[PNORMAL_PARAMS_LEN];//继电器
	char PlusTestdigital[PNORMAL_PARAMS_LEN];//数字输入
	char PlusTestsimulation[PNORMAL_PARAMS_LEN];//模拟输入
	char PlusTestpulse[PNORMAL_PARAMS_LEN];//脉冲
	char PlusTestChicken_scale[PNORMAL_PARAMS_LEN];//鸡秤
	char PlusTestversion[PNORMAL_PARAMS_LEN];//版本
}PlusAc2000Test;

typedef struct PlusAC2000calibration//校准大区
{
	char PlusCalibration_temperature[PNORMAL_PARAMS_LEN];//温度
	char PlusCalibration_humidityCo2[PNORMAL_PARAMS_LEN];//湿度
	char PlusCalibration_Chicken_scale[PNORMAL_PARAMS_LEN];//鸡秤因素
	char PlusCalibration_Feed_pulse_ratio[PNORMAL_PARAMS_LEN];//饲料脉冲比
	char PlusCalibration_Water_pulse_ratio[PNORMAL_PARAMS_LEN];//水量脉冲比
	char PlusCalibration_Co2[PNORMAL_PARAMS_LEN];
}PlusAc2000calibration;

typedef struct PlusAC2000Install//安装大区
{
	char PlusInstallConfiguration[PNORMAL_PARAMS_LEN]; //配置
	char PlusInstallVentilation_level[PHIGH_PARAMS_LEN]; //通风级别
	char PlusInstallRelay_layout[PNORMAL_PARAMS_LEN]; //继电器布局
	char PlusInstallSensor_layout[PNORMAL_PARAMS_LEN];//传感器布局
	char PlusInstallSystem_parameters[PNORMAL_PARAMS_LEN]; //系统参数
	char PlusInstallWind_cooling[PNORMAL_PARAMS_LEN]; //风冷
	char PlusInstallRoller_blind[PNORMAL_PARAMS_LEN];
}PlusAc2000Install;

typedef struct PlusAC2000PARAMS
{
	PlusAc2000Control 			PAc2000Control; //控制大区
	PlusAc2000Manage  			PAc2000Manage;  //管理大区
	PlusAc2000Test  				PAc2000Test;    //测试大区
	PlusAc2000calibration  	PlusAc2000calibration; //校准大区
	PlusAc2000Install  		  PAc2000Install; //安装大区
}PlusAc2000Params;




extern PlusAc2000Params *Ac2000PlusParams;


extern unsigned char TypeTemperature;
extern unsigned char moniandshidu;
extern unsigned char Relay;







#endif

