#include <rtthread.h>
#include <sys/socket.h> 
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include "crc32.h"
#include <mail_box.h>
#include "e6b2.h"
#include "mdebug.h"
#include "udp_weight.h"
#include "device_alarm.h"
#define BUFSZ   2048

#define CLIENT_IP "10.10.0.41"
#define LISTEN_PORT 5000
#define BUFSZS   2048
ALIGN(RT_ALIGN_SIZE)

#define UDP_READ_MESSAGE_HEAD 0x11223344
typedef struct 
{
uint32_t head;
uint16_t msg_len;
uint16_t mesg_id;
uint8_t  result;
uint32_t CRC32; 
}replay_mesg_info;



static struct sockaddr_in local_addr;
static struct sockaddr_in remote_addr;
static uint8_t recv_data[BUFSZS];
static uint16_t G_mesg_id_A=0xab12;
static uint16_t G_mesg_id_B=0xab12;
send_mesg_info recive_send_mesg;

int sock = -1;

const char send_datas[] = "UDP server\n";



//包装通道 0-5
// 收到蛋重量信息通过消息队列发出去
void send_weight_info_to_load_egg(uint8_t *buf)
{
 
	 rt_err_t result;			

	 result= rt_mq_send(&masseg_weight, buf, MESSAGE_DAT_LEN);

	 if (result != RT_EOK)
   {
     DEBUG_ERR("send_weight_info_to_load_egg result=%d\n",result);
   }
}
static void udpserver_send( uint8_t *buf,uint16_t len)
{
	  sendto(sock, buf, len, 0,(struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
      rt_thread_delay(2);
}




void replay_commond(uint16_t msg_id,uint8_t result,uint32_t head)
{
	 uint32_t crc;
	 replay_mesg_info  replay_mesg;
	 
	 rt_memset(&replay_mesg,0,sizeof(replay_mesg_info));
	 replay_mesg.head =head;
	 replay_mesg.msg_len = sizeof(replay_mesg_info)-8;  //去掉 头盒CRC32
	 replay_mesg.mesg_id = msg_id;
     replay_mesg.result =result;
	 crc = CRC32_file((uint8_t *)&replay_mesg,sizeof(replay_mesg_info)-4);
	 replay_mesg.CRC32 = crc;
	 udpserver_send((uint8_t*)&replay_mesg,sizeof(replay_mesg_info));
}

// 没有调用
void get_weight_crack_info(uint32_t id)
{
  replay_commond(id,1,UDP_READ_MESSAGE_HEAD);
}

extern rt_uint32_t hal_crc32(uint8_t *buf,uint16_t len);


// 
static void udp_check_weight_alarm(weight_alarm_info *weight_alarm)
{
   uint8_t i;
	 for(i=0;i<6;i++)
	 {
		 if(weight_alarm->ch[i].alarm_weight>0)
		 {
		    set_device_alarm(WEIGHT_DEVICE_ERROR);
		 }
		 if(weight_alarm->ch[i].alarm_adc>0)
		 {
		   // set_device_alarm(WEIGHT_DEVICE_ERROR);
		 }
		 if(weight_alarm->ch[i].alarm_pee>0)
		 {
		   // set_device_alarm(WEIGHT_DEVICE_ERROR);
		 }
		 if(weight_alarm->ch[i].alarm_can>0)
		 {
		   // set_device_alarm(CRACK_DEVICE_ERROR);
		 }
		 if(weight_alarm->ch[i].alarm_crack>0)
		 {
		    //set_device_alarm(CRACK_DEVICE_ERROR);
		 }
	
	 }
	 if(weight_alarm->encode_err>0)
		{
		  //  set_device_alarm(WEIGHT_DEVICE_ERROR);
		}
		if(weight_alarm->senser_err>0)
		{
		   // set_device_alarm(WEIGHT_DEVICE_ERROR);
		}

}

//static weight_alarm_info weight_crack_tmp;
void recive_weight_crack_info(send_mesg_info *info)
{
    rt_kprintf("获取到重量数据 id=%d\n",info->mesg_id);
}
extern void show_weightB_pluse();
static send_mesg_info deviceB_dat;
void get_deviceB_weight_info(send_mesg_info *dat)
{
  rt_memcpy(dat,&deviceB_dat,sizeof(send_mesg_info));
}

uint32_t totle_egg[2]={0,0};

void analysis_commond(send_mesg_info *msg,uint8_t device)
{
   uint32_t crc,crc1;
	
	uint8_t i;
	

	
	//rt_kprintf("mesg id=%d head=%08x\n",msg->mesg_id,msg->head);
    if(msg->head !=UDP_MESSAGE_HEAD)
	{
		if(msg->head ==UDP_READ_MESSAGE_HEAD)
		{
		  recive_weight_crack_info(msg);	// 就打印一下重量信息的id
		}
		else 
		{
			rt_kprintf("头错误\n");
		  return ;
		}
	 }
	  crc = hal_crc32((uint8_t *)msg,sizeof(send_mesg_info)-4);
	  if(crc == msg->CRC32)
	  {
			// 称重信息校验正确
		  if(device==0)
		  {
				// 如果信息id和上次不一样则说明有新的称重数据
			if(G_mesg_id_A !=msg->mesg_id)
			{
				for(i=0;i<6;i++)
				{
					// 遍历这一排的6个格子看是不是都有蛋
					if(msg->weight[i]>0)
					{
					   totle_egg[0]++;	// 有蛋的话，蛋的总数加1，这个总数指什么呢？是指这6个格子的总数还是从什么时候(开机开始计？)开始计的总数？
					}
				}
				
				udp_check_weight_alarm((weight_alarm_info *)&msg->alarm[0]);	// 检查一下，称重板子发过来的数据里面有没有报警信息，有报警信息则需要设置报警并停机
				G_mesg_id_A = msg->mesg_id;	// 重新记录当前称重数据的id
				send_weight_info_to_load_egg((uint8_t *)msg);		// 将A通道的称重信息发出去，主控另外的线程在处理
			}
//			rt_kprintf("device=0\n");
		  }
		  else 
		  {
			if(G_mesg_id_B !=msg->mesg_id)
			{
				for(i=0;i<6;i++)
				{
					if(msg->weight[i]>0)
					{
					   totle_egg[1]++;
					}
				}
				G_mesg_id_B = msg->mesg_id;
				rt_memcpy(&deviceB_dat,msg,sizeof(send_mesg_info));	// 将B通道的重量信息保存在 deviceB_dat
				show_weightB_pluse();	// 同步一下信息2同步值有什么用
			}
	//		rt_kprintf("device=1\n");
		  }
			// 解析玩数据后，回复称重设备
		  replay_commond(msg->mesg_id,1,UDP_MESSAGE_HEAD);
 			//DEBUG_INFO("revive egg weight info mesg id=%d\n",msg->mesg_id);
	  }
	  else 
	  {
			DEBUG_ERR("revive  egg weight info crc err\n");
	  }
	  
}
static void udpserver_recv(void *parameter)
{
	    uint8_t device;
		int bytes_read;
	    char *ip_addr;
		fd_set readset;
		socklen_t addr_len;
		struct timeval timeout;
		struct sockaddr_in socket_addr;
		char recive_ipaddr[20];
		addr_len = sizeof(struct sockaddr);
        timeout.tv_sec = 1;
		timeout.tv_usec = 0;
    while (1)
    {

				FD_ZERO(&readset);
        FD_SET(sock, &readset);
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) != 0)
        {
					 rt_memset(recv_data,0,BUFSZS);
							 bytes_read = recvfrom(sock, recv_data, BUFSZS - 1, 0, (struct sockaddr *)&socket_addr, &addr_len);
					 recv_data[bytes_read] = '\0'; 
					
					 rt_memset(recive_ipaddr,0,sizeof(recive_ipaddr));
					 ip_addr = inet_ntoa(socket_addr.sin_addr);
					 rt_memcpy(recive_ipaddr,ip_addr,strlen(ip_addr));
					
					 remote_addr.sin_addr.s_addr = inet_addr(recive_ipaddr);
					 //rt_kprintf("(%s , %d) len=%d \n", recive_ipaddr, ntohs(socket_addr.sin_port),strlen(ip_addr));
					
						
					 if(rt_memcmp(CLIENT_IP,recive_ipaddr,strlen(ip_addr))==0)
					 {
						 // 如果是则为通道A的称重板子发过来的信息
						 device =0;
					 }
					 else 
					 {
						 // 否则为通道B的称重板子发过来的信息
						 device =1;
					 }
					 rt_memcpy(&recive_send_mesg,recv_data,sizeof(send_mesg_info));
					 
					 analysis_commond(&recive_send_mesg,device);
					 rt_thread_mdelay(2);
        }
				else 
				{	
							//rt_kprintf("time out\n");
				}
    }
}

int run_udpserver(void)
{
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        rt_kprintf("Socket error\n");

        return -RT_ERROR;
    }

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LISTEN_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));

	  remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(LISTEN_PORT);
    remote_addr.sin_addr.s_addr = inet_addr(CLIENT_IP);
    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));
		
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Bind error\n");
        return -RT_ERROR;
    }
		
    rt_kprintf("UDPServer Waiting for client on port 5000...\n");
		
		rt_thread_t  thread2;
		
		thread2 = rt_thread_create("udpRcv", udpserver_recv, RT_NULL, 2048, 11, 10);
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

INIT_APP_EXPORT(run_udpserver);
