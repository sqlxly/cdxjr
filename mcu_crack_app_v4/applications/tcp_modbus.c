/*
 * Copyright (c) 2006-2022, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-06-21     flybreak     first version
 */

#include <rtthread.h>

#include "mb.h"
#include "user_mb_app.h"
#include "./tools/common.h"

#ifdef PKG_MODBUS_SLAVE_SAMPLE
#define SLAVE_ADDR      MB_SAMPLE_SLAVE_ADDR
#define PORT_NUM        MB_SLAVE_USING_PORT_NUM
#define PORT_BAUDRATE   MB_SLAVE_USING_PORT_BAUDRATE
#else
#define SLAVE_ADDR      0x01
#define PORT_NUM        2
#define PORT_BAUDRATE   115200
#endif

#define PORT_PARITY     MB_PAR_EVEN

#define MB_POLL_THREAD_PRIORITY  13
#define MB_SEND_THREAD_PRIORITY  RT_THREAD_PRIORITY_MAX - 2

#define MB_POLL_CYCLE_MS 20

static struct rt_mailbox mb;

static char mb_pool[512];

extern void kinco_set_installation_layout_param(rt_uint16_t addr);
extern void kinco_set_system_work_param(rt_uint16_t addr);

extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];

extern void kinco_set_weghing_param( rt_uint16_t addr );
extern void kinco_set_crack_param( rt_uint16_t addr );

int kinco_mailbox_init(void)
{
	rt_err_t result;
	
  result = rt_mb_init( &mb, "mbt", &mb_pool[0], sizeof(mb_pool) / 4,  RT_IPC_FLAG_FIFO ); 
	
  if (result != RT_EOK)
  {
      rt_kprintf("init mailbox failed.\n");

      return -RT_ERROR;
  }	
	
	return RT_EOK;
}

void user_kinco_mb_send(rt_uint16_t baseAddr, rt_uint16_t len)
{
	rt_ubase_t value = 0;
	
	value = len;
	value <<= 16;
	value |= (baseAddr&0x0000ffff);
	rt_mb_send(&mb, (rt_uint32_t)value);
}

static void send_thread_entry(void *parameter)
{
    USHORT         *usRegHoldingBuf;
		MBReceiveType  tRecv;
	
    usRegHoldingBuf = usSRegHoldBuf;

    while (1)
    {
				if (rt_mb_recv(&mb, (rt_ubase_t *)&tRecv, RT_WAITING_FOREVER) == RT_EOK)
				{
						rt_kprintf("kinco recv reg[%d]=%d, len=%d\n", tRecv.baseAddr, usRegHoldingBuf[tRecv.baseAddr], tRecv.len);
						
						kinco_set_weghing_param( tRecv.baseAddr );
					
						kinco_set_crack_param( tRecv.baseAddr );
					
						kinco_set_installation_layout_param( tRecv.baseAddr );
					
						kinco_set_system_work_param( tRecv.baseAddr );
				}
    }
}

static void mb_slave_poll(void *parameter)
{
    if (rt_strstr(parameter, "RTU"))
    {
#ifdef PKG_MODBUS_SLAVE_RTU
        eMBInit(MB_RTU, SLAVE_ADDR, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
#else
        rt_kprintf("Error: Please open RTU mode first");
#endif
    }
    else if (rt_strstr(parameter, "ASCII"))
    {
#ifdef PKG_MODBUS_SLAVE_ASCII
        eMBInit(MB_ASCII, SLAVE_ADDR, PORT_NUM, PORT_BAUDRATE, PORT_PARITY);
#else
        rt_kprintf("Error: Please open ASCII mode first");
#endif
    }
    else if (rt_strstr(parameter, "TCP"))
    {
#ifdef PKG_MODBUS_SLAVE_TCP
        eMBTCPInit(502);
#else
        rt_kprintf("Error: Please open TCP mode first");
#endif
    }
    else
    {
        rt_kprintf("Error: unknown parameter");
    }
		
    eMBEnable();
		
    while (1)
    {
        eMBPoll();
        rt_thread_mdelay(MB_POLL_CYCLE_MS);
    }
}

static int kinco_mb_slave_init(void)
{
    rt_thread_t tid1 = RT_NULL, tid2 = RT_NULL;

		kinco_mailbox_init();
	
    tid1 = rt_thread_create("md_s_poll", mb_slave_poll, "TCP", 2048, MB_POLL_THREAD_PRIORITY, 10);
    if (tid1 != RT_NULL)
    {
        rt_thread_startup(tid1);
    }
    else
    {
        goto __exit;
    }

    tid2 = rt_thread_create("md_s_send", send_thread_entry, RT_NULL, 4096, MB_SEND_THREAD_PRIORITY, 10);
    if (tid2 != RT_NULL)
    {
        rt_thread_startup(tid2);
    }
    else
    {
        goto __exit;
    }

    return RT_EOK;

__exit:
    if (tid1)
        rt_thread_delete(tid1);
    if (tid2)
        rt_thread_delete(tid2);

    return -RT_ERROR;
}

INIT_APP_EXPORT(kinco_mb_slave_init);
