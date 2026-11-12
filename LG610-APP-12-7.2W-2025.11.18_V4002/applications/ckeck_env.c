#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "uadc.h"
#include "udac.h"
#include "relay.h"
#include "E6B2.h"
#include "math.h"
#include "drv_key.h"
#include "mdebug.h"
#include "common.h"
#include "user_set.h"
#include "modbus_screen.h"
#include "modbus_packge.h"
#include "screen_chat.h"
#include "move_control.h"
#include "egg_control.h"
#include "product_set.h"
#include "product_control.h"
#include <mail_box.h>
#include "user_set.h"
#include "app_screen.h"
#include "ckeck_env.h"
#include "device_alarm.h"
#include "spt_egg.h"

ALIGN(RT_ALIGN_SIZE)

static uint8_t check_env_step=0;
void set_check_env_step(uint8_t step)
{
  check_env_step = step;
}
// 发送包装通道的运行状态给包装通道
// 当时0左通道则包装通道取message_buf[3]这个数据
// 当时1右通道则包装通道取message_buf[4]这个数据
void set_control_run_state(uint8_t ch,uint8_t state)
{
	uint8_t check_tray_flag;
	uint8_t servo_err_count;
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	user_set_part_type *user_set;
	get_user_set_point(&user_set); 	// 获取用户设置的参数，包括模块使能情况，各种等级等
	servo_err_count = user_set->commun_part.servo_moto_connect_number;	// 伺服电机重连次数
	check_tray_flag = user_set->mode_en.check_tray;	// 蛋托检测开关
	check_tray_flag =0;
	if(servo_err_count>50) servo_err_count =50;
	if(servo_err_count==0) servo_err_count =1;
	message_buf[0]=11;  //命令
	message_buf[1]= ch%2;  // 0 左 1:右  左右通道
	message_buf[2]= state;
	message_buf[3]= get_packge_ch_config((ch/2)*2);
	message_buf[4]= get_packge_ch_config((ch/2)*2+1);
	message_buf[5]= check_tray_flag;
	message_buf[6]= servo_err_count;
	send_message_to_packge_device(ch,message_buf);     
}


