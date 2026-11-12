
#include <rtthread.h>
#include <rtdevice.h>
#include "udac.h"
#include "relay.h"
#include "mdebug.h"
//#include "mail_box.h"
#define GP8403_I2C_BUS_NAME          "i2c2"  /* ??????I2C?????? */
#define GP8403_ADDR                  0x58    /* ???? */

#define MODE_ADDR                   0x01    //
#define VOTAGE_CH2_ADDR             0x02    //
#define AHT10_GET_DATA              0x02    /* ?????? */

ALIGN(RT_ALIGN_SIZE)

static struct rt_i2c_bus_device *i2c_bus = RT_NULL;     /* I2C?????? */

// 0 输出 0-5v : 1 0-10V
static rt_err_t G8403_setoutput_mode(struct rt_i2c_bus_device *bus,uint8_t mode)
{
		rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;
    buf[0] = MODE_ADDR; //cmd
	  buf[1] = (mode==0)? 0 : 0x11;
    msgs.addr = GP8403_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 2;
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}
// dat = 0-1000(10.00V) :0-10V //  精度0.01V
// 为什么10V跑的速度就是1000mm/s，这个是不是机械结构决定的
// 对于电机来说，只需要设置电机转速的电压0-10v，10v的时候按照最大转速转
// 那主控电机10v的时候也就是最大转速的时候是多少呢(实际是不是1000mm/s)？，需要看电机手册？
// 这个函数到底是想设置速度即通过速度换算为数字量
// 12Bit设置的数字量的范围只能是0x000-0xFFF即0-4096？
// 但是如设置1000，这个数字量就是78939？可行吗？
// 这个设置的值是0-1000mm/s吗？那么这就涉及到电机转速(不是电机转速1440转每分钟，是指电机带动分级线移动的速度)为1000mm/s的时候，设置的电压是多少也即设置的数字量是多少
// 6552,83是什么意思,是不是就涉及到中间的换算
static rt_err_t G8403_set_votage(uint16_t dat)
{ 
	  uint32_t votage=0;
		rt_uint8_t buf[3];
    struct rt_i2c_msg msgs;

	  if(dat>1000) return RT_ERROR;
		// 把速度(0-1000)转换为电压(因为主电机的速度(0-1000)是通过电压来设置的，电压为0-10V)
		// 那么每mm/s的速度对应电压为0.01V
		// GP8403 A0 A1都接地了, A2 悬空
		// 12Bit数字量0x000-0xFFF线性转换成两路独立的0-5V或者0-10V模拟电压
		// 用的是0-10v的模式，也就是0x000-0xFFF对应0-10V模拟电压，那么1V对应409,1对应0.0024V
		
		// 变频器用的是0-5v的模式，也就是0x000-0xFFF对应0-5V模拟电压，那么1V对应819.2,1对应0.0012V
	
		// 速度要转换为跑该速度的电压
		// 跑该速度的电压要转换为0x000-0xFFF的数字量并设置进去，然后就输出该电压
		// dat*0.01v=这么多电压
		// 这么多电压*409=这么多数字
		// 6552,83是什么意思
	
		// 是因为中间因为有中间齿轮(为了到达精确的1000mm/s)，所以改变这些值，
	
		// 正常的是 电压*(4096/10v)得到要填的数字
	
		// 速度乘以每个速度6552得到要设置的电压
		// 要设置的电压转换为数字：电压*(4096/10v)，但是这里是除以83，这个83是因为中间齿轮的原因？
	  votage = dat*6552;            // 655为 0.01V  dat扩大了10倍，就是65520 那就是655就是0.01？
	  votage = votage/83;	// votage = votage/72;不应该是除以72吗？他有些程序写的是votage = votage/71

		// GP8401 是12位，将这个12位数据放在data_low data_high,并且data_low的低4位无效，那么有效的就是data_high+data_low的高4位 
		// 6552/16=409.5(低4位不要就要将数据左移4位即扩大16)
		// 本身在设置这个=速度的时候又乘以了10
		// 即我要设置为4095，实际设置的值扩大了16倍即4095*16=65520
		
		// 65520/81.9=800
		// 50hz扩大16倍就是800,4096扩大16倍就是65520
		
		// 变频器速度	0 - 50hz
		// GP8401			0 - 4096
		// 4096/50=81.92,就是上面的83？

		// 鸡蛋速度 	0 - 36000个/时
		// 变频器速度	0 - 50hz，但是是不是虚拟的36hz就转换为实际的50hz？
		// 电压				0 - 5v
		// 传输线速度	0 - 635mm/s
		// GP8401			0 - 4095(65520)实际设置的是扩大了16倍的数据65520
		
		
		
		// 按照他的算,比如是要设置为635mm/s
		// 635/17.6*10*6552/83=28481，把这个值设到GP8403中
		
		// 
		
		// 正常逻辑算
		// 635/12.7得到这么多hz
		// 4095/50hz得到1hz是81.9
		// 635/12.7*81.9得到应该设置的值
		// 635/12.7*81.9*16得到要设置的值(GP8403要扩大16倍)
		// 635/12.7*81.9*16/2=32760得到0-5v的值，他程序设置的是0-10v，而变频器是0-5v
		
		// 正常逻辑算
		// 635/17.6得到这么多hz
		// 4095/50hz得到1hz是81.9
		// 635/17.6*81.9得到应该设置的值
		// 635/17.6*81.9*16得到要设置的值(GP8403要扩大16倍)
		// (635/17.6*81.9*16)/2=23639得到0-5v的值，他程序设置的是0-10v，而变频器是0-5v
		
    buf[0] = VOTAGE_CH2_ADDR; //	这个是操作OUT0输出的地址：02。原理图上也是OUT0
	  buf[1] = votage&0x00ff;
	  buf[2] = votage>>8;
    msgs.addr = GP8403_ADDR;
    msgs.flags = RT_I2C_WR;
    msgs.buf = buf;
    msgs.len = 3;
    if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1)
    {
       return RT_EOK;
    }
    else
    {
//			  DEBUG_ERR("SET SPEED ERR speed=%d\n",dat/10);
        return RT_ERROR;
    }
}


