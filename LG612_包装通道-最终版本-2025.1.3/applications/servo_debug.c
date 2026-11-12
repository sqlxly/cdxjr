
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
#include "servo_common.h"
#include "servo_debug.h"
#include "agile_modbus_user.h"
#include "alarm_control.h"


extern ch_device_type ch1_device;
extern ch_device_type ch2_device;
static debug_servo_type ch1_debug_recive_servo={0,0,0};
static debug_servo_type ch1_debug_packge_servo={0,0,0};

static debug_servo_type ch2_debug_recive_servo={0,0,0};
static debug_servo_type ch2_debug_packge_servo={0,0,0};
static uint8_t next_key_press_flag[2]={0,0};

// 调试模式  
//servo_device_id 设备ID
// dir 方向   
// flag  启动还是停止
// 这个函数没有使用
static rt_err_t debug_servo(servo_device_id id,uint8_t dir,uint8_t flag)
{
	rt_err_t result=0;
	
	if(flag==MOTO_START)
	{
		result += servo_write_reg(id,SERVO_ON_ADDR,1);					// 往0x23c地址写入1，先让伺服电机处于开启状态
		if(dir==0)  //正转
		{
			result += servo_write_reg(id,START_ROUT_PATH,62);			// 往0x50e地址写入62，第62组里面存的是正转的数据
		}
		else
		{
			result += servo_write_reg(id,START_ROUT_PATH,63);			// 往0x50e地址写入63，第63组里面存的是反转的数据
		}
	}
	else 
	{
		 result += servo_write_reg(id,START_ROUT_PATH,1000);		// 往0x50e地址写入1000，写1000表示停止命令
	}
	return result;
}

//state 0 停止  1启动
//dir 1上升 2 下降
// 调试接收伺服
void user_debug_recive_claw_servo(uint8_t id, uint8_t state,uint8_t dir)
{
	if(id==0)  //左通道
	{
		ch1_debug_recive_servo.excute_flag =1;
		ch1_debug_recive_servo.state = state;
		ch1_debug_recive_servo.dir = dir;
	}
	else //右通道
	{
	  ch2_debug_recive_servo.excute_flag =1;
		ch2_debug_recive_servo.state = state;
		ch2_debug_recive_servo.dir = dir;
	}
}

//state 0 停止  1启动
//dir 1上升 2 下降
// 调试包装伺服
void user_debug_packge_servo(uint8_t id, uint8_t state,uint8_t dir)
{
	if(id==0)  //左通道
	{
		ch1_debug_packge_servo.excute_flag =1;
		ch1_debug_packge_servo.state = state;
		ch1_debug_packge_servo.dir = dir;
	}
	else
	{ 
		ch2_debug_packge_servo.excute_flag =1;
		ch2_debug_packge_servo.state = state;
		ch2_debug_packge_servo.dir = dir;
	}
}
// 将调试信息给到已经定义了的伺服设备的参数中
void check_user_debug_servo(servo_type *servo,debug_servo_type *debug)
{
  if(debug->excute_flag)
	{ 
		if((servo->id ==1)||(servo->id ==3))
		{
		  ch1_device.origin_flag = RT_FALSE;
		}
		else 
		{
		  ch2_device.origin_flag = RT_FALSE;
		}
		debug->excute_flag =0;
	  if(debug->state ==MOTO_STOP)
		{
			servo->state = SERVO_STOP;
			rt_kprintf("servo is stop\n");
		}
		else 
		{
		  if( servo->state == SERVO_IDLE)
		  {
			servo->start_time = rt_tick_get();
			servo->state = SERVO_HOPE_RUN_PATH;
			if(debug->dir == TURN_UP)
			{
			  servo->hope_run_path=62;
			}
			else 
			{
			  servo->hope_run_path=63;
			}
			servo->current_step = 0;
		  }
		}
	}
}

