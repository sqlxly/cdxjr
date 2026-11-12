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
#include "app_screen.h"
#include <board.h>
#include <user_set.h>
#include "common.h"
#include "device_alarm.h"

ALIGN(RT_ALIGN_SIZE)
#define MAX_ERROR_COUNT 0   // 


#define PACKGE_CH1_SLAVE_ADDR 1
#define PACKGE_CH2_SLAVE_ADDR 1
#define PACKGE_CH3_SLAVE_ADDR 1
#define PACKGE_CH4_SLAVE_ADDR 1
#define PACKGE_CH5_SLAVE_ADDR 1
#define PACKGE_CH6_SLAVE_ADDR 1

#define PACKGE_CH1_PORT  601
#define PACKGE_CH2_PORT  601
#define PACKGE_CH3_PORT  601
#define PACKGE_CH4_PORT  601
#define PACKGE_CH5_PORT  601
#define PACKGE_CH6_PORT  601

#define PACKGE_CH1_IP_ADDR "10.10.0.60" 
#define PACKGE_CH2_IP_ADDR "10.10.0.61" 
#define PACKGE_CH3_IP_ADDR "10.10.0.62" 
#define PACKGE_CH4_IP_ADDR "10.10.0.63" 
#define PACKGE_CH5_IP_ADDR "10.10.0.64"
#define PACKGE_CH6_IP_ADDR "10.10.0.65"

#define BYTE_TIME_OUT    2*1000
#define RESPONSE_TIME_OUT  4*1000  //  20ms  
#define MODBUS_CYCLE_TIME    5  //ms
#define READ_BUF_MAX_DAT_LED 2048
#define RECONNECT_TIME_OUT  5




#define MB_P1_ID   0
#define MB_P2_ID   1
#define MB_P3_ID   2
#define MB_P4_ID   3
#define MB_P5_ID   4
#define MB_P6_ID   5

#define PACKGE_BOARD_COUNT 6
static uint16_t totle_err_count[PACKGE_BOARD_COUNT]={0,0,0,0};
static uint8_t board1_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static uint8_t board2_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static uint8_t board3_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static uint8_t board4_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static uint8_t board5_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static uint8_t board6_read_buf[READ_BUF_MAX_DAT_LED] = {0};
static servo_packge_type servo_packge_info[PACKGE_BOARD_COUNT];               //伺服电机包装信息
static packge_control_type packge_control_run_part[PACKGE_BOARD_COUNT];   //包装通道控制器运行参数，里面保存着包装通道运行状态，是能与否，各个接收爪是否有蛋
static uint8_t packge_control_mb_status[PACKGE_BOARD_COUNT]={0,0,0,0,0,0};


void inti_packge_control_run_part()
{
		rt_memset(&packge_control_run_part[0],0,sizeof(packge_control_type));
    rt_memset(&packge_control_run_part[1],0,sizeof(packge_control_type));
    rt_memset(&packge_control_run_part[2],0,sizeof(packge_control_type));
	  rt_memset(&packge_control_run_part[3],0,sizeof(packge_control_type));
	  rt_memset(&packge_control_run_part[4],0,sizeof(packge_control_type));
	  rt_memset(&packge_control_run_part[5],0,sizeof(packge_control_type));
}


uint32_t get_recive_cup_empty_id(uint8_t ch)
{
	uint32_t dat;
	uint8_t board_id;
	
	board_id = ch/2;
	if(ch%2==0)  //左通道
	{
	  dat = packge_control_run_part[board_id].ch1.empty_id;
	}
	else     //右通道
	{
	  dat = packge_control_run_part[board_id].ch2.empty_id;
	}
	return dat;
}
// 获取包装通道，各个接收爪是否有蛋
uint8_t get_packge_recive_cup_eggs(uint8_t ch)
{
	uint8_t dat;
	uint8_t board_id;
	
	board_id = ch/2;
	if(ch%2==0)  //左通道
	{
	  dat = packge_control_run_part[board_id].ch1.cup_eggs;
	}
	else     //右通道
	{
	  dat = packge_control_run_part[board_id].ch2.cup_eggs;
	}
	return dat;
}

uint8_t get_packge_control_mb_state(uint8_t board)
{
	return packge_control_mb_status[board];
	
}
uint16_t get_packge_ch_err_count(uint8_t ch)
{
		return totle_err_count[ch];
}