static uint16_t moto_speed=0;
uint16_t get_moto_speed()
{
  uint8_t speed;
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	speed = moto_speed;
	rt_hw_interrupt_enable(level);
 return speed;
}

//设置主电机速度  0-100

// 电机的速度是0-100
// 但是设置的电压是可以0-1000
// 相当于传入的主电机速度要乘以10？
rt_err_t set_main_moto_speed(uint16_t speed)  
{
	rt_err_t result;
	rt_base_t level;

	if(speed>0)
	{
	  frequency_moto_enable();
	}
	else 
	{
	  frequency_moto_disable();
	}
  result =  G8403_set_votage(speed);
	if(result == RT_EOK)
	{
	  level = rt_hw_interrupt_disable();
	  moto_speed =speed;
	  rt_hw_interrupt_enable(level);
	}
	return result;
}
static rt_err_t read_regs(struct rt_i2c_bus_device *bus, rt_uint8_t len, rt_uint8_t *buf)
{
    struct rt_i2c_msg msgs;
    msgs.addr = GP8403_ADDR;
    msgs.flags = RT_I2C_RD;
    msgs.buf = buf;
    msgs.len = len;
    if (rt_i2c_transfer(bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return RT_ERROR;
    }
}



rt_err_t G8403_init()
{
    rt_uint8_t temp[2] = {0xff, 0xff};

		i2c_bus = (struct rt_i2c_bus_device *)rt_device_find(GP8403_I2C_BUS_NAME);
    if (i2c_bus == RT_NULL)
    {
        rt_kprintf("can't find %s device!\n", GP8403_I2C_BUS_NAME);
    }
    else
    {
        G8403_setoutput_mode(i2c_bus,1);    //0-10V模式
			  rt_thread_mdelay(10);
			  set_main_moto_speed(0);
    }
}

