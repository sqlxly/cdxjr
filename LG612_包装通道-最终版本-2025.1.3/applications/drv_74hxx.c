#include "drv_74hxx.h"
#include <rtdevice.h>
#include <board.h>




static void stm32_udelay(rt_uint32_t us)
{
    rt_uint32_t ticks;
    rt_uint32_t told, tnow, tcnt = 0;
    rt_uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

void sn74hcxx_get_dat(uint8_t *rx_dat,uint8_t len)
{
	 uint8_t dat=0;
	 int8_t i,j;
	rt_pin_write(SN74HCXX_LD,PIN_HIGH);
	stm32_udelay(10);
	for(j=len-1;j>=0;j--)
	{
		dat =0;
		for(i=0;i<8;i++)
	 {
		 dat =dat<<1;
		 rt_pin_write(SN74HCXX_SCL, PIN_LOW);
		 stm32_udelay(1);
		 dat += (rt_pin_read(SN74HCXX_DAT)&0x01);
		 rt_pin_write(SN74HCXX_SCL, PIN_HIGH);
	 }
	 rx_dat[j]= dat;
 }
	rt_pin_write(SN74HCXX_LD,PIN_LOW);
//  dat=rx_dat[0];
 // rx_dat[0]= rx_dat[2];
//  rx_dat[2]= dat;
}

void device_75hcxx_init(void *args)
{
	 rt_pin_mode(SN74HCXX_SCL, PIN_MODE_OUTPUT);
   rt_pin_mode(SN74HCXX_DAT, PIN_MODE_INPUT);
   rt_pin_mode(SN74HCXX_LD, PIN_MODE_OUTPUT);

}
INIT_DEVICE_EXPORT(device_75hcxx_init);
//#endif
