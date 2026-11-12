#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <rthw.h>
#include "drv_hx71708.h"
#include "./tools/common.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_it.h"

/* Timer for ADC trigger parameters */
#define TIMER_FREQUENCY                ((uint32_t) 200)    /* Timer frequency (unit: 200Hz). With a timer 16 bits and time base freq min 1Hz, range is min=1Hz, max=32kHz. */
#define TIMER_FREQUENCY_RANGE_MIN      ((uint32_t)    1)    /* Timer minimum frequency (unit: Hz), used to calculate frequency range. With a timer 16 bits, maximum frequency will be 32000 times this value. */
#define TIMER_PRESCALER_MAX_VALUE      (0xFFFF-1)           /* Timer prescaler maximum value (0xFFFF for a timer 16 bits) */

#define RATE_STEP		4		//1:10Hz; 2:20Hz; 3:80Hz; 4:320Hz

#define	CHANNEL_NUM	6		//6路HX71708称重传感器

// 时钟线定义
#define CLK_1 			18	//GET_PIN(B,2)
#define CLK_2 			0		//GET_PIN(A,0)
#define CLK_3 			72	//GET_PIN(E,8)
#define CLK_4 			65	//GET_PIN(E,1)
#define CLK_5 			25	//GET_PIN(B,9)
#define CLK_6 			24	//GET_PIN(B,8)

//数据线定义
#define DOUT_1 			17	//GET_PIN(B,1)
#define DOUT_2 			16	//GET_PIN(B,0)
#define DOUT_3 			6		//GET_PIN(A,6)
#define DOUT_4 			5		//GET_PIN(A,5)
#define DOUT_5 			4		//GET_PIN(A,4)
#define DOUT_6 			3		//GET_PIN(A,3)

#define		SCLK(P, L)	rt_pin_write(P, L)
#define		DOUT(P)			rt_pin_read(P)		// 读怎么是DOUT，DIN不是更好理解吗？

const rt_base_t sclk_map[WEIGH_CHANNEL] = {18, 0, 72, 65, 25, 24};
const rt_base_t dout_map[WEIGH_CHANNEL] = {17, 16, 6, 5, 4, 3};	// 读数据，din_map不是更好理解吗？

extern uint8_t adcSensorErr[6];

extern void app_mailbox_send(uint32_t adc);

static uint8_t exp_mark = 0;
static uint8_t isr_flag[CHANNEL_NUM] = {0};

static rt_uint32_t HX71708_Read(const rt_uint8_t n);

void HX71708_Reset(void)
{
	int i, n = 100;

	//拉高SCK超过100us再拉低 ADC芯片复位
	for(i=0; i<CHANNEL_NUM; i++)
	{
		SCLK(sclk_map[i], PIN_HIGH);
	}
	
	rt_hw_us_delay(200);
	
	for(i=0; i<CHANNEL_NUM; i++)
	{
		SCLK(sclk_map[i], PIN_LOW);
	}

	//复位后需要等待至少4个数据周期
	rt_hw_us_delay(200);
	
	for(i=0; i<CHANNEL_NUM; i++)
	{
		HX71708_Read(i);			// 每个通道读了以后sclk_map会是一直是低电平吗？
	}	

	while(n--){
		//检查HX71708数据输出端口电平状态
		rt_thread_mdelay(20);
		if(DOUT(sclk_map[0]) == 1){
			continue;
		}else if(DOUT(sclk_map[1]) == 1){
			continue;
		}else if(DOUT(sclk_map[2]) == 1){
			continue;
		}else if(DOUT(sclk_map[3]) == 1){
			continue;
		}else if(DOUT(sclk_map[4]) == 1){
			continue;
		}else if(DOUT(sclk_map[5]) == 1){
			continue;
		}else{
			break;
		}
	}
}


// 主循环会一直检测这个
// 原理：
// exp_mark 这个会在中断中清零，如果在中断中清零了，是不是这里就应该读不到高电平了，或者不总是读到高电平了
// 如果连续调用3次这个函数，都读到是高电平是不是就认为异常了
// 那这个函数多久执行一次呢？
// 那中断多久执行一次，然后清零呢？
int HX71708_exception_handler(void)
{
	int ch = 0, ret = 0;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		if(getbitv(exp_mark, ch) == 1){
			SCLK(sclk_map[ch], PIN_HIGH);
			rt_hw_us_delay(200);
			SCLK(sclk_map[ch], PIN_LOW);
			HX71708_Read(ch);
			rt_kprintf("%d_error!\n", ch+1);
			ret = 1;
			adcSensorErr[ch] += 1;
			if(adcSensorErr[ch] >= 3){ //称重采集芯片故障报警
				adcSensorErr[ch] = 3;
			}
		}else{
			adcSensorErr[ch] = 0;
		}
		setbitv(exp_mark, ch);
	}
	
	return ret;
}

