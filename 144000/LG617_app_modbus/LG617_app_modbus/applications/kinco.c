#include <rtthread.h>
#include "at24cxx.h"
#include "user_mb_app.h"
#include "./tools/common.h"

#define eeprom_code 0x12345678

static at24cxx_device_t at_dev = RT_NULL;

USHORT *pRegHoldingBuf, *pERegHoldingBuf;

USHORT eRegHoldingBuf[S_REG_HOLDING_NREGS];

uint8_t calib_flag = 0; 			        //进行过称重校准

extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];

extern uint16_t get_adc_average_value(uint8_t ch);

extern void clear_current_weighing(uint8_t ch, uint32_t ad);

extern void deal_bkupdate_process(rt_uint16_t	*pReg);

extern void fdcan_submit_work(rt_ubase_t cmd);

void eepromWriteHoldingReg( rt_uint32_t addr )
{
	rt_err_t err;
	rt_base_t level;
	
	err = at24cxx_write( at_dev, addr*2, (uint8_t *)&pRegHoldingBuf[addr], 2);

	level = rt_hw_interrupt_disable();
	if(err == RT_EOK)
		pERegHoldingBuf[addr] = pRegHoldingBuf[addr];
	else
		pRegHoldingBuf[addr] = pERegHoldingBuf[addr];
	rt_hw_interrupt_enable(level);
}

rt_err_t user_page_write(rt_uint32_t Addr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead)
{
	rt_err_t err;
	err = at24cxx_page_write(at_dev, Addr, pBuffer, NumToRead);
	return err;
}

rt_err_t user_page_read(rt_uint32_t Addr, rt_uint8_t *pBuffer, rt_uint16_t NumToRead)
{
	rt_err_t err;
	err = at24cxx_page_read(at_dev, Addr, pBuffer, NumToRead);
	return err;
}

void setModbusHoldingReg(uint16_t addr, uint16_t v)
{
//	rt_base_t level;
//	level = rt_hw_interrupt_disable();
	*(pRegHoldingBuf + addr) = v;
//	rt_hw_interrupt_enable(level);
}

void setModbusHoldingRegU32(rt_uint16_t base_id, rt_uint32_t value)
{
//	rt_base_t level;
	
//	level = rt_hw_interrupt_disable();
	*(pRegHoldingBuf + base_id) = value&0xffff; 
	*(pRegHoldingBuf + base_id + 1) = (value>>16)&0xffff;
//	rt_hw_interrupt_enable(level);
}

uint16_t getModbusHoldingReg(uint16_t addr)
{
	return *(pERegHoldingBuf + addr);
}

uint16_t getModbusHoldingRegEx(uint16_t addr)
{
	return *(pRegHoldingBuf + addr);
}

void kinco_set_weghing_param( rt_uint16_t addr )
{
	uint8_t ch = 0, n = 0;
	uint16_t ad = 0, weights = 0;
	Weigh_Linear_Info *info;
	
	/* 称重校准页面参数 */
	if((addr>=99) && (addr<=129)){
		info = get_weigh_calibration();
		switch( addr )
		{
			case 99:  //称重传感器选择0~5
				eepromWriteHoldingReg(addr);
				break;
			case 100: //称重校准时 传感器清零
				ch = pRegHoldingBuf[99];
				info->ZeroAD[ch] = get_adc_average_value( ch );
				clear_current_weighing(ch, info->ZeroAD[ch]);
				break;
			case 102: //称重校准时 输入砝码重量
				ch = pRegHoldingBuf[99];
				ad = get_adc_average_value( ch );
				weights = pRegHoldingBuf[addr];
				pRegHoldingBuf[addr] = 0;
				info->linearK[ch] = (double)weights / 10.0f / ((double)ad - (double)info->ZeroAD[ch]);
				weights = pERegHoldingBuf[127];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->autoPeelAD[n] = weights / info->linearK[n] / 10.0f;
				}
				
				weights = pERegHoldingBuf[128];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->addPeelAlarmAD[n] = weights / info->linearK[n] / 10.0f;
				}

				weights = pERegHoldingBuf[129];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->weighThreshold[n] = weights / info->linearK[n] / 10.0f;
				}
				
				pRegHoldingBuf[115+ch] = info->linearK[ch]*1000000;
				
				setbitv(calib_flag, ch);
				
				break;
			case 127: //输入自动去皮值
				eepromWriteHoldingReg(addr);
				weights = pERegHoldingBuf[addr];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->autoPeelAD[n] = weights / info->linearK[n] / 10.0f;
				}
				break;
			case 128: //输入累计去皮报警值
				eepromWriteHoldingReg(addr);
				weights = pERegHoldingBuf[addr];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->addPeelAlarmAD[n] = weights / info->linearK[n] / 10.0f;
				}
				break;
			case 129: //输入鸡蛋基准重量值
				eepromWriteHoldingReg(addr);
				weights = pERegHoldingBuf[addr];
				for(n=0; n<WEIGH_CHANNEL; n++){
					info->weighThreshold[n] = weights / info->linearK[n] / 10.0f;
				}
				break;
			
			default:break;
		}
		
		if(addr != 99)
		{
			save_weigh_calibration();
		}
	}
}

