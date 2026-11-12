#include "agile_modbus.h"
//#include "serial.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <rtdevice.h>

#include <rtthread.h>
#include <rtdevice.h>



#define RT_DEVICE_FLAG_RX_BLOCKING        0x1000   
#define RT_DEVICE_FLAG_RX_NON_BLOCKING    0x2000   
#define RT_DEVICE_FLAG_TX_BLOCKING        0x4000   
#define RT_DEVICE_FLAG_TX_NON_BLOCKING    0x8000   
#define RT_DEVICE_FLAG_STREAM             0x040     
#define SAMPLE_UART_NAME       "uart3"    
#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME "rtu_master"
#define DBG_LEVEL        DBG_LOG
//#include "dbg_log.h"
#define BSP_UART1_RX_BUFSIZE 1024
static int _fd = -1;
//static struct termios _old_tios = {0};
struct rx_msg
{
    rt_device_t dev;
    rt_size_t size;
};
struct mb_recive
{
   uint8_t rx_buffer[BSP_UART1_RX_BUFSIZE + 1];
   uint32_t rx_length;
};

static rt_device_t serial;
static struct mb_recive mb_rx_dat;
static struct rt_messagequeue rx_mq;





#define UART4_RE 44

void my_modbus_send(uint8_t *str,int len)
{
	
  rt_pin_write(UART4_RE, PIN_HIGH);
	rt_thread_mdelay(10);
	rt_device_write(serial, 0, str, len);
}
int my_modbus_recive(uint8_t *red_bud,int len)
{
  uint16_t i=0;
	
	rt_thread_mdelay(100);  
	rt_kprintf("len=%d\n",mb_rx_dat.rx_length);
	for(i=0;i<mb_rx_dat.rx_length;i++)
	{
		rt_memcpy(red_bud,&mb_rx_dat.rx_buffer[0],mb_rx_dat.rx_length);
	  rt_kprintf("0x%02x ",mb_rx_dat.rx_buffer[i]);
	}
	rt_kprintf("\n");
	
	
  i = mb_rx_dat.rx_length;
  mb_rx_dat.rx_length=0;
	return i;
}
static void cycle_entry(void *param)
{
	  int read_len;
    uint8_t ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint8_t ctx_read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
	  uint16_t slave_addr=1;
    uint16_t hold_register[10];

    agile_modbus_rtu_t ctx_rtu;
    agile_modbus_t *ctx = &ctx_rtu._ctx;
    agile_modbus_rtu_init(&ctx_rtu, ctx_send_buf, sizeof(ctx_send_buf), ctx_read_buf, sizeof(ctx_read_buf));

    while (1) {
        
			  agile_modbus_set_slave(ctx, slave_addr);
			  slave_addr++;
				if(slave_addr>7) slave_addr =1;
			  
        rt_thread_mdelay(1000); 
        int send_len = agile_modbus_serialize_read_registers(ctx, 0, 10);
			  my_modbus_send(ctx->send_buf, send_len);
        read_len = my_modbus_recive(ctx->read_buf, ctx->read_bufsz);
        
        if (read_len < 0) {
            rt_kprintf("Receive error, now exit.");
            break;
        }

        if (read_len == 0) {
            rt_kprintf("Receive timeout.");
            continue;
        }

        int rc = agile_modbus_deserialize_read_registers(ctx, read_len, hold_register);
        if (rc < 0) {
            rt_kprintf("Receive failed.");
            if (rc != -1)
                rt_kprintf("Error code:%d", -128 - rc);

            continue;
        }

        rt_kprintf("Hold Registers addr=%d:",slave_addr-1);
        for (int i = 0; i < 10; i++)
            rt_kprintf("\nreg[%d]: 0x%04X", i, hold_register[i]);

        printf("\r\n\r\n\r\n");
				
    }

}







// DMX 发送完成后 RE 拉低
static rt_err_t tx_complete(rt_device_t dev, void *buffer)
{
	  rt_pin_write(UART4_RE, PIN_LOW);
    return RT_EOK;
}
// DMX 接受中断
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    struct rx_msg msg;
    rt_err_t result;
    msg.dev = dev;
    msg.size = size;
    
    result = rt_mq_send(&rx_mq, &msg, sizeof(msg));
    if (result == -RT_EFULL)
    {
        rt_kprintf("message queue full!\n");
    }
    return result;
}

// 串口接收函数 等待DMA中断触发 后接收
static void serial_thread_entry(void *parameter)
{  
	  static uint16_t count=0;
    struct rx_msg msg;
    rt_err_t result;
    uint16_t i;
    while (1)
    {
        rt_memset(&msg, 0, sizeof(msg));
        result = rt_mq_recv(&rx_mq, &msg, sizeof(msg), 5);
        if (result == RT_EOK)
        {
					  if((mb_rx_dat.rx_length +msg.size) >=  BSP_UART1_RX_BUFSIZE)
						{
							mb_rx_dat.rx_length = 0;
						}
            mb_rx_dat.rx_length += rt_device_read(msg.dev, 0,&mb_rx_dat.rx_buffer[mb_rx_dat.rx_length], msg.size);
        }
				else 
				{
					count++;
					if(count>10) count =10;
				}
    }
}
// DMA串口初始化
int uart_dma_sample()
{
    rt_err_t ret = RT_EOK;
    rt_thread_t thread,thread1;
    static char msg_pool[256];
	  static char mb_pool[256];
	  
	  rt_memset(&mb_rx_dat,0,sizeof(mb_rx_dat));
	
    rt_pin_mode(UART4_RE, PIN_MODE_OUTPUT);
    serial = rt_device_find(SAMPLE_UART_NAME);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", SAMPLE_UART_NAME);
        return RT_ERROR;
    }
    rt_mq_init(&rx_mq, "rx_mq",msg_pool,sizeof(struct rx_msg),sizeof(msg_pool), RT_IPC_FLAG_FIFO);    
		struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = 9600;
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    rt_device_open(serial, RT_DEVICE_FLAG_RX_NON_BLOCKING | RT_DEVICE_FLAG_TX_BLOCKING | RT_DEVICE_FLAG_DMA_RX | RT_DEVICE_FLAG_DMA_TX);
    rt_device_set_rx_indicate(serial, uart_input);    //设置中断接收回调函数
		rt_device_set_tx_complete(serial,tx_complete);    //设置中断发送完成回调函数
    thread = rt_thread_create("serial", serial_thread_entry, RT_NULL, 1024, 25, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
		
		thread1 = rt_thread_create("my_modbus", cycle_entry, RT_NULL, 1024, 25, 10);
    if (thread1 != RT_NULL)
    {
        rt_thread_startup(thread1);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}