// 检测接收伺服的极限位置
void check_recive_servo_in_limit_position(servo_type *servo)
{
	// 检测是否上升到位了，通过传感器来判断是否上升到位
	if(get_ext_pin(servo->origin_pin)==IS_OPEN)   //正转上升到位
	{
		 if((servo->hope_run_path ==62)||(servo->hope_run_path ==RESET_ORIGIN_PATH))   //两条路径  回归原点 59  上升 62
		 {
			 servo->current_step =0;
			servo->state = SERVO_IDLE;
		  servo->hope_run_path =0; 
			servo_write_reg(servo->id,START_ROUT_PATH,1000);			// 停止
			servo_write_reg(servo->id,START_ROUT_PATH,1000);			// 停止
			rt_thread_mdelay(10);
			servo_write_reg(servo->id,START_ROUT_PATH,0);					// 回到原点
			servo_write_reg(servo->id,START_ROUT_PATH,0);					// 回到原点
		  clear_encode_pluse(servo);
			servo->state = SERVO_IDLE;
			//DEBUG_CARE("接收爪上升限位ID=%d pin=%d key val=%d\n",servo->id,servo->origin_pin,get_ext_pin(servo->origin_pin));
		 }
	}
	// 检测是否下降到位了
	// 通过下降距离是否大于等于规定最大下降距离
  if(((abs(servo->move_distem))>=RECIVE_CLAW_LEN) &&(servo->hope_run_path ==63))    //下降到位
	{
		servo_write_reg(servo->id,START_ROUT_PATH,1000);
		servo->state = SERVO_IDLE;
		//DEBUG_CARE("接收爪下降限位ID=%d\n",servo->id);
	}
}


//cup_id 0 左通道  1 右通道
// 接收伺服和包装伺服都复位到原点并且让步进电机关闭状态
void set_servo_reset_to_origin(servo_type *packge_servo,servo_type *recive_servo,uint8_t cup_id)
{
	uint8_t i;
  uint8_t sersen_pin;
	uint8_t senser_state;
	uint16_t read_buf[3];
	int pluse;
	
	recive_servo->start_time = rt_tick_get();
	if(get_ext_pin(recive_servo->origin_pin) !=IS_OPEN)							// 伺服电机此时不处于原点位置，则回到原点位置
	{
		recive_servo->state = SERVO_HOPE_RESET_ORIGIN;
		recive_servo->hope_run_path = RESET_ORIGIN_PATH;
		recive_servo->current_step = 0;
	}
	else 																														// 伺服电机此时正处于原点位置
	{
		for(i=0;i<10;i++)
		{
			rt_thread_delay(50);
			clear_encode_pluse(recive_servo);														// 将主编码器的位置(编码值)清零
      servo_read_mutly_regs(recive_servo->id,0X0520,read_buf,2);	// 读取主编码器的位置(编码值)
			pluse = read_buf[0];
	    pluse+= read_buf[1]<<16;
		
			if(pluse ==0)																								// 读到已经清零了
			{
				DEBUG_INFO("接受抓编码器清零OK ID=%d\n",recive_servo->id);
				break;
			}
		}
	  recive_servo->current_step = 0;
	}
	
	packge_servo->start_time = rt_tick_get();
	packge_servo->state = SERVO_HOPE_RESET_ORIGIN;
	packge_servo->hope_run_path = RESET_ORIGIN_PATH;
	packge_servo->current_step = 0;
	
	if(cup_id ==0)
	{
	  sersen_pin =  CH1_EGG_CUP_OPEN_SENSER_PIN;
	}
	else 
	{
	  sersen_pin = CH2_EGG_CUP_OPEN_SENSER_PIN;
	}
	clear_moto_step(cup_id);																				// 步进电机的pwm脉冲计数清零
}

