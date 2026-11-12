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
#include <kick_egg.h>
#include "servo_control.h"
#include "servo_common.h"
#include "servo_debug.h"
#include "agile_modbus_user.h"
#include "alarm_control.h"
#include <mail_box.h>





ch_device_type ch1_device;
ch_device_type ch2_device;

 
// 通过命令写入产品包装信息
void write_product_packge(uint8_t ch,product_packge_info *packge)
{
	rt_base_t level;
  
	if(ch ==0)
	{
		// 如果主控发过来的包装信息和本地的包装信息不一样，则更新包装信息
		// 包装信息改变标志置为1
		// 复位检查蛋托标志置为1
		// 复位蛋杯打开标志置为1
		if(rt_memcmp(packge,&ch1_device.product_packge,sizeof(product_packge_info))!=0)
	  {
		  level = rt_hw_interrupt_disable();
      rt_memcpy(&ch1_device.product_packge,packge,sizeof(product_packge_info)); 
			ch1_device.change_packge_flag = RT_TRUE;
			ch1_device.reset_check_tray_flag = RT_TRUE;
			ch1_device.reset_cup_open_position_flag = RT_TRUE;
		  rt_hw_interrupt_enable(level);
		
			DEBUG_INFO("recive product ch=%d name=%s\n",ch,packge->name);
		}
		else
		{
			DEBUG_INFO("recive ch1 packge is the same\n");
		}
		
	  
	}
	else 
	{
		if(rt_memcmp(packge,&ch2_device.product_packge,sizeof(product_packge_info))!=0)
	  {
		  level = rt_hw_interrupt_disable();
      rt_memcpy(&ch2_device.product_packge,packge,sizeof(product_packge_info));
			ch2_device.change_packge_flag = RT_TRUE;
			ch2_device.reset_check_tray_flag = RT_TRUE;
			ch2_device.reset_cup_open_position_flag = RT_TRUE;
		  rt_hw_interrupt_enable(level);
			DEBUG_INFO("recive product ch=%d name=%s\n",ch,packge->name);
		}
		else
		{
			DEBUG_INFO("recive ch2 packge is the same\n");
		}
	}
}



static rt_uint8_t msg_modbus_recive[MESSAGE_DAT_LEN];


static void debug_packge_servo_box(uint8_t *dat)
{
	 uint8_t ch;
	 int16_t pluse;
	
	 if(dat[2]==2)  //执行步数
	 {
		ch =dat[1]+1;
		pluse = (dat[3]<<8);
		pluse += dat[4];
		
		 if((ch%2)==0)
		 {
		   pluse = 0-pluse;
		 }
		debug_packge_servo_run_step(ch,pluse);
	 }
}


// 系统在空闲状态
// 1 
void sys_is_idle(ch_device_type *device)
{
	// 接收调试命令中的包装伺服命令
	if (rt_mq_recv(&masseg_modbus_cmd, &msg_modbus_recive, sizeof(msg_modbus_recive), 10) == RT_EOK)
  {
		debug_packge_servo_box(msg_modbus_recive);	// 执行包装调试命令
    //rt_kprintf("thread1: recv msg from msg queue,=%d\n",msg_modbus_recive[0]);
  }
				
	if(device->device_id ==CH1_DEVICE_ID)
		ch1_servo_debug_mode();											// 检测复位按键，复位后执行相应动作
																								// 检测单步执行按键，按下后执行单步运行动作
																								// 检测调试伺服信息，又新的伺服调试数据，执行伺服调试指令
																								// 执行servo_control，让伺服根据状态机运行
																								// 每50次读取一下包装伺服运行的距离
																								// 每100次读取一下接收伺服运行的距离
	else 
		ch2_servo_debug_mode();
	
	if(device->change_packge_flag ==RT_TRUE)			// 如果包装信息变了，要重新计算包装运行路径等并写进到伺服路径参数里面
	{
		caculate_and_write_product_packge_run_path(device);
	}
	if(device->reset_cup_open_position_flag==RT_TRUE)		// 如果重新设置接收伺服打开位置标志为真则重新设置接收伺服相关参数
	{
		set_open_cup_part(device);
	}
}

