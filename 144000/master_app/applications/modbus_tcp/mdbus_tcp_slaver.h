#ifndef __MDBUS_TCP_SLAVER_H
#define __MDBUS_TCP_SLAVER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <pthread.h>
#include "agile_modbus.h"
#include "agile_modbus_slave_util.h"

extern uint16_t u_tab_registers[16384];
extern uint8_t _tab_bits[200];

extern pthread_mutex_t slave_mtx;
extern const agile_modbus_slave_util_t slave_util;	

extern const agile_modbus_slave_util_map_t bit_maps[1];
extern const agile_modbus_slave_util_map_t input_bit_maps[1];
extern const agile_modbus_slave_util_map_t register_maps[1];
extern const agile_modbus_slave_util_map_t input_register_maps[1];

extern int init_modbus_tcp_pthread(void);
	
#ifdef __cplusplus
}
#endif

#endif
