#include "./tools/common.h"
#include "at24cxx.h"

#define HEIGH_CALIBRATION_ADDR  2048

extern rt_err_t user_page_write(rt_uint32_t Addr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead);

extern rt_err_t user_page_read(rt_uint32_t Addr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead);

extern void setModbusHoldingReg(uint16_t addr, uint16_t v);

extern uint16_t getModbusHoldingReg(uint16_t addr);

extern uint16_t getModbusHoldingRegEx(uint16_t addr);

static Weigh_Linear_Info linear_info;

rt_err_t save_weigh_calibration(void)
{
	return user_page_write(HEIGH_CALIBRATION_ADDR, (rt_uint8_t *)&linear_info, sizeof(Weigh_Linear_Info));
}

rt_err_t read_weigh_calibration(void)
{
	return user_page_read(HEIGH_CALIBRATION_ADDR, (rt_uint8_t *)&linear_info, sizeof(Weigh_Linear_Info));
}

Weigh_Linear_Info *get_weigh_calibration(void)
{
	return &linear_info;
}

static Egg_Quality_Info eggs_level[6];
void add_eggs_level_info(Egg_Quality_Info *info)
{
	int ch=0, n= 0, pos;
	
	for(pos=1; pos<6; pos++)
	{
		eggs_level[pos-1] = eggs_level[pos];
	}
	
	rt_memcpy(&eggs_level[5], info, sizeof(Egg_Quality_Info));
	
	
	for(ch=0; ch<6; ch++)
	{
		for(n=0; n<6; n++)
		{
			setModbusHoldingReg(199+ch*6+n, eggs_level[ch].type[n]);		// 裂纹状态页面显示是不是裂纹蛋
			setModbusHoldingReg(241+ch*6+n, eggs_level[ch].level[n]);		// 裂纹状态页面显示
			setModbusHoldingReg(399+ch*6+n, eggs_level[ch].weight[n]);	// 裂纹状态页面显示
		}
	}
}

//热备区保存陶瓷压电传感器当前连续错误次数
Bkup_Sram_Alarm bkup_alarm AT(D3_BKPSRAM_BASE);

void BackupSRAM_Init(void)
{
	RCC->AHB4ENR |= 1 << 28;
	PWR->CR1 |= 1<<8;
	(RCC->D3AMR) |= (RCC_D3AMR_BKPRAMAMEN);
	RCC->AHB4ENR |= 1<<28;
	PWR->CR2 |= 1;
}

void BackupSRAM_config(void)
{
	uint16_t n,ch, threshold;
	BackupSRAM_Init();

	BackupSRAM_clear();   //重新开机时 清除备份区数据 因为实际观察发现开机可能出现缓冲数据异常

	if(bkup_alarm.safeCode != SRAM_SAFE_CODE)
	{
		rt_memset(&bkup_alarm, 0, sizeof(Bkup_Sram_Alarm));
		bkup_alarm.safeCode = SRAM_SAFE_CODE;
		rt_kprintf("The button battery is popup!\n");
	}

	threshold = getModbusHoldingReg(692);
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		for(n=0; n<32; n++){
			if(getModbusHoldingReg(699+32*ch+n)){  //传感器禁用
				setModbusHoldingReg(499+32*ch+n, 2);
			}else{
				if(bkup_alarm.errTimes[ch][n] >= threshold){
					setModbusHoldingReg(499+32*ch+n, 1);
				}else{
					setModbusHoldingReg(499+32*ch+n, 0);
				}
			}
		}
	}
}

void BackupSRAM_clear(void)
{
	uint16_t n,ch;
	
	rt_memset(&bkup_alarm, 0, sizeof(Bkup_Sram_Alarm));
	bkup_alarm.safeCode = SRAM_SAFE_CODE;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		for(n=0; n<32; n++){
			setModbusHoldingReg(499+32*ch+n, 0);
		}
	}
}

uint8_t canConnectedErr[6] = {0};
uint8_t peelOutErr[6] = {0};
uint8_t weighSensorErr[6] = {0};
uint8_t adcSensorErr[6] = {0};
struct AlarmInfo getAlarmInfo(void)
{
	struct AlarmInfo alarm;

	uint8_t ch = 0, n =0, loseCnta = 0, loseCntb = 0, swSensor = 0;
	
	if(getModbusHoldingReg(692) == 99){
		rt_memset(&alarm, 0, sizeof(struct AlarmInfo));
		return alarm;
	}
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		loseCnta = 0;
		loseCntb = 0;
		swSensor = 0;
		alarm.type[ch].errWeigh = weighSensorErr[ch] >= 3 ? 1 : 0;
		alarm.type[ch].errAdc = adcSensorErr[ch] >= 3 ? 1 : 0;
		alarm.type[ch].errPeel = peelOutErr[ch] >= 3 ? 1 : 0;
		alarm.type[ch].errCan = canConnectedErr[ch] >= 5 ? 1 : 0;
		alarm.type[ch].warnCrack = 0;
		for(n=0; n<32; n++){
			if(getModbusHoldingReg(699+32*ch+n)){  		//传感器禁用
//				if(n<16) loseCnta += 1;
//				else loseCntb += 1;
			}else{
				swSensor = 1;
				if(getModbusHoldingRegEx(499+32*ch+n)){ //传感器连续错误次数超限
					alarm.type[ch].warnCrack = 1;
					if(n<16) loseCnta += 1;
					else loseCntb += 1;
				}
			}
		}

		alarm.type[ch].errCrack = ((loseCnta >= 7)||(loseCntb >=7)) ? 1 : 0;

		if(swSensor == 0){
			alarm.type[ch].errCan = 0;
		}
		
		alarm.type[ch].bak = 0;
	}
	
	alarm.encoderErr = 0;
	alarm.proximitySensorErr = 0;
	
	return alarm;
}
