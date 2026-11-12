#include "common.h"
//#include "at24cxx.h"
#include "netTools.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

//IEMI
char stm32_uniqueId[33]= {0};

//1.8k缓存 开机的时候生成二维码数据
char qrcodeData[900] = {0};
void system_error(void)
{
//	while(1){}
	rt_kprintf("write error!\n");
}

/* 设置网络连接相关参数 */
static TNetworkInfo netInfo, netInfoBak;

void setDisplayMode(rt_uint8_t mode)
{
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	
	if(netInfo.displayMode != mode){
		temp_netInfo.displayMode = mode;
		err = flash_write_device_config(&temp_netInfo);
	}
	if(err != RT_EOK){
		netInfo.displayMode = mode;
		__set_FAULTMASK(1); 
		NVIC_SystemReset();
	}else system_error();	
}

void setSlaveType(rt_uint8_t mode)
{
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	
	if(netInfo.slave_type != mode){
		temp_netInfo.slave_type = mode;
		err = flash_write_device_config(&temp_netInfo);
	}
	if(err != RT_EOK){
		netInfo.slave_type = mode;
	}else system_error();	
}

void setNetworkDynamicInfo(TNetworkInfo dInfo)
{
	rt_memcpy(&netInfo, &dInfo, sizeof(TNetworkInfo));
}

void setNetworkMode(rt_uint8_t mode)
{
	char ipAddr[20];
	char gwAddr[20];
	char maskAddr[20];
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;

	if(netInfo.netMode != mode){
		temp_netInfo.netMode = mode;
		err = flash_write_device_config(&temp_netInfo);
	}

	if(err != RT_EOK){
		netInfo.netMode = mode;
		lan_or_4G_select(mode);
		if(netInfo.ipMode == 1){
			sprintf(ipAddr, "%d.%d.%d.%d", netInfoBak.ip[0], netInfoBak.ip[1], netInfoBak.ip[2], netInfoBak.ip[3]);
			sprintf(maskAddr, "%d.%d.%d.%d", netInfoBak.nm[0], netInfoBak.nm[1], netInfoBak.nm[2], netInfoBak.nm[3]);
			sprintf(gwAddr, "%d.%d.%d.%d", netInfoBak.gw[0], netInfoBak.gw[1], netInfoBak.gw[2], netInfoBak.gw[3]);
			netdev_work_mode_select(1, 3, "%s%s%s", ipAddr, maskAddr, gwAddr);
		}else{
			netdev_work_mode_select(0, 0, "%s%s%s", "0.0.0.0", "0.0.0.0", "0.0.0.0");
		}
	}else system_error();	
}

void setGetIpMode(rt_uint8_t mode)
{
	char ipAddr[20];
	char gwAddr[20];
	char maskAddr[20];
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	if(netInfo.ipMode != mode){
		temp_netInfo.ipMode = mode;
		err = flash_write_device_config(&temp_netInfo);
	}
	
	if(err != RT_EOK){
		netInfo.ipMode = mode;
		if(netInfo.ipMode == 1){
			sprintf(ipAddr, "%d.%d.%d.%d", netInfoBak.ip[0], netInfoBak.ip[1], netInfoBak.ip[2], netInfoBak.ip[3]);
			sprintf(maskAddr, "%d.%d.%d.%d", netInfoBak.nm[0], netInfoBak.nm[1], netInfoBak.nm[2], netInfoBak.nm[3]);
			sprintf(gwAddr, "%d.%d.%d.%d", netInfoBak.gw[0], netInfoBak.gw[1], netInfoBak.gw[2], netInfoBak.gw[3]);
			netdev_work_mode_select(1, 3, "%s%s%s", ipAddr, maskAddr, gwAddr);
		}else{
			netdev_work_mode_select(0, 0, "%s%s%s", "0.0.0.0", "0.0.0.0", "0.0.0.0");
		}		
	}else system_error();	
}

void setIpAddress(rt_uint8_t index, rt_uint8_t v)
{
	char addr[20];
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	
	if(netInfo.ip[index] != v){
		temp_netInfo.ip[index] = v;
		err = flash_write_device_config(&temp_netInfo);
	}
	
	if(err != RT_EOK){
		netInfo.ip[index] = v;
		netInfoBak.ip[index] = v;
		sprintf(addr, "%d.%d.%d.%d", netInfoBak.ip[0], netInfoBak.ip[1], netInfoBak.ip[2], netInfoBak.ip[3]);
		set_netdev_static_address(0, addr);
	}else system_error();	
}

void setNmAddress(rt_uint8_t index, rt_uint8_t v)
{
	char addr[20];

	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	
	if(netInfo.nm[index] != v){
		temp_netInfo.nm[index] = v;
		err = flash_write_device_config(&temp_netInfo);
	}
	
	if(err != RT_EOK){
		netInfo.nm[index] = v;
		netInfoBak.nm[index] = v;
		sprintf(addr, "%d.%d.%d.%d", netInfoBak.nm[0], netInfoBak.nm[1], netInfoBak.nm[2], netInfoBak.nm[3]);
		set_netdev_static_address(1, addr);
	}else system_error();	
}

