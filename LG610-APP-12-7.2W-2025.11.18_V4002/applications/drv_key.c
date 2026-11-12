#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_key.h"
#include "mdebug.h"
#include "drv_74hcxx.h"
#include <hal_delay.h>

static uint32_t old_key_dat=0;
static uint32_t now_key_dat=0;

uint8_t get_immediately_key_val(uint32_t key_flag)
{
	uint8_t key=0;
	key = ((now_key_dat&key_flag)&&1);
	return key;
}

//两次采样值
uint8_t ge_key_val(uint32_t key_flag)
{
	uint8_t key,nkey,okey;

	nkey = ((now_key_dat&key_flag)&&1);
 // okey = ((old_key_dat&key_flag)&&1);
	//if(nkey == okey)
	  return nkey;
//	else 
	//	return 0xff;
}

uint32_t ge_all_key()
{
	return now_key_dat;
}

void ext_key_scan()
{
  rt_base_t level;
	uint32_t dat_tmp,red_dat;

	 sn74hcxx_get_dat((uint8_t *)&red_dat,3);	
   level = rt_hw_interrupt_disable();
	 dat_tmp = now_key_dat;
	 now_key_dat = red_dat>>4;
	 old_key_dat = dat_tmp;
	 rt_hw_interrupt_enable(level);
}



		