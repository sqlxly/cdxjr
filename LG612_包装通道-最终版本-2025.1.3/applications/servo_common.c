#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <sys/select.h>
#include "stepper.h"
#include "rs4851.h"
#include "string.h"
#include "stepper.h"
#include "recode_dat.h"
#include "ext_gpio.h"
#include "mdebug.h"
#include "stdlib.h"
#include "servo_control.h"
#include "servo_common.h"
#include "servo_debug.h"
#include "agile_modbus_user.h"
#include "alarm_control.h"
#include "board_config.h"
#include <kick_egg.h>

#define MAX_PACKGE_ROW 15    //包装盒最大行数
#define SERVO_RUN_PART_PAR0 0x000C5582   //加减速等信息 
#define SERVO_RUN_PART_PAR1 0x000F5382 //0x000EF282   //最后一大步加减速等信息 

#define RECIVE_SERVO_RUN_PART_PAR0  0x000BF002       //0x0008F302
#define RECIVE_SERVO_RUN_PART_PAR1  0x000BF002//0x000BE002



// 得到差值的tick，1个tick是1ms
rt_tick_t count_use_time(rt_tick_t start_time)
{
	rt_tick_t ctime,use_time;
	
	ctime = rt_tick_get();
	if(ctime>=start_time)
	{
		use_time = ctime - start_time;
	}
	else 
	{
		use_time = (0xffffffff - start_time) + ctime;
	}
	return use_time;
}

// 写伺服驱动写数据，写一个地址 数据是2个字节的数据
rt_err_t servo_write_reg(servo_device_id device,int addr,uint16_t value)
{
	rt_err_t result;
	for(int i=0;i<3;i++)
	{
		if((device==RS4851_RECIVE_ID)||(device==RS4851_PACKGE_ID))
		{
			result = servo_write_reg_rs4851(device,addr,value);
		}
		else 
		{
			result = servo_write_reg_rs4852(device,addr,value);
		}
		if(result == RT_EOK)
		{
			break;
		}
	}
	if(result != RT_EOK)
	{
			DEBUG_ERR("485写寄存器失败ID=%d\n",device);
	}
	return result;
}
// 写伺服驱动写数据，写多个地址 个数是多少个2字节的数据
rt_err_t servo_write_mutly_regs(servo_device_id id,uint16_t addr, uint16_t *dat,uint16_t len)
{
	rt_err_t result;
	for(int i=0;i<3;i++)
	{
		if((id==RS4851_RECIVE_ID)||(id==RS4851_PACKGE_ID))
		{
			result = servo_write_mutly_regs_rs4851(id,addr,dat,len);
		}
		else 
		{
			result = servo_write_mutly_regs_rs4852(id,addr,dat,len);
		}
		if(result == RT_EOK)
		{
			break;
		}
	}
	if(result != RT_EOK)
	{
			DEBUG_ERR("485批量写寄存器失败ID=%d\n",id);
	}
	return result;
}

static uint8_t servo_loop_count=10;

// 设置读取伺服485的时候最大操作次数，即最大操作失败的次数
void set_servo_run_count(uint8_t dat)
{
    rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	servo_loop_count = dat;
	rt_hw_interrupt_enable(level);

}
// 获取读取伺服485的时候最大操作次数，即最大操作失败的次数
uint8_t get_servo_run_count()
{
	uint8_t dat;
    rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	dat = servo_loop_count;
	rt_hw_interrupt_enable(level);
    return dat;
}
// 读取伺服多个地址，长度是寄存器个数即读取多少个寄存器，一个寄存器的数据是2字节，例如读取4个寄存器，读取到的数据是8字节
rt_err_t servo_read_mutly_regs(servo_device_id id,uint16_t addr, uint16_t *dat,uint16_t len)
{
	rt_err_t result;
	uint8_t loop_count;
	uint8_t i;
	loop_count = get_servo_run_count();
	if(loop_count<10) loop_count =10;  //最小10次
	if(loop_count>50) loop_count =50;  //最大50次
	for(i=0;i<loop_count;i++)
	{
		if((id==RS4851_RECIVE_ID)||(id==RS4851_PACKGE_ID))
		{
			result = servo_read_mutly_regs_rs4851(id,addr,dat,len);
		}
		else 
		{
			result = servo_read_mutly_regs_rs4852(id,addr,dat,len);
		}
		if(result == RT_EOK)
		{
			break;
		}
		rt_thread_mdelay(5);
	}
	if(result != RT_EOK)
	{
		 if(id==RS4851_RECIVE_ID)
		 {
			 set_alarm(CH1_DEVICE_ID,recive_server_com_err,1);
		 }
		 else if(id==RS4851_PACKGE_ID)
		 {
			 set_alarm(CH1_DEVICE_ID,transport_server_com_err,1);
		 }
		 else if(id==RS4852_RECIVE_ID)
		 {
			 set_alarm(CH2_DEVICE_ID,recive_server_com_err,1);
		 }
		 else if(id==RS4852_PACKGE_ID)
		 {
			 set_alarm(CH2_DEVICE_ID,transport_server_com_err,1);
		 }
			DEBUG_ERR("485批量读寄存器失败ID=%d\n",id);
	}
	else 
	{
		 if(id==RS4851_RECIVE_ID)
		 {
		//	 set_alarm(CH1_DEVICE_ID,recive_server_com_err,0);
		 }
		 else if(id==RS4851_PACKGE_ID)
		 {
		//	 set_alarm(CH1_DEVICE_ID,transport_server_com_err,0);
		 }
		 else if(id==RS4852_RECIVE_ID)
		 {
		//	 set_alarm(CH2_DEVICE_ID,recive_server_com_err,0);
		 }
		 else if(id==RS4852_PACKGE_ID)
		 {
		//	 set_alarm(CH2_DEVICE_ID,transport_server_com_err,0);
		 }
	}
	
	return result;
}


//伺服电机开启
//************
rt_err_t set_servo_on(servo_type *servo,uint8_t state)
{
	rt_err_t result;
	result += servo_write_reg(servo->id,SERVO_ON_ADDR,state);
}
// 写给伺服让伺服执行具体的哪一组pr路径数据并开始执行
rt_err_t write_servo_run_path(uint8_t device_id,uint8_t path)
{
	uint8_t i;
  rt_err_t result;
  uint16_t dat;
	for(i=0;i<3;i++)
	{
		if((device_id==RS4851_RECIVE_ID)||(device_id==RS4851_PACKGE_ID))
		{
			result = servo_write_reg_rs4851(device_id,START_ROUT_PATH,path); // START_ROUT_PATH 0x50e是pr触发命令，path：写入0-1000表示执行0-99组pr的路径数据，1000是停止
		}																																		// 所为的触发pr指令就是伺服执行相关pr指令表示伺服电机在运动了
		else 
		{
			result = servo_write_reg_rs4852(device_id,START_ROUT_PATH,path);	// START_ROUT_PATH 0x50e是pr触发命令，path：写入0-1000表示执行0-99组pr的路径数据，1000是停止
		}																																		// 所为的触发pr指令就是伺服执行相关pr指令表示伺服电机在运动了
		if(result == RT_EOK)
		{
			return result;				// 运行成功了则退出本函数
		}
		else    //写失败了就要注意  确认伺服电机没有收到以及执行这个动作 要不然会多走一步
		{
			// 运行失败了，则读取一下写的写进去没有
			result = servo_read_mutly_regs(device_id,START_ROUT_PATH,&dat,1);
			if((result == RT_EOK)&&(dat == path))
			{
				return RT_EOK;			// 写的运行数据成功了也退出本函数
			}
			else
			{
				DEBUG_CARE("设置路径失败count=%d\n",i);
			}
		}
		rt_thread_mdelay(3);
	}
	
		// 让伺服运动失败了即写给伺服失败了则报警
	 if(device_id==RS4851_RECIVE_ID)
	 {
			set_alarm(CH1_DEVICE_ID,recive_server_place_err,1);
	 }
	 else if(device_id==RS4851_PACKGE_ID)
	 {
			set_alarm(CH1_DEVICE_ID,transport_server_place_err,1);
	 }
	 if(device_id==RS4852_RECIVE_ID)
	 {
			set_alarm(CH2_DEVICE_ID,recive_server_place_err,1);
	 }
	 else if(device_id==RS4852_PACKGE_ID)
	 {
			set_alarm(CH2_DEVICE_ID,transport_server_place_err,1);
	 }
	 return RT_ERROR;
}


//伺服电机执行
// path 路径  一共64个
// 写给伺服让伺服执行具体的哪一组pr路径数据并开始执行
rt_err_t set_servo_run_path(servo_type *servo)
{
	rt_err_t result;
	servo->reset_flag =  RT_FALSE;
	result = write_servo_run_path(servo->id,servo->hope_run_path); // 运行哪一个pr，servo->hope_run_path
	if(result != RT_EOK)
	{
		DEBUG_ERR("伺服驱动器通讯失败 id=%d\n",servo->id);
	}
	else 
	{
		servo->run_time =0;
		servo->start_time = rt_tick_get();
		servo->state = SERVO_RUNING_PATH;
	}
}

