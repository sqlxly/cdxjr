#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "drv_tcp_common.h"
#include "rtservice.h"
#include "mdbus_tcp_slaver.h"

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME "tcp_slave"
#define DBG_LEVEL        DBG_LOG
#include "drv_log.h"

struct mbtcp_session {
    int fd;
    uint32_t tick_timeout;
    rt_slist_t slist;
};

#define MBTCP_SESSION_MAX_NUM 5
#define MBTCP_SESSION_TIMEOUT 10

static int _listen_port = 0;
static pthread_mutex_t _mtx;
static rt_slist_t _header = RT_SLIST_OBJECT_INIT(_header);

pthread_mutex_t slave_mtx;

static int addr_check(agile_modbus_t *ctx, struct agile_modbus_slave_info *slave_info)
{
    int slave = slave_info->sft->slave;
    if ((slave != ctx->slave) && (slave != AGILE_MODBUS_BROADCAST_ADDRESS) && (slave != 0xFF))
        return -AGILE_MODBUS_EXCEPTION_UNKNOW;

    return 0;
}

const agile_modbus_slave_util_t slave_util = {
    bit_maps,
    sizeof(bit_maps) / sizeof(bit_maps[0]),
    input_bit_maps,
    sizeof(input_bit_maps) / sizeof(input_bit_maps[0]),
    register_maps,
    sizeof(register_maps) / sizeof(register_maps[0]),
    input_register_maps,
    sizeof(input_register_maps) / sizeof(input_register_maps[0]),
    addr_check,
    NULL,
    NULL
};

static int mbtcp_session_get_num(void)
{
    int num = 0;

    pthread_mutex_lock(&_mtx);
    num = rt_slist_len(&_header);
    pthread_mutex_unlock(&_mtx);

    return num;
}

static void mbtcp_session_delete(struct mbtcp_session *session)
{
    pthread_mutex_lock(&_mtx);
    rt_slist_remove(&_header, &(session->slist));
    pthread_mutex_unlock(&_mtx);

    modbus_tcp_close(session->fd);

    free(session);
}

static void *mbtcp_session_entry(void *param)
{
    struct mbtcp_session *session = (struct mbtcp_session *)param;
    int option = 1;
    int rc = setsockopt(session->fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&option, sizeof(int));
    if (rc < 0){
        goto _exit_excp;
		}

    int flags = fcntl(session->fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(session->fd, F_SETFL, flags);

    flags = fcntl(session->fd, F_GETFD);
    flags |= O_CLOEXEC;
    fcntl(session->fd, F_SETFD, flags);

    session->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(MBTCP_SESSION_TIMEOUT * 1000);

    uint8_t ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint8_t ctx_read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];

    agile_modbus_tcp_t ctx_tcp;
    agile_modbus_t *ctx = &ctx_tcp._ctx;
    agile_modbus_tcp_init(&ctx_tcp, ctx_send_buf, sizeof(ctx_send_buf), ctx_read_buf, sizeof(ctx_read_buf));
    agile_modbus_set_slave(ctx, 1);
 
    while (1) {
        rc = modbus_tcp_receive(session->fd, ctx->read_buf, ctx->read_bufsz, 1000);
        if (rc < 0)
            break;
        if (rc > 0) {
            int send_len = agile_modbus_slave_handle(ctx, rc, 0, agile_modbus_slave_util_callback, &slave_util, NULL);
            modbus_tcp_flush(session->fd);
            if (send_len > 0) {
                session->tick_timeout = rt_tick_get() + rt_tick_from_millisecond(MBTCP_SESSION_TIMEOUT * 1000);

                if (modbus_tcp_send(session->fd, ctx->send_buf, send_len) != send_len)
                    break;
            }
        }

        if ((rt_tick_get() - session->tick_timeout) < (RT_TICK_MAX / 2))
            break;
				
				rt_thread_mdelay(10);
    }

_exit_excp:
    LOG_W("socket %d close.", session->fd);
    mbtcp_session_delete(session);
		
		return NULL;
}

static int mbtcp_session_create(int fd)
{
    if (fd < 0)
        return -1;

    if (mbtcp_session_get_num() >= MBTCP_SESSION_MAX_NUM)
        return -1;

    struct mbtcp_session *session = malloc(sizeof(struct mbtcp_session));
    if (session == NULL)
        return -1;

    memset(session, 0, sizeof(struct mbtcp_session));
    session->fd = fd;
    rt_slist_init(&(session->slist));
    pthread_mutex_lock(&_mtx);
    rt_slist_append(&_header, &(session->slist));
    pthread_mutex_unlock(&_mtx);

    pthread_t tid;
    pthread_create(&tid, NULL, mbtcp_session_entry, session);
    pthread_detach(tid);

    return 0;
}

static void *mbtcp_entry(void *param)
{
    int server_fd = -1, ret;
		unsigned long ul = 1;
	
    // Select use
    fd_set readset, exceptset;
    // Select timeout
    struct timeval select_timeout;

_tcp_start:
    LOG_I("mbtcp server running.");
    server_fd = modbus_tcp_listen(_listen_port, 6);
    if (server_fd < 0)
        goto _tcp_restart;

		/* set socket to be non-blocking */
    ret = ioctlsocket(server_fd, FIONBIO, (unsigned long *)&ul);
    if (ret < 0)
    {
        LOG_E("set socket ctl failed");
    }
		
    while (1) {
        FD_ZERO(&readset);
        FD_ZERO(&exceptset);

        FD_SET(server_fd, &readset);
        FD_SET(server_fd, &exceptset);

        select_timeout.tv_sec = 1;
        select_timeout.tv_usec = 0;
        int rc = select(server_fd + 1, &readset, NULL, &exceptset, &select_timeout);
        if (rc == -1) {
            if (errno == EINTR)
                continue;
        }
        if (rc < 0)
            break;

        if (rc > 0) {
            if (FD_ISSET(server_fd, &exceptset))
                break;
            if (FD_ISSET(server_fd, &readset)) {
                int client_fd = modbus_tcp_accept(server_fd);
                if (client_fd < 0)
                    break;
                if (mbtcp_session_create(client_fd) < 0)
                    modbus_tcp_close(client_fd);
            }
        }
    }

_tcp_restart:
    LOG_W("mbtcp server go wrong, now wait restarting...");
    if (server_fd >= 0) {
        modbus_tcp_close(server_fd);
        server_fd = -1;
    }

    sleep(1);
    goto _tcp_start;
}

int tcp_slave_init(int port, pthread_t *tid)
{
    if (port <= 0) {
        LOG_E("Port must be greater than 0!");
        return -1;
    }

    _listen_port = port;

    pthread_mutex_init(&_mtx, NULL);

    pthread_create(tid, NULL, mbtcp_entry, NULL);

    return 0;
}

int init_modbus_tcp_pthread(void)
{

    pthread_mutex_init(&slave_mtx, NULL);

    pthread_t tcp_tid;

    int rc2 = tcp_slave_init(502, &tcp_tid);

//    if (rc2 == 0)
//        pthread_join(tcp_tid, NULL);

    return 0;
}
