#ifndef __USER_ADC_H
#define __USER_ADC_H
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


rt_err_t adc_init(void);
float get_adc_votage(void);
extern rt_uint32_t rt_adc_read1(rt_adc_device_t dev, rt_uint32_t channel);
#endif  //__USER_ADC_H
