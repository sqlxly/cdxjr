/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-05     whj4674672   first version
 */


/*
1.增加主机停机过了一圈才停机
2.修改了第四个产品的优先级最低，捡破烂。
3.修改停机1S前更新从机状态 速度为0后 5S强制为空闲状态
4.999 为清蛋功能 所有的蛋都从通道1出去
5.增加复制参数时 包装通道拷贝到EEPROM去
6.修改增加收到鸡蛋脉冲称重脉冲
7.存储历史有问题
8.增加脏蛋和裂设备故障
*/

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_key.h"
#include "vs1053.h"
#include "mdebug.h"
#include "e6b2.h"
#include "cpu.h"
#include "common.h"
#include "relay.h"
#include "app_screen.h"
#include "modbus_screen.h"
#include "screen_chat.h"
#include "product_control.h"
#include "dat_backup.h"
#include "device_alarm.h"
#include "app_io_control.h"
#include "system_part.h"
#include "app_security.h"

#define LED_RUN_PIN    GET_PIN(A, 15)
ALIGN(RT_ALIGN_SIZE)

//extern void calculate_separate_egg();
extern void show_force_ch_egg_info();


//1 修改 从机 中断函数  以及注释主机开启电机2
//2 注释模拟上蛋  transfer_egg_to_separate_line 
//

void mis_devide_run()
{
	  check_en_print();
    refresh_screen();
	  alarm_loop_check();
    transport_io_control();
}

int main(void)
{
    uint8_t state;
	uint8_t pay_money_flag;
	uint8_t cpu_hi,cpu_low;
	uint32_t time_count=0;
	uint32_t cpu_use;
	uint16_t egg_per_hou=0;
	rt_tick_t run_time;
    float buf=0;
	rt_pin_mode(LED_RUN_PIN, PIN_MODE_OUTPUT);
    uint32_t SysClk = HAL_RCC_GetSysClockFreq();
    uint32_t HClk = HAL_RCC_GetHCLKFreq();
    uint32_t PClk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t PClk2 = HAL_RCC_GetPCLK2Freq();
	rt_kprintf("SysClk=%dM\n HClk=%d PClk1=%d \n PClk2=%d\n",SysClk/1000000,HClk,PClk1,PClk2);
	show_save_addr();
	time_count=0;

  while (1)
  {
	  
      state = get_system_state();
		  lamp_pole_disp(state);
		  buf = get_spt_speed();
		  buf = buf/63.5;
		  buf = buf*3600;
		  egg_per_hou = (int)buf;
		  buf = buf*360;
      time_count+=2;
	    rt_thread_mdelay(2);
			cpu_usage_get(&cpu_hi,&cpu_low);
      disp_debug_number1(cpu_hi);
		  disp_debug_number3(egg_per_hou/10);
	    cpu_use += cpu_hi;
			if((time_count %500)==0)
			{
				//rt_kprintf("cpu=%d\n",cpu_hi);
				rt_pin_write(LED_RUN_PIN,1);
			}
			else if((time_count /1000) ==1)
			{
				show_force_ch_egg_info();
				cpu_use =0;
				rt_pin_write(LED_RUN_PIN, PIN_LOW);
				time_count=0;	
				//rt_kprintf("spt pluse=%d trsp=%d\n",get_spt_encode_pluse(),get_tsp_encode_pluse());
			}
			run_time = rt_tick_get();
			if(run_time<25000) 
			{
				clear_warn_alarm();
			}
			process_system_part_fun();
			mis_devide_run();
  }
  return RT_EOK;
}


