
#include <rtthread.h>

#ifndef _ALARM_CONTROL_H__
#define _ALARM_CONTROL_H__


typedef enum
{
	transport_server_com_err=0,   //通道1 传输伺服电机    通讯 错误
	transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	tick_egg_moto1_err,            //通道1 电磁铁1错误
	tick_egg_moto2_err,            //通道1 电磁铁2错误
	tick_egg_moto3_err,            //通道1 电磁铁3错误
	tick_egg_moto4_err,            //通道1 电磁铁4错误
	tick_egg_moto5_err,            //通道1 电磁铁5错误
	tick_egg_moto6_err,            //通道1 电磁铁6错误
	auto_send_tray_moto_err,       //通道1 自动发托电机故障
	slope_transport_moto_err,      //通道1 斜坡输送电机故障
	transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	egg_cup_step_moto_err,         //蛋杯开关步进电机故障
	tray_empty_err,
	alarm_max_id,
}CONTROL_ALARM_TYPE;


uint32_t get_control_alarm(uint8_t device);
uint8_t get_alarm(uint8_t ch,CONTROL_ALARM_TYPE type);
void set_alarm(uint8_t ch,CONTROL_ALARM_TYPE type,uint8_t val);
uint32_t clear_control_alarm(uint8_t device);

#endif  	// _ALARM_CONTROL_H__
