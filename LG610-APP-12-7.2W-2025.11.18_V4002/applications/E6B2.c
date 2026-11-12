
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "E6B2.h"
#include "stm32h7xx_hal.h"
#include "common.h"
#include "mdebug.h"
#include "drv_key.h"
#include "udac.h"
#include "math.h"
#include "device_alarm.h"
#include "app_key.h"
#include "app_screen.h"
#include "drv_key.h"
#include "err_check.h"
#include "sync_egginfo.h"

#define SPT_LINE_ENCODE_CYCLE_DISTEM        1000           //分离线编码器一圈 走的长度   1000mm
#define SPT_LINE_ENCODE_CYCLE_PLUSE         1000          //分离线编码器一圈 产生的脉冲个数  1000
#define MAX_LOST_PLUSE 2

ALIGN(RT_ALIGN_SIZE)



typedef struct
{
	uint8_t   power_up_reset_flag;
	uint8_t   up_pin_flag;
	rt_tick_t cycle_run_time;   //运行时间
	rt_tick_t count_run_time;   //起始时间
  uint32_t cycle_count;
}adjust_spt_pluse_type;


static int32_t spt_encode_pluse_count=0;    //分离通道编码器
static int32_t tsp_encode_pluse_count=0;    //传输通道编码器	// 其实传输通道编码器的值没有用
static uint16_t spt_peed=0;
static adjust_spt_pluse_type adjust_spt_plus=
{
		0,
		0,
		0,
		0,
		0,
};
// 获取上电复位标志
uint16_t get_poer_up_reset_flag()
{
  return  adjust_spt_plus.power_up_reset_flag;
}
// 获取分级线编码器的脉冲值
uint32_t get_spt_encode_pluse()
{
	int32_t pluse;
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	pluse = 	spt_encode_pluse_count;	
	rt_hw_interrupt_enable(level);
	if(pluse<0)
		pluse =0;
	return pluse;
}
// 其实传输通道编码器的值没有用
uint32_t get_tsp_encode_pluse()
{
	int32_t pluse;
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	pluse = 	tsp_encode_pluse_count;	
	rt_hw_interrupt_enable(level);
	if(pluse<0)
		pluse =0;
	return pluse;
}



static uint8_t  reset_origin_flag=0;

// 获取复位状态
uint8_t get_reset_origin_state()
{
   return reset_origin_flag;
}
// 设置主电机到复位的状态,状态为1表示已经复位了
// 当按下复位按键后会调用
void reset_main_moto_to_origin()  //主电机回归原点
{
   reset_origin_flag=1;
}