//read_reg_len 读几个寄存器 不是字节数量
// 读取伺服路径pr组的数据
rt_err_t read_servo_path_part(uint8_t id,uint16_t addr, uint16_t *dat,uint16_t read_reg_len)
{

	uint8_t i;
	rt_err_t result;
 
	result = servo_read_mutly_regs(id,addr,dat,read_reg_len);
	if(result != RT_EOK)
	{
		DEBUG_ERR("读取伺服驱动器通讯失败\n");
	}
	return result;
}

//write_reg_len 写几个寄存器 不是字节数量
// 设置伺服电机pr组的路径数据
rt_err_t set_servo_path_part(uint8_t id,uint16_t addr, uint16_t *dat,uint16_t write_reg_len)
{

	uint8_t i;
	rt_err_t result;
	result = servo_write_mutly_regs(id,addr,dat,write_reg_len);
	if(result != RT_EOK)
	{
		DEBUG_ERR("伺服驱动器通讯失败\n");
	}
	return result;
}


//伺服电机执行
// path 路径  一共64个
// 运行伺服回到原点的pr路径
rt_err_t set_servo_reset_origin(servo_type *servo)
{
	rt_err_t result;
	uint8_t i;
	DEBUG_INFO("servo reset origin id=%d\n",servo->id);
	result = servo_write_reg(servo->id,START_ROUT_PATH,RESET_ORIGIN_PATH);
	if(result != RT_EOK)
	{
		DEBUG_ERR("伺服驱动器通讯失败 id=%d\n",servo->id);
	}
	else 
	{
		servo->run_time =0;
		servo->start_time = rt_tick_get();
		servo->state = SERVO_RESET_ORIGIN;
	}
}
// 清除伺服本身的报警信息
rt_err_t clear_alarm(servo_type *servo)
{
	
	uint8_t i;
	rt_err_t result;
	uint32_t dat=0;
	result = servo_write_mutly_regs(servo->id,CLEAR_ALARM_ADDR,(uint16_t *)&dat,2);
	DEBUG_CARE("clear servo alarm\n");
	if(result != RT_EOK)
	{
		DEBUG_ERR("伺服驱动器通讯失败\n");
	}
}
// 清除伺服的编码数据信息
rt_err_t clear_encode_pluse(servo_type *servo)
{
	
	uint8_t i;
	rt_err_t result;
	uint32_t dat=0;
	result = servo_write_mutly_regs(servo->id,ENCODE_PLUSE_ADDR,(uint16_t *)&dat,2);
	if(result != RT_EOK)
	{
		DEBUG_ERR("伺服驱动器通讯失败\n");
	}
}

// 伺服空闲的时候，检测到伺服的状态引脚为高电平则让伺服停止
void servo_idle(servo_type *servo)
{
	if(get_ext_pin(servo->state_pin)==IS_OPEN)
	{
		servo_write_reg(servo->id,START_ROUT_PATH,1000);	// 向触发pr地址写入1000表示触发伺服停止
	}
}

void servo_stop(servo_type *servo)
{
	rt_kprintf("pin=%d state=%d\n",servo->state_pin,get_ext_pin(servo->state_pin));
	if(get_ext_pin(servo->state_pin)==IS_OPEN)				// 停止的时候，检测到伺服的状态引脚为高电平则让伺服停止
	{
		servo_write_reg(servo->id,START_ROUT_PATH,1000);
	}																									// 没有检测到状态引脚为高电平，则让伺服处于空闲状态，并且执行回到原点的操作
	else 
	{
	  servo->state = SERVO_IDLE;			
		servo->hope_run_path =0;												// 表示要伺服电机回到原点
	}
}

// 检测本次设置的pr组路径数据执行完成是否
void check_servo_run_path_is_ok(servo_type *servo)
{
	// 先计算运行的时间
	servo->run_time = count_use_time(servo->start_time);

	// 判断状态引脚是否是低电平(平时是高电平即在原位置的时候是高电平，运行的时候电机走了肯定检测不到就是低电平了)
	// 并且运行的时间大于设置的最小运行时间0.1s
	if((get_ext_pin(servo->state_pin)==IS_CLOSE)&&(servo->run_time>SERVO_MIN_RUN_TIME))
	{
		if(servo->run_time<80)
		//if(servo->id==1)
			DEBUG_CARE("packge path=%d time=%d\n",servo->hope_run_path,servo->run_time);
		//if(servo->id ==1)
		//	DEBUG_CARE("packge path=%d time=%d ctick=%d start tick=%d\n",servo->hope_run_path,servo->run_time,rt_tick_get(),servo->start_time);
		servo->state = SERVO_IDLE;
		servo->hope_run_path = 0xff;
	}
	else
	{
		// 如果上面的条件不成立则判断运行时间是不是大于2s，如果还没有回到原点并且时间大于2s则报警
		if(servo->run_time>2000)
		{
		  if(servo->id==1)
			{
			    set_alarm(CH1_DEVICE_ID,transport_server_place_err,1);
			}
      else if(servo->id==2)
			{
			    set_alarm(CH2_DEVICE_ID,transport_server_place_err,1);
			}
			else if(servo->id==3)
			{
			    set_alarm(CH1_DEVICE_ID,recive_server_place_err,1);
			}
			else if(servo->id==4)
			{
			    set_alarm(CH2_DEVICE_ID,recive_server_place_err,1);
			}
		}
	}
}

extern ch_device_type ch1_device;
extern ch_device_type ch2_device;


// 清除伺服编码器的脉冲计数
uint8_t check_clear_servo_encode_pluse(servo_type *servo)
{
	uint8_t i;
	uint8_t result;
	int pluse;
	uint16_t read_buf[3];
	
	result =0;
	for(i=0;i<10;i++)
	{
		rt_thread_delay(50);
		clear_encode_pluse(servo);
    servo_read_mutly_regs(servo->id,0X0520,read_buf,2);
		pluse = read_buf[0];
	  pluse+= read_buf[1]<<16;
		if(pluse ==0)
		{
			result = 1;
			break;
		}
	}
  return result;
}

// 复位原点的时候检测是否已经复位到原点了
void check_servo_reset_origin_is_ok(servo_type *servo,uint8_t clear_flag)
{
	uint8_t i;
  uint16_t read_buf[3];
	int pluse;
	servo->run_time = count_use_time(servo->start_time);
	// 判断是否检测到原点引脚电平是否是高电平，并且上一个原点引脚的状态是低电平(通过这种判断回到原点了)
	if((get_ext_pin(servo->origin_pin)==IS_OPEN)&&(servo->run_time>10)&&(servo->up_origin_pin ==IS_CLOSE))
	{
		
	    DEBUG_INFO("reset origin is ok id=%d time=%d\n",servo->id,servo->run_time);
		servo->state = SERVO_IDLE;												// 伺服是空闲状态
		servo_write_reg(servo->id,START_ROUT_PATH,1000);	// 伺服停止
		servo_write_reg(servo->id,START_ROUT_PATH,0);			// 伺服回到原点
		servo->reset_flag =  RT_TRUE;											// 伺服要复位
		if(clear_flag)																		// 如果要清零伺服编码器的数据则清零
		{
			for(i=0;i<10;i++)
			{
			  rt_thread_delay(50);
			  clear_encode_pluse(servo);
        servo_read_mutly_regs(servo->id,0X0520,read_buf,2);
			  pluse = read_buf[0];
	      pluse+= read_buf[1]<<16;
			  if(pluse ==0) break;
			}
			DEBUG_WARN("清零寄存器 i=%d servo id=%d pluse =%d\n",i,servo->id,pluse);
		}
		if(servo->id ==1)															// 包装伺服则当前步骤设置第0步，并设置包装伺服的高速比较参数
		{
		  servo->current_step =0;
			set_check_tray_part(&ch1_device);			
		}
		else if(servo->id ==2)												// 包装伺服则当前步骤设置第0步，并设置包装伺服的高速比较参数
		{
		  servo->current_step =0;
			set_check_tray_part(&ch2_device);
		}
		else if(servo->id ==3)												// 接收伺服则当前步骤设置第0步，并设置接收伺服为空闲状态
		{
			servo->state = SERVO_IDLE;
			servo->cycle_step =0;
		}
		else if(servo->id ==4)												// 接收伺服则当前步骤设置第0步，并设置接收伺服为空闲状态
		{
		  servo->state = SERVO_IDLE;
			servo->cycle_step =0;
		}
	}
	 servo->up_origin_pin = get_ext_pin(servo->origin_pin); // 记录每次读到的原点引脚的状态
}


