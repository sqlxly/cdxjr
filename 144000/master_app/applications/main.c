/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-03-05     whj4674672   first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "user_outputIO.h"
#include "user_inputIO.h"
#include "user_encoder.h"
#include "dev_config.h"
#include "user_button.h"
#include "user_FSmachine.h"
#include "sort_process_thread.h"

uint8_t egg_detect[4];
uint8_t egg = 0;
uint8_t  cup_step_pulse = 0xff,cup_step = 0;
int main(void)
{
	uint8_t button_cycle_cnt = 0;
	
	configRelayIO();						//初始化继电器端口
	
	hc165ConfigIO();						//初始化输入扩展端口
	
	hc595ConfigIO();						//初始化输出扩展端口
	
	channel_button_init();			//初始化按钮
	
	firstPulseEncoderInit();		//初始化第一个编码器
	
	allCacheQueueInit();				//初始化数据缓冲队列

	sort_process_thread_init();	//初始化鸡蛋分级计算任务
	
  while (1)
  {
			// 读取所有输入端口状态
			hc165ReadData(masterOperate.inputData, 5);

			// 每20ms周期调用按钮处理函数 检测按钮状态并处理按键值
			button_cycle_cnt = (button_cycle_cnt + 1) % 5;
			if(0 == button_cycle_cnt) Button_Process();

			//蛋品分级主控制流程
			machine_work_process();
		
			egg = (egg + 1) % 125;
			cup_step_pulse = get_input_io_sta(masterOperate.inputData, eInputIO15); // 蛋杯步进
		if(cup_step_pulse == 0)
		{
			 cup_step ++;
			 rt_kprintf("%d\n",cup_step);
		}
		else cup_step = 0;

		/*
			if(0 == egg)
			{
					egg_detect[0] = get_input_io_sta(masterOperate.inputData, eInputIO24); // 主轨 1 反射
					egg_detect[1] = get_input_io_sta(masterOperate.inputData, eInputIO17); // 主轨 2 反射
					egg_detect[2] = get_input_io_sta(masterOperate.inputData, eInputIO18); // 主轨 3 反射
					egg_detect[3] = get_input_io_sta(masterOperate.inputData, eInputIO19); // 主轨 4 反射
					rt_kprintf("%d,%d,%d,%d\n",egg_detect[0],egg_detect[1],egg_detect[2],egg_detect[3]);
			}						
		*/

			//更新输出端口状态
			hc595WriteData(masterOperate.outputData, 6);

			rt_thread_mdelay(4);
  }
}
