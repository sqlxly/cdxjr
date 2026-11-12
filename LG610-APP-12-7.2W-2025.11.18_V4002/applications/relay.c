
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_key.h"
#include "relay.h"
#include "mdebug.h"
#include "relay.h"
#include "E6B2.h"

#define STOP 0
#define UP 1
#define DOWN 2



static uint8_t expansion_link_state=10;

//伸缩杆下降 
// 继电器5.6联合来控制伸缩杆电机正反转
// 继电器6低，继电器5高是缩短
// 继电器6高，继电器5低是伸长
void expansion_link_down()
{
	if(expansion_link_state !=DOWN)
	{
		expansion_link_state = DOWN;
	  rt_pin_write(RELAY6,0);
		rt_thread_mdelay(30);
		rt_pin_write(RELAY5,1);
//		DEBUG_CARE("伸缩杆收缩 =%d\n",rt_pin_read(RELAY6));
	}
}

//伸缩杆上升
void expansion_link_up()
{
	if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)	// 如果到达了最大的位置，就要停止，不能走了
	{
			expansion_link_stop();
		  return;
	}
	if(expansion_link_state!=UP)
	{
		expansion_link_state = UP;
	  rt_pin_write(RELAY5,0);
		rt_thread_mdelay(30);
		rt_pin_write(RELAY6,1);
		//DEBUG_WARN("限位状态=%d\n",ge_key_val(EXPONSION_LINK_TOP_SENSER));
		//DEBUG_CARE("伸缩上升 =%d\n",rt_pin_read(RELAY6));
	}
}
//伸缩杆停止
void expansion_link_stop()
{
	if(expansion_link_state!=STOP)
	{
		expansion_link_state = STOP;
	  rt_pin_write(RELAY5,0);
		rt_pin_write(RELAY6,0);
	//	DEBUG_CARE("伸缩杆停止 =%d\n",rt_pin_read(RELAY6));
	}
}
uint8_t frequency_moto_disable()
{
	//rt_pin_write(RELAY1,0);       //为什么要注销 因为电机使能的时候对编码器有干扰,解决硬件干扰
}
uint8_t frequency_moto_enable()
{
  rt_pin_write(RELAY1,1);	// 继电器控制主电机的变频器的启停信号的
}

static uint8_t en_print_flag[2]={0,0};
// 设置喷码使能
// 在蛋控制里面，蛋喷码函数会检查来的蛋满不满足喷码的要求，满足的话就会使能喷码
void set_en_print_egg(uint8_t ch)
{ 
	if(ch>1) return ;
  rt_base_t level;
  level = rt_hw_interrupt_disable();
	en_print_flag[ch] = 1;
	rt_hw_interrupt_enable(level);	
}