// 检测系统环境，1是检测伺服是否处于on状态，2是检测设备是否处于原点状态，都满足则系统进入就绪状态
void sys_is_check_env(ch_device_type *device)
{
	 uint8_t result=0;
   if(check_servo_inter_reg(device)==RT_EOK)
	 {
		 // 检测伺服处于on状态
		 result += check_servo_on(&device->recive_servo);
		 result += check_servo_on(&device->packge_servo);
		 if(result==2)
		 {
			 // 如果设备处于原点位置
			 if(device->origin_flag ==RT_TRUE)
			 {
				 // 现将接收伺服编码器的脉冲计数清零
				 if(check_clear_servo_encode_pluse(&device->recive_servo)==1)
				 {
						device->status = SYSTEM_IS_READY;	// 系统处于准备好状态
					  rt_kprintf("包装通道已经准备好了 ch=%d\n",device->device_id);
				 }
				 else 
				 {
				   set_alarm(device->device_id,transport_server_origin_err,1);
				 }
			 }
			 // 不处于原点位置，则报伺服原点错误报警
			 else 
			 {
				 set_alarm(device->device_id,transport_server_origin_err,1);
			 }
		 }
	 }
	 rt_thread_mdelay(10);
}

// 检测系统是否进入空闲状态
void check_sys_enter_idle(ch_device_type *device)
{
	// 如果包装伺服，接收伺服处于空闲状态并且伺服的执行步骤为0，就说明系统需要进入空闲状态？
	if((device->packge_servo.state == SERVO_IDLE)&&(device->recive_servo.state == SERVO_IDLE)&&(device->recive_servo.current_step == 0))
	{
	  device->status = SYS_IS_IDLE;
		DEBUG_CARE("system is idle id=%d\n",device->device_id);
	}
	else
	{
		rt_kprintf("p state=%d r state=%d step=%d\n",device->packge_servo.state,device->recive_servo.state,device->recive_servo.current_step);
		rt_thread_mdelay(100);
	}
}

// 系统运行的时候
// 1检测斜坡是否堆积
// 2检测是否需要走下一个蛋托
// 3接收伺服运行按步骤运行
// 4步进电机运行
// 5包装伺服按照状态机运行
// 6接收伺服按照状态机运行
void sys_is_runing(ch_device_type *device)
{
	  egg_box_pile_up_check(device);  //蛋托堆积检测
	  jump_to_nex_tray(device);	// 获取蛋托
		recive_servo_run(device);   //这里面会改变接收伺服的状态，下面接收伺服就会依据状态来执行对应状态的指令
	  recive_cup_run(device);     // 接收爪 下降 蛋夹打开  上升 关闭
		recive_up_and_packge_run(device); //接收爪升起 同时 包装通道移动一格(必须接收伺服上升了)
		servo_control(&device->packge_servo);
		servo_control(&device->recive_servo);
}
// 系统挂起
void sys_is_suppend(ch_device_type *device)
{
  rt_thread_mdelay(10);
}
// 系统紧急停止
// 系统状态设置为不处于原点
// 包装改变并重新计数包装伺服的pr参数
// 蛋托复位
// 接收伺服重新设置pr参数
// 接收伺服停止
void sys_is_em_stop(ch_device_type *device)  //紧急停止
{
	 device->origin_flag = RT_FALSE;
	 device->change_packge_flag = RT_TRUE;
	 device->reset_check_tray_flag = RT_TRUE;
	 device->reset_cup_open_position_flag = RT_TRUE;
	
	 servo_write_reg(device->recive_servo.id,START_ROUT_PATH,1000);
	 servo_write_reg(device->recive_servo.id,START_ROUT_PATH,1000);
	 rt_thread_mdelay(50);
} 

