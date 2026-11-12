#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <user_set.h>
#include "app_screen.h"
#include "device_alarm.h"
#include "modbus_packge.h"
#include "modbus_throw.h"
#include "modbus_transport.h"
#include "err_check.h"
#include "sync_egginfo.h"
#include "common.h"
#include "mdebug.h"
ALIGN(RT_ALIGN_SIZE)

static uint8_t alarm_dat[ALARM_COUNT+1];
extern void clear_blood_com_err_pluse();
void set_device_alarm(ALARM_TYPE alarm)
{
	if(alarm<ALARM_COUNT)
		alarm_dat[alarm]=1;
}

uint8_t get_device_alarm(ALARM_TYPE alarm)
{
	if(alarm<ALARM_COUNT)
		return alarm_dat[alarm];
	else return 0;
}
void clear_device_alarm(ALARM_TYPE alarm)
{
	if(alarm<ALARM_COUNT)
		alarm_dat[alarm]=0;
}



void clear_warn_alarm()
{
	
	
	
	clear_device_alarm(PACKGE1_TCP_ERR);
	clear_device_alarm(PACKGE2_TCP_ERR);
	clear_device_alarm(PACKGE3_TCP_ERR);
	clear_device_alarm(PACKGE4_TCP_ERR);
	clear_device_alarm(PACKGE5_TCP_ERR);
	clear_device_alarm(PACKGE6_TCP_ERR);
	clear_device_alarm(CHECK_PRODUCK_PACKGE_INFO_IS_OK);
	
	clear_device_alarm(THROW_AWAY_BOARD1_COM_ERR);
	clear_device_alarm(THROW_AWAY_BOARD2_COM_ERR);
	clear_device_alarm(TRANSPORT_CH_COM_ERR);
	clear_device_alarm(PACKGE_BOARD1_COM_ERR);
	clear_device_alarm(PACKGE_BOARD2_COM_ERR);
	clear_device_alarm(PACKGE_BOARD3_COM_ERR);
	clear_device_alarm(PACKGE_BOARD4_COM_ERR);
	clear_device_alarm(PACKGE_BOARD5_COM_ERR);
	clear_device_alarm(PACKGE_BOARD6_COM_ERR);
	clear_device_alarm(LIGHT_CH_ERR1);
	clear_device_alarm(LIGHT_CH_ERR2);
	clear_device_alarm(MAST_MOTO_POSITION_ERR);
    clear_device_alarm(PER_PROTUCE_ERR);
	clear_device_alarm(RECIVE_SENSER_ERR);
	clear_device_alarm(PRODUCT_COMPLETE);
	clear_device_alarm(TRANSPORT_AND_SPT_LINE_SYNC_ERR);
	clear_device_alarm(TICK_EGG_MOTO_ERROR);
	clear_device_alarm(NOT_HAVE_CHECK_NUMBER_ONE_EGG);
	clear_device_alarm(EGG_CUP_ERR);
	clear_device_alarm(WEIGHT_DEVICE_ERROR);
	clear_device_alarm(CRACK_DEVICE_ERROR);
	clear_device_alarm(DIRTY_DEVICE_ERROR);
	clear_device_alarm(COLOUR_DEVICE_ERROR);
	clear_device_alarm(BLOOD_INFO_SYNC_ERR);
	clear_device_alarm(THROW_CH_EGG_ERROR);
	clear_device_alarm(PACKGE_TRAY_ERR);
	clear_device_alarm(DIRTY_INFO_SYNC_ERR);
	clear_device_alarm(BLOOD_INFO_SYNC_ERR);
	clear_device_alarm(WEIGHT_SYNC_ERR);
	clear_device_alarm(EXPANSION_LINK_ERR);
	clear_throw_ch_egg_err_count();
	clear_blood_com_err_pluse();
	clear_device_alarm(RECIVE_SENSER_ERR);
	for(int i=0;i<ALARM_COUNT;i++)
	{
	  clear_device_alarm(i);
	}
	//clear_dirty_and_wieght_alarm();
}
static uint16_t g_alarm_count=0;



void check_transport_alarm()
{
  transport_io_dat *transport_io;
	
	if(get_transport_ch_state()>0)   //输送通道
	{
		//clear_device_alarm(LIGHT_CH_ERR);  //关闭
	}
  else
	{
		transport_io = get_transport_io_info();
		if(transport_io->input.gpio[light_ch1_unnormal] ==0)
		{
			set_device_alarm(LIGHT_CH_ERR1);
		}	
    if(transport_io->input.gpio[light_ch2_unnormal]==0)
		{
			set_device_alarm(LIGHT_CH_ERR2);
		}				
	}
}

