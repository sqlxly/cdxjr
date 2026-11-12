#include "user_kinco.h"
#include "at24cxx.h"
#include "mdbus_tcp_slaver.h"
#include "production_param.h"
#include "dev_config.h"
#include "user_encoder.h"

extern void reset_diff_pos(void);
static at24cxx_device_t at_dev = RT_NULL;

uint16_t *u_RegHoldingPtr;
uint16_t *e_RegHoldingPtr;

rt_err_t eepromWriteHoldingReg( uint16_t addr, uint16_t offset)
{
	return at24cxx_write( at_dev, (addr-offset)*2, (uint8_t *)&u_RegHoldingPtr[addr], 2);
}

rt_err_t user_page_write(uint32_t Addr, uint8_t *pBuffer, uint16_t NumToRead)
{
	rt_err_t err;
	err = at24cxx_page_write(at_dev, Addr, pBuffer, NumToRead);
	return err;
}

rt_err_t user_page_read(uint32_t Addr, uint8_t *pBuffer, uint16_t NumToRead)
{
	rt_err_t err;
	err = at24cxx_page_read(at_dev, Addr, pBuffer, NumToRead);
	return err;
}

void setModbusHoldingRegU16(uint16_t addr, uint16_t v)
{
	*(u_RegHoldingPtr + addr) = v;
}

uint16_t getModbusHoldingRegU16(uint16_t addr)
{
	return *(u_RegHoldingPtr + addr);
}

void setModbusHoldingRegI16(uint16_t addr, int16_t v)
{
	*(u_RegHoldingPtr + addr) = v;
}

int16_t getModbusHoldingRegI16(int16_t addr)
{
	return *(u_RegHoldingPtr + addr);
}

void setModbusHoldingRegU32(uint16_t base_id, uint32_t value)
{
	*(u_RegHoldingPtr + base_id) = value&0xffff; 
	*(u_RegHoldingPtr + base_id + 1) = (value>>16)&0xffff;
}

uint32_t getModbusHoldingRegU32(uint16_t base_id)
{
	uint32_t temp;

	temp = *(u_RegHoldingPtr + base_id + 1);
	temp <<= 16;
	temp |= *(u_RegHoldingPtr + base_id);
	
	return temp;
}

//用于用户的一些点击事件的处理
void button_event_handler(int index, int len)
{
	if(index == 5500){												//点击确认报警按钮
		alarmConfirmBtn = RT_TRUE;
	}
}

void eepromPageAddressProc(int index, int len)
{
	uint16_t temp = 0;
	
	/* 生产管理--分级生产 */
	if((index>=8000)&&(index<=8009)){					//页面64 生产管理--分级管理--重量分级
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8050)&&(index<=8057)){		//页面65 生产管理--分级管理--颜色分级
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8100)&&(index<=8108)){		//页面66 生产管理--分级管理--脏蛋分级
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8150)&&(index<=8161)){		//页面67 生产管理--分级管理--裂纹分级
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8200)&&(index<=8201)){		//页面68 生产管理--分级管理--异常蛋分级
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8250)&&(index<=8251)){		//页面69 生产管理--分级管理--抛弃通道设置
		eepromWriteHoldingReg(index, 8000);
	}

	/* 设备管理--设备安装 */
	if((index>=8300)&&(index<=8338)){					//页面101 设备管理--设备安装--硬件配置
		eepromWriteHoldingReg(index, 8000);
	}else if((index>=8350)&&(index<=8365)){		//页面102 设备管理--设备安装--设备运行参数
		eepromWriteHoldingReg(index, 8000);
	}

	/* 系统管理--系统时间设置 */
	if((index>=6590)&&(index<=6592)){   			//页面134 系统管理--系统时间设置
		set_date(u_RegHoldingPtr[6590], u_RegHoldingPtr[6591], u_RegHoldingPtr[6592]);
	}else if((index>=6593)&&(index<=6595)){
		set_time(u_RegHoldingPtr[6593], u_RegHoldingPtr[6594], u_RegHoldingPtr[6595]);	
	}
	
	/* 生产管理--生产分级--包装管理 编辑 */
	if(index==4733){													//页面61 生产管理--生产分级--包装管理 点击回原点
		u_RegHoldingPtr[4732] = 1;
	}else if(index==4734){										//页面61 生产管理--生产分级--包装管理 点击下一步
		temp = u_RegHoldingPtr[4732];
		u_RegHoldingPtr[4732] = (temp+1)%9;
	}
	
//	else if(1442 == index){									//页面59 设备管理--安装调试--包装线调试 电磁铁动作提前
//		eepromWriteHoldingReg(index);
//	}
}

int reload_param_from_eeprom(void)
{
	rt_err_t err;

	u_RegHoldingPtr = u_tab_registers;

	e_RegHoldingPtr = u_tab_registers;
	
	at_dev = at24cxx_init("i2c1", 0);

	if(at_dev)
	{
		at24cxx_page_read(at_dev, 0, (uint8_t *)&u_RegHoldingPtr[8000], 2000);		//从eeprom地址0开始读取1000个寄存器地址  地址偏移10000后再存入modbus

		sorterUsedNumber = u_RegHoldingPtr[8318];
		
		return err;
	}

	return -RT_ERROR;
}

INIT_ENV_EXPORT(reload_param_from_eeprom);
