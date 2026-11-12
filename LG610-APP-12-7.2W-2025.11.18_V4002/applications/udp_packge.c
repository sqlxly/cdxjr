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
#include "udp_weight.h"
#include "modbus_packge.h"

#define BUFSZ   2048



#define LISTEN_PORT 10000


#define READ_BUF_MAX_DAT_LED   2048
ALIGN(RT_ALIGN_SIZE)



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
}packge_control_dat;



const char  CLIENT_IP[6][11]={{"10.10.0.60"},{"10.10.0.61"},{"10.10.0.62"},{"10.10.0.63"},{"10.10.0.64"},{"10.10.0.65"}};

static uint8_t udp_read_buf[6][READ_BUF_MAX_DAT_LED] = {{0},{0},{0},{0},{0},{0}};
	




static void udpserver_send(int socket, struct sockaddr *remote_addr,uint8_t *buf,uint16_t len)
{
			sendto(socket, buf, len, 0,remote_addr, sizeof(struct sockaddr));
      rt_thread_delay(1);
}


static void replay_commond(int socket, struct sockaddr *remote_addr,uint16_t msg_id,uint8_t result)
{
	 uint32_t crc;
	 replay_mesg_info  replay_mesg;
	 
	 rt_memset(&replay_mesg,0,sizeof(replay_mesg_info));
	 replay_mesg.head = UDP_MESSAGE_HEAD;
	 replay_mesg.msg_len = sizeof(replay_mesg_info)-8;  //È¥µô Í·ºÐCRC32
	 replay_mesg.mesg_id = msg_id;
   replay_mesg.result =result;
	 crc = CRC32_file((uint8_t *)&replay_mesg,sizeof(replay_mesg_info)-4);
	 replay_mesg.CRC32 = crc;
	 udpserver_send(socket,remote_addr,(uint8_t*)&replay_mesg,sizeof(replay_mesg_info));
}

void analysis_packge_conrol_dat(int socket, struct sockaddr *remote_addr,packge_control_dat *msg,uint8_t id)
{
   uint32_t crc,crc1;
   if(msg->head !=UDP_MESSAGE_HEAD)
	 {
			rt_kprintf("Í·´íÎó\n");
		  return ;
	 }
	  crc = CRC32_file((uint8_t *)msg,sizeof(packge_control_dat)-4);
	  if(crc == msg->CRC32)
		{
			replay_commond(socket,remote_addr,msg->mesg_id,0);
			analysis_packge_control_info(&msg->control_dat,id);
			//DEBUG_INFO("udp recive replay id=%d mesg_id=%d empty_id=%d\n",id,msg->mesg_id,msg->control_dat.ch1.empty_id);
		}
		else 
		{
			DEBUG_ERR("revive  replay info crc err\n");
		}
}

//static int sock = -1;
//static struct sockaddr_in local_addr;
//static struct sockaddr_in remote_addr;



static void udpserver_recv(void *parameter)
{
	  uint8_t device_id;
	  int sock = -1;
    struct sockaddr_in local_addr;
    struct sockaddr_in remote_addr;
		int bytes_read;
		fd_set readset;
		socklen_t addr_len;
		struct timeval timeout;
		struct sockaddr_in socket_addr;
   
	  device_id = *(uint8_t*)parameter;
	  if(device_id>5) return ;
	  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        rt_kprintf("Socket error\n");

        return ;
    }
    rt_thread_mdelay(2000);
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LISTEN_PORT+device_id);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));

		remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(LISTEN_PORT+device_id);
    remote_addr.sin_addr.s_addr = inet_addr(&CLIENT_IP[device_id][0]);
    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
		
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Bind error\n");
        return ;
    }
		
		
		addr_len = sizeof(struct sockaddr);
    timeout.tv_sec = 1;
		timeout.tv_usec = 0;
    while (1)
    {

				FD_ZERO(&readset);
        FD_SET(sock, &readset);
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) != 0)
        {
					 rt_memset(&udp_read_buf[device_id][0],0,READ_BUF_MAX_DAT_LED);
           bytes_read = recvfrom(sock, &udp_read_buf[device_id][0], READ_BUF_MAX_DAT_LED - 1, 0, (struct sockaddr *)&socket_addr, &addr_len);
					 udp_read_buf[device_id][bytes_read] = '\0'; 
			   //rt_kprintf("recive socket addr=%s\n",socket_addr.sin_addr);
					// rt_memset(&packge_control_part,0,sizeof(packge_control_dat));
				//	 rt_memcpy(&packge_control_part,recv_data,sizeof(packge_control_dat));
					 analysis_packge_conrol_dat(sock,(struct sockaddr *)&remote_addr,(packge_control_dat *)&udp_read_buf[device_id][0],device_id);
					// rt_kprintf("udp id=%d \n", device_id);
        }
				else 
				{	
					//rt_kprintf("time out\n");
				}
				rt_thread_mdelay(5);
    }
}

 static const uint8_t packge_device_id[6]={0,1,2,3,4,5};
 static int packge_udp_init(void)
{
     rt_thread_t tid1=RT_NULL;
     rt_thread_t tid2=RT_NULL;
	  rt_thread_t tid3=RT_NULL;
	rt_thread_t tid4=RT_NULL;
	rt_thread_t tid5=RT_NULL;
	 rt_thread_t tid6=RT_NULL;
	  rt_err_t result;
	  
      tid1 = rt_thread_create("udpP1",udpserver_recv, (void*)&packge_device_id[0],2048,14, 10);
	  tid2 = rt_thread_create("udPP2",udpserver_recv, (void*)&packge_device_id[1],2048,13, 10);
	  tid3 = rt_thread_create("udpP3",udpserver_recv, (void*)&packge_device_id[2],2048,12, 10);
	  tid4 = rt_thread_create("udpP4",udpserver_recv, (void*)&packge_device_id[3],2048,12, 10);
	  tid5 = rt_thread_create("udpP5",udpserver_recv, (void*)&packge_device_id[4],2048,12, 10);
	  tid6 = rt_thread_create("udpP6",udpserver_recv, (void*)&packge_device_id[5],2048,12, 10);
	  
	  if ((tid1 != RT_NULL)&&(tid2 != RT_NULL)&&(tid3 != RT_NULL)&&(tid4 != RT_NULL)&&(tid5 != RT_NULL)&&(tid6 != RT_NULL))
		{
			rt_thread_startup(tid1);
			rt_thread_startup(tid2);
			rt_thread_startup(tid3);
			rt_thread_startup(tid4);
			rt_thread_startup(tid5);
			rt_thread_startup(tid6);
			result = RT_EOK;
		}
		else 
		{
		  result = RT_ERROR;
		}
		return result;
}
INIT_APP_EXPORT(packge_udp_init);