// 获取伺服当前的移动的距离
int32_t get_current_distem(uint8_t id,uint32_t t_len,uint32_t cycle_step)
{
	float buf;
	int32_t pluse=0;
	uint16_t read_buf[6];
	
  servo_read_mutly_regs(id,0X0520,read_buf,2);										// 读取主编码器的值
	pluse = read_buf[0];
	pluse+= read_buf[1]<<16;																				// 算出编码器走了多少个脉冲

	if(pluse ==0)																										// 在原点
	{
		return 0;
	}
	else 
	{
		if((id==3)||(id==4))
		{
		  if(pluse<0)
		  {
		    pluse = 0-pluse;
		  }	
		  buf = (float)pluse/cycle_step;						// 转了多少个脉冲pluse，一圈cycle_step个脉冲，计算得到走了多少圈buf
																								// 这个得到转了多少圈，这个是接收伺服，圈数不能大于1
																								// 注意这个buf，不是0，这是个浮点数，可能是零点几
		}
		else 
		{
		  if(pluse<0)
		  {
		    pluse = 0-pluse;
		  }	
		  pluse = pluse%cycle_step;									// 包装伺服是往复运动的，所以理论读取到的pluse可以大于1圈cycle_step，所有要计算电机当前位置在一圈的什么位置
		  buf = (float)pluse/cycle_step;						// 计算得到圈数buf是指这个buf在一圈的什么位置
																								// 注意这个buf，不是0，这是个浮点数，可能是零点几
		}
		

	  return (int16_t)(buf*t_len);								// 距离就是圈数buf乘以一圈的距离t_len
	}
}

// 通道1的包装伺服复位到原点
void debug_ch1_packge_servo_reset_to_origin()
{

	clear_alarm(&ch1_device.packge_servo);
	set_servo_on(&ch1_device.packge_servo,1);
	
	ch1_device.packge_servo.start_time = rt_tick_get();
	ch1_device.packge_servo.state = SERVO_HOPE_RESET_ORIGIN;
	ch1_device.packge_servo.hope_run_path = RESET_ORIGIN_PATH;
	ch1_device.packge_servo.current_step = 0;

}
// 通道2的包装伺服复位到原点
void debug_ch2_packge_servo_reset_to_origin()
{

	clear_alarm(&ch2_device.packge_servo);
	set_servo_on(&ch2_device.packge_servo,1);
	ch2_device.packge_servo.start_time = rt_tick_get();
	ch2_device.packge_servo.state = SERVO_HOPE_RESET_ORIGIN;
	ch2_device.packge_servo.hope_run_path = RESET_ORIGIN_PATH;
	ch2_device.packge_servo.current_step = 0;
}

static uint8_t device_reset_origin_flag[2]={0,0};
static uint16_t device_reset_origin_time[2]={0,0};
// 清除设备复位到原点的信息
void clear_reset_origin_flag(uint8_t ch)
{
	if(ch ==CH1_DEVICE_ID)
	{
   device_reset_origin_flag[0]=0;
	 device_reset_origin_time[0]=0;
	}
	if(ch ==CH2_DEVICE_ID)
	{
   device_reset_origin_flag[1]=0;
	 device_reset_origin_time[1]=0;
	}
}