uint8_t get_packge_control_run_state(uint8_t ch)
{
	uint8_t state=0;
	uint8_t board_id;
	
	board_id = ch/2;
	
	if(ch<12)
	{
		if(ch%2==0)  //左通道
		{
			state = packge_control_run_part[board_id].ch1.state;
		}
		else     //右通道
		{
			state = packge_control_run_part[board_id].ch2.state;
		}
	}
   return state;
}

uint32_t get_packge_control_alarm(uint8_t ch)
{
	uint32_t alarm=0;
	uint8_t board_id;
	
	board_id = ch/2;
	
	if(ch%2==0)  //左通道
	{
	  alarm = packge_control_run_part[board_id].ch1.alarm;
	}
	else     //右通道
	{
	  alarm = packge_control_run_part[board_id].ch2.alarm;
	}
   return alarm;
}

uint8_t get_mb_packge_control_en_flag(uint8_t ch)
{
	uint8_t state=0;
	uint8_t board_id;
	
	board_id = ch/2;
	
	if(ch%2==0)  //左通道
	{
	  state = packge_control_run_part[board_id].ch1.en_flag;
	}
	else     //右通道
	{
	  state = packge_control_run_part[board_id].ch2.en_flag;
	}
   return state;
}
uint32_t get_packge_io_state(uint8_t control)
{
	uint32_t dat;
  dat = packge_control_run_part[control].input_gpio;
  return dat;
}
uint32_t get_recive_servo_offest(uint8_t ch)
{
	uint32_t dist=0;
	uint8_t board_id;
	
	board_id = ch/2;
	
	if(ch%2==0)  //左通道
	{
	  dist = packge_control_run_part[board_id].ch1.recive_servo_offest_distem;
	}
	else     //右通道
	{
	  dist = packge_control_run_part[board_id].ch2.recive_servo_offest_distem;
	}
   return dist;
}
uint32_t get_packge_servo_offest(uint8_t ch)
{
	uint32_t dist=0;
	uint8_t board_id;
	
	board_id = ch/2;
	
	if(ch%2==0)  //左通道
	{
	  dist = packge_control_run_part[board_id].ch1.packge_servo_offest_distem;
	}
	else     //右通道
	{
	  dist = packge_control_run_part[board_id].ch2.packge_servo_offest_distem;
	}
	
  return dist;
}

void clear_mb_servo_packge_info(uint8_t board_id)
{
	rt_base_t level;
  if(board_id>=PACKGE_BOARD_COUNT) return;
	level = rt_hw_interrupt_disable();
	rt_memset(&servo_packge_info[board_id],0,sizeof(servo_packge_type));
//	rt_memset(&servo_packge_info[1],0,sizeof(servo_packge_type));
//	rt_memset(&servo_packge_info[2],0,sizeof(servo_packge_type));
	rt_hw_interrupt_enable(level);
}
void get_control_packge_info(product_packge_info *dat,uint8_t ch)  // CH = 0-5  
{
	uint8_t state=0;
	uint8_t board_id;
	board_id = ch/2;
	
	if(ch%2==0)  //左通道
	{
	  rt_memcpy(dat,&servo_packge_info[board_id].ch1_packge_info,sizeof(product_packge_info));
	}
	else     //右通道
	{
	  rt_memcpy(dat,&servo_packge_info[board_id].ch2_packge_info,sizeof(product_packge_info));
	}
}

uint8_t check_mb_en_flag(uint8_t device_id)
{
	 uint8_t result=0;

	 user_set_part_type *user_set;
	 get_user_set_point(&user_set); 

	 result=0;  
	 if(device_id==MB_P1_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch1 + user_set->device_use_info.chB.packge_ch1;
	 }
	 else if(device_id==MB_P2_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch2 + user_set->device_use_info.chB.packge_ch2;
	 }
	 else if(device_id==MB_P3_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch3 + user_set->device_use_info.chB.packge_ch3;
	 }
	 else if(device_id==MB_P4_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch4 + user_set->device_use_info.chB.packge_ch4;
	 }
	 else if(device_id==MB_P5_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch5 + user_set->device_use_info.chB.packge_ch5;
	 }
	 else if(device_id==MB_P6_ID)
	 {
		 result= user_set->device_use_info.chA.packge_ch6 + user_set->device_use_info.chB.packge_ch6;
	 }
   return result;
}