void init_alarm_dat()
{
 rt_memset(alarm_dat,0,ALARM_COUNT);
}

static rt_tick_t cycle_time=0;
static rt_tick_t packge_tcp_state_err_count[6]={0,0,0,0,0,0};
static rt_tick_t throw_tcp_state_err_count[2]={0,0};


void set_packet_control_alarm(uint8_t i,uint32_t alarm)
{

  uint8_t j;
	
	//rt_kprintf("i=%d alarm=%04x\n",i,alarm);
	for(j=0;j<9;j++)
	{
	    if(alarm &0x01)
		{
						set_device_alarm(ch1l_transport_server_com_err+(i*13)+j);
		}
		else 
		{
						clear_device_alarm(ch1l_transport_server_com_err+(i*13)+j);
		}
		alarm=alarm>>1;
	}
    alarm=alarm>>6;
	
	if(alarm &0x01)
	{
		set_device_alarm(ch1l_auto_send_tray_moto_err+(i*13));  
	}
	else 
	{
		clear_device_alarm(ch1l_auto_send_tray_moto_err+i*13);
	}
	alarm=alarm>>1;	
	
	for(j=0;j<3;j++)
	{
		if(alarm &0x01)
		{
						set_device_alarm(ch1l_slope_transport_moto_err+(i*13)+j);
		}
		else 
		{
						clear_device_alarm(ch1l_slope_transport_moto_err+(i*13)+j);
		}
		alarm=alarm>>1;
	}

	if(alarm &0x01)
	{
		set_device_alarm(CH1_L_CUP_SENSER_ERR+i);
	}
	else 
	{
		clear_device_alarm(CH1_L_CUP_SENSER_ERR+i);
	}
		
	alarm=alarm>>1;
	if(alarm &0x01)   
	{
			set_device_alarm(PACKGE_TRAY_ERR);
	}
}
extern void check_light_err();
void alarm_loop_check()
{
  uint16_t use_flag=0;
	uint16_t i,j;
	uint16_t alarm_count;
	uint32_t alarm;
	uint32_t ctime;
	rt_tick_t disctime=0;
	user_set_part_type *user_set;
	get_user_set_point(&user_set);
	
	ctime = count_use_time(cycle_time); 
	
	if(ctime>100)
	{
		check_light_err();
	  ctime =rt_tick_get();
     alarm_count =0;
	// check_dirty_and_weight_device_alarm();
		for(i=0;i<12;i++)   //包装通道报警标志
		{
			use_flag = get_packge_ch_config(i);
			alarm = get_packge_control_alarm(packge_map_ch_id[i]);
			if(use_flag==0) alarm =0;
            set_packet_control_alarm(packge_map_ch_id[i],alarm);
		}
		check_transport_alarm();

		for(i=0;i<2;i++)
	  {
			if((get_throw_ch_state(i)>0)&&(get_throw_device_en_flag(i)>0))   //抛弃通道1
			{
				disctime =  count_use_time(throw_tcp_state_err_count[i]);
				if(disctime>(user_set->commun_part.reconnect_timeou_number))
				{
					set_device_alarm(THROW_AWAY_BOARD1_COM_ERR+i);
					// DEBUG_ERR("包装通道断网时间=%d\n",disctime);
					throw_tcp_state_err_count[i]= rt_tick_get();
				}
			}
			else 
			{
			  throw_tcp_state_err_count[i]= rt_tick_get();
			}
		}
	  for(i=0;i<6;i++)
	  {
      if((get_packge_control_mb_state(i)>0)&&(check_mb_en_flag(i)>0))
			{
				
				disctime =  count_use_time(packge_tcp_state_err_count[i]);
				if(disctime>(user_set->commun_part.reconnect_timeou_number))
				{
					set_device_alarm(PACKGE_BOARD1_COM_ERR+i);
					// DEBUG_ERR("包装通道断网时间=%d\n",disctime);
					packge_tcp_state_err_count[i]= rt_tick_get();
				}
			}
			else 
			{
				packge_tcp_state_err_count[i]= rt_tick_get();
			}
	  }
	  for(i=0;i<ALARM_COUNT;i++)
		{
			if(alarm_dat[i]>0)
			{
				disp_alarm(i,1);
				alarm_count++;
			}
			else 
			{
				disp_alarm(i,0);
			}
		}
		g_alarm_count = alarm_count;
		if(alarm_count>0)
	   set_alarm_flag(1);
		else 
		 set_alarm_flag(0);
	}
}
uint16_t get_alarm_count()
{
	 return g_alarm_count;
}


