#ifndef __NETTOOLS_H__
#define __NETTOOLS_H__

#include <rtthread.h>

void lan_or_4G_select(rt_int8_t sel);

void set_netdev_static_address(rt_int8_t type, const char *str);
	
void netdev_work_mode_select(rt_int8_t mode, rt_uint8_t len, char *fomart, ...);

#endif
