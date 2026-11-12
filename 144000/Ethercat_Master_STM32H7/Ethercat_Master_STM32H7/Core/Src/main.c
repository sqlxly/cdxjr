/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/*


*/
	
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "eth.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"
#include "usart2.h"
#include "stdio.h"
#include "DM9000.h"
#include "DM9161.h"
#include "bsp_timer.h"
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "SPI_DMA_master.h"

//#define EC_DEBUG

#ifdef EC_DEBUG
#define EC_PRINT printf
#else
#define EC_PRINT(...) do {} while (0)
#endif
#ifndef EC_MAX_MAPT
 #define EC_MAX_MAPT
#endif
	
/* Private function prototypes -----------------------------------------------*/
void Stm32_Clock_Init(uint32_t plln,uint32_t pllm,uint32_t pllp,uint32_t pllq); //配置系统时钟
static void MPU_Config(void);
void Cache_Enable(void); 

/*
DM9161 主网卡  DM9000付网卡
0    主网卡 发送 付网卡 接收   正常运行
1    主网卡 发送 主网卡 接收		付网卡坏或者付网卡网线没插
2    付网卡 发送 付网卡 接收   主网卡坏或者主网卡网线没插
3    主网卡 发送 主网卡 接收   付网卡 发送 付网卡接收  中间模块或者网线损坏
ff   主网卡 付网卡 均损坏
*/
char IOmap[1024];
uint32_t vid=0;
uint16 cur_status;
int32 cur_pos = 0;
uint8 cur_mode;
uint8 all_slave_op = 0;
__IO uint32_t pdoTimeFlag = 0;
PDO_Output *outputs[MAX_MODULE];
PDO_Input *inputs[MAX_MODULE];

uint8_t socket_mode;
int DM9000_Status;
int DM9162_Status;

uint8_t offlineFlag = 0;

/**
  * @brief  The application entry point.
  * @retval int
  */
uint16 SEEDM9000;
int main(void)
{
  uint32_t dm9161status = 0;
  uint32_t i,j;
  uint16_t slc,cnt;
	uint8 doWhile = 0;
	
  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  Cache_Enable();                													/* 打开L1-Cache */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  Stm32_Clock_Init(160,5,2,4);  		    									/* 设置时钟,400Mhz */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_TIM1_Init();																					/* ECAT DC同步相关定时器初始化 */
  MX_TIM2_Init();																				  /* DM9000相关操作定时器初始化 */
  DEBUG_USART_Config();
  EC_PRINT("Welcome To The Ethercat World\r\n\r");
  DM9000_Init();																					/* DM9000初始化 */
  vid = dm9k_ReadID();																		/* 读取dm9000 ID=0x0A469000 若不等于此值，并口的操作错误 */
  printf("DM9000 VID=%x\r\n\r", vid);
  PHY_Init();																							/* DM9161初始化 */
	bsp_InitSPIBus();																				/* SPI1_DMA_MASTER初始化 */
   __enable_irq();
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim2);
//	HAL_TIM_Base_Start_IT(&htim3);
//   do{
//     SEEDM9000=(dm9k_ReadReg(DM9000_REG_NSR)& DM9000_LINK_STATUE);
//   }while(SEEDM9000!=0x40);
  socket_mode=1;
  do
	{
		HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_BSR, &dm9161status); 
		if((dm9161status & 0x0004)&&(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_LINK_STATUE))  					/* 双网冗余运行模式 */
		{
			socket_mode=2;
		}else if((dm9161status & 0x0004)&&(!(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_LINK_STATUE))) { /* 单网运行模式 */
			socket_mode=1;
		}else{																																											/* 其他情况 未检测到网卡 错误灯闪烁 */
			socket_mode=0;
			EC_PRINT("Please plug in the Ethernet cable first \r\n\r");
		}
		Delay_ms(100);
		ERR_TOGGLE;   					 															/* 错误灯闪	*/
	}while(socket_mode==0); 																/* 检测到错误不允许运行 */
  ERR_OFF;																								/* 检测到网卡连接，正常运行错误灯灭 */
	ec_init();  																						/* 配置单网运行or双网冗余运行 */
  printf("\r\nSocket Mode: %d\r\n\r", socket_mode);	
	Delay_ms(3000);
  RUN_ON;   																							/* Run 灯点亮	 */

	if(ec_config_init(TRUE)>0)															/* 查找从站，初始化从站等	 */
	{
	  printf("%d slaves found and configured.\r\n",ec_slavecount);         
		if((ec_slavecount >= 1))
		{
			for(slc = 1; slc <= ec_slavecount; slc++)
			{
				printf("Found %s at position %d\r\n", ec_slave[slc].name, slc);
			}
		}
	  ec_config_map(&IOmap);																/* 分配从站PDO,与IOmap变量关联 */					
		EC_PRINT("Slaves mapped, state to SAFE_OP.\r\n");
    ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE); /* 等待所有从站状态切换到 SAFE_OP */