void analysis_servo_packge_info(servo_packge_type *buf,uint8_t board_id)
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_memcpy(&servo_packge_info[board_id],buf,sizeof(servo_packge_type));
	rt_hw_interrupt_enable(level);
	rt_kprintf("recive part board id=%d 1=%d 2=%d\n",board_id,
	servo_packge_info[board_id].ch1_packge_info.packge_line5_postion,
	servo_packge_info[board_id].ch2_packge_info.packge_line5_postion);
}
// 主控获取包装通道的状态，包装通道回复后，主控对回复的信息信息保存
// 其他地方就可以获取这些包装通道的信息，比如包装通道的状态，io信息，哪些接收爪有蛋
void analysis_packge_control_info(packge_control_type *dat,uint8_t board_id)
{

	rt_base_t level;
	level = rt_hw_interrupt_disable();
	
	rt_memcpy(&packge_control_run_part[board_id],dat,sizeof(packge_control_type));
	rt_hw_interrupt_enable(level);
	//rt_kprintf("recive part board id=%d 1=%d 2=%d\n",board_id,packge_control_run_part[board_id].ch1.);
}


void clear_packge_ch_messeg_buf(uint8_t device_id)
{
  uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
	if(device_id ==MB_P1_ID)
	{
		while(rt_mq_recv(&masseg_p1, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
	else if(device_id ==MB_P2_ID)
	{
		while(rt_mq_recv(&masseg_p2, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
	else if(device_id ==MB_P3_ID)
	{
		while(rt_mq_recv(&masseg_p3, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
	else if(device_id ==MB_P4_ID)
	{
		while(rt_mq_recv(&masseg_p4, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
	else if(device_id ==MB_P5_ID)
	{
		while(rt_mq_recv(&masseg_p5, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
	else if(device_id ==MB_P6_ID)
	{
		while(rt_mq_recv(&masseg_p6, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK);  //清空以前的数据
	}
}

modbus_t *create_cxt(uint8_t device_id)
{
	modbus_t *cxt;
	if(device_id==MB_P1_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH1_IP_ADDR, PACKGE_CH1_PORT, AF_INET);
	}
	else if(device_id==MB_P2_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH2_IP_ADDR, PACKGE_CH2_PORT, AF_INET);
	}
	else if(device_id==MB_P3_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH3_IP_ADDR, PACKGE_CH3_PORT, AF_INET);
	}
	else if(device_id==MB_P4_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH4_IP_ADDR, PACKGE_CH4_PORT, AF_INET);
	}
	else if(device_id==MB_P5_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH5_IP_ADDR, PACKGE_CH5_PORT, AF_INET);
	}
	else if(device_id==MB_P6_ID)
	{
		   cxt = modbus_new_tcp(PACKGE_CH6_IP_ADDR, PACKGE_CH6_PORT, AF_INET);
	}
		 
	return cxt;
}



void mb_data_init(uint8_t device_id,rt_mq_t *mq, uint8_t **read_buf)
{
	 if(device_id ==MB_P1_ID)
	 {
		 *mq= &masseg_p1;
		 *read_buf = board1_read_buf;
	 }
	 else if(device_id ==MB_P2_ID)
	 {
		 *mq= &masseg_p2;
		 *read_buf = board2_read_buf;
	 }
	 else if(device_id ==MB_P3_ID)
	 {
		 *mq= &masseg_p3;
		 *read_buf = board3_read_buf;
	 }
	 else if(device_id ==MB_P4_ID)
	 {
		 *mq= &masseg_p4;
		 *read_buf = board4_read_buf;
	 }
	 else if(device_id ==MB_P5_ID)
	 {
		 *mq= &masseg_p5;
		 *read_buf = board5_read_buf;
	 }
	 else if(device_id ==MB_P6_ID)
	 {
		 *mq= &masseg_p6;
		 *read_buf = board6_read_buf;
	 }
}

int8_t write_dat_to_device(modbus_t *ctx,uint16_t addr,uint8_t len,uint16_t *dat)
{
   uint8_t i;
	 rt_err_t result;
	 for(i=0;i<3;i++)
   {
		result = modbus_write_registers(ctx, addr,len, dat);
		if(result >0)
			break;
		else
		{
		  rt_thread_mdelay(2);
		}
	 }
  return result;
}
static void packge_modbus_mast(void *param)
{
     int regs;
	   rt_mq_t mq;
	   uint8_t cmd;
	   uint8_t device_id;
	   uint8_t error_count=0;
	   uint8_t *read_buf;
	   rt_tick_t connect_time=0;
	   rt_tick_t connect_use_time=0;
	   modbus_t *ctx = RT_NULL;
		 uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
		 rt_time_t up_read_mb_time=0;
		 rt_time_t time;
		 device_id = *(uint8_t*)param;
		 rt_kprintf("device id=%d\n",device_id);
		 ctx = create_cxt(device_id);
		 mb_data_init(device_id,&mq,&read_buf);
     modbus_set_slave(ctx, 1);
		 modbus_set_debug(ctx, 0);
		 modbus_set_byte_timeout(ctx, 0, BYTE_TIME_OUT);
     modbus_set_response_timeout(ctx, 0, RESPONSE_TIME_OUT);      //超过10MS 超时  
		 modbus_set_error_recovery(ctx,MODBUS_ERROR_RECOVERY_PROTOCOL );
		 rt_thread_mdelay(2000);
		 up_read_mb_time=  rt_tick_get();		

 _mbtcp_start:
		 
		 while(1)
		 {
			if(check_mb_en_flag(device_id)>0)
			{
				break;
			}	
			else 
			{
				 packge_control_mb_status[device_id]=0;
				 rt_thread_mdelay(2000);
			}
		 }
		 set_reconnect_flag(device_id,1);
     if(modbus_connect(ctx) < 0)
		 {
			   packge_control_mb_status[device_id]=1;
			//	 DEBUG_ERR("packge modbus connect fall ID=%d\n",device_id);
         goto _mbtcp_restart;
		 }
		 else 
		 {
			 connect_use_time = count_use_time(connect_time);
			 if(connect_use_time>10)
			 {
					clear_packge_ch_messeg_buf(device_id);	
				  clear_mb_servo_packge_info(device_id);
				  DEBUG_CARE("重新连接时间大于10ms 清除数据 time=%d\n",connect_use_time);
			 }
			 else 
			 {
			    DEBUG_CARE("mb cok ID=%d time=%d\n",device_id,connect_use_time);
			 }
		 }
     while (1)
     {
			 regs =0;
			 cmd = 100;
			 packge_control_mb_status[device_id]=0;
			 time = count_use_time(up_read_mb_time);
			 // 计算好包装通道踢蛋信息
			 if (rt_mq_recv(mq, &mesg_buf, MESSAGE_DAT_LEN, MODBUS_CYCLE_TIME) == RT_EOK)
			 {
				 time = count_use_time(up_read_mb_time);
				 cmd = *(uint8_t*)&mesg_buf;
				 //rt_kprintf(" packge_id=%d recive cmd=%d\n",device_id,cmd);
				 switch(cmd)
				 {
					  case RUNING_TICK_EGG_CMD:
						case CLOSE_EGG_CUP_CMD:	
							// 将包装信息发给包装通道，device_id
							 regs = write_dat_to_device(ctx, 0,10, mesg_buf);
						   if(regs<0)
							 {
								 // 失败了则继续发？
								 // device_id*2 是因为板子只有6个，0-5
								 // device_id:	0		1		2		3		4		5
								 // ch				:0,1	2,3 4,5	6,7 8,9 10,11
								 // 所以板子id*2才会发到相应的板子上
								 // 比如板子0和板子1不乘2，都发到板子0上了
							   send_message_to_packge_device(device_id*2,(uint8_t *)mesg_buf); 
							 }
						   break;
						case SEND_RUN_MODE_CMD:
					  case SET_PACKGE_SERVO_SETP_CMD:
						case SET_RECIVE_SERVO_SETP_CMD:
						case DEBUG_TICK_EGG_CMD:
						case SEND_PACKGE_INFO_CMD:
						case DEBUG_PACKGE_BOX_CMD:
						case DEBUG_RECIVE_CUP_CMD:
						case CLEAR_SLAVE_SERVO_ALARM:
							// 如果是其他命令则写一次就可以了，不判断写成功没有
						  regs =   write_dat_to_device(ctx, 0,MESSAGE_DAT_LEN, mesg_buf);
						  break;
						case GET_SERVO_PACKGE_CMD:
							 memset(read_buf, 0, READ_BUF_MAX_DAT_LED);
							 regs = modbus_read_input_registers(ctx, 0, sizeof(servo_packge_type), (uint16_t *)read_buf);
							 if(regs > 0)
							 {
								 analysis_servo_packge_info((servo_packge_type *)read_buf,device_id);
							 }
							break;
						case GET_SERVO_RUN_PART:
							   memset(read_buf, 0, READ_BUF_MAX_DAT_LED);
								 regs = modbus_read_input_registers(ctx, 1, sizeof(packge_control_type), (uint16_t *)read_buf);
								 if(regs > 0)
								 {
					         analysis_packge_control_info((packge_control_type *)read_buf,device_id);
				         }
								 break;
						default:
							rt_kprintf("cmd is not analisys cmd=%d\n",cmd);
							break;
				 }
					if(regs >0)
					{	
							up_read_mb_time=  rt_tick_get();		
					}	
					rt_thread_mdelay(1);		
			 }
			 
			 if(time>1000)   //50ms 轮询一次状态
			 {
			   up_read_mb_time=  rt_tick_get();
				 memset(read_buf, 0, READ_BUF_MAX_DAT_LED);
				 regs = modbus_read_input_registers(ctx, 1, sizeof(packge_control_type), (uint16_t *)read_buf);
				 if(regs > 0)
				 {
					 analysis_packge_control_info((packge_control_type *)read_buf,device_id);
				 }
				 rt_thread_mdelay(1);		
			 }
			 
			 if(regs < 0)
			 {
						DEBUG_ERR("mb fall id=%d result=%d cmd=%d\n",device_id,regs,cmd);
				        set_reconnect_flag(device_id,1);
						totle_err_count[device_id]++;
						error_count++;
				    if(error_count>1)  //连续丢包 
						{
						  if(get_system_state()!=SYSTEM_IS_IDLE)
						  {
							  set_device_alarm(PACKGE1_TCP_ERR+device_id);
						  }
						}
					  if(error_count>MAX_ERROR_COUNT)
					  {
						//	DEBUG_ERR("包装通道通讯异常ID=%d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",device_id);
							goto _mbtcp_restart;
						}
			 }
			 else 
			 {
				 if(regs>0)
				 {
					 connect_time = rt_tick_get();
					 error_count=0;
				 }
			 } 
     }
 _mbtcp_restart:
		
     modbus_close(ctx);
     goto _mbtcp_start;
     modbus_free(ctx);
 }


 static const uint8_t packge_device_id[PACKGE_BOARD_COUNT]={0,1,2,3,4,5};
 static uint8_t reconnect_flag[PACKGE_BOARD_COUNT]={0,0,0,0,0,0};
 static rt_tick_t reconnect_tick[PACKGE_BOARD_COUNT]={0,0,0,0,0,0};
 void set_reconnect_flag(uint8_t device_id,uint8_t flag)
 {
	 rt_base_t level;
	 
	 level = rt_hw_interrupt_disable();
   if(device_id>=PACKGE_BOARD_COUNT) 
	 {
		 return ;
	 }
	 reconnect_flag[device_id]= flag;
	 reconnect_tick[device_id] = rt_tick_get();
	 rt_hw_interrupt_enable(level);
 }
 


 
 static int modbus_packge_init(void)
{
    rt_thread_t tid1=RT_NULL;
    rt_thread_t tid2=RT_NULL;
	  rt_thread_t tid3=RT_NULL;
	  rt_thread_t tid4=RT_NULL;
	  rt_thread_t tid5=RT_NULL;
	  rt_thread_t tid6=RT_NULL;
	
		DEBUG_CARE("modbus_packge_init\n");
		
	  rt_err_t result;
	  inti_packge_control_run_part();
    tid1 = rt_thread_create("MBP1",packge_modbus_mast, (void*)&packge_device_id[0],3048,12, 10);
	  tid2 = rt_thread_create("MBP2",packge_modbus_mast, (void*)&packge_device_id[1],3048,13, 10);
	  tid3 = rt_thread_create("MBP3",packge_modbus_mast, (void*)&packge_device_id[2],3048,14, 10);
	  tid4 = rt_thread_create("MBP4",packge_modbus_mast, (void*)&packge_device_id[3],3048,15, 10);
	  tid5 = rt_thread_create("MBP5",packge_modbus_mast, (void*)&packge_device_id[4],3048,16, 10);
	  tid6 = rt_thread_create("MBP6",packge_modbus_mast, (void*)&packge_device_id[5],3048,17, 10);
	  if ((tid1 != RT_NULL)&&(tid2 != RT_NULL)&&(tid3 != RT_NULL)&&(tid4 != RT_NULL)&&(tid5 != RT_NULL)&&(tid6!= RT_NULL))
		{
			rt_thread_startup(tid1);
			rt_thread_startup(tid2);
			rt_thread_startup(tid3);
			rt_thread_startup(tid4);
			rt_thread_startup(tid5);
			rt_thread_startup(tid6);
			result = RT_EOK;
		}
		else 
		{
		  result = RT_ERROR;
		}
		return result;
}
INIT_APP_EXPORT(modbus_packge_init);

