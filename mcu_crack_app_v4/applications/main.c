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
#include "./tools/common.h"
#include "drv_hx71708.h"
#include "push_out.h"
#include <stdio.h>
#include <netif/ethernetif.h>
#include <netdev.h>
#include "mb.h"
#include "user_mb_app.h"

#define IPADDR_ID1 GET_PIN(E,4)
#define IPADDR_ID2 GET_PIN(E,3)
#define IPADDR_ID3 GET_PIN(E,2)

#define PULSE_ENCODER_DEV_NAME    "pulse1"

rt_device_t pulse_encoder_dev = RT_NULL;  

extern rt_uint8_t udisk_connected;

extern USHORT usSRegHoldBuf[S_REG_HOLDING_NREGS];

extern void deal_bkupdate_process(rt_uint16_t	*pReg);

extern int fdcan_auto_init(void);

extern int udpclient_init(void);

extern int system_workflow_init(void);

extern void software_filter_init(void);

extern int HX71708_exception_handler(void);

extern int app_weigh_process(void);

void Pulse_Encoder_init(void)
{
		rt_err_t ret = RT_EOK;
	
    pulse_encoder_dev = rt_device_find(PULSE_ENCODER_DEV_NAME);
    if (pulse_encoder_dev == RT_NULL)
    {
        rt_kprintf("pulse encoder sample run failed! can't find %s device!\n", PULSE_ENCODER_DEV_NAME);
    }

    ret = rt_device_open(pulse_encoder_dev, RT_DEVICE_OFLAG_RDONLY);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", PULSE_ENCODER_DEV_NAME);
    }
		
    ret = rt_device_control(pulse_encoder_dev, PULSE_ENCODER_CMD_ENABLE, RT_NULL);
    if (ret != RT_EOK)
    {
        rt_kprintf("enable pulse encoder failed!\n");
    }
}

int16_t read_pulse_encoder(void)
{
	int32_t count;
	int16_t encoder;
	
	rt_device_read(pulse_encoder_dev, 0, &count, 1);
	
	if(count > 0x3fff) encoder = (int16_t)(count - 0x7fff);
	else encoder = (int16_t)count;
	
	return encoder;
}

//int32_t read_pulse_encoder(void)
//{
//	int32_t count;
//	
//	rt_device_read(pulse_encoder_dev, 0, &count, 1);

//	return (int16_t)count;
//}

int netdev_ip4addr_aton_ex(const char *cp, ip4_addr_t *addr)
{
    uint32_t val;
    uint8_t base;
    char c;
    uint32_t parts[4];
    uint32_t *pp = parts;

    c = *cp;
    for (;;)
    {
        /*
         * Collect number up to ``.''.
         * Values are specified as for C:
         * 0x=hex, 0=octal, 1-9=decimal.
         */
        if (!isdigit(c))
        {
            return 0;
        }
        val = 0;
        base = 10;
        if (c == '0')
        {
            c = *++cp;
            if (c == 'x' || c == 'X')
            {
                base = 16;
                c = *++cp;
            }
            else
            {
                base = 8;
            }
        }
        for (;;)
        {
            if (isdigit(c))
            {
                val = (val * base) + (uint32_t) (c - '0');
                c = *++cp;
            }
            else if (base == 16 && isxdigit(c))
            {
                val = (val << 4) | (uint32_t) (c + 10 - (islower(c) ? 'a' : 'A'));
                c = *++cp;
            }
            else
            {
                break;
            }
        }
        if (c == '.')
        {
            /*
             * Internet format:
             *  a.b.c.d
             *  a.b.c   (with c treated as 16 bits)
             *  a.b (with b treated as 24 bits)
             */
            if (pp >= parts + 3)
            {
                return 0;
            }
            *pp++ = val;
            c = *++cp;
        }
        else
        {
            break;
        }
    }
    /*
     * Check for trailing characters.
     */
    if (c != '\0' && !isspace(c))
    {
        return 0;
    }
    /*
     * Concoct the address according to
     * the number of parts specified.
     */
    switch (pp - parts + 1)
    {

    case 0:
        return 0; /* initial nondigit */

    case 1: /* a -- 32 bits */
        break;

    case 2: /* a.b -- 8.24 bits */
        if (val > 0xffffffUL)
        {
            return 0;
        }
        if (parts[0] > 0xff)
        {
            return 0;
        }
        val |= parts[0] << 24;
        break;

    case 3: /* a.b.c -- 8.8.16 bits */
        if (val > 0xffff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16);
        break;

    case 4: /* a.b.c.d -- 8.8.8.8 bits */
        if (val > 0xff)
        {
            return 0;
        }
        if ((parts[0] > 0xff) || (parts[1] > 0xff) || (parts[2] > 0xff))
        {
            return 0;
        }
        val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
        break;
    default:
        RT_ASSERT(0);
        break;
    }
    if (addr)
    {
        ip4_addr_set_u32(addr, htonl(val));
    }
    return 1;
}

