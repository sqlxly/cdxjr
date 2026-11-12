#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "agile_modbus_user.h"

#ifndef __SERVO_COMMON_H
#define __SERVO_COMMON_H


#define CH1_DEVICE_ID 0
#define CH2_DEVICE_ID 1

#define TURN_UP  1
#define TURN_DOWM      2
#define BACK           2
#define MOTO_STOP      0
#define MOTO_START     1
#define DEBUG_MODE 0
#define RUNING_MODE 1

#define SERVO_IDLE 0
#define SERVO_HOPE_RUN_PATH 1
#define SERVO_RUNING_PATH  2
#define SERVO_HOPE_RESET_ORIGIN 3
#define SERVO_RESET_ORIGIN 4
#define SERVO_STOP 5


#define SYS_IS_IDLE 0         //空闲
#define SYS_IS_CHECK_ENV 1    //检测运行环境
#define SYS_IS_RUNING 2       //正在运行
#define SYSTEM_IS_RESET   4
#define SYSTEM_IS_STOP    5
#define SYS_IS_CRC_PART   6   //系统校验参数
#define SYSTEM_IS_READY   7   //系统准备好了
#define SYSTEM_IS_EMERGENCY_STOP 8
#define SYSTEM_IS_ENTER_IDLE 10


#define SERVO_MIN_RUN_TIME  100  //每个步骤最小运行时间
#define PACKGE_BOX_LEN      355 //mm 毫米  包装通道每个格子长度
#define RECIVE_CLAW_LEN      100 //mm 毫米  接收爪移动最大长度
#define PACKGE_SERVO_CYCLE_SETP  20000 //伺服电机走多少个步为一个盒子长度
#define RECIVE_SERVO_CYCLE_SETP  3200   // 最大步数 下降高度 3200个脉冲  最大5000 与传感器安装的位置会影响这个值

#define DEBUG_PACKGE_BOX_STEP_PATH 58
#define RESET_ORIGIN_PATH  59
#define JUMP_TO_NEX_BOX_BASE_PATH 20
//#define JUMP_TO_NEX_BOX_PATH  60
#define TRUN_UP_PATH   62  //-10000
#define TRUN_DOWN_PATH   63  //10000

#define CH1_MOTO_EGG_CUP_ID 0
#define CH2_MOTO_EGG_CUP_ID 1

typedef enum
{
	CUP_SENSER_PIN =0,
	RECIVE_LINE_ORIGIN_PIN,
	CHECK_TRAY_SENSER,
	PACKGE_LINE_ORIGIN_PIN,
	EMPTY,
}GPIO_ID;


typedef struct
{
	uint16_t name[10];                         //包装盒名称
	uint16_t row;                              //包装盒行数
	uint16_t line;                             //包装盒列数    
	uint16_t auto_transmit_egg_carton_positon; //自动派托位置
	uint16_t check_egg_tray_position;        //蛋托检测位置
	uint16_t packge_line1_postion;          //首排落蛋位置
	uint16_t packge_line2_postion;
	uint16_t packge_line3_postion;
	uint16_t packge_line4_postion;
	uint16_t packge_line5_postion;
	uint16_t recive_claw_fall_dowm_position;   //接收爪下降位置(其实是接收伺服下降的距离)
	uint16_t recive_claw_open_position;        //接收爪打开位置(其实是接收伺服下降的时候离原点的最大距离)
	uint16_t recive_claw_close_position;       //接收爪关闭位置(其实是接收伺服上升的时候离原点最小的距离)
	uint16_t transport_start_position;         //传输线启动位置
	uint16_t use_tray_count;                  //占用托盘数量
	uint32_t crc_32;
}product_packge_info;


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
	 uint32_t empty_id;    // 数据要和主机同步
	 uint32_t alarm;
	 uint32_t recive_servo_offest_distem;
	 uint32_t packge_servo_offest_distem;
}packge_control_info;

typedef struct
{
   packge_control_info ch1;
	 packge_control_info ch2;
	 uint32_t input_gpio;
}packge_control_type;


