#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <modbus.h>

#ifndef __RELAY_H
#define __RELAY_H

#include "drv_key.h"
#define RELAY1 GET_PIN(E, 8)
#define RELAY2 GET_PIN(E, 10)
#define RELAY3 GET_PIN(E, 12)
#define RELAY4 GET_PIN(E, 15)
#define RELAY5 GET_PIN(D, 7)
#define RELAY6 GET_PIN(D, 6)

#define PRINT_IO0 GET_PIN(B, 0)
#define PRINT_IO1 GET_PIN(B, 1)
#define PRINT_IO2 GET_PIN(E, 9)
#define PRINT_IO3 GET_PIN(E, 11)
#define PRINT1_EN GET_PIN(E, 13)
#define PRINT_IO5 GET_PIN(E, 14)
#define PRINT_IO6 GET_PIN(D, 12)
#define PRINT_IO7 GET_PIN(D, 13)
#define PRINT_IO8 GET_PIN(D, 14)
#define PRINT2_EN GET_PIN(D, 15)

void check_en_print();
void set_print_id(uint8_t ch,uint8_t print_id);
void expansion_link_up(void);
void expansion_link_down(void);
void expansion_link_stop(void);
rt_err_t relay_init(void);
void set_en_print_egg(uint8_t ch);
uint8_t frequency_moto_enable(void);
uint8_t frequency_moto_disable(void);
#endif  //__RELAY_H
