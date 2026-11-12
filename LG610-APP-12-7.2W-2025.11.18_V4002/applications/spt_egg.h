#include <rtthread.h>
#include <rtdevice.h>
#include <user_set.h>
#include "product_set.h"

#ifndef __SPT_EGG_H_
#define __SPT_EGG_H_


void init_egg_boxfull_info(uint8_t id,uint16_t min_weight,uint16_t max_weight);
void clear_avg_product_count(void);
void spt_egg_to_packge_line(void);
void clacute_blood_offest(uint16_t offest);
#endif  //__SPT_EGG_H_
