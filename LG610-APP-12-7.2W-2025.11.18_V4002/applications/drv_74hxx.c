
#include <rtdevice.h>
#include <board.h>
#include "drv_74hcxx.h"
#include "hal_delay.h"

#define SN74HCXX_SCL GET_PIN(B, 2)
#define SN74HCXX_DAT GET_PIN(E, 7)
#define SN74HCXX_LD  GET_PIN(A, 6)




void sn74hcxx_get_dat(uint8_t *rx_dat,uint8_t len)
{
	uint8_t dat=0;
	uint8_t i,j;
	
	rt_pin_write(SN74HCXX_LD,PIN_HIGH);
	for(j=len;j>0;j--)
	{
		dat =0;
		for(i=0;i<8;i++)
	 {
		 dat =dat<<1;
		 rt_pin_write(SN74HCXX_SCL, PIN_LOW);
		// hal_udelay(1);
		 rt_pin_read(SN74HCXX_DAT);  //ÉÔÎ¢µÈ´ý
		 dat += (rt_pin_read(SN74HCXX_DAT)&0x01);
		 rt_pin_write(SN74HCXX_SCL, PIN_HIGH);
		// hal_udelay(1);
	 }
	  rx_dat[j-1]= dat;
 }
	rt_pin_write(SN74HCXX_LD,PIN_LOW);
}

int device_75hcxx_init(void)
{
	 rt_pin_mode(SN74HCXX_SCL, PIN_MODE_OUTPUT);
   rt_pin_mode(SN74HCXX_DAT, PIN_MODE_INPUT);
   rt_pin_mode(SN74HCXX_LD, PIN_MODE_OUTPUT);
}
INIT_DEVICE_EXPORT(device_75hcxx_init);
//#endif