void kinco_set_crack_param( rt_uint16_t addr )
{
	uint16_t temp1, temp2;
	
	if((addr>=691) && (addr<=890)){
		if(addr == 691){
			BackupSRAM_clear();
		}else if(addr == 692){
			eepromWriteHoldingReg(addr);
		}else if((addr>=699) && (addr<=890)){
			eepromWriteHoldingReg(addr);
			if(pRegHoldingBuf[addr] == 1){
				setModbusHoldingReg(499+addr-699, 2);
			}else{
				setModbusHoldingReg(499+addr-699, 0);
			}
		}
	}else if((addr>=299) && (addr<=304)){
		if((addr==299)||(addr==300)){  //输入重量范围时
			temp1 = pRegHoldingBuf[299];
			temp2 = pRegHoldingBuf[300];
			if(temp1 <= temp2){
				eepromWriteHoldingReg(addr);
			}else{
				pRegHoldingBuf[addr] = pERegHoldingBuf[addr];
			}
		}else if((addr==301)||(addr==302)){ //输入裂纹等级范围时
			temp1 = pRegHoldingBuf[301];
			temp2 = pRegHoldingBuf[302];
			if(temp1 <= temp2){
				eepromWriteHoldingReg(addr);
			}else{
				pRegHoldingBuf[addr] = pERegHoldingBuf[addr];
			}
		}else{
			eepromWriteHoldingReg(addr);
		}
	}
}

void kinco_set_installation_layout_param(rt_uint16_t addr)
{
	if(addr == 995){
		if(pRegHoldingBuf[addr] == 1){												//升级应用程序
//			deal_bkupdate_process(&pRegHoldingBuf[999]);
		}
	}else if(addr == 998){																	//确定升级
		if(pRegHoldingBuf[997] > 0){
			eepromWriteHoldingReg(997);
			rt_thread_mdelay(1000);
			__set_FAULTMASK(1); 
			NVIC_SystemReset();
		}
	}
}

void kinco_set_system_work_param(rt_uint16_t addr)
{
	int i = 0;
	if((addr >= 1500)&&(addr <= 1506)){
		eepromWriteHoldingReg(addr);
	}else if(addr == 1599){   							//通道选择按钮
		pRegHoldingBuf[1602] = 0;
		if(pRegHoldingBuf[1599] == 100){ 			//写入值100时表示退出校准界面
			if(pRegHoldingBuf[1601] == 4){
				pRegHoldingBuf[1601] = 5;
				fdcan_submit_work(8005);					//发送结束校准命令
			}else{
				pRegHoldingBuf[1601] = 0;
			}
		}else{
			pRegHoldingBuf[1601] = 6;
			for(i=0; i<32; i++){
				pRegHoldingBuf[1200+i] = 0;
				pRegHoldingBuf[1250+i] = 0;
				pRegHoldingBuf[1300+i] = 0;
				pRegHoldingBuf[1350+i] = 0;
				pRegHoldingBuf[1400+i] = 0;
			}
		}
	}else if(addr == 1600){									//保存传感器校准参数
		pRegHoldingBuf[1601] = 1;
		pRegHoldingBuf[1602] = 0;
		fdcan_submit_work(8005);							//发送保存数据命令 并显示正在保存状态
	}else if(addr == 1602){									//开始和结束校准按钮
		if(pRegHoldingBuf[addr] == 1){
			pRegHoldingBuf[1601] = 4;
			fdcan_submit_work(8005);						//发送开始校准命令
		}else{
			pRegHoldingBuf[1601] = 5;
			fdcan_submit_work(8005);						//发送结束校准命令
		}
	}
}

CrackParam getCrackSensitivityParam(void)
{
	CrackParam p;
	
	p.sensitivity_1 = (pRegHoldingBuf[1500]&0xff);
	p.sensitivity_2 = (pRegHoldingBuf[1501]&0xff);
	p.sensitivity_3 = (pRegHoldingBuf[1502]&0xff);
	p.addCnt = pRegHoldingBuf[1506];
	
	return p;
}

void updateSensorStatus(int index)
{
	int flag = 0;
	
	if(pRegHoldingBuf[1250+index] < pRegHoldingBuf[1503]) flag += 1;
	if(pRegHoldingBuf[1300+index] < pRegHoldingBuf[1504]) flag += 1;
	if(pRegHoldingBuf[1350+index] < pRegHoldingBuf[1505]) flag += 1;
	if(flag > 0) pRegHoldingBuf[1200+index] = 1;
	else pRegHoldingBuf[1200+index] = 0;
}

int rt_hw_eeprom_init(void)
{
	rt_err_t err;
	uint32_t safe_code = 0;
	
	Weigh_Linear_Info *info = get_weigh_calibration();
	
	pRegHoldingBuf = usSRegHoldBuf;
	
	pERegHoldingBuf = eRegHoldingBuf;
	
	at_dev = at24cxx_init("i2c1", 0);

	if(at_dev){
		at24cxx_page_read( at_dev, 0, (uint8_t *)&safe_code, sizeof(uint32_t) );
		rt_memset(pERegHoldingBuf, 0, 1024*2);
		if(safe_code != eeprom_code){
			at24cxx_page_write( at_dev, 0,    (uint8_t *)pERegHoldingBuf, 1024*2 );
			at24cxx_page_write( at_dev, 2048, (uint8_t *)pERegHoldingBuf, 1024*2 );
			safe_code = eeprom_code;
			at24cxx_page_write( at_dev, 0, (uint8_t *)&safe_code, sizeof(uint32_t));
		}
		
		err = at24cxx_page_read( at_dev, 0, (uint8_t *)pERegHoldingBuf, 1024*4);
		if(err == RT_EOK) rt_memcpy(pRegHoldingBuf, pERegHoldingBuf, 1024*4);
		
		read_weigh_calibration();

		for(int n=0; n<WEIGH_CHANNEL; n++)
		{
			pRegHoldingBuf[115+n] = info->linearK[n]*1000000;
		}
		
		pRegHoldingBuf[996] = APP_VERSION;				//版本信息

		return err;
	}

	return -RT_ERROR;
}

INIT_ENV_EXPORT(rt_hw_eeprom_init);
