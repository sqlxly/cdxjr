#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <sys/select.h>
#include "ext_gpio.h"
#include "drv_74hxx.h"
#include "mdebug.h"
#include "servo_common.h"
#include "stepper.h"
#include "alarm_control.h"




static volatile uint32_t gpio_dat=0;

static volatile uint8_t ch1_up_status=0;
static volatile uint8_t ch2_up_status=0;

static volatile uint8_t ch1_tray_flag[5]={0,0};
static volatile uint8_t ch2_tray_flag[5]={0,0};

static uint16_t device1_input_dat=0;
static uint16_t device2_input_dat=0;
static uint8_t en_check_tray_falg=0;
/*
typedef enum
{
	CUP_SENSER_PIN =0,
	RECIVE_LINE_ORIGIN_PIN,
	CHECK_TRAY_SENSER,
	PACKGE_LINE_ORIGIN_PIN,
	EMPTY,
}GPIO_ID;
*/


void relay_init()
{
  rt_pin_mode(RELAY1, PIN_MODE_OUTPUT);
  rt_pin_mode(RELAY2, PIN_MODE_OUTPUT);
  rt_pin_mode(RELAY3, PIN_MODE_OUTPUT);
	rt_pin_mode(RELAY4, PIN_MODE_OUTPUT);
}


void set_relay1(uint8_t val)
{
  rt_pin_write(RELAY1, val);
}
void set_relay2(uint8_t val)
{
  rt_pin_write(RELAY2, val);
}
void set_relay3(uint8_t val)
{
  rt_pin_write(RELAY3, val);
}
void set_relay4(uint8_t val)
{
  rt_pin_write(RELAY4, val);
}


void set_check_tray_flag(uint8_t tray_flag)
{
	rt_base_t level;

	level = rt_hw_interrupt_disable();
		en_check_tray_falg = tray_flag;
	rt_hw_interrupt_enable(level);
}
uint32_t get_control_input_io_dat()
{
	uint32_t dat; 
	dat = gpio_dat;
	return dat;
}

void clear_tray_flag(uint8_t id)
{
	rt_base_t level;

	level = rt_hw_interrupt_disable();
	if(id ==0 )
	{
		ch1_tray_flag[0]=0;
		ch1_tray_flag[1]=0;
		ch1_tray_flag[2]=0;
		ch1_tray_flag[3]=0;
		ch1_tray_flag[4]=0;
	}
	else 
	{
		ch2_tray_flag[0]=0;
		ch2_tray_flag[1]=0;
		ch2_tray_flag[2]=0;
		ch2_tray_flag[3]=0;
		ch2_tray_flag[4]=0;
	}
	rt_hw_interrupt_enable(level);
}

void set_tray_flag(uint8_t id)
{
	rt_base_t level;

	level = rt_hw_interrupt_disable();
	if(id ==0 )
	{
		ch1_tray_flag[0]=1;
		ch1_tray_flag[1]=1;
	}
	else 
	{
		ch2_tray_flag[0]=1;
		ch2_tray_flag[1]=1;
	}
	rt_hw_interrupt_enable(level);
}

typedef struct
{
  uint8_t ch1_senser_buf[50];   //可以记录400次 *5ms   2s 以内的状态
	uint8_t ch2_senser_buf[50];   //可以记录400次 *5ms   2s 以内的状态
	uint16_t ch1_count;
	uint16_t ch2_count;
}check_tray_dat_type;

#define CHECK_TRAY_OFFEST_COUNT  5
static check_tray_dat_type check_tray_io;
// 获取是否有蛋托
uint8_t get_tray_position_status(uint8_t ch,uint16_t index)
{ 
  uint8_t *dat_buf;
	uint8_t  pos;
	uint8_t dat;
	uint8_t nb;
	uint8_t status;
  if(ch==1)
	{
		 dat_buf = &check_tray_io.ch1_senser_buf[0];
	}
	else 
	{
		 dat_buf = &check_tray_io.ch2_senser_buf[0];
	}
	
  pos = index/8;
  dat = dat_buf[pos];
	nb = index%8;
	nb = 1<<nb;
	status = nb&dat;
	status = status&&1;
	return status;
}

