
#include <rtthread.h>

#ifndef _PRODUCT_CONTROL_H__
#define _PRODUCT_CONTROL_H__

#define DEBUG_IDLE 0
#define DEBUG_CUP_JUMP_TO_LOCK_POSITION 1
#define DEBUG_MAIN_MOTO 2

uint8_t get_debug_mode_index();
void set_debug_mode_index(uint8_t index);
void lamp_pole_disp(uint8_t state);
void set_cup_move_to_position(uint16_t number);

#endif  	// _PRODUCT_CONTROL_H__
