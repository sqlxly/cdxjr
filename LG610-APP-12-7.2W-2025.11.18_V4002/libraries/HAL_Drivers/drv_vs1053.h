
#include <board.h>
#include <rtthread.h>
#ifndef _DERV_VS1053_H__
#define _DERV_VS1053_H__

rt_err_t rt_vs1053_dev_init(GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin);
rt_err_t vs1053_write_byte(uint8_t dat);
rt_err_t vs1053_read_byte(uint8_t *recive_dat);
rt_err_t vs1053_transfer(uint8_t *sbuf,uint8_t *rbuf,uint8_t len);
uint8_t VS_SPI_ReadWriteByte(uint8_t dat);
#endif  	// _DERV_VS1053_H__
