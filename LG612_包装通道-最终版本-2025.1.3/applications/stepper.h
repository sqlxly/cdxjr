#ifndef __STEPPER_H_
#define __STEPPER_H_


#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>



#define MOTO_CYCLE_PLUSE       3200         //3200个脉冲为一圈
#define PER_SECOND_TIME_COUNT   1000000000   //1000,000 *1000 =1S 

#define CUP_IS_OPEN 1
#define CUP_IS_CLOSE 2

//步进电机A使能和方向控制引脚
#define MTRA_ENABLE_PIN     GET_PIN(D, 14)
#define MTRA_DIRECT_PIN     GET_PIN(D, 13)

//步进电机B使能和方向控制引脚
#define MTRB_ENABLE_PIN     GET_PIN(C, 8)
#define MTRB_DIRECT_PIN     GET_PIN(C, 7)

//电机编号定义
typedef enum
{
    MTA = 0,
    MTB,
    MTERR,
}motor_typedef; 

typedef struct 
{

	uint8_t ch;          //PWM 通道
	uint8_t en_pin;      //使能管脚
	uint8_t dir_pin;     //反向管脚
	struct rt_device_pwm *device;   //PWM设备

}MOTOR_TYPE;

void clear_moto_step(uint8_t id);
void set_recive_moto(uint8_t id,uint8_t state);
void decode_recive_cup_dat(uint8_t *dat);
int update_moto_speed(uint8_t id,uint8_t dir,uint32_t val);
#endif

