
#include <rtthread.h>
#include "anasis_egg.h"

#ifndef _PRODUCT_EGG_RECODE_H__
#define _PRODUCT_EGG_RECODE_H__

#define EGG_WEIGHT_LEVE_COUNT 11
#define EGG_COLOUR_LEVE_COUNT 5
#define EGG_CRACK_LEVE_COUNT 5
#define EGG_DITY_LEVE_COUNT  4
#define EGG_BOODY_LEVE_COUNT 2

typedef struct
{
  uint32_t totle_weight;
  uint32_t egg_count;
}weight_leve_info;

typedef struct
{
  uint32_t totle_weight;
  uint32_t egg_count;
}colour_leve_info;

typedef struct
{
  uint32_t totle_weight;
  uint32_t egg_count;
}crack_leve_info;
typedef struct
{
  uint32_t totle_weight;
  uint32_t egg_count;
}dity_leve_info;

typedef struct
{
  uint32_t totle_weight;
  uint32_t egg_count;
}boody_info;

typedef struct
{
	uint8_t supply_id;
	uint16_t use_flag;
	uint16_t supply_name[10];
  weight_leve_info weight_leve[EGG_WEIGHT_LEVE_COUNT];   //10个等级 + 1个自定义
	colour_leve_info colour_leve[EGG_COLOUR_LEVE_COUNT];    // 绿白 粉 褐  +1 自定义
	crack_leve_info crack_leve[EGG_CRACK_LEVE_COUNT];     // 4 +1
	dity_leve_info dity_leve[EGG_DITY_LEVE_COUNT];       //3+1
	boody_info boody_leve[EGG_BOODY_LEVE_COUNT];          // 2
}recode_product_eggs_body;

typedef struct
{
	uint8_t supply_id;
    uint8_t weight_leve;    
	uint8_t colour_leve;    
	uint8_t crack_leve;    
	uint8_t dity_leve;       
	uint8_t boody_leve;   
	uint16_t weight;
}recode_egg_dat;


#define MAX_SUPPLY_COUNT (RECODE_ANAYSIS_EGG_DAT_LEN/sizeof(recode_product_eggs_body))
#define MIX_SYPPL_ID_FLAG   (MAX_SUPPLY_COUNT -1)
#define ALL_SYPPL_ID_FLAG    MAX_SUPPLY_COUNT

typedef struct
{
  uint32_t flag;
	uint16_t date;  //日期 月 日
	recode_product_eggs_body body[MAX_SUPPLY_COUNT];
}recode_analisys_product_type;

void load_backups_anaysis_dat();
void save_backups_anaysis_dat();

void disp_sypply_info(void);
void show_analisys_quality();
void set_disp_sypply_id(char *name);
uint32_t get_up_dat(uint32_t date,uint8_t dir);
uint8_t find_supply_id_from_recode_dat(char *supply_name);
void recode_analysis_egg_info(recode_egg_dat *egg_dat,uint8_t index);  
void get_today_quity(recode_analisys_product_type *dat);
#endif  	// _PRODUCT_EGG_RECODE_H__
