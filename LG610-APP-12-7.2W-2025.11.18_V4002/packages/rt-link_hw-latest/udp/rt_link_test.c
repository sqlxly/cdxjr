#include <rtthread.h>
#include <rtlink.h>
#include "mdebug.h"
#define DBG_ENABLE
#define DBG_TAG "rtlink_exam"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define TEST_CONTEXT    "This message is sent by RT-Link"

static struct rt_link_service serv_socket;

static void send_cb(struct rt_link_service *service, void *buffer)
{
    DEBUG_CARE("send_cb: service (%d) buffer (0x%p) err(%d)", service->service, buffer, service->err);
}

static void recv_cb(struct rt_link_service *service, void *data, rt_size_t size)
{
    DEBUG_WARN("recive dat=%s len=%d\n",data,size);

    if (size)
    {
      //  LOG_HEX("example",8,data,size); /* ??????? 16 ???????? ulog ??*/
        rt_free(data);/* data ?????? rtlink ????,???????????? */
    }
}

static int rtlink_exsend(int argc, char **argv)
{
    char *data = RT_NULL;
    rt_size_t length = 0;

    if (argc == 1)
    {
        data = rt_malloc(sizeof(TEST_CONTEXT));
        rt_memcpy(data, TEST_CONTEXT, sizeof(TEST_CONTEXT) - 1);
        length = rt_link_send(&serv_socket, data, sizeof(TEST_CONTEXT) - 1);
        LOG_I("send data length: %d.", length);
        rt_free(data);
    }
    return 0;
}
MSH_CMD_EXPORT(rtlink_exsend, rt link layer send test);

int rtlink_exinit(void)
{
    /* service ???????? */
    serv_socket.service = RT_LINK_SERVICE_SOCKET;
    serv_socket.timeout_tx = RT_WAITING_FOREVER;
    serv_socket.flag =  RT_LINK_FLAG_CRC;
    serv_socket.recv_cb = recv_cb;
    serv_socket.send_cb = send_cb;
    rt_link_service_attach(&serv_socket);
    return RT_EOK;
}
MSH_CMD_EXPORT(rtlink_exinit, rt link example init);
