#ifndef __USER_OUTPUTIO_H__
#define __USER_OUTPUTIO_H__

#include <rtthread.h>
#include <drv_gpio.h>

// 初始化SN74HC595PWR GPIO_PIN
void hc595ConfigIO(void);

// 同时发送数据到6个SN74HC595PWR芯片
void hc595WriteData(uint8_t* rData, uint8_t nByte);
	
// 继电器控制IO初始化
extern void configRelayIO(void);

// 继电器输出控制 使用位操作7bit
extern void updateRelayIO(int8_t ctrl);

#endif
