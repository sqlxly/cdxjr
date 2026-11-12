#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <kick_egg.h>
#include "servo_control.h"
#include "mdebug.h"
#include "board_config.h"

#define CUP_CLOSE 0
#define CUP_OPEN  1
#define CUP_OPENING  2
//#define CUP_OPEN_time 30    //100MS




static kick_egg_dat kick_egg[EGG_CUP_NUMBER];
const uint8_t cup_pin[EGG_CUP_NUMBER]={OUT1,OUT2,OUT3,OUT4,OUT5,OUT6,OUT7,OUT8,OUT9,OUT10,OUT11,OUT12};

static uint8_t recive_cup_egg_buf[12]={0,0,0,0,0,0,0,0,0,0,0,0};

static uint8_t spt_speed=1;


static rt_tick_t more_time[2]={0,0};

void set_spt_speed(uint8_t speed)
{
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	spt_speed = speed;
	rt_hw_interrupt_enable(level);

}

uint8_t get_spt_speed()
{
	uint8_t speed;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	speed = spt_speed;
	rt_hw_interrupt_enable(level);
  return speed;
}

void clear_recive_cup_egg_buf(uint8_t packge_ch)
{
	rt_base_t level;
	uint8_t i;
	level = rt_hw_interrupt_disable();
	if(packge_ch==0)
	{
		for(i=0;i<6;i++)
		{
			recive_cup_egg_buf[i]=0;
		}
		more_time[0]=rt_tick_get();
	}
	else 
	{
		for(i=6;i<12;i++)
		{
			recive_cup_egg_buf[i]=0;
		}
		more_time[1]=rt_tick_get();
	}
	rt_hw_interrupt_enable(level);
	//rt_kprintf("clear cup eggs ch=%d\n",packge_ch);
	
}

void set_recive_cup_egg_buf(uint8_t ch)
{
	uint8_t dat;
	if(ch>11)
	{
		return;
	}
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	recive_cup_egg_buf[ch]=1;
	rt_hw_interrupt_enable(level);
}
// 没有调用
uint8_t get_recive_cup_egg_count(uint8_t ch)
{
	uint8_t i;
	uint8_t count;
	
	ch=ch*6;
	count =0;
  for(i=0;i<6;i++)
	{
	
	 count += recive_cup_egg_buf[ch+i];
	}
}
uint8_t get_recive_cup_egg_buf(uint8_t ch)
{
	uint8_t dat;
	if(ch>11)
	{
		return 0;
	}
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	dat = recive_cup_egg_buf[ch];
	rt_hw_interrupt_enable(level);
	return dat;
}
uint8_t servo_down_number[2]={0,0};
void set_servo_down_number(uint8_t ch,uint8_t count)
{
  if(ch>1)
		return;
	servo_down_number[ch] =(ch*6)+ count;
	DEBUG_CARE("serdown packge_ch=%d count=%d\n",ch,servo_down_number[ch]);
}
//解析运行模式踢蛋数据
void decode_run_egg_cup_dat(uint8_t *dat)
{
	 uint8_t ch,state;
	 uint8_t servo_down;
   uint16_t open_time;
   ch = dat[1]*6 +  dat[2];
	 state = dat[3]&&0x01;
	 open_time = dat[4]<<8;
	 open_time +=dat[5];
	 set_spt_speed(dat[6]);	
	 servo_down = dat[7];
	 if(ch<EGG_CUP_NUMBER)
	 {
		
		 //rt_kprintf("%d\n",ch);
		 //if(get_recive_cup_egg_buf(ch)==0)  //接受抓是空的 才接收
		 if(state)  //开关状态      开,是开才执行
		 {
			  // 电磁铁踢蛋
			  set_recive_cup_egg_buf(ch);
				kick_egg[ch].state=state;
				kick_egg[ch].open_time = open_time;
			 
			 
			 // 满了6个后，接收伺服下降标志会置为1
				if(servo_down==1)
				{
					set_ch1_recive_egg();
				}
				if(servo_down==2)
				{
					set_ch2_recive_egg();
				}
		 }
		 else
		 {
			 kick_egg[ch].state=state;
			 kick_egg[ch].open_time = 0;
				//DEBUG_ERR("RECIVE CUP HAVE EGG ch=%d dat=%d\n!!!!!!!!!!!!\n",ch,get_recive_cup_egg_buf(ch));
		 }
	 }		 
}