//检测托盘位置是否正确  返回1  正确 0 错误
// 没有调用
uint8_t check_tray_poistion(uint8_t ch,uint16_t count) 
{
	int16_t i,j;
	uint16_t status=0;
	uint16_t pluse;
	uint16_t result=0;
	if(count>399)
	{
		return 0;
	}
	
	status = get_tray_position_status(ch,i-1);
	return status ;
	for(i=count;i>0;i--)
	{
	  status = get_tray_position_status(ch,i-1);
		if(status==0)
		{
		  j=i;
			break;
		}
	}
	rt_kprintf("o pluse=%d\n",j);
	for(i=0;i<j;i++)
	{
	  status = get_tray_position_status(ch,i-1);
    result += status;
	}
	result = (result>0)? 0:1;
	return result;
}
// 设置蛋托状态？
// 设备运行到原点的时候会调用设置
// 电平取反了，有蛋托的时候，设置为0？
void set_check_tray_io_status(uint8_t ch,uint16_t index,uint8_t val)  //ch=1 对应通道1   ch=2对应通道2
{
	uint8_t id;
	uint8_t nb;
	uint8_t dat;
	

	if(index>399)
	{
		return;
	}
	id = index/8;
	nb = index%8;
	nb = 1<<nb;
	
	if(ch==1)
	{
		dat = check_tray_io.ch1_senser_buf[id];
		if(val>0)
		{
			check_tray_io.ch1_senser_buf[id] = dat | nb;
		}
		else 
		{
			nb = ~nb;
			check_tray_io.ch1_senser_buf[id] = dat & nb;
		}
	}
	else 
	{
	  dat = check_tray_io.ch2_senser_buf[id];
		if(val>0)
		{
			check_tray_io.ch2_senser_buf[id] = dat | nb;
		}
		else 
		{
			nb = ~nb;
			check_tray_io.ch2_senser_buf[id] = dat & nb;
		}
	}
	//rt_kprintf("index=%d val=%02x\n",index,check_tray_io.ch2_senser_buf[id]);
}



//MSH_CMD_EXPORT(count, list device in system);
// 获取蛋托的标记，该标记保存着是否有蛋托
uint8_t get_tray_flag(uint8_t id)
{
	uint8_t tary;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	if(id ==0 )
		tary = ch1_tray_flag[0];
	else 
		tary = ch2_tray_flag[0];
	rt_hw_interrupt_enable(level);
	return 1;//tary;
}
// 获取下一个蛋托的标记，该标记保存着是否有下一个蛋托
uint8_t get_nex_tray_flag(uint8_t id)
{
	uint8_t tary;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	if(id ==0 )
		tary = ch1_tray_flag[1];
	else 
		tary = ch2_tray_flag[1];
	rt_hw_interrupt_enable(level);
	return 1; //tary;
}
static uint8_t ch1_up_origin_pin=0;
static uint8_t ch2_up_origin_pin=0;

static uint8_t tray_err[2]={0,0};
static void check_cross_origin()
{
  uint8_t cstate;
  rt_base_t level;
	
	cstate = get_ext_pin(CH1_PACKGE_RESET_ORIGIN_PIN);
	if((cstate !=ch1_up_origin_pin)&&(cstate==0))
	{ 
		
		if(ch1_tray_flag[1]==0)
		{
			tray_err[0]++;
		}
		else 
		{
		  tray_err[0]=0;
		}
		if(tray_err[0]>0)
		{
			tray_err[0]=0;
		  set_alarm(CH1_DEVICE_ID,tray_empty_err,1);
		}
		
		level = rt_hw_interrupt_disable();
    ch1_tray_flag[0] = ch1_tray_flag[1] ;
		ch1_tray_flag[1] = 0; 
		check_tray_io.ch1_count=0;
	  rt_hw_interrupt_enable(level);
	//	DEBUG_INFO("ch1 cross origin time=%d tray=%d\n",rt_tick_get(),ch2_tray_flag[0]);
	}
	ch1_up_origin_pin = cstate;
 
	cstate = get_ext_pin(CH2_PACKGE_RESET_ORIGIN_PIN);
	if((cstate !=ch2_up_origin_pin)&&(cstate==0))
	{ 
		
		if(ch2_tray_flag[1]==0)
		{
			tray_err[1]++;
		}
		else 
		{
		  tray_err[1]=0;
		}
		if(tray_err[1]>0)
		{
			tray_err[1]=0;
		    set_alarm(CH2_DEVICE_ID,tray_empty_err,1);
		}
		
		level = rt_hw_interrupt_disable();
        ch2_tray_flag[0] = ch2_tray_flag[1] ;
		ch2_tray_flag[1] = 0; 
		check_tray_io.ch2_count=0;
	  rt_hw_interrupt_enable(level);
		DEBUG_INFO("ch2 cross origin time=%d tray=%d\n",rt_tick_get(),ch2_tray_flag[0]);
	}
	ch2_up_origin_pin = cstate;
}



