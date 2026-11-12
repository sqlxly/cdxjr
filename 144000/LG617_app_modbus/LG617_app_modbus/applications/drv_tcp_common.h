#ifndef __DRV_TCP_COMMON_H
#define __DRV_TCP_COMMON_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int modbus_tcp_listen(int port, int nb_connection);
int modbus_tcp_accept(int s);
void modbus_tcp_close(int s);
int modbus_tcp_send(int s, const uint8_t *buf, int length);
int modbus_tcp_receive(int s, uint8_t *buf, int bufsz, int timeout);
int modbus_tcp_flush(int s);
int modbus_tcp_connect(const char *ip, int port);

#ifdef __cplusplus
}
#endif

#endif
