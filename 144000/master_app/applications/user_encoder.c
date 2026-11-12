#include "user_encoder.h"

#define PULSE_ENCODER1_DEV_NAME    "pulse4"    /* Âö³å±àÂëÆ÷1Ãû³Æ */

#define PULSE_ENCODER2_DEV_NAME    "pulse2"    /* Âö³å±àÂëÆ÷2Ãû³Æ */

#define PULSE_ENCODER3_DEV_NAME    "pulse5"    /* Âö³å±àÂëÆ÷3Ãû³Æ */

rt_device_t firstEncoderDev = RT_NULL;   		 
rt_device_t secondEncoderDev = RT_NULL;   	
rt_device_t thirdEncoderDev = RT_NULL;  

/************±àÂëÆ÷1½Ó¿Úº¯Êý*************/
int firstPulseEncoderInit(void)
{
	rt_err_t ret = RT_EOK;
	
	firstEncoderDev = rt_device_find(PULSE_ENCODER1_DEV_NAME);
	if (firstEncoderDev == RT_NULL)
	{
		rt_kprintf("pulse encoder sample run failed! can't find %s device!\n", PULSE_ENCODER1_DEV_NAME);
    return RT_ERROR;
	}
	
  ret = rt_device_open(firstEncoderDev, RT_DEVICE_OFLAG_RDONLY);
  if (ret != RT_EOK)
  {
      rt_kprintf("open %s device failed!\n", PULSE_ENCODER1_DEV_NAME);
      return ret;
  }	
	
	return RT_EOK;
}

int16_t firstPulseEncoderGet(void)
{
	int32_t count = 0;
	rt_device_read(firstEncoderDev, 0, &count, 1);
	return (int16_t)count;
}

void firstPulseEncoderClear(void)
{
	if(firstEncoderDev == RT_NULL) return;
	
	rt_device_control(firstEncoderDev, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);
}

void firstPulseEncoderClose(void)
{
	rt_device_close(firstEncoderDev);
}

/************±àÂëÆ÷2½Ó¿Úº¯Êý*************/
int secondPulseEncoderInit(void)
{
	rt_err_t ret = RT_EOK;
	
	secondEncoderDev = rt_device_find(PULSE_ENCODER2_DEV_NAME);
	if (secondEncoderDev == RT_NULL)
	{
		rt_kprintf("pulse encoder sample run failed! can't find %s device!\n", PULSE_ENCODER2_DEV_NAME);
    return RT_ERROR;
	}	
	
  ret = rt_device_open(secondEncoderDev, RT_DEVICE_OFLAG_RDONLY);
  if (ret != RT_EOK)
  {
      rt_kprintf("open %s device failed!\n", PULSE_ENCODER2_DEV_NAME);
      return ret;
  }
	
	return RT_EOK;
}

int16_t secondPulseEncoderGet(void)
{
	int32_t count = 0;
	rt_device_read(secondEncoderDev, 0, &count, 1);
	return (int16_t)count;
}

void secondPulseEncoderClear(void)
{
	if(secondEncoderDev == RT_NULL) return;
	
	rt_device_control(secondEncoderDev, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);
}

void secondPulseEncoderClose(void)
{
	rt_device_close(secondEncoderDev);
}

/************±àÂëÆ÷3½Ó¿Úº¯Êý*************/
int thirdPulseEncoderInit(void)
{
	rt_err_t ret = RT_EOK;
	
	thirdEncoderDev = rt_device_find(PULSE_ENCODER3_DEV_NAME);
	if (thirdEncoderDev == RT_NULL)
	{
		rt_kprintf("pulse encoder sample run failed! can't find %s device!\n", PULSE_ENCODER3_DEV_NAME);
    return RT_ERROR;
	}		
	
  ret = rt_device_open(thirdEncoderDev, RT_DEVICE_OFLAG_RDONLY);
  if (ret != RT_EOK)
  {
      rt_kprintf("open %s device failed!\n", PULSE_ENCODER3_DEV_NAME);
      return ret;
  }	
	
	return RT_EOK;
}

int16_t thirdPulseEncoderGet(void)
{
	int32_t count = 0;
	rt_device_read(thirdEncoderDev, 0, &count, 1);
	return (int16_t)count;
}

void thirdPulseEncoderClear(void)
{
	if(thirdEncoderDev == RT_NULL) return;
	
	rt_device_control(thirdEncoderDev, PULSE_ENCODER_CMD_CLEAR_COUNT, RT_NULL);
}

void thirdPulseEncoderClose(void)
{
	rt_device_close(thirdEncoderDev);
}
