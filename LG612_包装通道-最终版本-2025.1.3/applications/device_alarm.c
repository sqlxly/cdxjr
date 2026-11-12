
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <sys/select.h>




void servo_control_thread(void *arg)
{
	uint8_t busy=0;
	uint16_t cycle_count=0;
	rt_thread_mdelay(1000);   //µÈ´ýMODBUS ÆðÀ´
	device_init();
	product_packge_init(&ch1_device);
	product_packge_init(&ch2_device);
	
	set_servo_on(&ch1_device.packge_servo,1);
	rt_thread_mdelay(50);
	set_servo_on(&ch1_device.recive_servo,1);
	rt_thread_mdelay(50);

  int count=0;
	while(1)
	{
		recive_egg_delay_fun();
		system_is_runing(&ch1_device);
//		system_is_runing(&ch2_device);
		rt_thread_mdelay(4);
		
		if(count++>500)
		{
			rt_kprintf("ch1 state=%d ch2 state=%d\n",ch1_device.status,ch2_device.status);
			count =0;
		}
	}
}



static int servo_control_init(void)
{
    rt_thread_t tid,tid1,tid2;

    tid = rt_thread_create("amb_m",servo_control_thread, RT_NULL,2048,15, 10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

		tid1 = rt_thread_create("test",test_run, RT_NULL,512,15, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}

INIT_APP_EXPORT(servo_control_init);