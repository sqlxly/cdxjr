#ifndef __MODBUS_TCP_TEST_H
#define __MODBUS_TCP_TEST_H
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <modbus.h>


typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

extern struct rt_mailbox screen_mb;
extern modbus_mapping_t *mb_mapping ;

uint16_t get_screen_set_val_type16(uint16_t addr);
uint32_t get_screen_set_val_type32(uint16_t addr);

#endif
