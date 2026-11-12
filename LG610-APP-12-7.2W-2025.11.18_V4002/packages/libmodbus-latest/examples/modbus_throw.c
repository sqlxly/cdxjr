#include "modbus_throw.h"
#include "modbus_packge.h"
#include <modbus.h>
#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include <mail_box.h>
#include <mdebug.h>
#include <user_set.h>
#include "common.h"
#include "device_alarm.h"

ALIGN(RT_ALIGN_SIZE)

#define MAX_ERROR_COUNT 5   // 




#define THROW_CH1_SLAVE_ADDR 1
#define THROW_CH2_SLAVE_ADDR 1


#define THROW_CH1_PORT  601
#define THROW_CH1_IP_ADDR "10.10.0.66" 


#define RESPONSE_TIME_OUT  20*1000  //  20ms  
#define MODBUS_CYCLE_TIME    200  //ms


static uint8_t  throw_ch_state[2]={0,0};
static uint8_t reconnect_flag[2]={0,0};
static rt_tick_t reconnect_tick[2]={0,0};
static const uint8_t throw_device_id[2]={0,1};
static uint16_t throw_device_com_err_count[2]={0,0};


uint16_t get_throw_ch_err_count(uint8_t ch)
{
		return throw_device_com_err_count[ch];
}
static void throw_ch_set_reconnect_flag(uint8_t ch,uint8_t flag)
{
	 rt_base_t level;
	 if(ch>1) return;
	 level = rt_hw_interrupt_disable();
   reconnect_flag[ch] = flag;
	 reconnect_tick[ch]  = rt_tick_get();
	 rt_hw_interrupt_enable(level);
} 

