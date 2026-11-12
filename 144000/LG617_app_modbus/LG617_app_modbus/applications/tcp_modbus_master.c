#include "agile_modbus.h"
#include "drv_tcp_common.h"
#include <ipc/workqueue.h>
#include <rtthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include "netdb.h"
#include "string.h"		
#include "stdio.h"
#include "./tools/common.h"

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME "mbtcp_master"
#define DBG_LEVEL        DBG_LOG
#include "drv_log.h"

uint16_t modbus_baseAddr = 8000;

//创建邮箱
static struct rt_mailbox mbSnd;
static char mbSnd_pool[128];

static struct rt_thread mb_tcp_thread;
static char mb_tcp_thread_stack[2048];
static Egg_Quality_Info Egg_q_vector_box;
static int master_sock = -1;

//当需要发送UDP数据时 异步执行数据发送
void udp_submit_work(rt_ubase_t addr)
{
	 rt_memcpy(&Egg_q_vector_box, (Egg_Quality_Info *)addr, sizeof(Egg_Quality_Info));
	 rt_mb_send(&mbSnd, (rt_ubase_t)&Egg_q_vector_box);
}

int modbus_write_holdregisters(agile_modbus_t *ctx, int _sock, uint16_t addr, int nb,  const uint16_t *value)
{
	int send_len = agile_modbus_serialize_write_registers(ctx, addr, nb, value);
	modbus_tcp_send(_sock, ctx->send_buf, send_len);
	int read_len = modbus_tcp_receive(_sock, ctx->read_buf, 12, 300);
	if (read_len < 0) {
		LOG_E("Receive error, now exit.");
		return -1;
	}
	if (read_len == 0) {
		LOG_W("Receive timeout.");
		return -2;
	}
	int rc = agile_modbus_deserialize_write_registers(ctx, read_len);
	if (rc < 0) {
			LOG_W("Receive failed. %d", read_len);
			if (rc != -1)
					LOG_W("Error code:%d", -128 - rc);
			return -3;
	}

	return 0;
}

int modbus_read_holdregisters(agile_modbus_t *ctx, int _sock, uint16_t addr, int nb, uint16_t *value)
{
	int send_len = agile_modbus_serialize_read_registers(ctx, addr, nb);
	modbus_tcp_send(_sock, ctx->send_buf, send_len);
	int read_len = modbus_tcp_receive(_sock, ctx->read_buf, 9+nb*2, 300);
	if (read_len < 0) {
		LOG_E("Receive error, now exit.");
		return -1;						
	}
	if (read_len == 0) {
		LOG_W("Receive timeout. %d", _sock);
		return -2;
	}
	int rc = agile_modbus_deserialize_read_registers(ctx, read_len, value);
	if (rc < 0) {
		LOG_W("Receive failed.");
		if (rc != -1)
			LOG_W("Error code:%d", -128 - rc);
		return -3;
	}
	return 0;
}

static void modbus_cycle_thread_entry(void *param)
{
    uint8_t ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint8_t ctx_read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint16_t hold_register[30] = {0};
		rt_ubase_t *dataAddr;
		Egg_Quality_Info *pData;
		struct AlarmInfo alarm;
		
		//初始化modbus协议
    agile_modbus_tcp_t ctx_tcp;
    agile_modbus_t *ctx = &ctx_tcp._ctx;
    agile_modbus_tcp_init(&ctx_tcp, ctx_send_buf, sizeof(ctx_send_buf), ctx_read_buf, sizeof(ctx_read_buf));
    agile_modbus_set_slave(ctx, 1);

    LOG_I("Running.");
		while(1) 
		{
			if(master_sock < 0){  
				master_sock = modbus_tcp_connect("10.10.0.50", 502);
				if (master_sock == -1) {
						LOG_E("Connect %s:%d failed!", "10.10.0.50", 502);
						rt_thread_mdelay(200);
						continue;
				}
			}
			rt_kprintf("Master-slave motor initialization successful.\n");
			agile_modbus_set_slave(ctx, 1);
			while (1) 
			{
//					if (rt_mb_recv(&mbSnd, (rt_ubase_t *)&dataAddr, RT_WAITING_FOREVER) == RT_EOK)
					if (rt_mb_recv(&mbSnd, (rt_ubase_t *)&dataAddr, 2000) == RT_EOK)
					{
							pData = (Egg_Quality_Info *)dataAddr;
						  hold_register[0] = pData->mesg_id;
							hold_register[1] = pData->weight[0];
							hold_register[2] = pData->weight[1];
							hold_register[3] = pData->weight[2];
							hold_register[4] = pData->weight[3];
							hold_register[5] = pData->weight[4];
							hold_register[6] = pData->weight[5];
							hold_register[7] = pData->level[0];
							hold_register[8] = pData->level[1];
							hold_register[9] = pData->level[2];
							hold_register[10] = pData->level[3];
							hold_register[11] = pData->level[4];
							hold_register[12] = pData->level[5];
							alarm = getAlarmInfo();
							rt_memcpy(&hold_register[13], &alarm, sizeof(struct AlarmInfo));
							if(pData->command == 1001) {
									if(modbus_write_holdregisters(ctx, master_sock, modbus_baseAddr, 21,  hold_register) < 0) { 	//发送重量裂纹信息
											break;			
									}
							}
					}else{
							alarm = getAlarmInfo();
							rt_memcpy(&hold_register[13], &alarm, sizeof(struct AlarmInfo));
							if(modbus_write_holdregisters(ctx, master_sock, modbus_baseAddr+13, 8, &hold_register[13]) < 0) { //发送故障信息 也作为心跳报文
									break;			
							}	
					}
			}

			modbus_tcp_close(master_sock); //关闭socket连接
			master_sock = -1;
		}
}

int modbus_tcp_startup(void)
{
		rt_err_t result;

    result = rt_mb_init(&mbSnd, "mbSnd", &mbSnd_pool[0], sizeof(mbSnd_pool) / 4, RT_IPC_FLAG_FIFO);          
    if (result != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
		
		rt_thread_init(&mb_tcp_thread,
                   "mbTCP",
                   modbus_cycle_thread_entry,
                   RT_NULL,
                   &mb_tcp_thread_stack[0],
                   sizeof(mb_tcp_thread_stack),
                   12, 10);
    rt_thread_startup(&mb_tcp_thread);
									 
		return RT_EOK;
}

//INIT_APP_EXPORT(modbus_tcp_startup);

//static void show_status(int argc, char **argv)
//{
//	rt_kprintf("slave ETA:%04x %04x %04x\n\r\n", motor.s_ETA,motor.s_RFRD,motor.s_LCR);
//	rt_kprintf("master ETA:%04x %04x %04x\n", motor.m_ETA,motor.m_RFRD,motor.m_LCR);
//}
//MSH_CMD_EXPORT_ALIAS(show_status, show_status, show status of motor);