// 检测伺服驱动器自身的报警信息
void check_servo_alarm(ch_device_type *device)
{
	
	uint16_t read_buf[2];
	
	// 读取伺服驱动器的报警信息
	// 先读取伺服就绪引脚是否就绪，如果没有异常，则就绪引脚输出高电平
	// 设备就绪了，就应该没有报警了呀，还去度报警干什么？
	// 应该是一直没有就绪，去读出现了什么报警？
  if(get_ext_pin(device->servo_alarm_pin) ==IS_OPEN)
	{
		read_buf[0]=0;
	  servo_read_mutly_regs(device->recive_servo.id,0X0002,read_buf,1);
		if(read_buf[0]>0)
		{
		  set_alarm(device->device_id,recive_server_alarm_err,1);	// 设置接收伺服报警
		}
		read_buf[0]=0;
	  servo_read_mutly_regs(device->packge_servo.id,0X0002,read_buf,1);
		if(read_buf[0]>0)
		{
		  set_alarm(device->device_id,transport_server_alarm_err,1); // 设置包装伺服报警
		}
	}
	else 																													// 没有报警
	{
	   set_alarm(device->device_id,recive_server_alarm_err,0);		// 清除接收伺服报警
	   set_alarm(device->device_id,transport_server_alarm_err,0); // 清除包装伺服报警
	}
}
// 系统按照系统状态机运行
void system_is_runing(ch_device_type *device)
{
	if(device->en_flag==0)
	{
		clear_control_alarm( device->device_id);
		return ;
	}
	switch(device->status)
	{
		case SYS_IS_IDLE:
			sys_is_idle(device);
			break;
		case SYS_IS_CHECK_ENV:
			sys_is_check_env(device);
			break;
		case SYS_IS_RUNING:
			sys_is_runing(device);
		  break;
		case SYSTEM_IS_STOP:
			sys_is_runing(device);
		  check_sys_enter_idle(device);
			break;
		case SYSTEM_IS_ENTER_IDLE:
			//sys_is_runing(device);
		  //check_sys_enter_idle(device);
			break;
		case SYSTEM_IS_EMERGENCY_STOP:  //紧急停止
			sys_is_em_stop(device);
			break;
		default:
			break;
	}
	
	check_servo_alarm(device);				// 检测伺服驱动器自身的报警信息
	if(device->status != SYS_IS_IDLE)	// 系统运行的时候，清除复位操作标志
	{
	  clear_reset_origin_flag(device->device_id);
	}
}
// 产品包装参数初始化，可以通过命令修改
void product_packge_init(ch_device_type *device)
{
	
  device->product_packge.row = 5;
	device->product_packge.line=6;
	
	// 这6个参数是相对于包装通道的原点的距离
	device->product_packge.check_egg_tray_position = 80;
	device->product_packge.packge_line1_postion = 200;
	device->product_packge.packge_line2_postion = 230;
	device->product_packge.packge_line3_postion = 260;
	device->product_packge.packge_line4_postion = 290;
	device->product_packge.packge_line5_postion = 310;
	
	// 这3个参数是相对于接收伺服的原点的距离
	device->product_packge.recive_claw_fall_dowm_position = 99;
	device->product_packge.recive_claw_close_position = 70;
	device->product_packge.recive_claw_open_position = 70;
	// 这个参数呢是什么意思？这个表示接收伺服在上升的过程中，上升到离原点位置小于这个值的时候则包装伺服可以移动包装了
	device->product_packge.transport_start_position = 80;
	
	
	//caculate_and_write_product_packge_run_path(device);
}

