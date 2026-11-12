#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#ifndef __KICK_EGG_H
#define __KICK_EGG_H

typedef struct 
{
	uint8_t state;
	uint16_t open_time;
	rt_tick_t start_time;
}kick_egg_dat;


typedef enum
{
  left_egg_cup_1=0, //◊Û≈≈µ∞±≠
	left_egg_cup_2,
	left_egg_cup_3,
	left_egg_cup_4,
	left_egg_cup_5,
	left_egg_cup_6,
	right_egg_cup_1,  //”“≈≈µ∞±≠
	right_egg_cup_2,
	right_egg_cup_3,
	right_egg_cup_4,
	right_egg_cup_5,
	right_egg_cup_6,
	EGG_CUP_NUMBER,
}mb_device_type;

uint8_t get_spt_speed();
uint8_t get_recive_cup_egg_buf(uint8_t ch);

void decode_close_egg_cup_dat(uint8_t *dat);
void decode_run_egg_cup_dat(uint8_t *dat);
void decode_debug_egg_cup_dat(uint8_t *dat);
void clear_recive_cup_egg_buf(uint8_t packge_ch);
uint8_t get_recive_cup_egg_count(uint8_t ch);
void set_servo_down_number(uint8_t ch,uint8_t count);
#endif  //__KICK_EGG_H
