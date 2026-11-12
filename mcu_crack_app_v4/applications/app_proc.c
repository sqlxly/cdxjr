#include <rtthread.h>

#define THREAD_PRIORITY      5
#define THREAD_TIMESLICE     5

static char app_thread_stack[1024];

static struct rt_thread app_thread;

static struct rt_mailbox mbapp;

static char mb_pool[128];

extern void balance_signal_handler(rt_uint32_t adc, int ch);

void app_mailbox_send(rt_uint32_t adc)
{
	rt_mb_send(&mbapp, (rt_ubase_t)adc);
}

static void app_proc_entry(void *parameter)
{
    rt_ubase_t adc;
		rt_uint8_t ch = 0;
	
    while (RT_TRUE)
    {
        if (rt_mb_recv(&mbapp, (rt_ubase_t *)&adc, RT_WAITING_FOREVER) == RT_EOK)
        {
						ch = (adc & 0x1f);			// adc在编码(存储)的时候，将低5位表示为每个通道，感觉还是有点不对，通道数不对，6个通道，难道0通道就是0，其他就用该位？

						balance_signal_handler((adc >> 5), ch);	// 收到数据处理
        }
    }

    rt_mb_detach(&mbapp);
}

int app_weigh_process(void)
{
  rt_err_t result;

  result = rt_mb_init(&mbapp, "mb_app", &mb_pool[0], sizeof(mb_pool) / 4, RT_IPC_FLAG_FIFO);       
  if (result != RT_EOK)
  {
     rt_kprintf("init mailbox failed.\n");
     return -1;
  }
	 
	rt_thread_init(&app_thread,
                   "app",
                   app_proc_entry,
                   RT_NULL,
                   &app_thread_stack[0],
                   sizeof(app_thread_stack),
                   THREAD_PRIORITY, THREAD_TIMESLICE);
									 
   rt_thread_startup(&app_thread);
									 
	 return RT_EOK;
}
