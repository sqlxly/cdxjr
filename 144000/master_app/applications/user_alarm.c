#include "user_alarm.h"
#include "mdbus_tcp_slaver.h"
#include "master402_od.h"
#include "dev_config.h"

static alarm_state_t alarmState;

static uint8_t triger_delay_cnt[eAlarmTypeUndefine] = {0};
	
extern PackageAlarmMaskTypeDef *get_package_alarm_mask_info(void);		//获取报警信息
extern void clear_package_alarm_mask(void);														//点击确认报警后 一键清除所有通道上的报警
	
//主控制线程获取报警后需要执行的动作信息
alarm_state_t get_alarm_action(void)
{
	return alarmState;
}

//警告级别报警解析
void warning_level_alarm_process(AlarmTypeDef type, char *msg)
{
	uint16_t addr = 0;
	
	switch(type)
	{
		case ePackageConnectError:					//包装通道1~20通讯故障
			addr = 6670;
			break;
		case ePackageLayerCache1Error:			//包装通道1~20第1层缓存蛋杯故障
			addr = 6690;
			break;
		case ePackageLayerCache2Error:			//包装通道1~20第2层缓存蛋杯故障
			addr = 6710;
			break;
		case ePackageLayerCache3Error:			//包装通道1~20第3层缓存蛋杯故障
			addr = 6730;
			break;
		case ePackageMoveCacheConnectError:	//包装通道1~20移动缓存伺服通讯故障
			addr = 6750;
			break;
		case ePackageMoveCacheDeviceError:	//包装通道1~20移动缓存伺服设备故障
			addr = 6770;
			break;
		case ePackageMoveCacheCupCloseError://包装通道1~20移动缓存蛋杯开闭故障
			addr = 6790;
			break;
		case ePackageAcceptConnectError:		//包装通道1~20接受抓伺服通讯故障
			addr = 6810;
			break;
		case ePackageAcceptDeviceError:			//包装通道1~20接受抓伺服设备故障
			addr = 6830;
			break;
		case ePackageAcceptClearError:			//包装通道1~20接受抓蛋夹中有鸡蛋未放掉
			addr = 6850;
			break;
		case ePackageConveyConnectError:		//包装通道1~20蛋托输送伺服通讯故障
			addr = 6870;
			break;
		case ePackageConveyDeviceError:			//包装通道1~20蛋托输送伺服设备故障
			addr = 6890;
			break;
		case ePackageAutoDispatchError:			//包装通道1~20自动发托机故障
			addr = 6910;
			break;
		case ePackageTrayMissError:					//包装通道1~20蛋托缺失
			addr = 6930;
			break;
		case ePackageTrayStackedError:			//包装通道1~20蛋托堆叠
			addr = 6950;
			break;
		case ePackageTargetCompleteInfo:		//包装通道1~20完成产量(只提示不停机)
			addr = 6970;
			break;		
		default: break;
	}
	
	if(addr > 0){
		rt_memcpy(&u_tab_registers[addr], msg, strlen(msg));  //把报警内容用字符串填充
		triger_delay_cnt[type]++;
	}else{
		triger_delay_cnt[type] = 0;
	}
	
	if(triger_delay_cnt[type] >= 15){
		_tab_bits[type] = 1;								//先填充报警内容 再触发报警采样  避免触发报警后采样值为空
	}
}

//错误级别报警解析
void error_level_alarm_process(AlarmTypeDef type)
{
	_tab_bits[type] = 1;
}

void trigger_alarm(alarm_level_t level, AlarmTypeDef type, char *msg)
{
		if(type >= eAlarmTypeUndefine) return; 

		switch(level)
		{
			case ALARM_LEVEL_NULL:			//无报警
				break;
			case ALARM_LEVEL_INFO:			//提示级别
				if(alarmState.action < ALARM_ACTION_NOTIFY) alarmState.action = ALARM_ACTION_NOTIFY;
				break;
			case ALARM_LEVEL_WARNING:		//警告级别
				warning_level_alarm_process(type, msg);
				if(alarmState.action < ALARM_ACTION_SHUTDOWN){
					alarmState.action = ALARM_ACTION_SHUTDOWN;
					alarmState.isAlarmFlag = RT_TRUE;
				}
				break;
			case ALARM_LEVEL_ERROR:			//错误级别
				error_level_alarm_process(type);
				if(alarmState.action < ALARM_ACTION_SHUTDOWN){
					alarmState.action = ALARM_ACTION_SHUTDOWN;
					alarmState.isAlarmFlag = RT_TRUE;
				}
				break;
			case ALARM_LEVEL_CRITICAL:	//致命级别
				break;
		}
}

