
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "E6B2.h"
#include "drv_key.h"
#include "mdebug.h"
#include "common.h"
#include "ckeck_env.h"
#include "screen_chat.h"
#include "device_alarm.h"
#include "move_control.h"
#include "relay.h"
#include "app_key.h"
#include "app_screen.h"
#include "app_io_control.h"
#include "device_alarm.h"
#include "egg_control.h"

ALIGN(RT_ALIGN_SIZE)

static uint8_t sys_status;

// 复位按键检测
// 当上一次的复位按键为低电平，本次检测到复位电平为高电平，说明复位按键被按下了
// 复位按键按下了，要在系统在空闲的时候才起作业，才能复位
void reset_key_fun()
{
  static uint8_t up_reset_key=0;
	uint8_t reset_key=0;
	
	reset_key = ge_key_val(RESET_KEY);
  if((reset_key ==1)&&(up_reset_key !=1))
	{
		if(sys_status ==SYSTEM_IS_IDLE)
		{
			reset_main_moto_to_origin();
			sys_status = SYSTEM_IS_RESET;
			DEBUG_CARE("主电机复位=%d\n",up_reset_key);
		}
	}
	up_reset_key = reset_key;
}


// 启动按键
void start_key_fun()
{
	static uint8_t up_start_key=0;
	uint8_t start_key=0;
	uint16_t c_page;
	
	// 启动按键被按下并且上一次的启动按键的状态为没有按下(意思就是说明启动按下了并且不能重复操作)
	start_key = ge_key_val(START_KEY);
	if((up_start_key != start_key)||(start_key ==PRESS))   //启动按键
	{
		c_page = get_current_screen_page();

			// 57和108都包装通道的调试界面
			if(((c_page ==57)||(c_page == 108))&&(sys_status ==SYSTEM_IS_IDLE))    // 57 和 108 界面按下启动键都是 调试功能
			{
				ext_key_debug_device(start_key);
			}
			// 52分级线调试
			else if((c_page ==52)&&(sys_status ==SYSTEM_IS_IDLE))   //主电机和推杆调试
			{
				debug_mast_control_moto(start_key);
			}
			// 除了上面3个调试界面可以动，其他调试界面按开始按键不动
			else if((((c_page>49)&&(c_page<63))||(c_page == 108))&&(sys_status ==SYSTEM_IS_IDLE))  //调试模式
			{
		
			}
			// 其他不是调试界面时候，按下启动按键
			else if((up_start_key !=PRESS)&&(start_key ==PRESS))   //设备启动
			{
				DEBUG_CARE("system  status=%d ",sys_status);
				// 如果设备在空闲按下启动，则清除报警并且开始检查环境
				if(sys_status ==SYSTEM_IS_IDLE)
				{
					start_clear_warn_alarm();
					rt_thread_mdelay(100);
					set_check_env_step(0);
					sys_status = SYS_IS_CHECK_ENV;
				}
				// 如果是运行的时候按下启动，则设备停止
				else if(sys_status ==SYSTEM_IS_RUING)   
				{
					set_check_env_step(0);
					sys_status = SYSTEM_IS_STOP;
					set_expansion_link_move(2,0);	// 伸缩杆是往哪里搞，是伸长还是缩短
				}
				// 在检查环境状态则进入空闲状态
				else if(sys_status ==SYS_IS_CHECK_ENV)   
				{
					set_check_env_step(0);
					sys_status = SYSTEM_IS_IDLE;
					set_ext_device_stop();	// 设置停机时间，然后其他要延迟停机的设备，在这个停机时间的基础上延迟相应的时间在停机
				}
				DEBUG_CARE("system run status=%d\n",sys_status);
			}
	}
	up_start_key = start_key;
}
// 紧急停机按键
extern void close_throw_all_pin();
extern void transport_ch_emergency_stop();
static rt_tick_t stop_time=0;
void emergency_key_fun()
{
   uint8_t emergency_key;
   uint32_t press_time=0;
   emergency_key = ge_key_val(EMERGENCY_KEY);
   if(emergency_key ==0)
	 {
		 // 检测到紧急停机按键按下(平时是接通即高电平，按下的时候是低电平，和其他按键是反的)
		 // 记录紧急停机按键按下时间，大于100ms
	     press_time = count_use_time(stop_time);
		 if(press_time>100)
		 {
			 // 大于100ms则让设备进入到紧急停机状态
			 // 并设置紧急停机报警
			 // 
			sys_status = SYSTEM_IS_EMERGENCY_STOP;	// 系统紧急停机状态，在状态机里面，主控会发信息给包装通道，实际中我看包装通道也都关电了
			set_device_alarm(EMERGENCY_STOP);
			close_throw_all_pin();					// 关闭抛弃通道的所有io
			transport_ch_emergency_stop();	// 关闭传输线的所有的io
			stop_time = rt_tick_get();
		 }
		 return ;
	 }
	 else 
	 {
		 // 这个会不断地记录紧急按键没有按下的时刻
		 // 没有按下了说明恢复了或者本身就是正常的，紧急报警就要清除
		 // 紧急按键恢复了，如果上一个状态是紧急状态，恢复后让设备进入到空闲状态
		 stop_time = rt_tick_get();
		 clear_device_alarm(EMERGENCY_STOP);
		 if(sys_status == SYSTEM_IS_EMERGENCY_STOP)  //接触报警
		 {
			 sys_status = SYSTEM_IS_IDLE;
		 }
	 }
}

