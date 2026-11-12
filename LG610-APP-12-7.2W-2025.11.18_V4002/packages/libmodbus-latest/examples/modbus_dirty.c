
#include <modbus.h>
#include <stdio.h>
#include <string.h>
#include <dfs_posix.h>
#include <sys/time.h>
#include <dfs_select.h>
#include <sal_socket.h>
#include "mdebug.h"
#include "modbus_dirty.h"
#include "mail_box.h"
#include "device_alarm.h"
#include "E6B2.h"
#include "app_screen.h"

#define MAX_CLIENT_NUM  6
#define MAX_ERROR_COUNT 2
#define BYTE_TIME_OUT    1*1000
#define CLIENT_TIMEOUT  10      //?? s
#define INPUT_REG_NB    512
#define HOT_REG_NB    512
#define PORT1         601
ALIGN(RT_ALIGN_SIZE)


static transport_eg_type tsp_egg;

static uint32_t dirty_mesg_id=0x12437834;
void init_trsp_egg_info()
{
  rt_memset(&tsp_egg,0,sizeof(transport_eg_type));
}

uint16_t recive_dirty_info_pluse[5];	// 这些变量赋值了，最后没有地方用呀，搞这个干什么
static uint8_t dirty_err_flag=0;
void send_message_to_ai_device(uint8_t *dat)
{
	rt_err_t result;	
	
	result= rt_mq_send(&masseg_ai, dat, AI_MESSAGE_DAT_LEN);
	if (result != RT_EOK)
    {
	   dirty_err_flag=1;
	   set_device_alarm(DIRTY_DEVICE_ERROR);
      //DEBUG_ERR("ai ch send_message ERR ch=%d\n");
	   recive_dirty_info_pluse[0]= recive_dirty_info_pluse[1];
	   recive_dirty_info_pluse[1]= recive_dirty_info_pluse[2];
	   recive_dirty_info_pluse[2]= recive_dirty_info_pluse[3];
	   recive_dirty_info_pluse[3]= recive_dirty_info_pluse[4];
	   recive_dirty_info_pluse[4]= ((get_spt_encode_pluse()%10000)/100);
    }
	if(dirty_err_flag==0)
	{
	   recive_dirty_info_pluse[0]= recive_dirty_info_pluse[1];
	   recive_dirty_info_pluse[1]= recive_dirty_info_pluse[2];
	   recive_dirty_info_pluse[2]= recive_dirty_info_pluse[3];
	   recive_dirty_info_pluse[3]= recive_dirty_info_pluse[4];
	   recive_dirty_info_pluse[4]= ((get_spt_encode_pluse()%10000)/100);
	}
}

uint32_t disp_dirty_id[20];
uint32_t disp_dirty_pluse[20];
uint16_t disp_count=0;
uint8_t dirty_sync_err_flag=0;
uint8_t dirty_sync_err_count=0;
void decode_dirty_dat(uint16_t *dat)
{

	uint8_t buf[AI_MESSAGE_DAT_LEN+10];
	rt_memset(buf,0,sizeof(buf));
	rt_memcpy(buf,dat,AI_MESSAGE_DAT_LEN);
	if(dirty_mesg_id != dat[0])	// 脏蛋的信息id和上一次不一样，说明有新数据来了
	{
		disp_count++;
		if(disp_count>19)
		{
		  disp_count =0;
		}
		if(dirty_sync_err_flag==0)
		{
			disp_dirty_pluse[disp_count]= get_spt_encode_pluse()%50000;		// 往屏幕的脏蛋同步数据页面(215页面)同步数据，但是该页面好像没有办法跳过去,最多同步20个数据，然后重头开始
			disp_dirty_id[disp_count]=dat[0];															// 往屏幕的脏蛋同步数据页面(215页面)同步数据，但是该页面好像没有办法跳过去,最多同步20个数据，然后重头开始
		}
		dirty_mesg_id = dat[0];
		send_message_to_ai_device(buf);								// 往脏蛋设备发数据干什么
	}
}

//offest(0-1000) 偏移量      决定什么时候位移
// pluse 当前脉冲数
static uint32_t move_count=0;
void trsp_egg_move_one_line(uint32_t pluse,uint32_t offest)   //
{
  uint16_t i;
	uint32_t line;
	uint32_t adj_pluse;
	if(pluse>=offest)
	{
		adj_pluse = pluse - offest;
		line = adj_pluse/1000;
		if(line ==(move_count+1))  //走了一格了
		{
			move_count = line;
			for(i=0;i<TRANSPORT_STEP_NUMBER-1;i++)
			{
				rt_memcpy(&tsp_egg.info[i+1][0],&tsp_egg.info[i][0],sizeof(ai_dat_type));
			}
			uint16_t have_egg=0;
			for(i=0;i<6;i++)
			{
			//	tsp_egg.info[0][i].empty = current_dat.body[i].is_egg;
			//	tsp_egg.info[0][i].dity_leve = current_dat.body[i].dirty;
		//		tsp_egg.info[0][i].colour = current_dat.body[i].level;
		//	  have_egg+=current_dat.body[i].is_egg;
			}
			//rt_memcpy(&tsp_egg.info[0][0],&current_dat,sizeof(ai_dat_type));
			DEBUG_WARN("recive diry info pluse=%d egg count=%d\n",pluse%1000,have_egg);
		}
	}
}

