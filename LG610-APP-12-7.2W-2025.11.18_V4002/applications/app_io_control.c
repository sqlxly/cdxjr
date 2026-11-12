
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "mdebug.h"
#include "common.h"
#include <user_set.h>
#include <mail_box.h>
#include "relay.h"
#include "screen_chat.h"
#include "app_io_control.h"
#include "modbus_transport.h"

//清洗毛刷  OK
//鼓风-> 干燥风机
typedef enum 
{
	 THROW_DEVICE_ID =0,    //强制抛弃 传输电机
   WASH_DEVICE_ID,
	 DRY_DEVICE_ID,
	 LIGHT_DEVICE_ID,
	 UV_LAMP_DEVICE_ID,   //紫外杀菌
	 PRINT_OIL_DEVICE_ID,
	 ABSORB_OIL_DEVICE_ID,
	 DELAY_TIME_MAX_ID,
}EXT_DELAY_ID;
static uint8_t excuse_flag=0;

static rt_tick_t stop_delay_time[DELAY_TIME_MAX_ID+1];
static uint8_t  stop_flag[DELAY_TIME_MAX_ID+1];
void set_ext_device_start()
{
	uint8_t i;
  excuse_flag =1;
  for(i=0;i<DELAY_TIME_MAX_ID;i++)
	{
		stop_flag[i] =0;
	}
}
void set_ext_device_stop()
{
	uint8_t i;
	if(excuse_flag>0)  //设备开启的时候关闭
	{
		excuse_flag =2;
		for(i=0;i<DELAY_TIME_MAX_ID;i++)
		{
			stop_flag[i] =1;
			stop_delay_time[i]= rt_tick_get();
		}
	}
}



void throw_moto_tontrol(uint8_t dat)
{
	
	uint8_t result;
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	rt_memset(message_buf,0,sizeof(message_buf));
	
	message_buf[0]=1;      //命令
	message_buf[1]=7;     //电机驱动 第7路
	message_buf[2]=dat;    // 开关
	message_buf[3] = ALWAYS_TYPE;
	send_message_to_throw_device(0,message_buf); 
	send_message_to_throw_device(1,message_buf); 
}


void transport_ch_emergency_stop()
{

    set_transport_io(WASH_BRUSH,0);
    set_transport_io(ADD_MEDICINE,0);
	set_transport_io(WASH_MOTO,0);
	set_transport_io(WASH_FAN,0);
	set_transport_io(DRY_FAN,0);
	set_transport_io(DRY_BRUSH,0);
	set_transport_io(OIL_PRINT,0);
	set_transport_io(OIL_ABSORB,0);
	set_transport_io(UV_LAMP,0);

}
void ext_device_start()
{
	user_set_part_type *dat;
	
	dat= rt_malloc(sizeof(user_set_part_type));
	if(dat ==RT_NULL)
		return;
	get_user_set_dat(dat);
	
	throw_moto_tontrol(1);
	if(dat->device_use_info.dry_system>0)   //风干系统硬件安装
	{
	   if(dat->mode_en.dry_brush>0)  //毛刷启用
		 {
		   set_transport_io(DRY_BRUSH,1);
		 }
		 if(dat->mode_en.dry_fan>0)  //毛刷启用
		 {
		   set_transport_io(DRY_FAN,1);
		 }
	}
	if(dat->device_use_info.wash_system>0)   //清洗系统硬件安装
	{
	   if(dat->mode_en.wash_brush>0)  //清洗毛刷
		 {
		   set_transport_io(WASH_BRUSH,1);
		 }
		 if(dat->mode_en.wash_fan>0)  //鼓风机
		 {
		   set_transport_io(WASH_FAN,1);
		 }
		 if(dat->mode_en.wash_moto>0)  //清洗水泵
		 {
		   set_transport_io(WASH_MOTO,1);
		 }
	}
	if(dat->device_use_info.uv_lamp>0)   //紫外杀菌
	{
		if(dat->mode_en.uv_lamp>0)
		{
			set_transport_io(UV_LAMP,1);
		}
	}
	if(dat->device_use_info.oil_system)  //喷油
	{
	  if(dat->mode_en.oil_print>0)
		{
			set_transport_io(OIL_PRINT,1);
		}
		if(dat->mode_en.absorb_oil>0)
		{
			set_transport_io(OIL_ABSORB,1);
		}
	}
	
	if(dat->device_use_info.auto_add_medicine)  //加药
	{
	  if(dat->mode_en.add_medicine>0)
		{
			set_transport_io(ADD_MEDICINE,1);
		}
	}
	
	if(dat->device_use_info.human_eye_recognize>0) 
	{
	  if(dat->mode_en.light_lamp>0)
		{
			//set_transport_io(LED_LIGHT,1);
		}
	}
	rt_free(dat);

}