void show_alarm_information(uint32_t value, AlarmTypeDef type)
{
	char buf[41]="", tbuf[5];
	uint8_t n = 0, index = 0, len;
	
//	rt_memset(buf, '\0', 21);
	for(n=0; n<sorterUsedNumber; n++){
		if(getbitv(value, n)){
			sprintf(tbuf, "%d,", n+1);
			strcat(buf, tbuf);
			index++;
			if(index >= 5) break;
		}
	}

	if(index > 0){
		len = strlen(buf) - 1;
		buf[len] = '\0';
//		rt_kprintf("buf=%s\n",buf);
		trigger_alarm(ALARM_LEVEL_WARNING, type, buf);
	}
}

//光检排列通道检测
static uint8_t array_detect_counter = 0;
uint8_t array_line_detect_check(void)
{
	uint8_t status = 0, ret = 0;

	status = get_input_io_sta(masterOperate.inputData, eInputIO3);

	if(0 == status){
		array_detect_counter++;
		if(array_detect_counter >= 10){
			array_detect_counter = 10;
			ret = 1;
		}
	}else{
		array_detect_counter = 0;
	}
	
	return ret;
}

//报警触发和报警确认处理函数   声光报警 45-黄+beep 46-绿 47-红
void system_alarm_process(void)
{
	alarm_state_t state;
	PackageAlarmMaskTypeDef *alarmMask_t;
	
	if(RT_TRUE == alarmConfirmBtn){
		clear_package_alarm_mask();
		for(int i=0; i<eAlarmTypeUndefine; i++){
			_tab_bits[i] = 0;
		}
		malpositionDetectFlag = 0;
		cupDetectFlag = 0;
		arrayDetectFlag = 0;
		alarmState.action = ALARM_ACTION_NONE;
		alarmState.isAlarmFlag = RT_FALSE;
		rt_thread_mdelay(50);   //这里做延时 让ecat线程有足够的时间将报警清除信号发送给每个包装通道
		alarmConfirmBtn = RT_FALSE;
	}

	if(1 == getbitv(statusword_6041, 3)){  //电动推缸伺服设备故障
		trigger_alarm(ALARM_LEVEL_ERROR, eCylinderDeviceError, "NULL");
	}

	if(cupDetectFlag > 0){								//主轨道蛋杯闭合异常
		trigger_alarm(ALARM_LEVEL_ERROR, eObritCupCloseError, "NULL");
	}

	if(malpositionDetectFlag > 0) {				//主轨道蛋杯鸡蛋位置异常
//		trigger_alarm(ALARM_LEVEL_ERROR, eObritEggsPostionError, "NULL");
	}

	arrayDetectFlag = array_line_detect_check();
	if(arrayDetectFlag > 0){							//光检排列通道异常检测
		trigger_alarm(ALARM_LEVEL_ERROR, eArraysDeviceError, "NULL");
	}

	alarmMask_t = get_package_alarm_mask_info();

	show_alarm_information(alarmMask_t->cache1ErrorMask, ePackageLayerCache1Error);									//第1层缓存蛋杯故障
	show_alarm_information(alarmMask_t->cache2ErrorMask, ePackageLayerCache2Error);									//第2层缓存蛋杯故障
	show_alarm_information(alarmMask_t->cache3ErrorMask, ePackageLayerCache3Error);									//第3层缓存蛋杯故障
	show_alarm_information(alarmMask_t->moveCacheConnectErrorMask, ePackageMoveCacheConnectError);	//移动缓存伺服通讯故障
	show_alarm_information(alarmMask_t->moveCacheDeviceErrorMask, ePackageMoveCacheDeviceError);		//移动缓存伺服设备故障
	show_alarm_information(alarmMask_t->moveCacheCupErrorMask, ePackageMoveCacheCupCloseError);			//移动缓存蛋杯开闭故障
	show_alarm_information(alarmMask_t->acceptConnectErrorMask, ePackageAcceptConnectError);				//接受抓伺服通讯故障
	show_alarm_information(alarmMask_t->acceptDeviceErrorMask, ePackageAcceptDeviceError);					//接受抓伺服设备故障
	show_alarm_information(alarmMask_t->acceptClearErrorMask, ePackageAcceptClearError);						//接受抓蛋夹中有鸡蛋未放掉
	show_alarm_information(alarmMask_t->conveyConnectErrorMask, ePackageConveyConnectError);				//蛋托输送伺服通讯故障
	show_alarm_information(alarmMask_t->conveyDeviceErrorMask, ePackageConveyDeviceError);					//蛋托输送伺服设备故障
	show_alarm_information(alarmMask_t->autoDispatchErrorMask, ePackageAutoDispatchError);					//自动发托机故障
	show_alarm_information(alarmMask_t->trayStackedErrorMask, ePackageTrayMissError);								//蛋托缺失
	show_alarm_information(alarmMask_t->targetCompleteInfoMask, ePackageTrayStackedError);					//蛋托堆叠

	state = get_alarm_action();
	if(state.isAlarmFlag == RT_TRUE){			//控制报警指示灯闪烁
		u_tab_registers[1] = 1;
	}else{
		u_tab_registers[1] = 0;
	}
}
