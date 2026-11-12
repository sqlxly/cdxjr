
#include <user_set.h>
#include <rtthread.h>

#ifndef __COMMON_H__
#define __COMMON_H__


#define SYSTEM_IS_IDLE    0
#define SYS_IS_CHECK_ENV  1
#define SYSTEM_IS_RUING   2
#define SYSTEM_IS_SUPPEND 3
#define SYSTEM_IS_RESET   4
#define SYSTEM_IS_STOP    5
#define SYS_IS_CRC_PART   6   //系统校验参数
#define SYSTEM_IS_READY   7   //系统准备好了
#define SYSTEM_IS_EMERGENCY_STOP 8
#define SYSTEM_IS_SCHECK 9
#define SYSTEM_IS_ENTER_IDLE 10


#define MAX_CUP_COUNT 360
#define MIN_CUP_COUNT 100
#define MAX_TRANSPORT_LINE_COUNT 110  //最大200根


typedef struct
{
    uint8_t is_egg;
	  uint8_t dirty;
	  uint8_t rgb[3];
	  uint8_t level;
}ai_body_type;

typedef struct
{
	uint16_t id;							// 收到脏蛋信息的时候保存脏蛋信息里面的输送线id？
	 uint8_t camera1_state;
	 uint8_t camera2_state;
	 ai_body_type body[6];		// 一排6个蛋，每个蛋里面有是否有蛋标记，脏蛋值，颜色值？等级值？
}ai_dat_type;

extern uint16_t SEPARATE_CHA_CUP_NUMBER;      //分级线蛋夹数量   
extern uint16_t SEPARATE_CHB_CUP_NUMBER;      //分级线蛋夹数量   
extern uint8_t const packge_map_ch_id[12];
//#define  SEPARATE_CUP_NUMBER   270

int bkp_sram_init(void);
uint8_t get_system_state(void);
void  set_system_state(uint8_t state);
void send_message_to_packge_device(uint8_t ch,uint8_t *buf);
rt_tick_t count_use_time(rt_tick_t start_time);
void bkp_sram_write(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len);
void bkp_sram_read(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len);
#endif /*__COMMON_H__ */