static rt_tick_t step_moto_power_down_time[2]={0,0};
// 检测步进电机的时间，大于1s则关闭步进电机电源，大于1.3s则步进电机
uint8_t check_step_moto_power_down(uint8_t ch,uint16_t pin)
{
	uint32_t run_time;
	
	run_time = count_use_time(step_moto_power_down_time[ch]);
	if(run_time>1000)					// 大于1s
	{
	  rt_pin_write(pin, 0);		// 步进电机电源开启
	}
	if(run_time>1300)					// 大于1.3s
	{
		if(device_reset_origin_flag[ch]==1)	// 如果设备复位到原点
		{
			device_reset_origin_flag[ch]=2;
			set_recive_moto(ch,CUP_IS_CLOSE);	// 让蛋杯处于闭合状态
		}
	}
	if(run_time>1500)
	{
	  return 1;
	}
	else 
	{
	  return 0;
	}
}
// 检查复位是否完成
// 先关闭步进电机电源
// 检查接收伺服和包装伺服伺服到原点
void ch1_check_reset_origin_state()
{
	uint8_t state;
	if(device_reset_origin_flag[0]>0)		// 如果需要复位
	{
		 state = check_step_moto_power_down(0,RELAY1);
		 device_reset_origin_time[0]++;
			// 如果包装伺服和接受抓伺服处在原点位置并且复位时间大于50ms了
	   if((get_ext_pin(ch1_device.packge_servo.origin_pin)==IS_OPEN)&&(get_ext_pin(ch1_device.recive_servo.origin_pin)==IS_OPEN)&&(device_reset_origin_time[0]>50))
		 {
			 // 如果包装伺服和接受抓伺服处在空闲状态并且步进电机处于开机蛋杯闭合状态
			 if((ch1_device.recive_servo.state == SERVO_IDLE)&&(ch1_device.packge_servo.state== SERVO_IDLE)&&(state==1))
			 {
					// 清除包装伺服和接收伺服的主编码器数据
				 if((check_clear_servo_encode_pluse(&ch1_device.packge_servo)==1)&&(check_clear_servo_encode_pluse(&ch1_device.recive_servo)==1))
				 {
						set_alarm(CH1_DEVICE_ID,transport_server_origin_err,0);	// 报警清空
						ch1_device.origin_flag = RT_TRUE;		// 设备处于原点位置标志置为1
						ch1_device.reset_cup_open_position_flag=RT_TRUE;
						set_open_cup_part(&ch1_device);
					// next_key_press_flag[0]=1;
					  rt_kprintf("通道1复位成功\n");
				 }
				 else
				 {
					  rt_kprintf("通道1复位失败\n");
				 }
				 device_reset_origin_time[0]=0;		// 复位完了后将复位标志清零
				 device_reset_origin_flag[0]=0;		// 复位完了后将复位标志清零
			 }
	
		 }
	}
	else 
	{
		rt_pin_write(RELAY1, 0);
	}
}

//返回执行后的步数   1:按下  启动传输线, 2 接受抓下降  3 接受抓 返回 4 完成
uint8_t nex_step_key_press(ch_device_type *device,uint8_t step)
{
	uint8_t result=0;
	if(step==1)
	{
		if(device->packge_servo.state ==SERVO_IDLE)
		{
			if(device->packge_servo.current_step==0)
			{
			  device->packge_servo.hope_run_path=1;
				device->packge_servo.state = SERVO_HOPE_RUN_PATH;
				device->packge_servo.current_step =1;		// 先让包装通道步骤为第一步，此时包装伺服会执行步骤1的路径数据
			}
			result =2;
		}
		else
		{
		  rt_kprintf("包装通道忙\n");
			result = 0;
		}			
	}
	else if(step==2)
	{
		if(device->packge_servo.state ==SERVO_IDLE)
		{
			result =3;
		}
		else 
		{
		  result =2;
		}
	}
	else if(step==3)
	{
		uint8_t buf=0;
		
		buf = recive_servo_run_netx_step(device);	// 以后得包装通道步骤跟随接收伺服的运行而动作，接收伺服每放下一排鸡蛋，包装伺服的步骤就会加1步，然后执行该步骤
		if(buf>0)
		{
		  result=4;
		}
		else 
		{
		  result=3;
		}
	}
	else if(step==4)
	{
		
		if(device->packge_servo.state ==SERVO_IDLE)
		{
			result =5;
		}
	}
	recive_cup_run_not_check_tray(device);     // 接收爪 下降 蛋夹打开  上升 关闭
	recive_up_and_packge_run(device); //接收爪升起 同时 包装通道启动下一步
	servo_control(&device->packge_servo);
	servo_control(&device->recive_servo);
   
  if(result==5)
	{
	  result=0;
	}
	//rt_kprintf("return result=%d\n",result);
  return result;	
}

