#include <rtthread.h>
#include <sys/socket.h> 
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "crc32.h"
#include <mail_box.h>
#include "mdebug.h"
#include "servo_common.h"
#include "servo_control.h"
#include "recode_dat.h"

#define BUFSZ   2048

#define CLIENT_IP "10.10.0.58"
#define LISTEN_PORT 10000
#define BUFSZS   2048
ALIGN(RT_ALIGN_SIZE)

#define UDP_MESSAGE_HEAD 0x33445566

static uint8_t udp_restart_flag=0;											// 2026.04.08 qianxuping

typedef struct 
{
uint32_t head;
uint16_t msg_len;
uint16_t mesg_id;
uint8_t  result;
uint32_t CRC32; 
}replay_mesg_info;

typedef struct 
{
	uint32_t head;
  packge_control_type control_dat;
	uint32_t mesg_id;
  uint32_t CRC32; 
}send_mesg_info;


static send_mesg_info send_mesg;
static rt_uint8_t msg_recive[MESSAGE_DAT_LEN];
static struct sockaddr_in local_addr;
static struct sockaddr_in remote_addr;
static uint8_t recv_data[BUFSZS];
uint32_t recive_respone_id=0x1234;


static int sock = -1;

static const char send_datas[] = "UDP server\n";


static void udpserver_send( uint8_t *buf,uint16_t len)
{
			sendto(sock, buf, len, 0,(struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
      rt_thread_delay(2);
	//    DEBUG_INFO("udp sync state\n");
}

void analysis_commond(replay_mesg_info *msg)
{
   uint32_t crc,crc1;
   if(msg->head !=UDP_MESSAGE_HEAD)
	 {
			rt_kprintf("msg head error\n");
		  return ;
	 }
	  crc = CALCRC32_file((uint8_t *)msg,sizeof(replay_mesg_info)-4);
	  if(crc == msg->CRC32)
		{
			recive_respone_id = msg->mesg_id;
		//	DEBUG_INFO("recive replay id=%d\n",msg->mesg_id);
		}
		else 
		{
			DEBUG_ERR("revive  replay info crc err\n");
		}
	  
}


static uint8_t mb_dat[MESSAGE_DAT_LEN];

void sync_control_state(void)
{
	rt_err_t result;	
  mb_dat[0]=1;	
	result= rt_mq_send(&send_control_state_cmd, mb_dat, MESSAGE_DAT_LEN);
	if(result != RT_EOK)
	{
		DEBUG_ERR("send mesg err\n");
	}
}


#include <dfs_posix.h>									// 2026.04.08 qianxuping
#include <sys/time.h>										// 2026.04.08 qianxuping
#include <dfs_select.h>									// 2026.04.08 qianxuping		
#include <sal_socket.h>									// 2026.04.08 qianxuping
// 2026.04.08 qianxuping
void udp_close_fd(void)
{
		rt_kprintf("udp_close_fd\n");
		close(sock);
}

// 2026.04.08 qianxuping
int udp_create_fd(void)
{
		uint8_t board_addr;
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        rt_kprintf("udp_create_fd Socket error\n");

        return -RT_ERROR;
    }
		
		board_addr = read_board_addr();
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LISTEN_PORT+board_addr);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
    
		remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons((LISTEN_PORT+board_addr));
    remote_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
		
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("udp_create_fd Bind error\n");
        return -RT_ERROR;
    }
		
		return RT_EOK;
}

// 2026.04.08 qianxuping
void udp_close_fd_flag(void)
{
		udp_restart_flag=1;
}