static uint8_t pluse_err_count=0;
void adjust_encode_pluse()  //一圈校验一下
{
	rt_base_t level;
	uint8_t c_key=0;
	uint8_t egg_count_key;
	int adj;
    uint32_t adjust_pluse;
 // c_key = get_immediately_key_val(RESET_SERSER);
    c_key = (rt_pin_read(ENCODER3_PHASE_Z)&&1);		// 为什么是编码器3的z相呢，不应该是编码器1吗？图纸上是接的1呢，3都没有接呢？
	egg_count_key = get_immediately_key_val(EGG_COUNT_SENSER);	// 有无鸡蛋检测，但是这里没有用
	if(c_key==0)
	// 如果上次检测到是高电平，本次是低电平，因为通过检测缺口来判断转了一圈，当是缺口的时候是低电平
	if((c_key==0)&&(adjust_spt_plus.up_pin_flag==1))
	{
		// 亦即复位到位了
		if(reset_origin_flag>0)   //这个按键复位按下的时候的复位标记
		{
			
		  set_main_moto_speed(0);
			reset_origin_flag=0;
      if(adjust_spt_plus.power_up_reset_flag==0)	// 这个是上电复位的标志
																									// 因为上电的时候为0
																									// 这里置为1后，就相当于上电执行一次，脉冲清零等
			{
			  clear_spt_pluse();	// 复位后将编码器的值清零
				adjust_spt_plus.power_up_reset_flag=1;	// 标记为已复位
				adjust_spt_plus.cycle_run_time =0;
			  adjust_spt_plus.count_run_time = rt_tick_get();
			}
			adjust_pluse = get_spt_encode_pluse();
		  
			//DEBUG_CARE("当前脉冲=%d cycle=%d\n",spt_encode_pluse_count,adjust_spt_plus.cycle_count);
		}
		// 编码器转1圈的时候会是大于450ms吗，不管快慢都会大于450ms吗
		// 也就是大于了450ms就认为编码器转了1圈了？
		if(adjust_spt_plus.cycle_run_time>450)
		{
			adjust_spt_plus.cycle_count++;
			//DEBUG_INFO("cycle time=%d\n",adjust_spt_plus.cycle_run_time);
		}
		//DEBUG_INFO("cycle time=%d\n",adjust_spt_plus.cycle_run_time);
		adjust_spt_plus.cycle_run_time =0;
		// 这个用时最后来计算一圈运行的时间
		adjust_spt_plus.count_run_time = rt_tick_get();
		adjust_pluse = adjust_spt_plus.cycle_count*1000;	// 圈数乘以每圈多少个脉冲，得到运行这么多圈走的脉冲数
		
		/*********注释这一行就OK了*/
		//adjust_pluse = spt_encode_pluse_count;  /*********注释这一行就OK了*/
		
		adj = adjust_pluse - spt_encode_pluse_count;		// 走了这么多圈的脉冲数和通过b相中断得到的脉冲数作差
																										// 1个是通过z相得到圈数然后乘以1圈走1000个脉冲得到的总脉冲数，1个是直接通过b相得到的总脉冲数
		//rt_kprintf("adj=%d spt=%d\n",adjust_pluse,spt_encode_pluse_count);
		adj = abs(adj);
		//如果这2个脉冲差值大于了设定的最大脉冲丢失数量的话，将脉冲数量设置为通过z相得到的脉冲 
		if(adj>MAX_LOST_PLUSE)
		{
			level = rt_hw_interrupt_disable();
			spt_encode_pluse_count=adjust_pluse;	// 将脉冲数量设置为通过z相得到的脉冲 
			rt_hw_interrupt_enable(level);
			//DEBUG_ERR("where loss egg adj pluse=%d  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",adj);
		}
		if(adj>30)	// 如果这2个脉冲差值大于30个脉冲了
		{
			pluse_err_count++;
			if(adjust_spt_plus.power_up_reset_flag !=0)
			{
				DEBUG_ERR("where loss egg adj pluse=%d  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",adj);
				if(pluse_err_count>1)		// 要2次才报警
				{
					WriteMBHotRegU16(822,adj);	// 显示在屏幕的823上
					set_pluse_adjust_val(adj);	// 这个没有使用呢，因为上面已经显示在屏幕上了？
					set_device_alarm(TRANSPORT_AND_SPT_LINE_SYNC_ERR);	// 设置同步错误报警
				}
			}
		}
		else 
		{
			pluse_err_count=0;
		}
		if(adj>40)	// 如果这2个脉冲差值大于40个脉冲了,这个判断有什么用，上面已经判断30了
		{						// 大于40个脉冲差值就直接报警了，上面30个要2次才会报警
			if(adjust_spt_plus.power_up_reset_flag !=0)
			{
				WriteMBHotRegU16(822,adj);
				set_pluse_adjust_val(adj);
				set_device_alarm(TRANSPORT_AND_SPT_LINE_SYNC_ERR);
			}
		}
		WriteMBHotRegU16(821,adj);	// 822的差值和823的差值范围都是这个值？
		//WriteMBHotRegU16(822,MAX_LOST_PLUSE);
	}
	
	adjust_spt_plus.up_pin_flag = c_key;		// 记录z相的电平值
	if(get_spt_speed()<10)	// 小于10mm/s
	{
	  adjust_spt_plus.count_run_time = rt_tick_get();
	}
	else 										// 大于10mm/s则用计时算圈时
	{
	  adjust_spt_plus.count_run_time = count_use_time(adjust_spt_plus.count_run_time);
		adjust_spt_plus.cycle_run_time += adjust_spt_plus.count_run_time;
	  adjust_spt_plus.count_run_time = rt_tick_get();
	}
}
void clear_spt_pluse()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	spt_encode_pluse_count=0;
	tsp_encode_pluse_count =0;
	rt_hw_interrupt_enable(level);
}

// 获取速度  毫米 mm/s  635mm - 36000/h
uint16_t get_spt_speed()
{
	return spt_peed;
}


/*
*\编码器B相中断回调函数
*/

// 触发了b相编码器
// 此时读取a相的电平值就可以判断是正转还是反转
// b高a低是反转
// b高a高是正转
static void e6b2_pb1_callback(void *args)
{
	rt_base_t level;
	
	
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==0)  //123 反转后退屏蔽 有可能是干扰
	{
		//level = rt_hw_interrupt_disable();    
		spt_encode_pluse_count--;
		//rt_hw_interrupt_enable(level);
	}
	else
	{
		//level = rt_hw_interrupt_disable();
		spt_encode_pluse_count++;
		//rt_hw_interrupt_enable(level);
	}
}

/*
*\编码器Z相中断回调函数
*/

