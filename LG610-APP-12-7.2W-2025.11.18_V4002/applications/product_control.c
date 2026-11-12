
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
#include "app_key.h"
#include "math.h"
#include "app_io_control.h"
#include "modbus_packge.h"
#include "err_check.h"
ALIGN(RT_ALIGN_SIZE)



static uint8_t power_up_reset_flag=0;
static uint8_t debug_mode_index=0;
static uint16_t jump_cup_number=1000;
static rt_tick_t stop_delay_time;


void set_cup_move_to_position(uint16_t number)
{
	jump_cup_number = number;
  debug_mode_index = 1;
}
void set_debug_mode_index(uint8_t index)
{
	set_main_moto_speed(0);
  debug_mode_index = index;
}
uint8_t get_debug_mode_index()
{
  return debug_mode_index;
}
#define LOCK_POSITION_PLUSE  5000
void jump_to_find_cup(uint16_t index)
{
	 uint32_t spt_pluse;
	 uint32_t offest_pluse;
	 int32_t buf;
	 uint32_t cup_pluse;
	
		// ((SEPARATE_CHA_CUP_NUMBER/6)*1000)
		// 举例 270/6*1000=45000，这个数字号熟悉呀，我实际数的时候，分级线转一圈大概26秒，脉冲值好像就是在45000左右
		// 编码器转一圈的脉冲数是1000个脉冲就是编码器转一圈会发送1000个脉冲，因为是增量型的是一直发
		// 那分级线转一圈，编码器转了多少个脉冲呢？
		// 这个除以6是什么意思，我看接蛋的地方一次接6个，下一次接也是接6个，所以分级线接一次也要走6个蛋杯，并且注意到接的地方，接一次转一次，所以推理出转一圈走6个蛋杯？
		// 然后总蛋杯数除以6得到要转的圈数级编码器要转的圈数？
   cup_pluse =((SEPARATE_CHA_CUP_NUMBER/6)*1000);
   spt_pluse = get_spt_encode_pluse();
	 spt_pluse = spt_pluse%cup_pluse;   //获取当前的实时脉冲值,spt_pluse%cup_pluse是避免大于总脉冲值，这个得到的范围永远在0-cup_pluse之间
	
	 offest_pluse = index*16666;			// 一个蛋杯是166.66，要调到第几号蛋杯(以第一个蛋杯为基准)需要走的脉冲为index*166.66(这里扩大了100)
	 offest_pluse = offest_pluse/100;	// 上面扩大了100，这里就缩小100？不管怎么样，这个是要跳到该蛋杯走的脉冲？
	
	// 这个是什么意思
	// 一个蛋杯的位置在一个圈的相对位置
	// 以某个0-cup_pluse个脉冲之间距离，因为是不断往复的
	// 以某个点为锚点
	// 可以认为要找的蛋杯的位置位于该锚点的前面也可以认为要找的蛋杯的位置位于该锚点的后面
	// 比如3000，位于锚点5000的前面，这种情况再跑2000个脉冲就到锚点了
	// 比如16000，位于锚点的后面，还要转(cup_pluse-offest_pluse)+LOCK_POSITION_PLUSE这么多个脉冲才到锚点
	// 以上解释对不对？？？
	// 反正不管怎么样，下面计算的是要找的蛋杯的脉冲值，还要走多久到锚点
	// 但是这样减是得到的一个相对距离
	// 又和后面的spt_pluse绝对脉冲作差
	
	// 这种方法可以确保电机总是沿最短路径运动到目标位置，是旋转控制中更稳健的做法
	
	 if(offest_pluse<LOCK_POSITION_PLUSE)
	 {
	   offest_pluse = LOCK_POSITION_PLUSE - offest_pluse;
	 }
	 else 
	 {
		
	   offest_pluse = (cup_pluse-offest_pluse)+LOCK_POSITION_PLUSE;
	 }
	  
		// 计算出要跑到的蛋杯id的脉冲值和当前蛋杯(相对于第一号蛋杯)脉冲值之差
		buf = (offest_pluse - spt_pluse);

		// 如果要跳到的蛋杯的脉冲值和当前蛋杯的脉冲值差别不到，也就是要跳到的蛋杯马上就要来了或者差别不大认为已经到了
    if((buf<100)&&(buf>0))
	 {
		 // 到了则关闭主电机
		  set_debug_mode_index(0);
		  set_main_moto_speed(0);
		  DEBUG_INFO("JUMP ID=%d  offest pluse=%d cup offest pluse=%d\n",index,spt_pluse,offest_pluse);
	    DEBUG_CARE("jump to expect position cpluse=%d\n",get_spt_encode_pluse());
	 }
	 // 5000/166.66=30大概差的蛋杯个数在30个范围内
	 else if((buf<5000)&&(buf>0))
	 {
		 // 主电机速度设置为50
	   set_main_moto_speed(50);
	 }
	 // 大概差的蛋杯个数超过30个
	 else 
	 {
		 // 主电机速度设置为360
	   set_main_moto_speed(360);
	 }
}