static uint8_t ch1_up_state=0;
static uint8_t ch2_up_state=0;
void check_recive_servo_position()
{
	 uint8_t state;
   uint8_t cstate;
   uint8_t dat;
	// 检测到接收伺服运动到了当前设置的路径的目的地了
	 cstate = get_ext_pin(CH1_RECIVE_SERVO_CMP_PIN);		// 检测接收伺服电机是否到了下降的位置，到了步进电机就可以打开了
   if((cstate>0)&&(ch1_up_state==0))
	 {
//		 rt_kprintf("ch1 recive cmp pin is 1\n");
		 set_cup_open_signal(0);
		 
	 }
	 ch1_up_state = cstate;
	 
	 cstate = get_ext_pin(CH2_RECIVE_SERVO_CMP_PIN);
   if((cstate>0)&&(ch2_up_state==0))
	 {
//		 rt_kprintf("ch2 recive cmp pin is 1\n");
		 set_cup_open_signal(1);
		 
	 }
	 ch2_up_state = cstate;
}
static void check_tray()
{
	uint8_t pin_val;
    uint8_t cstate;
	rt_base_t level;
	uint8_t position;

	
	// 到达设置的蛋托比较的位置的时候，检查是不是有蛋托
	//就是一个蛋托20000个脉冲
	// 在每次7000脉冲的时候就检测一下是不是有蛋托
	cstate = get_ext_pin(CH1_PACKGE_SERVO_CMP_PIN);
	if(( cstate !=ch1_up_status)&&(cstate ==1))
	{ 
	    level = rt_hw_interrupt_disable();
        pin_val = get_ext_pin(CH1_CHECK_TRAY_PIN) ;
		if(pin_val !=1)
		{ 
		  set_alarm(CH1_DEVICE_ID,tray_empty_err,1);
		 // set_alarm(CH1_DEVICE_ID,slope_transport_ch_cumulate,1);
		}
	    rt_hw_interrupt_enable(level);
		DEBUG_INFO("ch1 tray flag=%d \n",pin_val);
	}
	ch1_up_status = cstate;
	
	cstate = get_ext_pin(CH2_PACKGE_SERVO_CMP_PIN);
	if(( cstate !=ch2_up_status)&&(cstate ==1))
	{ 
		
	    level = rt_hw_interrupt_disable();
        pin_val = get_ext_pin(CH2_CHECK_TRAY_PIN) ;
		if(pin_val !=1)
		{ 
		  set_alarm(CH2_DEVICE_ID,tray_empty_err,1);
		}
	    rt_hw_interrupt_enable(level);
		DEBUG_INFO("ch2 tray=%d position=%d,cout=%d\n",ch2_tray_flag[1],position,check_tray_io.ch2_count);
	}
	ch2_up_status = cstate;
	
	
	// 在原点的时候，检查一下是不是有蛋托
	// 但是原点是只有运行的时候一次或者按了原点复位后停在哪里一直检查是不是有蛋托？
	// 运行的时候原点引脚应该不会有高电压输出？即使过了原点，因为没有执行恢复原点指令？
	if(get_ext_pin(CH1_PACHGE_SERVO_STATE_PIN)>0)  //输送电机在动 时候检测
	{
	  pin_val = get_ext_pin(CH1_CHECK_TRAY_PIN);
		pin_val = !pin_val;
	  set_check_tray_io_status(1,check_tray_io.ch1_count,pin_val);
		check_tray_io.ch1_count++;
	}
	
	if(get_ext_pin(CH2_PACHGE_SERVO_STATE_PIN)>0)  //输送电机在动 时候检测
	{
	  pin_val = get_ext_pin(CH2_CHECK_TRAY_PIN);
		pin_val = !pin_val;
	  set_check_tray_io_status(2,check_tray_io.ch2_count,pin_val);
		check_tray_io.ch2_count++;
	}
  if(check_tray_io.ch1_count>10000) check_tray_io.ch1_count =10000;
	if(check_tray_io.ch2_count>10000) check_tray_io.ch2_count =10000;
}

uint8_t get_ext_pin(ext_key_id pin_id)
{
  uint32_t val;
	uint32_t dat;
	
	dat = gpio_dat;
	val = 1<<pin_id;
	dat = dat&val;
	dat = dat&&1;
	return (uint8_t )dat;
}




static void gpio_scan(void *arg)
{

 	rt_base_t level;
	uint8_t sn74hc165_dat[5];
	
	
	while(1)
	{	
		sn74hcxx_get_dat(sn74hc165_dat,3);
		level = rt_hw_interrupt_disable();
		rt_memcpy(&gpio_dat,sn74hc165_dat,sizeof(gpio_dat));
		rt_hw_interrupt_enable(level);
		check_recive_servo_position();
		if(en_check_tray_falg>0)
		{
	        //check_cross_origin();
			check_tray();
		}
		else 
		{
		    ch1_tray_flag[0] = 1 ;
			ch1_tray_flag[1] = 1; 
			ch2_tray_flag[0] = 1 ;
			ch2_tray_flag[1] = 1; 
		}
	  rt_thread_mdelay(7);
	}
}

int ext_gpio_init(void)
{
	  rt_thread_t tid;
		tid = rt_thread_create("egpio",gpio_scan, RT_NULL,1224,20, 1);
    if (tid == RT_NULL)
		{
			return RT_ERROR;
		}
    rt_thread_startup(tid);
    return RT_EOK;
}

INIT_APP_EXPORT(ext_gpio_init);

