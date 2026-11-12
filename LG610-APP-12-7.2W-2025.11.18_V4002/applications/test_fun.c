
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "err_check.h"
uint8_t clear_egg_count=0;
void set_clear_egg_flag()
{
  clear_egg_count=0x66;
}
extern void clear_product_egg();
extern uint16_t lose_info[2];
void clear_egg_fun()
{
    if(clear_egg_count==0x66)
	 {
		 lose_info[0]=0;
		 lose_info[1]=0;
		 clear_product_egg();
		 clear_egg_count=0;
	     clear_totle_eggs_count();
	 }
 }

 static void teat_app_fun()
 {
 
  while(1)
  {
    clear_egg_fun();
    rt_thread_mdelay(20);
  }
 }
 
 static int test_app_init()
{
		rt_thread_t tid=RT_NULL;

		tid = rt_thread_create("tapp", teat_app_fun, RT_NULL, 2048, RT_THREAD_PRIORITY_MAX-2, 10);
		if (tid == RT_NULL)
			return RT_ERROR;
        rt_thread_startup(tid);
       return 0;
	
}

INIT_APP_EXPORT(test_app_init);