typedef struct 
{
	uint8_t id;             //id
	uint8_t state;          //状态
	uint8_t busy;           //忙碌
	uint8_t reset_flag;      //复位标记
	uint8_t cup_open_flag;  //蛋杯打开标记
	uint8_t cup_close_flag;  //蛋杯关闭标记
	uint8_t packge_move_flag; //包装通道移动标记
	uint8_t current_step;   //当前步数
	uint8_t cycle_step;     //一个周期有几步
	uint8_t state_pin;      //状态引脚
	uint8_t up_origin_pin;
	uint8_t origin_pin;     //原点复位脚
	uint8_t hope_run_path;  //期望运行路径
  int32_t move_distem;    //距离原点 0 的值
	rt_tick_t start_time;   //开始执行路径时间
	rt_tick_t run_time;     //执行路径时间
	rt_tick_t setp_delay_time;
}servo_type;



typedef struct ch_device
{
	uint8_t en_flag;
	uint8_t device_id;
	uint8_t status;         //状态  空闲的时候运行调试模式
	uint8_t recive_egg;     //落蛋标记
	uint8_t cup_id;         //蛋杯电机ID 
	uint8_t cup_senser_pin; 
	uint8_t tray_senser_pin;
	uint8_t cup_step_count;
	uint8_t transport_count;
	uint8_t cup_en;         //蛋夹开关启用标记
	uint8_t packge_move_en; //输送通道启动标记
	uint8_t servo_alarm_pin;
	uint8_t tray_flag;       //托盘标记
	uint8_t origin_flag;       //原点标记
	uint8_t change_packge_flag; //包装改变标记
	uint8_t reset_check_tray_flag;  //重新设置检查托盘位置标记
	uint8_t reset_cup_open_position_flag;  //重新设置检查托盘位置标记
	uint32_t recive_cup_empty_id;
  servo_type packge_servo;// 包装伺服电机
  servo_type recive_servo;//  接收抓伺服电机
	product_packge_info product_packge;  //产品包装信息

}ch_device_type;
void set_servo_run_count(uint8_t dat);
rt_err_t set_open_cup_part(ch_device_type *device);
rt_err_t set_check_tray_part(ch_device_type *device);
rt_tick_t count_use_time(rt_tick_t start_time);
rt_err_t servo_write_reg(servo_device_id device,int addr,uint16_t value);
rt_err_t servo_write_mutly_regs(servo_device_id id,uint16_t addr, uint16_t *dat,uint16_t len);
rt_err_t servo_read_mutly_regs(servo_device_id id,uint16_t addr, uint16_t *dat,uint16_t len);
rt_err_t set_servo_on(servo_type *servo,uint8_t state);
rt_tick_t count_use_time(rt_tick_t start_time);
rt_err_t set_servo_run_path(servo_type *servo);
rt_err_t set_servo_path_part(uint8_t id,uint16_t addr, uint16_t *dat,uint16_t write_reg_len);
rt_err_t set_servo_reset_origin(servo_type *servo);
rt_err_t clear_encode_pluse(servo_type *servo);
rt_err_t clear_alarm(servo_type *servo);
rt_err_t check_servo_on(servo_type *servo);
void recive_up_and_packge_run(ch_device_type *device);
void servo_control(servo_type *servo);
void recive_servo_run(ch_device_type *device);
void recive_cup_run(ch_device_type *device);
void recive_cup_run_not_check_tray(ch_device_type *device);
void jump_to_nex_tray(ch_device_type *device);
rt_err_t check_servo_inter_reg(ch_device_type *device);
rt_err_t caculate_and_write_product_packge_run_path(ch_device_type *device);
uint8_t recive_servo_run_netx_step(ch_device_type *device);
void set_cup_open_signal(uint8_t ch);
void egg_box_pile_up_check(ch_device_type *device);
uint8_t check_clear_servo_encode_pluse(servo_type *servo);
#endif  //__SERVO_COMMON_H

