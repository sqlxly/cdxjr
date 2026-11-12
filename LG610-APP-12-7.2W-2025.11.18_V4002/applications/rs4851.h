#ifndef __RS4851_H_
#define __RS4851_H_


#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


#define UART3_RX_BUFSIZE 1024

typedef struct 
{
	uint8_t dat[UART3_RX_BUFSIZE];
	uint16_t rx_len;
  rt_tick_t tick;
}rs4851_type;

void rs4851_clear_buf(void);
rt_err_t RS485_1_init(void);
rt_err_t rs4851_send_dat(uint8_t *str,int len);
uint16_t rs4851_read_dat(uint8_t *buf,uint16_t len ,uint8_t time_out);
#endif

