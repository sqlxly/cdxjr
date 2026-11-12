
#include <rtthread.h>
#include "product_set.h"

#ifndef _MODBUS_DIRTY_H_
#define _MODBUS_DIRTY_H_

#define TRANSPORT_ROW_NUMBER   6        //传输线行数
#define TRANSPORT_STEP_NUMBER  100      //传输线格子数量

typedef struct
{
    int fd;
    rt_tick_t tick_timeout;
}client_session_t;



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


#endif  	// _MODBUS_DIRTY_H__

