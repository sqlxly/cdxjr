#include "modbus_transport.h"
#include "device_alarm.h"
#include <modbus.h>
#include <stdio.h>
#include <rtdevice.h>
#include <board.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include <mail_box.h>
#include <mdebug.h>
#include "common.h"

ALIGN(RT_ALIGN_SIZE)
#define MAX_ERROR_COUNT 1   // 

#define TRANSPORT_CH1_PORT  502

#define TRANSPORT_CH1_SLAVE_ADDR 1


#define TRANSPORT_CH1_IP_ADDR "10.10.0.67" //67

#define RESPONSE_TIME_OUT  20*1000  //  20ms  
#define MODBUS_CYCLE_TIME    200  //ms

static uint16_t totle_err_count[3]={0,0,0};
static uint8_t  transport_ch_state=0;
static uint16_t ch1_bad_egg_num=1;
static uint16_t ch2_bad_egg_num=1;
static uint16_t ch3_bad_egg_num=1;

static transport_io_dat transport_io;
// 这个屏幕定时刷新使用，需要获取最新的传输线的io状态
transport_io_dat *get_transport_io_info()
{
	return &transport_io;
}
// 没有调用
void reflash_transport_io()
{
	 uint8_t result;
   uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
	 result = rt_mq_send(&masseg_t1, &mesg_buf,MESSAGE_DAT_LEN);
   if (result != RT_EOK)
   {
       rt_kprintf("rt_mq_send ERR\n");
   }
}

//0-11 对应 12个继电器
// 屏幕有设置或者逻辑有变化的时候，则会调用这个函数
void set_transport_io(TRANSPORT_OUTPUT_IO_ID id,uint8_t val)
{
  if(id<16)
	{
		transport_io.output.gpio[id]=val;
	}
}

// 这个坏蛋的3个检测引脚
// qianxuping 这个是原来有个手动来计坏蛋的功能，可能没有1个1个的计，是比如10个蛋记一次，计的时候转一下按钮产生一个脉冲，这个脉冲表示10个蛋？
static uint16_t bad_egg_count=0;
uint16_t get_bad_egg_count()
{
  uint16_t count=0;
  rt_base_t level;
  level = rt_hw_interrupt_disable();
  count = bad_egg_count;
  bad_egg_count = 0;	
  rt_hw_interrupt_enable(level);
  return count;
}
// 没有调用
void count_bad_egg(uint8_t ch1,uint8_t ch2,uint8_t ch3)
{
    rt_base_t level;
	
	uint16_t egg_cout=0;
	egg_cout = ch1*ch1_bad_egg_num;
	egg_cout += ch2*ch2_bad_egg_num;
	egg_cout += ch3*ch3_bad_egg_num;
    level = rt_hw_interrupt_disable();
	bad_egg_count+=egg_cout;
    rt_hw_interrupt_enable(level);
}
// 这个是输送线板子发过来(是主控主动去读然后输送线板子回复的)的输入引脚的状态
void analysis_transport_dat(uint8_t *dat)
{
	uint8_t i;
	
	rt_memcpy(&transport_io.input.gpio[0],dat,sizeof(transport_input_io_type));
}

// 光检通道平时是亮的，所以是要初始化为高电平，异常的时候是没有亮即低电平
void transport_io_init()
{
	 rt_memset(&transport_io,0,sizeof(transport_io_dat));
     transport_io.input.gpio[light_ch1_unnormal] =1;   //光检通道初始化为1
	 transport_io.input.gpio[light_ch2_unnormal] =1;   //光检通道初始化为1
}