// 扫描通道1的单步执行按键
// 扫描到按键按下，则单步执行按键标志置为1
// 当单步执行按键标志为1，并且设备处于原点状态
void scan_ch1_press_next_step_key()
{

	if(get_ext_pin(CH1_RUN_NEX_STEP_PIN)==IS_OPEN)
	{
		rt_thread_delay(50);    //去抖动
		if((get_ext_pin(CH1_RUN_NEX_STEP_PIN)==IS_OPEN)&&(next_key_press_flag[0]==0))
		{
			next_key_press_flag[0]=1;
		}
	}		
	if(next_key_press_flag[0]>0)
	{
		if(ch1_device.origin_flag == RT_TRUE)
		{
			next_key_press_flag[0] = nex_step_key_press(&ch1_device,next_key_press_flag[0]);
		}
		else 
		{
		  next_key_press_flag[0]= 0;
		}
	}
}
// 扫描通道2的单步执行按键
// 扫描到按键按下，则单步执行按键标志置为1
// 当单步执行按键标志为1，并且设备处于原点状态
void scan_ch2_press_next_step_key()
{

	if(get_ext_pin(CH2_RUN_NEX_STEP_PIN)==IS_OPEN)
	{
		rt_thread_delay(50);    //去抖动
		if((get_ext_pin(CH2_RUN_NEX_STEP_PIN)==IS_OPEN)&&(next_key_press_flag[1]==0))
		{
			next_key_press_flag[1]=1;
		}
	}		
	if(next_key_press_flag[1]>0)
	{
		if(ch2_device.origin_flag == RT_TRUE)
		{
			next_key_press_flag[1] = nex_step_key_press(&ch2_device,next_key_press_flag[1]);		// 不停地调用这个函数，这里面的步骤会变化，不是next_key_press_flag[1]一直为1
		}
		else 
		{
		  next_key_press_flag[1]= 0;
		}
	}
}

