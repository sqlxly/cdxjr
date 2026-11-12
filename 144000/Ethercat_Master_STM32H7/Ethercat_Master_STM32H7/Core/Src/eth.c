/**
  ******************************************************************************
  * @file    eth.c
  * @brief   This file provides code for the configuration
  *          of the ETH instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eth.h"
#include "DM9000.h"
#include "gpio.h"
#include "stdio.h"
#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#pragma location=0x30040000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x30040060
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
#pragma location=0x30040200
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffers */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x30040000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x30040060))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */
__attribute__((at(0x30040200))) uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffer */
__attribute__((at(0x30044000))) uint8_t Tx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE]; /* Ethernet Receive Buffer */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_MAX_PACKET_SIZE] __attribute__((section(".RxArraySection"))); /* Ethernet Receive Buffers */

#endif

ETH_TxPacketConfig TxConfig;

#include "DM9161.h"
#include "string.h"
#include "gpio.h"
uint8_t RecvLength=0;
uint32_t current_pbuf_idx =0;

ETH_HandleTypeDef heth;

/* ETH init function */
void MX_ETH_Init(void)
{
  heth.Instance = ETH;
  heth.Init.MACAddr[0] =   0x01;
  heth.Init.MACAddr[1] =   0x01;
  heth.Init.MACAddr[2] =   0x01;
  heth.Init.MACAddr[3] =   0x01;
  heth.Init.MACAddr[4] =   0x01;
  heth.Init.MACAddr[5] =   0x01;
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1528;

	HAL_ETH_DeInit(&heth);
	
  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
}

void HAL_ETH_MspInit(ETH_HandleTypeDef* ethHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(ethHandle->Instance==ETH)
  {
    /* ETH clock enable */
    __HAL_RCC_ETH1MAC_CLK_ENABLE();
    __HAL_RCC_ETH1TX_CLK_ENABLE();
    __HAL_RCC_ETH1RX_CLK_ENABLE();

		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
    /**ETH GPIO Configuration
    PG14     ------> ETH_TXD1
    PG13     ------> ETH_TXD0
    PG11     ------> ETH_TX_EN
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PC4     ------> ETH_RXD0
    PA2     ------> ETH_MDIO
    PC5     ------> ETH_RXD1
    PA7     ------> ETH_CRS_DV
    */
    GPIO_InitStruct.Pin = RMII_TXD1_Pin|RMII_TXD0_Pin|RMII_TX_EN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_ETH_MspDeInit(ETH_HandleTypeDef* ethHandle)
{
  if(ethHandle->Instance==ETH)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ETH1MAC_CLK_DISABLE();
    __HAL_RCC_ETH1TX_CLK_DISABLE();
    __HAL_RCC_ETH1RX_CLK_DISABLE();

    /**ETH GPIO Configuration
    PG14     ------> ETH_TXD1
    PG13     ------> ETH_TXD0
    PG11     ------> ETH_TX_EN
    PC1     ------> ETH_MDC
    PA1     ------> ETH_REF_CLK
    PC4     ------> ETH_RXD0
    PA2     ------> ETH_MDIO
    PC5     ------> ETH_RXD1
    PA7     ------> ETH_CRS_DV
    */
    HAL_GPIO_DeInit(GPIOG, RMII_TXD1_Pin|RMII_TXD0_Pin|RMII_TX_EN_Pin);

    HAL_GPIO_DeInit(GPIOC, RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin);

    HAL_GPIO_DeInit(GPIOA, RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin);
  }
}

/* USER CODE BEGIN 1 */
void PHY_Init(void)
{
    uint32_t idx, duplex, speed = 0;
    int32_t PHYLinkState;
    uint8_t i=0;
    ETH_MACConfigTypeDef MACConf;

    
    ETH_MACFilterConfigTypeDef filterDef;
    
    HAL_ETH_GetMACFilterConfig(&heth,&filterDef);
    filterDef.PromiscuousMode = ENABLE;
    HAL_ETH_SetMACFilterConfig(&heth,&filterDef);
    
    
    for(idx = 0; idx < ETH_RX_DESC_CNT; idx ++)
    {
        HAL_ETH_DescAssignMemory(&heth, idx, Rx_Buff[idx], NULL);
    }
    
    DM9161_Init();
    
    do
    {
        HAL_Delay(100);
        
        PHYLinkState = DM9161_GetLinkState();
        
        printf("DM9161_GetLinkState = %d \r\n",PHYLinkState);
		
        ERR_TOGGLE;    /* ´íÎóµÆÉÁ		*/
		
//				i++;
//				
//				if(i>10)
//				{
//					printf("DM9161 STATUS is ERROR\r\n");
//					break;
//				}
    }while(PHYLinkState <= DM9161_STATUS_LINK_DOWN);
    
    switch (PHYLinkState)
    {
    case DM9161_STATUS_100MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        
        printf("DM9161_STATUS_100MBITS_FULLDUPLEX \r\n");
        break;
    case DM9161_STATUS_100MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        
        printf("DM9161_STATUS_100MBITS_HALFDUPLEX \r\n");
        break;
    case DM9161_STATUS_10MBITS_FULLDUPLEX:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_10M;
        
        printf("DM9161_STATUS_10MBITS_FULLDUPLEX \r\n");
        break;
    case DM9161_STATUS_10MBITS_HALFDUPLEX:
        duplex = ETH_HALFDUPLEX_MODE;
        speed = ETH_SPEED_10M;
        
        printf("DM9161_STATUS_10MBITS_HALFDUPLEX \r\n");
        break;
    default:
        duplex = ETH_FULLDUPLEX_MODE;
        speed = ETH_SPEED_100M;
        
//        printf("ETH_FULLDUPLEX_MODE ETH_SPEED_100M\r\n");
        break;      
    }
    
    /* Get MAC Config MAC */
    HAL_ETH_GetMACConfig(&heth, &MACConf); 
    MACConf.DuplexMode = duplex;
    MACConf.Speed = speed;
    MACConf.TransmitQueueMode = ETH_TRANSMITTHRESHOLD_128;
    HAL_ETH_SetMACConfig(&heth, &MACConf);
    HAL_ETH_Start_IT(&heth);
    
    HAL_ETH_BuildRxDescriptors(&heth);
}


uint32_t sendfinishflag=0;

void low_level_output(uint8_t *p,uint32_t length)
{
    uint32_t framelen = 0;
    
    ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];
    
    memset(Txbuffer, 0 , ETH_TX_DESC_CNT*sizeof(ETH_BufferTypeDef));
    
    Txbuffer[0].buffer = p;
    Txbuffer[0].len = length;
    framelen += length;
        
    TxConfig.Length = framelen;
    TxConfig.TxBuffer = Txbuffer;
    
    SCB_CleanInvalidateDCache();
    
    //while( sendfinishflag == 1 );
    HAL_ETH_Transmit(&heth, &TxConfig, 5);
    sendfinishflag = 1;
}

