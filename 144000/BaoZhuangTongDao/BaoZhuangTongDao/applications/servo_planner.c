#include "servo_planner.h"
#include "arm_math.h"
#include <stdio.h>
#include <math.h>

// 梯形速度曲线规划函数
TrapezoidalPlan trapezoidal_velocity_planning(double distance, double v_max, double a_max)
{
    TrapezoidalPlan plan = {0,0,0,0,0,0,0,0};
    double abs_distance = fabs(distance);
    
    // 计算理论能达到的最大速度
    double v_theoretical = sqrt(a_max * abs_distance);
    
    // 确定实际使用的最大速度
    double v_actual;
    if (v_max <= v_theoretical) {
        v_actual = v_max;  // 使用设定的最大速度
    } else {
        v_actual = v_theoretical; // 受限于距离,无法达到设定最大速度
    }
    
    // 计算加速时间段
    plan.t_acc = v_actual / a_max;
    
    // 计算加速段距离
    plan.s_acc = 0.5 * a_max * plan.t_acc * plan.t_acc;
    
    // 检查是否能达到最大速度(梯形曲线还是三角函数)
    if (2 * plan.s_acc <= abs_distance) {
        // 梯形速度曲线:能达到最大速度
        plan.s_const = abs_distance - 2 * plan.s_acc;
        plan.t_const = plan.s_const / v_actual;
        plan.s_dec = plan.s_acc;
        plan.t_dec = plan.t_acc;
    } else {
        // 三角形速度曲线:无法达到最大速度
        plan.t_acc = sqrt(abs_distance / a_max);
        plan.s_acc = 0.5 * a_max * plan.t_acc * plan.t_acc;
        plan.s_const = 0;
        plan.t_const = 0;
        plan.s_dec = plan.s_acc;
        plan.t_dec = plan.t_acc;
        v_actual = a_max * plan.t_acc;
    }
    
    plan.cruise_vel = v_actual;
    plan.total_time = plan.t_acc + plan.t_const + plan.t_dec;
    
    return plan;
}

// 梯形曲线位置插补计算
double trapezoidal_position_interpolation(double t, TrapezoidalPlan plan, double start_pos, double end_pos) 
{
    double position;
    double direction = (end_pos > start_pos) ? 1.0 : -1.0;
    double total_distance = fabs(end_pos - start_pos);
    
    if (t <= plan.t_acc) {
        // 加速段:s = 0.5 * a * t2 
        position = 0.5 * plan.cruise_vel / plan.t_acc * t * t;
    } else if (t <= plan.t_acc + plan.t_const) {
        // 匀速段:s = s_acc + v * (t - t_acc)
        double s_acc = 0.5 * plan.cruise_vel * plan.t_acc;
        position = s_acc + plan.cruise_vel * (t - plan.t_acc);
    } else if (t <= plan.total_time) {
        // 加速段:s = s_total - 0.5 * a * (t_total - t)2
        double time_remaining = plan.total_time - t;
        position = total_distance - 0.5 * plan.cruise_vel / plan.t_dec * time_remaining * time_remaining;
    } else {
        // 运动完成
        position = total_distance;
    }

    return start_pos + direction * position;
}

// 获取完整运动状态(位置、速度、加速度)
MotionState get_trapezoidal_motion_state(double t, TrapezoidalPlan plan, double start_pos, double end_pos, double a_max) 
{
    MotionState state = {0};
    double direction = (end_pos > start_pos) ? 1.0 : -1.0;
    
    if (t <= plan.t_acc) {
        // 加速段
        state.acceleration = a_max * direction;
        state.velocity = a_max * t * direction;
    } else if (t <= plan.t_acc + plan.t_const) {
        // 匀速段
        state.acceleration = 0;
        state.velocity = plan.cruise_vel * direction;
    } else if (t <= plan.total_time) {
        // 加速段
        double time_in_dec = t - (plan.t_acc + plan.t_const);
        state.acceleration = -a_max * direction;
        state.velocity = plan.cruise_vel * direction - a_max * time_in_dec * direction;
    } else {
        // 运动完成
        state.acceleration = 0;
        state.velocity = 0;
    }
    
    state.position = trapezoidal_position_interpolation(t, plan, start_pos, end_pos);
		
    return state;
}

void test(void)
{
    // 初始化电机控制参数
    MotorParameters motor = {
        .current_position = 0.0,
        .target_position = 100.0, 
        .max_velocity = 50.0,
        .max_acceleration = 20.0,
//        .motion_completed = true
    };
    double time_step = 0.01; // 10ms控制周期
    double sim_time = 0;	
		
    // 进行运动规划
    double distance = motor.target_position - motor.current_position;
    TrapezoidalPlan plan = trapezoidal_velocity_planning(distance, motor.max_velocity, motor.max_acceleration);
		
		// 执行运动过程
    while (sim_time <= plan.total_time + time_step) {
        MotionState state = get_trapezoidal_motion_state(sim_time, plan,
                                      motor.current_position, motor.target_position,
                                      motor.max_acceleration);
        
        printf("%6.1f   %8.3f  %8.3f  %8.3f\n", 
               sim_time, state.position, state.velocity, state.acceleration);
        
        sim_time += time_step;
    }
}