// 发送包装通道的参数，包括行列以及落点位置信息
void send_product_packge_to_control(uint16_t ch,product_packge_info *packge_info)
{

  uint8_t message_buf[MESSAGE_DAT_LEN]; 
	
	if(sizeof(product_packge_info)>=(MESSAGE_DAT_LEN-3))
	{
		DEBUG_ERR("send dat very big !!!!!\n");
	}
	rt_memset(message_buf,0,sizeof(message_buf));
	message_buf[0]=10;  //命令
	message_buf[1]= ch%2;  // 0 左 1:右  左右通道
	rt_memcpy(&message_buf[2],packge_info,sizeof(product_packge_info));
	send_message_to_packge_device(ch,message_buf);     
}
// 发送获取伺服包装参数
void get_product_packge_to_control(uint16_t ch)
{

  uint8_t message_buf[MESSAGE_DAT_LEN]; 
	
	rt_memset(message_buf,0,sizeof(message_buf));
	message_buf[0]=GET_SERVO_PACKGE_CMD;  //命令
	send_message_to_packge_device(ch,message_buf);     
}
// 检查预生产的信息设置是否完整
uint8_t check_per_product_info() //检测与生产的产品设置是否完整
{
	 uint8_t count=0;
   uint8_t i;
	 uint8_t result=0;
	 
	 production_type *production;
	
	 production = rt_malloc(sizeof(production_type));
	 if(production ==RT_NULL)
	 {
		 return 0;
	 }
	 rt_memset(production,0,sizeof(production_type));
	 // 获取预生产信息
	 get_production_mange_set(production);
	 for(i=0;i<12;i++)
	 {
		 // 最多预生产12个
		 if(production->pre_production[i].empty !=0)  //不为空
		 {
			 // 预生产信息不为空
			 count++;	// 记录一下不为空的预生产信息
			 /*
			  count++;
		    if(production.pre_production[i].weight_leve>10)
					result =  1;
				if(production.pre_production[i].colour_leve>10)
					result = 2;
				if(production.pre_production[i].crack_leve>10)
					result = 3;
				if(production.pre_production[i].dirty_leve>10)
					result = 4;
				if(production.pre_production[i].print_id>10)
					result = 5;
				
				if(production.pre_production[i].task_box_count==0)
					result = 7;
			 */
			 
			 // 检查该预生产的产品线都没有即没有设置那个包装通道
			 if(production->pre_production[i].packge_line==0)
			 {
					result = 6;
			 }
			 // 自动配盘检测
			 if(production->pre_production[i].auto_clacure_weight[0]>0) //启用自动配盘
			 {
			    if(production->pre_production[i].box_count==0)   //未完善
					{
						result = 9;
					}
					if((production->pre_production[i].auto_clacure_weight[3]<10)||(production->pre_production[i].auto_clacure_weight[4]>100))  //设置错误
					{
						result = 10;
					}
					uint32_t tray_egg_count;
					uint32_t min_weight;
					uint32_t max_weight;
					
					tray_egg_count = production->pre_production[i].tray_line*production->pre_production[i].tray_row;
					tray_egg_count = tray_egg_count * production->pre_production[i].box_count;
					min_weight = production->pre_production[i].auto_clacure_weight[1]*1000;
					min_weight = min_weight/tray_egg_count;
					max_weight = production->pre_production[i].auto_clacure_weight[2]*1000;
					max_weight = max_weight/tray_egg_count;
					init_egg_boxfull_info(i,min_weight,max_weight);
			 }
		 }
	 }
	 // 如果没有预生产信息
	 if(count==0)
	 {
		 result =8;
	 }
	 
	 // 如果设置都对了则清除报警
	 if(result ==0 )
	 {
		clear_device_alarm(PER_PROTUCE_ERR);
		set_check_env_step(1);	// 然后检查包装通道
	 }
	 else 
	 {
			set_device_alarm(PER_PROTUCE_ERR);
	 }
	 rt_free(production);
   DEBUG_CARE("预生产 检测结果为%d\n",result);
  
}
// 同步包装信息
void sync_product_packge()
{
	 uint8_t alarm_flag=0;
	 uint16_t product_id,i;
	 uint8_t control_state;
	 uint16_t packge_line;
	 product_packge_info mast_packge,control_packge;	// 定义一个主控这边的包装信息和包装通道的包装信息
	 production_type *production;
	
	 production = rt_malloc(sizeof(production_type));
	 if(production ==RT_NULL)
	 {
		 return ;
	 }
	 rt_memset(production,0,sizeof(production_type));
	 rt_memset(&mast_packge,0,sizeof(product_packge_info));
	 rt_memset(&control_packge,0,sizeof(product_packge_info));
	 // 先将预生产信息
	 get_production_mange_set(production);
	 packge_line = 0;
	 // 检查预生产信息
	 for(product_id=0;product_id<12;product_id++)    //6个产品
	 {
		 if(production->pre_production[product_id].empty !=0)  //不为空
		 {
			  packge_line =  production->pre_production[product_id].packge_line;	// 有预生产信息则看这个预生产信息准备用哪些包装通道
		
				// 用的包装通道有可能是这12个通道的一个或者多个通道
			  for(i=0;i<12;i++) //6条包装线
			  {
					 if(packge_line&(1<<i))
					 {
						 //获取控制器(即设置或者包装通道的？)的包装信息
						 get_control_packge_info(&control_packge,packge_map_ch_id[i]); 
							// 根据产品id获取该产品的包装信息
						 get_pruduct_packge_info(production->pre_production[product_id].packge_id,&mast_packge); 	
						 // 比较这两个信息是不是一样
						 if (rt_memcmp(&control_packge,&mast_packge, sizeof(product_packge_info)) != 0)
						 {
								// 如果不一样则将主控这边的包装信息发给包装通道
							  send_product_packge_to_control(packge_map_ch_id[i],&mast_packge);
							  rt_thread_mdelay(20);
							  get_product_packge_to_control(packge_map_ch_id[i]);
							 
							  alarm_flag = get_device_alarm(PACKGE_BOARD1_COM_ERR+(packge_map_ch_id[i]/2));

							  if(alarm_flag==0)
								{
									//DEBUG_INFO("SYNC PACKGE ch=%d packge id=%d\n",i,production->pre_production[product_id].packge_id);
								}
						 }
						 else 
						 {
						  // DEBUG_INFO("ch=%d conntrol name=%s   mast name=%s\n",i,control_packge.name,mast_packge.name);
						 }
					  }
			  }
		 }
	 }
   rt_free(production);
}
// 检查包装通道
uint8_t check_packge_line_is_ok(ch)
{
	uint8_t base;

  uint32_t io_dat;
	uint32_t buf;
	uint32_t val;
	uint8_t count=0;
    io_dat = get_packge_io_state(ch/2);
	if((ch%2)==0)   //一共 24个IO口  0-11为 通道1  12-23为通道2
	{
	  base = 0;
	}
	else
	{
	  base = 12;
	}
	
	buf = 1<<(CH1_EGG_CUP_OPEN_SENSER_PIN+base);	// CH1_EGG_CUP_OPEN_SENSER_PIN或者CH2_EGG_CUP_OPEN_SENSER_PIN检查这个引脚是不是高电平
	val = buf & io_dat;
	count+= (val&&1);
	//WriteMBHotRegU16(1230,val&&1);
	
	buf = 1<<(CH1_RECIVE_RESET_ORIGIN_PIN+base);	// CH1_RECIVE_RESET_ORIGIN_PIN或者CH2_RECIVE_RESET_ORIGIN_PIN检查这个引脚是不是高电平
	val = buf & io_dat;
	count+= (val&&1);
	
	if(count==2) // 如果都是高电平则返回1
		return 1;
	else 
		return 0;		// 有一个或者2个都不是高电平则返回0
}
extern void init_avg_egg_line_max_count(uint8_t id,uint8_t line);
//检测伺服电机的包装是否OK
void check_servo_packge()
{
	 uint8_t t_line=0;
	 uint8_t ready=0;
	 uint16_t product_id,i;
	 uint8_t control_state;
	 uint16_t packge_line;

	 product_packge_info mast_packge,control_packge;
   
	 production_type *production;
	
	 production = rt_malloc(sizeof(production_type));
	 if(production ==RT_NULL)
	 {
		 return ;
	 }
	 rt_memset(production,0,sizeof(production_type));
	 rt_memset(&mast_packge,0,sizeof(product_packge_info));
	 rt_memset(&control_packge,0,sizeof(product_packge_info));
	 get_production_mange_set(production);
	 packge_line = 0;
	
	 for(product_id=0;product_id<12;product_id++)    //12个产品
	 {
		 if(production->pre_production[product_id].empty !=0)  //不为空
		 {
			  packge_line =  production->pre_production[product_id].packge_line;
			  init_avg_spt_egg_info(product_id,production->pre_production[product_id].packge_line,production->pre_production[product_id].tray_line);
			 
			  for(i=0;i<PACKGE_LINE_COUNT;i++) //12条包装线
			  {
					 if(packge_line&(1<<i))
					 {
						 
						 if(check_packge_line_is_ok(packge_map_ch_id[i])==0)
						 {
							set_device_alarm(PACKGE_RECIVE1_ERR+i);
							//set_device_alarm(RECIVE_SENSER_ERR);
							set_system_state(SYSTEM_IS_IDLE);
							// DEBUG_ERR("RECIVE IO ERR ch=%d\n",i);
							 return ;
						 }
						 rt_kprintf("产品ID=%d  通道ID=%d 包装ID=%d\n",product_id,i,production->pre_production[product_id].packge_id);
						 t_line++;
						 get_control_packge_info(&control_packge,packge_map_ch_id[i]);  //从servo_packge_info里面获取控制器的包装信息
						 get_pruduct_packge_info(production->pre_production[product_id].packge_id,&mast_packge); 
						 init_recode_kick_egg_info(packge_map_ch_id[i],mast_packge.line);		 //	初始化踢蛋失败后重启的，packge_map_ch_id[i]哪个通道， mast_packge.line 包装的列数
             init_avg_egg_line_max_count(packge_map_ch_id[i],mast_packge.line);		// 设置每个通道的蛋托每排的最大数量					 
						 control_state = get_packge_control_run_state(packge_map_ch_id[i]);		
						 // 将每个包装通道的包装信息和主控这边产品的包装信息对比是不是一样
						 // 不一样就要给包装该通道发送让包装通道处于系统校验参数状态
						 // 然后发送包装参数给包装通道
						 // 然后在获取包装通道的包装参数(在servo_packge_info里)(获取后哪里对比呢)
						 if (rt_memcmp(&control_packge,&mast_packge, sizeof(product_packge_info)) != 0)
						 {
							  set_control_run_state(packge_map_ch_id[i],SYS_IS_CRC_PART); 
							  rt_thread_mdelay(5);
							  send_product_packge_to_control(packge_map_ch_id[i],&mast_packge);
							  rt_thread_mdelay(5);
							  get_product_packge_to_control(packge_map_ch_id[i]);
							  rt_kprintf("产品包装不一致 ch=%d packge id=%d\n",packge_map_ch_id[i],production->pre_production[product_id].packge_id);
						 }
						 else 
						 {
							 // 如果两边的包装参数一样了，说明包装通道就准备好了
								if(control_state != SYSTEM_IS_READY )
								{
									set_control_run_state(packge_map_ch_id[i],SYS_IS_CHECK_ENV); 
								}		
								ready++;
								rt_kprintf("产品包装一致 ch=%d  packge id=%d\n",packge_map_ch_id[i],production->pre_production[product_id].packge_id);
							}						 
					  }
			  }
		 }
	 }
	   rt_thread_mdelay(500);
	   if(ready == t_line )   //所有的包装都一致了
		 {
			 set_check_env_step(2);
		 }
    rt_free(production);
}

