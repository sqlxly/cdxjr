#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "recode_dat.h"
#include "recode_dat.h"
#include <drv_flash.h>

#define BOARD_ADDR_OFFEST   0x08

void set_board_addr(uint8_t board_addr)
{
	uint8_t buf[8];
	buf[0]=board_addr;
	stm32_flash_erase(STM32_FLASH_END_ADDRESS-BOARD_ADDR_OFFEST, 0x08);
	stm32_flash_write(STM32_FLASH_END_ADDRESS-BOARD_ADDR_OFFEST, buf, 8);
	stm32_flash_read(STM32_FLASH_END_ADDRESS-BOARD_ADDR_OFFEST, &buf[0], 8);
	if(board_addr!=buf[0])
	{
		rt_kprintf("µØÖ·ÉèÖÃÊ§°Ü\n");
	}
}

uint8_t read_board_addr()
{
	uint8_t buf[8];
	buf[0]=0xff;
	stm32_flash_read(STM32_FLASH_END_ADDRESS-BOARD_ADDR_OFFEST, &buf[0], 8);
	
	if(buf[0]>9)
	{
		buf[0]=0;
	}
	return buf[0];
}
static void bkp_sram_init(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();	

    __HAL_RCC_BKPSRAM_CLK_ENABLE();

    HAL_PWR_EnableBkUpAccess();

    HAL_PWREx_EnableBkUpReg();

    HAL_PWR_DisableBkUpAccess();
}
static void bkp_sram_write(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;

    if(len > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE;
    HAL_PWR_EnableBkUpAccess();
    rt_memcpy((rt_uint8_t *)(BKPSRAM_BASE + offset),(rt_uint8_t *)data,len);
    HAL_PWR_DisableBkUpAccess();
}



static void bkp_sram_read(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;

    if(len > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE;
    rt_memcpy((rt_uint8_t *)data, (rt_uint8_t *)(BKPSRAM_BASE + offset), len);
}

INIT_DEVICE_EXPORT(bkp_sram_init);

