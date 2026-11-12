#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <math.h>
#include "mdebug.h"
#include "stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include "agile_modbus.h"
#include "rs4851.h"

#define RT_DEVICE_FLAG_RX_NON_BLOCKING    0x2000   
#define RT_DEVICE_FLAG_TX_BLOCKING        0x4000   

#define SAMPLE_UART_NAME  "uart3"    
#define RS4851_RE_PIN     GET_PIN(D, 8)
ALIGN(RT_ALIGN_SIZE)
static rt_device_t serial;
static rs4851_type rs4851; 
static rt_mutex_t rs4851_mutex = RT_NULL;



static rt_err_t rs4851_tx_complete(rt_device_t dev, void *buffer)
{
	  rt_pin_write(RS4851_RE_PIN, PIN_LOW);
    return RT_EOK;
}
// DMX 接受中断
static rt_err_t rs4851_rx_string(rt_device_t dev, rt_size_t size)
{
	  rt_base_t level;
    rt_size_t rx_length;
	  level = rt_hw_interrupt_disable();
	  if((UART3_RX_BUFSIZE -rs4851.rx_len)<size)
		{
			rs4851.rx_len=0;     //缓存溢出
		}
		rx_length = rt_device_read(serial, -1,&rs4851.dat[rs4851.rx_len], size);
		rs4851.rx_len += rx_length;
		rs4851.tick = rt_tick_get();
		rt_hw_interrupt_enable(level);
    return 0;
}

void rs4851_clear_buf()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_memset(&rs4851,0,sizeof(rs4851));	
	rt_hw_interrupt_enable(level);
}

//警告  如果有开销很大的线程 休眠延时不准确  一定要切换延时方式
uint16_t rs4851_read_dat(uint8_t *buf,uint16_t len ,uint8_t time_out)
{
    uint16_t rx_ren=0;
	  rt_base_t level;
    rt_tick_t ctick;
	
	  while(time_out--)
		{
			ctick = rt_tick_get();
			if(((ctick>(rs4851.tick+3))||(ctick<rs4851.tick))&&(rs4851.rx_len>0))   //有数据 + 断帧			2ms 为一帧
			{
				
				level = rt_hw_interrupt_disable();
				rx_ren = rs4851.rx_len;
				if(len>rs4851.rx_len)
				{
					rt_memcpy(buf,&rs4851.dat[0],rs4851.rx_len);
				}
				else 
				{
					rt_memcpy(buf,&rs4851.dat[0],len);
				}
				rt_memset(&rs4851,0,sizeof(rs4851));	
				rt_hw_interrupt_enable(level);
				return rx_ren;
			}
			rt_thread_mdelay(10);
		}
		return 0;   //超时
}

rt_err_t rs4851_send_dat(uint8_t *str,int len)
{
	rt_err_t ret = RT_EOK;
  
	if(rt_mutex_take(rs4851_mutex, 10)!=RT_EOK)                //上锁 
		return RT_ERROR;		
  rt_pin_write(RS4851_RE_PIN, PIN_HIGH);
	rt_thread_mdelay(1);
	ret = rt_device_write(serial, 0, str, len);
	rt_mutex_release(rs4851_mutex);        //释放锁
	return ret;
}


// DMA串口初始化
rt_err_t RS485_1_init()
{
    rt_err_t ret = RT_EOK;

    rt_pin_mode(RS4851_RE_PIN, PIN_MODE_OUTPUT);
	  rt_memset(&rs4851,0,sizeof(rs4851));
	
	  rs4851_mutex = rt_mutex_create("4851_lock", RT_IPC_FLAG_PRIO);
    if (rs4851_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return RT_ERROR;
    }
    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
        return RT_ERROR;
    }
		struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
		
		config.baud_rate = BAUD_RATE_115200;        
		config.data_bits = DATA_BITS_8;           
		config.stop_bits = STOP_BITS_1;           
		config.bufsz     = 512;                   
		config.parity    = PARITY_NONE;           
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    ret = rt_device_open(serial,   RT_DEVICE_FLAG_INT_RX |RT_DEVICE_FLAG_DMA_TX );
		rt_kprintf("usart open result=%d\n",ret);
    rt_device_set_rx_indicate(serial, rs4851_rx_string);    //设置中断接收回调函数
		rt_device_set_tx_complete(serial,rs4851_tx_complete);    //设置中断发送完成回调函数
    return ret;
}