// 检查包装通道的运行状态
void check_packge_control_state()
{
	 uint16_t j;
	 uint8_t control_state;
	 uint8_t t_line,ready;
   uint16_t packge_line;
	 
	 t_line =0;
	 ready =0;
	 packge_line = get_use_packge_line();
	 for(j=0;j<12;j++) //12条包装线
	 {
		 if(packge_line&(1<<j))
		 {
				t_line++;
				control_state = get_packge_control_run_state(packge_map_ch_id[j]);
				if(control_state == SYSTEM_IS_READY )
				{
					ready++;		
				}
				rt_kprintf("line=%d state=%d e=%d\n",j,control_state,SYSTEM_IS_READY);
			}
		}
    if(ready == t_line )   //所有的包装都一致了
		{
			for(j=0;j<12;j++) //6条包装线
			{
				if(packge_line&(1<<j))
				{
					// 给对应的包装通道发送系统为运行状态
					set_control_run_state(packge_map_ch_id[j],SYSTEM_IS_RUING); 
					set_check_env_step(3);
				}
			}
		}	 
}
// 检查包装是否处于运行状态
// 就是上面设置了运行状态后，包装通道应该会返回状态到packge_control_run_part包装运行参数这个变量里面
// 这个函数就去获取返回的包装运行参数从而得到该包装通道的运行状态
void check_packge_control_is_ready()
{
   uint16_t j;
	 uint8_t control_state;
	 uint8_t t_line,ready;
   uint16_t packge_line;
    
	 t_line =0;
	 ready =0;
	 packge_line = get_use_packge_line();
   for(j=0;j<12;j++) //6条包装线
	 {
		 if(packge_line&(1<<j))
		 {
				t_line++;
				control_state = get_packge_control_run_state(packge_map_ch_id[j]);
				if(control_state == SYSTEM_IS_RUING )
				{
					ready++;		
				}
			}
		}
	 if(ready == t_line )   //所有的包装都一致了
	 {
			//for(j=0;j<6;j++) //6条包装线
			//{
			 // DEBUG_CARE("system check is ok\n");
				set_check_env_step(4);
			//}
		}
	 else 
	 {
			rt_kprintf("read=%d line=%d\n",ready,t_line);
	 }
}
//同步包装控制器状态,如果该通道的包装控制器没有使能或者不是空闲状态则让该包装通道处于空闲状态
void sync_packge_control_state_idle()
{
	uint8_t i;
  uint8_t state;
	uint8_t config;
  uint8_t en_flag;
	for(i=0;i<12;i++)
	{
		config = get_packge_ch_config(i);
		if(config>0)
		{
			en_flag = get_mb_packge_control_en_flag(packge_map_ch_id[i]);
			state = get_packge_control_run_state(packge_map_ch_id[i]);               //获取当前通道状态
			if((state != SYSTEM_IS_IDLE)||(en_flag ==0))
			{
				set_control_run_state(packge_map_ch_id[i],SYSTEM_IS_IDLE); 
				rt_thread_mdelay(3);
			}
		}
	}
}

