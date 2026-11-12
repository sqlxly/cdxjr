#ifndef __IODRIVER_H__
#define __IODRIVER_H__

#include <rtthread.h>
#include <drv_gpio.h>

// 继电器控制IO初始化
extern void configRelayIO(void);

// 继电器输出控制 使用位操作7bit
extern void updateRelayIO(uint8_t ctrl);
	
// 电磁铁控制IO初始化
extern void configDctIO(void);

// 电磁铁输出控制 使用位操作24bit
extern void updateDctIO(uint32_t ctrl);

// 气缸开闭控制IO初始化
extern void configDcfIO(void);

// 气缸开闭输出控制 使用位操作20bit
extern void updateDcfIO(uint32_t ctrl);
	
#endif
