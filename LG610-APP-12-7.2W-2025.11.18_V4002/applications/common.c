
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "common.h"
#include "mdebug.h"
#include <mail_box.h>
#include <stm32h743xx.h>

ALIGN(RT_ALIGN_SIZE)

                            //0,1,2,3,4,5,6,7,8,9,10,11
uint8_t const packge_map_ch_id[12]={0,1,2,3,4,5,6,7,8,9,10,11};

#define CFG_BKP_SRAM_SIZE       (4096)      //bkp_sram区域的大小
static uint8_t system_state = SYSTEM_IS_IDLE;
uint16_t SEPARATE_CHA_CUP_NUMBER = 270;
uint16_t SEPARATE_CHB_CUP_NUMBER = 270;
/*
*\初始化BKP_SRAM
*/
int bkp_sram_init(void)
{

    __HAL_RCC_BKPRAM_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    HAL_PWREx_EnableBkUpReg();
    HAL_PWR_DisableBkUpAccess();
}
/*
*\向BKP_SRAM写入数据
*\offset:写入地址相对BKP_SRAM起始地址的偏移量
*\data:数据指针
*\len:要写入数据的长度
*/
void bkp_sram_write(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
	rt_base_t level;
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;
		
		
	
    level = rt_hw_interrupt_disable();
    if(len + offset > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE - offset;
    memcpy((rt_uint8_t *)(D3_BKPSRAM_BASE + offset),(rt_uint8_t *)data,len);
		rt_hw_interrupt_enable(level);
}

void bkp_sram_write_NO_INT(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{

    if(offset >= CFG_BKP_SRAM_SIZE)
        return;
    if(len + offset > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE - offset;
    memcpy((rt_uint8_t *)(D3_BKPSRAM_BASE + offset),(rt_uint8_t *)data,len);
}
/*
*\从BKP_SRAM读取数据
*\offset:读取地址相对BKP_SRAM起始地址的偏移量
*\data:数据指针
*\len:要读取数据的长度
*/
void bkp_sram_read(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;
    if(len + offset > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE - offset;
    memcpy((rt_uint8_t *)data, (rt_uint8_t *)(D3_BKPSRAM_BASE + offset), len);
}

uint8_t get_system_state()
{
  uint8_t state;
	rt_base_t level;
	
  level = rt_hw_interrupt_disable();
  state = system_state;
	rt_hw_interrupt_enable(level);
	return state;
}

void  set_system_state(uint8_t state)
{
	rt_base_t level;
  level = rt_hw_interrupt_disable();
  system_state = state;
	rt_hw_interrupt_enable(level);
}

//包装通道 0-5
void send_message_to_packge_device(uint8_t ch,uint8_t *buf)
{
 
	 rt_err_t result;			
	 switch(ch)
	 {
			case 0:
			case 1:
				result= rt_mq_send(&masseg_p1, buf, MESSAGE_DAT_LEN);
        break;
			case 2:
			case 3:
				result= rt_mq_send(&masseg_p2, buf, MESSAGE_DAT_LEN);
        break;
			case 4:
			case 5:
				result= rt_mq_send(&masseg_p3, buf, MESSAGE_DAT_LEN);
        break;
			case 6:
			case 7:
				result= rt_mq_send(&masseg_p4, buf, MESSAGE_DAT_LEN);
        break;
			case 8:
			case 9:
				result= rt_mq_send(&masseg_p5, buf, MESSAGE_DAT_LEN);
        break;
			case 10:
			case 11:
				result= rt_mq_send(&masseg_p6, buf, MESSAGE_DAT_LEN);
        break;
			defult:
				break;
	 }				
	 if (result != RT_EOK)
   {
    // DEBUG_ERR("rt_send_message ERR ch=%d result=%d\n",ch,result);
   }
}

rt_tick_t count_use_time(rt_tick_t start_time)
{
	rt_tick_t ctime,use_time;
	
	ctime = rt_tick_get();
	if(ctime>=start_time)
	{
		use_time = ctime - start_time;
	}
	else 
	{
		use_time = (0xffffffff - start_time) + ctime;
	}
	return use_time;
}
