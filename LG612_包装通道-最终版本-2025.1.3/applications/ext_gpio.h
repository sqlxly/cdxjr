#include <rtthread.h>
#ifndef _EXT_GPIO_H__
#define _EXT_GPIO_H__

#include "board_config.h"




#define RELAY1  GET_PIN(A, 4)
#define RELAY2  GET_PIN(A, 5)
#define RELAY3  GET_PIN(A, 6)
#define RELAY4  GET_PIN(B, 0)


typedef enum
{
	left_tray_senser_pin=0,            //左托盘检测传感器脚
	left_conveyor_pass_zero_pin,       //左传输带过零脚
	left_egg_close_reach_pin,          //左蛋杯关闭到位
	left_receive_claw_up_reach_pin,       //左接收抓上升到位
}ext_key_id;


void set_relay1(uint8_t val);
void set_relay2(uint8_t val);
void set_relay3(uint8_t val);
void set_relay4(uint8_t val);
void relay_init();

uint8_t get_nex_tray_flag(uint8_t id);
void clear_tray_flag(uint8_t id);
uint8_t get_tray_flag(uint8_t id);
void read_gpio_pin(void);
uint8_t get_ext_pin(ext_key_id pin_id);
uint32_t get_control_input_io_dat(void);
void set_tray_flag(uint8_t id);
void set_check_tray_flag(uint8_t tray_flag);
#endif  //_EXT_GPIO_H__
