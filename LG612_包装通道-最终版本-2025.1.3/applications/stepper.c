#include "stepper.h"
#include "string.h"
#include "math.h"
#include "stdlib.h"
#include "mdebug.h"
#include <rtdbg.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "ext_gpio.h"
#include "alarm_control.h"
#include "servo_common.h"

#define MAX_STEP 600
#define OPEN_STEP 530
#define MIN_STEP 0
#define MOTO_SPEED_PLUSE (3200*2)   //每秒多少个脉冲

static  MOTOR_TYPE motor[MTERR];              //步进电机设备
static  int32_t pwm_callback_count[MTERR]={MIN_STEP,MIN_STEP};   //步进电机PWM反馈计数
static  uint8_t moto_dir1=0xff;
static  uint8_t moto_dir2=0xff;
/*
*\??A pwm??????
*/

void clear_moto_step(uint8_t id)
{
  rt_base_t level;
	if(id>1)
	{
		return ;
	}
  level = rt_hw_interrupt_disable();
	pwm_callback_count[id] =0;
	rt_hw_interrupt_enable(level);
}
void check_a_step_moto_senser()
{
     uint8_t senser_state;
	   if((pwm_callback_count[MTA]>=OPEN_STEP)||(pwm_callback_count[MTA]<=MIN_STEP))
		{
	    rt_thread_mdelay(50);
	   if((pwm_callback_count[MTA]>=OPEN_STEP)||(pwm_callback_count[MTA]<=MIN_STEP))
		 {
			
			
	    senser_state = get_ext_pin(CH1_EGG_CUP_OPEN_SENSER_PIN);
			if((moto_dir1 ==CUP_IS_CLOSE)&&(senser_state !=1))  //关闭传感器未到位
			{
				set_alarm(CH1_DEVICE_ID,egg_cup_step_moto_err,1);
				rt_kprintf("电机关闭未到位1\n");
			}
			if((moto_dir1 ==CUP_IS_OPEN)&&(senser_state ==1))  //开启传感器未到位
			{
				set_alarm(CH1_DEVICE_ID,egg_cup_step_moto_err,1);
				rt_kprintf("电机开启未到位1\n");
			}
		}
		}
}

void check_b_step_moto_senser()
{
     uint8_t senser_state;
	    if((pwm_callback_count[MTB]>=OPEN_STEP)||(pwm_callback_count[MTB]<=MIN_STEP))
		  {
			rt_thread_mdelay(50);	
	    if((pwm_callback_count[MTB]>=OPEN_STEP)||(pwm_callback_count[MTB]<=MIN_STEP))
		  {
	    senser_state = get_ext_pin(CH2_EGG_CUP_OPEN_SENSER_PIN);
			if((moto_dir2 ==CUP_IS_CLOSE)&&(senser_state !=1))  //关闭传感器未到位
			{
				set_alarm(CH2_DEVICE_ID,egg_cup_step_moto_err,1);
				rt_kprintf("电机关闭未到位2\n");
			}
			if((moto_dir2 ==CUP_IS_OPEN)&&(senser_state ==1))  //开启传感器未到位
			{
				set_alarm(CH2_DEVICE_ID,egg_cup_step_moto_err,1);
				rt_kprintf("电机开启未到位2\n");
			}
		 }
		}
}
rt_err_t pwm_mtra_callback(rt_device_t dev, rt_size_t size)
{
		
		if(moto_dir1 ==CUP_IS_CLOSE)  //关闭
		{
			pwm_callback_count[MTA]++;
			
			if(get_ext_pin(CH1_EGG_CUP_OPEN_SENSER_PIN)==1)    //??????
			{
				pwm_callback_count[MTA] = OPEN_STEP;
				rt_pwm_disable(motor[MTA].device, motor[MTA].ch);                     
				rt_pwm_set(motor[MTA].device, motor[MTA].ch, 1500000, 0);  // ch  ??   ??
			}
		}
		else      //打开
		{
			pwm_callback_count[MTA]--;
			if(pwm_callback_count[MTA]<=MIN_STEP)
			{
				pwm_callback_count[MTA] =MIN_STEP;
			  rt_pwm_disable(motor[MTA].device, motor[MTA].ch);                     
				rt_pwm_set(motor[MTA].device, motor[MTA].ch, 1500000, 0);  // ch  ??   ??
			}
		}
		
		
		if((pwm_callback_count[MTA]>MAX_STEP)||(pwm_callback_count[MTA]<MIN_STEP))//检测打开到位
		{
			rt_pwm_disable(motor[MTA].device, motor[MTA].ch);                     
			rt_pwm_set(motor[MTA].device, motor[MTA].ch, 1500000, 0);  // ch  ??   ??
			rt_kprintf("moto1 is stop count=%d\n",pwm_callback_count[MTA]);
		}
    
    return RT_EOK;
}
/*
*\??B pwm??????
*/

rt_err_t pwm_mtrb_callback(rt_device_t dev, rt_size_t size)
{        
	  uint8_t senser_state;
    if(moto_dir2 ==CUP_IS_CLOSE)
		{
			pwm_callback_count[MTB]++;
			if(get_ext_pin(CH2_EGG_CUP_OPEN_SENSER_PIN)==1)    //??????
			{
				pwm_callback_count[MTB] = OPEN_STEP;
				rt_pwm_disable(motor[MTB].device, motor[MTB].ch);                     
			  rt_pwm_set(motor[MTB].device, motor[MTB].ch, 1500000, 0);  // ch  ??   ??
			}
			
		}
		else 
		{
			pwm_callback_count[MTB]--;
			
			if(pwm_callback_count[MTB]<=MIN_STEP)
			{
				pwm_callback_count[MTB] = MIN_STEP;
			  rt_pwm_disable(motor[MTB].device, motor[MTB].ch);                     
			  rt_pwm_set(motor[MTB].device, motor[MTB].ch, 1500000, 0);  // ch  ??   ??
			}
		}
		
		if((pwm_callback_count[MTB]>MAX_STEP)||(pwm_callback_count[MTB]<MIN_STEP))
		{
			rt_pwm_disable(motor[MTB].device, motor[MTB].ch);                     
			rt_pwm_set(motor[MTB].device, motor[MTB].ch, 1500000, 0);  // ch  ??   ??
			rt_kprintf("moto2 is stop count=%d\n",pwm_callback_count[MTB]);
		}
    
    return RT_EOK;
}