//同步包装控制器状态
void sync_packge_control_state(uint8_t cstate)
{
	uint8_t i;
  uint8_t packge_control_status;
	uint8_t device_config;
	uint8_t product_select;
  uint8_t packge_line;
	
	packge_line = get_use_packge_line();
	for(i=0;i<12;i++)
	{
		device_config = get_packge_ch_config(i);
		if(device_config>0)
		{
		   packge_control_status = get_packge_control_run_state(packge_map_ch_id[i]);               //获取当前通道状态
			product_select = (packge_line&(1<<i));
			// 如果该包装通道使用了并且要同步的状态和现在的状态不一样，则将要同步的状态发给包装通道
			 if((packge_control_status !=cstate)&&(product_select>0))
			 {
				 // rt_kprintf("SYNC STATE CH=%d  CSTATE=%d SET STATE=%d\n",i,packge_control_status,cstate);
				  set_control_run_state(packge_map_ch_id[i],cstate); 
				  rt_thread_mdelay(20);
			 }
		}
	}
	
}
static uint32_t check_env_time=0;
//检查系统预期启动的全部参数
uint8_t check_system_per_start_part()
{
	 uint32_t count_time=0;
	 switch(check_env_step)
	 {
	   case 0:
				check_per_product_info();
				break;
		 case 1:
			    check_env_time=rt_tick_get();
				check_servo_packge();
		    break;
		 case 2:
			    count_time = count_use_time(check_env_time);
		        if((count_time>5000)&&(count_time<10000))
				{
				  set_device_alarm(CHECK_PRODUCK_PACKGE_INFO_IS_OK);
				}
				check_packge_control_state();
		    break;
		 case 3:
				check_packge_control_is_ready();
				break;
		 case 4:
				return 0xA5;    //校验已完成
				break;
		  default:
				break;
	 }
	 rt_kprintf("step =%d\n",check_env_step);
	 return check_env_step;
 }
	
	
