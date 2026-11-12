#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "modbus_dirty.h"
#include "common.h"
#ifndef _SYNC_EGG_INFO_H__
#define _SYNC_EGG_INFO_H__

typedef struct 
{
	uint8_t empty;
  uint8_t crack[6];
	uint16_t weight[6];
	uint16_t mesg_id;
	uint32_t pluse;
}weight_and_crack_info;

void init_recive_dirty_delay_time(uint32_t pluse);
void init_recive_weight_delay_time(uint32_t pluse);
void check_dirty_and_weight_device_alarm(void);
void clear_dirty_and_wieght_alarm(void);
uint32_t get_dirty_info(uint8_t ch,ai_dat_type *dat);
uint32_t get_weight_info(uint8_t ch,weight_and_crack_info *dat);

#endif  	// _SYNC_EGG_INFO_H__
