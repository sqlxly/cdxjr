#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <anlysis_cmd.h>
#include "kick_egg.h"
#include "mail_box.h"
#include "stepper.h"
#include "agile_modbus_user.h"
#include "servo_control.h"
#include "servo_common.h"
#include "servo_debug.h"
#include <mail_box.h>
#include "alarm_control.h"

#define MESSAGE_DAT_LEN 64

static uint8_t mb_dat[MESSAGE_DAT_LEN];

void send_message_to_servo_thread(uint8_t *buf)
{
	rt_err_t result;			
	result= rt_mq_send(&masseg_modbus_cmd, buf, MESSAGE_DAT_LEN);
}

//解析接收抓伺服电机数据
void decode_recive_claw_servo_dat(uint8_t servo_id,uint8_t state,uint8_t dirrection)
{
	 user_debug_recive_claw_servo(servo_id,state,dirrection);
}
//解析包装通道伺服电机数据
void decode_packge_servo_dat(uint8_t servo_id,uint8_t state,uint8_t dirrection)
{

	 user_debug_packge_servo(servo_id,state,dirrection);
	
}
static uint8_t messeg_dat[MESSAGE_DAT_LEN];
void decode_debug_packge_box(uint8_t *dat)
{
	 rt_err_t result;
	 uint8_t ch,state;
	 uint16_t pluse;
	 if(dat[2]==1)  //回原点
	 {
	   ch =dat[1];
		 if(ch==0)
		 {
		   debug_ch1_packge_servo_reset_to_origin();
		 }
		 else if(ch==1)
		 {
		   debug_ch2_packge_servo_reset_to_origin();
		 }
	 }
	 else if(dat[2]==2)  //执行步数
	 {
		 rt_memcpy(messeg_dat,dat,MESSAGE_DAT_LEN);
		 result = rt_mq_send(&masseg_modbus_cmd, &messeg_dat, MESSAGE_DAT_LEN);
     if (result != RT_EOK)
      {
         rt_kprintf("rt_mq_send ERR\n");
      }
	 }
}
void anlysis_modbus_write_dat(uint16_t addr,uint16_t *hot_reg)
{
  if(addr==0)
	{
	  rt_memcpy((uint8_t *)&mb_dat,(uint8_t *)hot_reg,MESSAGE_DAT_LEN);
		//rt_kprintf("dat[0]=%d dat[1]=%d dat[2]=%d dat[3]=%d\n",mb_dat[0],mb_dat[1],mb_dat[2],mb_dat[3]);
		switch(mb_dat[0])
		{
			case 1:		// 运行的时候，踢蛋数据
				decode_run_egg_cup_dat(mb_dat);     
				break;
			case 2:		// 设置包装伺服电机
				decode_packge_servo_dat(mb_dat[1],mb_dat[2],mb_dat[3]);
				break;
			case 3:		// 设置接收伺服电机
				decode_recive_claw_servo_dat(mb_dat[1],mb_dat[2],mb_dat[3]);
			  break;
			case 4:		// 调试接收爪步进电机
				decode_recive_cup_dat(mb_dat);
				break;
			case 5:		// 调试电磁铁踢蛋功能	
				decode_debug_egg_cup_dat(mb_dat);    //蛋杯
			  break;
			case 6:		// 调试包装通道，走指定步数
				decode_debug_packge_box(mb_dat);    //蛋杯
				break;
			case 7:
				decode_close_egg_cup_dat(mb_dat);     
				break;
			case 10:	// 写入包装信息
				write_product_packge(mb_dat[1],(product_packge_info *)&mb_dat[2]);
			  break;
			case 11:	// 设置伺服运行模式(其实不是伺服运行模式，是整个系统的运作模式(运行状态))
				set_servo_run_state(&mb_dat[1]);
			  break;
			case 13:
				 clear_control_alarm(CH1_DEVICE_ID );
			     clear_control_alarm(CH2_DEVICE_ID );
				 break;
			default:
				break;
				
		}
	}
 //rt_kprintf("modbus werite addr=%d cmd=%d\n",addr,mb_dat[0]);
}


void anlysis_modbus_read_input_reg(uint16_t addr,uint16_t *hot_reg)
{

	 if(addr ==0)
	 {
		 get_servo_packge_info((servo_packge_type *)&hot_reg[0]);
		 
	 }
	 else if(addr ==1)
	 {
	   get_servo_run_state((packge_control_type *)&hot_reg[1]);
	 }
   //rt_kprintf("read hot reg addr=%d\n",addr);
	//get_servo_run_state(&servo_state);
  //	rt_memcpy(hot_reg,&servo_state,sizeof(servo_run_state_type));
}