void jump_to_nex_tray(ch_device_type *device)
{
	 uint8_t path;
	 device->tray_flag =get_tray_flag(device->device_id);
	 if(device->tray_flag==0) //没有托盘
	 {
	   if(device->packge_servo.state ==SERVO_IDLE)
		 {
			 if(device->packge_servo.current_step == 0) //走一个加首排落蛋位置,包装的packge_servo.current_step步骤，只有初始化或者调试的时候是为0的，其他的时候都在1和最大步骤加1之间跳转
			 {																					// 例如5排的蛋托
																									// 6个步骤
																									// 第1个参数：原点到第一排的距离脉冲(即第一个蛋托的开始位置？)
																									// 第2个参数：第一排到第二排的距离脉冲
																									// 第3个参数：第二排到第三排的距离脉冲
																									// 第4个参数：第三排到第四排的距离脉冲
																									// 第5个参数：第四排到第五排的距离脉冲
																									// 第6个参数：第五排到这个蛋托的最后位置的距离脉冲+第二个蛋托开始到第二个蛋托的第一排的距离脉冲
																									// 开始的时候，current_step=0
																									// current_step=0，原点到第一排蛋托
																									// current_step=1，第1排到第2排蛋托
																									// current_step=2，第2排到第3排蛋托
																									// current_step=3，第3排到第4排蛋托
																									// current_step=4，第4排到第5排蛋托
																									// current_step=5，第5排到第二个蛋托的第1排蛋托
																									// 下一次没有原点到第一排了，是直接第一排到第二排
         	device->packge_servo.hope_run_path= JUMP_TO_NEX_BOX_BASE_PATH + device->packge_servo.cycle_step;	// 现在是第0步，则直接走一整个蛋托的距离？
					device->packge_servo.state = SERVO_HOPE_RUN_PATH;
					device->packge_servo.current_step =1;  
			 }
			 else
			 {
				  path = device->packge_servo.current_step -1;													// 比如现在是第4步，走一整个蛋托的距离，则是走到下一个蛋托的第3？
         	device->packge_servo.hope_run_path= JUMP_TO_NEX_BOX_BASE_PATH + path;
					device->packge_servo.state = SERVO_HOPE_RUN_PATH;
					device->packge_servo.current_step =1;  
			 }
			 DEBUG_WARN("没有托盘走下一个托盘位step=1\n");
		 }
	 }
	 else 
	 {
		 // 包装的packge_servo.current_step步骤，只有初始化或者调试的时候是为0的，其他的时候都在1和最大步骤加1之间跳转
		 // 说明这个函数是在设备开始的时候，去走一个蛋托的距离
		 // 其他正常运行的时候，会按照步骤走的
	   if((device->packge_servo.current_step ==0)&&(device->packge_servo.state ==SERVO_IDLE))
		 {
			device->packge_servo.current_step = 1;
			device->packge_servo.state = SERVO_HOPE_RUN_PATH;
			device->packge_servo.hope_run_path=1;
	   }
	 }
} 


extern void sync_control_state(void);

//不检测有没有蛋托就让步进电机运行
void recive_cup_run_not_check_tray(ch_device_type *device)
{
	// 让步进电机带动蛋杯开
	if(device->recive_servo.cup_open_flag ==RT_TRUE)
	{
		
		if(0)//(device->tray_flag ==0)
		{
		  DEBUG_CARE(" 没有托盘!!!!!!!!!!!!!\n");
		}
		//不管托盘是否到位都开启接受抓了
		else if(device->packge_servo.state == SERVO_IDLE)
		{
//			DEBUG_CARE("这里要改\n");
			set_recive_moto(device->cup_id,CUP_IS_OPEN);
			device->recive_servo.cup_open_flag = RT_FALSE;
		}
		//else if(device->packge_servo.state != SERVO_IDLE)
		//{
		//  rt_kprintf("pack busy!!!\n");
		//}
	}
	// 让步进电机带动蛋杯关
	else if(device->recive_servo.cup_close_flag ==RT_TRUE)
	{
		set_recive_moto(device->cup_id,CUP_IS_CLOSE);
		device->recive_servo.cup_close_flag = RT_FALSE;
		
		device->recive_cup_empty_id ++;
		//clear_recive_cup_egg_buf(device->device_id);   //丢蛋 清空接收抓蛋杯
	  sync_control_state();
	}
}
//当接收爪打开标志为1(接收伺服下降到设置的位置后会将接收爪打开标志置为1)
	// 没有蛋托的话，打印
	// 有蛋托则接收爪步进电机打开
// 当接收爪关闭标志为1(接收伺服上升到设置的位置后会将接收爪关闭标志置为1)
	// 接收爪步进电机关闭
void recive_cup_run(ch_device_type *device)
{
	if(device->recive_servo.cup_open_flag ==RT_TRUE)
	{
		
		if(device->tray_flag ==0)	// 没有蛋托的时候，不停机报警，只是打印了一下，本来已经有6个蛋了该打开放到托盘，现在不放
		{
		  DEBUG_CARE(" 没有托盘!!!!!!!!!!!!!\n");
		}
		//不管托盘是否到位都开启接受抓了
		else if(device->packge_servo.state == SERVO_IDLE)
		{
//			DEBUG_CARE("这里要改\n");
			set_recive_moto(device->cup_id,CUP_IS_OPEN);
			device->recive_servo.cup_open_flag = RT_FALSE;
		}
		//else if(device->packge_servo.state != SERVO_IDLE)
		//{
		//  rt_kprintf("pack busy!!!\n");
		//}
	}
	// 蛋杯闭合的时候，肯定不用管有没有蛋托
	else if(device->recive_servo.cup_close_flag ==RT_TRUE)
	{
		set_recive_moto(device->cup_id,CUP_IS_CLOSE);
		device->recive_servo.cup_close_flag = RT_FALSE;
	}
}
// 接收伺服往上即回到原点然后包装伺服走一排
// 接收伺服上升后会将device->recive_servo.packge_move_flag标志置为1，表示包装通道可以运行
void recive_up_and_packge_run(ch_device_type *device)
{
	if(device->recive_servo.packge_move_flag ==RT_TRUE)		// 蛋托可以往前移动
	{
		device->recive_servo.packge_move_flag = RT_FALSE;
		if(device->packge_servo.state ==SERVO_IDLE)	// 包装伺服空闲的时候即没有运行的时候
		{
			device->packge_servo.current_step++;			// 包装运行的pr数据组加1
//			DEBUG_CARE("run step=%d cycle step=%d\n",device->packge_servo.current_step,device->packge_servo.cycle_step);
			if(device->packge_servo.current_step>(device->packge_servo.cycle_step))
			{
				device->packge_servo.hope_run_path=device->packge_servo.current_step;	// 执行一步
				device->packge_servo.state = SERVO_HOPE_RUN_PATH;
				device->packge_servo.current_step =1;																	// 因为是一个包装的最后一步，执行完后重新执行第一个pr	
			}
			else 
			{
				device->packge_servo.hope_run_path=device->packge_servo.current_step;	// 执行一步
				device->packge_servo.state = SERVO_HOPE_RUN_PATH;
			}
		}
		else 
		{
			set_alarm(device->device_id,transport_server_place_err,1);
			DEBUG_ERR("包装通道搞不赢 step=%d state=%d\n",device->packge_servo.current_step,device->packge_servo.state);
		}
	}
}
static rt_tick_t u_tick=0;

