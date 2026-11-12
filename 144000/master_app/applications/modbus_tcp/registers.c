#include "mdbus_tcp_slaver.h"
#include "weighAndCrackQueue.h"

uint16_t u_tab_registers[16384] = {0};

extern void mb_tcp_send(rt_uint16_t baseAddr, rt_uint16_t len);

static int get_map_buf(int index, int len, void *buf, int bufsz)
{
    uint16_t *ptr = (uint16_t *)buf;

    pthread_mutex_lock(&slave_mtx);
    for (int i = 0; i < len; i++) {
				ptr[i] = u_tab_registers[index + i];
    }
    pthread_mutex_unlock(&slave_mtx);

    return 0;
}

static int set_map_buf(int index, int len, void *buf, int bufsz)
{
    uint16_t *ptr = (uint16_t *)buf;

    pthread_mutex_lock(&slave_mtx);
    for (int i = 0; i < len; i++) {
        u_tab_registers[index + i] = ptr[i];
    }
    pthread_mutex_unlock(&slave_mtx);

		// 发送邮件通知modbus数据处理线程
		mb_tcp_send(index, len);

    return 0;
}

const agile_modbus_slave_util_map_t register_maps[1] = {
    {0, 16383, get_map_buf, set_map_buf}
};
