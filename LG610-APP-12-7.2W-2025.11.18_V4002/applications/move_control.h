#ifndef __MOVE_CONTROL_H
#define __MOVE_CONTROL_H
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

void sys_start_clalure_expansion_link_offest(void);
uint8_t get_expansion_link_to_orgin_state(void);
uint16_t get_expansion_link_offeset(void);
uint16_t get_expansion_link_res(void);
void expansion_link_to_orgin(void);
void expansion_link_control(uint8_t state);
void debug_frequency_motor(uint8_t mode,uint8_t speed,uint8_t state);
void set_expansion_link_move(uint8_t dir,uint16_t position);

#endif  //__MOVE_CONTROL_H
