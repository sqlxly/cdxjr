#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "servo_common.h"

#ifndef __SERVO_CONTROL_H
#define __SERVO_CONTROL_H

#define RS4851_RECIVE_ID 3
#define RS4851_PACKGE_ID 1

#define RS4852_RECIVE_ID 4
#define RS4852_PACKGE_ID 2

void set_ch1_recive_egg(void);
void set_ch2_recive_egg(void);
void set_servo_run_state(uint8_t *buf);
void get_servo_packge_info(servo_packge_type *control);
void get_servo_run_state(packge_control_type *control);
void write_product_packge(uint8_t ch,product_packge_info *packge);


#endif  //__SERVO_CONTROL_H
