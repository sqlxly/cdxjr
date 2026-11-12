#ifndef SERVO_PLANNER_H
#define SERVO_PLANNER_H

#include <stdbool.h>
#include <drv_common.h>

/****** 实现伺服电机梯形加减速运动控制算法 ******/

// 电机参数结构体
typedef struct {
    double current_position;    // 当前位置 (单位PUU)
    double target_position;     // 目标位置
    double max_velocity;        // 最大速度 (单位PUU/sec)
    double max_acceleration;    // 最大加速度
		double time_step; 				  // 10ms控制周期
		double sim_time;	 				  // 运行了多少个周期		
    uint8_t motion_completed;   // 运动完成标志 执行原点复归的标志
		int32_t package_postion[6]; // 包装容器位置信息
	  uint8_t row;								// 容器放多少排鸡蛋
	  uint8_t chainLinkCnt;				// 占用几个蛋格
} MotorParameters;

// 规划参数结构体
typedef struct {
    double t_acc;        // 加速段时间
    double t_const;      // 匀速段时间
    double t_dec;        // 减速段时间
    double total_time;   // 总时间
    double s_acc;        // 加速段距离
    double s_const;      // 匀速段距离
    double s_dec;        // 减速段距离
    double cruise_vel;   // 实际巡航速度
} TrapezoidalPlan;

// 运动状态结构体
typedef struct {
    double position;     	// 当前位置
    double velocity;    	// 当前速度
    double acceleration; 	// 当前加速度
} MotionState;

// 相关函数
TrapezoidalPlan trapezoidal_velocity_planning(double distance, double v_max, double a_max);

double trapezoidal_position_interpolation(double t, TrapezoidalPlan plan, double start_pos, double end_pos);

MotionState get_trapezoidal_motion_state(double t, TrapezoidalPlan plan, double start_pos, double end_pos, double a_max);

void update_motor_position(MotorParameters* motor, double delta_time);

#endif
