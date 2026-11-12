
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


ALIGN(RT_ALIGN_SIZE)
#define PWM1_DEV_NAME        "pwm1"  /* PWM???? */
#define PWM3_DEV_NAME        "pwm3"  /* PWM???? */
#define PWM4_DEV_NAME        "pwm4"  /* PWM???? */
struct rt_device_pwm *pwm_dev[10];      /* PWM???? */

void test_pwm()
{
	uint32_t cycle,i;
  
	cycle = 	1500000;
	
	pwm_dev[0] =	(struct rt_device_pwm *) rt_device_find(PWM1_DEV_NAME);
  if (pwm_dev[0] == RT_NULL)
  {
     rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM1_DEV_NAME);
		 return ;
  }  
	 for(i=1;i<5;i++)
	 {
		rt_pwm_set(pwm_dev[0], i, cycle, cycle>>1);  // ch  ÆµÂÊ   Âö¿í
		rt_pwm_enable(pwm_dev[0], i);   
	 }
	 
	pwm_dev[1] =	(struct rt_device_pwm *) rt_device_find(PWM3_DEV_NAME);
  if (pwm_dev[1] == RT_NULL)
  {
     rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM3_DEV_NAME);
		 return ;
  }  
	 for(i=3;i<5;i++)
	 {

		rt_pwm_set(pwm_dev[1], i, cycle, cycle>>1);  // ch  ÆµÂÊ   Âö¿í
		rt_pwm_enable(pwm_dev[1], i);   
	 }
	 
	 
	 pwm_dev[2] =	(struct rt_device_pwm *) rt_device_find(PWM4_DEV_NAME);
  if (pwm_dev[2] == RT_NULL)
  {
     rt_kprintf("pwm sample run failed! can't find %s device!\n", PWM4_DEV_NAME);
		 return ;
  }  
	 for(i=1;i<5;i++)
	 {
		rt_pwm_set(pwm_dev[2], i, cycle, cycle>>1);  // ch  ÆµÂÊ   Âö¿í
		rt_pwm_enable(pwm_dev[2], i);   
	 }
	 rt_kprintf("pwm set ok\n");
}