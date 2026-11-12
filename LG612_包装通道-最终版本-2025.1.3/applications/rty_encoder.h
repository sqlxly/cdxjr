#ifndef __RTY_ENCODER_H_
#define __RTY_ENCODER_H_

#include <rtthread.h>
#include <rtdevice.h>


#define TAG_KEY_ACTION      (0x01)
#define LEFT_KEY_ACTION     (0x02)
#define RIGHT_KEY_ACTION    (0x03)

#pragma pack(1)
//编码器信息
struct ec11_device
{
    rt_mailbox_t mail;
    rt_timer_t stimer;

    rt_base_t pha_pin;
    rt_base_t phb_pin;
    rt_base_t tag_pin;

    rt_uint8_t pha_last;
    rt_uint8_t phb_last;
    rt_uint8_t type;        //编码器类型，0：一定位一脉冲，1：两定位一脉冲
};
typedef struct ec11_device *ec11_device_t;

#pragma pack()


ec11_device_t ec11_device_init(rt_base_t pha, rt_base_t phb, rt_base_t tag, rt_uint8_t type);

rt_ubase_t ec11_result_read(ec11_device_t ec11);

#endif 
