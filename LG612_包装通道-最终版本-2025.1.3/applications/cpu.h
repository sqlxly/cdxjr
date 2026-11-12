#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#ifndef __CPU_USAGE__H
#define __CPU_USAGE__H

void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor);

#endif  //__CPU_USAGE__H
