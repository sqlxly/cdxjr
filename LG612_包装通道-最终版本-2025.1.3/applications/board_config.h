
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


#ifndef __BOARD_CONFIG__H
#define __BOARD_CONFIG__H

#define NOT_FORWARD 0
#define FORWARD 1
#define SERVO_NOT_FORWORD  NOT_FORWARD
#define SERVO_FORWORD  FORWARD

#define CH1_RECIVE_DIR SERVO_FORWORD  
#define CH2_RECIVE_DIR SERVO_FORWORD  

#define CH1_PACKGE_DIR SERVO_FORWORD  
#define CH2_PACKGE_DIR SERVO_NOT_FORWORD  

#define CH1_CUP_DIR FORWARD

#define CH2_CUP_DIR FORWARD

#define IS_CLOSE 0
#define IS_OPEN 1

// 伺服这些引脚为什么具有这样的功能，是因为要设置相应引脚的功能，相应引脚的功能对应不同的参数，只要设置对应的参数就设置了对应引脚的功能了
// DO1+,DO1-(P2.018)
// DO2+,DO2-(P2.019)
// DO3+,DO3-(P2.020)
// DO4+,DO4-(P2.021)
// DO5+,DO5-(P2.022)
// DO6+,DO6-(P2.041)
#define CH1_CHECK_TRAY_PIN (1-1)						// 蛋托检测引脚						
#define CH1_PACKGE_RESET_ORIGIN_PIN (2-1)
#define CH1_EGG_CUP_OPEN_SENSER_PIN (3-1)		// 蛋夹关闭到位检测引脚
#define CH1_RECIVE_RESET_ORIGIN_PIN (4-1)		// 蛋夹上升到位(接收上升到位)检测引脚
#define CH1_PILE_UP_PIN             (5-1)		// 斜坡堆积检测引脚
#define CH1_RECIVE_SERVO_CMP_PIN    (6-1)		// --接收伺服高速比较检测引脚
#define CH1_RESET_ORIGIN_PIN        (7-1)		// 复位检测引脚
#define CH1_RUN_NEX_STEP_PIN        (8-1)		// 单步执行检测引脚
#define CH1_PACKGE_SERVO_CMP_PIN    (9-1)		// --包装伺服高速比较检测引脚																		伺服驱动器比较完成后，输出高电平的引脚
#define CH1_PACHGE_SERVO_STATE_PIN  (10-1)	// --包装伺服回到原点检测引脚																		包装伺服驱动器状态引脚，输出高电平的引脚
#define CH1_RECIVE_SERVO_STATE_PIN  (11-1)	// --接收伺服回到原点检测引脚																		接收伺服驱动器状态引脚，输出高电平的引脚
#define CH1_SERVO_ALARM_PIN         (12-1)	// ----包装和接收伺服报警检测引脚		经过台达的手册以及读出来的参数发现，设置的是准备好信号的输出，当没有准备好则读取0X0002位置的报警信息
																						// 该位置是警报代码位置

#define CH2_CHECK_TRAY_PIN          (13-1)
#define CH2_PACKGE_RESET_ORIGIN_PIN (14-1) 
#define CH2_EGG_CUP_OPEN_SENSER_PIN (15-1)
#define CH2_RECIVE_RESET_ORIGIN_PIN (16-1)
#define CH2_PILE_UP_PIN             (17-1)
#define CH2_RECIVE_SERVO_CMP_PIN    (18-1)
#define CH2_RESET_ORIGIN_PIN        (19-1)
#define CH2_RUN_NEX_STEP_PIN        (20-1)
#define CH2_PACKGE_SERVO_CMP_PIN    (21-1)	// 伺服驱动器比较完成后，输出高电平的引脚
#define CH2_PACHGE_SERVO_STATE_PIN  (22-1)	// 包装伺服驱动器状态引脚，输出高电平的引脚
#define CH2_RECIVE_SERVO_STATE_PIN  (23-1)	// 接收伺服驱动器状态引脚，输出高电平的引脚
#define CH2_SERVO_ALARM_PIN         (24-1)



/*
#define OUT12 GET_PIN(B, 1)
#define OUT11 GET_PIN(B, 2)
#define OUT10 GET_PIN(E, 7)
#define OUT9 GET_PIN(E, 8)
#define OUT8 GET_PIN(E, 9)
#define OUT7 GET_PIN(E, 10)

#define OUT6 GET_PIN(D, 7)
#define OUT5 GET_PIN(D, 6)
#define OUT4 GET_PIN(B, 14)
#define OUT3 GET_PIN(B, 15)
#define OUT2 GET_PIN(D, 8)
#define OUT1 GET_PIN(D, 10)
*/
#define OUT1 GET_PIN(B, 1)
#define OUT2 GET_PIN(B, 2)
#define OUT3 GET_PIN(E, 7)
#define OUT4 GET_PIN(E, 8)
#define OUT5 GET_PIN(E, 9)
#define OUT6 GET_PIN(E, 10)
#define OUT7 GET_PIN(D, 7)
#define OUT8 GET_PIN(D, 6)
#define OUT9 GET_PIN(B, 14)
#define OUT10 GET_PIN(B, 15)
#define OUT11 GET_PIN(D, 8)
#define OUT12 GET_PIN(D, 10)


#endif  //__BOARD_CONFIG__H
