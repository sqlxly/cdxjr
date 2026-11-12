#ifndef __USER_KINCO_H__
#define __USER_KINCO_H__

#include <drv_common.h>

//eeprom读写相关函数接口
rt_err_t eepromWriteHoldingReg( uint16_t addr, uint16_t offset);
rt_err_t user_page_write(uint32_t Addr, uint8_t *pBuffer, uint16_t NumToRead);
rt_err_t user_page_read(uint32_t Addr, uint8_t *pBuffer, uint16_t NumToRead);

//根据modbus地址设置或读取变量值
void setModbusHoldingRegU16(uint16_t addr, uint16_t v);
uint16_t getModbusHoldingRegU16(uint16_t addr);
void setModbusHoldingRegI16(uint16_t addr, int16_t v);
int16_t getModbusHoldingRegI16(int16_t addr);
void setModbusHoldingRegU32(uint16_t base_id, uint32_t value);
uint32_t getModbusHoldingRegU32(uint16_t base_id);

//modobus线程周期调用函数处理事件
void eepromPageAddressProc(int index, int len);
void button_event_handler(int index, int len);
		
#endif
