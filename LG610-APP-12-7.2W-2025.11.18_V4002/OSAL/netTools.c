#include "netTools.h"
#include <netif/ethernetif.h>
#include <netdev.h>

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

void lan_or_4G_select(rt_int8_t sel)
{
	struct netdev *netdev = RT_NULL;

	if(sel == 1){
		if (check_networcard_default_set("air720") != RT_TRUE){
			netdev = netdev_get_by_name("air720");
			netdev_set_default(netdev);
		}
	}else{
		if (check_networcard_default_set("e0") != RT_TRUE){
			netdev = netdev_get_by_name("e0");
			netdev_set_default(netdev);
		}
	}
}

static void netdev_set_dhcp(struct netdev *netdev, rt_bool_t is_enabled)
{
	netdev_dhcp_enabled(netdev, is_enabled);
}

void set_netdev_static_address(rt_int8_t type, const char *str)
{
	struct netdev *netdev = RT_NULL;
	ip_addr_t addr;
	
	netdev = netdev_get_by_name("e0");
	switch(type){
		case 0: /* set IP address */
			if ((str != RT_NULL) && netdev_ip4addr_aton_ex(str, &addr))
			{
					netdev_set_ipaddr(netdev, &addr);
			}
			break;
		case 1: /* set gateway address */
			if ((str != RT_NULL) && netdev_ip4addr_aton_ex(str, &addr))
			{
					netdev_set_netmask(netdev, &addr);
			}
			break;
		case 2: /* set netmask address */
			if ((str != RT_NULL) && netdev_ip4addr_aton_ex(str, &addr))
			{
					netdev_set_gw(netdev, &addr);
			}
			break;
		default: break;
	}
}

void netdev_work_mode_select(rt_int8_t mode, rt_uint8_t len, char *fomart, ...)
{
	ip_addr_t addr;
	va_list args;
	struct netdev *netdev = RT_NULL;
	char *argn[3];
	rt_uint8_t n = 0;
	
	netdev = netdev_get_by_name("e0");
	if(mode == 1){  	
		/* 设置静态IP模式 */
		netdev_set_dhcp(netdev, RT_FALSE);
		
		va_start(args,fomart);
		for(n=0; n<len; n++){
			argn[n] = va_arg(args, char *);
			rt_kprintf("[arg%d]:%s\n", n, argn[n]);
		}
		va_end(args);		
		/* set IP address */
		if ((argn[0] != RT_NULL) && netdev_ip4addr_aton_ex(argn[0], &addr))
		{
				netdev_set_gw(netdev, &addr);
		}
		/* set gateway address */
		if ((argn[1] != RT_NULL) && netdev_ip4addr_aton_ex(argn[1], &addr))
		{
				netdev_set_gw(netdev, &addr);
		}
		/* set netmask address */
		if ((argn[2] != RT_NULL) && netdev_ip4addr_aton_ex(argn[2], &addr))
		{
				netdev_set_netmask(netdev, &addr);
		}
	}else{
		if(netdev!=RT_NULL)
		{
			rt_kprintf("netdev------DHCP--------\n");
		/* 设置动态IP模式 自动获取IP */		
		netdev_set_dhcp(netdev, RT_TRUE);
		}
		else
		{
			rt_kprintf("netdev--------------\n");
		}
	}
}
