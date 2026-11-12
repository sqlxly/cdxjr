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
#include "rs485.h"


#define SAMPLE_UART_NAME  "uart2"    
#define RS4852_RE_PIN     GET_PIN(D, 4)


static rt_tick_t send_sucess_time=0;
static rt_device_t serial;
static rs485_type rs4852; 
static rt_mutex_t rs4852_mutex = RT_NULL;



static rt_err_t rs4852_tx_complete(rt_device_t dev, void *buffer)
{
	  send_sucess_time= rt_tick_get();
	  rt_pin_write(RS4852_RE_PIN, PIN_LOW);
    return RT_EOK;
}
// DMX 接受中断
static rt_err_t rs4852_rx_string(rt_device_t dev, rt_size_t size)
{
//	  rt_base_t level;
    rt_size_t rx_length;
//	  level = rt_hw_interrupt_disable();
	  if((UART_RX_BUFSIZE -rs4852.rx_len)<size)
		{
			rs4852.rx_len=0;     //缓存溢出
		}
		rx_length = rt_device_read(serial, -1,&rs4852.dat[rs4852.rx_len], size);
		rs4852.rx_len += rx_length;
		rs4852.tick = rt_tick_get();
//		rt_hw_interrupt_enable(level);
    return 0;
}

void rs4852_clear_buf()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_memset(&rs4852,0,sizeof(rs4852));	
	rt_hw_interrupt_enable(level);
}

//警告  如果有开销很大的线程 休眠延时不准确  一定要切换延时方式
uint16_t rs4852_read_dat(uint8_t *buf,uint16_t len ,uint8_t time_out)
{
    uint16_t rx_ren=0;
	  rt_base_t level;
    rt_tick_t ctick;
	  uint8_t count=0;
	  while(time_out--)
		{
			ctick = rt_tick_get();
			count++;
			if(((ctick>(rs4852.tick+PER_FARME_TIME))||(ctick<rs4852.tick))&&(rs4852.rx_len>0))   //有数据 + 断帧			2ms 为一帧
			{
				
				level = rt_hw_interrupt_disable();
				rx_ren = rs4852.rx_len;
				if(len>rs4852.rx_len)
				{
					rt_memcpy(buf,&rs4852.dat[0],rs4852.rx_len);
				}
				else 
				{
					rt_memcpy(buf,&rs4852.dat[0],len);
				}
				rt_memset(&rs4852,0,sizeof(rs4852));	
				rt_hw_interrupt_enable(level);
				//rt_kprintf("time_out=%d\n",count);
				return rx_ren;
			}
			rt_thread_mdelay(1);
		}
		return 0;   //超时
}

rt_err_t rs4852_send_dat(uint8_t *str,int len)
{
	rt_err_t ret = RT_EOK;
  rt_tick_t ctime;
	ctime = rt_tick_get();
	if((ctime - send_sucess_time)<3)
	{
		rt_thread_mdelay(3);
	}		
	if(rt_mutex_take(rs4852_mutex, 10)!=RT_EOK)                //上锁 
		return RT_ERROR;		
  rt_pin_write(RS4852_RE_PIN, PIN_HIGH);
	ret = rt_device_write(serial, 0, str, len);
	rt_mutex_release(rs4852_mutex);        //释放锁
	return ret;
}


// DMA串口初始化
int RS4852_init(void)
{
    rt_err_t ret = RT_EOK;

    rt_pin_mode(RS4852_RE_PIN, PIN_MODE_OUTPUT);
	  rt_memset(&rs4852,0,sizeof(rs4852));
	
	  rs4852_mutex = rt_mutex_create("4852_lock", RT_IPC_FLAG_PRIO);
    if (rs4852_mutex == RT_NULL)
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
		config.stop_bits = STOP_BITS_2;           
		config.bufsz     = 256;                   
		config.parity    = PARITY_NONE;           
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    ret = rt_device_open(serial,   RT_DEVICE_FLAG_INT_RX |RT_DEVICE_FLAG_DMA_TX );
		rt_kprintf("usart open result=%d\n",ret);
    rt_device_set_rx_indicate(serial, rs4852_rx_string);    //设置中断接收回调函数
		rt_device_set_tx_complete(serial,rs4852_tx_complete);    //设置中断发送完成回调函数
    return ret;
}

INIT_ENV_EXPORT(RS4852_init);