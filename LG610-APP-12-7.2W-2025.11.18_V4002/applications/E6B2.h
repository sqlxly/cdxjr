
#include <rtthread.h>

#ifndef _E2B6_H__
#define _E2B6_H__


//行车测速编码器三相引脚定义
#define ENCODER1_PHASE_A     GET_PIN(A, 5)
#define ENCODER1_PHASE_B     GET_PIN(C, 13)
#define ENCODER1_PHASE_Z     GET_PIN(E, 6)
#define ENCODER2_PHASE_A     GET_PIN(E, 5)
#define ENCODER2_PHASE_B     GET_PIN(E, 4)
#define ENCODER2_PHASE_Z     GET_PIN(E, 3)
#define ENCODER3_PHASE_A     GET_PIN(E, 2)
#define ENCODER3_PHASE_Z     GET_PIN(E, 1)
#define ENCODER3_PHASE_B     GET_PIN(E, 0)
#define ENCODER_CYCLE_COUNT               1000

uint16_t get_avg_per_hour_eggs();
void clear_spt_pluse(void);
uint16_t get_spt_speed(void);
uint32_t get_tsp_encode_pluse(void);
uint32_t get_spt_encode_pluse(void);
int32_t get_tspdis_encode_pluse(void);
void reset_main_moto_to_origin();  //主电机回归原点
uint8_t get_reset_origin_state();
uint16_t get_poer_up_reset_flag();
void adjust_encode_pluse();
#endif  	// _E2B6_H__
