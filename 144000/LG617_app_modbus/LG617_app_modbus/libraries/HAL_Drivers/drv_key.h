#ifndef __DRV_KEY_H
#define __DRV_KEY_H

#include <rtthread.h>

#define TAG_KEY_ACTION      (0x01)
#define LEFT_KEY_ACTION     (0x02)
#define RIGHT_KEY_ACTION    (0x03)
#define QUIT_KEY_ACTION     (0x04)

int key_scan_init(void);

rt_ubase_t wait_key_plus(rt_uint32_t timeout);

#endif
