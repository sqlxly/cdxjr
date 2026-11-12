/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     misonyo   first version
 * 2023-12-03     Meco Man     support nano version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <board.h>

extern int ecat_main_loop(void);

int main(void)
{
		ecat_main_loop();				//运行Ethercat从站代码
}
