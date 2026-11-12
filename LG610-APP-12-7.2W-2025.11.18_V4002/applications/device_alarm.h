
#include <rtthread.h>

#ifndef _DEVICE_ALARM_H__
#define _DEVICE_ALARM_H__

typedef enum
{
	
	POWER_PROTECT_ERR=0,   //跳闸保护
	LIGHT_CH_ERR1,          //光通道错误
	LIGHT_CH_ERR2,          //光通道错误
	MEDICINE_LEVE_LOW,     //药液位低报警
	TRANSPORT_CH_COM_ERR,
	PACKGE_BOARD1_COM_ERR,
	PACKGE_BOARD2_COM_ERR,
	PACKGE_BOARD3_COM_ERR,
	PACKGE_BOARD4_COM_ERR,
	PACKGE_BOARD5_COM_ERR,
	PACKGE_BOARD6_COM_ERR,
	THROW_AWAY_BOARD1_COM_ERR,
	THROW_AWAY_BOARD2_COM_ERR,
	TRANSPORT_AND_SPT_LINE_SYNC_ERR,  //传输线以及分离线 同步错误
	
	MAST_DIRVER_MOTO_ERR,   //主驱动电机故障
	LOAD_EGG_CONTROL_ERR,   //上蛋机故障
	PRINT_EGG_CONTROL_ERR,  //打印机故障
	EXPANSION_LINK_ERR,     //推杆故障
	EGG_CUP_ERR,            //蛋杯损毁
  NOT_HAVE_CHECK_NUMBER_ONE_EGG,
	EMERGENCY_STOP=20,     //急停
	
	ch1l_transport_server_com_err,     //通道1 传输伺服电机    通讯 错误
	ch1l_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch1l_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch1l_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch1l_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch1l_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch1l_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch1l_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch1l_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警

	ch1l_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch1l_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch1l_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch1l_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch1r_transport_server_com_er,   //通道1 传输伺服电机    通讯 错误
	ch1r_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch1r_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch1r_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch1r_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch1r_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch1r_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch1r_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch1r_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警

	ch1r_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch1r_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch1r_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch1r_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch2l_transport_server_com_err,   //通道1 传输伺服电机    通讯 错误
	ch2l_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch2l_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch2l_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch2l_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch2l_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch2l_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch2l_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch2l_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警

	ch2l_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch2l_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch2l_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch2l_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch2r_transport_server_com_er,   //通道1 传输伺服电机    通讯 错误
	ch2r_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch2r_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch2r_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch2r_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch2r_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch2r_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch2r_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch2r_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch2r_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch2r_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch2r_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch2r_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch3l_transport_server_com_err,   //通道1 传输伺服电机    通讯 错误
	ch3l_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch3l_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch3l_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch3l_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch3l_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch3l_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch3l_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch3l_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch3l_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch3l_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch3l_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch3l_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch3r_transport_server_com_er,   //通道1 传输伺服电机    通讯 错误
	ch3r_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch3r_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch3r_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch3r_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch3r_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch3r_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch3r_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch3r_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch3r_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch3r_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch3r_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch3r_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch4l_transport_server_com_err,   //通道1 传输伺服电机    通讯 错误
	ch4l_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch4l_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch4l_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch4l_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch4l_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch4l_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch4l_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch4l_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch4l_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch4l_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch4l_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch4l_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch4r_transport_server_com_er,   //通道1 传输伺服电机    通讯 错误
	ch4r_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch4r_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch4r_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch4r_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch4r_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch4r_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch4r_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch4r_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch4r_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch4r_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch4r_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch4r_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch5l_transport_server_com_err,   //通道1 传输伺服电机    通讯 错误
	ch5l_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch5l_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch5l_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch5l_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch5l_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch5l_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch5l_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch5l_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch5l_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch5l_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch5l_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch5l_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	ch5r_transport_server_com_er,   //通道1 传输伺服电机    通讯 错误
	ch5r_transport_server_moment_err,  //通道1 传输伺服电机    力矩 错误
	ch5r_transport_server_place_err,   //通道1 传输伺服电机    位置 错误
	ch5r_transport_server_alarm_err,   //通道1 传输伺服电机    故障 报警
	ch5r_transport_server_origin_err,  //通道1 传输伺服电机    原点 错误
	ch5r_recive_server_com_err,        //通道1 接收爪伺服电机  通讯 错误
	ch5r_recive_server_moment_err,     //通道1 接收爪伺服电机  力矩 错误
	ch5r_recive_server_place_err,      //通道1 接收爪伺服电机  位置 错误
	ch5r_recive_server_alarm_err,      //通道1 接收爪伺服电机  故障 报警
	ch5r_auto_send_tray_moto_err,       //通道1 自动发托电机故障
	ch5r_slope_transport_moto_err,      //通道1 斜坡输送电机故障
	ch5r_transport_ch_is_not_have_tray, //通道1 传输通道没有蛋托
	ch5r_slope_transport_ch_cumulate,   //通道1 斜坡输送通道堆积
	
	
	PER_PROTUCE_ERR=177,   // 预生产产品错误
	PRODUCT_COMPLETE,  //产品生产完成
	CH1_L_CUP_SENSER_ERR,
	CH1_R_CUP_SENSER_ERR,
	CH2_L_CUP_SENSER_ERR,
	CH2_R_CUP_SENSER_ERR,
	CH3_L_CUP_SENSER_ERR,
	CH3_R_CUP_SENSER_ERR,
	CH4_L_CUP_SENSER_ERR,
	CH4_R_CUP_SENSER_ERR,
	CH5_L_CUP_SENSER_ERR,
	CH5_R_CUP_SENSER_ERR,
	CH6_L_CUP_SENSER_ERR,
	CH6_R_CUP_SENSER_ERR,
	
	MAST_MOTO_POSITION_ERR,  
	TICK_EGG_MOTO_ERROR,  
	CRACK_DEVICE_ERROR, //
	DIRTY_DEVICE_ERROR,
	COLOUR_DEVICE_ERROR,
	WEIGHT_DEVICE_ERROR ,
	THROW_CH_EGG_ERROR ,
	SISTME_PART_READ_ERROR ,   
	PACKGE_TRAY_ERR ,
	WEIGHT_SYNC_ERR,
	DIRTY_INFO_SYNC_ERR,      
	BLOOD_INFO_SYNC_ERR,
	RECIVE_SENSER_ERR,
	PACKGE1_TCP_ERR,
	PACKGE2_TCP_ERR,
	PACKGE3_TCP_ERR,
	PACKGE4_TCP_ERR,
	PACKGE5_TCP_ERR,
	PACKGE6_TCP_ERR,
	CHECK_PRODUCK_PACKGE_INFO_IS_OK,
	PACKGE_RECIVE1_ERR,
	PACKGE_RECIVE2_ERR,
	PACKGE_RECIVE3_ERR,
	PACKGE_RECIVE4_ERR,
	PACKGE_RECIVE5_ERR,
	PACKGE_RECIVE6_ERR,
	PACKGE_RECIVE7_ERR,
	PACKGE_RECIVE8_ERR,
	PACKGE_RECIVE9_ERR,
	PACKGE_RECIVE10_ERR,
	PACKGE_RECIVE11_ERR,
	PACKGE_RECIVE12_ERR,
	

	ALARM_COUNT,
}ALARM_TYPE;

uint16_t get_alarm_count();
void init_alarm_dat();
void clear_warn_alarm();
void alarm_loop_check();
void set_device_alarm(ALARM_TYPE alarm);
void clear_device_alarm(ALARM_TYPE alarm);
uint8_t get_device_alarm(ALARM_TYPE alarm);
#endif  	// _DEVICE_ALARM_H__
