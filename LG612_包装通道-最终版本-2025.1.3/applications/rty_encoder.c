#include "rty_encoder.h"
#include <rtdevice.h>
#include <board.h>

#define EC11_PINA GET_PIN(E, 14)
#define EC11_PINB GET_PIN(E, 13)
#define EC11_PINC GET_PIN(E, 15)

//#if defined(PKG_USING_ENCODER_EC11)
/*
*\读取旋转编码器按键动作
*\return:键值
*/
static rt_uint8_t ec11_tag_scan(ec11_device_t ec11)
{
    static rt_uint32_t key_tag = 0x01, press_cnt = 0;
    rt_uint8_t key = 0, tag = 0;

    key = rt_pin_read(ec11->tag_pin);
	
    if(!key)
    {
        if(key_tag != key)
        {
            key_tag = key;
            press_cnt = 0;
        }
        press_cnt++;
        if(press_cnt >= 200)
            press_cnt = 201;
    }
    else
    {
        if(press_cnt >= 2)
            tag = TAG_KEY_ACTION;
        press_cnt = 0;
        key_tag = 0x01;
    }

    return tag;
}
/*
*\一定位一脉冲类型旋转编码器解析
*/
static rt_uint8_t ec11_single_scan(ec11_device_t ec11)
{
    rt_uint8_t result = 0;
    int pha_level = 0, phb_level = 0;

    pha_level = rt_pin_read(ec11->pha_pin);
    phb_level = rt_pin_read(ec11->phb_pin);

    if(pha_level != ec11->pha_last)
    {
        if(pha_level == 0)
        {
            if(phb_level != 0)
                result = LEFT_KEY_ACTION;
            else
                result = RIGHT_KEY_ACTION;
        }

        ec11->pha_last = pha_level;
        ec11->phb_last = phb_level;
    }

    return result;
}
/*
*\两定位一脉冲类型旋转编码器计息
*/
static rt_uint8_t ec11_double_scan(ec11_device_t ec11)
{
    rt_uint8_t result = 0;
    int pha_level = 0, phb_level = 0;

    pha_level = rt_pin_read(ec11->pha_pin);
    phb_level = rt_pin_read(ec11->phb_pin);

    if(pha_level != ec11->pha_last)
    {
        if(pha_level != 0)
        {
            if((ec11->phb_last != 0)&&(phb_level == 0))
                result = LEFT_KEY_ACTION;
            if((ec11->phb_last == 0)&&(phb_level != 0))
                result = RIGHT_KEY_ACTION;
            
            if((ec11->phb_last == phb_level)&&(phb_level == 0))
                result = LEFT_KEY_ACTION;

            if((ec11->phb_last == phb_level)&&(phb_level != 0))
                result = RIGHT_KEY_ACTION;
        }
        else
        {
            if((ec11->phb_last != 0)&&(phb_level == 0))
                result = RIGHT_KEY_ACTION;
            if((ec11->phb_last == 0)&&(phb_level != 0))
                result = LEFT_KEY_ACTION;
            
            if((ec11->phb_last == phb_level)&&(phb_level == 0))
                result = RIGHT_KEY_ACTION;

            if((ec11->phb_last == phb_level)&&(phb_level != 0))
                result = LEFT_KEY_ACTION;
        }

        ec11->pha_last = pha_level;
        ec11->phb_last = phb_level;
    }

    return result;
}
/*
*\旋转编码器动作解析
*/
static rt_uint8_t ec11_action_analysis(ec11_device_t ec11)
{
    rt_uint8_t result = 0;

    if(ec11->type == 0)
    {
        result = ec11_single_scan(ec11);
        if(result != 0)
            return result;
    }
    else
    {
        result = ec11_double_scan(ec11);
        if(result != 0)
            return result;
    }

    result = ec11_tag_scan(ec11);

    return result;
}
/*
*\读取编码器值
*/
rt_ubase_t ec11_result_read(ec11_device_t ec11)
{
    rt_ubase_t key = 0;

    if(rt_mb_recv(ec11->mail, &key, 100) == RT_EOK)
        return key;

    return 0;
}

extern void set_key_val(uint8_t key);
static void ec11_scan(void *parameters)
{
	rt_uint8_t key = 0;
  ec11_device_t ec11 = (ec11_device_t)parameters;
	while(1)
	{
	
		rt_thread_mdelay(10);
	  key = ec11_action_analysis(ec11);
    if(key != 0)
		{
     //   rt_mb_send_wait(ec11->mail, key, 100);
			set_key_val(key);
			//rt_kprintf("key val=%d\n",key);
		}
	}
}
/*
*\选择编码器初始化
*\pha_pin:
*\phb_pin:
*\phc_pin:
*\return:ec11设备句柄
*/
static ec11_device_t dev;
ec11_device_t ec11_device_init(rt_base_t pha, rt_base_t phb, rt_base_t tag, rt_uint8_t type)
{
   
    rt_thread_t tid;
    dev = rt_malloc(sizeof(struct ec11_device));
    if(dev == RT_NULL)
        return RT_NULL;

    dev->mail = rt_mb_create("mkey", 16, RT_IPC_FLAG_FIFO);
    if(dev->mail == RT_NULL)
    { 
        rt_free(dev);
        return RT_NULL;
    }        
    dev->pha_pin = pha;
    dev->phb_pin = phb;
    dev->tag_pin = tag;

    rt_pin_mode(dev->pha_pin, PIN_MODE_INPUT);
    rt_pin_mode(dev->phb_pin, PIN_MODE_INPUT);
    rt_pin_mode(dev->tag_pin, PIN_MODE_INPUT);

    dev->pha_last = 0;
    dev->phb_last = 0;
    dev->type = type;
		tid = rt_thread_create("ec11",ec11_scan, dev,2048,RT_THREAD_PRIORITY_MAX-3, 10);
    if (tid == RT_NULL)
		{
			return RT_NULL;
		}
		rt_thread_startup(tid);
    return dev;
}

void get_key_mail_addr(rt_mailbox_t **mail)
{
	rt_thread_mdelay(100);
	*mail = &dev->mail;
}
int ec11_init(void)
{
	ec11_device_init(EC11_PINA, EC11_PINB, EC11_PINC, 0);

}
INIT_APP_EXPORT(ec11_init);
//#endif