void low_level_input()
{
    
}


void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    low_level_input();
   printf("rx isr\r\n");
}



void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    sendfinishflag = 0;
    printf("tx isr\r\n");
}

int bfin_EMAC_send (void *packet, int length)
{
	unsigned char i;
    memcpy(&Tx_Buff[0][0],packet,length);
//	printf("DM9161 Tx_Buff DATA is :");
//    for(i=0;i<16;i++)
//	  printf("%x ", Tx_Buff[0][i]);
//	printf("\n\r");
    low_level_output(Tx_Buff[0],length);
    
    return 0;
}


int bfin_EMAC_recv (uint8_t * packet, size_t size)
{
    ETH_BufferTypeDef RxBuff;
    uint32_t framelength = 0;
    
    SCB_CleanInvalidateDCache();
    
    HAL_StatusTypeDef status = HAL_ETH_GetRxDataBuffer(&heth, &RxBuff);
    
    if( status == HAL_OK) 
    {
        HAL_ETH_GetRxDataLength(&heth, &framelength);
        
        SCB_InvalidateDCache_by_Addr((uint32_t *)Rx_Buff, (ETH_RX_DESC_CNT*ETH_MAX_PACKET_SIZE));
        
//        printf("Recv = %d current_pbuf_idx=%d\r\n",framelength,current_pbuf_idx);
//        printfBuffer(&Rx_Buff[current_pbuf_idx][0],framelength);
        
        memcpy(packet, Rx_Buff[current_pbuf_idx], framelength);
        
        if(current_pbuf_idx < (ETH_RX_DESC_CNT -1))
        {
            current_pbuf_idx++;
        }
        else
        {
            current_pbuf_idx = 0;
        }
        
        /* Invalidate data cache for ETH Rx Buffers */
        HAL_ETH_BuildRxDescriptors(&heth);
        receiveLen=framelength;
        return framelength;
    }


    return -1;
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
