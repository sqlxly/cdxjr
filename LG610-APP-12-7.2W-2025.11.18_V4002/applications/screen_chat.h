
#include <stdio.h>
#include <rtthread.h>
#include "product_set.h"

#ifndef _SCREEN_CHAT_H__
#define _SCREEN_CHAT_H__


#define SCREEN_PACKGE_CH_ADDR    1188 
#define SCREEN_CURRENT_PAGE_ADDR 59
#define DEBUG_PACKGE_PAGE_NUMB   57
#define ALWAYS_TYPE  0  //一直持续状态

uint16_t get_packge_debug_ch(void);
uint16_t get_current_screen_page(void);
void dispose_screen_dat(uint16_t *tab_registers,uint16_t addr,uint16_t cmd);
uint8_t dispose_debug_packeg_dat(uint16_t *tab_registers,uint16_t addr );
void ext_key_debug_device(uint8_t key_val);
void debug_mast_control_moto(uint8_t key_val);
void start_clear_warn_alarm(void);
void dispose_throw_ch_set(void);
void disp_mix_device_part(uint16_t *addr);
void send_message_to_throw_device(uint8_t ch,uint8_t *buf);
void disp_screen_packge_set_dat(product_packge_info *dat,uint16_t *tab_registers);
void copy_screen_packge_set_dat(product_packge_info *dat,uint16_t *tab_registers);
#endif  	// _SCREEN_CHAT_H_
