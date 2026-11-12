#include "mynet.h"
#include "ac2000_3g.h"
#include "ac2000_plus.h"
#include "ac2000_platinum.h"
#include "ac2000_platinum_pro.h"
#include "data_table.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

unsigned char secret_key[17]="202005051400neal";
unsigned char INIT_IV[17]="1234567890123456";
unsigned char IV[17];

AIRXXXSTATUS AirxxxStatus;


void IntToString(int32_t num,char *string_tmp)
{
  sprintf(string_tmp,"%d",num);
} 
void Float1ToString(float num,char *string_tmp)
{
  sprintf(string_tmp,"%0.1f",num);
}
void Float2ToString(float num,char *string_tmp)
{
  sprintf(string_tmp,"%0.2f",num);
}

int get_alarm_num(int version)
{
	int num=0;
	if(version==1)
	{
		for(u16 j=0;j<=ALARM_NUM_3G;j++)
		{
			if(AlarmCode3g[ALARM_3G_11515+j][1]==1)
			{
				num++;
			}
		}
		//AIRXXX_DEBUG("\r\n 3g alarm_num=%d \r\n",num);
	}
	else if(version==2||version==3)
	{
		for(u16 j=0;j<=ALARM_NUM_PLATINUM;j++)
		{
			if(AlarmCodePlatinum[ALARM_PLATINUM_13000+j][1]==1)
			{
				num++;
			}
		}
		//AIRXXX_DEBUG("\r\n platinum alarm_num=%d \r\n",num);
	}
	else
	{
		for(u16 j=0;j<ALARM_NUM_3G;j++)
		{
			if(AlarmCode3g[ALARM_3G_11515+j][1]==1)
			{
				num++;
			}
		}
	
	}
	return num;
}

