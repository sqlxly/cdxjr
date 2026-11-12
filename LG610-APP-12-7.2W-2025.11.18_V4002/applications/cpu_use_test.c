

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


extern void eudisk();
void gpioi1_run(void *dat)
{

	while(1)
	{
	
	    eudisk();
		rt_thread_mdelay(10);
	}
}

static int cpu_use_init(void)
{
    rt_thread_t tid,tid1;
    uint8_t id=0;
    
		tid1 = rt_thread_create("test",gpioi1_run, (void *)&id,2048,RT_THREAD_PRIORITY_MAX-6, 10);
		if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		
	

    return RT_EOK;
}

INIT_APP_EXPORT(cpu_use_init);