#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#include "osal.h"

void bsp_InitTimer(void);
uint32_t bsp_GetTickCount(void);
void TIM2_Int_Init(uint32_t arr,uint16_t psc);
void Delay_us(uint32_t time);
void Delay_ms(uint32_t time);
#endif


