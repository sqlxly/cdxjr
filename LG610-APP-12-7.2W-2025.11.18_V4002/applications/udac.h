#ifndef __USER_DAC_H
#define __USER_DAC_H
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


rt_err_t adc_init(void);
rt_err_t G8403_init(void);
uint16_t get_moto_speed(void);
rt_err_t set_main_moto_speed(uint16_t speed);     
#endif  //__USER_ADC_H