static uint32_t recive_blood_dat_pluse=0;


static uint8_t bood_dat=0;
void clear_blood_com_err_pluse()
{
	recive_blood_dat_pluse = get_spt_encode_pluse();
}



uint8_t get_blood_dat()
{
	uint32_t pluse;
    uint32_t add;
 
	pluse = get_spt_encode_pluse();
    add = pluse- recive_blood_dat_pluse;
	WriteMBHotRegU16(830,add);
	if(add>500)
	{
		if(pluse>2000)
		{
		///DEBUG_INFO("ERR pluse=%d\n",add);
			set_device_alarm(COLOUR_DEVICE_ERROR);
		}
		return 0;
	}
	return bood_dat;
}
extern void set_cha_crack_info(uint16_t *dat);	
extern void set_chb_crack_info(uint16_t *dat);

static void test_thread_slave(void *param)
{
    int server_fd = -1;
	rt_base_t level;
	  uint8_t mb_cmd;
	  uint16_t write_addr;
    modbus_t *ctx = NULL;
	  int max_fd = -1;
    fd_set readset;
    int rc;
    struct timeval select_timeout;
    modbus_mapping_t *ai_mb_mapping = NULL;
	
	  init_trsp_egg_info();
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
								if(client_sock_fd ==-1)
								{
									goto _mbtcp_restart;
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
                        if (rc > 0)	// 收到脏蛋发过来的数据
                        {
													  mb_cmd =0;			// modbus的命令类型
													  write_addr =0;	// modbus的读写地址
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
																	if(mb_cmd==16)	// 写多个保持寄存器
																	{
																		if(write_addr==128)		// 血斑信息地址
																		{
																			level = rt_hw_interrupt_disable();
																			recive_blood_dat_pluse = get_spt_encode_pluse();
																			bood_dat = ai_mb_mapping->tab_registers[128];	// 得到血斑的蛋的信息，这个是1个鸡蛋还是1排鸡蛋的血斑信息
																			rt_hw_interrupt_enable(level);
																
																			//rt_kprintf("cp=%d  a=%d val=%d \n",get_spt_encode_pluse(),get_spt_encode_pluse()-up_pluse,ai_mb_mapping->tab_registers[128]);
													
																		}
																		else if(write_addr==0)	// 脏蛋信息地址，这个是1个鸡蛋还是1排鸡蛋的脏蛋信息
																		{
																			decode_dirty_dat(&ai_mb_mapping->tab_registers[0]);
																	
																		}
																		#if 0	// qianxuping 裂纹是和重量一起发过来的,这里将光检裂纹信息注释
																		else if(write_addr==200)	// 裂纹a信息地址，这个是1个鸡蛋还是1排鸡蛋的裂纹信息
																		{
																			set_cha_crack_info(&ai_mb_mapping->tab_registers[200]);
													
																		}
																		else if(write_addr==210)	// 裂纹b信息地址
																		{
																			 set_chb_crack_info(&ai_mb_mapping->tab_registers[210]);
																		}
																		#endif
																	}
																}
													
                                client_session[i].tick_timeout = rt_tick_get() + rt_tick_from_millisecond(CLIENT_TIMEOUT * 1000);
                            }
                        }
                        else
                        {
							              //DEBUG_CARE("dirty close socket\n");
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
				//	DEBUG_CARE("dirty close socket\n");
                    close(client_session[i].fd);
                    client_session[i].fd = -1;
                }
            }
        }
    }

_mbtcp_restart:
    if(server_fd >= 0)
    {
		//DEBUG_CARE("dirty close socket\n");
        close(server_fd);
        server_fd = -1;
    }

    for(int i =0;i<MAX_CLIENT_NUM;i++)
    {
        if(client_session[i].fd >= 0)
        {
			//DEBUG_CARE("dirty close socket\n");
            close(client_session[i].fd);
            client_session[i].fd = -1;
        }
    }
    rt_kprintf("tcp restart\n");
    rt_thread_mdelay(100);
    goto _mbtcp_start;

    modbus_free(ctx);
}

static int modbus_dirty_init(void)
{
    rt_thread_t  tid1;
	  rt_err_t result;


		tid1 = rt_thread_create("dir_mb",
                           test_thread_slave, RT_NULL,
                           3048,
                           16, 10);
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		else 
			return RT_ERROR;
    return RT_EOK;
}
INIT_APP_EXPORT(modbus_dirty_init);

