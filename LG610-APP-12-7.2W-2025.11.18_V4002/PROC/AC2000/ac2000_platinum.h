#ifndef __AC2000_PLATINUM_H
#define __AC2000_PLATINUM_H

#include "stm32h7xx.h"
#include "data_table.h"
#define AC2000_Platinum_CmdNum 9


extern int PlatinumRecvFlag;
#define AC2000PlatinumCmdLength 18 
#define PLOW_PARAMS_LEN            128
#define PNORMAL_PARAMS_LEN         256
#define PHIGH_PARAMS_LEN           512
#define High_LEN                   2048

#define PlatinumCONTROTEMPERATURECURVE_CMD 1210  //???¨¨?¨²??
#define PlatinumCONTROHUMIDITYTREATM_CMD   1211  //¨ºa?¨¨¡ä|¨¤¨ª
#define PlatinumCONTROMINMAX_CMD           1212  //¡Á???¡Á?¦Ì¨ª??¡Àe  
#define PlatinumCONTROSTATICPRESSURE_CMD   1213  //?2?1
#define PlatinumCONTROMODE_CMD             1214  //?????¡ê¨º?
#define PlatinumCONTROSYSTEMMODE_CMD       1215  //?¦Ì¨ª3?¡ê¨º?
#define PlatinumCONTROCO2PROCESSING_CMD    1216  //Co2¡ä|¨¤¨ª
#define PlatinumCONTROAMMONIATREATMENT_CMD 1217  //¡ã¡À¡ä|¨¤¨ª


#define PlatinumMECHANISMVENTILATIONLEVEL_CMD     1218  //¨ª¡§¡¤???¡Àe
#define PlatinumMECHANISMSPEEDCONTROFAN_CMD       1219  //¦Ì¡Â?¨´¡¤?¨¦¨¨??¡Àe
#define PlatinumMECHANISMCURTAINLEVEL_CMD         1220  //¡¤?¨¢¡À??¡Àe
#define PlatinumMECHANISMSTIRRINGFANLEVEL_CMD     1221  //?¨¢¨¦¨¨??¡Àe
#define PlatinumMECHANISMSTIRRINGFANPROGRAM_CMD   1222  //?¨¢¨¦¨¨3¨¬D¨°
#define PlatinumMECHANISMSPRAYER_CMD              1223  //???¨ª?¡Â
#define PlatinumMECHANISMREFRIGERATION_CMD        1224  //??¨¤?¡ã?
#define PlatinumMECHANISMLIGHT_FEED_CMD           1225  //¦Ì?1a¨®D??¨¢?
#define PlatinumMECHANISMLIGHT_CMD                1226  //¦Ì?1a?T??¨¢?
#define PlatinumMECHANISMWATERFEED_CMD            1227  //??o¨ª??¨¢?
#define PlatinumMECHANISMADDITIONALSYSTEM_CMD     1228  //???¨®?¦Ì¨ª3
#define PlatinumMECHANISMWATERONDEMAND_CMD        1229  //Water On Demand 



#define PlatinumMANAGECHICKENSTOCK_CMD            1232  //?|¦Ì?¡ä?¨¢?
#define PlatinumMANAGEFEEDSTOCK_CMD           		1233  //??¨¢?¦Ì?¡ä?¨¢?
#define PlatinumMANAGETOMEDATE_CMD           		  1234  //¨º¡À??o¨ª¨¨??¨²
#define PlatinumMANAGEGROWTHPERIODFLOCK_CMD       1235  //¨¦¨²3¡è?¨²o¨ª?|¨¨o
#define PlatinumMANAGEALARMSETTING_CMD            1236  //?¡¥¡À¡§¨¦¨¨??
#define PlatinumMANAGEALARMRESET_CMD            	1237	 //?¡¥¡À¡§????
#define PlatinumMANAGESAVESUCCESS_CMD             1238  //??¨®D¡À¡ê¡ä?3¨¦1|¨¦¨¨??
#define PlatinumMANAGEPASSWORD_CMD            		1239  //?¨¹??
#define PlatinumMANAGERELAYCURR_CMD               1240  //?¨¬¦Ì??¡Â¦Ì?¨¢¡Â¨¦¨¨?¡§


