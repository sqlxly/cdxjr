#ifndef __AC2000_PLATINUM_PRO_H
#define __AC2000_PLATINUM_PRO_H

#include "stm32h7xx.h"
#include "data_table.h"
#define AC2000_Pro_CmdNum 19

extern int PorRecvFlag;
extern unsigned int Number_relays;
#define AC2000ProCmdLength 18 
#define PLOW_PARAMS_LEN            128
#define PNORMAL_PARAMS_LEN         256
#define PHIGH_PARAMS_LEN           512
#define High_LEN                   2048

#define PORCONTROTEMPERATURECURVE_CMD 1110  //温度曲线
#define PORCONTROHUMIDITYTREATM_CMD   1111  //湿度处理
#define PORCONTROMINMAX_CMD           1112  //最高最低级别  
#define PORCONTROSTATICPRESSURE_CMD   1113  //静压
#define PORCONTROMODE_CMD             1114  //控制模式
#define PORCONTROSYSTEMMODE_CMD       1115  //系统模式
#define PORCONTROCO2PROCESSING_CMD    1116  //Co2处理
#define PORCONTROAMMONIATREATMENT_CMD 1117  //氨处理


#define PORMECHANISMVENTILATIONLEVEL_CMD     1118  //通风级别
#define PORMECHANISMSPEEDCONTROFAN_CMD       1119  //调速风扇级别
#define PORMECHANISMCURTAINLEVEL_CMD         1120  //风帘级别
#define PORMECHANISMSTIRRINGFANLEVEL_CMD     1121  //搅扇级别
#define PORMECHANISMSTIRRINGFANPROGRAM_CMD   1122  //搅扇程序
#define PORMECHANISMSPRAYER_CMD              1123  //喷雾器
#define PORMECHANISMREFRIGERATION_CMD        1124  //制冷板
#define PORMECHANISMLIGHT_FEED_CMD           1125  //灯光有饲料
#define PORMECHANISMLIGHT_CMD                1126  //灯光无饲料
#define PORMECHANISMWATERFEED_CMD            1127  //水和饲料
#define PORMECHANISMADDITIONALSYSTEM_CMD     1128  //附加系统
#define PORMECHANISMWATERONDEMAND_CMD        1129  //Water On Demand 
#define PORMECHANISMNATURALPROGRAM_CMD       1130  //Natural Program
#define PORMECHANISMFEEDSCALEPROGRAM_CMD     1131  //Feed Scale Program


#define PORMANAGECHICKENSTOCK_CMD            1132  //鸡的存量
#define PORMANAGEFEEDSTOCK_CMD           		 1133  //饲料的存量
#define PORMANAGETOMEDATE_CMD           		 1134  //时间和日期
#define PORMANAGEGROWTHPERIODFLOCK_CMD       1135  //生长期和鸡群
#define PORMANAGEALARMSETTING_CMD            1136  //警报设置
#define PORMANAGEALARMRESET_CMD            	 1137	 //警报重置
#define PORMANAGESAVESUCCESS_CMD             1138  //没有保存成功设置
#define PORMANAGEPASSWORD_CMD            		 1139  //密码
#define PORMANAGERELAYCURR_CMD               1140  //继电器电流设定


#define PORTESTSWITCH_CMD                    1141  //开关和继电器
#define PORTESTANALOGSENSOR_CMD              1142  //模拟传感器
#define PORTESTDIGITALSENSOR_CMD             1143  //数字传感器
#define PORTESTSTATICPRESSURE_CMD            1144  //静压
#define PORTESTHARDWAREINVENTORY_CMD         1145  //硬件清单

#define PORSERVICETEMPERATURE_CMD            1146   //温度校准
#define PORSERVICEHUMIDITY_CMD               1147   //湿度校准
#define PORSERVICEFEED_CMD                   1148   //饲料校准
#define PORSERVICEWATER_CMD                  1149   //水的校准
#define PORSERVICESETPLUG_CMD                1150   //设置插头界面
#define PORSERVICEWATERLINENIPPLERINSE_CMD   1151   //水线乳头冲洗
#define PORSERVICEENNDER_CMD                 1152   //喂料器和喂水器
#define PORSERVICEAMMONIACALIBRATION_CMD     1153   //Ammonia Calibration

