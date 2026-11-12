#ifndef __TERMINAL_PROC_H__
#define __TERMINAL_PROC_H__

#include <rtthread.h>
#include <rtdevice.h>

rt_err_t _operate_mbus1_mq_send(char *send, rt_int32_t len);
rt_err_t _operate_mbus1_mq_recv(char *recv, int len, rt_int32_t timeout);

//rt_err_t _operate_mbus2_mq_send(char *send, rt_int32_t len);
//rt_err_t _operate_mbus2_mq_recv(char *recv, int len, rt_int32_t timeout);

//rt_err_t _operate_mbus3_mq_send(char *send, rt_int32_t len);
//rt_err_t _operate_mbus3_mq_recv(char *recv, int len, rt_int32_t timeout);

#endif