void decode_close_egg_cup_dat(uint8_t *dat)
{
	 uint8_t ch,state;
	 uint8_t servo_down;
	
   //message_buf[1]=ch%2;      //蛋带通道通道				  0-1
	 //message_buf[2]=spt_egg->tick_ch;   //蛋杯通道   0-5
	
	// dat[1]: 0,1
	// dat[2]:0-5
	// ch:0*6+(0-5),1*6+(0-5)
	
	
	
   ch = dat[1]*6 +  dat[2];
	 //state = dat[3]&&0x01;
	 servo_down = dat[7];
	
	 if(ch<EGG_CUP_NUMBER)
	 {
		 rt_pin_write(cup_pin[ch],0);
	 }		 
	 if(servo_down==1)
	 {
		 set_ch1_recive_egg();
	 }
	 if(servo_down==2)
	 {
		 set_ch2_recive_egg();
	 }
}
//解析调试模式蛋杯状态
void decode_debug_egg_cup_dat(uint8_t *dat)
{
	 uint8_t ch,state;
   uint16_t run_time;
   ch = dat[1]*6 +  dat[2];
	 state = dat[3]&&0x01;
	 if(ch<EGG_CUP_NUMBER)
	 {
		 kick_egg[ch].state =0;
		 rt_pin_write(cup_pin[ch],state);
	 }
	 
	 if(state==0)
	 {
		 run_time = count_use_time(kick_egg[ch].start_time);
		  rt_kprintf("io close ch=%d start time=%d run=%d\n",ch,kick_egg[ch].start_time,run_time);	
	 }		 
	 
}

void kick_egg_pin_init()
{
	uint8_t i;
	
	for(i=0;i<EGG_CUP_NUMBER;i++)
	{
		rt_pin_mode(cup_pin[i],PIN_MODE_OUTPUT);
		rt_pin_write(cup_pin[i],0);
	}
}

#include "ext_gpio.h"
#include "alarm_control.h"
#include "servo_common.h"
#include "mdebug.h"


void disp_packge_more_time(uint8_t ch)
{

 if(ch==5)
 {
		//	rt_kprintf("ch1 time=%d\n",rt_tick_get()-more_time[0]);
	}
  else if(ch==11)
	{
		//	rt_kprintf("ch2 time=%d\n",rt_tick_get()-more_time[1]);
	}


}
static void kick_egg_thread(void *args)
{
		uint8_t i;
	  rt_tick_t otick,ctick;
	  
		kick_egg_pin_init();
    rt_memset(kick_egg,0,sizeof(kick_egg_dat)*EGG_CUP_NUMBER);
	  otick = rt_tick_get();
		while(1)
		{
			for(i=0;i<EGG_CUP_NUMBER;i++)
			{
				if(kick_egg[i].state ==CUP_OPEN)
				{
//					disp_packge_more_time(i);
					kick_egg[i].state=CUP_OPENING;
					kick_egg[i].start_time=rt_tick_get();							// 记录踢蛋的时刻
					rt_pin_write(cup_pin[i],1);
				}
				else if(kick_egg[i].state ==CUP_OPENING)
				{
					otick  = count_use_time(kick_egg[i].start_time);	// 根据踢蛋的时刻计算已经打开了的时间
				  if(otick>=kick_egg[i].open_time)	// 踢蛋时间大于设置的打开时间
					{
						rt_pin_write(cup_pin[i],0);	
						kick_egg[i].state = CUP_CLOSE;
					}
				}
			 }
			 rt_thread_mdelay(2);
		}
}



static int kick_egg_init(void)
{
    rt_thread_t tid;
		tid = rt_thread_create("kickegg",kick_egg_thread, RT_NULL,1024,16, 14);
    if (tid == RT_NULL)
		{
			return RT_ERROR;
		}
    rt_thread_startup(tid);
    return RT_EOK;
}

INIT_APP_EXPORT(kick_egg_init);