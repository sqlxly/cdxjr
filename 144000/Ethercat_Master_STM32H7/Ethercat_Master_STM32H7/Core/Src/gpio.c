/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins
     PH0-OSC_IN (PH0)   ------> RCC_OSC_IN
     PH1-OSC_OUT (PH1)   ------> RCC_OSC_OUT
*/
void MX_GPIO_Init(void)
{

 	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef  GPIO_InitStruct;
//	GPIO_InitTypeDef  GPIO_InitStructure;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; 
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);		
	GPIOA->BSRR = GPIO_PIN_4;
	
	////////////////      LED	   ////////////////
	/*开启LED相关的GPIO外设时钟*/
	RUN_GPIO_CLK_ENABLE();
	ERR_GPIO_CLK_ENABLE();
	
	ETH_RST_GPIO_CLK_ENABLE();

	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 
		
		/*选择要控制的GPIO引脚*/															   
	GPIO_InitStruct.Pin = RUN_PIN;	
	HAL_GPIO_Init(RUN_GPIO_PORT, &GPIO_InitStruct);	
		
		/*选择要控制的GPIO引脚*/															   
	GPIO_InitStruct.Pin = ERR_PIN;	
	HAL_GPIO_Init(ERR_GPIO_PORT, &GPIO_InitStruct);		

		/*选择要控制的GPIO引脚*/															   
	GPIO_InitStruct.Pin = ETH_RST;	
	HAL_GPIO_Init(ETH_RST_GPIO_PORT, &GPIO_InitStruct);	

	ETH_RST_ON;
	HAL_Delay(100);
	ETH_RST_OFF;
	HAL_Delay(100);
	ETH_RST_ON;

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