// 系统初始化，这个没有用
static void device_init1()
{
	  rt_memset(&ch1_device,0,sizeof(ch_device_type));
	  rt_memset(&ch2_device,0,sizeof(ch_device_type));
	
	  ch1_device.en_flag = 1;
	  ch1_device.device_id = CH1_DEVICE_ID;
	  ch1_device.cup_id =0;
	  ch1_device.tray_senser_pin = CH1_CHECK_TRAY_PIN;
	  ch1_device.cup_senser_pin = CH1_EGG_CUP_OPEN_SENSER_PIN;
		ch1_device.packge_servo.id =1;
	  ch1_device.packge_servo.state_pin = CH1_PACHGE_SERVO_STATE_PIN;
    ch1_device.packge_servo.origin_pin = CH1_PACKGE_RESET_ORIGIN_PIN;
	  ch1_device.recive_servo.id =3;
		ch1_device.recive_servo.state_pin =  CH1_RECIVE_SERVO_STATE_PIN;
	  ch1_device.recive_servo.origin_pin = CH1_RECIVE_RESET_ORIGIN_PIN;
    ch1_device.servo_alarm_pin =  CH1_SERVO_ALARM_PIN;
	  ch1_device.origin_flag = RT_FALSE;
	  ch1_device.tray_flag =0;
	  ch1_device.change_packge_flag = RT_TRUE;
		ch1_device.reset_check_tray_flag = RT_TRUE;
		ch1_device.reset_cup_open_position_flag = RT_TRUE;
	
	  ch2_device.en_flag = 1;
	  ch2_device.device_id = CH2_DEVICE_ID;
	  ch2_device.cup_id =1;
	  ch2_device.tray_senser_pin = CH2_CHECK_TRAY_PIN;
	  ch2_device.cup_senser_pin = CH2_EGG_CUP_OPEN_SENSER_PIN;
		ch2_device.packge_servo.id =2;
	  ch2_device.packge_servo.state_pin = CH2_PACHGE_SERVO_STATE_PIN;
    ch2_device.packge_servo.origin_pin = CH2_PACKGE_RESET_ORIGIN_PIN;
	  ch2_device.recive_servo.id =4;
		ch2_device.recive_servo.state_pin =  CH2_RECIVE_SERVO_STATE_PIN;
	  ch2_device.recive_servo.origin_pin = CH2_RECIVE_RESET_ORIGIN_PIN;
    ch2_device.servo_alarm_pin =  CH2_SERVO_ALARM_PIN;
		ch2_device.origin_flag = RT_FALSE;
		ch2_device.tray_flag =0;
		ch2_device.change_packge_flag = RT_TRUE;
		ch2_device.reset_check_tray_flag = RT_TRUE;
		ch2_device.reset_cup_open_position_flag = RT_TRUE;
		
}


// 系统初始化
static void device_init()
{
	  rt_memset(&ch1_device,0,sizeof(ch_device_type));
	  rt_memset(&ch2_device,0,sizeof(ch_device_type));
	
	  ch1_device.en_flag = 1;
	  ch1_device.device_id = CH1_DEVICE_ID;
	  ch1_device.cup_id =0;
	  ch1_device.tray_senser_pin = CH1_CHECK_TRAY_PIN;
	  ch1_device.cup_senser_pin = CH1_EGG_CUP_OPEN_SENSER_PIN;
		ch1_device.packge_servo.id =1;
	  ch1_device.packge_servo.state_pin = CH1_PACHGE_SERVO_STATE_PIN;
    ch1_device.packge_servo.origin_pin = CH1_PACKGE_RESET_ORIGIN_PIN;
	  ch1_device.recive_servo.id =3;
		ch1_device.recive_servo.state_pin =  CH1_RECIVE_SERVO_STATE_PIN;
	  ch1_device.recive_servo.origin_pin = CH1_RECIVE_RESET_ORIGIN_PIN;
    ch1_device.servo_alarm_pin =  CH1_SERVO_ALARM_PIN;
	  ch1_device.origin_flag = RT_FALSE;
	  ch1_device.tray_flag =0;
	  ch1_device.change_packge_flag = RT_TRUE;
		ch1_device.reset_check_tray_flag = RT_TRUE;
		ch1_device.reset_cup_open_position_flag = RT_TRUE;
	
	  ch2_device.en_flag = 1;
	  ch2_device.device_id = CH2_DEVICE_ID;
	  ch2_device.cup_id =1;
	  ch2_device.tray_senser_pin = CH2_CHECK_TRAY_PIN;
	  ch2_device.cup_senser_pin = CH2_EGG_CUP_OPEN_SENSER_PIN;
		ch2_device.packge_servo.id =2;
	  ch2_device.packge_servo.state_pin = CH2_PACHGE_SERVO_STATE_PIN;
    ch2_device.packge_servo.origin_pin = CH2_PACKGE_RESET_ORIGIN_PIN;
	  ch2_device.recive_servo.id =4;
		ch2_device.recive_servo.state_pin =  CH2_RECIVE_SERVO_STATE_PIN;
	  ch2_device.recive_servo.origin_pin = CH2_RECIVE_RESET_ORIGIN_PIN;
    ch2_device.servo_alarm_pin =  CH2_SERVO_ALARM_PIN;
		ch2_device.origin_flag = RT_FALSE;
		ch2_device.tray_flag =0;
		ch2_device.change_packge_flag = RT_TRUE;
		ch2_device.reset_check_tray_flag = RT_TRUE;
		ch2_device.reset_cup_open_position_flag = RT_TRUE;
}

