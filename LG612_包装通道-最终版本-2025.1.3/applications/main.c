/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2018-11-19     flybreak     add stm32f407-atk-explorer bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <sys/select.h>
#include "stepper.h"
#include "rs4851.h"
#include "string.h"
#include "mdebug.h"
#include "cpu.h"
#include "recode_dat.h"
#include "ext_gpio.h"
#include "alarm_control.h"
#include "servo_common.h"
#include "board_config.h"
#define LED0_PIN    GET_PIN(C, 10)


#include "stm32f4xx.h"

static void app_checkResetSource(void)
{
		rt_kprintf("app_checkResetSource\n");
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        rt_kprintf("Reset reason : POR/PDR reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        rt_kprintf("Reset reason : BOR or POR/PDR reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        rt_kprintf("Reset reason : PIN reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        rt_kprintf("Reset reason : Software Reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        rt_kprintf("Reset reason : Independent Watchdog reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        rt_kprintf("Reset reason : Window watchdog reset\n");
    }
    if(__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        rt_kprintf("Reset reason : Low power reset\n");
    }
    
    //__HAL_RCC_CLEAR_RESET_FLAGS();
}


uint32_t GetResetSource(void)
{
    uint32_t resetFlags = 0;
    
    // 读取RCC_CSR寄存器
    resetFlags = RCC->CSR;
    
    rt_kprintf("\n========== reset reasion ==========\n");
    rt_kprintf("\nresetFlags=0x%X\n", resetFlags);
	
    if(resetFlags & RCC_CSR_BORRSTF_Msk)
    {
        rt_kprintf("  - BORRSTF(BOR)\n");
    }
    if(resetFlags & RCC_CSR_PINRSTF_Msk)
    {
        rt_kprintf("  - PINRSTF(NRST)\n");
    }
    if(resetFlags & RCC_CSR_PORRSTF_Msk)
    {
        rt_kprintf("  - PORRSTF(POR/PDR)\n");
    }
    if(resetFlags & RCC_CSR_SFTRSTF_Msk)
    {
        rt_kprintf("  - SFTRSTF\n");
    }
    if(resetFlags & RCC_CSR_IWDGRSTF_Msk)
    {
        rt_kprintf("  - IWDGRSTF\n");
    }
    if(resetFlags & RCC_CSR_WWDGRSTF_Msk)
    {
        rt_kprintf("  - WWDGRSTF\n");
    }
    if(resetFlags & RCC_CSR_LPWRRSTF_Msk)
    {
        rt_kprintf("  - LPWRRSTF\n");
    }
    
    printf("==================================\n\n");
    
    // 清除所有复位标志（必须设置RMVF位）
    RCC->CSR |= RCC_CSR_RMVF;
    
    return resetFlags;
}


void show_sysclk()
{
		int SysClk = HAL_RCC_GetSysClockFreq();
    int HClk = HAL_RCC_GetHCLKFreq();
    int PClk1 = HAL_RCC_GetPCLK1Freq();
    int PClk2 = HAL_RCC_GetPCLK2Freq();
	  rt_kprintf("SysClk=%d\n",SysClk);
    rt_kprintf("HClk=%d\n",HClk);
    rt_kprintf("PClk1=%d\n",PClk1);
    rt_kprintf("PClk2=%d\n",PClk2);
}

uint8_t rs4851_rx[1024];

extern void check_a_step_moto_senser();
extern void check_b_step_moto_senser();
extern int wdt_start();

int main(void)
{
    /* set LED0 pin mode to output */
	  uint32_t tick_time;
   	uint32_t time_count=0;
	  uint8_t sn74hc165_dat[5];
	  uint8_t cpu_hi,cpu_low;
	  uint8_t up_flag=0;
	  uint8_t up_alarm_pin=0;
	  uint32_t  cpu_use=0;
    rt_pin_mode(LED0_PIN, PIN_MODE_OUTPUT);

		rt_kprintf("board addr=%d\n",read_board_addr());
    show_sysclk();
		//app_checkResetSource();
		//GetResetSource();
	  tick_time = rt_tick_get();
	 // update_moto_speed(1,1,PER_SECOND_TIME_COUNT/(3200));
	  tick_time  = rt_tick_get();
		wdt_start();

    while (1)
    { 
      time_count+=10;
			rt_thread_mdelay(10);
			cpu_usage_get(&cpu_hi,&cpu_low);
	    cpu_use += cpu_hi;

			check_a_step_moto_senser();
      check_b_step_moto_senser();
			if(time_count ==50)
			{
					rt_pin_write(LED0_PIN, PIN_HIGH);
			}
			else if(time_count ==100)
			{
				//if((rt_tick_get()-tick_time)>1000)
				//{
					if((cpu_use/100)>20)
					rt_kprintf("delay 1s cycle=%d  cpu use percentage=%d\n",rt_tick_get()-tick_time,cpu_use/100);
				//}
				cpu_use =0;
				time_count =0;
				tick_time  = rt_tick_get();
				rt_pin_write(LED0_PIN, PIN_LOW);

			}
		}

}
