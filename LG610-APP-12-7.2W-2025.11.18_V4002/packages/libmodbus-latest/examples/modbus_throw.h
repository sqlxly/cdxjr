
#include <rtthread.h>
#ifndef __MODBUS_THROW_H
#define __MODBUS_THROW_H

uint8_t get_throw_ch_state(uint8_t ch);
uint8_t get_throw_device_en_flag(uint8_t id);
uint16_t get_throw_ch_err_count(uint8_t ch);
#endif  //__MODBUS_THROW_H