//		EC_PRINT("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
//		for(slc = 1; slc <= ec_slavecount; slc++)
//		{
//			EC_PRINT("Slave %d State=0x%04x\r\n", slc, ec_slave[slc].state);
//		}

		do
		{
			doWhile = FALSE;
			ec_statecheck(0, EC_STATE_SAFE_OP, 50000);
			if(ec_slave[0].state != EC_STATE_SAFE_OP){
				doWhile = TRUE;
			}
			for(slc = 1; slc <= ec_slavecount; slc++)
			{
				ec_statecheck(slc, EC_STATE_SAFE_OP, 50000);
				if(ec_slave[slc].state != EC_STATE_SAFE_OP)
				{
					doWhile = TRUE;
				}
			}
		}
		while (doWhile);

		EC_PRINT("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
		for(slc = 1; slc <= ec_slavecount; slc++)
		{
			EC_PRINT("Slave %d State=0x%04x\r\n", slc, ec_slave[slc].state);
		}

    for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)					/* 打印分配的信息 */
    {
      EC_PRINT("\r\nSlave:%d\r\n Name:%s\r\n Output size: %dbits\r\n Input size: %dbits\r\n State: %d\r\n Delay: %d[ns]\r\n Has DC: %d\r\n",
																											cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
																											ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
      EC_PRINT(" Configured address: %x\r\n", ec_slave[cnt].configadr);
      EC_PRINT(" Outputs address: %x Obytes:%d %d\r\n", ec_slave[cnt].outputs, ec_slave[cnt].Obytes, ec_slave[cnt].Obits);
      EC_PRINT(" Inputs address: %x Ibytes:%d %d\r\n", ec_slave[cnt].inputs, ec_slave[cnt].Ibytes, ec_slave[cnt].Ibits);

      for(j = 0 ; j < ec_slave[cnt].FMMUunused ; j++)
      {
				EC_PRINT(" FMMU%1d Ls:%x Ll:%4d Lsb:%d Leb:%d Ps:%x Psb:%d Ty:%x Act:%x\r\n", j,
                        (int)ec_slave[cnt].FMMU[j].LogStart, ec_slave[cnt].FMMU[j].LogLength, ec_slave[cnt].FMMU[j].LogStartbit,
                        ec_slave[cnt].FMMU[j].LogEndbit, ec_slave[cnt].FMMU[j].PhysStart, ec_slave[cnt].FMMU[j].PhysStartBit,
                        ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive);
      }
      EC_PRINT(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\r\n",
                      ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);
    }
		EC_PRINT("Request operational state for all slaves\r\n");
    ec_slave[0].state = EC_STATE_OPERATIONAL;
    /* send one valid process data to make outputs in slaves happy */
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);
    /* 请求所有从站运行到OP 状态 */
    ec_writestate(0);
    /* 等待所有从站切换到 OP 状态 */
    ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);								
		do
		{
			doWhile = FALSE;
			ec_send_processdata();
			ec_receive_processdata(EC_TIMEOUTRET);
			ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
			if(ec_slave[0].state != EC_STATE_OPERATIONAL)
			{
				doWhile = TRUE;
			}
			for(slc = 1; slc <= ec_slavecount; slc++)
			{
				ec_statecheck(slc, EC_STATE_OPERATIONAL, 50000);
				if(ec_slave[slc].state != EC_STATE_OPERATIONAL)
				{
					doWhile = TRUE;
				}
			}
		}
		while (doWhile);
		
		EC_PRINT("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
		for(slc = 1; slc <= ec_slavecount; slc++)
		{
			EC_PRINT("Slave %d State=0x%04x\r\n",slc, ec_slave[slc].state);
		}
		
    if (ec_slave[0].state == EC_STATE_OPERATIONAL) {	/* 如果所有从站状态为OP 则对从站的 输入输出进行重映射 */
      printf("Operational state reached for all slaves.\r\n");
			all_slave_op = 1;		
			for(i = 1; i<=ec_slavecount ; i++){
				outputs[i] = (PDO_Output *)ec_slave[i].outputs;
				inputs[i]  = (PDO_Input *)ec_slave[i].inputs;
			}
    } else {																					/* 反之 打印从站状态 */
       printf("Not all slaves reached operational state.\r\n");
       ec_readstate();
       for(i = 1; i<=ec_slavecount ; i++)
       {
         if(ec_slave[i].state != EC_STATE_OPERATIONAL)
         {
           printf("Slave %d State=0x%04x StatusCode=0x%04x\r\n",
                    i, ec_slave[i].state, ec_slave[i].ALstatuscode);
         }
       }
		}
	} else {
		printf("No slaves found!\r\n");
	}
	
	memset(g_spiTxBuf, 0, 1024);
	memset(g_spiRxBuf, 0, 1024);
	
	while(1)
	{
 		DM9162_Status=DM9161_GetLinkState();																		/* 主网口link状态标志 */
		DM9000_Status=(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_PHY);							/* 冗余口link状态标志 */
		if(((DM9161_STATUS_READ_ERROR == DM9162_Status)||(DM9161_STATUS_LINK_DOWN == DM9162_Status)) && (!DM9000_Status)) {	/* 主网口和冗余口都掉线时跳转到re_check_net 等待网口重新连接 */
			printf("Ethernets are all error,NOW must reset! goto  lable re_check_net\n");
			HAL_NVIC_SystemReset();  																							/* 两根网线都掉了 所有模块掉线 重启系统 */
		}

		if(offlineFlag > 0){																										/* 从站离线了 */
			Delay_ms(100);
			HAL_NVIC_SystemReset();  
		}

		//PDO的通讯在TIM3中断函数中
		if(all_slave_op == 1)	/* PDO通讯定时标志位 */
	  {
				switch (wTransferState)
				{
					case TRANSFER_WAIT:
						break;
					case TRANSFER_ACTIVE:
						wTransferState = TRANSFER_WAIT;
						bsp_spiTransfer();
						break;
					case TRANSFER_COMPLETE:
						break;
					case TRANSFER_ERROR:
						bsp_InitSPIBus();	
						wTransferState = TRANSFER_ACTIVE;
						break;
					default: break;
				}
	  }
	}
}

