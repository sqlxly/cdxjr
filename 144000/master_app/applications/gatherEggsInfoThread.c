#include <rtthread.h>
#include <drv_common.h>
#include "agile_modbus.h"
#include "dev_config.h"
#include "mdbus_tcp_slaver.h"
#include "dirtyAndColorEggsQueue.h"
#include "weighAndCrackQueue.h"
#include "user_kinco.h"
#include "repostCacheQueue.h"
#include "mdbus_tcp_slaver.h"
#include "production_param.h"

static struct rt_thread gather_thread;
static char gather_eggs_info_thread_stack[2048];

static struct rt_mailbox mb_gat;
static char mb_gat_pool[512];

int kinco_mailbox_init(void)
{
	rt_err_t result;
	
  result = rt_mb_init( &mb_gat, "mb_gat", &mb_gat_pool[0], sizeof(mb_gat_pool) / 4,  RT_IPC_FLAG_FIFO ); 
	
  if (result != RT_EOK)
  {
      rt_kprintf("init mailbox failed.\n");

      return -RT_ERROR;
  }	
	
	return RT_EOK;
}

void mb_tcp_send(rt_uint16_t baseAddr, rt_uint16_t len)
{
	rt_ubase_t value = 0;
	
	value = len;
	value <<= 16;
	value |= (baseAddr&0x0000ffff);
	rt_mb_send(&mb_gat, (rt_uint32_t)value);
}

void gather_eggs_info_thread_entry(void *param)	
{
	MBReceiveType  tRecv;
	
	kinco_mailbox_init();  			//初始化邮箱

	init_modbus_tcp_pthread(); 	//初始化modbus tcp从站线程
	
	find_storage_descriptor();  //初始化norflash存储数据区
	
	externalPageAddressInit();  //初始化步科屏的一些参数
	
	while(RT_TRUE)
	{
			if (rt_mb_recv(&mb_gat, (rt_ubase_t *)&tRecv, RT_WAITING_FOREVER) == RT_EOK)
			{
					dirtyAndColorModbusDataProc(tRecv.baseAddr, tRecv.len);  //收到脏蛋和颜色数据
					
					weighAndCrackModbusDataProc(tRecv.baseAddr, tRecv.len);  //收到称重和裂纹数据
					
					if(tRecv.baseAddr< 10000)
					{
							rt_kprintf("modbus recv reg=%d, len=%d %d\n", tRecv.baseAddr, tRecv.len,u_tab_registers[tRecv.baseAddr]);	
						
							externalPageAddressProc(tRecv.baseAddr, tRecv.len);		 //根据步科屏幕页面地址处理数据
						
							eepromPageAddressProc(tRecv.baseAddr, tRecv.len);			 //根据步科屏幕页面地址处理EEPROM数据存取逻辑
						
							button_event_handler(tRecv.baseAddr, tRecv.len);			 //用于用户的一些点击事件的处理
					}
			}
	}
}

int gatherEggsInfoThreadInit(void)
{
		rt_thread_init(&gather_thread,
                   "gather",
                   gather_eggs_info_thread_entry,
                   RT_NULL,
                   &gather_eggs_info_thread_stack[0],
                   sizeof(gather_eggs_info_thread_stack),
                   16, 10);
    rt_thread_startup(&gather_thread);
									 
		return RT_EOK;
}

INIT_APP_EXPORT(gatherEggsInfoThreadInit);

static void PrintfQueueInfo(int argc)
{
	rt_kprintf("weighA front %d, rear %d\r\n", weighQueueA.front, weighQueueA.rear);
	rt_kprintf("weighB front %d, rear %d\r\n", weighQueueB.front, weighQueueB.rear);
	
	rt_kprintf("dirtyA front %d, rear %d\r\n", dirtyQueueA.front, dirtyQueueA.rear);
	rt_kprintf("dirtyB front %d, rear %d\r\n", dirtyQueueB.front, dirtyQueueB.rear);
	
	rt_kprintf("gather front %d, rear %d\r\n", gatherQueue.front, gatherQueue.rear);
}

MSH_CMD_EXPORT_ALIAS(PrintfQueueInfo, printf_queue_info, print queue information);
