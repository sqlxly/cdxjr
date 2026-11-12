#include <rtconfig.h>
#include <rtthread.h>
#include <rtdevice.h>

#include "stm32f4xx_hal.h"
static IWDG_HandleTypeDef hiwdg;


void MX_IWDG_Init(void)
{
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;//
  hiwdg.Init.Reload = 4095;//  0-4096
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
		  rt_kprintf("wdg set timeout failed.");
  }
}

static void feed_wdt(void *args)
{
	while(1)
	{
	  HAL_IWDG_Refresh(&hiwdg); //ÖØ×°Î¹¹·Öµ
		rt_thread_mdelay(100);
	}
}

int wdt_start()
{
   rt_thread_t tid1 = RT_NULL;

   MX_IWDG_Init();
	 tid1 = rt_thread_create("wdt", feed_wdt, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX-1, 10);
   if (tid1 != RT_NULL)
   {
       rt_thread_startup(tid1);
   }
   else
   {
     rt_thread_delete(tid1);
		 return -RT_ERROR;
   }
   return RT_EOK;
}