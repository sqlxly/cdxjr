
#include <rtthread.h>
#include <rtdevice.h>
#include <relay.h>
#include "drv_key.h"
#include "uadc.h"
#define ADC_DEV_NAME        "adc1"      
#define ADC_DEV_CHANNEL     18           
#define REFER_VOLTAGE       330         
#define CONVERT_BITS        (1 << 12)  //转换精度 12位

#define ADC_BUF_LEN 100     //5ms读一次   50ms 更新一下AD值
#define ADC_FILTER_LEN 20   //比 ADC_BUF_LEN 小

ALIGN(RT_ALIGN_SIZE)



float get_adc_votage()
{
	return 1.0;
	// return adc_votage;
}
/*
static rt_adc_device_t adc_dev;
static uint32_t arr[ADC_BUF_LEN];
static float adc_votage=0;

int sort_arry()
{
	for (int i = 0; i < (ADC_BUF_LEN-1); i++)
	{
		for (int j = 0; j < (ADC_BUF_LEN-1)-i; j++)
		{
			if (arr[j] > arr[j + 1])//????????
			{
				uint32_t temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
		}
	}
	return 0;
}
void adc_main()
{
	float votage;
	rt_uint32_t value=0;
	rt_base_t level;
	rt_tick_t cycle_time=0;
	
	
	while(1)
	{
		cycle_time = rt_tick_get();
    value =0;
		for(int i=0;i<ADC_BUF_LEN;i++)
		{
			arr[i]= rt_adc_read1(adc_dev, ADC_DEV_CHANNEL);
			rt_thread_mdelay(1);
		}
		sort_arry();
		for(int i=ADC_FILTER_LEN;i<(ADC_BUF_LEN-ADC_FILTER_LEN);i++)
		{
			value += arr[i];
		}
		value =  value/(ADC_BUF_LEN-(ADC_FILTER_LEN*2));
		votage = (value * REFER_VOLTAGE) / CONVERT_BITS;
		level = rt_hw_interrupt_disable();
		adc_votage =  votage/100.0;
		rt_hw_interrupt_enable(level);	
//	  rt_kprintf("adc votage=%d cycle time=%d min=%d max=%d\n",(uint16_t)(adc_votage*100),rt_tick_get()-cycle_time,arr[0],arr[19]);
	}
}
rt_err_t adc_init()
{
	rt_thread_t  tid1;
	rt_err_t ret = RT_EOK;
	adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
  if (adc_dev == RT_NULL)
  {
    rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
    return RT_ERROR;
  }
	ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);
	rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
	tid1 = rt_thread_create("adc",adc_main, RT_NULL,1024,20, 10);
  if (tid1 == RT_NULL)
		return RT_ERROR;
	rt_thread_startup(tid1);
	return ret;
}
*/