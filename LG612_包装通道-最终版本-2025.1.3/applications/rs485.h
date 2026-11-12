#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


#ifndef __RS485_H_
#define __RS485_H_

#define RT_DEVICE_FLAG_RX_NON_BLOCKING    0x2000   
#define RT_DEVICE_FLAG_TX_BLOCKING        0x4000  

#define UART_RX_BUFSIZE 512
#define PER_FARME_TIME    1   //ms  ∂œ’Î ±º‰

typedef struct 
{
	uint8_t dat[UART_RX_BUFSIZE];
	uint16_t rx_len;
  rt_tick_t tick;
}rs485_type;

int RS4851_init(void);
int RS4852_init(void);

void rs4851_clear_buf(void);
void rs4852_clear_buf(void);

rt_err_t rs4851_send_dat(uint8_t *str,int len);
rt_err_t rs4852_send_dat(uint8_t *str,int len);
uint16_t rs4851_read_dat(uint8_t *buf,uint16_t len ,uint8_t time_out);
uint16_t rs4852_read_dat(uint8_t *buf,uint16_t len ,uint8_t time_out);

#endif   //__RS485_H_


