
#include <rtthread.h>
#include "product_set.h"
#include "mdebug.h"
#ifndef _EGG_CONTROL_H__
#define _EGG_CONTROL_H__


#define MAX_SEPARATE_LINE      12        //最大分离线条数 
#define MAX_RECIVE_CUP_NUMB    6       //每通道最大接收蛋杯数量 

#define EGG_FALL_DOWN_TIME     150     //145   //鸡蛋掉落时间单位MS
#define MAX_LINE_NUMBER 18

#define SEPARATE_CUP_LEN       166      //分级线蛋夹长度  单位脉冲    
#define SEPARATE_RECIVE_CUP_LEN 157     //接收爪蛋杯间距
#define SEPARATE_CUP_distem 63.5        //分级线蛋杯间距 mm

#define CUP_IS_EMPTY         0    //空蛋杯
#define NEED_TICK_EGG        1    //没有踢蛋
#define TICK_EGG_DOWN        2    //踢蛋中
#define TICK_EGG_IS_OVER     3    //踢蛋完成
#define TICK_EGG_IS_BUSY     4    //接受抓忙碌
#define TICK_EGG_IS_OVERFLOW 5    //踢蛋时脉冲超过范围
/*
************************ 按下复位后 在传输通道6号)(离主电机进的这个抓)放一个蛋这个就是0号杯，也就是基准点*****
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
************************ 可以用这个基准点 设置每个通道的偏移量************************************************
*/





typedef struct
{
	uint8_t empty;
	uint8_t colour;
	uint8_t dity;    //脏蛋
	uint8_t crack;        //裂纹
	uint8_t spt_ch_pos;        //
	uint8_t print_id;
	uint8_t family_ch;
	uint8_t separate_ch;
	uint8_t tick_ch;
	uint16_t wetght;       //重量
	uint16_t id;
	uint16_t cup_open_time;
	uint32_t pluse;
  rt_tick_t tick_time;
}separate_egg_info_type;



//    pos
//123456
//******
//          pos
//      123456
//************
//分离线鸡蛋数据   POS是居于原点向后偏移  也就是鸡蛋从后面添加;
typedef struct
{
	separate_egg_info_type cha_info[MAX_CUP_COUNT+50]; 
	separate_egg_info_type chb_info[MAX_CUP_COUNT+50]; 
	uint16_t cha_pos;
	uint16_t chb_pos;
    	   //基于原点的偏移   
}separte_egg_type;    

typedef struct 
{
	separate_egg_info_type *start_addr;
	separate_egg_info_type *end_addr;
}separate_egg_info_point_type;

void start_tick_eggs(void);
void show_tick_egg_ch(void);
void clear_save_analisys_egg_recode_index(void);
void get_save_ayaylisis_egg_index(char *name);
void sync_packge_recive_cup_eggs(void);
uint8_t caculate_spt_ch(uint8_t product_id,uint16_t spt_ch);
uint32_t get_packge_ch_alread_product_egg(uint8_t ch);
void init_recode_kick_egg_info(uint8_t ch,uint16_t line);
void get_already_product_egg(alread_production_type *dat);
void init_avg_spt_egg_info(uint8_t product_id,uint16_t falimy,uint8_t max_eggs);
void clear_per_ch_already_pruduct_egg_dat(uint16_t index);
void check_recive_servo_egg_is_down();
void clear_already_pruduct_egg_dat(uint16_t index);
void get_pst_egg_info(uint8_t ch,uint16_t offeset,separate_egg_info_type **dat,uint8_t debug);
#endif  	// _EGG_CONTROL_H__
