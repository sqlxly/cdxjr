
#include <rtthread.h>
#ifndef __TRANSPORT_MODBUS_H
#define __TRANSPORT_MODBUS_H

/*
typedef enum
{
	WASH_BRUSH=0, //清洗毛刷 11
	ADD_MEDICINE, //加药     12
	WASH_MOTO,    //清洗电机 13
	WASH_FAN,     //清洗风机 14
  DRY_FAN,     //干燥风机  16
	DRY_BRUSH,    //干燥毛刷 15
	OIL_PRINT,    //喷油     18
	OIL_ABSORB,   //吸油     19
	AUTO_WASH_BRUSH,    //自动清洗毛刷    //照明   100
	UV_LAMP,      //紫外杀菌                       17
	AUTO_WASH_PUMP, //自动清洗水泵                 101
	AUTO_WASH_MOTO,  //自动清洗水泵                102
}TRANSPORT_OUTPUT_IO_ID;
*/

typedef enum
{
	WASH_BRUSH=0, //清洗毛刷 11
	WASH_FAN,     //清洗风机 14
	DRY_BRUSH,    //干燥毛刷 15
	DRY_FAN,     //干燥风机  16
	OIL_ABSORB,   //吸油     19
	AUTO_WASH_BRUSH,    //自动清洗毛刷    //照明   100
	UV_LAMP,      //紫外杀菌                       17
	AUTO_WASH_PUMP, //自动清洗水泵                 101
	AUTO_WASH_MOTO,  //自动清洗水泵                102
	N1,
	N2,
	N3,
	ADD_MEDICINE, //加药     12
	WASH_MOTO,    //清洗电机 13
	OIL_PRINT,    //喷油     18
}TRANSPORT_OUTPUT_IO_ID;

typedef enum
{
    ch1_bad_egg_count=0,
	ch2_bad_egg_count,
	ch3_bad_egg_count,
    medicine_leve_senser,
	Electrical_trip_alarm,
	light_ch1_unnormal,         //光检通道异常
	reset_key,
	light_ch2_unnormal,
	ch9_not_use,
	ch10_not_use,
	ch11_not_use,
	ch12_not_use,
}TRANSPORT_INPUT_IO_ID;

typedef struct
{
	uint8_t gpio[16];  //12个输出

}transport_output_io_type;

typedef struct
{
  uint8_t gpio[12];
}transport_input_io_type;

typedef struct
{
	transport_output_io_type output;
	transport_input_io_type input;
}transport_io_dat;
uint16_t get_bad_egg_count(void);
void set_transport_io(TRANSPORT_OUTPUT_IO_ID id,uint8_t val);
transport_io_dat *get_transport_io_info(void);
uint8_t get_transport_ch_state(void);
#endif  //
