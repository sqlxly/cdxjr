#ifndef __USER_ENCODER_H__
#define __USER_ENCODER_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <drv_gpio.h>
#include <board.h>

// ±àÂëÆ÷1²Ù×÷º¯Êý
int firstPulseEncoderInit(void);
int16_t firstPulseEncoderGet(void);
void firstPulseEncoderClear(void);
void firstPulseEncoderClose(void);

// ±àÂëÆ÷2²Ù×÷º¯Êý
int secondPulseEncoderInit(void);
int16_t secondPulseEncoderGet(void);
void secondPulseEncoderClear(void);
void secondPulseEncoderClose(void);

// ±àÂëÆ÷3²Ù×÷º¯Êý
int thirdPulseEncoderInit(void);
int16_t thirdPulseEncoderGet(void);
void thirdPulseEncoderClear(void);
void thirdPulseEncoderClose(void);

#endif
