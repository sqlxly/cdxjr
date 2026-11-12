
#include <rtthread.h>

#include "product_set.h"

#ifndef _MODBUS_PACKGE_H__
#define _MODBUS_PACKGE_H__

#define RUNING_TICK_EGG_CMD  1
#define SET_PACKGE_SERVO_SETP_CMD  2   //设置伺服电机
#define SET_RECIVE_SERVO_SETP_CMD  3   //设置伺服电机
#define DEBUG_RECIVE_CUP_CMD       4
#define DEBUG_TICK_EGG_CMD         5
#define DEBUG_PACKGE_BOX_CMD       6
#define CLOSE_EGG_CUP_CMD          7
#define SEND_PACKGE_INFO_CMD       10
#define SEND_RUN_MODE_CMD          11
#define GET_SERVO_PACKGE_CMD       12
#define CLEAR_SLAVE_SERVO_ALARM    13
#define GET_SERVO_RUN_PART         14
typedef struct
{
	product_packge_info ch1_packge_info;
	product_packge_info ch2_packge_info;
}servo_packge_type;

typedef struct
{
   uint8_t state;
	 uint8_t en_flag;
	 uint8_t cup_eggs;     //低位对于 1 高位对应 6   bit 真为有蛋 0 无蛋
	 uint32_t empty_id;    // 数据要和主机同步	// 这个是每一次包装通道发过来一个状态或者每一次包装通道状态变化了，这个状态就加1了，主机这边通过读取最新的这个值和原来的值判断
																							// 如果一样，说明包装通道那边没有变，就说明包装通道很忙？
	 uint32_t alarm;				// 包装通道有没有报警
	 uint32_t recive_servo_offest_distem;
	 uint32_t packge_servo_offest_distem;
}packge_control_info;


typedef struct
{
   packge_control_info ch1;
	 packge_control_info ch2;
	 uint32_t input_gpio; 
}packge_control_type;

uint32_t get_recive_cup_empty_id(uint8_t ch);
uint8_t check_mb_en_flag(uint8_t device_id);
void clear_mb_servo_packge_info(uint8_t board_id);
uint8_t get_packge_control_run_state(uint8_t ch);
uint32_t get_packge_control_alarm(uint8_t ch);
uint16_t get_packge_ch_err_count(uint8_t ch);
uint32_t get_recive_servo_offest(uint8_t ch);
uint32_t get_packge_servo_offest(uint8_t ch);
uint8_t get_mb_packge_control_en_flag(uint8_t ch);
void get_control_packge_info(product_packge_info *dat,uint8_t ch);
uint8_t get_packge_control_mb_state(uint8_t board);
uint8_t get_packge_recive_cup_eggs(uint8_t ch);
void analysis_packge_control_info(packge_control_type *dat,uint8_t board_id);
uint32_t get_packge_io_state(uint8_t control);

 void set_reconnect_flag(uint8_t device_id,uint8_t flag);

#endif  	// _MODBUS_PACKGE_H__
