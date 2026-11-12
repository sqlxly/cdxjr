#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#ifndef __AGLIGE_MODBUS_USER_H
#define __AGLIGE_MODBUS_USER_H

void decode_packge_servo_dat(uint8_t ch,uint8_t start,uint8_t dir);
void decode_recove_egg_servo_dat(uint8_t ch,uint8_t start,uint8_t dir);
#endif
