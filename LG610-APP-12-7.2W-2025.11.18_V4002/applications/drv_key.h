#ifndef __USER_KEY_H
#define __USER_KEY_H
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <modbus.h>

#define EGG_COUNT_SENSER              (1<<(2-1))   	// 分级线鸡蛋脉冲
#define NUMBER_ONE_CUP_KEY            (1<<(5-1))  	// A通道的1号蛋杯检测
#define EGG_CUP_EMPTY_KEYA            (1<<(6-1))  	// A通道的空杯检测
#define EXPONSION_LINK_BUTTOM_SENSER  (1<<(7-1))    // 伸缩杆0%检测
#define EXPONSION_LINK_TOP_SENSER     (1<<(8-1))		// 伸缩杆到达100%的位置即最大位置？
#define RESET_SERSER                  (1<<(9-1))		// 复位到位检测
#define EGG_CUP_EMPTY_KEYB            (1<<(12-1))  	// B通道的空杯检测

#define CHA_LIGHT_SENSER_ERR         	(1<<(13-1)) 	// 光通道异常传感器A
#define CHB_LIGHT_SENSER_ERR          (1<<(14-1)) 	// 光通道异常传感器B

#define START_KEY                     (1<<(20-1))		// 启停按钮
#define RESET_KEY                     (1<<(19-1))		// 复位按钮
#define EMERGENCY_KEY                 (1<<(18-1))		// 急停按钮
#define PRESS 1


uint32_t ge_all_key(void);
uint8_t ge_key_val(uint32_t key_flag);
uint8_t get_immediately_key_val(uint32_t key_flag);
void ext_key_scan();
#endif  //__USER_KEY_H
