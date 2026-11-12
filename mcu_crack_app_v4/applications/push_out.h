#ifndef __PUSH_OUT_H__
#define __PUSH_OUT_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

extern void push_out_gpio_init(void);

extern void push_out_control(int ch, rt_base_t val);
	
#endif

