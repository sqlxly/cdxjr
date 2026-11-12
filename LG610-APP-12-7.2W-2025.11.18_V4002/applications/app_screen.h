
#include "modbus_packge.h"
#include <user_set.h>
#include <rtthread.h>

#ifndef __APP_SCREEN_H__
#define __APP_SCREEN_H__


#define CH1_CHECK_TRAY_PIN (1-1)
#define CH1_PACKGE_RESET_ORIGIN_PIN (2-1)
#define CH1_EGG_CUP_OPEN_SENSER_PIN (3-1)
#define CH1_RECIVE_RESET_ORIGIN_PIN (4-1)
#define CH1_PILE_UP_PIN             (5-1)
#define CH1_RECIVE_SERVO_CMP_PIN    (6-1)
#define CH1_RESET_ORIGIN_PIN        (7-1)
#define CH1_RUN_NEX_STEP_PIN        (8-1)
#define CH1_PACKGE_SERVO_CMP_PIN    (9-1)
#define CH1_PACHGE_SERVO_STATE_PIN  (10-1)
#define CH1_RECIVE_SERVO_STATE_PIN  (11-1)

#define CH1_SERVO_ALARM_PIN         (12-1)
#define CH2_CHECK_TRAY_PIN          (13-1)
#define CH2_PACKGE_RESET_ORIGIN_PIN (14-1) 
#define CH2_EGG_CUP_OPEN_SENSER_PIN (15-1)
#define CH2_RECIVE_RESET_ORIGIN_PIN (16-1)
#define CH2_PILE_UP_PIN             (17-1)
#define CH2_RECIVE_SERVO_CMP_PIN    (18-1)
#define CH2_RESET_ORIGIN_PIN        (19-1)
#define CH2_RUN_NEX_STEP_PIN        (20-1)
#define CH2_PACKGE_SERVO_CMP_PIN    (21-1)
#define CH2_PACHGE_SERVO_STATE_PIN  (22-1)
#define CH2_RECIVE_SERVO_STATE_PIN  (23-1)
#define CH2_SERVO_ALARM_PIN         (24-1)
void show_decive_offest();
void set_alarm_flag(uint8_t flag);
void disp_debug_number1(uint16_t a);
void disp_debug_number2(uint16_t a);
void disp_debug_number3(uint16_t a);
void disp_debug_srt(uint8_t *str,uint8_t len);
void refresh_real_time(void);
void refresh_dat_to_screen(void);
uint16_t ReadMBHotRegU16(uint16_t addr);
uint32_t ReadMBHotRegU32(uint16_t addr);
void WriteMBHotRegU16(uint16_t addr,uint16_t value);
void WriteMBHotRegU32(uint16_t addr, uint32_t value);
void CopyToMBHotReg(uint16_t addr,uint8_t *dat,uint16_t len);
void disp_alarm(uint8_t offset,uint8_t state);
void show_throw_ch_set(void);
void refresh_screen();
void clear_update_text_buf();
void set_pluse_adjust_val(uint32_t val);
void CopyFromMBHotReg(uint16_t addr,uint8_t *dat,uint16_t len);
void disp_force_egg_info(alread_production_type *dat);
#endif /*__APP_SCREEN_H__ */

