void setGwAddress(rt_uint8_t index, rt_uint8_t v)
{
	char addr[20];
	rt_err_t err = RT_EOK;
	TNetworkInfo temp_netInfo;
	temp_netInfo = netInfo;
	
	if(netInfo.gw[index] != v){
		temp_netInfo.gw[index] = v;
		err = flash_write_device_config(&temp_netInfo);
	}
	
	if(err != RT_EOK){
		netInfo.gw[index] = v;
		netInfoBak.gw[index] = v;
		sprintf(addr, "%d.%d.%d.%d", netInfoBak.gw[0], netInfoBak.gw[1], netInfoBak.gw[2], netInfoBak.gw[3]);
		set_netdev_static_address(2, addr);		
	}else system_error();	
}

rt_uint8_t getTNetworInfo_net_mode(void)
{
	rt_uint8_t mode = 0;
	mode = netInfo.netMode;
	return mode;
}

rt_uint8_t getTNetworInfo_ip_mode(void)
{
	rt_uint8_t mode;
	mode = netInfo.ipMode;
	return mode;
}

rt_uint8_t getTNetworInfo_ac2000_type(void)
{
	rt_uint8_t mode;
	mode = netInfo.slave_type;
	return mode;
}
rt_uint8_t getTNetworInfo_display_mode(void)
{
	rt_uint8_t mode;
	mode = netInfo.displayMode;
	return mode;
}
TNetworkInfo *getTNetworInfo(void)
{
	return &netInfo;
}

void readStm32UniqueID(void) 
{
	rt_uint32_t uid[4];

	uid[0] = *(__IO uint32_t *)(UID_BASEADDR);
	uid[1] = *(__IO uint32_t *)(UID_BASEADDR + 0x04);
	uid[2] = *(__IO uint32_t *)(UID_BASEADDR + 0x08);
	sprintf(stm32_uniqueId, "%08X%08X%08X", uid[0], uid[1], uid[2]);
	rt_kprintf("UID:%s\n", stm32_uniqueId);
}
#include "drv_flash.h"
#define STM32_FLASH_BASE_ADDR 0x8000000
#define DEST_ADDDR STM32_FLASH_BASE_ADDR+(2047*1024)//1K flash

static int flashInitSystemParam(void)
{
	rt_err_t rt = RT_EOK;
	TNetworkInfo netInfoDefault;
	netInfoDefault.displayMode = 1;
	netInfoDefault.ipMode = 0;
	netInfoDefault.netMode = 1;
	netInfoDefault.slave_type =1;
	
	sscanf(RT_LWIP_IPADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.ip[0],(int *)&netInfoDefault.ip[1],(int *)&netInfoDefault.ip[2],(int *)&netInfoDefault.ip[3]);
	sscanf(RT_LWIP_GWADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.nm[0],(int *)&netInfoDefault.nm[1],(int *)&netInfoDefault.nm[2],(int *)&netInfoDefault.nm[3]);
	sscanf(RT_LWIP_MSKADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.gw[0],(int *)&netInfoDefault.gw[1],(int *)&netInfoDefault.gw[2],(int *)&netInfoDefault.gw[3]);
	rt = stm32_flash_read(DEST_ADDDR, (rt_uint8_t *)&netInfo, sizeof(TNetworkInfo));
	if(rt == RT_ERROR) system_error();
	if(((1!=netInfo.displayMode)&&(2!=netInfo.displayMode))||(netInfo.slave_type>4))
	{
		stm32_flash_erase(DEST_ADDDR, sizeof(TNetworkInfo));
		stm32_flash_write(DEST_ADDDR,(rt_uint8_t *)&netInfoDefault,sizeof(TNetworkInfo));
		rt_memcpy(&netInfo, &netInfoDefault, sizeof(TNetworkInfo));
	}
	rt_memcpy(&netInfoBak, &netInfo, sizeof(TNetworkInfo));
	return rt;
}

rt_err_t factory_reset_device_config(void)
{
	TNetworkInfo netInfoDefault;
	netInfoDefault.displayMode = 1;
	netInfoDefault.ipMode = 0;
	netInfoDefault.netMode = 1;
	netInfoDefault.slave_type =1;
	sscanf(RT_LWIP_IPADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.ip[0],(int *)&netInfoDefault.ip[1],(int *)&netInfoDefault.ip[2],(int *)&netInfoDefault.ip[3]);
	sscanf(RT_LWIP_GWADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.nm[0],(int *)&netInfoDefault.nm[1],(int *)&netInfoDefault.nm[2],(int *)&netInfoDefault.nm[3]);
	sscanf(RT_LWIP_MSKADDR,"%d.%d.%d.%d", (int *)&netInfoDefault.gw[0],(int *)&netInfoDefault.gw[1],(int *)&netInfoDefault.gw[2],(int *)&netInfoDefault.gw[3]);
	stm32_flash_erase(DEST_ADDDR, sizeof(TNetworkInfo));
	return stm32_flash_write(DEST_ADDDR,(rt_uint8_t *)&netInfoDefault,sizeof(TNetworkInfo));
}

rt_err_t flash_write_device_config(TNetworkInfo *buf)
{
	stm32_flash_erase(DEST_ADDDR, sizeof(TNetworkInfo));
	return stm32_flash_write(DEST_ADDDR,(rt_uint8_t *)buf,sizeof(TNetworkInfo));
}

rt_err_t flash_read_device_config(TNetworkInfo *buf)
{
	return stm32_flash_read(DEST_ADDDR, (rt_uint8_t *)buf, sizeof(TNetworkInfo));
}

INIT_APP_EXPORT(flashInitSystemParam);
