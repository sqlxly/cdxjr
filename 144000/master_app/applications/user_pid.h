#ifndef _USER_PID_H_
#define _USER_PID_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// PID参数结构体
typedef struct {
    float kp;           	// 比例系数
    float ki;           	// 积分系数
    float kd;           	// 微分系数
    float setpoint;     	// 设定值
    float integral;     	// 积分项累计值
    float previous_error;	// 上一次误差
    float output_min;   	// 输出最小值
    float output_max;   	// 输出最大值
} PIDController;

// 电机状态结构体
typedef struct {
    int id;             		// 电机ID
    float speed;        		// 当前速度
    float position;     		// 当前位置
    float target_speed; 		// 目标速度
    float target_position;	// 目标位置
} MotorState;

// 同步控制系统结构体
typedef struct {
    PIDController speed_pid_master;   // 主电机速度PID
    PIDController speed_pid_slave;    // 从电机速度PID
    PIDController position_pid;       // 位置同步PID
 //   MotorState master_motor;          // 主电机状态
 //   MotorState slave_motor;           // 从电机状态
    float sync_error;                 // 同步误差
} SyncControlSystem;

// 函数声明
void pid_init(PIDController *pid, float kp, float ki, float kd, float min_output, float max_output);

float pid_compute(PIDController *pid, float setpoint, float actual_value, float dt);

void sync_control_system_init(SyncControlSystem *system);

void update_motor_states(SyncControlSystem *system, float master_speed, float master_pos, float slave_speed, float slave_pos);

//void apply_control_outputs(SyncControlSystem *system, float dt);

#endif
