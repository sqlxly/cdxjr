
#include <rtthread.h>
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
#include "netdb.h"
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>

#define BUFSZ   1024

uint8_t recive_buf[BUFSZ];

#define UDP_SERVER_PORT 8000
#define UDP_CLIENT_PORT 8000

#define SERVER_url  "10.10.0.159"
#define CLIENT_url  "10.10.0.60"

static  int server_fd=-1;
static  int clent_fd=-1;

static struct sockaddr_in server_addr, client_addr;

static socklen_t addr_len;





const char send_data[] = "This is UDP Client from RT-Thread.\n"; /* 发送用到的数据 */


void udpclient()
{
    int  port, count;
    struct hostent *host;
    struct sockaddr_in server_addr;

    port = UDP_CLIENT_PORT;
    host = (struct hostent *) gethostbyname(CLIENT_url);
    if ((clent_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("Socket error\n");
        return;
    }

    /* 初始化预连接的服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

    /* 总计发送count次数据 */
    while (count)
    {
        /* 发送数据到服务远端 */
        sendto(clent_fd, send_data, strlen(send_data), 0,
               (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

        /* 线程休眠一段时间 */
        rt_thread_delay(500);

        /* 计数值减一 */
        count --;
    }

    /* 关闭这个socket */
    closesocket(UDP_CLIENT_PORT);
}


static void rebuld_server()
{
    
	 
	  if(server_fd != -1)
		{
		  closesocket(server_fd);
		}
      /* 创建一个socket，类型是SOCK_DGRAM，UDP类型 */
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("creat Socket error\n");
        return;
    }

   /* 初始化服务端地址 */
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
		
		 /* 绑定socket到服务端地址 */
    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        /* 绑定地址失败 */
        rt_kprintf("Bind error\n");
        return;
    }
		addr_len = sizeof(struct sockaddr);
		rt_kprintf("UDPServer Waiting for client on port 8000...\n");
}


void udp_server_main()
{
  int bytes_read;
	int i,count=0;
	struct timeval tv;
	fd_set readfds;
	
	
  rebuld_server(); 
  
	while(1)
	{
		
		for(i=0;i<3;i++)
		{
		  if(server_fd !=-1)
			{
	      memset(recive_buf,0,BUFSZ);
				FD_ZERO(&readfds);
				FD_SET(server_fd,&readfds);
				tv.tv_sec=1;
				tv.tv_usec =0;
				select(server_fd+1,&readfds,NULL,NULL,&tv);
				if(FD_ISSET(server_fd,&readfds))
				{
	        count =0;
						bytes_read = recvfrom(server_fd, recive_buf, BUFSZ - 1, 0,
                              (struct sockaddr *)&client_addr, &addr_len);
						recive_buf[bytes_read] = '\0'; /* 把末端清零 */
						rt_kprintf("\n(%s , %d) said : ", inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
						rt_kprintf("%s", recive_buf);
					  udpclient();
					}
				}
			else 
			{
			  rt_thread_mdelay(1000);
			}
			 count++;
			if(count>10)
			{
				rt_kprintf("重新建立监听\n");
			  rebuld_server();
			}
			}
		rt_kprintf("count=%d\n",count);
	}


   return;

}
 static int udp_connect()
{
	rt_thread_t tid1;
  rt_err_t result;

  tid1 = rt_thread_create("udps",udp_server_main, RT_NULL,2048,15, 10);

	 
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

//INIT_APP_EXPORT(udp_connect);
