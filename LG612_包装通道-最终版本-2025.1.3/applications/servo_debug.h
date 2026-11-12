#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "servo_common.h"
#ifndef __SERVO_DEBUG_H
#define __SERVO_DEBUG_H


typedef struct
{
	uint8_t excute_flag;
	uint8_t state;
	uint8_t dir;
}debug_servo_type;

int32_t get_current_distem(uint8_t id,uint32_t t_len,uint32_t cycle_step);
void user_debug_packge_servo(uint8_t id, uint8_t state,uint8_t dir);
void user_debug_recive_claw_servo(uint8_t id, uint8_t state,uint8_t dir);
void check_recive_servo_in_limit_position(servo_type *servo);
void check_user_debug_servo(servo_type *servo,debug_servo_type *debug);
void set_servo_reset_to_origin(servo_type *packge_servo,servo_type *recive_servo,uint8_t cup_id);
void ch1_servo_debug_mode(void);
void ch2_servo_debug_mode(void);
void clear_reset_origin_flag(uint8_t ch);
void debug_ch1_packge_servo_reset_to_origin(void);
void debug_ch2_packge_servo_reset_to_origin(void);
void debug_packge_servo_run_step(uint8_t id,int16_t pluse);
#endif  //__SERVO_DEBUG_H