#define PORINSTALL_SETUP_CMD                 1154   //设置
#define PORINSTALL_RELAYLAYOUT_CMD           1155   //继电器布局
#define PORINSTALL_ANALOGSENSOR_CMD          1156   //模拟传感器
#define PORINSTALL__DIGITALSENSOR_CMD        1157   //数字传感器
#define PORINSTALL_ANALOGOUTPUT_CMD          1158   //模拟输出
#define PORINSTALL_CURTAINSETTING_CMD        1159   //风帘设置
#define PORINSTALL_TEMPERATUREDE_CMD         1160   //温度定义
#define PORINSTALL_FANAIRCAPACITY_CMD        1161   //风扇空气容量
#define PORINSTALL_COOPSIZE_CMD              1162   //鸡舍大小
#define PORINSTALL_COMMUNICATION_CMD         1163   //通信
#define PORINSTALL_LIGHTDIMMER_CMD           1164   //light dimmer



extern const unsigned char Ac2000PorContorTemp[3][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorHumitidy[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorMinMax[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorStaticpressure[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorMode[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorSystem[19][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorCo2processing[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorContorAmmoniatreatment[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechVentilationlevel[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechSpeedcontrolfanlevel[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechCurtainlevel[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechStirfan[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechstirringfanprogarm[1][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechsprayer[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechRefrigerationplate[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechLightNoFeed[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechLightFeed[2][AC2000ProCmdLength];
extern const unsigned char Ac2000PorMechWaterFeed[2][AC2000ProCmdLength];
extern const unsigned char Ac2000ProMechAddSystem[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ProMechWaterOnDemand[2][AC2000ProCmdLength];
extern const unsigned char Ac2000ProMechNatural[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ProMechFeedScale[2][AC2000ProCmdLength];
extern const unsigned char Ac2000ManageProChicken[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ProManageFeed[15][AC2000ProCmdLength];
extern const unsigned char Ac2000ProMangageTimeDay[2][AC2000ProCmdLength];
extern const unsigned char Ac2000Chickens[2][AC2000ProCmdLength];
extern const unsigned char Ac2000AlarmSetting[2][AC2000ProCmdLength];
extern const unsigned char Ac2000NoSuccessfulSet[1][AC2000ProCmdLength];
extern const unsigned char Ac2000MangPassword[1][AC2000ProCmdLength];
extern const unsigned char Ac2000TestSwitch[2][AC2000ProCmdLength];
extern const unsigned char Ac2000TestAnalogsensor[1][AC2000ProCmdLength];
extern const unsigned char Ac2000TestDialogSensor[1][AC2000ProCmdLength];
extern const unsigned char Ac2000TestStaticPressure[1][AC2000ProCmdLength];
extern const unsigned char Ac2000Server_TemTemperaturecalibration[2][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerHumidtiycalibration[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerFeedCalibration[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerWaterCalibration[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerPluginterface[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerWaterWaterlinenipple[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerFeeder[1][AC2000ProCmdLength];
extern const unsigned char Ac2000ServerAmmoniaCalibration[1][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallCurtainSetting[2][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallTemperaturedefinition[2][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallFanAirCapacity[2][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallChickSizeof[1][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallCommunication[1][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallSetup[1][AC2000ProCmdLength];
extern const unsigned char Ac2000InstallRelaysensor[2][AC2000ProCmdLength];

#if SINGLE_PRO_TEST==1
extern const unsigned char AC2000ProCmd[3][AC2000CmdLength];
#else
extern const unsigned char AC2000ProCmd[3][AC2000CmdLength];
#endif

void CreateAC2000ProSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
void connect_ac2000_Pro(uint8_t *buff,uint8_t house);
int Ac2000DataProParse(uint8_t *data,uint16_t Counter);
void Ac2000DataProMalloc(void);
void CreateAC2000PorNormal(const uint8_t CmdSource[][AC2000ProCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
void PorOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000ProCmdLength],uint8_t cmd_num,uint8_t *cmd_index);

void Por_Coll_Temperaturecalibration(unsigned char *dataSource,uint16_t len);
void Por_Coll_Humidity(unsigned char *dataSource,uint16_t len);
void Por_Coll_Min_Max(unsigned char *dataSource,uint16_t len);
void Por_Coll_Static_Pressure(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_temperaturecali(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Cycleheater(unsigned char  *dataSource,uint16_t len);
void Por_Coll_System_Radiant_heater(unsigned char  *dataSource,uint16_t len);
void Por_Coll_System_VariableHeater(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Co2_Treatment(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Ammonia(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Staticpressure(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_ventilation_tunnel(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Curtains_roof(unsigned char *dataSource,uint16_t len);//Curtains 跟屋顶
void Por_Coll_System_Wetcurtain_Cool(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_humidifier(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_light(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Feed_stock(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_feed_water(unsigned char *dataSource,uint16_t len);
void Por_Coll_System_Nec_Wre(unsigned char *dataSource,uint16_t len);
void Por_Coll_Co2Mode(unsigned char *dataSource,uint16_t len);
void Por_Coll_Ammoniatreatment(unsigned char *dataSource,uint16_t len);
//机件
void Por_Mechanism_Ventilationlevel(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_Inverterfan(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_Ventilation_curtain_rating(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_Stirfan(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_Stirring_fanprogram(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_sprayer(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_Refrigeratedversion(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_light_Nofeed(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_light_feed(unsigned char *dataSource,uint16_t len);
void Por_Mechanism_water_feed(unsigned char *dataSource,uint16_t len);

//管理
void Por_Management_ChickenSum(unsigned char *dataSource,uint16_t len);
void Por_management_Feed(unsigned char *dataSource,uint16_t len);
void Por_MangTimeDay(unsigned char *dataSource,uint16_t len);
void Por_MangDayoldflock(unsigned char *dataSource,uint16_t len);
void Por_MangAlarm_system(unsigned char *dataSource,uint16_t len);
void Pro_Mang_Nosuccessfulsettingssaved(unsigned char *dataSource,uint16_t len);
void Pro_MangPassword(unsigned char *dataSource,uint16_t len);

//测试
void Pro_TestSwitchrelay(unsigned char *dataSource,uint16_t len);
void Pro_TestAnalogsensor(unsigned char *dataSource,uint16_t len);
void Pro_TestDialogSenSor(unsigned char *dataSource,uint16_t len);
void Pro_TestStaticpressure(unsigned char *dataSource,uint16_t len);
//服务
void Pro_Service_Temperaturecalibration(unsigned char *dataSource,uint16_t len);
void Pro_Server_Humiditycalibration(unsigned char *dataSource,uint16_t len);
void Pro_Server_FeedCalibration(unsigned char *dataSource,uint16_t len);
void Pro_Server_WaterCalibration(unsigned char *dataSource,uint16_t len);
void Pro_Server_Pluginterface(unsigned char *dataSource,uint16_t len);
void Pro_Server_Waterlinenipplerinse(unsigned char *dataSource,uint16_t len);
void Pro_Server_Waterfeeder(unsigned char *dataSource,uint16_t len);
void Pro_Server_AmmoniaCalibration(unsigned char *dataSource,uint16_t len);

//安装
void Pro_Install_Setting(unsigned char *dataSource,uint16_t len);
void Pro_Install_Relaysensor(unsigned char *dataSource,uint16_t len);
void Pro_Install_Curtainsetting(unsigned char *dataSource,uint16_t len);
void Por_Temperaturedefinition_sum(unsigned char *dataSource,uint16_t len);
void Pro_Install_Temperaturedefinition(unsigned char *dataSource,uint16_t len);
void Pro_Install_Fanaircapacity(unsigned char *dataSource,uint16_t len);
void Pro_InstallHenSize(unsigned char *dataSource,uint16_t len);
void Pro_InstallCommunication(unsigned char *dataSource,uint16_t len);



void Lengthacquisition(unsigned char *dataSource,uint16_t len);


void relaySend(const uint8_t Cmd[][AC2000ProCmdLength],unsigned short porv,unsigned char difference);
void RelayCreateAC2000PorNormal(const uint8_t CmdSource[][AC2000ProCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex,unsigned short dex,unsigned short prov,unsigned char state,unsigned short prov1);

void PorWrite_data_Empty(void);
typedef struct PORSYSTEMPARAMETER
{
	char SyTemperature_curve[PNORMAL_PARAMS_LEN];
	char Cycle_Heater[PNORMAL_PARAMS_LEN];
	char SyRadiant_heater[PNORMAL_PARAMS_LEN];
	char Variable_heater[PNORMAL_PARAMS_LEN];
	char Humidity_treatment[PNORMAL_PARAMS_LEN];
	char Co2_dealwith[PNORMAL_PARAMS_LEN];
	char Ammonia_Treatment[PNORMAL_PARAMS_LEN];
	char Static_pressure[PNORMAL_PARAMS_LEN];
	char Ventilation_level_tunnel[PNORMAL_PARAMS_LEN];
	char Curtains_Vents[PNORMAL_PARAMS_LEN];
	char Curtain_Calibraation[PNORMAL_PARAMS_LEN];
	char roof[PNORMAL_PARAMS_LEN];
	char Wet_curtain[PNORMAL_PARAMS_LEN];
	char Cool_pad_Plush[PNORMAL_PARAMS_LEN]; 
	char humidifier[PNORMAL_PARAMS_LEN];
	char light[PNORMAL_PARAMS_LEN];
	char Water_feed[PNORMAL_PARAMS_LEN];
	char Feed_stock[PNORMAL_PARAMS_LEN];
	char Natural_entrance_conditions[PNORMAL_PARAMS_LEN];
	char Wind_Rain_Effect[PNORMAL_PARAMS_LEN];
	char Water_Demand[PNORMAL_PARAMS_LEN];
	char Alarm_system[PNORMAL_PARAMS_LEN];

}PorContorSystemparameter;


typedef struct PORAC2000CONTROL  //控制大区数据
{
  char PorAc2000ControlTemperatureCurve[PNORMAL_PARAMS_LEN];//温度
	char Porshumidity_treatm[PNORMAL_PARAMS_LEN]; //湿度处理
	char Pormin_max_level[PNORMAL_PARAMS_LEN];    //最高最低级别
	char PorStaticpressure[PNORMAL_PARAMS_LEN];   //静压
	char PorControlmode[PHIGH_PARAMS_LEN];        //控制模式
   PorContorSystemparameter PorContorSystem;  //系统参数
	char PorCo2processing[PNORMAL_PARAMS_LEN];    //CO2处理
	char PorAmmoniaTreatment[PNORMAL_PARAMS_LEN]; //氨处理
  //
}PorAc2000Control;


typedef struct PORAC2000MECHANISM
{
	char PorVentilationlevel[High_LEN]; 	   //通风级别
	char PorSpeedcontrolfanlevel[PNORMAL_PARAMS_LEN]; //调速风扇级别
	char PorCurtainlevel[PNORMAL_PARAMS_LEN];         //风帘级别
	char PorStirringfanlevel[PNORMAL_PARAMS_LEN];     //搅扇级别
	char PorStirringfanprogram[PNORMAL_PARAMS_LEN];   //搅扇程序
	char Porsprayer[PNORMAL_PARAMS_LEN];              //喷雾器
	char PorRefrigerationplate[PNORMAL_PARAMS_LEN];   //制冷板
	char Porlight_feed[PNORMAL_PARAMS_LEN];                //灯光
	char Porlight_Nofeed[PNORMAL_PARAMS_LEN];                //灯光
	char PorWaterfeed[PNORMAL_PARAMS_LEN];            //水和饲料
	char PorAdditionalsystem[PNORMAL_PARAMS_LEN];     //附加系统
	char PorWaterOnDemand[PNORMAL_PARAMS_LEN];        //Water On Demand 
	char PorNaturalProgram[PNORMAL_PARAMS_LEN];       //Natural Program
	char PorFeedScaleProgram[PNORMAL_PARAMS_LEN];     //Feed Scale Program
}PorAc2000Mechanism;


typedef struct PORAC2000MANAGE
{
	char PorManageChickenstock[PNORMAL_PARAMS_LEN];            //鸡的存量
	char PorManageFeedstock[PHIGH_PARAMS_LEN];			         //饲料的存量
	char PorManageTimedate[PNORMAL_PARAMS_LEN];			           //时间和日期
	char PorManageGrowthperiodflock[PNORMAL_PARAMS_LEN];      //生长期和鸡群
	char PorManageAlarmsettings[PNORMAL_PARAMS_LEN];	        //警报设置
	char PorManageAlarmreset[PNORMAL_PARAMS_LEN];			        //警报重置
	char PorManageSavesuccessfulsettings[PNORMAL_PARAMS_LEN];	//没有保存成功设置
	char PorManagepassword[PNORMAL_PARAMS_LEN];			          //密码
	char PorManageRelaycurrentsetting[PNORMAL_PARAMS_LEN];		//继电器电流设定

}PorAc2000Manage;

typedef struct PORAC2000CHICKENS
{
	char PorChickChickenscalelayout[PNORMAL_PARAMS_LEN];      //鸡秤布局
	char PorChickChickenscalesettings[PNORMAL_PARAMS_LEN];    //鸡秤设置
	char PorChickSiloSetting[PNORMAL_PARAMS_LEN];             //silo setting 
	char PorChickBirdWeight[PNORMAL_PARAMS_LEN];              //Bird Weight 
	char PorChickChickenscalehistory[PNORMAL_PARAMS_LEN];     //鸡秤历史
	char PorChickFeedconversion[PNORMAL_PARAMS_LEN];          //饲料转换
	char PorChickChickenscaletest[PNORMAL_PARAMS_LEN];        //鸡秤测试
	char PorChickChickenscalecalibration[PNORMAL_PARAMS_LEN]; //鸡秤校准
	char PorChickSiloAugerLaout[PNORMAL_PARAMS_LEN];          //Silo/Auger Layout 
	
}PorAc2000Chickenscale;

typedef struct PORAC2000TEST
{
	char PorTestswitch[PNORMAL_PARAMS_LEN];                  //开关和继电器
	char PorTestAnalogsensor[PNORMAL_PARAMS_LEN];            //模拟传感器
	char PorTestDigitalsensor[PNORMAL_PARAMS_LEN];           //数字传感器
	char PorTestStaticpressure[PNORMAL_PARAMS_LEN];          //静压
	char PorTestHardwareinventory[PNORMAL_PARAMS_LEN];       //硬件清单
}PorAc2000Test;


typedef struct PORAC2000SERVICE
{
	char PorSerTemperaturecalibration[PNORMAL_PARAMS_LEN];    //温度校准
	char PorSerHumiditycalibration[PNORMAL_PARAMS_LEN];       //湿度校准
	char PorSerFeedcalibration[PNORMAL_PARAMS_LEN];           //饲料校准
	char PorSerWatercalibration[PNORMAL_PARAMS_LEN];          //水的校准
	char PorSerSetpluginterface[PNORMAL_PARAMS_LEN];          //设置插头界面
	char PorSerWaterlinenipplerinse[PNORMAL_PARAMS_LEN];      //水线乳头冲洗
	char PorSerFeeder[PNORMAL_PARAMS_LEN];                    //喂料器和喂水器
	char PorSerAmmoniaCalibration[PNORMAL_PARAMS_LEN];        //Ammonia Calibration
	
}PorAc2000service;

typedef struct PORAC2000INSTALL
{
	uint8_t AnalogSensorAndOutput;
	char PorInstallSetup[PNORMAL_PARAMS_LEN]; 								 //设置
	char PorInstallRelaylayout[PNORMAL_PARAMS_LEN];           //继电器布局
	char PorInstallAnalogsensor[PNORMAL_PARAMS_LEN];          //模拟传感器
	char PorInstallDigitalsensor[PNORMAL_PARAMS_LEN];         //数字传感器
	char PorInstallAnalogoutput[PNORMAL_PARAMS_LEN];          //模拟输出
	char PorInstallCurtainsetting[PNORMAL_PARAMS_LEN];        //风帘设置
	char PorInstallTemperaturedefinition[PNORMAL_PARAMS_LEN]; //温度定义
	char PorInstallFanaircapacity[PNORMAL_PARAMS_LEN];        //风扇空气容量
	char PorInstallCoopsize[PNORMAL_PARAMS_LEN];              //鸡舍大小
	char PorInstallCommunication[PNORMAL_PARAMS_LEN];         //通信
	char PorInstallLightDimmer[PNORMAL_PARAMS_LEN];           //Light Dimmer
	
}PorAc2000Install;

typedef struct PORAC2000PARAMS
{
	PorAc2000Control 			Ac2000PorControl;   //控制大区
	PorAc2000Mechanism    Ac2000PorMechanism; //机件大区
	PorAc2000Manage  			Ac2000PorManage;    //管理大区
	PorAc2000Chickenscale Ac2000PorChickenscale; //鸡秤大区
	PorAc2000Test  				Ac2000PorTest;      //测试大区
	PorAc2000service  	  Ac2000Porservice;   //服务大区
	PorAc2000Install  		Ac2000PorInstall;   //安装大区
}PorAc2000Params;



extern PorAc2000Params *Ac2000PorParams;



#endif