//使能CPU的L1-Cache
void Cache_Enable(void)
{
  SCB_EnableICache();//使能I-Cache
  SCB_EnableDCache();//使能D-Cache   
	SCB->CACR|=1<<2;   //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题	
}

//时钟设置函数
//Fvco=Fs*(plln/pllm);
//Fsys=Fvco/pllp=Fs*(plln/(pllm*pllp));
//Fq=Fvco/pllq=Fs*(plln/(pllm*pllq));

//Fvco:VCO频率
//Fsys:系统时钟频率,也是PLL1的p分频输出时钟频率
//Fq:PLL1的q分频输出时钟频率
//Fs:PLL输入时钟频率,可以是HSI,CSI,HSE等. 

//plln:PLL1倍频系数(PLL倍频),取值范围:4~512.
//pllm:PLL1预分频系数(进PLL之前的分频),取值范围:2~63.
//pllp:PLL1的p分频系数(PLL之后的分频),分频后作为系统时钟,取值范围:2~128.(且必须是2的倍数)
//pllq:PLL1的q分频系数(PLL之后的分频),取值范围:1~128.

//CPU频率(rcc_c_ck)=sys_d1cpre_ck=400Mhz 
//rcc_aclk=rcc_hclk3=200Mhz
//AHB1/2/3/4(rcc_hclk1/2/3/4)=200Mhz  
//APB1/2/3/4(rcc_pclk1/2/3/4)=100Mhz  
//FMC时钟频率=pll2_r_ck=((25/25)*512/2)=256Mhz

