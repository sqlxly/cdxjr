#include <rtthread.h>
#include <ipc/workqueue.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./tools/common.h"

#define SERVER_IP "10.10.0.58"
#define LISTEN_PORT 5000
#define BUFSZS   2048

#define VECTOR_BOX_LEN 10    //缓存10帧数据用于主控查询

//socket通信数据
static struct sockaddr_in local_addr;
static struct sockaddr_in remote_addr;
static uint8_t recv_data[BUFSZS];
static int sock = -1;

//创建邮箱
static struct rt_mailbox mbSnd;
static char mbSnd_pool[128];
static struct rt_mailbox mbRcv;
static char mbRcv_pool[128];

static Send_Mesg_Info sndMsg;

static Egg_Quality_Info Egg_q_vector_box[VECTOR_BOX_LEN];

static Egg_Quality_Info req_Info;
	
static uint8_t box_save_pos = 0;

//当需要发送UDP数据时 异步执行数据发送
void udp_submit_work(rt_ubase_t addr)
{ 
	 rt_memcpy(&Egg_q_vector_box[box_save_pos], (Egg_Quality_Info *)addr, sizeof(Egg_Quality_Info));
	 rt_mb_send(&mbSnd, (rt_ubase_t)&Egg_q_vector_box[box_save_pos]);
	 box_save_pos = (box_save_pos + 1) % VECTOR_BOX_LEN;
}

uint32_t hardware_crc32(uint8_t *data, int len)
{
		uint32_t result = 0;
    struct rt_hwcrypto_ctx *ctx;
    
    struct hwcrypto_crc_cfg cfg =
    {
        .last_val = 0xFFFFFFFF,
        .poly = 0x04C11DB7,
        .width = 32,
        .xorout = 0x00000000,
        .flags = 0,
    };

    ctx = rt_hwcrypto_crc_create(rt_hwcrypto_dev_default(), HWCRYPTO_CRC_CRC32);
		
    rt_hwcrypto_crc_cfg(ctx, &cfg);

    result = rt_hwcrypto_crc_update(ctx, data, len);

    rt_hwcrypto_crc_destroy(ctx);
		
		return result;
}

//工作队列中发送UDP数据到远端
static void udpclient_send(void *parameter)
{
	int ch = 0, times = 3, isSnd = RT_TRUE;
	rt_ubase_t sequenceNo = 0;
	struct AlarmInfo alarm;
	
	rt_ubase_t *dataAddr;
	Egg_Quality_Info *pData;
	
	rt_memset(&sndMsg.alarm[0], 0, 22);

	while(1)
	{
		if (rt_mb_recv(&mbSnd, (rt_ubase_t *)&dataAddr, RT_WAITING_FOREVER) == RT_EOK)
		{
			pData = (Egg_Quality_Info *)dataAddr;
			switch(pData->command)
			{
				case 1001:    //发送一排鸡蛋的裂纹和重量信息
				{
						sndMsg.head = UDP_MSG_HEAD;
						sndMsg.msg_len = 40;  						//sizeof(weight) + sizeof(level) + sizeof(alarm)
						//sndMsg.mesg_id = rand();  				//产生随机数
						sndMsg.mesg_id = pData->mesg_id;
//					rt_kprintf("id=%d weight=%d\n", sndMsg.mesg_id, pData->weight[0]);

						for(ch=0; ch<WEIGH_CHANNEL; ch++)
						{
							sndMsg.weight[ch] = pData->weight[ch];
							sndMsg.level[ch] = pData->level[ch];
							
							// 这个位置称重打印出鸡蛋时候的重量数据
						}

						alarm = getAlarmInfo();
						rt_memcpy(&sndMsg.alarm[0], &alarm, sizeof(struct AlarmInfo));

						sndMsg.CRC32 = hardware_crc32((uint8_t *)&sndMsg, sizeof(Send_Mesg_Info)-4);

						isSnd = RT_TRUE;
						for(times=0; times<3; times++)
						{
							if(isSnd == RT_TRUE)
							{
								sendto(sock, &sndMsg, sizeof(Send_Mesg_Info), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
//								rt_kprintf("send to host %d\n", times+1);
							}
							// 发送了数据后一直等待610回复
							if (rt_mb_recv(&mbRcv, (rt_ubase_t *)&sequenceNo, 40) == RT_EOK)
							{
								if(sndMsg.mesg_id == sequenceNo)
								{
									break;
								}else{
									isSnd = RT_FALSE;
									continue;
								}
							}else{
								isSnd = RT_TRUE;
							}
						}
					} break;
				case 1002:  //查询一排鸡蛋数据
						sndMsg.head = REQ_UDP_MSG_HEAD; 	//固定值为 0x11223344
						sndMsg.msg_len = 40;  						//sizeof(weight) + sizeof(level) + sizeof(alarm)
						sndMsg.mesg_id = pData->mesg_id;
						for(ch=0; ch<WEIGH_CHANNEL; ch++)
						{
							sndMsg.weight[ch] = pData->weight[ch];
							sndMsg.level[ch] = pData->level[ch];
						}
						alarm = getAlarmInfo();
						rt_memcpy(&sndMsg.alarm[0], &alarm, sizeof(struct AlarmInfo));
						sndMsg.CRC32 = hardware_crc32((uint8_t *)&sndMsg, sizeof(Send_Mesg_Info)-4);
						sendto(sock, &sndMsg, sizeof(Send_Mesg_Info), 0, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr));
					break;
				default: break;
			}
		}
	}
}

