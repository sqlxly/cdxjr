#ifndef _USER_INTERFACE_H_
#define _USER_INTERFACE_H_

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "stm32f4xx_hal.h"

#define DESELECT_SPI   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET)
#define SELECT_SPI     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET)

void spi_config(void);
uint8_t WR_CMD(uint8_t cmd);
void TIM_Configuration(int32_t period);

void EXTI1_Configuration(void);
void EXTI2_Configuration(void);
void EXTI3_Configuration(void);
		
#endif