// 查找第一个蛋杯
#define SENSER_POSITON_OFFEST (SEPARATE_CUP_NUMBER-12)
uint8_t find_first_cup_flag=0;
void number_one_cup_find()
{

   uint8_t key;
	 uint16_t first_cup,buf;
	 uint32_t pluse,offest_pluse;
	 uint32_t cup_pluse;
   key = ge_key_val(NUMBER_ONE_CUP_KEY);
   separate_egg_info_type *egg_info;
	
   cup_pluse =((SEPARATE_CHA_CUP_NUMBER/6)*1000);
	// 如果没有找到第一个蛋杯
   if(find_first_cup_flag ==0)	
	 {
		 // 检测到了第一个蛋杯的信号
		 if(key>0)
		 {
			 // 当前第一个蛋杯脉冲在分级线一圈蛋杯数的脉冲中的位置(即第一个蛋杯脉冲相对于整个蛋杯脉冲的位置，或者理解为第一个蛋杯在整个蛋杯的位置)
			 find_first_cup_flag =1;
			 offest_pluse = get_spt_encode_pluse();
			 offest_pluse = offest_pluse%cup_pluse;		// 这个得到，此时到开机的时候走的位置在一圈中的相对位置
				
			 
			 
			// first_cup = pluse/1000;
			// first_cup = first_cup*6;
			// buf = pluse%1000;
			 //buf = buf/166;
			// first_cup += buf;
			 
			 // 分级线总蛋杯数走一圈的脉冲
			 pluse = SEPARATE_CHA_CUP_NUMBER*16666;
			 pluse = pluse/100;
			 
			 // 如果第一个蛋杯的脉冲位置在整个蛋杯脉冲范围内
			 if(offest_pluse<pluse)
			 {
				 offest_pluse = pluse - offest_pluse;
				 offest_pluse = offest_pluse*100;
				 first_cup = offest_pluse/16666;
				 //first_cup = SENSER_POSITON_OFFEST - first_cup;
			 }
			 else 
			 {
				 offest_pluse = offest_pluse - pluse;
				 offest_pluse = offest_pluse*100;
				 first_cup = offest_pluse/16666;
				 first_cup = SEPARATE_CHA_CUP_NUMBER + first_cup;
				 first_cup -=3;
			 }
			 WriteMBHotRegU16(4399,first_cup);
			// DEBUG_WARN("first key val =%d pluse=%d cup id=%d\n",key,get_spt_encode_pluse(),first_cup);
		 }
	 }
		pluse = get_spt_encode_pluse();
		if((pluse>120000)&&(find_first_cup_flag==0))
		{
			//set_device_alarm(NOT_HAVE_CHECK_NUMBER_ONE_EGG);
		}
		else 
		{
			 clear_device_alarm(NOT_HAVE_CHECK_NUMBER_ONE_EGG);
		}
}
// 不停地扫描这3个按键，并处理这3个按键的状态
void user_key_scan()
{
	uint8_t state;
	state = get_system_state();
	sys_status = state;
	start_key_fun();
	reset_key_fun();
	emergency_key_fun();
	if(state != sys_status)
	{
	  set_system_state(sys_status);
		rt_thread_mdelay(50);
		expansion_link_stop();
	}
	
}
