#include <rtthread.h>
#ifndef __ANLYSIS_CMD_H
#define __ANLYSIS_CMD_H


typedef struct 
{
	uint8_t addr;     //包装通道设备地址
	uint8_t cmd;      //控制命令
	uint16_t val;     //命令值
}debug_dat_type;






void anlysis_modbus_read_input_reg(uint16_t addr,uint16_t *input_reg);
void anlysis_modbus_write_dat(uint16_t addr,uint16_t *hot_reg);

#endif /* __ANLYSIS_CMD_H */