#define FALL_DOWN_EGG_TIME  200

uint8_t ch1_recive_egg_flag=0;
uint8_t ch2_recive_egg_flag=0;

rt_tick_t ch1_tick;
rt_tick_t ch2_tick;

void set_ch1_recive_egg()
{
	if(ch1_device.en_flag>0)
	{
		ch1_recive_egg_flag=1;
		ch1_tick = rt_tick_get();
	}
}
void set_ch2_recive_egg()
{
	if(ch2_device.en_flag>0)
	{
		ch2_recive_egg_flag=1;
		ch2_tick = rt_tick_get();
	}
}
// 计算落蛋的时候的延迟
// 这里是不是应该在计算落蛋的时候，由于抛物线的原因，电磁阀踢开后到落到接收抓蛋杯的水平的运行时间
// 亦即是不是想通过这个水平的运行时间，计算开始落到落到蛋杯这段时间的水平距离，通过这个来计算提前多久开始踢蛋？
uint16_t clacute_fall_down_delay_time()
{
  uint16_t speed;
	float time;
	speed = get_spt_speed();		// 这个是通过命令调用decode_run_egg_cup_dat函数，里面有运行的速度这个参数
															// 这个速度是指什么，是指包装分离主线的主电机运行速度吗？
	speed = speed*10;						// 速度扩大10，这个有什么意义呢？经排查，在主控端发送的时候缩小了10
	time = 635.0/speed;  				//一秒10个蛋的距离  63.5mm *10;
															// 1个鸡蛋蛋杯的距离是63.5,10个鸡蛋的蛋杯距离
															// 用10个蛋杯的距离除以速度，算出这个10个蛋杯的距离要运行多久
															// 因为是10个鸡蛋的距离，那么算出来的时间则要缩小10倍
	time = time*0.1;						// 时间在缩小10倍
															// 不管缩小了多少倍，mm/(mm/s)得到这个时间单位是秒
	time = time*1000; //单位ms	// 乘以1000变为毫秒
	if(time>100)								// 如果大于100ms则减去一个100ms，为什么？
	{
	  time = time-100;
	}
	else 												// 如果小于100ms则置为零，为什么？
	{
	  time =0;
	}
	if(time>1000)								// 如果上面减了100都还大于1000，重新设置为1000ms
		time = 1000;
	return (uint16_t)(time);		// 最终返回这个落蛋的延迟时间
}
// 计算蛋杯接收蛋的延迟时间
static rt_tick_t up_time=0;
void recive_egg_delay_fun(uint8_t device_id)
{
	uint16_t delay_time;
	uint16_t alread_delay_time=0;
	
  delay_time = clacute_fall_down_delay_time();
	
	if(device_id == CH1_DEVICE_ID)
	{
		if(ch1_recive_egg_flag)													// 接收标志为真(分析了半天，结果这个标志是接收伺服能不能下降的标志，这样主控会把控接收爪有没有6个蛋，有就让接收伺服下降)
																										// 并且每次电磁铁踢蛋后，有个变化表示该通道有蛋，这个状态会返回给主机，主机根据这些信息判断，下一次放哪个通道，能不能下降了
		{
			alread_delay_time = count_use_time(ch1_tick);	// 则算踢蛋已经延迟的时间，接收到要接收蛋的标志后，记录该时刻，然后等待一段时间
			if(alread_delay_time>(FALL_DOWN_EGG_TIME+delay_time))	// 如果计算出已经延迟的时间大于计算的需要延迟的时间加上1个落蛋时间定值的总和
			{
				ch1_recive_egg_flag =0;
				ch1_device.recive_egg =RT_TRUE;				// 这个变为真，则会影响接收伺服recive_servo_run函数，线程里面层层调用，最后伺服会按照步骤接蛋放蛋回来
			}
		}
	}
	else 
	{
		if(ch2_recive_egg_flag)
		{
		
			alread_delay_time = count_use_time(ch2_tick);
		//rt_kprintf("atime=%d delay time=%d fadown time=%d\n",alread_delay_time,FALL_DOWN_EGG_TIME,delay_time);
			if(alread_delay_time>(FALL_DOWN_EGG_TIME+delay_time))
			{
			//DEBUG_WARN(" recive egg time=%d tick=%d\n",rt_tick_get()-up_time,rt_tick_get());
				up_time = rt_tick_get();
				ch2_recive_egg_flag =0;
				ch2_device.recive_egg =RT_TRUE;
			}
		}
	}
}

