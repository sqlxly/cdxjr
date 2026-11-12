#include "modbus_transport.h"
#include <modbus.h>
#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include <mail_box.h>
#include <mdebug.h>

ALIGN(RT_ALIGN_SIZE)
#define MAX_ERROR_COUNT 1   // 

#define print_PORT  601
#define print_SLAVE_ADDR 1
#define print_CH1_IP_ADDR "10.10.0.57" 

#define RESPONSE_TIME_OUT  20*1000  //  20ms  
#define MODBUS_CYCLE_TIME    200  //ms


uint16_t temp_reg[10];

static void print_mast_main(void *param)
{
     int regs;
	   uint8_t mb_error_count=0;
     uint16_t read_buf[64] = {0};
		 uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
		 uint16_t modbus_reconnect_count=0;
		 
     modbus_t *ctx = RT_NULL;
		 packge_mail_box_type *recive_mb;

     rt_thread_mdelay(100);
     ctx = modbus_new_tcp(print_CH1_IP_ADDR,print_PORT, AF_INET);
     modbus_set_slave(ctx, print_SLAVE_ADDR);
		 modbus_set_byte_timeout(ctx, 0, 3000);
     modbus_set_response_timeout(ctx, 0, RESPONSE_TIME_OUT);      //³¬¹ý10MS ³¬Ê±  
 _mbtcp_start:
     if(modbus_connect(ctx) < 0)
		 {
				 rt_kprintf("print modbus ch1  connect fall\n");
         goto _mbtcp_restart;
		 }
		 else 
		 {
				 DEBUG_CARE("print modbus ch1  connect scuess\n");
		 }
     int num = 0;
     while (1)
     {
			 if (rt_mq_recv(&masseg_print, &mesg_buf, sizeof(MESSAGE_DAT_LEN), MODBUS_CYCLE_TIME) == RT_EOK)
			 {
				  regs = modbus_write_registers(ctx, 0,10,&mesg_buf[0]);
				  if(regs < 0)
					{
						DEBUG_ERR("print send reg err\n");
					
						mb_error_count++;
					}
					else 
					{
						mb_error_count=0;
					}
			 }
			 else 
			 {
				 memset(read_buf, 0, sizeof(read_buf));
				 regs = modbus_read_input_registers(ctx, 0,10, (uint16_t *)read_buf);
				 if(regs > 0)
				 {
					 
				 }
					if(regs < 0)
					{
						mb_error_count++;
					}
					else 
					{
						mb_error_count=0;
					}
				}
			 	if(mb_error_count>MAX_ERROR_COUNT)
				{
						modbus_reconnect_count++;
						//DEBUG_ERR("mb1 ch1 recive fall !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
						goto _mbtcp_restart;
				}
        //rt_thread_mdelay(10);
     }
 _mbtcp_restart:
     modbus_close(ctx);
     rt_thread_mdelay(2000);
     goto _mbtcp_start;
     modbus_free(ctx);
 }

 static int print_mast_init(void)
{
    rt_thread_t tid1;
	  rt_err_t result;

    tid1 = rt_thread_create("MB_p",print_mast_main, RT_NULL,2048,20, 10);

	 
		if (tid1 != RT_NULL)
		{
			rt_thread_startup(tid1);
		}
		else 
		{
			return RT_ERROR;
		}
    return RT_EOK;
}
INIT_APP_EXPORT(print_mast_init);

