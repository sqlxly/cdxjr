#include <stm32h7xx.h>
#include "dev_config.h"

MasterOperateTypeDef masterOperate; 		//系统运行状态全局变量

ProductionDataType proData;							//生产数据 用于统计和屏幕显示

uint8_t alarmConfirm = 0;								//报警确认按钮 按下则清除所有当前报警标志
uint8_t cupDetectFlag = 0;							//主轨道蛋杯闭合异常信号
uint8_t malpositionDetectFlag = 0;			//主轨道蛋杯鸡蛋位置异常
uint8_t	arrayDetectFlag	= 0;						//排列通道排列异常

uint8_t	alarmConfirmBtn	= RT_FALSE;			//报警确认按钮标志
uint16_t sorterUsedNumber = 20;							//当前安装的包装通道数量

uint8_t get_input_io_sta(uint8_t *iData, enum EnumInputIO e)
{
	return getbitv(iData[e/8], e%8);
}

void set_output_io_sta(uint8_t *oData, enum EnumOutputIO e, int8_t sta)
{
	if(1 == sta)
		setbitv(oData[e/8], e%8);
	else	
		clrbitv(oData[e/8], e%8);
}

rt_bool_t name_is_empty(uint16_t *nm, uint8_t len)
{
	uint8_t i=0;
	
	for(i=0; i<len; i++){
		if(nm[i] != 0) return RT_FALSE;
	}
	
	return RT_TRUE;
}

//数据比较
rt_bool_t compare_names(uint16_t *fn, uint16_t *en, uint8_t len)
{
	uint8_t i=0;
	
	for(i=0; i<len; i++){
		if(fn[i] != en[i]) return RT_FALSE;
	}
	
	return RT_TRUE;
}

//判断数据是否在某个区间内
rt_bool_t compare_range(int16_t rg1, int16_t rg2, int16_t real)
{
	if((real>=rg1)&&(real<=rg2)) return RT_TRUE;
	
	return RT_FALSE;
}

void led_and_beep_ctrl(enum EnumLedAndBeep color, uint8_t sta)
{
	switch(color)
	{
		case eYellow: //声光报警器黄色
		set_output_io_sta(masterOperate.outputData, eOutputIO47, 0);
		set_output_io_sta(masterOperate.outputData, eOutputIO46, 0);
		set_output_io_sta(masterOperate.outputData, eOutputIO45, sta);
			break;
		case eGreen: //声光报警器绿色
			set_output_io_sta(masterOperate.outputData, eOutputIO47, 0);
			set_output_io_sta(masterOperate.outputData, eOutputIO46, sta);
			set_output_io_sta(masterOperate.outputData, eOutputIO45, 0);
			break;
		case eRed:   //声光报警器红色
			set_output_io_sta(masterOperate.outputData, eOutputIO47, sta);
			set_output_io_sta(masterOperate.outputData, eOutputIO46, 0);
			set_output_io_sta(masterOperate.outputData, eOutputIO45, 0);
			break;
		default: break;
	}
}	