#define PlatinumTESTSWITCH_CMD                    1241  //?a1?o¨ª?¨¬¦Ì??¡Â
#define PlatinumTESTANALOGSENSOR_CMD              1242  //?¡ê?a¡ä??D?¡Â
#define PlatinumTESTDIGITALSENSOR_CMD             1243  //¨ºy¡Á?¡ä??D?¡Â
#define PlatinumTESTSTATICPRESSURE_CMD            1244  //?2?1
#define PlatinumTESTHARDWAREINVENTORY_CMD         1245  //¨®2?t??¦Ì£¤

#define PlatinumSERVICETEMPERATURE_CMD            1246   //???¨¨D¡ê¡Á?
#define PlatinumSERVICEHUMIDITY_CMD               1247   //¨ºa?¨¨D¡ê¡Á?
#define PlatinumSERVICEFEED_CMD                   1248   //??¨¢?D¡ê¡Á?
#define PlatinumSERVICEWATER_CMD                  1249   //??¦Ì?D¡ê¡Á?
#define PlatinumSERVICESETPLUG_CMD                1250   //¨¦¨¨??2?¨ª¡¤????
#define PlatinumSERVICEWATERLINENIPPLERINSE_CMD   1251   //????¨¨¨¦¨ª¡¤3??¡ä
#define PlatinumSERVICEENNDER_CMD                 1252   //?1¨¢??¡Âo¨ª?1???¡Â
#define PlatinumSERVICEAMMONIACALIBRATION_CMD     1253   //Ammonia Calibration

#define PlatinumINSTALL_SETUP_CMD                 1254   //¨¦¨¨??
#define PlatinumINSTALL_RELAYLAYOUT_CMD           1255   //?¨¬¦Ì??¡Â2???
#define PlatinumINSTALL_ANALOGSENSOR_CMD          1256   //?¡ê?a¡ä??D?¡Â
#define PlatinumINSTALL__DIGITALSENSOR_CMD        1257   //¨ºy¡Á?¡ä??D?¡Â
#define PlatinumINSTALL_ANALOGOUTPUT_CMD          1258   //?¡ê?a¨º?3?
#define PlatinumINSTALL_CURTAINSETTING_CMD        1259   //¡¤?¨¢¡À¨¦¨¨??
#define PlatinumINSTALL_TEMPERATUREDE_CMD         1260   //???¨¨?¡§¨°?
#define PlatinumINSTALL_FANAIRCAPACITY_CMD        1261   //¡¤?¨¦¨¨????¨¨Y¨¢?
#define PlatinumINSTALL_COOPSIZE_CMD              1262   //?|¨¦¨¢¡ä¨®D?
#define PlatinumINSTALL_COMMUNICATION_CMD         1263   //¨ª¡§D?
#define PlatinumINSTALL_LIGHTDIMMER_CMD           1264   //light dimmer

extern unsigned char TunnelFan_Buffer[20];
extern unsigned char ExhaustFan_Buffer[10];