//UDP数据接收线程
static void udpclient_recv(void *parameter)
{
		int bytes_read, n, flag = 0;
		fd_set readset;
		socklen_t addr_len;
		struct sockaddr_in socket_addr;
		struct timeval timeout;
		Send_Mesg_Info *p;

		addr_len = sizeof(struct sockaddr);
		timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    while (1)
    {
				FD_ZERO(&readset);
        FD_SET(sock, &readset);
        if (select(sock + 1, &readset, RT_NULL, RT_NULL, &timeout) == 0)
        {
            continue;
        }else{
            bytes_read = recvfrom(sock, recv_data, BUFSZS - 1, 0, (struct sockaddr *)&socket_addr, &addr_len);	
						if(bytes_read <= 0) continue;
						recv_data[bytes_read] = '\0';

						p = (Send_Mesg_Info *)recv_data;
						switch(p->head)
						{
							case UDP_MSG_HEAD:				// 收到的是我发送后610回复的数据
								rt_mb_send(&mbRcv, (rt_uint32_t)p->mesg_id);
								break;
							case REQ_UDP_MSG_HEAD:		// 收到的是610主动请求查询的数据
								flag = 0;
								for(n=0; n<VECTOR_BOX_LEN; n++){
									if(Egg_q_vector_box[n].mesg_id == p->mesg_id){		// 找610请求的是查询那一包的数据
										rt_memcpy(&req_Info, &Egg_q_vector_box[n], sizeof(Egg_Quality_Info));
										flag = 1;
										break;
									}
								}
								if(0 == flag){  //没有查询到结果 发送空数据过去
									rt_memset(&req_Info, 0, sizeof(Egg_Quality_Info));
									req_Info.mesg_id = p->mesg_id;
								}
								req_Info.command = 1002;  //修改命令码为1002 表示发送查询结果数据包
								rt_mb_send(&mbSnd, (rt_ubase_t)&req_Info);
								break;
							default: break;
						}
//						rt_kprintf("\n(%s , %d) said : ", inet_ntoa(socket_addr.sin_addr), ntohs(socket_addr.sin_port));
				}
    }
}

int udpclient_init(void)
{
		rt_err_t result;

    result = rt_mb_init(&mbSnd, "mbSnd", &mbSnd_pool[0], sizeof(mbSnd_pool) / 4, RT_IPC_FLAG_FIFO);          
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }

    result = rt_mb_init(&mbRcv, "mbRcv", &mbRcv_pool[0], sizeof(mbRcv_pool) / 4, RT_IPC_FLAG_FIFO);          
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
		
		//创建udp通信套接字
    if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        rt_kprintf("Socket error\n");
				return -RT_ERROR;
    }

		//创建UDP本地坚监听数据端口
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(LISTEN_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(local_addr.sin_zero), 0, sizeof(local_addr.sin_zero));

		//创建UDP数据发送到远端服务数据
		remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(LISTEN_PORT);
    remote_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    rt_memset(&(remote_addr.sin_zero), 0, sizeof(remote_addr.sin_zero));

		//开始监听数据
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("Bind error\n");
        return -RT_ERROR;
    }
		
		rt_thread_t thread1, thread2;
		
		thread1 = rt_thread_create("udpSnd", udpclient_send, RT_NULL, 2048, 15, 10);
		if (thread1 != RT_NULL)
		{
				rt_thread_startup(thread1);
		}
		else
		{
				rt_kprintf("create udpSnd thread failed!\n");
				return -RT_ERROR;
		}
		
		//UDP接收数据处理线程
		thread2 = rt_thread_create("udpRcv", udpclient_recv, RT_NULL, 1024, 16, 10);
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