//static uint8_t sys_status;
// 灯柱状态显示
// 继电器2是黄色(报警)
// 继电器3是绿色
// 继电器4是红色
static rt_tick_t lamp_time=0;
void lamp_pole_disp(uint8_t state)
{
  uint32_t time; 
	if(state == SYS_IS_CHECK_ENV)
	{
		// 黄色闪送
		time = count_use_time(lamp_time);
	  if((time/500)%2==0)
		{
		  rt_pin_write(RELAY2,1);
			rt_pin_write(RELAY3,0);
			rt_pin_write(RELAY4,0);
		}
		else 
		{
			rt_pin_write(RELAY2,0);
			rt_pin_write(RELAY3,0);
			rt_pin_write(RELAY4,0);
		}
	}
	else 
	{
	  lamp_time =rt_tick_get();
		if(state == SYSTEM_IS_RUING)
		{
			// 亮绿色
		  //rt_pin_write(RELAY2,0);
			rt_pin_write(RELAY3,1);
			rt_pin_write(RELAY4,0);
		}
		else if(state == SYSTEM_IS_STOP)
		{
			// 亮绿色
		  rt_pin_write(RELAY2,0);
			rt_pin_write(RELAY3,1);
			rt_pin_write(RELAY4,0);
		}
		else 
		{
			// 亮红色
			rt_pin_write(RELAY2,0);
			rt_pin_write(RELAY3,0);
			rt_pin_write(RELAY4,1);
		}
		
	}
 

}
// 执行调试模式
void excues_debug_mode()
{
	// 如果设置的跳转到指定蛋杯大于分级线的总蛋杯数量
	if(jump_cup_number>SEPARATE_CHA_CUP_NUMBER)
	{
		set_debug_mode_index(0);
	}
	// 如果模式为调试跳转到指定蛋杯
	if(debug_mode_index ==DEBUG_CUP_JUMP_TO_LOCK_POSITION)
	{
		// 跳转到指定蛋杯
	  jump_to_find_cup(jump_cup_number);
	}
	else
	{
	  debug_mode_index = DEBUG_IDLE;
	
	}
}
static uint8_t idle_time_count=0;
static void system_idle()
{
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	uint8_t config;
	uint8_t i;
	idle_time_count++;
	
	if(idle_time_count>20)
	{
		// 空闲的时候，每隔1.2s就去查询一下使能的包装通道的伺服运行路径
		idle_time_count=0;
		for(i=0;i<12;i++)
		{
			config = get_packge_ch_config(i);
			if(config>0)
			{
				message_buf[0]=GET_SERVO_RUN_PART;      //空闲的时候获取使用了的通道的伺服路径信息
				send_message_to_packge_device(packge_map_ch_id[i],message_buf);  
			}			
		}
	}
	// 跳转到指定蛋杯
	if(debug_mode_index==DEBUG_CUP_JUMP_TO_LOCK_POSITION)
	{
	  excues_debug_mode();
	}
	else
	{
		
		sync_packge_control_state_idle();	// 同步主机的空闲状态到包装设备上
																			// 如果该通道的包装控制器没有使能或者不是空闲状态则让该包装通道处于空闲状态
		rt_thread_mdelay(20);
		sync_product_packge();						// 同步主机的包装信息到包装设备上
																			// 如果主机这边的包装信息和包装通道的包装信息不一样则将主控这边的包装信息发给包装通道
																			// 并读取设置后的包装信息到主机上
		rt_thread_mdelay(50);
	}
	rt_thread_mdelay(60);
}
// 系统复位状态执行的函数
static void system_reset()
{
	set_debug_mode_index(0);
	
	if(get_reset_origin_state()==0)	// 没有检测到复位标志为高电平，让电机停下，并进入到空闲状态
	{																// 空闲的时候按下复位，则会将该标志置为1
																	// 当脉冲校准的时候，检测到该标志为高电平会设置为低电平
																	// 设置低电平的前提是检测到编码器z相，即说明系统复位是复位到z相的地方
		// // 系统没有复位状态，则设置主电机的速度为0
		set_main_moto_speed(0);
		power_up_reset_flag = RT_TRUE;		// 脉冲调整复位标志置为1
	  //sys_status = SYSTEM_IS_IDLE;
		set_system_state(SYSTEM_IS_IDLE);
	}
	else  // 检测到复位标志为高电平，让电机一直跑
	{
	  set_main_moto_speed(70);
	}
	//rt_thread_mdelay(5);
}

