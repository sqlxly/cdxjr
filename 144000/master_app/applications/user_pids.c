#include "user_pid.h"
#include <rtthread.h>

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
    float error = setpoint - actual_value;
    
    // 比例项
    float proportional = pid->kp * error;
    
    // 积分项
    pid->integral += error * dt;
    float integral = pid->ki * pid->integral;
    
    // 微分项
    float derivative = pid->kd * (error - pid->previous_error) / dt;
    
    // 计算总输出
    float output = proportional + integral + derivative;
    
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

// 初始化电机状态
void motor_state_init(MotorState *motor, int id) 
{
    motor->id = id;
    motor->speed = 0.0f;
    motor->position = 0.0f;
    motor->target_speed = 0.0f;
    motor->target_position = 0.0f;
}

// 初始化同步控制系统
void sync_control_system_init(SyncControlSystem *system) 
{
    // 初始化主电机速度PID (参数根据实际系统调整)
    pid_init(&system->speed_pid_master, 0.8f, 0.2f, 0.1f, -100.0f, 100.0f);
    
    // 初始化从电机速度PID
    pid_init(&system->speed_pid_slave, 0.8f, 0.2f, 0.1f, -100.0f, 100.0f);
    
    // 初始化位置同步PID
    pid_init(&system->position_pid, 1.2f, 0.3f, 0.05f, -50.0f, 50.0f);
    
    // 初始化电机状态
    motor_state_init(&system->master_motor, 1);
    motor_state_init(&system->slave_motor, 2);
    
    system->sync_error = 0.0f;
}

// 更新电机状态
void update_motor_states(SyncControlSystem *system, float master_speed, float master_pos, float slave_speed, float slave_pos) 
{
    system->master_motor.speed = master_speed;
    system->master_motor.position = master_pos;
    system->slave_motor.speed = slave_speed;
    system->slave_motor.position = slave_pos;
}

// 计算同步误差(位置差)
float calculate_sync_error(SyncControlSystem *system) 
{
    system->sync_error = system->master_motor.position - system->slave_motor.position;
    return system->sync_error;
}

// 应用控制输出
void apply_control_outputs(SyncControlSystem *system, float dt) {
    // 1. 计算位置同步补偿量
    float position_compensation = pid_compute(&system->position_pid, 0.0f, system->sync_error, dt);
    
    // 2. 主电机速度控制
    float master_output = pid_compute(&system->speed_pid_master, 
                                    system->master_motor.target_speed, 
                                    system->master_motor.speed, dt);
    
    // 3. 从电机速度控制,加入同步补偿
    float slave_target_speed = system->slave_motor.target_speed + position_compensation;
    float slave_output = pid_compute(&system->speed_pid_slave, 
                                   slave_target_speed, 
                                   system->slave_motor.speed, dt);
    
    // 将master_output和slave_output发送给相应的变频器
    printf("Master Motor Output: %.2f\n", master_output);
    printf("Slave Motor Output: %.2f\n", slave_output);
    printf("Sync Error: %.2f\n", system->sync_error);
}

int motor_pid_test(void)
{
    // 创建同步控制系统实例
    SyncControlSystem control_system;
    
    // 初始化控制系统
    sync_control_system_init(&control_system);
    
    // 设置目标值
    control_system.master_motor.target_speed = 1000.0f;  // RPM
    control_system.slave_motor.target_speed = 1000.0f;   // RPM
    control_system.master_motor.target_position = 0.0f;
    control_system.slave_motor.target_position = 0.0f;
    
    printf("双电机同步PID控制系统...\n");
    printf("目标速度: %.2f RPM\n", control_system.master_motor.target_speed);
    
    // 模拟时间变量
//    clock_t start_time = clock();
    float simulation_time = 0.0f;
    const float time_step = 0.01f; // 10ms采样周期

    // 模拟运行10秒
    for(int i = 0; i < 1000; i++) {  
        // 模拟读取编码器和速度反馈
        static float master_actual_speed = 0.0f;
        static float slave_actual_speed = 0.0f;
        static float master_actual_position = 0.0f;
        static float slave_actual_position = 0.0f;
        
        // 简化的电机动态模型
        master_actual_speed += (control_system.master_motor.target_speed - master_actual_speed) * 0.1f;
        slave_actual_speed += (control_system.slave_motor.target_speed - slave_actual_speed) * 0.09f; // 稍微慢一点
        
        // 位置积分
        master_actual_position += master_actual_speed * time_step / 60.0f; // 转换为转速
        slave_actual_position += slave_actual_speed * time_step / 60.0f;
        
        // 更新电机状态
        update_motor_states(&control_system, master_actual_speed, master_actual_position,
                          slave_actual_speed, slave_actual_position);
        
        // 计算同步误差
        calculate_sync_error(&control_system);
        
        // 执行PID控制计算并输出结果
        apply_control_outputs(&control_system, time_step);
        
        // 等待下一个控制周期
//        usleep((useconds_t)(time_step * 1000000));
        rt_thread_mdelay(10);
				
        simulation_time += time_step;
        
        // 每秒打印一次详细信息
        if(i % 100 == 0) {
            printf("\n--- ??: %.2fs ---\n", simulation_time);
        }
    }
    
    printf("\n控制系统模拟结束。\n");
    return 0;
}

MSH_CMD_EXPORT_ALIAS(motor_pid_test, motor_pid_test, print motor test of pid);
