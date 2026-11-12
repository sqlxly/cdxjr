
#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__
#include <rtthread.h>

typedef struct 
{
	uint16_t cmd_type;
	uint16_t packeg_ch;	
	uint16_t transfer_motor_dir;          //包装通道正反转
	uint16_t transfer_motor_state;        //包装通道电机启停
	uint16_t transfer_motor_postion;    //包装通道当前位置
	uint16_t egg_cup_moto_dir;            //蛋杯移动方向
	uint16_t egg_cup_moto_state;          //蛋杯电机状态
	uint16_t egg_cup_move_setp;           //蛋杯移动步数   //等分为100步   1%为一步
	uint16_t egg_cup_postion;             //蛋杯当前位置
	uint16_t egg_clamp_state;             //蛋夹电机启停
	uint16_t egg_clamp_switch;            //蛋夹开关      
}packeg_dubug_type;
typedef struct
{
	packeg_dubug_type pdebug;
	
}mb_packge_type;

#endif /* __DATA_TYPE_H__ */