static uint8_t check_packge_time_count=0;
// 系统检查
static void system_scheck(rt_tick_t time)
{ 
	uint8_t state;
	uint8_t system_status;
	uint32_t use_time;
	
	check_packge_time_count++;
	set_debug_mode_index(0);
	if(check_packge_time_count>20)
	{
	  check_packge_time_count =0;
		state = check_system_per_start_part();
		if(state==0xA5)
		{
			// 伸缩杆到原点
			if(get_expansion_link_to_orgin_state()==1) //一直检测等待伸缩杆到原点，才会执行以下动作，并且让设备进入运行状态
			{
				use_time = count_use_time(time);
				if(time>6000)
				{
					sync_packge_recive_cup_eggs();		// 同步各个通道的踢蛋位置
					show_tick_egg_ch();								// 打印各个通道的踢蛋位置
					start_tick_eggs();								// 启动的时候，要把分级线已有的也即到了踢蛋的位置的蛋踢了？
																						// 正常运行的时候，分级线程一直在计算并发送踢蛋信号？
					check_recive_servo_egg_is_down();	// 检查接收伺服是否放蛋完成
																						// 原理是kick_ch_pos == max_ch，表示接收伺服应该是在处理放蛋过程
																						// 并且放蛋完成后empty_id会变，和本地的就不一样了
																						// 如果kick_ch_pos == max_ch并且empty_id一直没有变，说明有问题
																						// 最多等分级线跑300个脉冲这么久的时间，还是有问题
																						// 则发一次关闭电磁铁，并让接收伺服在放一次
																						// 没有放下的情况可能是按了暂停键或是怎么还没有完成就停下了
					rt_pin_write(RELAY2,0);						// 灯柱的黄灯关闭
					set_system_state(SYSTEM_IS_RUING);
					set_ext_device_start();						// 所有通道的停表志清零即表示所有通道开始
					sys_start_clalure_expansion_link_offest();	// 然后根据屏幕设置的速度计算伸缩杆应该的位置
																											// 在设备运行的状态，伸缩杆会运行到上面设置的位置
					DEBUG_CARE("sys is check ok\n");
				}
			}
			else 
			{
					rt_kprintf("伸缩杆复位\n");
			}
		}
		else if(state==0)
		{
			set_system_state(SYSTEM_IS_IDLE);
		}
	}
	// 复位后。让系统进入到空闲状态，并报主机位置错误
  if(power_up_reset_flag==RT_FALSE)
	{
		set_system_state(SYSTEM_IS_IDLE);
		set_device_alarm(MAST_MOTO_POSITION_ERR);
		return;
	}
	else 
	{
	  clear_device_alarm(MAST_MOTO_POSITION_ERR);
	}
	system_status =get_system_state();
	if(system_status == SYSTEM_IS_IDLE)
	{
	  expansion_link_stop();
		set_main_moto_speed(0);
	}
	 rt_thread_mdelay(20);
}


static void system_runing()
{ 
     stop_delay_time = rt_tick_get();
	 rt_thread_mdelay(2);
}



void check_control_enter_idle()
{
	 uint8_t j;
	 uint8_t ready,t_line;
   uint8_t packge_line;
	 uint8_t control_state;
   packge_line = get_use_packge_line();
	
   ready = 0;
   t_line =0;
   for(j=0;j<12;j++) //12条包装线
   {
		 if(packge_line&(1<<j))
		 {
				t_line++;
				control_state = get_packge_control_run_state(j);
				if(control_state == SYSTEM_IS_IDLE )
				{
					ready++;		
				}
		}
	}
   
	 if(t_line == ready)
	 {
		 set_ext_device_stop();		// 设置输送通道的各种电机为停止状态，并记录停止的时候的延迟时间
	   set_system_state(SYSTEM_IS_IDLE);
	 }
	 else 
	 {
		// set_system_state(SYSTEM_IS_IDLE);
	   DEBUG_INFO("waite packge control is idle t_line=%d read=%d\n",t_line,ready);
	 }
	 rt_thread_mdelay(100);
}

void check_packge_control_enter_stop()
{
	uint8_t j;
	uint8_t control_state;
	uint8_t packge_line;
	uint8_t ready,t_line;
}