// 检查复位是否完成
// 先关闭步进电机电源
// 检查接收伺服和包装伺服伺服到原点
void ch2_check_reset_origin_state()
{
	uint8_t state;
	if(device_reset_origin_flag[1]>0)
	{
		 state = check_step_moto_power_down(1,RELAY2);
		 device_reset_origin_time[1]++;
	   if((get_ext_pin(ch2_device.packge_servo.origin_pin)==IS_OPEN)&&(get_ext_pin(ch2_device.recive_servo.origin_pin)==IS_OPEN)&&(device_reset_origin_time[1]>200))
		 {
			 if((ch2_device.recive_servo.state == SERVO_IDLE)&&(ch2_device.packge_servo.state== SERVO_IDLE)&&(state==1))
			 {
				 
				 if((check_clear_servo_encode_pluse(&ch2_device.packge_servo)==1)&&(check_clear_servo_encode_pluse(&ch2_device.recive_servo)==1))
				 {
						set_alarm(CH2_DEVICE_ID,transport_server_origin_err,0);
						ch2_device.origin_flag = RT_TRUE;
						ch2_device.reset_cup_open_position_flag=RT_TRUE;
						set_open_cup_part(&ch2_device);
						// next_key_press_flag[1]=1;
						rt_kprintf("通道2复位成功 time=%d\n",device_reset_origin_time[1]);
				 }
				 else 
				 {
				    rt_kprintf("通道2复位失败\n");
				 }
				 device_reset_origin_flag[1]=0;
				 device_reset_origin_time[1]=0;
			 }
		 }
	}
	else 
	{
		rt_pin_write(RELAY2, 0);
	}
}
// 扫描通道1的复位按键
// 扫描到按键按下，执行复位动作
// 清除伺服报警
// 伺服处于开启状态
// 蛋杯使能关闭
// 接收伺服的蛋杯打开，蛋杯关闭标志置为0
// 单步执行标志清零
// 步进电机电源开启
// 让接收伺服，包装伺服，步进电机运行到原点位置
void scan_ch1_reset_to_origin_key()
{
	static uint8_t press_flag=0;
	
	
	ch1_check_reset_origin_state();
	if((get_ext_pin(CH1_RESET_ORIGIN_PIN)==IS_OPEN)&&(press_flag==0))
	{
		rt_thread_delay(50);    //去抖动
		if((get_ext_pin(CH1_RESET_ORIGIN_PIN)==IS_OPEN))
		{
			clear_alarm(&ch1_device.packge_servo);
			set_servo_on(&ch1_device.packge_servo,1);
			ch1_device.cup_en = RT_FALSE;
			ch1_device.recive_servo.cup_open_flag =RT_FALSE;
			ch1_device.recive_servo.cup_close_flag =RT_FALSE;
	    next_key_press_flag[0]= 0;
			clear_alarm(&ch1_device.recive_servo);
			set_servo_on(&ch1_device.recive_servo,1);
			step_moto_power_down_time[0] = rt_tick_get();
			rt_pin_write(RELAY1, PIN_HIGH);
			set_servo_reset_to_origin(&ch1_device.packge_servo,&ch1_device.recive_servo,ch1_device.cup_id);
			device_reset_origin_flag[0]=1;		// 让设备复位标志
			rt_kprintf("通道1 恢复原点\n");
		}
	}
	press_flag = get_ext_pin(CH1_RESET_ORIGIN_PIN);
}
// 扫描通道1的复位按键
// 扫描到按键按下，执行复位动作
// 清除伺服报警
// 伺服处于开启状态
// 蛋杯使能关闭
// 接收伺服的蛋杯打开，蛋杯关闭标志置为0
// 单步执行标志清零
// 步进电机电源开启
// 让接收伺服，包装伺服，步进电机运行到原点位置
void scan_ch2_reset_to_origin_key()
{
	static uint8_t press_flag=0;
	
	ch2_check_reset_origin_state();
	if((get_ext_pin(CH2_RESET_ORIGIN_PIN)==IS_OPEN)&&(press_flag==0))
	{
		rt_thread_delay(50);    //去抖动
		if((get_ext_pin(CH2_RESET_ORIGIN_PIN)==IS_OPEN))
		{
			clear_alarm(&ch2_device.packge_servo);
			set_servo_on(&ch2_device.packge_servo,1);
			
			ch2_device.cup_en = RT_FALSE;
			ch2_device.recive_servo.cup_open_flag =RT_FALSE;
			ch2_device.recive_servo.cup_close_flag =RT_FALSE;
			next_key_press_flag[1]= 0;
			clear_alarm(&ch2_device.recive_servo);
			set_servo_on(&ch2_device.recive_servo,1);
			step_moto_power_down_time[1] = rt_tick_get();
			rt_pin_write(RELAY2, PIN_HIGH);
			set_servo_reset_to_origin(&ch2_device.packge_servo,&ch2_device.recive_servo,ch2_device.cup_id);
			device_reset_origin_flag[1]=1;		// 让设备复位标志
			rt_kprintf("通道2 恢复原点\n");
		}
	}
	press_flag = get_ext_pin(CH2_RESET_ORIGIN_PIN);
}




