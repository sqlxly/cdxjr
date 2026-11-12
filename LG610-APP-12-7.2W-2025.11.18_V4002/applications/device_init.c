#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_key.h"
#include "mdebug.h"
#include "vs1053.h"
#include "mdebug.h"
#include "uadc.h"
#include "udac.h"
#include "relay.h"
#include "spi_flash_sfud.h"
#include "user_set.h"
#include "product_set.h"
#include "common.h"
#include "device_alarm.h"

int board_device_init()
{
  rt_err_t result=RT_EOK;
	
  bkp_sram_init(); 
	init_alarm_dat();
//	result +=adc_init();
  result +=G8403_init();
  result +=relay_init();
	result +=spi_flash_init();
	user_dat_init();
	DEBUG_CARE("SPI 包装信息总数=%d\n",PRODUCT_PACKGE_INFO_COUNT);
	DEBUG_CARE("DEVICE INIT RESULT=%d\n",result);
}

INIT_ENV_EXPORT(board_device_init);