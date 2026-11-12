#include "modbus_screen.h"
#include <modbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include <screen_chat.h>
#include <user_set.h>
#include "mdebug.h"
#define MAX_CLIENT_NUM  4

#define CLIENT_TIMEOUT  10      //单位 s

ALIGN(RT_ALIGN_SIZE)
modbus_mapping_t *mb_mapping = NULL;

extern void disp_mp3_file();




uint16_t get_screen_set_val_type16(uint16_t addr)
{

	return mb_mapping->tab_registers[addr];

}
uint32_t get_screen_set_val_type32(uint16_t addr)
{
  uint32_t val;
   
  val = (mb_mapping->tab_registers[addr]) + (mb_mapping->tab_registers[addr+1]<<16);
	return  val;
}
extern void init_screen_mb_dat();
static int mb_dat_init()
{
	mb_mapping = modbus_mapping_new(128, 128,8300, 1024);
	init_screen_mb_dat();
}


static void test_thread_slave(void *param)
{
    int server_fd = -1;
	
	  uint8_t mb_cmd;
	  uint16_t write_addr;
	  uint32_t mail_box_dat;
    modbus_t *ctx = NULL;
    
    client_session_t client_session[MAX_CLIENT_NUM];

    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        client_session[i].fd = -1;
        client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
    }

    int max_fd = -1;
    fd_set readset;
    int rc;
    struct timeval select_timeout;
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;
    
    ctx = modbus_new_tcp(RT_NULL, 502, AF_INET);
		modbus_set_slave(ctx, 1);
		modbus_set_debug(ctx, 0);
		modbus_set_byte_timeout(ctx, 0, 3000);
		
    RT_ASSERT(ctx != RT_NULL);
    
    RT_ASSERT(mb_mapping != RT_NULL);
		if(mb_mapping != RT_NULL)
		{
			mb_mapping->tab_registers[59] = 0;     //切换到主界面
		}
		//disp_mp3_file();
_mbtcp_start:
    server_fd = modbus_tcp_listen(ctx, 3);
    if (server_fd < 0)
        goto _mbtcp_restart;
    while (1)
    {
			
	
			 
        max_fd = -1;
        FD_ZERO(&readset);
        FD_SET(server_fd, &readset);

        if(max_fd < server_fd)
            max_fd = server_fd;
        
        for (int i = 0; i < MAX_CLIENT_NUM; i++)
        {
            if(client_session[i].fd >= 0)
            {
                FD_SET(client_session[i].fd, &readset);
                if(max_fd < client_session[i].fd)
                    max_fd = client_session[i].fd;
            }
        }
        rc = select(max_fd + 1, &readset, RT_NULL, RT_NULL, &select_timeout);
        if(rc < 0)
        {
            goto _mbtcp_restart;
        }
        else if(rc > 0)
        {
            if(FD_ISSET(server_fd, &readset))
            {
                int client_sock_fd = modbus_tcp_accept(ctx, &server_fd);
                if(client_sock_fd >= 0)
                {
                    int index = -1;
                    for (int i = 0; i < MAX_CLIENT_NUM; i++)
                    {
                        if(client_session[i].fd < 0)
                        {
                            index = i;
                            break;
                        }
                    }
                    if(index >= 0)
                    {
                        client_session[index].fd = client_sock_fd;
                        client_session[index].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                    }
                    else
                    {
                        close(client_sock_fd);
												DEBUG_ERR("screen connect time out close fd\n");
                    }
                }
            }
            for (int i = 0; i < MAX_CLIENT_NUM; i++)
            {
                if(client_session[i].fd >= 0)
                {
                    if(FD_ISSET(client_session[i].fd, &readset))
                    {
                        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
                        modbus_set_socket(ctx, client_session[i].fd);

                        rc = modbus_receive(ctx, query);
                        if (rc > 0)
                        {
													  mb_cmd =0;
													  write_addr =0;
                            rc = modbus_reply(ctx, query, rc, mb_mapping,&mb_cmd,&write_addr);
                            if(rc < 0)
                            {
                                close(client_session[i].fd);
                                client_session[i].fd = -1;
																DEBUG_ERR("screen connect time out close fd\n");
                            }
                            else
                            {
															if(mb_cmd>4) //  1-4 为读
																{
																		dispose_screen_dat(mb_mapping->tab_registers,write_addr,mb_cmd);
																    rt_thread_mdelay(10);
																}
																if(0)
																{
																	 mail_box_dat = (mb_cmd<<16) + write_addr;
																}
                                client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                            }
                        }
                        else
                        {
                            close(client_session[i].fd);
                            client_session[i].fd = -1;
														DEBUG_ERR("screen connect time out close fd\n");
                        }
                    }
                }
            }
        }

        // 客户端超时未收到数据断开
        for(int i =0;i<MAX_CLIENT_NUM;i++)
        {
            if(client_session[i].fd >= 0)
            {
                //超时
                if((rt_tick_get() - client_session[i].tick_timeout) < (RT_TICK_MAX / 2))
                {
                    close(client_session[i].fd);
                    client_session[i].fd = -1;
										DEBUG_ERR("screen connect time out close fd\n");
                }
            }
        }
    }

_mbtcp_restart:
    if(server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
		DEBUG_ERR("screen connect time out close fd\n");
    }

    for(int i =0;i<MAX_CLIENT_NUM;i++)
    {
        if(client_session[i].fd >= 0)
        {
            close(client_session[i].fd);
            client_session[i].fd = -1;
        }
    }

    rt_thread_mdelay(5000);
    goto _mbtcp_start;

    modbus_free(ctx);
}

static int modbus_screen_init(void)
{
    rt_thread_t  tid1;
	  rt_err_t result;


		tid1 = rt_thread_create("mbscreen",
                           test_thread_slave, RT_NULL,
                           4096,
                           25, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		else 
			return RT_ERROR;
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(mb_dat_init);
INIT_APP_EXPORT(modbus_screen_init);
void writebuf(int argc, char *argv[])
{
	int addr;
	int val;
	if(argc==3)
	{
		addr =atoi(argv[1]);
		rt_memcpy(&mb_mapping->tab_registers[addr],argv[2],strlen(argv[2]));
	}
	else if(argc==4)
	{
		addr =atoi(argv[1]);
		val = atoi(argv[3]);
	  mb_mapping->tab_registers[addr]= val;
	}
	else 
	{
		rt_kprintf("arg is empty\n");
	}
}

MSH_CMD_EXPORT(writebuf,libmodbus wite buf);