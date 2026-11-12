

#include <rtthread.h>
#include <rtdevice.h>

void gpioi1_run()
{
	rt_pin_mode(75, PIN_MODE_OUTPUT);
	while(1)
	{
	
	   rt_pin_write(75,0);
		 rt_thread_mdelay(1);
	   rt_pin_write(75,1);
		 rt_thread_mdelay(1);
	}


}

static int cpu_use_init(void)
{
    rt_thread_t tid,tid1;


		tid1 = rt_thread_create("test",gpioi1_run, RT_NULL,512,RT_THREAD_PRIORITY_MAX-1, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);

    return RT_EOK;
}

//INIT_APP_EXPORT(cpu_use_init);