#include <rtthread.h>
#include "user_spi_slaver.h"
#include "user_alarm.h"

/* defined the LED0 pin: PB1 */
#define LED_RUN    GET_PIN(C, 12)

static struct rt_thread ecat_thread;
static char ecat_thread_stack[1024];

extern __IO uint8_t u_spiTxBuf[1024];   // SPI发送数据缓存
extern __IO uint8_t u_spiRxBuf[1024];	 	// SPI接收数据缓存

/* ethercat通讯守护线程 负责与EC-master芯片通讯并执行异常处理逻辑 */
void ethercat_master_thread_entry(void *param)	
{
	int32_t	timeout = 30*1000; 	//开机等待30秒 
	
	rt_pin_mode(LED_RUN, PIN_MODE_OUTPUT);

	createSpiSemaphore();				//创建SPI接受完成信号量
	
	bsp_InitSPIBus();						//初始化SPI从机

	bsp_spiTransfer(); 					//SPI从机先启动一次DMA传输 然后等待主机来读取数据
	
	while(RT_TRUE)
	{
		if(waitting_spi_sem_take(timeout) == RT_EOK)
		{
			bsp_spiTransfer();			//启动SPI DMA传输同步数据

			// 从SPI DMA通道读取数据到用户区
			rt_memcpy((uint8_t *)u_spiRxBuf, g_spiRxBuf, 1024);
			
			// 更新用户发送数据到SPI DMA缓存区
			rt_memcpy(g_spiTxBuf, (uint8_t *)u_spiTxBuf, 1024);
			
			/* 添加逻辑 处理节点在线断线 报警停机等逻辑 */
			
			timeout = 50;						//重置SPI通信超时时间为50ms

		} else {

			bsp_InitSPIBus();				//重新配置SPI参数
			
			rt_thread_mdelay(5);
			
			bsp_spiTransfer();			//再次启动数据传输
			
			timeout = 100;					//重置SPI通信超时时间为100ms
			
			trigger_alarm(ALARM_LEVEL_ERROR, ePackageConnectError, "NULL");
			
			rt_kprintf("timeout...\n");
		}
	}
}

int ethercat_master_thread_init(void)
{
		rt_thread_init(&ecat_thread,
                   "ecat",
                   ethercat_master_thread_entry,
                   RT_NULL,
                   &ecat_thread_stack[0],
                   sizeof(ecat_thread_stack),
                   5, 10);
    rt_thread_startup(&ecat_thread);
									 
		return RT_EOK;
}

INIT_APP_EXPORT(ethercat_master_thread_init);

static void ecat_send_data(int argc, char **argv)
{
		uint16_t port, sta;
		uint16_t *p;

		if(argc > 1){
			port = atoi(argv[1]);
		}
    if(argc > 2) {
			sta = atoi(argv[2]);
    }

		p = (uint16_t *)&u_spiTxBuf[256];
		
		*(p+port) = sta;
}

MSH_CMD_EXPORT_ALIAS(ecat_send_data, ecat_send_data, print send data to slave);

static void ecat_show_data(int argc, char **argv)
{
	int pos,ch,i;
	uint16_t *ptr;
	
	if(argc > 1){
		ch = atoi(argv[1]);
	}
	pos = 256+ch*16;
	ptr = (uint16_t *)&u_spiTxBuf[pos];
	for(i=0; i<8; i++){
		rt_kprintf("%04x ", *ptr);
		ptr++;
	}
	rt_kprintf("\n");
}

MSH_CMD_EXPORT_ALIAS(ecat_show_data, ecat_show_data, show ecat data);

int findClosestPosition(int arr[], int n, int target) 
{
    if (n <= 0) return -1;

    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2; 

        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (right < 0) {
        return 0; 
    }
    if (left >= n) {
        return n-1; 
    }

    int diffLeft = abs(arr[left] - target);
    int diffRight = abs(arr[right] - target);

    if (diffLeft < diffRight) {
        return left;
    } else {
        return right;
    }
}

static void printf_test(int argc, char **argv)
{
	int arr[8], target;
  for (int i = 0; i < 8; i++) {
      arr[i] = (i + 1) * 10;
  }
	if(argc > 1){
		target = atoi(argv[1]);
	}	
	
	int position = findClosestPosition(arr, 8, target);

    if (position != -1) {
        rt_kprintf("%d near %d is [%d]\n",  target, position, arr[position]);
    } else {
        rt_kprintf("NULL\n");
    }
}
MSH_CMD_EXPORT_ALIAS(printf_test, printf_test, printf test data);
