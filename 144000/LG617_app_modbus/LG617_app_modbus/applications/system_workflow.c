#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "ledblink.h"
#include "./tools/common.h"

#define LED_RUN GET_PIN(D, 2)

#define SYNC_PULSE_PORT 	77

static struct rt_thread _thread;                                 
static uint8_t _thread_stack[1024]; 

extern int calc_egg_weight(double *pweight, int ch);
extern void clear_dynamic_zero_vector(void);
extern void fdcan_submit_work(rt_ubase_t cmd);
extern uint16_t getModbusHoldingRegEx(uint16_t addr);

extern int32_t encoder_pulse;

extern uint8_t calib_flag;																						//进行过称重校准的标志

static rt_uint8_t cnt1 = 0, cnt2 = 0;

static void system_workflow_thread_entry(void *parameter)
{
		uint16_t status = 0;
		int pin_sta = 0, pin_sta_bak = 0;
	
		led_add_device( LED_RUN );
		led_blink(1, 400, 200);
		
    while (1)
		{	
			pin_sta = rt_pin_read(SYNC_PULSE_PORT);	
//			encoder_pulse = 200;
			if((0 == pin_sta) && (1 == pin_sta_bak) && (encoder_pulse > 200))  //检查同步脉冲的下降沿
			{
				encoder_pulse = 0;
				clear_dynamic_zero_vector();
				
				fdcan_submit_work(80	02);
				
//   			rt_kprintf("[sync pulse click event]\r\n");	
			}
			pin_sta_bak = pin_sta;

			if(cnt1 == 5)  														//每60ms读取一次编码器的位置并发送给信号处理板
			{
				fdcan_submit_work(8001);
			}
			if(cnt1 == 3){
				status = getModbusHoldingRegEx(1601);
				if((status == 4)||(status == 6)){				//在校准状态下 每6个周期读取一次传感器校准值
					fdcan_submit_work(8007);
				}
			}
			cnt1 = (cnt1 + 1) % 6;

			if(cnt2 == 99)														//每秒钟计算一次称重校准页面参数
			{
				fdcan_submit_work(8003);
			}
			cnt2 = (cnt2 + 1) % 100;	

			led_process( 10 );
			
			rt_thread_mdelay(10);
    }
}

int system_workflow_init(void)
{
		rt_pin_mode(SYNC_PULSE_PORT, PIN_MODE_INPUT);
	
    rt_thread_init(&_thread,
                   "agbtn",
                   system_workflow_thread_entry,
                   RT_NULL,
                   &_thread_stack[0],
                   sizeof(_thread_stack),
                   6,
                   100);

    rt_thread_startup(&_thread);

    return RT_EOK;
}
