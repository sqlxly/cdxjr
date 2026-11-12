#include "user_pid.h"
#include <rtthread.h>
#include <string.h>

// 初始化PID控制器
void pid_init(PIDController *pid, float kp, float ki, float kd, float min_output, float max_output) 
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = 0.0f;
    pid->integral = 0.0f;
    pid->previous_error = 0.0f;
    pid->output_min = min_output;
    pid->output_max = max_output;
}

// 计算PID输出
float pid_compute(PIDController *pid, float setpoint, float actual_value, float dt)
{
    // 计算误差
//    float error = setpoint - actual_value;
    float error = actual_value - setpoint;
	
    // 比例项
    float proportional = pid->kp * error;
    
    // 积分项
    pid->integral += error * dt;
    float integral = pid->ki * pid->integral;
    
    // 微分项
    float derivative = pid->kd * (error - pid->previous_error) / dt;
    
    // 计算总输出
//    float output = proportional + integral + derivative;
	float output = proportional + derivative;
    
    // 输出限幅
    if (output > pid->output_max) {
        output = pid->output_max;
        // 抗积分饱和
        if (error * pid->output_max < 0) {
            pid->integral -= error * dt;
        }
    } else if (output < pid->output_min) {
        output = pid->output_min;
        // 抗积分饱和
        if (error * pid->output_min > 0) {
            pid->integral -= error * dt;
        }
    }

    // 更新历史误差
    pid->previous_error = error;
    pid->setpoint = setpoint;
    
    return output;
}