uint8_t get_throw_device_en_flag(uint8_t id)
{
	 uint8_t result=0;

	 user_set_part_type *user_set;
	 get_user_set_point(&user_set); 

	if(id==0)
	{
	  result= user_set->device_use_info.chA.throw_ch + user_set->device_use_info.chB.throw_ch;
	}
	else 
	{
		result= user_set->device_use_info.chA.throw_ch + user_set->device_use_info.chB.throw_ch;
	}
   return result;
}
uint8_t get_throw_ch_state(uint8_t ch)
{
	return  throw_ch_state[ch];
}
// 屏幕有数据变化的时候，会将数据通过消息队列的形式发过来
void clear_throw_ch_messeg_buf(uint8_t ch)
{
  uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
  if(ch>1) return;
	if(ch==0)
	{
		while(rt_mq_recv(&masseg_throw1, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
  else if(ch==1)
	{
		while(rt_mq_recv(&masseg_throw1, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
}
uint8_t packge_disp_state[12];
void disp_packge_ch_state()
{
	uint8_t i;

	uint16_t packge_line;
	uint32_t tmp;
	
	for(i=0;i<12;i++)
	{
		// 获取包装通道有没有安装
	  tmp = get_packge_ch_config(packge_map_ch_id[i]);
	  if(tmp>0)
	  {
			// 如果安装了，判断该包装通道有没有报警
			tmp = get_packge_control_alarm(i);
			if(tmp>0)
			{
			  packge_disp_state[i]=11;	// 如果有报警，状态设置为11,11是什么意思呢？
			}
			else 
			{
					// 在获取该通道有没有使用，没有使用状态就设置为0
					// 使用了的话，状态就显示为主控的系统状态
					// 主控的状态有几种，分别是哪些呢？
			     packge_line = get_use_packge_line();
				 if(packge_line &(1<<i))
				 {
				   packge_disp_state[i] = get_system_state();
				 }
				 else 
				 {
				   packge_disp_state[i] =0;
				 }
			}
		}
		else 
		{
			// 没有使能(即屏幕上的没有安装？)，则状态设置为10,10是什么意思呢
		  packge_disp_state[i]=10;
		}
	}
}



static void throw_modbus_mast_ch1(void *param)
{
     int regs;
	   int num = 0;
	
	   uint8_t device_id;
	   uint8_t mb_error_count=0;
     uint8_t tab_reg[64] = {0};
		 uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
		 uint16_t modbus_reconnect_count=0;
		 rt_tick_t connect_time=0;
		 rt_tick_t connect_use_time;
     modbus_t *ctx = RT_NULL;
		
		 device_id = 0;
     rt_thread_mdelay(100);
     ctx = modbus_new_tcp(THROW_CH1_IP_ADDR, THROW_CH1_PORT, AF_INET);
     modbus_set_slave(ctx, THROW_CH1_SLAVE_ADDR);
		 modbus_set_byte_timeout(ctx, 0, 3000);
     modbus_set_response_timeout(ctx, 0, RESPONSE_TIME_OUT);      //超过10MS 超时  
 _mbtcp_start:
		 
		 while(1)
		 {
			if(get_throw_device_en_flag(0)>0)	// 抛弃通道开始安装了，然后再使用的过程中又卸载了，设备会因为连接不上，重连，然后运行到这里
			{
				
				break;
			}	
			else 
			{
				 throw_ch_state[0]=0;
				 rt_thread_mdelay(2000);
			}
		 }
     if(modbus_connect(ctx) < 0)
		 {
			   throw_ch_state[0] =1;
				 DEBUG_CARE("throw modbus ch1  connect fall\n");
			   rt_thread_mdelay(100);
         goto _mbtcp_restart;
		 }
		 else 
		 {
			 connect_use_time = count_use_time(connect_time);
			 if(connect_use_time>10)
			 {
					clear_throw_ch_messeg_buf(0);	
				  DEBUG_CARE("重新连接时间大于10ms 清除数据 time=%d\n",connect_use_time);
			 }
			 DEBUG_CARE("throw modbus ch1  connect scuess\n");
		 }
     throw_ch_state[0]=0;
     while (1)
     {
			 // 接收屏幕发过来的数据，然后通过modbus tcp写给抛弃通道的板子
			 // 内容是什么呢，发的长度为64个字，实际只有4个字节有意义，分别为：调试的时候：命令(1)	哪个电磁阀	该电磁阀的值	电磁阀动作(持续)
			 //																																 关闭的时候：命令直接20
			 //																																 踢蛋的时候：命令(1),但是字节是6个字节 电磁阀动作(0xaa)，还跟有2字节的电磁阀打开时间
			 if (rt_mq_recv(&masseg_throw1, &mesg_buf, MESSAGE_DAT_LEN, MODBUS_CYCLE_TIME) == RT_EOK)
			 {
				
				  regs = modbus_write_registers(ctx, 0,MESSAGE_DAT_LEN, mesg_buf);
				  if(regs < 0)
					{
						DEBUG_ERR("throw modbus wrige reg err\n");
						throw_device_com_err_count[0]++;
						mb_error_count++;
						if(mb_error_count>1)  //连续丢包 
						{
						  if(get_system_state()!=SYSTEM_IS_IDLE)
						  {
							  set_device_alarm(THROW_AWAY_BOARD1_COM_ERR);
						  }
						}
					}
					else 
					{
						mb_error_count=0;
					}
			 }
			 else 
			 {
				  // 如果超时了没有屏幕的数据，则给抛弃通道发送每个包装通道的状态
				 // 为什么要给抛弃通道发送包装通道的状态呢，是因为抛弃通道控制的每个包装通道的点阵屏的显示状态
				 // 并且后面的项目，将每个包装的斜坡点击也由抛弃板子来控制了，抛弃通道也需要知道每个包装通道的状态
				 // 发送包装通道的状态：命令(10)	哪个电磁阀	该电磁阀的值	电磁阀动作(持续)
				  memset(tab_reg, 0, 64);
				  disp_packge_ch_state();
				  tab_reg[0]=10;
				  rt_memcpy(&tab_reg[2],packge_disp_state,12);
				 // rt_kprintf("t1=%d t2=%d t3=%d t4=%d t5=%d t6=%d t7=%d t8=%d\n",packge_disp_state[0],packge_disp_state[1],packge_disp_state[2],packge_disp_state[3],
					//packge_disp_state[4],packge_disp_state[5],packge_disp_state[6],packge_disp_state[7]);
					// 写了10个字就是20个字节，第一个字是命令10，后面6个字是12个包装通道张贴(实际填了14个字节的数据，并且第1个字节没有用到就没有管)
				  regs = modbus_write_registers(ctx, 0,10, (uint16_t *)tab_reg);
					if(regs < 0)
					{
						throw_device_com_err_count[0]++;
						mb_error_count++;
						if(mb_error_count>1)  //连续丢包 
						{
						  if(get_system_state()!=SYSTEM_IS_IDLE)
						  {
							  set_device_alarm(THROW_AWAY_BOARD1_COM_ERR);
						  }
						}
					}
					else 
					{
						mb_error_count=0;
					}
				}
			 	if(mb_error_count>0)
				{
						modbus_reconnect_count++;
						DEBUG_ERR("mb1 ch1 recive fall !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						goto _mbtcp_restart;
				}
				connect_time = rt_tick_get();
     }

 _mbtcp_restart:
		 throw_ch_set_reconnect_flag(0,1);
     modbus_close(ctx);
     goto _mbtcp_start;
     modbus_free(ctx);
 }

 

static int modbus_throw_init(void)
{
    rt_thread_t tid1=RT_NULL;
    rt_thread_t tid2=RT_NULL;
    rt_thread_mdelay(100);
	
    tid1 = rt_thread_create("mbthrow1",throw_modbus_mast_ch1, (void*)&throw_device_id[0],3048,16, 10);
  //  tid2 = rt_thread_create("mbthrow2",throw_modbus_mast_ch2, (void*)&throw_device_id[1],3048,16, 10);
	 
		if (tid1 != RT_NULL)
		{
			rt_thread_startup(tid1);
		//	rt_thread_startup(tid2);
		}
		else 
		{
			return RT_ERROR;
		}
    return RT_EOK;
}
INIT_APP_EXPORT(modbus_throw_init);


