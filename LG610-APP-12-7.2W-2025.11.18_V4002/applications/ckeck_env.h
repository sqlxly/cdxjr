
#include <rtthread.h>

#ifndef _CHECK_ENV_H__
#define _CHECK_ENV_H__

void sync_packge_control_state_idle();
void sync_packge_control_state(uint8_t cstate);
void set_control_run_state(uint8_t ch,uint8_t state);
void set_check_env_step(uint8_t step);
uint8_t check_system_per_start_part();
uint8_t check_per_product_info();
void sync_product_packge();
#endif  	// _CHECK_ENV_H__