static void system_stop()
{
	uint16_t time;
	if(get_spt_speed()==0)	// 系统停止的时候，会去获取主电机的速度是不是为0了
	{												// 如果是为0了
		time = count_use_time(stop_delay_time);
		//if(time<1000)
		//{
		//  sync_packge_control_state(SYSTEM_IS_STOP);
		//}
		//else if(time<5000)
		//{
		//  check_control_enter_idle();
		//}
		if(time>3000)				// 等到了0的时候，再过3s，让设备进入空闲状态
		{
			set_ext_device_stop();
			set_system_state(SYSTEM_IS_IDLE);
			set_main_moto_speed(0);
			//DEBUG_ERR("系统停机已超时\n");
		}
	}
	else 
	{
	  stop_delay_time = rt_tick_get();
	}
	rt_thread_mdelay(2);
}
static void system_emergency_stop()
{ 
	uint8_t i;
	uint8_t speed;
	uint16_t *addr;
	user_set_part_type *user_set;
	set_debug_mode_index(0);
	speed = get_moto_speed();
	if(speed !=0)
	{
		set_main_moto_speed(0);
	}
	
	get_user_set_point(&user_set); 
//	addr = &user_set->use_mode.egg_packge_ch6;
	for(i=0;i<12;i++)
	{
		//if(addr[5-i]>0)
		//{
		set_control_run_state(i,SYSTEM_IS_EMERGENCY_STOP); 		// 给包装通道发送紧急停机的指令
	//	}
	}
	rt_thread_mdelay(100);  //外部有强制断电
}

// 遇到报警，在运行的时候要停下来，在检查环境的时候要停止并进入空闲状态
void check_alarm()
{
	uint16_t alarm_count;
  uint8_t state;
	alarm_count = get_alarm_count();
  if(alarm_count>0)
	{
		 state = get_system_state();
		 if(state == SYSTEM_IS_RUING)  //运行的时候遇到报警
		 {
			 set_system_state(SYSTEM_IS_STOP);
			 set_expansion_link_move(2,0);
		 }
		 else if(state == SYS_IS_CHECK_ENV)  //运行的时候遇到报警
		 {
			 set_system_state(SYSTEM_IS_IDLE);
			 expansion_link_stop();
			 set_main_moto_speed(0);
			 set_ext_device_stop();
		 }
	}
}

static uint32_t system_check_start_time=0;
// 系统运行控制
void system_run_main(void *arg)
{
	uint8_t state,nstate;

	rt_thread_mdelay(1000*5);
	while(1)
  {
		check_alarm();
		user_key_scan();
		state = get_system_state();
		switch(state)
		{
			case SYSTEM_IS_IDLE:
				system_idle();
				break;
			case SYS_IS_CHECK_ENV:
				system_scheck(system_check_start_time);
				break;
			case SYSTEM_IS_RUING:
				system_runing();		// 系统运行的时候，本线程休眠，分离蛋的线程则在运行
				break;
			case SYSTEM_IS_STOP:
				system_stop();
				break;
			case SYSTEM_IS_RESET:
				system_reset();
				break;
			case SYSTEM_IS_EMERGENCY_STOP:
				system_emergency_stop();
				break;
			case SYSTEM_IS_ENTER_IDLE:
				check_control_enter_idle();
				break;
		  default:
				break;
		}
		// 重新获取当前设备的状态
		nstate = get_system_state();
		// 如果状态变化了，且变为空闲状态的话，伸缩杆停止
		if((nstate !=state)&&(nstate==SYSTEM_IS_IDLE))
		{
		   expansion_link_stop();
		}
		expansion_link_control(nstate);	// 动态控制伸缩杆的位置
		
		if(nstate !=SYS_IS_CHECK_ENV)
		{
		   system_check_start_time = rt_tick_get();
		}
		
		rt_thread_mdelay(10);
  }
}

static int product_contorl_init(void)
{
    rt_thread_t  tid1;
		tid1 = rt_thread_create("prot",system_run_main, RT_NULL,4096,10, 10);
    if (tid1 == RT_NULL) return RT_ERROR;
		rt_thread_startup(tid1);
    return RT_EOK;
}

INIT_APP_EXPORT(product_contorl_init);


static int strcmp1(int argc, char **argv)
{
	int result;

	result =strcmp(argv[1],argv[2]);
	rt_kprintf("result=%d str1=%s  str2=%s\n",result,argv[1],argv[2]);
}
MSH_CMD_EXPORT(strcmp1, list device in system);