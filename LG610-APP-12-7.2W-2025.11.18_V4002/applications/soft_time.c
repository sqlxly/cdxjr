/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-08-24     yangjie      the first version
 */
/*
* ????:?????
*
* ??????????????,???????,????????
* ??????????????????
*/
#include <rtthread.h>
/* ??????? */
 rt_timer_t timer1;
 rt_timer_t timer2;
 int cnt = 0;

static void timeout1(void *parameter)
{
   
   
    if ((cnt++) %100==0)
    {
       
        rt_kprintf("periodic timer wcnt=%d \n",cnt);
    }
}
/* ???2???? */
static void timeout2(void *parameter)
{
    rt_kprintf("one shot timer is timeout\n");
}
int timer_sample(void)
{
    /* ?????1  ????? */
    timer1 = rt_timer_create("timer1", timeout1,
                             RT_NULL, 10,
                             RT_TIMER_FLAG_PERIODIC);
    /* ?????1 */
    if (timer1 != RT_NULL)
        rt_timer_start(timer1);
    return 0;
}
/* ??? msh ????? */
MSH_CMD_EXPORT(timer_sample, timer sample);