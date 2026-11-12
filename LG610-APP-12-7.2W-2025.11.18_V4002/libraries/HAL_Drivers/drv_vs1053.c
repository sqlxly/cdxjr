/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2016-09-28     armink       first version.
 */
#include <rtthread.h>
#include <stdint.h>
#include <string.h>
#include <rtdevice.h>
#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include "drv_spi.h"
#include "drv_vs1053.h"
#include "mdebug.h"


#define RT_USING_VS1053
#define vs1053_SPI_DEVICE_NAME     "vs1053"
#define RT_VS1053_SPI_MAX_HZ 10000

#define RT_SFUD_DEFAULT_SPI_CFG                  \
{                                                \
    .mode = RT_SPI_MODE_0 | RT_SPI_MSB,          \
    .data_width = 8,                             \
    .max_hz = RT_VS1053_SPI_MAX_HZ,                \
}

static struct rt_spi_device *spi_dev_vs1053;


static rt_err_t vs1053_probe_ex( const char *spi_dev_name,struct rt_spi_configuration *spi_cfg)
{
    rt_spi_flash_device_t rtt_dev = RT_NULL;

    RT_ASSERT(spi_dev_name);

    rtt_dev = (rt_spi_flash_device_t) rt_malloc(sizeof(struct spi_flash_device));
    if (rtt_dev) {
        rt_memset(rtt_dev, 0, sizeof(struct spi_flash_device));
    }
  
 
    rtt_dev->rt_spi_device = (struct rt_spi_device *) rt_device_find(spi_dev_name);
    if (rtt_dev->rt_spi_device == RT_NULL || rtt_dev->rt_spi_device->parent.type != RT_Device_Class_SPIDevice) {
                LOG_E("ERROR: SPI device %s not found!", spi_dev_name);
    }
    rt_err_t result = rt_spi_configure(rtt_dev->rt_spi_device, spi_cfg);
    return result;
}

rt_err_t rt_vs1053_dev_init(GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin)
{
	 struct rt_spi_configuration cfg = RT_SFUD_DEFAULT_SPI_CFG;
	 char name[RT_NAME_MAX];
	 int result;
	
    __HAL_RCC_GPIOB_CLK_ENABLE();
    rt_hw_spi_device_attach("spi2", "vs1053", cs_gpiox, cs_gpio_pin);
    result = vs1053_probe_ex("vs1053", &cfg);
	  rt_strncpy(name, vs1053_SPI_DEVICE_NAME, RT_NAME_MAX);
    spi_dev_vs1053 = (struct rt_spi_device *)rt_device_find(name);
    if (!spi_dev_vs1053)
    {
			  result =-1;
        DEBUG_ERR("spi sample run failed! can't find %s device!\n", name);
    }
	  DEBUG_CARE("spi2 init result=%d\n",result);
	  return result;
}


rt_err_t vs1053_read_byte(uint8_t *recive_da)
{
	uint8_t send_dat;
	  if (spi_dev_vs1053 !=RT_NULL)
    {
			 return rt_spi_recv(spi_dev_vs1053,recive_da,1);
			 //return rt_spi_send_then_recv(spi_dev_vs1053, &send_dat, 0, &recive_dat, 1);
		}
		else 
			return -1;

}
rt_err_t vs1053_write_byte(uint8_t dat)
{
    uint8_t rev_dat;
	  uint8_t s_buf[10]="hello";
	  uint8_t r_buf[10];
	  if (spi_dev_vs1053 !=RT_NULL)
    {
		//	return rt_spi_send_then_recv(spi_dev_vs1053, dat, len);
		}
		else 
			return -1;
}


uint8_t VS_SPI_ReadWriteByte(uint8_t dat)
{
	  uint8_t rbuf;
		if (spi_dev_vs1053 !=RT_NULL)
    {
			 rt_spi_transfer(spi_dev_vs1053,&dat,&rbuf,1);
		}
		else 
		{
			return -1;
		}
		return rbuf;
}
rt_err_t vs1053_transfer(uint8_t *sbuf,uint8_t *rbuf,uint8_t len)
{
	  if (spi_dev_vs1053 !=RT_NULL)
    {
			return rt_spi_transfer(spi_dev_vs1053,sbuf,rbuf,len);
		}
		else 
			return -1;
}