// 检测输送通道状态的时候需要调用，状态异常主控则会报警
uint8_t get_transport_ch_state(void)
{
	return  transport_ch_state;
}
// 调用了，但是没有卵用
void init_bad_egg_key_set()
{
  user_set_part_type *dat;
	
    dat= rt_malloc(sizeof(user_set_part_type));
	if(dat ==RT_NULL)
		return;
	get_user_set_dat(dat);
    ch1_bad_egg_num = dat->transport.ch1_nb;
	ch2_bad_egg_num = dat->transport.ch2_nb;
	ch3_bad_egg_num = dat->transport.ch3_nb;
    free(dat);
}
static void transport_modbus_mast_ch1(void *param)
{
     int regs;
	   uint8_t mb_error_count=0;
     uint8_t read_buf[40] = {0};
		 uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
		 uint16_t modbus_reconnect_count=0;
		 
     modbus_t *ctx = RT_NULL;
		 packge_mail_box_type *recive_mb;
		 transport_io_init();
		 init_bad_egg_key_set();
     rt_thread_mdelay(100);
		 while (1)
     {
			    rt_thread_mdelay(200);
		 }
     ctx = modbus_new_tcp(TRANSPORT_CH1_IP_ADDR, TRANSPORT_CH1_PORT, AF_INET);
     modbus_set_slave(ctx, TRANSPORT_CH1_SLAVE_ADDR);
		 modbus_set_byte_timeout(ctx, 0, 3000);
     modbus_set_response_timeout(ctx, 0, RESPONSE_TIME_OUT);      //超过10MS 超时  
 _mbtcp_start:
     if(modbus_connect(ctx) < 0)
		 {
			   transport_ch_state=1;
			   set_device_alarm(TRANSPORT_CH_COM_ERR);
			   rt_kprintf("transport modbus ch1  connect fall\n");
         goto _mbtcp_restart;
		 }
		 else 
		 {
				 DEBUG_CARE("transport modbus ch1  connect scuess\n");
		 }
      transport_ch_state=0;
     int num = 0;
		 
		 
     while (1)
     {
			    rt_thread_mdelay(200);
					// 主控循环写入输送通道的output io值信息
			 // 485程序会将位组成一个字节一个字节，然后按字节发送
			 // MODBUS_FC_WRITE_MULTIPLE_COILS 
			 // 但是解析数据的时候怎么用的是tab_registers呢？是不是因为这个的原因导致新绛的设备，点击屏幕，下面设备没有反应
				// 因为解析或者处理的数据位置都不对?
				  regs = modbus_write_bits(ctx, 0,sizeof(transport_output_io_type),(uint8_t *)&transport_io.output.gpio[0]);
				  if(regs < 0)
					{
						DEBUG_ERR("transport modbus wrige reg err\n");
						totle_err_count[0]++;
						mb_error_count++;
					}
					else 
					{
						mb_error_count=0;
					}
         rt_thread_mdelay(200);
				 memset(read_buf, 0, sizeof(read_buf));
					// 主控循环读取输送通道的input io值信息
					// MODBUS_FC_READ_DISCRETE_INPUTS是读取离散输入
					// 但是611输送板子为什么在MODBUS_FC_READ_INPUT_REGISTERS/MODBUS_FC_READ_HOLDING_REGISTERS里面回复呢
					// 这样对不对
				 regs = modbus_read_input_bits(ctx, 0, sizeof(transport_input_io_type), (uint8_t *)read_buf);
         if(regs < 0)
				 {
						DEBUG_ERR("transport modbus read reg err\n");
						totle_err_count[0]++;
						mb_error_count++;
				 }
				 else 
				 {
					  analysis_transport_dat(read_buf);
						mb_error_count=0;
				 }
			 	if(mb_error_count>MAX_ERROR_COUNT)
				{
						modbus_reconnect_count++;
						//DEBUG_ERR("mb1 ch1 recive fall !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						goto _mbtcp_restart;
				}
     }
 _mbtcp_restart:
     modbus_close(ctx);
     rt_thread_mdelay(2000);
     goto _mbtcp_start;
     modbus_free(ctx);
 }

 static int modbus_transport_init(void)
{
    rt_thread_t tid1;
	  rt_err_t result;

    tid1 = rt_thread_create("MB_t1",transport_modbus_mast_ch1, RT_NULL,2048,20, 10);

	 
		if (tid1 != RT_NULL)
		{
			rt_thread_startup(tid1);
		}
		else 
		{
			return RT_ERROR;
		}
    return RT_EOK;
}
INIT_APP_EXPORT(modbus_transport_init);

void show_mb_transport(void)
{
	rt_kprintf("totle err count  P1=%d  P2=%d  P3=%d\n",totle_err_count[0],totle_err_count[1],totle_err_count[2]);
}
MSH_CMD_EXPORT(show_mb_transport, show packge reconnect time);