void PackageDataToServer(int index,char *data)
{
		if(ac2000_table.ac2000_3g_house!=0)
		{
		switch (index)//发送AC2000控制指令
		{
			
			case TEMPERATURE_CURVE_CMD:      //抓包获取的指令            
			{
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve1);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve2);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ControlTemperatureCurve.temperature_curve3);
			}break;
			case HUMIDITY_TREATM_CMD:    
			{
				strcat(data,Ac20003GParams->Ac2000Control.humidity_treatm);
			}break;
			case MIN_MAX_LEVEL_CMD:     
			{
				strcat(data,Ac20003GParams->Ac2000Control.min_max_level);
			}break;
			case STATIC_PRESSURE_CMD:             
			{
				strcat(data,Ac20003GParams->Ac2000Control.static_pressure);
			}break;
			case CONTROL_MODE_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Control.control_mode);
			}break;
			case SYSTEM_INFORAMATION_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_tmperature_curvel);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_cycle_heater);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_radiant_heater);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_variable_heater);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_humidity_treatment);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_C02_treatment);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_static_pressure);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_ventilation_level);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_rolling_shutter_and_roof);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_curtain_calibration);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_wet_curtain_cool_pad_flush);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_Humidifier);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_lighting);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_water);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_left);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_food_leftlog);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Control.Ac2000ContyrolSystemParams.system_inforamation_alarm_set);			
			}break;
			case CO2_HANDLE_CMD:                  
			{
				strcat(data,Ac20003GParams->Ac2000Control.CO2_Handle);
			}break;
			
			//设备
			case DEVICE_LEVELS_OF_VENTILATION_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.levels_of_ventilation);
			}break;
			case DEVICE_VARIABLE_SPEED_FAN_LEVELS_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.variable_speed_fan_levels);
			}break;
			case DEVICE_VENT_AND_CURTIAN_LEVELS_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.vent_and_curtain_Levels);
			}break;
			case DEVICE_STIR_FAN_LEVELS_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.stir_fan_levels);
			}break;
			
			case DEVICE_STIR_FAN_PROGRAM_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.stir_fan_program);
			}break;
			case DEVICE_FOGGERS_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.foggers);
			}break;
			case DEVICE_COOL_PAD_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.cool_pad);
			}break;
			case DEVICE_LIGHT_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.light);
			}break;
			case DEVICE_WATER_AND_FEED_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.water_and_feed);
			}break;
			case DEVICE_EXTRA_SYSTEM_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.extra_system);
			}break;
			case DEVICE_WATER_ON_DEMAND_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Device.water_on_demand);
			}break;

			//管理
			case MANAGE_STOCKOFCHICKEN_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.StockOfChicken);
			}break;
			case MANAGE_STOCKOFFEED_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.StockOfFeed);
			}break;
			case MANAGE_TIMEANDDATA_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.TimeAndDate1);
				strcat(data,",");
				strcat(data,Ac20003GParams->Ac2000Manage.TimeAndDate2);
      }break;
			case MANAGE_GROWTHANDHEN_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.GrowthAndHen);
			}break;
			case MANAGE_ALARMSET_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.AlarmSet);
			}break;
			case MANAGE_ALARMRESET_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.AlarmReset);
			}break;
			case MANAGE_PASSWORD_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Manage.Password);
			}break;
			
			//历史
			case HISTORT_TEMPERATURE_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Temperature);
			}break;
			case HISTORY_HUMIDITY_TREATM_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Humidity_treatm);
			}break;
			case HISTORY_CO2_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.CO2);
			}break;
			case HISTORY_WATER_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Water);
			}break;
			case HISTORY_FEED_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Feed);
			}break;
			case HISTORY_DEAD_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Dead);
			}break;
			case HISTORY_HEATER_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Heater);
			}break;			
			case HISTORY_RADIANT_HEATER_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Radiant_Heater);
			}break;
			case HISTORY_ALARM_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Alarm);
			}break;
			case HISTORY_EVENT_MAP_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Event_Map);
			}break;
			case HISTORY_WATCH_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Watch);
			}break;
			case HISTORY_SET_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000History.Set);
			}break;
		
			//测试
			case TEST_SWITCHRELAY_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Test.SwitchRelay);
			}break;
			case TEST_ANALOGSENSOR_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Test.AnalogSensor);
			}break;
			case TEST_DIGITALSENSOR_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Test.DigitalSensor);
			}break;
			case TEST_STATICPRESSURE_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Test.StaticPressure);
			}break;
						
			//服务
			case SERVICE_TEMPERATURE_ADJUST_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Service.Temperature);
			}break;
			case SERVICE_HUMIDITY_ADJUST_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Service.Humidity);
			}break;
			case SERVICE_FEED_ADJUST_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Service.Feed);
			}break;
			case SERVICE_WATER_ADJUST_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Service.Water);
			}break;
			
			//安装
			case INSTALL_NORMALSET_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.NormalSet);
			}break;
			case INSTALL_RELAYSETUP_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.RelaySetUp);
			}break;
			case INSTALL_ANALOGSENSOR_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.AnalogSensor);
			}break;
			case INSTALL_DIGITALSENSOR_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.DigitalSensor);
			}break;
			case INSTALL_ANALOGOUTPUT_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.AnalogOutput);
			}break;
			case INSTALL_VENTCURTAINSETUP_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.VentCurtainSetUp);
			}break;
			case INSTALL_TEMPERATUREDEFINE_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.TemperatureDefine);
			}break;
			case INSTALL_CAPACITYOFFANAIR_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.CapacityOfFanAir);
			}break;
			case INSTALL_HENSIZE_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.HenSize);
			}break;	
			case INSTALL_COMMUNICATION_CMD:                
			{
				strcat(data,Ac20003GParams->Ac2000Install.Communication);
			}break;
			
			default:break;
		}
	}
	else if(ac2000_table.ac2000_plus_house!=0)
	{
		switch (index)//发送AC2000控制指令
		{
			case PLUSTEMPERATURE_CURVE_CMD:      //抓包获取的指令            
			{
				//rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve);
				//rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve);
			}break;
			case PLUSMIN_MAX_LEVEL_CMD:
			{
			
				strcat(data,Ac2000PlusParams->PAc2000Control.Plusmin_max_level);
				//rt_kprintf("最小最大级别Ac2000PlusParams->PAc2000Control.Plusmin_max_level:%s\n",Ac2000PlusParams->PAc2000Control.Plusmin_max_level);
			}break;
			case PLUSHUMIDITY_TREATM_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.Plushumidity_treatm);
				//rt_kprintf("湿度Co2Ac2000PlusParams->PAc2000Control.Plushumidity_treatm:%s\n",Ac2000PlusParams->PAc2000Control.Plushumidity_treatm);
			}break;
			case PLUSREFRIGERATION_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.PlusRefrigeration);
				//rt_kprintf("制冷Ac2000PlusParams->PAc2000Control.PlusRefrigeration:%s\n",Ac2000PlusParams->PAc2000Control.PlusRefrigeration);
			}break;
			case PLUSILLUMINATION_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.Plusillumination);
				//rt_kprintf("照明Ac2000PlusParams->PAc2000Control.Plusillumination:%s\n",Ac2000PlusParams->PAc2000Control.Plusillumination);
			}break;
			case PLUSFEED_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.PlusFeed);
				//rt_kprintf("饲料Ac2000PlusParams->PAc2000Control.PlusFeed:%s\n",Ac2000PlusParams->PAc2000Control.PlusFeed);
			}break;
			case PLUSADDITIONAL_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.PlusAdditional_system);
				//rt_kprintf("附加系统Ac2000PlusParams->PAc2000Control.PlusAdditional_system:%s\n",Ac2000PlusParams->PAc2000Control.PlusAdditional_system);
			}break;
			case PLUSSTATICPRESSURE_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.Plusstatic_pressure);
				//rt_kprintf("压力Ac2000PlusParams->PAc2000Control.Plusstatic_pressure:%s\n",Ac2000PlusParams->PAc2000Control.Plusstatic_pressure);
			}break;
			case PLUSHUMIDIFIER_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Control.Plushumidifier);
				//rt_kprintf("加湿器Ac2000PlusParams->PAc2000Control.Plushumidifier:%s\n",Ac2000PlusParams->PAc2000Control.Plushumidifier);
			}break;
			
			//管理
			case MANAGEDEATH_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusChicken_Death);
			}break;
			case MANAGEPOUITRY_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusPoultry_metering);
			}break;
			case MANAGEFEEDSTOCK_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusFeed_stock);
			}break;
			case MANAGETIMEDAY_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusTime_day);
			}break;
			case MANAGESTOCK_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusFlock);
			}break;
			case MANAGENEWSTOCK_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusNew_Flock);
			}break;
			case MANAGEALARMTES_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusAlarm_reset);
			}break;
			case MANAGEALARMTEST_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Manage.PlusAlarm_test);
			}break;
			//测试
			
			case TESTTEMPERAURE_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTesttemperature);
			}break;
			case TESTHUMIDITY_CO2_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTesthumidityCo2);
			}break;
			case TESTRELAY_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestRelay);
			}break;
			case TESTDIGITAL_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestdigital);
			}break;
			case TESTSIMULATION_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestsimulation);
			}break;
			case TESTPULSE_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestpulse);
			}break;
			case TESTCHICKEN_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestChicken_scale);
			}break;
			case TESTVERSION_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Test.PlusTestversion);
			}break;
			//校准
			
			case CALIBRATIONTEMPERAURE_CMD:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_temperature);
			}break;
			case CALIBRATTONHUMIDITY:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_humidityCo2);
			}break;
			case CALIBRATIONCHICKEN_CMD:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Chicken_scale);
			}break;
			case CALIBRATIONFEED_CMD:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Feed_pulse_ratio);
			}break;
			case CALIBRATIONWATER_CMD:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Water_pulse_ratio);
			}break;
			case CALIBRATIONCO2_CMD:
			{
				strcat(data,Ac2000PlusParams->PlusAc2000calibration.PlusCalibration_Co2);
			}break;
			//安装
			case INSTALLCONFIGURATION_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallConfiguration);
			}break;
			case INSTALLVENTI_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallVentilation_level);
			}break;
			case INSTALLROLLERBLIND_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallRoller_blind);
			}break;
			case INSTALLRELAY_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallRelay_layout);
			}break;
			case INSTALLSENSOR_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallSensor_layout);
			}break;
			case INSTALLSYSTEM_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallSystem_parameters);
			}break;
			case INSTALLWINDCOLLING_CMD:
			{
				strcat(data,Ac2000PlusParams->PAc2000Install.PlusInstallWind_cooling);
			}break;
		}
	}
	else if(ac2000_table.ac2000_pro_house!=0)
	{
		switch (index)//发送AC2000控制指令
		{
			case PORCONTROTEMPERATURECURVE_CMD:               
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorAc2000ControlTemperatureCurve);
				rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PorParams->Ac2000PorControl.PorAc2000ControlTemperatureCurve);
			}break;
			case PORCONTROHUMIDITYTREATM_CMD:                 
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.Porshumidity_treatm);
				rt_kprintf("湿度Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PorParams->Ac2000PorControl.Porshumidity_treatm);
			}break;
			case PORCONTROMINMAX_CMD:                  
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.Pormin_max_level);
				rt_kprintf("最大最小Ac2000PorParams->Ac2000PorControl.Pormin_max_level:%s\n",Ac2000PorParams->Ac2000PorControl.Pormin_max_level);
			}break;
			case PORCONTROSTATICPRESSURE_CMD:                  
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorStaticpressure);
				rt_kprintf("静压Ac2000PlusParams->PAc2000Control.PorStaticpressure:%s\n",Ac2000PorParams->Ac2000PorControl.PorStaticpressure);
			}break;
			case PORCONTROMODE_CMD:                 
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorControlmode);
				rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PorControlmode:%s\n",Ac2000PorParams->Ac2000PorControl.PorControlmode);
			}break;
			case PORCONTROSYSTEMMODE_CMD:                  
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyTemperature_curve);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cycle_Heater);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.SyRadiant_heater);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Variable_heater);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Humidity_treatment);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Co2_dealwith);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ammonia_Treatment);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Static_pressure);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Ventilation_level_tunnel);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtains_Vents);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Curtain_Calibraation);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.roof);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wet_curtain);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Cool_pad_Plush);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.humidifier);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.light);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_feed);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Feed_stock);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Natural_entrance_conditions);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Wind_Rain_Effect);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Water_Demand);
				strcat(data,",");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorContorSystem.Alarm_system);
				strcat(data,",");
			}break;
			case PORCONTROCO2PROCESSING_CMD:                 
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorCo2processing);
				rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PorCo2processing:%s\n",Ac2000PorParams->Ac2000PorControl.PorCo2processing);
			}break;
			case PORCONTROAMMONIATREATMENT_CMD:                  
			{
				rt_kprintf("tianchongchenggong\n");
				strcat(data,Ac2000PorParams->Ac2000PorControl.PorAmmoniaTreatment);
				rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PorParams->Ac2000PorControl.PorAmmoniaTreatment);
			}break;
			//设备
			case  PORMECHANISMVENTILATIONLEVEL_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorVentilationlevel);
				rt_kprintf("温度Ac2000PlusParams->PAc2000Control.PlusAc2000ControlTemperatureCurve:%s\n",Ac2000PorParams->Ac2000PorMechanism.PorVentilationlevel);
			}break;
			case PORMECHANISMSPEEDCONTROFAN_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorSpeedcontrolfanlevel);
			}break;
			case PORMECHANISMCURTAINLEVEL_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorCurtainlevel);
			}break;
			case PORMECHANISMSTIRRINGFANLEVEL_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorStirringfanlevel);
			}break;
			case PORMECHANISMSTIRRINGFANPROGRAM_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorStirringfanprogram);
			}break;
			case PORMECHANISMSPRAYER_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.Porsprayer);
			}break;
			case PORMECHANISMREFRIGERATION_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorRefrigerationplate);
			}break;
			
			case PORMECHANISMLIGHT_FEED_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.Porlight_feed);
			}break;
			case PORMECHANISMLIGHT_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.Porlight_Nofeed);
			}break;
			case PORMECHANISMWATERFEED_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorWaterfeed);
			}break;
			case PORMECHANISMADDITIONALSYSTEM_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorAdditionalsystem);
			}break;
			case PORMECHANISMWATERONDEMAND_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorWaterOnDemand);
			}break;
			case PORMECHANISMNATURALPROGRAM_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorNaturalProgram);
			}break;
			case PORMECHANISMFEEDSCALEPROGRAM_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorMechanism.PorFeedScaleProgram);
			}break;
			//管理
			case PORMANAGECHICKENSTOCK_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageChickenstock);
			}break;
			case PORMANAGEFEEDSTOCK_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageFeedstock);
			}break;
			case PORMANAGETOMEDATE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageTimedate);
			}break;
			case PORMANAGEGROWTHPERIODFLOCK_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageGrowthperiodflock);
			}break;
			case PORMANAGEALARMSETTING_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageAlarmsettings);
			}break;
			case PORMANAGEALARMRESET_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageAlarmreset);
			}break;
			case PORMANAGESAVESUCCESS_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageSavesuccessfulsettings);
			}break;
			case PORMANAGEPASSWORD_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManagepassword);
			}break;
			case PORMANAGERELAYCURR_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorManage.PorManageRelaycurrentsetting);
			}break;
			//测试
			case PORTESTSWITCH_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorTest.PorTestswitch);
			}break;
			case PORTESTANALOGSENSOR_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorTest.PorTestAnalogsensor);
			}break;
			case PORTESTDIGITALSENSOR_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorTest.PorTestDigitalsensor);
			}break;
			case PORTESTSTATICPRESSURE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorTest.PorTestStaticpressure);
			}break;
			case PORTESTHARDWAREINVENTORY_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorTest.PorTestHardwareinventory);
			}break;
			//服务
			case PORSERVICETEMPERATURE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerTemperaturecalibration);
			}break;
			case PORSERVICEHUMIDITY_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerHumiditycalibration);
			}break;
			case PORSERVICEFEED_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerFeedcalibration);
			}break;
			case PORSERVICEWATER_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerWatercalibration);
			}break;
			case PORSERVICESETPLUG_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerSetpluginterface);
			}break;
			case PORSERVICEWATERLINENIPPLERINSE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerWaterlinenipplerinse);
			}break;
			case PORSERVICEENNDER_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerFeeder);
			}break;
			case PORSERVICEAMMONIACALIBRATION_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000Porservice.PorSerAmmoniaCalibration);
			}break;
			//安装
			case PORINSTALL_SETUP_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallSetup);
			}break;
			case PORINSTALL_RELAYLAYOUT_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallRelaylayout);
			}break;
			case PORINSTALL_ANALOGSENSOR_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogsensor);
			}break;
			case PORINSTALL__DIGITALSENSOR_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallDigitalsensor);
			}break;
			case PORINSTALL_ANALOGOUTPUT_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallAnalogoutput);
			}break;
			case PORINSTALL_CURTAINSETTING_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallCurtainsetting);
			}break;
			case PORINSTALL_TEMPERATUREDE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallTemperaturedefinition);
			}break;
			case PORINSTALL_FANAIRCAPACITY_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallFanaircapacity);
			}break;
			case PORINSTALL_COOPSIZE_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallCoopsize);
			}break;
			case PORINSTALL_COMMUNICATION_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallCommunication);
			}break;
			case PORINSTALL_LIGHTDIMMER_CMD:
			{
				strcat(data,Ac2000PorParams->Ac2000PorInstall.PorInstallLightDimmer);
			}
			
			
		}
	}
	else if(ac2000_table.ac2000_platinum_house!=0)
	{
		switch (index)//发送AC2000控制指令
		{
			case PlatinumMECHANISMVENTILATIONLEVEL_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumVentilationlevel);
			}break;
			case PlatinumMECHANISMSPEEDCONTROFAN_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumSpeedcontrolfanlevel);
			}break;
			case PlatinumMECHANISMCURTAINLEVEL_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumCurtainlevel);
			}break;
			case PlatinumMECHANISMSTIRRINGFANLEVEL_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanlevel);
			}break;
			case PlatinumMECHANISMSTIRRINGFANPROGRAM_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumStirringfanprogram);
			}break;
			case PlatinumMECHANISMSPRAYER_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumsprayer);
			}break;
			case PlatinumMECHANISMREFRIGERATION_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumRefrigerationplate);
			}break;
			case PlatinumMECHANISMLIGHT_FEED_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_feed);
			}break;
			case PlatinumMECHANISMLIGHT_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.Platinumlight_Nofeed);
			}break;
			case PlatinumMECHANISMWATERFEED_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterfeed);
			}break;
			case PlatinumMECHANISMADDITIONALSYSTEM_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumAdditionalsystem);
			}break;
			case PlatinumMECHANISMWATERONDEMAND_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumMechanism.PlatinumWaterOnDemand);
			}break;
			case PlatinumCONTROTEMPERATURECURVE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumAc2000ControlTemperatureCurve);
			}break;
			case PlatinumCONTROHUMIDITYTREATM_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.Platinumshumidity_treatm);
			}break;
			case PlatinumCONTROMINMAX_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.Platinummin_max_level);
			}break;
			case PlatinumCONTROSTATICPRESSURE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumStaticpressure);
			}break;
			case PlatinumCONTROMODE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumControlmode);
			}break;
			case PlatinumCONTROSYSTEMMODE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyTemperature_curve);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.SyRadiant_heater);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Variable_heater);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Humidity_treatment);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Co2_dealwith);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Min_Max_Level);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Static_pressure);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Ventilation_level);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.roof);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Wet_curtain);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.humidifier);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.light);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Water_feed);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Feed_stock);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Activate_funnel);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Emergency_Settings);
				strcat(data,",");
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumContorSystem.Alarm_system);
				strcat(data,",");
			}break;
			case PlatinumCONTROCO2PROCESSING_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumControl.PlatinumCo2processing);
			}break;
			//管理
			case PlatinumMANAGECHICKENSTOCK_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageChickenstock);
			}break;
			case PlatinumMANAGEFEEDSTOCK_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageFeedstock);
			}break;
			case PlatinumMANAGETOMEDATE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageTimedate);
			}break;
			case PlatinumMANAGEGROWTHPERIODFLOCK_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageGrowthperiodflock);
			}break;
			case PlatinumMANAGEALARMSETTING_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmsettings);
			}break;
			case PlatinumMANAGEALARMRESET_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageAlarmreset);
			}break;
			case PlatinumMANAGESAVESUCCESS_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManageSavesuccessfulsettings);
			}break;
			case PlatinumMANAGEPASSWORD_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumManage.PlatinumManagepassword);
			}break;
			//测试
			case PlatinumTESTSWITCH_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestswitch);
			}break;
				case PlatinumTESTANALOGSENSOR_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestAnalogsensor);
			}break;
				case PlatinumTESTDIGITALSENSOR_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestDigitalsensor);
			}break;
				case PlatinumTESTSTATICPRESSURE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumTest.PlatinumTestStaticpressure);
			}break;
			//服务
			case PlatinumSERVICETEMPERATURE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerTemperaturecalibration);
			}break;
			case PlatinumSERVICEHUMIDITY_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerHumiditycalibration);
			}break;
			case PlatinumSERVICEFEED_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeedcalibration);
			}break;
			case PlatinumSERVICEWATER_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWatercalibration);
			}break;
			case PlatinumSERVICESETPLUG_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerSetpluginterface);
			}break;
			case PlatinumSERVICEWATERLINENIPPLERINSE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerWaterlinenipplerinse);
			}break;
			case PlatinumSERVICEENNDER_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerFeeder);
			}break;
			case PlatinumSERVICEAMMONIACALIBRATION_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000Platinumservice.PlatinumSerAmmoniaCalibration);
			}break;
			//安装
			case PlatinumINSTALL_SETUP_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallSetup);
			}break;
			case PlatinumINSTALL_RELAYLAYOUT_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallRelaylayout);
			}break;
			case PlatinumINSTALL_ANALOGSENSOR_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogsensor);
			}break;
			case PlatinumINSTALL__DIGITALSENSOR_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallDigitalsensor);
			}break;
			case PlatinumINSTALL_ANALOGOUTPUT_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallAnalogoutput);
			}break;
			case PlatinumINSTALL_CURTAINSETTING_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCurtainsetting);
			}break;
			case PlatinumINSTALL_TEMPERATUREDE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallTemperaturedefinition);
			}break;
			case PlatinumINSTALL_FANAIRCAPACITY_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallFanaircapacity);
			}break;
			case PlatinumINSTALL_COOPSIZE_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCoopsize);
			}break;
			case PlatinumINSTALL_COMMUNICATION_CMD:
			{
				strcat(data,Ac2000PlatinumParams->Ac2000PlatinumInstall.PlatinumInstallCommunication);
			}break;
		
		}
	}			
}