// 通道1调试模式
// 不停地扫描单步执行按键
// 不停地扫描复位按键
// 检查并执行610发过来的接收伺服调试命令
// 检查并执行610发过来的包装伺服调试命令
// 接收伺服状态机执行
// 包装伺服状态机执行
// 每50次读取一下包装伺服运行的距离
// 每100次读取一下接收伺服运行的距离
void ch1_servo_debug_mode()
{
	static uint16_t ch1_cycle_count;
	
	scan_ch1_press_next_step_key();
	scan_ch1_reset_to_origin_key();
	check_user_debug_servo(&ch1_device.recive_servo,&ch1_debug_recive_servo);
	check_user_debug_servo(&ch1_device.packge_servo,&ch1_debug_packge_servo);
	check_recive_servo_in_limit_position(&ch1_device.recive_servo);
	servo_control(&ch1_device.recive_servo);
	servo_control(&ch1_device.packge_servo);
	ch1_cycle_count++;
  if(ch1_cycle_count==50)	// 每50次读取一下包装伺服运行的距离
	{
		ch1_device.packge_servo.move_distem = get_current_distem(ch1_device.packge_servo.id,PACKGE_BOX_LEN,PACKGE_SERVO_CYCLE_SETP);
	}
	if(ch1_cycle_count>=100)	// 每100次读取一下接收伺服运行的距离
	{
		ch1_cycle_count=0;
		ch1_device.recive_servo.move_distem = get_current_distem(ch1_device.recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
//		  rt_kprintf("packge1 len=%d recive1 len=%d\n",ch1_device.packge_servo.move_distem,ch1_device.recive_servo.move_distem);
	}
}
// 让包装伺服运行指定的路径
void debug_packge_servo_run_step(uint8_t id,int16_t pluse)
{
	rt_err_t result;
	int32_t wite_buf[2];
	wite_buf[0] = 0x0004EE42;
	wite_buf[1] = pluse;
	
	rt_thread_mdelay(100);
  result = set_servo_path_part(id,0x0720,(uint16_t *)&wite_buf[0],4);	// path58,地址0x0720，0x0004EE42 0100  1110  1110 		0100 0010 
																																			// 																速度 减速度时间编号 加速度时间编号		相对位置		single定位模式，完毕则停止
	DEBUG_CARE("设置路径PLUSE=%d result=%d\n",pluse,result);
	rt_thread_mdelay(100);
	if(id==1)
	{
		ch1_device.packge_servo.state = SERVO_HOPE_RUN_PATH;
		ch1_device.packge_servo.hope_run_path = DEBUG_PACKGE_BOX_STEP_PATH;	// 运行pr58
	}
	else if(id==2)
	{
		ch2_device.packge_servo.state = SERVO_HOPE_RUN_PATH;
		ch2_device.packge_servo.hope_run_path = DEBUG_PACKGE_BOX_STEP_PATH;	// 运行pr58
	}
	
	rt_kprintf("debug packge box id=%d step pluse=%d\n",id,pluse);
}
// 通道2调试模式
// 不停地扫描单步执行按键
// 不停地扫描复位按键
// 检查并执行610发过来的接收伺服调试命令
// 检查并执行610发过来的包装伺服调试命令
// 接收伺服状态机执行
// 包装伺服状态机执行
// 每50次读取一下包装伺服运行的距离
// 每100次读取一下接收伺服运行的距离
void ch2_servo_debug_mode()
{
	static uint16_t ch2_cycle_count;
	scan_ch2_press_next_step_key();
	scan_ch2_reset_to_origin_key();
	check_user_debug_servo(&ch2_device.recive_servo,&ch2_debug_recive_servo);
	check_user_debug_servo(&ch2_device.packge_servo,&ch2_debug_packge_servo);
	check_recive_servo_in_limit_position(&ch2_device.recive_servo);
	servo_control(&ch2_device.recive_servo);
	servo_control(&ch2_device.packge_servo);
	ch2_cycle_count++;
  if(ch2_cycle_count==50)
	{
		ch2_device.packge_servo.move_distem = get_current_distem(ch2_device.packge_servo.id,PACKGE_BOX_LEN,PACKGE_SERVO_CYCLE_SETP);
		//rt_thread_mdelay(5);
	}
	if(ch2_cycle_count>=100)
	{
		ch2_cycle_count=0;
		ch2_device.recive_servo.move_distem = get_current_distem(ch2_device.recive_servo.id,RECIVE_CLAW_LEN,RECIVE_SERVO_CYCLE_SETP);
	//	  rt_kprintf("packge2 len=%d recive2 len=%d\n",ch2_device.packge_servo.move_distem,ch2_device.recive_servo.move_distem);
	}
}