uint8_t cycle_flag=0;
int32_t sptpdat=0;
// 转一圈记录一下当前的编码器数据
static void e6b2_pz1_callback(void *args)
{    
  rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	cycle_flag =1;
	sptpdat = spt_encode_pluse_count;
	rt_hw_interrupt_enable(level);
}



static uint16_t avg_per_hour_egg=0;
uint16_t get_avg_per_hour_eggs()
{
  return avg_per_hour_egg;
}
static uint32_t pluse_z_count=0;
static uint32_t avg_totle_speed=0;
static uint16_t avg_speed_count=0;
void e6b2_thread_entery(void *arg)
{
	int32_t spt_upluse;
	int32_t spt_cpluse;
	int32_t inc_plus;
	int32_t speed;
	uint16_t count;
	uint16_t time_count=0;
	rt_tick_t utick,inc_tick;
	
	int32_t avg_spt_iplus;
	int32_t avg_spt_upluse=0;
	int32_t avg_spt_cpluse=0;
	
	rt_tick_t avg_up_tick=0,avg_inc_tick=0;
	float buf;
	float time;
	rt_base_t level;
	
	spt_upluse = get_spt_encode_pluse();
	utick = rt_tick_get();
	avg_up_tick = rt_tick_get();
	rt_thread_mdelay(20);
	while(1)
	{
		time_count++;
		// 1s钟确认一次
		if(time_count%50==0)
		{
			spt_cpluse = get_spt_encode_pluse();
		  inc_plus = spt_cpluse - spt_upluse;	// 本次脉冲减去上一次脉冲得到变化的脉冲(可能根据正反转会变大或者变小)
		  spt_upluse = spt_cpluse;
		  inc_tick = count_use_time(utick);		// 算一下本次和上次时间
		  utick = rt_tick_get();
		
		  buf = (float)1000/6;								// 这个是什么意思呢
			
			//#define SPT_LINE_ENCODE_CYCLE_DISTEM        1000           //分离线编码器一圈 走的长度   1000mm
			//#define SPT_LINE_ENCODE_CYCLE_PLUSE         1000          //分离线编码器一圈 产生的脉冲个数  1000
			// buf = (float)1000/6 是不是这样理解
					// 1	// 分级线的编码器一圈有6个格，转一圈走了1000mm即100cm即10dm即1m
								// 然后每转动1个格就走了1000/6=166.67mm这么多距离
					// 2	// 分级线的编码器一圈有6个格，转一圈走了1000脉冲
								// 然后每转动1个格就走了1000/6=166.67这么多个脉冲
					// 3	// 分级线的编码器一圈走6个蛋杯，转一圈走了1000脉冲
								// 然后每转动1个蛋杯就走了1000/6=166.67这么多个脉冲
					// 好像要以第3种理解，因为166.67好熟悉，有地方是16666
					// 那这个buf可以理解为1个蛋杯距离的脉冲值吗，不然下面的怎么理解
			
		  buf = (float)inc_plus/buf;					// 这个是得到走了多少个蛋杯了吗?
		  buf =buf*63.5;//(这段时间内移动的距离mm)		// 蛋杯数乘以每个蛋杯距离得到走的总共距离
		  
			time = 1000.0/inc_tick;				// 1个tick是1ms，那1000.0/inc_tick这个是什么意思？
		  speed = buf*time;
			
			// 是不是应该这样算(或者这样理解)
			// time = inc_tick/1000.0;	因为1个tick是1ms，多少个tick除以1000就得到走了多少秒
			// speed = buf/time;	距离除以时间就是速度
			
		  speed= (int16_t)(speed);
		  level = rt_hw_interrupt_disable();
		  spt_peed = speed;	// 这个速度单位是 mm/s
		  avg_totle_speed +=spt_peed;		// 这个平均速度后面扩大了10倍，这里又加没有扩大的速度行不行这样？
		  avg_speed_count++;
			// 每1分半钟计算一下平均速度
		  if(avg_speed_count>=100)
		  {
			avg_totle_speed = avg_totle_speed/avg_speed_count;
		    //rt_kprintf("avg speed=%d\n",avg_totle_speed);
			avg_speed_count = 10;
			avg_totle_speed = avg_totle_speed*10;	// 这里扩大10倍是不是乱了
		  }
			rt_hw_interrupt_enable(level);
			//DEBUG_CARE("add speed=%d cspeed=%d\n",spt_add_speed,spt_peed);
		}
		// 每2s
		if(time_count%100==0)
		{
			time_count =0;
			
			avg_spt_cpluse = get_spt_encode_pluse();
			
		  avg_spt_iplus = avg_spt_cpluse - avg_spt_upluse;
		  avg_spt_upluse = avg_spt_cpluse;
			
		  avg_inc_tick = count_use_time(avg_up_tick);
		  avg_up_tick = rt_tick_get();
		
		  buf = (float)1000/6;
		  buf = (float)avg_spt_iplus/buf;
		  buf =buf*63.5;//(这段时间内移动的距离mm)
		  time = 1000.0/avg_inc_tick;
		  speed = buf*time;   //每 S 移动距离
			speed = speed*100;	// 速度扩大了100倍
			
			speed = speed/63.5;	// 速度除以每个蛋杯的距离？应该是
			avg_per_hour_egg = speed * 36;
			
			// speed表示每秒钟走多少mm
			// speed/63.5 一个蛋杯是63.5mm，这个转换就相当于每秒钟走多少个蛋杯
			// speed * 36 每秒钟走多少个蛋乘以一个小时(3600秒，因为上面扩大了100倍，这里少乘以100)得到一个小时走多少个蛋
			
			
			//DEBUG_INFO("EGG PER HOUR =%d\n",avg_per_hour_egg);
		}
		// 每转一圈，记录一下检测到1圈的时候的编码器脉冲值
		// 但是这个值没有使用
		if(cycle_flag)
		{
		//	count = (10000/cycle_time)*6;
		  cycle_flag =0;
		 // WriteMBHotRegU16(822,sptpdat -pluse_z_count);
		  pluse_z_count = sptpdat;	
		}
		ext_key_scan();	// 获取外部输入引脚电平
		adjust_encode_pluse();	// 编码器脉冲调整
		number_one_cup_find();	// 查找第一个蛋杯

       
		rt_thread_mdelay(2);
	}
}
/*
*\初始化增量式编码器
*/
int e6b2_device_init()
{   
    rt_thread_t tid = RT_NULL;

    rt_pin_mode(ENCODER1_PHASE_A, PIN_MODE_INPUT_PULLDOWN);
    rt_pin_mode(ENCODER1_PHASE_B, PIN_MODE_INPUT_PULLDOWN);
    rt_pin_mode(ENCODER1_PHASE_Z, PIN_MODE_INPUT_PULLDOWN);
	rt_pin_mode(ENCODER2_PHASE_A, PIN_MODE_OUTPUT);
    rt_pin_mode(ENCODER2_PHASE_B, PIN_MODE_OUTPUT);
    rt_pin_mode(ENCODER2_PHASE_Z, PIN_MODE_OUTPUT);
	rt_pin_mode(ENCODER3_PHASE_A, PIN_MODE_OUTPUT);
	rt_pin_mode(ENCODER3_PHASE_B, PIN_MODE_OUTPUT);
	rt_pin_mode(ENCODER3_PHASE_Z, PIN_MODE_INPUT_PULLDOWN);
	rt_pin_write(ENCODER1_PHASE_Z,0);
	rt_pin_write(ENCODER2_PHASE_A,0);
	rt_pin_write(ENCODER2_PHASE_B,0);
	rt_pin_write(ENCODER2_PHASE_Z,0);
	rt_pin_write(ENCODER3_PHASE_A,0);
	rt_pin_write(ENCODER3_PHASE_B,0);
	rt_pin_write(ENCODER3_PHASE_Z,0);
    rt_pin_attach_irq(ENCODER1_PHASE_B, PIN_IRQ_MODE_FALLING, e6b2_pb1_callback, RT_NULL);
    rt_pin_irq_enable(ENCODER1_PHASE_B, PIN_IRQ_ENABLE);
	
	rt_pin_attach_irq(ENCODER1_PHASE_Z, PIN_IRQ_MODE_FALLING, e6b2_pz1_callback, RT_NULL);
    rt_pin_irq_enable(ENCODER1_PHASE_Z, PIN_IRQ_ENABLE);
	
	 // rt_pin_attach_irq(ENCODER2_PHASE_B, PIN_IRQ_MODE_FALLING, e6b2_pb2_callback, RT_NULL);
     // rt_pin_irq_enable(ENCODER2_PHASE_B, PIN_IRQ_ENABLE);

  //  rt_pin_attach_irq(ENCODER2_PHASE_Z, PIN_IRQ_MODE_FALLING, e6b2_pz2_callback, RT_NULL);
   // rt_pin_irq_enable(ENCODER2_PHASE_Z, PIN_IRQ_ENABLE);
	
    tid = rt_thread_create("e6b2", e6b2_thread_entery, RT_NULL, 1024, 8, 10);
    if(tid == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(tid);

    return RT_EOK;
}
INIT_APP_EXPORT(e6b2_device_init);
