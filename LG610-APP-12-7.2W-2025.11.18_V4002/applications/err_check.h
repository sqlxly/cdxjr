#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <modbus.h>

#ifndef __CUP_POSITION_H
#define __CUP_POSITION_H


typedef struct
{
	uint32_t no_weight;
	uint32_t empty_cup_count;
	uint32_t need_tick_count;
	uint32_t tick_over_count;
  uint32_t recive_busy_count;
	uint32_t over_flow_count;
	uint32_t totle_count;
}throw_ch_eggs_type;

void check_empty_cup(void);
void per_min_egg_dat_move(void);
void clear_throw_ch_egg_err_count(void);
void count_per_min_egg(uint8_t ch);
uint16_t *get_cup_err_count(uint8_t ch);
uint16_t *get_electromagnet_err_count(uint8_t ch);
void clear_totle_eggs_count(void);
void clear_electromagnet_err_count(void);
void clear_one_electromagnet_err_count(uint8_t ch,uint8_t id);
void clear_all_cup_err_count(void);
void clear_ont_cup_err_count(uint8_t ch,uint16_t id);
void power_up_read_cup_err_count_from_epp(void);
#endif  //__CUP_POSITION_H
