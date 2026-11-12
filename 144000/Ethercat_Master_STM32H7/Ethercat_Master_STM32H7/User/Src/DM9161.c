#include "DM9161.h"
#include "stdio.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_eth.h"
#include "main.h"

extern ETH_HandleTypeDef heth;

int DM9161_Init(void)
{
    uint32_t regvalue = 0;
    HAL_StatusTypeDef rtn;
   
    HAL_ETH_SetMDIOClockRange(&heth);

    //Èí¼þ¸´Î»Ò»ÏÂ
//    printf("start soft reset\r\n");
    
    rtn = HAL_ETH_WritePHYRegister(&heth,DM9161addr,DM9161_BSR,DM9161_BCR_SOFT_RESET);
    
    if( rtn != HAL_OK )
    {
        printf("HAL_ETH_WritePHYRegister DM9161_BCR DM9161_BCR_SOFT_RESET Error \r\n");
        return -1;
    }
    
    //²éÑ¯ÊÇ·ñ¸´Î»Íê³É
//    printf("check soft reset is finish or not\r\n");
    do
    {
        rtn = HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_BSR, &regvalue);
        if( rtn != HAL_OK )
        {
            printf("HAL_ETH_ReadPHYRegister DM9161_BCR Error \r\n");
            return -1;
        }
    }while( regvalue & DM9161_BCR_SOFT_RESET );
    
//    printf("soft reset is finish\r\n");

    return 0;
}

int DM9161_GetLinkState(void)
{
    uint32_t readval = 0;
    int32_t rtn;
    
    //¶Á×´Ì¬¼Ä´æÆ÷
    rtn = HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_BSR, &readval);
    if( rtn != HAL_OK )
    {
        printf("HAL_ETH_ReadPHYRegister DM9161_BSR Error \r\n");
        return DM9161_STATUS_READ_ERROR;
    }
    
    //ÔÙ¶ÁÒ»±é×´Ì¬¼Ä´æÆ÷
    rtn = HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_BSR, &readval);
    if( rtn != HAL_OK )
    {
        printf("HAL_ETH_ReadPHYRegister DM9161_BSR Error \r\n");
        return DM9161_STATUS_READ_ERROR;
    }
    
    if((readval & DM9161_BSR_LINK_STATUS) == 0)
    {
        return DM9161_STATUS_LINK_DOWN;    
    }
    
    /* Check Auto negotiaition */
    rtn = HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_BCR, &readval);
    if( rtn != HAL_OK )
    {
        return DM9161_STATUS_READ_ERROR;
    }
    
    if((readval & DM9161_BCR_AUTONEGO_EN) != DM9161_BCR_AUTONEGO_EN)
    {
        if(((readval & DM9161_BCR_SPEED_SELECT) == DM9161_BCR_SPEED_SELECT) && ((readval & DM9161_BCR_DUPLEX_MODE) == DM9161_BCR_DUPLEX_MODE)) 
        {
            return DM9161_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((readval & DM9161_BCR_SPEED_SELECT) == DM9161_BCR_SPEED_SELECT)
        {
            return DM9161_STATUS_100MBITS_HALFDUPLEX;
        }        
        else if ((readval & DM9161_BCR_DUPLEX_MODE) == DM9161_BCR_DUPLEX_MODE)
        {
            return DM9161_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
            return DM9161_STATUS_10MBITS_HALFDUPLEX;
        }  		
    }
    else /* Auto Nego enabled */
    {
        rtn = HAL_ETH_ReadPHYRegister(&heth, DM9161addr, DM9161_PHYSCSR, &readval);
        if( rtn != HAL_OK )
        {
            return DM9161_STATUS_READ_ERROR;
        }
        
        /* Check if auto nego not done */
        if((readval & DM9161_PHYSCSR_AUTONEGO_DONE) == 0)
        {
            return DM9161_STATUS_AUTONEGO_NOTDONE;
        }
        
        if((readval & DM9161_PHYSCSR_HCDSPEEDMASK) == DM9161_PHYSCSR_100BTX_FD)
        {
            return DM9161_STATUS_100MBITS_FULLDUPLEX;
        }
        else if ((readval & DM9161_PHYSCSR_HCDSPEEDMASK) == DM9161_PHYSCSR_100BTX_HD)
        {
            return DM9161_STATUS_100MBITS_HALFDUPLEX;
        }
        else if ((readval & DM9161_PHYSCSR_HCDSPEEDMASK) == DM9161_PHYSCSR_10BT_FD)
        {
            return DM9161_STATUS_10MBITS_FULLDUPLEX;
        }
        else
        {
            return DM9161_STATUS_10MBITS_HALFDUPLEX;
        }				
    }
}