//外部晶振为25M的时候,推荐值:plln=160,pllm=5,pllp=2,pllq=2.
//得到:Fvco=25*(160/5)=800Mhz
//Fsys=800/2=400Mhz
//Fq=800/2=400Mhz
//返回值:0,成功;1,失败。
void Stm32_Clock_Init(uint32_t plln,uint32_t pllm,uint32_t pllp,uint32_t pllq)
{
	HAL_StatusTypeDef ret=HAL_OK;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
  
	MODIFY_REG(PWR->CR3,PWR_CR3_SCUEN, 0);
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) {}
  
	RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState=RCC_HSE_ON;
	RCC_OscInitStruct.HSIState=RCC_HSI_OFF;
	RCC_OscInitStruct.CSIState=RCC_CSI_OFF;
	RCC_OscInitStruct.PLL.PLLState=RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource=RCC_PLLSOURCE_HSE;

	RCC_OscInitStruct.PLL.PLLN=plln;
	RCC_OscInitStruct.PLL.PLLM=pllm;
	RCC_OscInitStruct.PLL.PLLP=pllp;
	RCC_OscInitStruct.PLL.PLLQ=pllq;

	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	ret=HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret!=HAL_OK) while(1);
  
	RCC_ClkInitStruct.ClockType=(RCC_CLOCKTYPE_SYSCLK|\
                                RCC_CLOCKTYPE_HCLK |\
                                RCC_CLOCKTYPE_D1PCLK1 |\
                                RCC_CLOCKTYPE_PCLK1 |\
                                RCC_CLOCKTYPE_PCLK2 |\
                                RCC_CLOCKTYPE_D3PCLK1);

	RCC_ClkInitStruct.SYSCLKSource=RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider=RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider=RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB1CLKDivider=RCC_APB1_DIV2; 
	RCC_ClkInitStruct.APB2CLKDivider=RCC_APB2_DIV2; 
	RCC_ClkInitStruct.APB3CLKDivider=RCC_APB3_DIV2;  
	RCC_ClkInitStruct.APB4CLKDivider=RCC_APB4_DIV4; 
	ret=HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
	if(ret!=HAL_OK) while(1);

	__HAL_RCC_CSI_ENABLE() ;
	__HAL_RCC_SYSCFG_CLK_ENABLE() ;  
	HAL_EnableCompensationCell();
	
/* AXI SRAM的时钟是上电自动使能的，而D2域的SRAM1，SRAM2和SRAM3要单独使能 */	
#if 1
	__HAL_RCC_D2SRAM1_CLK_ENABLE();
	__HAL_RCC_D2SRAM2_CLK_ENABLE();
	__HAL_RCC_D2SRAM3_CLK_ENABLE();
#endif
}

/* MPU Configuration */
void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30044000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
	/* 配置FMC扩展IO的MPU属性为Device或者Strongly Ordered */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x68000000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_32MB;	  //ARM_MPU_REGION_SIZE_64MB
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;	/* 不能用MPU_ACCESS_CACHEABLE;会出现2次CS、WE信号 */
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x24000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;	/* 不能用MPU_ACCESS_CACHEABLE;会出现2次CS、WE信号 */
	MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
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

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM1)
	{
		if(all_slave_op==1)  	/* 所有从站已经处于OP状态 可以进行PDO的操作 */
		{
			pdoTimeFlag = 1;   	/* PDO通讯定时标志位 */
		  ecat_loop(); 				/* 主控计算的逻辑赋值到RxPDO */
		} else {
		  ec_send_processdata();	
		}
	}		

  if (htim->Instance == TIM7) {
    HAL_IncTick();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**

  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