uint8_t ext_device_stop()
{
	uint8_t i;
	uint8_t result=0;
	user_set_part_type *dat;
	uint32_t run_time;
	
	dat= rt_malloc(sizeof(user_set_part_type));
	if(dat ==RT_NULL)
		return 2;
	get_user_set_dat(dat);
  set_transport_io(OIL_PRINT,0);  //喷油
	run_time = count_use_time(stop_delay_time[THROW_DEVICE_ID]);
	if(run_time>3000)
	{
		if(stop_flag[THROW_DEVICE_ID] ==1)
		{
			throw_moto_tontrol(0);
			stop_flag[THROW_DEVICE_ID]=0;
		}
	}
	else                      //未完成 延时未到
	{
	  result++;
	}

	if((dat->device_use_info.wash_system>0))  //清洗启用
	{
	  run_time = count_use_time(stop_delay_time[WASH_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.wash_delay)
		{
		  if(stop_flag[WASH_DEVICE_ID] ==1)
			{
				if(dat->mode_en.wash_brush>0)  //毛刷启用
				{
					set_transport_io(WASH_BRUSH,0);
				}
				if(dat->mode_en.wash_moto>0)  //毛刷启用
				{
					set_transport_io(WASH_MOTO,0);
				}
				if(dat->mode_en.wash_fan>0)  //鼓风机
				{
					set_transport_io(WASH_FAN,0);
				}
				
				if(dat->device_use_info.auto_add_medicine)  //加药
				{
					if(dat->mode_en.add_medicine>0)
					{
						set_transport_io(ADD_MEDICINE,0);
					}
				}
			  stop_flag[WASH_DEVICE_ID] =0;
			}
		}
		else                                 //未完成 延时未到
		{
		   result++;
		}
	}
	
	if((dat->device_use_info.dry_system>0))  //干燥
	{
	  run_time = count_use_time(stop_delay_time[DRY_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.dry_delay)
		{
		  if(stop_flag[DRY_DEVICE_ID] ==1)
			{
				if(dat->mode_en.dry_brush>0)  //干燥毛刷
				{
					set_transport_io(DRY_BRUSH,0);
				}
				if(dat->mode_en.dry_fan>0)  //干燥风机
				{
					set_transport_io(DRY_FAN,0);
				}
			  stop_flag[DRY_DEVICE_ID] =0;
			}
		}
		else                                 //未完成 延时未到
		{
		   result++;
		}
	}
	
	if(dat->device_use_info.uv_lamp>0)   //紫外杀菌
	{
		run_time = count_use_time(stop_delay_time[UV_LAMP_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.uv_lamp_delay)
		{
			if(stop_flag[UV_LAMP_DEVICE_ID] ==1)
			{
				if(dat->mode_en.uv_lamp>0)
				{
					set_transport_io(UV_LAMP,0);
				}
				stop_flag[UV_LAMP_DEVICE_ID]=0;
			}
		}
		else 
		{
				result++;
		}
	}
	
	
	if(dat->device_use_info.oil_system>0)   //喷油系统
	{
		run_time = count_use_time(stop_delay_time[PRINT_OIL_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.print_oil_delay)
		{
			if(stop_flag[PRINT_OIL_DEVICE_ID] ==1)
			{
				if(dat->mode_en.oil_print>0)
				{
					
				}
				if(dat->mode_en.absorb_oil>0)
				{
					//set_transport_io(OIL_ABSORB,0); //吸油
				}
				stop_flag[PRINT_OIL_DEVICE_ID]=0;
			}
		}
		else 
		{
				result++;
		}
		
		run_time = count_use_time(stop_delay_time[ABSORB_OIL_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.absorb_oil_delay)
		{
			if(stop_flag[ABSORB_OIL_DEVICE_ID] ==1)
			{
				if(dat->mode_en.oil_print>0)
				{
					set_transport_io(OIL_ABSORB,0);
				}
				stop_flag[ABSORB_OIL_DEVICE_ID]=0;
			}
		}
		else 
		{
				result++;
		}
	}
	if(dat->device_use_info.human_eye_recognize>0)   //人工光检
	{
		run_time = count_use_time(stop_delay_time[LIGHT_DEVICE_ID])/1000;  //清洗ID
	  if(run_time > dat->transport.light_delay)
		{
			if(stop_flag[LIGHT_DEVICE_ID] ==1)
			{
				if(dat->mode_en.light_lamp>0)
				{
				//	set_transport_io(LED_LIGHT,0);
				}
				stop_flag[LIGHT_DEVICE_ID]=0;
			}
		}
		else 
		{
				result++;
		}
	}
	
	rt_free(dat);
	if(result>0)
	{
		result =2;
	}
	return result;
}

static uint8_t tast_complete_alarm=0;
rt_tick_t alarm_start_time=0;
void set_tast_complete_alarm()
{
	alarm_start_time =rt_tick_get();
  tast_complete_alarm =1;
}
void product_alarm()
{
  int run_time;
  if(tast_complete_alarm)
	{
	   run_time = count_use_time(alarm_start_time);
	   if((run_time%1000)<500)
		 {
		     rt_pin_write(RELAY2,1);
		 }
		 else 
		 {
		   rt_pin_write(RELAY2,0);
		 }
		 if(run_time>3000)
		 {
			 rt_pin_write(RELAY2,0);
		   tast_complete_alarm=0;
		 }
	}
}

void transport_io_control()
{
		product_alarm();
		if(excuse_flag==1)
		{
			ext_device_start();
		  excuse_flag=3;
		}
		if(excuse_flag ==2)
		{
		  excuse_flag = ext_device_stop();
		}
}
