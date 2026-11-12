
#include <rtthread.h>
#include "product_set.h"

#ifndef _AI_MODBUS_H__
#define _AI_MODBUS_H__

#define TRANSPORT_ROW_NUMBER   6        //传输线行数
#define TRANSPORT_STEP_NUMBER  100      //传输线格子数量

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;

typedef struct
{
    uint8_t is_egg;
	  uint8_t dirty;
	  uint8_t rgb[3];
	  uint8_t level;
}ai_body_type;

typedef struct
{
   uint16_t id;
	 uint8_t camera1_state;
	 uint8_t camera2_state;
	 ai_body_type body[6];
}ai_dat_type;

typedef struct
{
	uint8_t empty;
	uint8_t colour;       //颜色
	uint8_t dity_leve;    //脏蛋
}tramsport_egg_info_type;
//传输线鸡蛋数据
typedef struct
{
	tramsport_egg_info_type info[TRANSPORT_STEP_NUMBER+5][TRANSPORT_ROW_NUMBER]; 
}	transport_eg_type;                                


#endif  	// _AI_MODBUS_H__
