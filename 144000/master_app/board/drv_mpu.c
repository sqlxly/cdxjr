/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-04-14     whj4674672   first version
 */
#include <rtthread.h>
#include "stm32h7xx.h"

int mpu_init(void)
{
    MPU_Region_InitTypeDef MPU_InitStruct;

    /* Disable the MPU */
    HAL_MPU_Disable();

    /* Configure the MPU attributes as WT for AXI SRAM */
    MPU_InitStruct.Enable            = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress       = 0x24000000;
    MPU_InitStruct.Size              = MPU_REGION_SIZE_512KB;
    MPU_InitStruct.AccessPermission  = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable      = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable       = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable       = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number            = MPU_REGION_NUMBER0;
    MPU_InitStruct.TypeExtField      = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable  = 0X00;
    MPU_InitStruct.DisableExec       = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);

#ifdef BSP_USING_SDRAM
    /* Configure the MPU attributes as WT for SDRAM */
    MPU_InitStruct.Enable            = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress       = 0xC0000000;
    MPU_InitStruct.Size              = MPU_REGION_SIZE_32MB;
    MPU_InitStruct.AccessPermission  = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable      = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable       = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable       = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number            = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField      = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable  = 0x00;
    MPU_InitStruct.DisableExec       = MPU_INSTRUCTION_ACCESS_ENABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

#ifdef BSP_USING_ETH
		/* Configure the MPU attributes as Device not cacheable 
			for ETH DMA descriptors */
		MPU_InitStruct.Enable = MPU_REGION_ENABLE;
		MPU_InitStruct.BaseAddress = 0x30040000;
		MPU_InitStruct.Size = MPU_REGION_SIZE_2KB;
		MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
		MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
		MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
		MPU_InitStruct.Number = MPU_REGION_NUMBER0;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
		MPU_InitStruct.SubRegionDisable = 0x00;
		MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
	
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
		
		/* Configure the MPU attributes as Normal Non Cacheable
			for LwIP RAM heap which contains the Tx buffers */
		MPU_InitStruct.Enable = MPU_REGION_ENABLE;
		MPU_InitStruct.BaseAddress = 0x30044000;
		MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
		MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
		MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
		MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
		MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
		MPU_InitStruct.Number = MPU_REGION_NUMBER1;
		MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
		MPU_InitStruct.SubRegionDisable = 0x00;
		MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

		HAL_MPU_ConfigRegion(&MPU_InitStruct);	
#endif

		/* 配置SRAM4的MPU属性为Non-cacheable */
		MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
		MPU_InitStruct.BaseAddress      = 0x38000000;
		MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
		MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
		MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
		MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
		MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
		MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
		MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
		MPU_InitStruct.SubRegionDisable = 0x00;
		MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	
		HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

    /* Enable CACHE */
    SCB_EnableICache();
    SCB_EnableDCache();
		SCB->CACR|=1<<2;   //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题
		
    return 0;
}

INIT_BOARD_EXPORT(mpu_init);
