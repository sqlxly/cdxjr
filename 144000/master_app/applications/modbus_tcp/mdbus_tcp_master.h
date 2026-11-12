#ifndef __MDBUS_TCP_MASTER_H__
#define __MDBUS_TCP_MASTER_H__

#include <rtthread.h>

#include "dev_config.h"

//struct MotorETA
//{
//    uint16_t bit0							 :1;  //通电准备就绪
//    uint16_t bit1              :1;	//已通电
//    uint16_t bit2              :1;	//运行被允许
//    uint16_t bit3              :1;	//故障 检测到错误
//    uint16_t bit4              :1;	//已加电
//    uint16_t bit5              :1;	//快速停车
//    uint16_t bit6              :1;	//通电被禁止
//    uint16_t bit7              :1;	//警告被激活
//    uint16_t bit8              :1;	//bak
//    uint16_t bit9              :1;	//远程
//    uint16_t bit10             :1;	//达到目标速度
//    uint16_t bit11             :1;	//内部限值有效
//    uint16_t bit12             :1;	//bak
//    uint16_t bit13             :1;	//bak
//    uint16_t bit14             :1;	//通过STOP键停车
//    uint16_t bit15             :1;	//转动方向
//};

//typedef struct {
//	/* 读取到的电机数据 */
//	struct MotorETA m_ETA;		//主电机状态字
//	int16_t m_RFRD;						//主电机输出转速
//	int16_t m_LCR;						//主电机电流

//	struct MotorETA s_ETA;		//从电机状态字
//	int16_t s_RFRD;						//从电机输出转速
//	int16_t s_LCR;						//从电机电流
//	
//	/* 需要写入电机的参数 */
//	int16_t CMD;						//命令控制字
//	int16_t LFRD;						//电机目标转速 转每分钟
//	int16_t ACC;						//加速斜坡 单位0.1s
//	int16_t DEC;						//减速斜坡 单位0.1s
//} MotorParam;

void set_master_motor_param(enum MainMotorStatus state, int16_t target, uint8_t smooth);

void set_slaver_motor_param(enum MainMotorStatus state, int16_t target, uint8_t smooth);

void reset_ms_motor_stateEx(void); //重置电机扩展状态值  用于电机复位
	
int16_t get_main_motor_LFRD(void);

uint8_t get_main_motor_state(void);

#endif
