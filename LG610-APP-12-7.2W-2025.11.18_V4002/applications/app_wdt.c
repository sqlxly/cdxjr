#include <rtconfig.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <rtdbg.h>
#include "mdebug.h"
#include "stm32H7xx.h"
#include "stm32H7xx_hal.h"
static IWDG_HandleTypeDef hiwdg;


void MX_IWDG_Init(void)
{
	
  hiwdg.Instance = IWDG1;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;//
  hiwdg.Init.Reload = 4093;//  0-4096
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
		  DEBUG_ERR("wdg set timeout failed.");
  }
}

static void feed_wdt(void *args)
{
	MX_IWDG_Init();
	while(1)
	{
		
	  HAL_IWDG_Refresh(&hiwdg); //ÖØ×°Î¹¹·Öµ
		//DEBUG_WARN("feed dog\n");
		rt_thread_mdelay(5);
	}
}

int wdt_start()
{
   rt_thread_t tid1 = RT_NULL;

   
	 tid1 = rt_thread_create("wdt", feed_wdt, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX-10, 10);
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