// 设备1的运行线程
void ch1_control_thread(void *arg)
{

  device_init();
	rt_thread_mdelay(1000);   //等待MODBUS 起来
	product_packge_init(&ch1_device);
	product_packge_init(&ch2_device);
	while(1)
	{
		recive_egg_delay_fun(ch1_device.device_id);	// 当有蛋放下的时候，会计算放的延迟时间，延迟时间到了，才置为伺服电机动作执行真正的放蛋动作？
		system_is_runing(&ch1_device);							// 系统运行是一个状态机
																								// 在运行状态，伺服电机会根据自身的状态机运行
		rt_thread_mdelay(5);
	}
}
// 设备2的运行线程
void ch2_control_thread(void *arg)
{
	rt_thread_mdelay(1000);   //等待MODBUS 起来
	product_packge_init(&ch2_device);
	uint32_t tick;
	tick = rt_tick_get();
	while(1)
	{
		recive_egg_delay_fun(ch2_device.device_id);
		system_is_runing(&ch2_device);  //上面和下面这里
		rt_thread_mdelay(5);
	}
}

uint16_t test_flag=0;
uint16_t run_speed=1000;
// 测试
void test_run()
{
   while(1)
	 {
		 if(test_flag)
		 {	 
				//set_ch1_recive_egg();
				//set_ch2_recive_egg();
			 ch1_device.recive_egg =RT_TRUE;
			 ch2_device.recive_egg =RT_TRUE;
			  
		 }
		 rt_thread_mdelay(run_speed);
	 }
}

