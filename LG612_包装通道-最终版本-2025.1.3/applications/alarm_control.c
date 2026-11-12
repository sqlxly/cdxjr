
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "alarm_control.h"
#include "servo_common.h"

static uint32_t ch1_alarm=0;
static uint32_t ch2_alarm=0;

void set_alarm(uint8_t ch,CONTROL_ALARM_TYPE type,uint8_t val)
{
	uint32_t falg=0;
	rt_base_t level;
	
	falg = 1<<type;
	level = rt_hw_interrupt_disable();
	if(ch==CH1_DEVICE_ID)
	{
		if(val>0)
			ch1_alarm |= falg;
		else 
			ch1_alarm  &= (~falg);
	}
	else if(ch==CH2_DEVICE_ID)
	{
		//rt_kprintf("flag=0x%08x  val=%d\n",falg,val); 
		if(val>0)
			ch2_alarm |= falg;
		else 
			ch2_alarm  &= (~falg);
	}
	rt_hw_interrupt_enable(level);
}
uint8_t get_alarm(uint8_t ch,CONTROL_ALARM_TYPE type)
{
	uint8_t result;
	uint32_t falg=0;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	falg = 1<<type;
	if(ch==CH1_DEVICE_ID)
	{
		result = (ch1_alarm & falg)&&1;
	}
	else if(ch==CH2_DEVICE_ID)
	{
		result = (ch2_alarm & falg)&&1;
	}
	rt_hw_interrupt_enable(level);
	return result;
}
uint32_t get_control_alarm(uint8_t device)
{
	uint32_t dat;
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	if(device ==CH1_DEVICE_ID)
	{
		dat =  ch1_alarm;
	}
	else if(device ==CH2_DEVICE_ID)
	{
		dat = ch2_alarm;
	}
	rt_hw_interrupt_enable(level);
	
	return dat;
}

uint32_t clear_control_alarm(uint8_t device)
{
	uint32_t dat;
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	if(device ==CH1_DEVICE_ID)
	{
	   ch1_alarm=0;
	}
	else if(device ==CH2_DEVICE_ID)
	{
		ch2_alarm =0;
	}
	rt_hw_interrupt_enable(level);
	
	return dat;
}