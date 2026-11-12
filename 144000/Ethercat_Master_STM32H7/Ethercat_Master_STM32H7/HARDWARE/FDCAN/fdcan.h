#ifndef _FDCAN_H
#define _FDCAN_H

#include "main.h"
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32H7开发板
//FDCAN驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2018/6/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//FDCAN1接收RX0中断使能
#define FDCAN1_RX0_INT_ENABLE	0		//0,不使能;1,使能.

unsigned char FDCAN1_Mode_Init(uint16_t presc,unsigned char ntsjw,uint16_t ntsg1,unsigned char ntsg2,uint32_t mode);
unsigned char FDCAN1_Send_Msg(unsigned char* msg,uint32_t len);
unsigned char FDCAN1_Receive_Msg(unsigned char *buf);
#endif
