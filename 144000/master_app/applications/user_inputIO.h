#ifndef __USER_INPUTIO_H__
#define __USER_INPUTIO_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <board.h>

// 初始化SN74HC165PWR GPIO
extern void hc165ConfigIO(void);

// 读取74HC165的端口状态
extern void hc165ReadData(uint8_t* rData, uint8_t num);
	
#endif