#include"drv_eth.h"													// 2026.04.08 qianxuping
extern void modbus_tcp_close_fd(void);			// 2026.04.08 qianxuping
extern void modbus_tcp_close_fd_flag(void); // 2026.04.08 qianxuping
void send_control_state(uint32_t id)
{
    uint32_t crc;
	  uint8_t i=0;
	  uint8_t j;
		static uint32_t should_reconnect=0;	// 2026.04.08 qianxuping
	
	 rt_tick_t time=0;
	
	 time = rt_tick_get();
	 for(i=0;i<3;i++)
	 {
		send_mesg.head = UDP_MESSAGE_HEAD;
		send_mesg.mesg_id = id;
		get_servo_run_state(&send_mesg.control_dat);
		crc = CALCRC32_file((uint8_t *)&send_mesg,sizeof(send_mesg_info)-4);
		send_mesg.CRC32 = crc;
		udpserver_send((uint8_t *)&send_mesg,sizeof(send_mesg_info));
		 
		for(j=0;j<5;j++)
		{
			rt_thread_mdelay(1); 
		  if(recive_respone_id ==id)
			{
			  break;
			}
		}
		if(recive_respone_id ==id)
		{
		  break;
		}
	 }
	 
	 // 发给主控后，等待主控回复
	 // 如果没有回复则说明网络有问题
	 if(recive_respone_id !=id)
	 {
		  rt_kprintf("udp response err i=%d\n",i);
			should_reconnect++;
			if(should_reconnect>6)			// 2026.04.08 qianxuping 这个是为了消除开机的时候eth没有起来，这里开始发送，会发送不过
			{
					#if 1										// 2026.04.08 qianxuping link状态直接强制link up
					rt_kprintf("ifconfig!!!!!!!!!!!!\n");
					system("ifconfig");
					rt_kprintf("ps!!!!!!!!!!!!!!!!!!\n");
					system("ps");
					rt_kprintf("********************\n");
						#if 0
						my_eth_linkup();				// 2026.04.08 qianxuping
						#else
						my_eth_phyreset();			// 2026.04.10 qianxuping
						#endif
					#else										// 2026.04.08 qianxuping 先关闭应用层的socket，然后重新初始化
					//modbus_tcp_close_fd();
					//udp_close_fd();
					//my_eth_stop_reset();
					//udp_create_fd();
				
					//modbus_tcp_close_fd_flag();
					//udp_close_fd();
					//rt_base_t level;
					//level = rt_hw_interrupt_disable();
					//my_eth_stop_reset();
					//rt_hw_interrupt_enable(level);
					//udp_create_fd();
					//udp_restart_flag=1;
					#endif
			}
	 }
	 else	// 2026.04.10 qianxuping
	 {
			should_reconnect=0;
	 }
//	 rt_kprintf("use time=%d ch1 eggs=%d ch2 eggs =%d\n",rt_tick_get()-time,send_mesg.control_dat.ch1.cup_eggs,send_mesg.control_dat.ch2.cup_eggs);
}
static void udp_mast_run(void *parameter)
{
	uint16_t time_count=0;
	uint32_t id=0;


  while(1)
	{
		
   	if (rt_mq_recv(&send_control_state_cmd, &msg_recive, sizeof(msg_recive), 10) == RT_EOK)
    {
			time_count =0;
			id++;
		  send_control_state(id);
    }
		else 
		{
		  time_count++;
			if(time_count>100)
			{
				time_count =0;
				id++;
		    send_control_state(id);
			}
		}
		//udpserver_send((uint8_t *)&control_state,sizeof(packge_control_type));
	}
}

static void udpserver_recv(void *parameter)
{
		int bytes_read;
		fd_set readset;
		socklen_t addr_len;
		struct timeval timeout;
		struct sockaddr_in socket_addr;
		
		addr_len = sizeof(struct sockaddr);
    timeout.tv_sec = 1;
		timeout.tv_usec = 0;
    while (1)
    {
				// 2026.04.08 qianxuping
//				if(udp_restart_flag)
//				{
//						udp_restart_flag=0;
//						udp_close_fd();
//						udp_create_fd();
//				}
				
				FD_ZERO(&readset);
        FD_SET(sock, &readset);
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) != 0)
        {
					rt_memset(recv_data,0,BUFSZS);
           bytes_read = recvfrom(sock, recv_data, BUFSZS - 1, 0, (struct sockaddr *)&socket_addr, &addr_len);
					 recv_data[bytes_read] = '\0'; 
					 #if 0
					 analysis_commond((replay_mesg_info *)recv_data);
					 #else	// 2026.04.08 qianxuping
					 if(bytes_read != -1)
					 {
							//rt_kprintf("bytes_read=%d\n",bytes_read);
							analysis_commond((replay_mesg_info *)recv_data);
					 }
					 #endif
					 //rt_kprintf("%s", recv_data);
        }
				else 
				{	
					//rt_kprintf("time out\n");
				}
    }
}

static int packge_udp_comm(void)
{
	  uint8_t board_addr;
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        rt_kprintf("Socket error\n");

        return -RT_ERROR;
    }
		
		board_addr = read_board_addr();
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LISTEN_PORT+board_addr);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));
    
		remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons((LISTEN_PORT+board_addr));
    remote_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
		
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Bind error\n");
        return -RT_ERROR;
    }
		
    DEBUG_INFO("PACKGE UDP IS RUN  on port 10000...\n");
		
		rt_thread_t  thread1;
		
		thread1 = rt_thread_create("udpRcv", udpserver_recv, RT_NULL, 2048, 21, 10);
		if (thread1 != RT_NULL)
		{
				rt_thread_startup(thread1);
		}
		else
		{
				rt_kprintf("create udpRcv thread failed!\n");
				return -RT_ERROR;
		}
		
		
		rt_thread_t  thread2;
		
		thread2 = rt_thread_create("udpRcv", udp_mast_run, RT_NULL, 2048, 21, 10);
		if (thread2 != RT_NULL)
		{
				rt_thread_startup(thread2);
		}
		else
		{
				rt_kprintf("create udpRcv thread failed!\n");
				return -RT_ERROR;
		}
		
		return RT_EOK;
}

INIT_APP_EXPORT(packge_udp_comm);