uint32_t print_egg_tick[2]={0,0};
uint8_t print_falg[2]={0,0};
extern uint32_t print_count[2];
// 喷码使能检查，如果使能了则要使用喷码
// 主函数会不停地检查有没有要喷码的，有的话就会执行喷码操作
// 喷码操作很简单，只需要控制该喷码机的继电器就可以
void check_en_print()
{
	uint8_t flag1,flag2;
  rt_base_t level;
	uint32_t pluse;
	uint32_t use_time=0;
	level = rt_hw_interrupt_disable();
	flag1 = en_print_flag[0];
	flag2 = en_print_flag[1];
	rt_hw_interrupt_enable(level);
	level = rt_hw_interrupt_disable();
	if(flag1>0)
	{
		  print_falg[0]=1;
			en_print_flag[0] = 0;
		 // rt_pin_write(PRINT1_EN,1);
		  print_egg_tick[0]=rt_tick_get();
	}
	if(flag2>0)
	{
		print_falg[1]=1;
		en_print_flag[1] = 0;
		//rt_pin_write(PRINT2_EN,1);
		print_egg_tick[1]=rt_tick_get();
	}
	rt_hw_interrupt_enable(level);
  if(print_falg[0]>0)	// 使能了喷码的话
	{
		
		use_time = count_use_time(print_egg_tick[0]);
		if((use_time>20)&&(use_time<40))
		{
			
		  rt_pin_write(PRINT1_EN,1);	// 开启喷码
		}
		else if(use_time>40)					// 到了一定时间则关闭喷码，并记录已经喷码的个数
		{
		   rt_pin_write(PRINT1_EN,0);
			 print_falg[0]=0;
			 print_count[0]++;
		}
	}
	if(print_falg[1]>0)
	{
		
		use_time = count_use_time(print_egg_tick[1]);
		if((use_time>20)&&(use_time<40))
		{
			
		  rt_pin_write(PRINT2_EN,1);	// 开启喷码
		}
		else if(use_time>40)		// 到了一定时间则关闭喷码，并记录已经喷码的个数
		{
			 print_count[1]++;
		   rt_pin_write(PRINT2_EN,0);
		   print_falg[1]=0;
		}
	}
}
// 通过引脚来设置该打印哪个喷码数据id，喷码机会读取这些引脚，从而组合出需要喷码数据的id？
void set_print_id(uint8_t ch,uint8_t print_id)
{
	//rt_kprintf("id=%d\n",print_id);
	
	print_id = print_id&0x0f;
	if(ch==0)
	{
			if(print_id &0x01)
			{
				rt_pin_write(PRINT_IO0,1);
			}
			else 
			{
				rt_pin_write(PRINT_IO0,0);
			}
	
			if(print_id & 0x02)
			{
				rt_pin_write(PRINT_IO1,1);
			}
			else 
			{
				rt_pin_write(PRINT_IO1,0);
			}

			if(print_id & 0x04)
			{
				rt_pin_write(PRINT_IO2,1);
			}
			else 
			{
				rt_pin_write(PRINT_IO2,0);
			}

			if(print_id & 0x08)
			{
				rt_pin_write(PRINT_IO3,1);
			}
			else 
			{
				rt_pin_write(PRINT_IO3,0);
			}
	}
	else 
	{
	  if(print_id & 0x01)
		{
			rt_pin_write(PRINT_IO5,1);
		}
		else 
		{
			rt_pin_write(PRINT_IO5,0);
		}
	
		if(print_id & 0x02)
		{
			rt_pin_write(PRINT_IO6,1);
		}
		else 
		{
			rt_pin_write(PRINT_IO6,0);
		}
	
		if(print_id & 0x04)
		{
			rt_pin_write(PRINT_IO7,1);
		}
		else 
		{
			rt_pin_write(PRINT_IO7,0);
		}
	
		if(print_id & 0x08)
		{
			rt_pin_write(PRINT_IO8,1);
		}
		else 
		{
			rt_pin_write(PRINT_IO8,0);
		}
	}
}
// 喷码设备的喷码数据id设置引脚的初始化
void print_io_init()
{
   rt_pin_write(PRINT_IO0,0);
	 rt_pin_write(PRINT_IO1,0);
	 rt_pin_write(PRINT_IO2,0);
	 rt_pin_write(PRINT_IO3,0);
	 rt_pin_write(PRINT1_EN,0);
	 rt_pin_write(PRINT_IO5,0);
	 rt_pin_write(PRINT_IO6,0);
	 rt_pin_write(PRINT_IO7,0);
	 rt_pin_write(PRINT_IO8,0);
	 rt_pin_write(PRINT2_EN,0);
	
	 rt_pin_mode(PRINT_IO0, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO1, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO2, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO3, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT1_EN, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO5, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO6, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO7, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT_IO8, PIN_MODE_OUTPUT);
	 rt_pin_mode(PRINT2_EN, PIN_MODE_OUTPUT);
}
// 继电器引脚的初始化
rt_err_t relay_init()
{
	print_io_init();
	rt_pin_write(RELAY1,0);
	rt_pin_write(RELAY2,0);
	rt_pin_write(RELAY3,0);
	rt_pin_write(RELAY4,0);
	rt_pin_write(RELAY5,0);
	rt_pin_write(RELAY6,0);
	rt_pin_mode(RELAY1, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY2, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY3, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY4, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY5, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY6, PIN_MODE_OUTPUT);
	expansion_link_stop();
	return RT_EOK;
	
}