#define RECIVE_SERVO_GO_UP_DELAY_TIME  5
#define RECIVE_SERVO_GO_UP_DELAY_STEP  20
// 如果伺服电机运行某个路径的时候完成了，这个时候接收爪打开标志又是为1的时候
// 一般下降的时候将接收爪打开标志置为为1，说明下降到位了可以打开接收爪了
void set_cup_open_signal(uint8_t ch)
{
  rt_base_t level;
	
	
	 level = rt_hw_interrupt_disable();
    
   if(ch ==0)
	 {
	   if(ch1_device.cup_en ==  1)
		 {
			 ch1_device.cup_step_count |=0x01;		// 理论上这个时候ch1_device.cup_step_count为0，或上1则此时为1
			 ch1_device.cup_en = RT_FALSE;
			 ch1_device.recive_servo.cup_open_flag =RT_TRUE;	// 这个时候就要将接收爪置为打开
		 }
	 }
	 else 
	 {
	   if(ch2_device.cup_en ==  1)
		 {
			 ch2_device.cup_step_count |=0x01;
			 ch2_device.cup_en =RT_FALSE;
			 ch2_device.recive_servo.cup_open_flag =RT_TRUE;
		 }
	 }
	  if(ch ==0)
		{
			recive_cup_run(&ch1_device);
		}
		else 
		{
		  recive_cup_run(&ch2_device);
		}
   rt_hw_interrupt_enable(level);

}
// 接收伺服单步运行控制逻辑
uint8_t recive_servo_run_netx_step(ch_device_type *device)
{
	  uint8_t result=0;
		if((device->recive_servo.current_step==0)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			device->recive_servo.start_time = rt_tick_get();
			device->recive_servo.state = SERVO_HOPE_RUN_PATH;
			device->recive_servo.hope_run_path=1;
			device->recive_servo.current_step = 1;
			device->cup_en =  1;
		}
		else if((device->recive_servo.current_step==1)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			//rt_kprintf("recive servo run step 1 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
			device->recive_servo.start_time = rt_tick_get();
			device->recive_servo.current_step=RECIVE_SERVO_GO_UP_DELAY_STEP;
			device->recive_servo.setp_delay_time = rt_tick_get();
		}
		else if((device->recive_servo.current_step==RECIVE_SERVO_GO_UP_DELAY_STEP)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			
			if(count_use_time(device->recive_servo.setp_delay_time)>RECIVE_SERVO_GO_UP_DELAY_TIME)
			{
				//rt_kprintf("recive servo run step 100 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
				device->recive_servo.start_time = rt_tick_get();
			  device->recive_servo.state = SERVO_HOPE_RUN_PATH;
			  device->recive_servo.hope_run_path=2;
			  device->recive_servo.current_step = 2;
			  device->packge_move_en= RT_TRUE;
				device->cup_en =  2;
			}
		}
		else if((device->recive_servo.current_step==2)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			//rt_kprintf("recive servo run step 2 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
			result = 1;
			device->recive_servo.current_step = 0;
		}
		if(device->recive_servo.current_step==1)  //接收爪下降
		{
			device->recive_servo.move_distem = get_current_distem(device->recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
	//	  rt_kprintf("tick=%d recive len=%d\n",rt_tick_get()-u_tick,device->recive_servo.move_distem);
			u_tick =rt_tick_get();
			if(device->recive_servo.move_distem>=device->product_packge.recive_claw_open_position)
			{
				if(device->cup_en ==  1)
				{
					//device->cup_en = RT_FALSE;
					//device->recive_servo.cup_open_flag =RT_TRUE;
				}
			}
		}
		else if(device->recive_servo.current_step==2)  //接收爪上升
		{
		  device->recive_servo.move_distem = get_current_distem(device->recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
	//	  rt_kprintf("tick=%d recive len=%d\n",rt_tick_get()-u_tick,device->recive_servo.move_distem);
			u_tick =rt_tick_get();
			if(device->recive_servo.move_distem<=device->product_packge.recive_claw_close_position)
			{
				if(device->cup_en ==  2)
				{
					device->cup_en = RT_FALSE;
					device->recive_servo.cup_close_flag =RT_TRUE;
				}
			}
			
			if(device->recive_servo.move_distem<=device->product_packge.transport_start_position)  //接收爪上升 到了指定位置 包装可以启动下一个路径
			{
				if(device->packge_move_en==RT_TRUE)
				{
					device->packge_move_en= RT_FALSE;
				  device->recive_servo.packge_move_flag =RT_TRUE;
					
					// DEBUG_CARE("STEP=%d\n",device->packge_servo.current_step);
				}
			}
		}
//		rt_kprintf("result =%d\n",result);
		return result;
}

void check_recive_servo_fun_count(ch_device_type *device)
{
		// 上一个下降放蛋上升的过程完成后，cup_step_count这个标志肯定为3
		// 如果不为3则说明是伺服在这个过程运行有问题？
   	if(device->cup_step_count !=0x03)
		{
			   set_alarm(device->device_id,egg_cup_step_moto_err,1);
			   DEBUG_ERR("DEVICE ID=%d cup setp err step=%02x\n",device->device_id,device->cup_step_count);
		}
		if(device->transport_count !=0x01)
		{
			   set_alarm(device->device_id,transport_server_origin_err,1);
			   DEBUG_ERR("DEVICE ID=%d transport setp err step=%02x\n",device->device_id,device->transport_count);
		}
}
// 清除蛋杯步骤数
// 清除传输通道蛋杯个数？
// 判断接收伺服当前的位置，根据接收伺服当前脉冲值得到圈数，圈数乘以每圈的距离得到当前的距离
void clear_recive_servo_fun_count(ch_device_type *device)
{
   	device->cup_step_count=0;
		device->transport_count=0;
	  if(device->recive_servo.move_distem>=5)
		{
      device->recive_servo.move_distem = get_current_distem(device->recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
			if(device->recive_servo.move_distem>=5)
			{
				set_alarm(device->device_id,egg_cup_step_moto_err,1);
				DEBUG_ERR("接受抓原点错误\n");
			}
		}
}
// 接收伺服运行控制逻辑
void recive_servo_run(ch_device_type *device)
{
	  uint8_t next_tray;
		if((device->recive_servo.current_step==0)&&(device->recive_servo.state ==SERVO_IDLE)&&(device->recive_egg)&&(device->tray_flag>0))
		{
      next_tray = get_nex_tray_flag(device->device_id);
			if((device->packge_servo.current_step== 1)&&(device->packge_servo.hope_run_path !=0xff)&&(next_tray==0))
			{
				rt_kprintf("waite tary\n");
			}
			else 
			{
				 clear_recive_servo_fun_count(device);						// 先判断接收伺服的位置是不是离原点比较远，远就报警
			   device->recive_egg =0; 													// 接收蛋标志置为0
				 device->recive_servo.start_time = rt_tick_get();	// 接收伺服执行下降的开始时间
			   device->recive_servo.state = SERVO_HOPE_RUN_PATH;
			   device->recive_servo.hope_run_path=1;
			   device->recive_servo.current_step = 1;
			   device->cup_en =  1;
				
				// 这个步骤后接收伺服就会运行，运行完就会变成SERVO_IDLE
			}
		}
		// 判断当前步骤是不是1，并且伺服是不是是空闲状态(是空闲状态，说明伺服下降完成了)
		else if((device->recive_servo.current_step==1)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			//rt_kprintf("recive servo run step 1 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
			device->recive_servo.start_time = rt_tick_get();	// 接收伺服执行上升的开始时间
			device->recive_servo.current_step=RECIVE_SERVO_GO_UP_DELAY_STEP;
			device->recive_servo.setp_delay_time = rt_tick_get();
			
			// 设置伺服步骤为等待延迟上升的状态
		}
		// 判断伺服是不是为等待延迟上升的状态
		else if((device->recive_servo.current_step==RECIVE_SERVO_GO_UP_DELAY_STEP)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			
			// 计算等待的时间到没有
			if(count_use_time(device->recive_servo.setp_delay_time)>RECIVE_SERVO_GO_UP_DELAY_TIME)
			{
			
				//rt_kprintf("recive servo run step 100 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
				if(device->packge_servo.state ==SERVO_IDLE)
				{
					// 时间到了，判断拖的什么东西
					// 如果满足了则进入到步骤2即上升
					if(device->product_packge.use_tray_count !=2)
					{
						device->recive_servo.start_time = rt_tick_get();
						device->recive_servo.state = SERVO_HOPE_RUN_PATH;
						device->recive_servo.hope_run_path=2;
						device->recive_servo.current_step = 2;
						device->packge_move_en= RT_TRUE;		// 进入上升的时候，包装伺服也可以向前移动
						device->cup_en =  2;
						device->recive_cup_empty_id ++;		// 结合主控程序，这个变量相当于包序号的意思，主控收到这个和自己的判断，不一样，说明收到了包装通道的新的数据包
						clear_recive_cup_egg_buf(device->device_id);   // 将A通道或者B通道的6个电磁铁踢蛋有无蛋的标志都清零
						sync_control_state();		// 同步一下信息给主控,里面有A通道或者B通道,6个接收爪各自有没有蛋，该变量是cup_eggs
					}
					else 
					{
						// 如果不满足了则等待的时间是不是超过70ms
						// 超过70ms还没有满足，不等了，直接上升
						if(count_use_time(device->recive_servo.start_time)>70)
						{
							device->recive_servo.state = SERVO_HOPE_RUN_PATH;
							device->recive_servo.hope_run_path=2;
							device->recive_servo.current_step = 2;
							device->packge_move_en= RT_TRUE;
							device->cup_en =  2;
							device->recive_cup_empty_id ++;
							clear_recive_cup_egg_buf(device->device_id);   //丢蛋 清空接收抓蛋杯
							sync_control_state();
						}
					}
				}
				else
				{
				  rt_kprintf("包装通道为什么在忙???\n");
					device->recive_servo.setp_delay_time = rt_tick_get();
					device->recive_servo.start_time = rt_tick_get();
				}
			}
		}
		// 当前是步骤2，??蛋杯步骤是不是3，不是3则报警
		else if((device->recive_servo.current_step==2)&&(device->recive_servo.state ==SERVO_IDLE))
		{
			check_recive_servo_fun_count(device);
			//rt_kprintf("recive servo run step 2 use time=%d\n",rt_tick_get() - device->recive_servo.start_time);
			device->recive_servo.current_step = 0;
		}
		// 如果上面都不满足，但是要接蛋，则等待
		else if(device->recive_egg)
		{
			 if(device->tray_flag==0)
			 {
			   DEBUG_CARE("wait tray\n");
				 rt_thread_mdelay(50);
			 }
			 else
			 {
			   DEBUG_CARE("recive busy ch=%d step=%d state=%d\n",device->recive_servo.id,device->recive_servo.current_step,device->recive_servo.state);
			 }
		}

		// 接收伺服下降
		// 判断下降的距离是不是超过设置的下降距离
		if(device->recive_servo.current_step==1)  //接收爪下降
		{
			device->recive_servo.move_distem = get_current_distem(device->recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
		 // rt_kprintf("tick=%d recive len=%d\n",rt_tick_get()-u_tick,device->recive_servo.move_distem);
			u_tick =rt_tick_get();
			if(device->recive_servo.move_distem>=device->product_packge.recive_claw_open_position)
			{
				//device->cup_step_count |=0x01;
				if(device->cup_en ==  1)
				{
				//	device->cup_en = RT_FALSE;
				//	device->cup_step_count |=0x01;
					//rt_kprintf("cup open \n");
				//	device->recive_servo.cup_open_flag =RT_TRUE;
				}
			}
		}
		// 接收爪上升
		// 判断伺服这个时候的距离是不是小于等于设置的上升的最小距离(离原点最小的距离)
		else if(device->recive_servo.current_step==2)  //接收爪上升
		{
		  device->recive_servo.move_distem = get_current_distem(device->recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
		 // rt_kprintf("tick=%d recive len=%d\n",rt_tick_get()-u_tick,device->recive_servo.move_distem);
			u_tick =rt_tick_get();
			// 上升到了关闭蛋杯的位置的时候
			// 这个时候则可以关闭接收爪(步进电机)蛋杯
			if(device->recive_servo.move_distem<=device->product_packge.recive_claw_close_position)
			{
				if(device->cup_en ==  2)
				{
					device->cup_en = RT_FALSE;
					//rt_kprintf("cup close\n");
					
					device->cup_step_count +=0x02;
					device->recive_servo.cup_close_flag =RT_TRUE;
				}
			}
			// 上升到了可以移动包装的位置的时候
			// 这个时候则包装伺服可以移动包装
			if(device->recive_servo.move_distem<=device->product_packge.transport_start_position)  //接收爪上升 到了指定位置 包装可以启动下一个路径
			{
				if(device->packge_move_en==RT_TRUE)		// 当接收伺服上升到指定的距离后并且包装伺服是可以移动
				{
					device->packge_move_en= RT_FALSE;
				  device->recive_servo.packge_move_flag =RT_TRUE;
					device->transport_count+=1;
					//rt_kprintf("pack start\n");
				}
			}
		}
}
// P5-56=50   比较数据起始地址
// P5-57=5400 初始值
// P5-58=1    比较数量
// P5-59=0x001E003B

#define P5_11_ADDR 0x516
#define P5_12_ADDR 0x518

#define P5_56_ADDR 0x570
#define P5_57_ADDR 0x572
#define P5_58_ADDR 0x574
#define P5_59_ADDR 0x576   



// 设置伺服比较参数
// 设置某个路径的参数，当运行该路径时，运行完了，有个引脚会输出一段时间高电平，这个高电平的时间可以设置
rt_err_t set_servo_cmp_dat(uint8_t id,uint8_t offest,uint32_t val)
{
	rt_err_t result;
  uint32_t dat;
	dat = offest;
  result = servo_write_mutly_regs(id,P5_11_ADDR,(uint16_t *)&dat,2);  //关闭比较功能
  dat = val;
  result = servo_write_mutly_regs(id,P5_12_ADDR,(uint16_t *)&dat,2);  //关闭比较功能
	return result; 
}
// 读取伺服比较参数
rt_err_t read_servo_cmp_dat(uint8_t id,uint8_t offest,uint32_t *val)
{
	rt_err_t result;
  uint32_t dat;
	dat = offest;
  result = servo_write_mutly_regs(id,P5_11_ADDR,(uint16_t *)&dat,2);  //关闭比较功能
  result = servo_read_mutly_regs(id,P5_12_ADDR,(uint16_t *)&dat,2);  //关闭比较功能
	*val = dat;
	return result; 
}

// 设置接收伺服的高速比较参数
rt_err_t set_servo_open_cup_part(ch_device_type *device)  
{
	uint8_t i;
	rt_err_t result;
    uint32_t dat=0x001E003A;
	uint16_t position;
	uint16_t fall_down_posetion;
	int16_t  set_pluse=0;
	uint32_t *dat_buf;
	uint32_t per_mm_to_pluse;
	uint32_t check_pluse;
	rt_base_t level;
	float buf;
	
	result = 1;
	level = rt_hw_interrupt_disable();
	position = device->product_packge.recive_claw_open_position; 									// device->product_packge.recive_claw_open_position=70
	fall_down_posetion = device->product_packge.recive_claw_fall_dowm_position;		// device->product_packge.recive_claw_fall_dowm_position=99
	rt_hw_interrupt_enable(level);
	if(position>fall_down_posetion)
	{
		DEBUG_ERR("PACKGE SET  ERR CUP OPEN PISITION HAVE MAX\n");
		return RT_ERROR;
	}
	
	per_mm_to_pluse = (RECIVE_SERVO_CYCLE_SETP * 100)/RECIVE_CLAW_LEN;	// 接收伺服一圈是3200个脉冲，一圈是100mm，那么每mm是per_mm_to_pluse这么个脉冲，这里扩大了100倍
	check_pluse = (per_mm_to_pluse*position); //扩大了100								// 打开的位置乘以	每个mm多少个脉冲得到走这么长的距离需要多少个脉冲
	check_pluse = check_pluse/277;    // 每2.77个脉冲= 1个CMP值					// 
	
	if(device->device_id == CH1_DEVICE_ID)
	{
		if(CH1_RECIVE_DIR !=SERVO_FORWORD)
		{
		   check_pluse = 0-check_pluse;
		}
	}
	
	if(device->device_id == CH2_DEVICE_ID)
	{
		if(CH2_RECIVE_DIR !=SERVO_FORWORD)
		{
		   check_pluse = 0-check_pluse;
		}
	}
	// 设置接收伺服第50个比较数据的内容(其实只需要比较接收伺服下降的最大位置脉冲即可即只需要比较一个位置的数据)
	set_pluse = check_pluse;
	set_servo_cmp_dat(device->recive_servo.id,50,set_pluse);			// 设置高速比较，地址是50(下面设置了阵列开始地址是50)，数据是set_pluse
	read_servo_cmp_dat(device->recive_servo.id,50,&check_pluse);	// 读取高速比较设置，地址是50(下面设置了阵列开始地址是50)，数据是check_pluse
	if(check_pluse !=set_pluse)																		// 如果不相等则设置失败
	{
		DEBUG_ERR("set cmp pluse fall write dat=%d read dat=%d\n",set_pluse,check_pluse);
		return RT_ERROR;
	}
	
	// 通过 P5.056 设置数据数组的起始地址（例如，设为8，就从第8号数组位置开始读取）
	// 通过 P5.058 设定需要比较的数据总数（数量必须大于0）
	// 通过 P5.059 来启用高速位置比较功能
	// 驱动器会实时将轴的位置与设定的数据进行硬件级比较，一旦满足条件，就会通过高速数字输出引脚DO4（响应时间仅5微秒）立即输出一个信号，用于实现精准的同步控制或触发其他动作
	
	// 设置接收伺服的比较参数
	// 这个参数在接收伺服中固定，并且比较参数也只有1个
	dat_buf = rt_malloc(30);
	if(dat_buf ==RT_NULL)  return RT_ERROR;
	rt_memset(dat_buf,0,30);
	servo_write_mutly_regs(device->recive_servo.id,P5_59_ADDR,(uint16_t *)&dat,2);  // dat=0x001E003A  0001 1110 0000 0000 0011 1010(根据手册可知，设置的是输出脉冲时间30)
																																									// 比较轴来源是CN2，触发逻辑为常开，执行完成后自动执行pr45，触发输出脉冲持续时间30ms
	// 因为上面设置接收伺服第50个比较数据的内容，所以这里比较就从第50开始比较
	dat = 50;
	servo_write_mutly_regs(device->recive_servo.id,P5_56_ADDR,(uint16_t *)&dat,2);	// 比较阵列开始地址是50
	dat = 0;
	servo_write_mutly_regs(device->recive_servo.id,P5_57_ADDR,(uint16_t *)&dat,2);	// 轴位置是0，
	// (其实只需要比较接收伺服下降的最大位置脉冲即可即只需要比较一个位置的数据)
	dat = 1;
	servo_write_mutly_regs(device->recive_servo.id,P5_58_ADDR,(uint16_t *)&dat,2);	// 比较数量，这里设置为1个
	dat = 0x001E0033;																																// 0x001E0033 0001 1110 0000 0000 0011 0011(根据手册可知，设置的启动比较并且为循环模式，是输出脉冲时间30)
																																									// 比较轴来源是CN2，触发逻辑为常开，执行完成后自动执行pr45，触发输出脉冲持续时间30ms
	servo_write_mutly_regs(device->recive_servo.id,P5_59_ADDR,(uint16_t *)&dat,2);
	
	// 读取并打印设置的值
	for(i=0;i<4;i++)
	{
	  servo_read_mutly_regs(device->recive_servo.id,P5_56_ADDR+i*2,(uint16_t*)&dat_buf[i],2);
	}
  for(i=0;i<3;i++)
	{
	 DEBUG_INFO("set cup open position cmp i=%d reg=%d\n",i,dat_buf[i]);
	}
	
	DEBUG_INFO("set cup open position  cmp i=%d reg=%08x\n",3,dat_buf[3]);
	DEBUG_WARN("set cup open position =%dmm  cmp dat=%d\n",position,set_pluse);
	if(position == device->product_packge.recive_claw_open_position)
	{
		// 读到的是和设置一样则打印
		if((dat_buf[0]==50)&&(dat_buf[1]==0)&&(dat_buf[2]==1)&&(dat_buf[3]==0x001E0033))
		{
		  device->reset_cup_open_position_flag =0;		// 重新设置接收伺服打开的时候的位置伺服参数标志清零
			DEBUG_CARE("SET CUP OPEN POSITION IS SUCESS\n");
			result =0;
		}
	}
	rt_free(dat_buf);
	
	return result;
}


// 设置接收伺服的高速比较参数
// 只能在设备处于原点状态，并且接收伺服为空闲状态
// 并且接收伺服处于原点
// 才能写接收伺服的参数
rt_err_t set_open_cup_part(ch_device_type *device)
{
  uint8_t i;
	uint8_t result;
	
	if((device->origin_flag ==RT_TRUE )&&(device->recive_servo.current_step==0)&&(device->recive_servo.state==SERVO_IDLE))
	{
		if(get_ext_pin(device->recive_servo.origin_pin)==IS_OPEN)
		{
			for(i=0;i<3;i++)
			{
				result = set_servo_open_cup_part(device);
				if(result ==0) break;
			}
		}
	}
	return result;
}


// 设置包装伺服的高速比较参数
// 7200 是写死在EEPROM 里面
rt_err_t set_servo_check_tray_part(ch_device_type *device)  
{

	
	uint8_t i;
	rt_err_t result;
  uint32_t dat=0x001E003A;
	uint16_t position;
	uint32_t  set_pluse=0;
	uint32_t *dat_buf;
	uint32_t per_mm_to_pluse;
	uint32_t check_pluse;
	rt_base_t level;
	int32_t cmp_dat;
	int32_t read_buf;
	float buf;
	
	result = 1;
	level = rt_hw_interrupt_disable();
	position = device->product_packge.check_egg_tray_position;	// device->product_packge.check_egg_tray_position=80
	rt_hw_interrupt_enable(level);
	if(position>PACKGE_BOX_LEN)
	{
		DEBUG_ERR("CHECK TRAY POSITION ERR\n");
		return RT_ERROR;
	}
	per_mm_to_pluse = (PACKGE_SERVO_CYCLE_SETP * 100)/PACKGE_BOX_LEN;	// 包装伺服一圈20000个脉冲，一圈是355mm则算出每个mm是多少个脉冲
	check_pluse = (per_mm_to_pluse*position)/100;											// 距离乘以每个距离的脉冲得到走这么长的距离需要多少个脉冲
	
	buf = (float)check_pluse/PACKGE_SERVO_CYCLE_SETP;									// 算出要走多少圈
	check_pluse = (int)(buf*7200);																		// 乘以7200是什么意思？

	cmp_dat = 7200;																										// 比较值是7200

	
	if(device->device_id == CH1_DEVICE_ID)
	{
		if(CH1_PACKGE_DIR !=SERVO_FORWORD)
		{
		   cmp_dat = -7200;																							// 反向则比较值为0										
		}
	}
	if(device->device_id == CH2_DEVICE_ID)
	{
		if(CH2_PACKGE_DIR !=SERVO_FORWORD)
		{
		   cmp_dat = -7200;																							// 反向则比较值为0	
		}
	}
	
	// 这个高速比较值只是用来在这个位置检查后面是不是有蛋托
	// 就大概在每圈7200脉冲位置检查有没有蛋托
	set_servo_cmp_dat(device->packge_servo.id,50,cmp_dat);						// 设置比较值
	read_servo_cmp_dat(device->packge_servo.id,50,(uint32_t*)&read_buf);
	if(cmp_dat !=read_buf)
	{
		DEBUG_ERR("set cmp pluse fall write dat=%d read dat=%d\n",cmp_dat,read_buf);
		return RT_ERROR;
	}
	//DEBUG_INFO("EEP ROM cmp dat=%d read dat=%d\n",cmp_dat,read_buf);
	set_pluse = 7200 - check_pluse;

	if(device->device_id == CH1_DEVICE_ID)  //通道2 的这个值是负数
	{
		if(CH1_PACKGE_DIR !=SERVO_FORWORD)
		{
		  set_pluse = 0 - set_pluse;
		}
	}
	
	if(device->device_id == CH2_DEVICE_ID)  //通道2 的这个值是负数
	{
		if(CH2_PACKGE_DIR !=SERVO_FORWORD)
		{
		  set_pluse = 0 - set_pluse;
		}
	}
	set_tray_flag(device->device_id);
	dat_buf = rt_malloc(30);
	if(dat_buf ==RT_NULL)  return RT_ERROR;
	rt_memset(dat_buf,0,30);
	
	servo_write_mutly_regs(device->packge_servo.id,P5_59_ADDR,(uint16_t *)&dat,2);  //关闭比较功能
	dat = 50;
	servo_write_mutly_regs(device->packge_servo.id,P5_56_ADDR,(uint16_t *)&dat,2);
	
	dat = set_pluse;
	servo_write_mutly_regs(device->packge_servo.id,P5_57_ADDR,(uint16_t *)&dat,2);
	dat = 1;
	servo_write_mutly_regs(device->packge_servo.id,P5_58_ADDR,(uint16_t *)&dat,2);
	dat = 0x001E003B;																															// 0x001E003B 0001 1110 0000 0000 0011 1011(根据手册可知，设置的启动比较并且为循环模式且比较完了后比较轴位置归零，是输出脉冲时间30)
																																									// 比较轴来源是CN2，触发逻辑为常开，执行完成后自动执行pr45，触发输出脉冲持续时间30ms
	servo_write_mutly_regs(device->packge_servo.id,P5_59_ADDR,(uint16_t *)&dat,2);
	
	for(i=0;i<4;i++)
	{
	  servo_read_mutly_regs(device->packge_servo.id,P5_56_ADDR+i*2,(uint16_t*)&dat_buf[i],2);
	}
    for(i=0;i<3;i++)
	{
	 //DEBUG_INFO("set check tary cmp i=%d reg=%d\n",i,dat_buf[i]);
	}
	//DEBUG_INFO("set check tary cmp i=%d reg=%08x\n",3,dat_buf[3]);
	DEBUG_WARN("check tray position=%dmm  cmp dat=%d\n",position,set_pluse);
	if(position == device->product_packge.check_egg_tray_position)
	{
		if((dat_buf[0]==50)&&(dat_buf[1]==set_pluse)&&(dat_buf[2]==1)&&(dat_buf[3]==0x001E003B))
		{
		  device->reset_check_tray_flag =0;	// 重新设置包装伺服蛋托检测位置伺服参数标志清零
			DEBUG_CARE("SET CHECK TRAY IS SUCESS\n");
			result =0;
		}
	}
	rt_free(dat_buf);
	
	return result;
}

// 设置包装伺服的高速比较参数
rt_err_t set_check_tray_part(ch_device_type *device)
{
  uint8_t i;
	uint8_t result;
	
	
	for(i=0;i<3;i++)
	{
	  result = set_servo_check_tray_part(device);
		if(result ==0) break;
	}
	return result;
}


// 这个在干什么
uint8_t calutare_recive_servo_down_egg_id(uint8_t line_count)
{
   uint8_t id;
	 if(line_count==0)
	 {
		 id=0;
	 }
	 else if(line_count==1)
	 {
		 id=0;
	 }
	 else if(line_count==2)
	 {
		 id=2;
	 }
	 else if(line_count==3)
	 {
		 id=0;
	 }
	 else if(line_count==4)
	 {
		 id=1;
	 }
	 else if(line_count==5)
	 {
		 id=0;
	 }
	 else if(line_count==6)
	 {
		 id=0;
	 }
	 return id;
}

// 计算包装的路径
// 包装信息变化了后，设备处于空闲状态，要将变化后的参数计算处理并写入到伺服路径参数里面
rt_err_t caculate_and_write_product_packge_run_path(ch_device_type *device)
{
	//PACKGE_BOX_SERVO_SETP
	uint8_t i,j;
	int32_t buf=0;
	int32_t buf1=0;
    uint32_t jump_next_path_dat[2];
	uint32_t read_jump_next_path_dat[2];
	uint32_t setp1_distem=0;   //首排落蛋位置
	uint32_t setp_end_distem=0;

	uint32_t egg_distem=0;
	uint32_t per_egg_distem=0;
	uint32_t per_mm_to_pluse;
	uint32_t pluse_buf=0;
	uint32_t *wrige_reg=RT_NULL;
	uint32_t *read_reg=RT_NULL;
	rt_base_t level;
	product_packge_info *packge;

	packge = rt_malloc(sizeof(product_packge_info));
	if(packge == RT_NULL)
	{
		DEBUG_CARE("mem is not abailable");
		return RT_ERROR ;
	}
	
	level = rt_hw_interrupt_disable();
	rt_memcpy(packge,&device->product_packge,sizeof(product_packge_info));
	rt_hw_interrupt_enable(level);
	
	i= calutare_recive_servo_down_egg_id(packge->line);		// packge->line 指这个蛋托一排有几个，当有6个的时候，用第几个电磁铁开始踢来
																												// 接收爪5-0,6个，但是托盘可能不是6个
																												// 6(5*6)个时候，max_ch=5，min_ch=0
																												// 3(2*3)个时候，max_ch=5，min_ch=3；也有可能是max_ch=3，min_ch=1等，也有可能是max_ch=2，min_ch=0等，看包装的位置
	
	
	set_servo_down_number(device->device_id,i);						// 根据蛋托一排的个数得到该用第几个电磁铁开始踢来设置电磁铁
	
	per_mm_to_pluse = (PACKGE_SERVO_CYCLE_SETP * 100)/PACKGE_BOX_LEN;	// 20000*100/355=5633即每mm是5633个脉冲，扩大了100？
   device->packge_servo.cycle_step = packge->row;		// 5行，包装伺服一圈执行的步骤数
   setp1_distem = packge->packge_line1_postion;			// 200
	
	if((packge->row<1)||(packge->row>MAX_PACKGE_ROW))
	{
		rt_free(packge);
		DEBUG_ERR("包装盒信息错误\n");
		return RT_ERROR ;
	}
  
	wrige_reg = rt_malloc((MAX_PACKGE_ROW+2)*8);
	
	if(wrige_reg == RT_NULL)
	{
		rt_free(packge);
		DEBUG_CARE("mem is not abailable");
		return RT_ERROR ;
	}
	// 第0到第row-1的参数信息
	// 第一个参数是加减速信息
	// 第二个参数是距离转换为脉冲的信息
	
	// SERVO_RUN_PART_PAR0 0x000C5582
	// 1100 0101 0101 1000 0010
	// 1100:目标速度 12-P5.072-2000ms
	// 0101:减速时间 5-P5.025-900ms
	// 0101:加速时间 5-P5.025-900ms
	// 1000:INC增量定位 8-INC增量定位
	// 0010:路径形式 2-SINGLE定位控制，完毕则停止
	// SINGLE定位控制:Mode 2 (SINGLE)：单次定位。PR路径执行完成后即停止，不会自动执行下一段路径。
	wrige_reg[0] = SERVO_RUN_PART_PAR0;	// 0x000C5582
	pluse_buf = (setp1_distem*per_mm_to_pluse)/100;	// 200*5633/100(上面扩大了100这里缩小100)=11266
	wrige_reg[1] = (uint32_t)(pluse_buf);
	uint16_t *addr;
	addr = &packge->packge_line1_postion;
	for(i=1;i<packge->row;i++)  					// 1-4共4次运算
	{
		wrige_reg[i*2] = SERVO_RUN_PART_PAR0;
		pluse_buf = addr[i] - addr[i-1];		// 一次计算后一排落蛋的蛋托位置离前一排落蛋的蛋托位置之间的间隔(这个间隔相当于蛋托往前走一排走多少的距离)
		pluse_buf = (pluse_buf*per_mm_to_pluse)/100;
		wrige_reg[i*2+1] = (uint32_t)(pluse_buf);
	}
	// 求取总的包装距离脉冲
	for(i=0;i<(packge->row);i++)  
	{
		buf +=wrige_reg[i*2+1];							// 这算出移动这5次(200+30+30+30+30这5个数的和的距离然后转换为脉冲)一共需要走的脉冲数
	}																			// 320*5633/100=18025个脉冲
	// 判断这个脉冲是不是大于接收伺服转一圈的脉冲值，也就是一个蛋托几个步骤加起来的脉冲值不能大于接收伺服一圈的脉冲
	if(buf>PACKGE_SERVO_CYCLE_SETP)				// 这个基本上转一圈就移动一个蛋托，相当于第一次走200，后面都走30，相当于上一个拖的最后一排到本次的蛋托的第一排落蛋位置的距离是200mm
	{
		rt_free(packge);
		rt_free(wrige_reg);
		DEBUG_ERR("包装盒信息错误\n");
		return RT_ERROR ;
	}
	// 第1个参数：原点到第一排的距离脉冲(即第一个蛋托的开始位置？)
	// 第2个参数：第一排到第二排的距离脉冲
	// 第3个参数：第二排到第三排的距离脉冲
	// 第4个参数：第三排到第四排的距离脉冲
	// 第5个参数：第四排到第五排的距离脉冲
	// 第6个参数：第五排到这个蛋托的最后位置的距离脉冲+第二个蛋托开始到第二个蛋托的第一排的距离脉冲
	wrige_reg[packge->row*2] = SERVO_RUN_PART_PAR1;							// 0x000F5382 最后一步的加减速信息
	wrige_reg[packge->row*2+1] =PACKGE_SERVO_CYCLE_SETP - buf;	// 一圈走一个蛋托的距离，前面共走了18025个脉冲，这一圈中还剩下1957个没有走
	wrige_reg[packge->row*2+1] = wrige_reg[packge->row*2+1] +  wrige_reg[1] ;   //最后一步 和第一步合并，这个是什么意思？？
	
	for(i=0;i<(packge->row+1);i++)  
	{
		buf1= wrige_reg[i*2+1];
		
		if(device->device_id == CH1_DEVICE_ID)  //通道2 的这个值是负数
		{
			if(CH1_PACKGE_DIR !=SERVO_FORWORD)
			{
				wrige_reg[i*2+1] = 0-buf1;			// 能这样减吗？
			}
		}
		
		if(device->device_id == CH2_DEVICE_ID)  //通道2 的这个值是负数
		{
			if(CH2_PACKGE_DIR !=SERVO_FORWORD)
			{
				wrige_reg[i*2+1] = 0-buf1;			// 能这样减吗？
			}
		}
		set_servo_path_part(device->packge_servo.id,ROUT_PATH_PATH_START_ADDR+(i*4),(uint16_t *)&wrige_reg[i*2],4);
			
			// ROUT_PATH_PATH_START_ADDR  0x604  这个是以PR1的属性地址为基准，也就是下一个PR2的属性地址刚好就是0x604+4
			// 这个每次操作4个16位即2字节的数据共8个字节，而每一组PR的属性和值各4字节一共刚好8个字节
			// 这里一共操作了6次即设置了6个PR组，PR1-PR6，pr1设置的走到第一排落蛋位置的脉冲，pr2设置的是第一排走到第二排要走的位置的脉冲
			// pr3设置的是第二排走到第三排要走的位置的脉冲，pr4设置的是第三排走到第四排要走的位置的脉冲，pr5设置的是第四排走到第五排要走的位置的脉冲
			// pr6设置的是第五排走到下一个蛋托的第一排要走的位置的脉冲
		DEBUG_WARN("part=%08x setp%d=%d\n",wrige_reg[i*2],i,wrige_reg[i*2+1]);
	}
	
	read_reg = rt_malloc((MAX_PACKGE_ROW+2)*8);
	if(read_reg == RT_NULL)
	{
		rt_free(packge);
		rt_free(wrige_reg);
		DEBUG_CARE("mem is not abailable");
		return RT_ERROR ;
	}
	// 将刚刚设置的值读出来
	for(i=0;i<(packge->row+1);i++)                          //读取设置是否成功
	{
		read_servo_path_part(device->packge_servo.id,ROUT_PATH_PATH_START_ADDR+(i*4),(uint16_t *)&read_reg[i*2],4);
		//DEBUG_WARN("part=%08x setp%d=%d\n",read_reg[i*2],i,read_reg[i*2+1]);
	}
	// 比较读出的和设置的是不是一样
	if(rt_memcmp(read_reg,wrige_reg,(packge->row+1)*8)!=0)
	{
		rt_free(packge);
		rt_free(wrige_reg);
	  rt_free(read_reg);
		DEBUG_WARN("check path failed\n");
		return RT_ERROR ;
	}
	
	// 这下面是设置单步执行的时候的步骤的路径参数
	buf = wrige_reg[1];
	for(i=0;i<(packge->row+1);i++)  
	{
		buf1 =0;
		for(j=0;j<(i+1);j++)
		{
		  buf1+= wrige_reg[j*2+1];			// 第1次 buf1=第一排落蛋位置200
																		// 第2次 buf1=第一排落蛋位置+第一排到第二排的距离30
																		// 第3次 buf1=第一排落蛋位置+第一排到第二排的距离30+第二排到第三排的距离30
																		// 第4次 buf1=第一排落蛋位置+第一排到第二排的距离30+第二排到第三排的距离30+第三排到第四排的距离30
																		// 第5次 buf1=第一排落蛋位置+第一排到第二排的距离30+第二排到第三排的距离30+第三排到第四排的距离30+第四排到第五排的距离30
																		// 第6次 buf1=第一排落蛋位置+第一排到第二排的距离30+第二排到第三排的距离30+第三排到第四排的距离30+第四排到第五排的距离30+第五排到第二个蛋托的第一排的距离
		}
    if(buf1>0)
    {
		   buf1 = PACKGE_SERVO_CYCLE_SETP - buf1;
		}
    else
    {
		   buf1 = PACKGE_SERVO_CYCLE_SETP + buf1;
			 buf1 = 0- buf1;
		}
    if(i == packge->row)   //第 0 步
		{
			if(buf>0)
			{
				buf1 = PACKGE_SERVO_CYCLE_SETP +	buf;	//和第一步合并
			}
			else 
			{
			  buf1 = 	buf - PACKGE_SERVO_CYCLE_SETP;	//和第一步合并
			}
		}
    else 
    {
		  buf1 = buf1 +	buf;	//和第一步合并
		}			
    jump_next_path_dat[0] = SERVO_RUN_PART_PAR0;
    jump_next_path_dat[1] = buf1;
		
		// JUMP_NEXT_STEP_PATH_START_ADDR 0x0650 路径20即PR20
		// 这个是设置单步运行的时候的路径参数吗？好像和上面重合了，如果是单独的就好理解
		
		set_servo_path_part( device->packge_servo.id,JUMP_NEXT_STEP_PATH_START_ADDR+(i*4),(uint16_t *)&jump_next_path_dat[0],4);
		read_servo_path_part(device->packge_servo.id,JUMP_NEXT_STEP_PATH_START_ADDR+(i*4),(uint16_t *)&read_jump_next_path_dat[0],4);
		if(rt_memcmp(jump_next_path_dat,read_jump_next_path_dat,8)!=0)
		{
			rt_free(packge);
			rt_free(wrige_reg);
			rt_free(read_reg);
			DEBUG_WARN("check path failed\n");
			return RT_ERROR ;
		}
		DEBUG_INFO("ID=%d setp=%d pluse=%d\n",device->device_id,i,buf1);
	}
	
	
	// 计算接收伺服的路径参数
	// packge->recive_claw_fall_dowm_position 99
	// RECIVE_SERVO_CYCLE_SETP 3200
	// RECIVE_CLAW_LEN      100
	// 接收伺服最大100mm，并且下降这100mm最大的脉冲数是3200个脉冲，那么下面就是求下降99mm需要走的脉冲数
	
	// RECIVE_SERVO_RUN_PART_PAR0 0x000BF002
	// 1011 1111 0000 0000 0010
	// 1011:目标速度 13-P5.073-2300ms
	// 1111:减速时间 15-P5.035-30ms
	// 0000:加速时间 0-P5.020-200ms
	// 0000:ABS绝对寻址 0-ABS绝对寻址
	// 0010:路径形式 2-SINGLE定位控制，完毕则停止
	// SINGLE定位控制:Mode 2 (SINGLE)：单次定位。PR路径执行完成后即停止，不会自动执行下一段路径。
	
	buf = (packge->recive_claw_fall_dowm_position * RECIVE_SERVO_CYCLE_SETP)/RECIVE_CLAW_LEN;
	wrige_reg[0] = RECIVE_SERVO_RUN_PART_PAR0;	// 0x000BF002 加减速信息
	
	
	if(device->device_id == CH1_DEVICE_ID)  //通道2 的这个值是负数
	{
		if(CH1_RECIVE_DIR !=SERVO_FORWORD)
		{
			buf = 0- buf;
		}
	}
    if(device->device_id == CH2_DEVICE_ID)  //通道2 的这个值是负数
	{
		if(CH2_RECIVE_DIR !=SERVO_FORWORD)
		{
			buf = 0- buf;
		}
	}
	wrige_reg[1] = buf;
	wrige_reg[2] = RECIVE_SERVO_RUN_PART_PAR1;
	wrige_reg[3] = 0;
	
	// ROUT_PATH_PATH_START_ADDR 0X604 即PR1的地址，注意这个是接收抓的伺服的参数地址
	set_servo_path_part(device->recive_servo.id,ROUT_PATH_PATH_START_ADDR,(uint16_t *)&wrige_reg[0],4);
	set_servo_path_part(device->recive_servo.id,ROUT_PATH_PATH_START_ADDR+4,(uint16_t *)&wrige_reg[2],4);	// 这个数据为0，设置了没有意义吧
	
	read_servo_path_part(device->recive_servo.id,ROUT_PATH_PATH_START_ADDR,(uint16_t *)&read_reg[0],4);
	read_servo_path_part(device->recive_servo.id,ROUT_PATH_PATH_START_ADDR+4,(uint16_t *)&read_reg[2],4);
	
	if(rt_memcmp(read_reg,wrige_reg,(2*8)!=0))
	{
		rt_free(packge);
		rt_free(wrige_reg);
	  rt_free(read_reg);
		DEBUG_WARN("check path failed\n");
		return RT_ERROR ;
	}
	
	// 如果复位检测蛋托标志为真并且伺服复位为真并且伺服是处在第一步并且伺服是空闲状态
	if((device->reset_check_tray_flag>0)&&(device->packge_servo.reset_flag==RT_TRUE)&&(device->packge_servo.current_step==0)&&(device->packge_servo.state ==SERVO_IDLE))
	{
    set_check_tray_part(device);	// 重新设置包装伺服蛋托检测位置的伺服参数
	}
	if((device->reset_cup_open_position_flag>0)&&(device->recive_servo.reset_flag==RT_TRUE)&&(device->recive_servo.current_step==0)&&(device->recive_servo.state ==SERVO_IDLE))
	{
		set_open_cup_part(device);		// 重新设置接收伺服打开的时候的位置伺服参数
	}
	// 比较设置的包装盒设备的包装是不是一样(中途这个包装信息可能会被改？)
	if(rt_memcmp(packge,&device->product_packge,sizeof(product_packge_info))==0)
	{
		device->change_packge_flag =0;
	}
	rt_free(packge);
  rt_free(wrige_reg);
	rt_free(read_reg);

	return RT_EOK ;
}


// 检查伺服是否处于打开状态
rt_err_t check_servo_on(servo_type *servo)
{
	 rt_err_t result;
	 uint16_t dat_buf[2]={0,0};
   
   result = servo_read_mutly_regs(servo->id,SERVO_ON_ADDR,dat_buf,1);
	 if(dat_buf[0] != 1)
	 {
	   set_servo_on(servo,1);
		 return 0;
	 }
	 else
	 {
		 return 1;
	 }
}

// 依次检查这个标志对应的是否处理了，处理理论上应该为0
// 在空闲状态，这些都处理了，设备才能进入到就绪状态
rt_err_t check_servo_inter_reg(ch_device_type *device)
{
	 uint8_t result;

   if(device->change_packge_flag>0)
   {
			return RT_ERROR;
	 }
   else if((device->change_packge_flag==0)&&(device->reset_check_tray_flag>0))
   {
		 // DEBUG_ERR("检测托盘位置错误\n");
		  set_alarm(device->device_id,transport_server_origin_err,1);
			return RT_ERROR;
	 }
	 else if((device->change_packge_flag==0)&&(device->reset_cup_open_position_flag>0))
   {
		  //DEBUG_ERR("蛋杯打开位置错误\n");
		  set_alarm(device->device_id,recive_server_place_err,1);
			return RT_ERROR;
	 }
	 else if((device->change_packge_flag==0)&&(device->reset_check_tray_flag==0)) 
	 {
	   DEBUG_INFO("package crc ok\n");
		 return RT_EOK;
	 }
	 return RT_ERROR;
}



#define  PILE_UP_TIME  5000
static rt_tick_t pile_up_time[2]={0,0};
// 斜坡堆积检测，设备不处于空闲且检测到高电平大于5s(即堆积超过5s)则报警
void egg_box_pile_up_check(ch_device_type *device)  //蛋托堆积检测
{
	 uint32_t time;
   if(device->device_id == CH1_DEVICE_ID)
	 {
		 if(get_ext_pin(CH1_PILE_UP_PIN)>0)
		 {
			 time= count_use_time(pile_up_time[0]);			// 检测到堆积计算堆积时间，大于5s才报警，因为正常走也会检测到高电平，但是时间都不长
			 if((time>PILE_UP_TIME)&&(device->status != SYS_IS_IDLE))
			 {
			    set_alarm(CH1_DEVICE_ID,slope_transport_ch_cumulate,1);
			 }
		 }
		 else 
		 {
		   pile_up_time[0]= rt_tick_get();						// 没有检测到堆积则更新时间
		 }
	 }
	 else 
	 {
	 
	   if(get_ext_pin(CH2_PILE_UP_PIN)>0)
		 {
			 time= count_use_time(pile_up_time[1]);
			 if((time>PILE_UP_TIME)&&(device->status != SYS_IS_IDLE))
			 {
			    set_alarm(CH2_DEVICE_ID,slope_transport_ch_cumulate,1);
			 }
		 }
		 else 
		 {
		   pile_up_time[1]= rt_tick_get();
		 }
	 }
}

// 伺服的状态控制
void servo_control(servo_type *servo)
{
	switch(servo->state)
	{
		case SERVO_IDLE:
			servo_idle(servo);					// 空闲的时候，检测伺服状态引脚是高电平则伺服停止
			break;
		case SERVO_HOPE_RUN_PATH:
			set_servo_run_path(servo);	// 将伺服运行数据写进去即选择要运行哪一个pr组的数据
			break;
		case SERVO_RUNING_PATH:
			check_servo_run_path_is_ok(servo);	// 这里检测伺服当前的pr组数据是否运行完成,完成后则又进入到空闲状态并且servo->hope_run_path = 0xff;这个是什么，这个是随便设置的一个，没有用？
			break;
	 case SERVO_HOPE_RESET_ORIGIN:					// 如果是伺服复位到原点
			set_servo_reset_origin(servo);			// 直接执行pr0即回到原点的指令，如果执行pr0即回到原点的指令没有成功进入到下面这个伺服状态 SERVO_RESET_ORIGIN
			break;
	 case SERVO_RESET_ORIGIN:											// 伺服复位到原点
			check_servo_reset_origin_is_ok(servo,1);
			break;
	 case SERVO_STOP:												// 伺服停止
		  servo_stop(servo);									// 写入1000表示伺服电机停止
			break;
	 default:
		 break;
	}
}