rt_err_t pwm_mtrb_callback1(rt_device_t dev, rt_size_t size)
{        

		
    return RT_EOK;
}

/****************************************/
//更新步进电机转速  0=停止
//周期 =PER_SECOND_TIME_COUNT / val  
//例如 1s一圈 val  = PER_SECOND_TIME_COUNT/3200  3200个脉冲为一圈 
/****************************************/
int update_moto_speed(uint8_t id,uint8_t dir,uint32_t val)
{
	static uint32_t up_val[6];
  rt_err_t result=0;
  MOTOR_TYPE *cmotor;
	
	cmotor = &motor[id];
	if(val ==0 )    //电机停止  清零转速计算
	{
		up_val[id]=0;
		rt_pin_write(cmotor->dir_pin, PIN_HIGH);
		rt_pin_write(cmotor->en_pin, PIN_LOW);
		rt_pwm_disable(cmotor->device, cmotor->ch);                     
		rt_pwm_set(cmotor->device, cmotor->ch, 1500000, 0);  // ch  频率   脉宽
		pwm_callback_count[id]=0;
		return RT_EOK;
	}
	else
	{
		up_val[id] =val;
		if(dir)
		{
			
			rt_pin_write(cmotor->dir_pin, PIN_HIGH);  //
		}
		else 
		{
			rt_pin_write(cmotor->dir_pin, PIN_LOW);  //
		}
		rt_pin_write(cmotor->en_pin, PIN_LOW);
		rt_pwm_enable(cmotor->device, cmotor->ch);        
		result = rt_pwm_set(cmotor->device, cmotor->ch, val, val>>1);  //百分之50的脉宽
	}
  return result;
}


/****************************************/
//步进电机 绑定PWM通道 以及 正转 使能 IO口  
/****************************************/
rt_err_t bind_motor_device(MOTOR_TYPE *motor,char *name,uint8_t ch,uint8_t en_pin,uint8_t dir_pin)
{
	 motor->ch = ch;
	 motor->dir_pin = dir_pin;
	 motor->en_pin = en_pin;
	 rt_pin_mode(dir_pin, PIN_MODE_OUTPUT);
   rt_pin_mode(en_pin, PIN_MODE_OUTPUT);    
   rt_pin_write(en_pin, PIN_HIGH);

	 motor->device = (struct rt_device_pwm *)rt_device_find(name);
   if (motor->device == RT_NULL)
   {
       rt_kprintf("pwm sample run failed! can't find %s device!\n", name);
       return RT_ERROR;
   }       
	 rt_pwm_set(motor->device, motor->ch, 1500000, 0);  // ch  频率   脉宽
	 rt_pwm_disable(motor->device, motor->ch);   
	 return RT_EOK;
}


void set_recive_moto(uint8_t id,uint8_t state)
{
	
	if(id>1)
	{
		rt_kprintf("moto id=%d\n",id);
		return;
	}
	if(state ==CUP_IS_OPEN)  //开启
	{
		if(id==0)
			moto_dir1=CUP_IS_OPEN;
		else
			moto_dir2=CUP_IS_OPEN;
	
		
		update_moto_speed(id,0,PER_SECOND_TIME_COUNT/MOTO_SPEED_PLUSE);
		
	}
	else if(state ==CUP_IS_CLOSE)  //关闭
	{
		if(id==0)
		  moto_dir1=CUP_IS_CLOSE;
		else
			moto_dir2=CUP_IS_CLOSE;
	
		update_moto_speed(id,1,PER_SECOND_TIME_COUNT/MOTO_SPEED_PLUSE);
		//rt_kprintf("close pwm val=%d\n",pwm_callback_count[id]);
	}
}


/****************************************/
//步进电机初始化 
/****************************************/
static int moto_init()
{
	rt_err_t result=0;
	result =  bind_motor_device(&motor[MTA],"pwm4", 1, MTRA_ENABLE_PIN, MTRA_DIRECT_PIN);  
	result += bind_motor_device(&motor[MTB],"pwm3", 1, MTRB_ENABLE_PIN, MTRB_DIRECT_PIN);   
	result += rt_device_set_rx_indicate(&motor[MTA].device->parent, pwm_mtra_callback);
	result += rt_device_set_rx_indicate(&motor[MTB].device->parent, pwm_mtrb_callback);
	if(result != RT_EOK)
	{
		rt_kprintf("电机初始化错误\n");
		return RT_ERROR;
	}
	rt_kprintf("电机初始化\n");
  return RT_EOK;
}

//解析接收抓电机命令
void decode_recive_cup_dat(uint8_t *dat)   
{
  set_recive_moto(dat[1],dat[2]);	
}

INIT_ENV_EXPORT(moto_init);

static void smoto()
{
	rt_kprintf("A=%d B=%d\n",pwm_callback_count[0],pwm_callback_count[1]);
}
MSH_CMD_EXPORT(smoto, 111);