// 创建系统运行线程
static int servo_control_init(void)
{
    rt_thread_t tid,tid1,tid2;
	
    device_init();
	  relay_init();
    tid = rt_thread_create("ch1_c",ch1_control_thread, RT_NULL,2048,15, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
		
		tid1 = rt_thread_create("ch2_c",ch2_control_thread, RT_NULL,2048,15, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

		tid2 = rt_thread_create("test",test_run, RT_NULL,512,30, 10);
    if (tid2 != RT_NULL)
        rt_thread_startup(tid2);

    return RT_EOK;
}
extern void sync_control_state(void);

// 通过命令的方式设置系统运行状态以及系统是否使能
void set_servo_run_state(uint8_t *buf)//(,,uint8_t ch2_en_flag,)
{
	uint8_t ch;
	uint8_t state;
	uint8_t ch1_en_flag;
	uint8_t ch2_en_flag;
	uint8_t check_tray;
	uint8_t servo_com_err_count;
	ch = buf[0];
	state = buf[1];
	ch1_en_flag = buf[2];
	ch2_en_flag = buf[3];
	check_tray = buf[4];
	servo_com_err_count = buf[5];
	set_servo_run_count(servo_com_err_count);
		if(ch == 0)
		{
			// 如果系统是空闲状态，此时设置为系统停止则直接退出
			if((state ==SYSTEM_IS_STOP)&&(ch1_device.status == SYS_IS_IDLE))
			{
				return ;
			}
			// 否则设置系统状态为要设置的状态，并且系统使能设置为要使能的状态	 
			ch1_device.status = state;
			ch1_device.en_flag = ch1_en_flag;
			ch2_device.en_flag = ch2_en_flag;
		}
		else 
		{
			if((state ==SYSTEM_IS_STOP)&&(ch2_device.status == SYS_IS_IDLE))
			{
				return ;
			}
			ch2_device.status = state;
			ch1_device.en_flag = ch1_en_flag;
			ch2_device.en_flag = ch2_en_flag;
		}
		set_check_tray_flag(check_tray);	// 检测是否有空托
		sync_control_state();
    rt_kprintf("m=%d c=%d \n",state,ch);
}

// 通过命令的方式获取包装信息
void get_servo_packge_info(servo_packge_type *control)
{

	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	rt_memcpy(&control->ch1_packge_info,&ch1_device.product_packge,sizeof(product_packge_info));
  rt_memcpy(&control->ch2_packge_info,&ch2_device.product_packge,sizeof(product_packge_info));
	rt_hw_interrupt_enable(level);
}

// 通过命令的方式或者udp定时发送系统的运行状态数据
void get_servo_run_state(packge_control_type *control)
{
	uint8_t i;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();

	rt_memset(control,0,sizeof(packge_control_type));
	
	control->ch1.empty_id = ch1_device.recive_cup_empty_id;	// recive_cup_empty_id 相当于包序号的意思？
	control->ch1.state =  ch1_device.status;								// status 某通道的设备的状态
	control->ch1.en_flag = ch1_device.en_flag;							// 获取通道的使能状态(设备使能后才能运动跑起来)			
	control->ch1.alarm   = get_control_alarm(CH1_DEVICE_ID);	// 获取通道的报警信息
	control->ch1.packge_servo_offest_distem = ch1_device.packge_servo.move_distem;	// 包装通道电机运行的距离
	control->ch1.recive_servo_offest_distem = ch1_device.recive_servo.move_distem;	// 接收伺服电机运行的距离
  control->ch1.cup_eggs=0;
	for(i=6;i>0;i--)
	{
		control->ch1.cup_eggs = control->ch1.cup_eggs<<1;		// 将要获取该位的蛋空间先腾出来
		if(get_recive_cup_egg_buf(i-1))
		{
		  control->ch1.cup_eggs  +=1;  											// 获取到该位有蛋的话，将该位加1即将该位置1(因为上面已经将该位腾出来了并置0了)			
		}
	}
	
	// 通道2同理通道1
	control->ch2.empty_id = ch2_device.recive_cup_empty_id;
  control->ch2.state =  ch2_device.status;
	control->ch2.en_flag = ch2_device.en_flag;
	control->ch2.alarm   = get_control_alarm(CH2_DEVICE_ID);
	control->ch2.packge_servo_offest_distem = ch2_device.packge_servo.move_distem;
	control->ch2.recive_servo_offest_distem = ch2_device.recive_servo.move_distem;
	
	control->ch2.cup_eggs=0;
	for(i=12;i>6;i--)
	{
		control->ch2.cup_eggs = control->ch2.cup_eggs<<1;
		if(get_recive_cup_egg_buf(i-1))
		{
		  control->ch2.cup_eggs  +=1;  
		}
	}
	
	control ->input_gpio = get_control_input_io_dat();	// 获取612板子输入端子的数据
	rt_hw_interrupt_enable(level);
}



static int servo(int argc, char **argv)
{

	run_speed = atoi(argv[2]);
	test_flag = atoi(argv[1]);

	
	ch1_device.status = atoi(argv[1]);
	ch2_device.status = atoi(argv[1]);
	
	ch1_device.en_flag =ch1_device.status &&1;
	ch2_device.en_flag =ch2_device.status &&1;
	
  rt_kprintf("run_speed=%d\n",run_speed);
}
MSH_CMD_EXPORT(servo, list device in system);



INIT_APP_EXPORT(servo_control_init);