/**
 * 根据拨码选择LWIP的IP地址
 */
void reset_ipaddr(void)
{
	int ipaddr_offset = 0;
	char ipAddr[20];
	ip_addr_t addr;
	struct netdev *netdev = RT_NULL;
	
	rt_pin_mode(IPADDR_ID1, PIN_MODE_INPUT);
	rt_pin_mode(IPADDR_ID2, PIN_MODE_INPUT);
	rt_pin_mode(IPADDR_ID3, PIN_MODE_INPUT);
	
	if(rt_pin_read(IPADDR_ID3) == 0)
		ipaddr_offset += 4;
	if(rt_pin_read(IPADDR_ID2) == 0)
		ipaddr_offset += 2;
	if(rt_pin_read(IPADDR_ID1) == 0)
		ipaddr_offset += 1;

	netdev = netdev_get_by_name("e0");
	sprintf(ipAddr, "%d.%d.%d.%d", 10, 10, 0, 41+ipaddr_offset);
	
	netdev_ip4addr_aton_ex(ipAddr, &addr);
	
	netdev_set_ipaddr(netdev, &addr);
	
	/* set IP address */
	netdev_ip4addr_aton_ex(ipAddr, &addr);
	netdev_set_ipaddr(netdev, &addr);
	
//	/* set netmask address */
//	netdev_ip4addr_aton_ex("255.255.255.0", &addr);
//	netdev_set_netmask(netdev, &addr);
//	
//	/* set gateway address */
//	netdev_ip4addr_aton_ex("10.10.0.1", &addr);
//	netdev_set_gw(netdev, &addr);
//	netdev_set_dns_server(netdev, 0, &addr);
}

int main(void)
{
		rt_int32_t delayms = 2000;
		int updateList = 0;
	
		hx71708_gpio_config();
	
		push_out_gpio_init();
	
		software_filter_init();		//初始化软件滤波器
		
		fdcan_auto_init();				//初始化CAN总线
	
		Pulse_Encoder_init();   	//初始化编码器驱动

		reset_ipaddr();						//根据拨码开关重置IP地址
	
		udpclient_init();					//先初始化UDP线程 创建好UDP发送数据的工作队列 然后再启动接近开关检测线程
	
		system_workflow_init();

		hx71708_auto_work();

		app_weigh_process();

    while (1)
    {
			rt_thread_mdelay(delayms);

			if(HX71708_exception_handler())  //判断hx71708芯片是否异常 若出现异常 对异常芯片复位
			{
					delayms = 2000;
			}else{
					delayms = 100;			
			}

			if(udisk_connected == RT_TRUE){  //扫描U盘状态 更新文件列表
				if(updateList == 0){
					updateList = 1;
					deal_bkupdate_process(&usSRegHoldBuf[999]);
				}
			}else{
				deal_bkupdate_process(&usSRegHoldBuf[999]);
				updateList = 0;
			}
    }
}