extern const unsigned char AC2000PlatinumContorTemp[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorHumitidy[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorMinMax[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorStaticpressure[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorMode[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumSystemparameters[17][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorCo2processing[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumContorAmmoniatreatment[1][AC2000PlatinumCmdLength];

extern const unsigned char Ac2000PlatinumMechVentilationlevel[3][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechSpeedcontrolfanlevel[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechCurtainlevel[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechStirfan[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechstirringfanprogarm[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechsprayer[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechRefrigerationplate[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechLightNoFeed[3][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechLightFeed[3][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechWaterFeed[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechAddSystem[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechWaterOnDemand[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechNatural[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMechFeedScale[2][AC2000PlatinumCmdLength];

extern const unsigned char Ac2000PlatinumManageProChicken[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumManageFeed[8][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMangageTimeDay[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumManageChickens[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumManageAlarmSetting[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumManageAlarmReset[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumManageNoSuccessfulSet[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumMangPassword[1][AC2000PlatinumCmdLength];

extern const unsigned char Ac2000PlatinumTestSwitch[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumTestAnalogsensor[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumTestDialogSensor[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumTestStaticPressure[1][AC2000PlatinumCmdLength];

extern const unsigned char Ac2000PlatinumServer_TemTemperaturecalibration[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerHumidtiycalibration[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerFeedCalibration[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerWaterCalibration[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerPluginterface[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerWaterWaterlinenipple[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerFeeder[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumServerAmmoniaCalibration[1][AC2000PlatinumCmdLength];

extern const unsigned char Ac2000PlatinumInstallCurtainSetting[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallTemperaturedefinition[4][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallFanAirCapacity[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallChickSizeof[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallCommunication[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallSetup[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallRelayLayout[2][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallAnalogSensor[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallDialogSensor[1][AC2000PlatinumCmdLength];
extern const unsigned char Ac2000PlatinumInstallAnalogOutput[1][AC2000PlatinumCmdLength];


//#if SINGLE_PLATINUM_TETS==1
extern const unsigned char AC2000PlatinumCmd[2][AC2000CmdLength];
//#else
//extern const unsigned char AC2000PlatinumCmd[AC2000_Platinum_CmdNum][AC2000CmdLength];
//#endif
void CreateAC2000PlatinumSendBuf(uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
extern void connect_ac2000_Platinum(uint8_t *buff,uint8_t house);
void Ac2000DataPlatinumMalloc(void);
void CreateAC2000PlatinumNormal(const uint8_t CmdSource[][AC2000PlatinumCmdLength],uint8_t *AC2000_SendBuff,uint8_t HenHouseNum,uint8_t CmdIndex);
void PlatinumOverLoadSendNormalCmd(const uint8_t Cmd[][AC2000PlatinumCmdLength],uint8_t cmd_num,uint8_t *cmd_index);



void Platinum_Coll_Temperaturecalibration(unsigned char *dataSource,uint16_t len);
void Platinum_Coll_Humidity(unsigned char *dataSource,uint16_t len);
void Platinum_Coll_Min_Max(unsigned char *dataSource,uint16_t len);
void Platinum_Coll_Static_Pressure(unsigned char *dataSource,uint16_t len);
void Platinum_Coll_ControlMode(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Shutter_Roof(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_WetCurtain(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_humidifier(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Water_Feed(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Waterstock(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Activate_funnel(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_EmerGency_Setting(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_light(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_temperaturecali(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Radiant_heater(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_Humidity_treatment(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_System_VariableHeater(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_Alarmset(uint8_t *dataSource,uint16_t len);
void Platinum_Coll_Co2Mode(uint8_t *dataSource,uint16_t len);


void Platinum_Coll_Co2Treatment(uint8_t *dataSource,uint16_t len);
//?¨²
//1¨¹¨¤¨ª
void Platinum_Manage_chicken_stock(uint8_t *dataSource,uint16_t len);
void Platinum_Manage_Feed_Stock(uint8_t *dataSource,uint16_t len);
void Platinum_ManageTimeAndDate1(uint8_t *dataSource,uint16_t len);
void Platinum_ManageGrowingData(uint8_t *dataSource,uint16_t len);
void Platinum_ManageAlarmSet(uint8_t *dataSource,uint16_t len);
void Platinum_ManageNoSaveSuccess(uint8_t *dataSource,uint16_t len);
void Platinum_ManagePassWord(uint8_t *dataSource,uint16_t len);

//¨¦¨¨¡À?
void Platinum_Mechanism_Ventilationlevel(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_Inverterfan(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_Ventilation_curtain_rating(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_Stirfan(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_Stirring_fanprogram(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_sprayer(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_Refrigeratedversion(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_light_Nofeed(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_light_feed(unsigned char *dataSource,uint16_t len);
void Platinum_Mechanism_water_feed(unsigned char *dataSource,uint16_t len);

//2a¨º?
void Platinum_TestRelay(uint8_t *dataSource,uint16_t len);
void Platinum_TestAnalogsensor(uint8_t *dataSource,uint16_t len);
void Platinum_TestDialogsensor(uint8_t *dataSource,uint16_t len);
void Platinum_TestStaticPressure(uint8_t *dataSource,uint16_t len);

//¡¤t??
void Platinum_ServiceTemperaturecalibration(uint8_t *dataSource,uint16_t len);
void Platinum_ServiceHumiditycalibration(uint8_t *dataSource,uint16_t len);
void Platinum_ServiceFeedcalibration(uint8_t *dataSource,uint16_t len);
void Platinum_ServiceWatercalibraton(uint8_t *dataSource,uint16_t len);
void Platinum_ServiceSetpluginterface(uint8_t *dataSource,uint16_t len);
void Platinum_ServiceWaterlinenippleflushing(uint8_t *dataSource,uint16_t len);
//¡ã2¡Á¡ã
void Platinum_Install_Set(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Relay_Layout(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Analogsensor(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Digitalsensor(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Analogoutput(uint8_t *dataSource,uint16_t len);
void Platinum_Install_windcurtainset(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Temperaturedefined(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Fan_capacity(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Henhousesize(uint8_t *dataSource,uint16_t len);
void Platinum_Install_Communication(uint8_t *dataSource,uint16_t len);



int Ac2000DataParsePlatinum(uint8_t *data,unsigned short Counter);










typedef struct PlatinumSYSTEMPARAMETER
{
	char SyTemperature_curve[PNORMAL_PARAMS_LEN];
	char Cycle_Heater[PNORMAL_PARAMS_LEN];
	char SyRadiant_heater[PNORMAL_PARAMS_LEN];
	char Variable_heater[PNORMAL_PARAMS_LEN];
	char Humidity_treatment[PNORMAL_PARAMS_LEN];
	char Co2_dealwith[PNORMAL_PARAMS_LEN];
	char Ammonia_Treatment[PNORMAL_PARAMS_LEN];
	char Static_pressure[PNORMAL_PARAMS_LEN];
	char Ventilation_level[PNORMAL_PARAMS_LEN];
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
	char Min_Max_Level[PNORMAL_PARAMS_LEN];
	char Activate_funnel[PNORMAL_PARAMS_LEN];
	char Emergency_Settings[PNORMAL_PARAMS_LEN];
}PlatinumContorSystemparameter;


typedef struct PlatinumAC2000CONTROL  //????¡ä¨®??¨ºy?Y
{
  char PlatinumAc2000ControlTemperatureCurve[PNORMAL_PARAMS_LEN];//???¨¨
	char Platinumshumidity_treatm[PNORMAL_PARAMS_LEN]; //¨ºa?¨¨¡ä|¨¤¨ª
	char Platinummin_max_level[PNORMAL_PARAMS_LEN];    //¡Á???¡Á?¦Ì¨ª??¡Àe
	char PlatinumStaticpressure[PNORMAL_PARAMS_LEN];   //?2?1
	char PlatinumControlmode[PHIGH_PARAMS_LEN];        //?????¡ê¨º?
   PlatinumContorSystemparameter PlatinumContorSystem;  //?¦Ì¨ª32?¨ºy
	char PlatinumCo2processing[PNORMAL_PARAMS_LEN];    //CO2¡ä|¨¤¨ª
	char PlatinumAmmoniaTreatment[PNORMAL_PARAMS_LEN]; //¡ã¡À¡ä|¨¤¨ª
  //
}PlatinumAc2000Control;


typedef struct PlatinumAC2000MECHANISM
{
	char PlatinumVentilationlevel[High_LEN]; 	   //¨ª¡§¡¤???¡Àe
	char PlatinumSpeedcontrolfanlevel[PNORMAL_PARAMS_LEN]; //¦Ì¡Â?¨´¡¤?¨¦¨¨??¡Àe
	char PlatinumCurtainlevel[PNORMAL_PARAMS_LEN];         //¡¤?¨¢¡À??¡Àe
	char PlatinumStirringfanlevel[PNORMAL_PARAMS_LEN];     //?¨¢¨¦¨¨??¡Àe
	char PlatinumStirringfanprogram[PNORMAL_PARAMS_LEN];   //?¨¢¨¦¨¨3¨¬D¨°
	char Platinumsprayer[PNORMAL_PARAMS_LEN];              //???¨ª?¡Â
	char PlatinumRefrigerationplate[PNORMAL_PARAMS_LEN];   //??¨¤?¡ã?
	char Platinumlight_feed[PNORMAL_PARAMS_LEN];                //¦Ì?1a
	char Platinumlight_Nofeed[PNORMAL_PARAMS_LEN];                //¦Ì?1a
	char PlatinumWaterfeed[PNORMAL_PARAMS_LEN];            //??o¨ª??¨¢?
	char PlatinumAdditionalsystem[PNORMAL_PARAMS_LEN];     //???¨®?¦Ì¨ª3
	char PlatinumWaterOnDemand[PNORMAL_PARAMS_LEN];        //Water On Demand 
	char PlatinumNaturalProgram[PNORMAL_PARAMS_LEN];       //Natural Program
	char PlatinumFeedScaleProgram[PNORMAL_PARAMS_LEN];     //Feed Scale Program
}PlatinumAc2000Mechanism;


typedef struct PlatinumAC2000MANAGE
{
	char PlatinumManageChickenstock[PNORMAL_PARAMS_LEN];           //?|¦Ì?¡ä?¨¢?
	char PlatinumManageFeedstock[PHIGH_PARAMS_LEN];			         //??¨¢?¦Ì?¡ä?¨¢?
	char PlatinumManageTimedate[PNORMAL_PARAMS_LEN];			           //¨º¡À??o¨ª¨¨??¨²
	char PlatinumManageGrowthperiodflock[PNORMAL_PARAMS_LEN];      //¨¦¨²3¡è?¨²o¨ª?|¨¨o
	char PlatinumManageAlarmsettings[PNORMAL_PARAMS_LEN];	        //?¡¥¡À¡§¨¦¨¨??
	char PlatinumManageAlarmreset[PNORMAL_PARAMS_LEN];			        //?¡¥¡À¡§????
	char PlatinumManageSavesuccessfulsettings[PNORMAL_PARAMS_LEN];	//??¨®D¡À¡ê¡ä?3¨¦1|¨¦¨¨??
	char PlatinumManagepassword[PNORMAL_PARAMS_LEN];			          //?¨¹??
	char PlatinumManageRelaycurrentsetting[PNORMAL_PARAMS_LEN];		//?¨¬¦Ì??¡Â¦Ì?¨¢¡Â¨¦¨¨?¡§

}PlatinumAc2000Manage;

typedef struct PlatinumAC2000CHICKENS
{
	char PlatinumChickChickenscalelayout[PNORMAL_PARAMS_LEN];      //?|3¨®2???
	char PlatinumChickChickenscalesettings[PNORMAL_PARAMS_LEN];    //?|3¨®¨¦¨¨??
	char PlatinumChickSiloSetting[PNORMAL_PARAMS_LEN];             //silo setting 
	char PlatinumChickBirdWeight[PNORMAL_PARAMS_LEN];              //Bird Weight 
	char PlatinumChickChickenscalehistory[PNORMAL_PARAMS_LEN];     //?|3¨®¨¤¨²¨º¡¤
	char PlatinumChickFeedconversion[PNORMAL_PARAMS_LEN];          //??¨¢?¡Áa??
	char PlatinumChickChickenscaletest[PNORMAL_PARAMS_LEN];        //?|3¨®2a¨º?
	char PlatinumChickChickenscalecalibration[PNORMAL_PARAMS_LEN]; //?|3¨®D¡ê¡Á?
	char PlatinumChickSiloAugerLaout[PNORMAL_PARAMS_LEN];          //Silo/Auger Layout 
	
}PlatinumAc2000Chickenscale;

typedef struct PlatinumAC2000TEST
{
	char PlatinumTestswitch[PNORMAL_PARAMS_LEN];                  //?a1?o¨ª?¨¬¦Ì??¡Â
	char PlatinumTestAnalogsensor[PNORMAL_PARAMS_LEN];            //?¡ê?a¡ä??D?¡Â
	char PlatinumTestDigitalsensor[PNORMAL_PARAMS_LEN];           //¨ºy¡Á?¡ä??D?¡Â
	char PlatinumTestStaticpressure[PNORMAL_PARAMS_LEN];          //?2?1
	char PlatinumTestHardwareinventory[PNORMAL_PARAMS_LEN];       //¨®2?t??¦Ì£¤
}PlatinumAc2000Test;


typedef struct PlatinumAC2000SERVICE
{
	char PlatinumSerTemperaturecalibration[PNORMAL_PARAMS_LEN];    //???¨¨D¡ê¡Á?
	char PlatinumSerHumiditycalibration[PNORMAL_PARAMS_LEN];       //¨ºa?¨¨D¡ê¡Á?
	char PlatinumSerFeedcalibration[PNORMAL_PARAMS_LEN];           //??¨¢?D¡ê¡Á?
	char PlatinumSerWatercalibration[PNORMAL_PARAMS_LEN];          //??¦Ì?D¡ê¡Á?
	char PlatinumSerSetpluginterface[PNORMAL_PARAMS_LEN];          //¨¦¨¨??2?¨ª¡¤????
	char PlatinumSerWaterlinenipplerinse[PNORMAL_PARAMS_LEN];      //????¨¨¨¦¨ª¡¤3??¡ä
	char PlatinumSerFeeder[PNORMAL_PARAMS_LEN];                    //?1¨¢??¡Âo¨ª?1???¡Â
	char PlatinumSerAmmoniaCalibration[PNORMAL_PARAMS_LEN];        //Ammonia Calibration
	
}PlatinumAc2000service;

typedef struct PlatinumAC2000INSTALL
{
	uint8_t AnalogSensorAndOutput;
	char PlatinumInstallSetup[PNORMAL_PARAMS_LEN]; 								 //¨¦¨¨??
	char PlatinumInstallRelaylayout[PNORMAL_PARAMS_LEN];           //?¨¬¦Ì??¡Â2???
	char PlatinumInstallAnalogsensor[PNORMAL_PARAMS_LEN];          //?¡ê?a¡ä??D?¡Â
	char PlatinumInstallDigitalsensor[PNORMAL_PARAMS_LEN];         //¨ºy¡Á?¡ä??D?¡Â
	char PlatinumInstallAnalogoutput[PNORMAL_PARAMS_LEN];          //?¡ê?a¨º?3?
	char PlatinumInstallCurtainsetting[PNORMAL_PARAMS_LEN];        //¡¤?¨¢¡À¨¦¨¨??
	char PlatinumInstallTemperaturedefinition[PNORMAL_PARAMS_LEN]; //???¨¨?¡§¨°?
	char PlatinumInstallFanaircapacity[PNORMAL_PARAMS_LEN];        //¡¤?¨¦¨¨????¨¨Y¨¢?
	char PlatinumInstallCoopsize[PNORMAL_PARAMS_LEN];              //?|¨¦¨¢¡ä¨®D?
	char PlatinumInstallCommunication[PNORMAL_PARAMS_LEN];         //¨ª¡§D?
	char PlatinumInstallLightDimmer[PNORMAL_PARAMS_LEN];           //Light Dimmer
	
}PlatinumAc2000Install;

typedef struct PlatinumAC2000PARAMS
{
	PlatinumAc2000Control 			Ac2000PlatinumControl;   //????¡ä¨®??
	PlatinumAc2000Mechanism     Ac2000PlatinumMechanism; //?¨²?t¡ä¨®??
	PlatinumAc2000Manage  			Ac2000PlatinumManage;    //1¨¹¨¤¨ª¡ä¨®??
	PlatinumAc2000Chickenscale  Ac2000PlatinumChickenscale; //?|3¨®¡ä¨®??
	PlatinumAc2000Test  				Ac2000PlatinumTest;      //2a¨º?¡ä¨®??
	PlatinumAc2000service  	    Ac2000Platinumservice;   //¡¤t??¡ä¨®??
	PlatinumAc2000Install  		  Ac2000PlatinumInstall;   //¡ã2¡Á¡ã¡ä¨®??
}PlatinumAc2000Params; 



extern PlatinumAc2000Params *Ac2000PlatinumParams;

#endif


