
#include <modbus.h>
#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include "mdebug.h"
#include "ai_modbus.h"

#define MAX_CLIENT_NUM  3
#define MAX_ERROR_COUNT 2
#define BYTE_TIME_OUT    1*1000
#define CLIENT_TIMEOUT  10      //?? s

#define INPUT_REG_NB    512
#define HOT_REG_NB    512
#define PORT1         701






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
#define MAX_CLIENT_NUM  3
#define CLIENT_TIMEOUT  10      //?? s




static void test_thread_slave(void *param)
{
    int server_fd = -1;
	
	  uint8_t mb_cmd;
	  uint16_t write_addr;
    modbus_t *ctx = NULL;
	  int max_fd = -1;
    fd_set readset;
    int rc;
    struct timeval select_timeout;
	
    modbus_mapping_t *ai_mb_mapping = NULL;
	
    client_session_t client_session[MAX_CLIENT_NUM];
    for (int i = 0; i < MAX_CLIENT_NUM; i++)
    {
        client_session[i].fd = -1;
        client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
    }

    
    select_timeout.tv_sec = 1;
    select_timeout.tv_usec = 0;
    
    ctx = modbus_new_tcp(RT_NULL, 601, AF_INET);
		modbus_set_slave(ctx, 1);
		modbus_set_debug(ctx, 0);
		modbus_set_byte_timeout(ctx, 0, 3000);
		
    RT_ASSERT(ctx != RT_NULL);

	  ai_mb_mapping = modbus_mapping_new(128, 128,4096, 2048);

    RT_ASSERT(ai_mb_mapping != RT_NULL);
		if(ai_mb_mapping != RT_NULL)
		{
			ai_mb_mapping->tab_registers[58] = 36;     //??????
		}
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
                            rc = modbus_reply(ctx, query, rc, ai_mb_mapping,&mb_cmd,&write_addr);
                            if(rc < 0)
                            {
                                close(client_session[i].fd);
                                client_session[i].fd = -1;
                            }
                            else
                            {
															if(mb_cmd>4) //  1-4 ??
																{
																	rt_kprintf("mb ai recive dat cmd=%d\n",mb_cmd);
																
																 if(mb_cmd==16)
																 {
													
																	 uint8_t *addr;
						                       decode_ai_modbus((ai_dat_type *)&ai_mb_mapping->tab_registers[0]);
																 }
																}
													
                                client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                            }
                        }
                        else
                        {
                            close(client_session[i].fd);
                            client_session[i].fd = -1;
                        }
                    }
                }
            }
        }

        // ????????????
        for(int i =0;i<MAX_CLIENT_NUM;i++)
        {
            if(client_session[i].fd >= 0)
            {
                //??
                if((rt_tick_get() - client_session[i].tick_timeout) < (RT_TICK_MAX / 2))
                {
                    close(client_session[i].fd);
                    client_session[i].fd = -1;
                }
            }
        }
    }

_mbtcp_restart:
    if(server_fd >= 0)
    {
        close(server_fd);
        server_fd = -1;
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

static int colour_modbus_init(void)
{
    rt_thread_t  tid1;
	  rt_err_t result;


		tid1 = rt_thread_create("col_mb",
                           test_thread_slave, RT_NULL,
                           2048,
                           12, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		else 
			return RT_ERROR;
    return RT_EOK;
}
INIT_APP_EXPORT(colour_modbus_init);