static rt_uint32_t HX71708_Read(const rt_uint8_t n)
{
	rt_uint32_t bcd = 0;
	int i = 0;

		// 读数据流程
		// 判断是否有数据
		// 芯片的DOUT 脚为高电平的时候，表明数据还没有准备好，单片机检测到变为低电平了则表示有数据了，进入中断读取数据
		// 芯片的DOUT 脚为高电平的时候，此时单片机给芯片的时钟SCK应为低电平，也就是平时时钟应该处于低电平
		// 当数据准备好了，则SCK应该输入25-28个时钟脉冲，其中第1个时钟的上升沿读出第一个数据，一直到第24个数据
		// 第25-28个脉冲用来选择下一次AD转换输出的速率，只输入第25个则表示10HZ,输入第25,26个则表示20HZ,输入第25,26,27个则表示80HZ,输入第25,26,27,28个则表示320HZ,
	for(i=0;i<24;i++)
	{
		SCLK(sclk_map[n], PIN_HIGH); // 高电平小于50uS
		__NOP();__NOP();__NOP();			// 但是这个函数在中断里面调用了，是不是不能用rt_hw_us_delay()这个函数了？所以用这种延迟方式
		SCLK(sclk_map[n], PIN_LOW);
		bcd = bcd<<1;
		if(DOUT(dout_map[n]) == 1) bcd++;
	}

	for (i=0;i<RATE_STEP;i++) 		// N=1:10Hz;N=2:20Hz;N=3:80Hz;N=4:320Hz
	{
		SCLK(sclk_map[n], PIN_HIGH);
		__NOP();__NOP();__NOP();
		SCLK(sclk_map[n], PIN_LOW);
		__NOP();__NOP();__NOP();
	}

	bcd = bcd^0x800000; // 有符号变成无符号数
	
	bcd = bcd >> 5;			// 右移5位（丢弃低5位）
	
	bcd = bcd << 5;			// 左移5位（低5位补0）
	
	bcd = bcd | n;			// 将通道号编码到低5位

	return bcd;
}

static void balance_1_read_enable_irq(void *args)
{
	uint32_t adc = 0;
	
	if(isr_flag[0] == 1)
	{
		rt_pin_irq_enable(dout_map[0], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[0], PIN_MODE_INPUT);				// 为什么要配置成输入呢，不是已经是输入了吗？
																										// 是因为这个也是数据线的原因？
		adc = HX71708_Read(0);
		
		clrbitv(exp_mark, 0);
		
		rt_pin_irq_enable(dout_map[0], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[0] = (isr_flag[0] + 1) % 2;
}

static void balance_2_read_enable_irq(void *args)
{
	uint32_t adc = 0;

	if(isr_flag[1] == 1)
	{	
		rt_pin_irq_enable(dout_map[1], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[1], PIN_MODE_INPUT);
	
		adc = HX71708_Read(1);
		
		clrbitv(exp_mark, 1);
		
		rt_pin_irq_enable(dout_map[1], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[1] = (isr_flag[1] + 1) % 2;
}

static void balance_3_read_enable_irq(void *args)
{
	uint32_t adc = 0;

	if(isr_flag[2] == 1)
	{	
		rt_pin_irq_enable(dout_map[2], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[2], PIN_MODE_INPUT);
	
		adc = HX71708_Read(2);
		
		clrbitv(exp_mark, 2);
		
		rt_pin_irq_enable(dout_map[2], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[2] = (isr_flag[2] + 1) % 2;
}

static void balance_4_read_enable_irq(void *args)
{
	uint32_t adc = 0;

	if(isr_flag[3] == 1)
	{	
		rt_pin_irq_enable(dout_map[3], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[3], PIN_MODE_INPUT);
	
		adc = HX71708_Read(3);
		
		clrbitv(exp_mark, 3);
		
		rt_pin_irq_enable(dout_map[3], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[3] = (isr_flag[3] + 1) % 2;
}

static void balance_5_read_enable_irq(void *args)
{
	uint32_t adc = 0;

	if(isr_flag[4] == 1)
	{	
		rt_pin_irq_enable(dout_map[4], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[4], PIN_MODE_INPUT);
	
		adc = HX71708_Read(4);
		
		clrbitv(exp_mark, 4);
		
		rt_pin_irq_enable(dout_map[4], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[4] = (isr_flag[4] + 1) % 2;
}

static void balance_6_read_enable_irq(void *args)
{
	uint32_t adc = 0;

	if(isr_flag[5] == 1)
	{	
		rt_pin_irq_enable(dout_map[5], PIN_IRQ_DISABLE);
		
		rt_pin_mode(dout_map[5], PIN_MODE_INPUT);
	
		adc = HX71708_Read(5);
		
		clrbitv(exp_mark, 5);
		
		rt_pin_irq_enable(dout_map[5], PIN_IRQ_ENABLE);
		
		app_mailbox_send(adc);
	}
	
	isr_flag[5] = (isr_flag[5] + 1) % 2;
}

// 时钟线引脚和数据线引脚的配置
void hx71708_gpio_config(void)
{
	rt_uint8_t i = 0;
	
	for(i=0; i<CHANNEL_NUM; i++){
		rt_pin_mode(sclk_map[i], PIN_MODE_OUTPUT_PULLUP); 	//时钟线配置为输出
		rt_pin_write(sclk_map[i], PIN_LOW); 								//SCLK为低电平时芯片处于正常工作状态
		rt_pin_mode(dout_map[i], PIN_MODE_INPUT);
	}

	//复位HX71708芯片
	HX71708_Reset();	
}

int hx71708_auto_work(void)
{
	rt_uint8_t i = 0;
	
	void(*func[CHANNEL_NUM])(void *) = { balance_1_read_enable_irq, balance_2_read_enable_irq, balance_3_read_enable_irq, balance_4_read_enable_irq,\
																			 balance_5_read_enable_irq, balance_6_read_enable_irq };
	
	//延时100ms等待HX71708芯片启动完成
	rt_thread_mdelay(100);

	for(i=0; i<CHANNEL_NUM; i++){
		rt_pin_attach_irq(dout_map[i], PIN_IRQ_MODE_FALLING, func[i], RT_NULL);		// 配置gpio引脚为中断模式，中断回调函数
		rt_pin_irq_enable(dout_map[i], PIN_IRQ_ENABLE);														// 使能该gpio中断
	}
	
	return RT_EOK;
}
