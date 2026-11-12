#include "user_interface.h"
#include "ecat_def.h"
#if EL9800_HW
#include "user_interface.h"
#include "ecatslv.h"

#define    _EL9800HW_ 1
#include "el9800hw.h"
#undef    _EL9800HW_
/* ECATCHANGE_START(V5.11) ECAT10*/
/*remove definition of _EL9800HW_ (#ifdef is used in el9800hw.h)*/
/* ECATCHANGE_END(V5.11) ECAT10*/

#include "ecatappl.h"

#include <drv_gpio.h>

#define HWTIMER_DEV_NAME   "timer13"

SPI_HandleTypeDef hspi1;
TIM_HandleTypeDef TimHandle;

void EcatIsr(void *args);
void Sync0Isr(void *args);
void Sync1Isr(void *args);
rt_err_t  TimerIsr(rt_device_t dev, rt_size_t size);

void spi_config(void)
{
	rt_base_t pin_num;
	
	pin_num = GET_PIN(A, 4);
	
	rt_pin_mode(pin_num, PIN_MODE_OUTPUT);
	
  /*##-1- Configure the SPI peripheral #######################################*/
  /* Set the SPI parameters */
  hspi1.Instance               = SPI1;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4; //168/4=42M  42/2=21M
  hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
  hspi1.Init.CLKPhase          = SPI_PHASE_2EDGE;
  hspi1.Init.CLKPolarity       = SPI_POLARITY_HIGH; 
  hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial     = 7;
  hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
  hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  hspi1.Init.NSS               = SPI_NSS_SOFT;
  hspi1.Init.TIMode            = SPI_TIMODE_DISABLE;
  hspi1.Init.Mode 						 = SPI_MODE_MASTER;
  
  if(HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    /* Initialization Error */
    while(1) {}
  }
}

uint8_t WR_CMD(uint8_t cmd) 
{
	uint8_t RxData;
	
  HAL_SPI_TransmitReceive(&hspi1, &cmd, &RxData, 1, 100);
	
  return RxData;
} 

/*******************************************************************************
* Function Name  : TIM_Configuration
* Description    : TIM_Configuration program.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void TIM_Configuration(int32_t period)		//1ms定时器
{
    rt_err_t ret = RT_EOK;
    rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
    rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */
    rt_hwtimer_mode_t mode;         /* 定时器模式 */
    rt_uint32_t freq = 10000;       /* 计数频率 */

    hw_dev = rt_device_find(HWTIMER_DEV_NAME);
    if (hw_dev == RT_NULL)
    {
        rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
    }

    ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
    if (ret != RT_EOK)
    {
        rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
    }

    rt_device_set_rx_indicate(hw_dev, TimerIsr);

    rt_device_control(hw_dev, HWTIMER_CTRL_FREQ_SET, &freq);
    mode = HWTIMER_MODE_PERIOD;
    ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
    if (ret != RT_EOK)
    {
        rt_kprintf("set mode failed! ret is :%d\n", ret);
    }

    timeout_s.sec = 0;   
    timeout_s.usec = period;    
    if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
    {
        rt_kprintf("set timeout value failed\n");
    }
}

// SPI_PDI ISR
void EXTI1_Configuration(void)
{
	rt_base_t pin_num;
	
	pin_num = GET_PIN(A, 1);
	
  rt_pin_mode(pin_num, PIN_MODE_INPUT_PULLUP);
	
	rt_pin_attach_irq(pin_num, PIN_IRQ_MODE_FALLING, EcatIsr, RT_NULL);
	
	rt_pin_irq_enable(pin_num, PIN_IRQ_ENABLE);
}

//SYNC0
void EXTI2_Configuration(void)
{
	rt_base_t pin_num;
	
	pin_num = GET_PIN(A, 3);
	
  rt_pin_mode(pin_num, PIN_MODE_INPUT_PULLUP);
	
	rt_pin_attach_irq(pin_num, PIN_IRQ_MODE_FALLING, Sync0Isr, RT_NULL);
	
	rt_pin_irq_enable(pin_num, PIN_IRQ_ENABLE);
}

//SYNC1
void EXTI3_Configuration(void)
{
	rt_base_t pin_num;
	
	pin_num = GET_PIN(A, 2);
	
  rt_pin_mode(pin_num, PIN_MODE_INPUT_PULLUP);
	
	rt_pin_attach_irq(pin_num, PIN_IRQ_MODE_FALLING, Sync1Isr, RT_NULL);
	
	rt_pin_irq_enable(pin_num, PIN_IRQ_ENABLE);
}

void EcatIsr(void *args)		//void __attribute__ ((__interrupt__, no_auto_psv)) EscIsr(void)
{		
    PDI_Isr();
}

void Sync0Isr(void *args)
{
    Sync0_Isr();
}

void Sync1Isr(void *args)
{	
		Sync1_Isr();
}

// Timer 2 ISR (1ms)
rt_err_t  TimerIsr(rt_device_t dev, rt_size_t size)
{
		ECAT_CheckTimer();
	
		return RT_EOK;
}

#endif //#if EL9800_HW
