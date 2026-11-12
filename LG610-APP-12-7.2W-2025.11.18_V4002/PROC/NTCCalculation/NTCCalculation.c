/**
  ******************************************************************************
  * @file    NTCCalculation.cpp
  * @author  LG Application Team
  * @brief   Calculation NTC temperature.
  *
  @verbatim
  ==============================================================================
*/
/* Includes ------------------------------------------------------------------*/
#include "NTCCalculation.h"
#include <stdio.h>

static const unsigned short AC2000_NTCTAB[NTC_TABLE_Length]= 
{
	0x03F3,	0x03CB,	0x039B,	0x035B,	0x02FE,	0x0294,	0x0220,	0x01B1,	0x014E,	0x00FD,
	0x00BD,	0x008C,	0x0063,	0x0040,	0x001F,	0x00B
};

/**
  * @brief Look up table function
  * @param a Table list
  * @param ArrayLong Array length
  * @param data Ntc ad value
  * @retval float Temperature value
  */
unsigned char lookUpTable(const unsigned short *a,unsigned char ArrayLong,unsigned short data)
{    
    unsigned short begin,end,middle ;  
    unsigned char i ;  
    begin = 0 ;  
    end = ArrayLong-1 ;  
    i = 0  ;  
    if(data >= a[begin]) return begin ;  
    else if(data <= a[end]) return end ;  
    while(begin < end)  
    {  
            middle = (begin+end)/2 ;  
            if(data == a[middle] ) break ;  
            if(data < a[middle] && data > a[middle+1]) break ;   
            if(data < a[middle+1]) begin = middle ;                      
            else end = middle ;      
            if(i++ > ArrayLong) break ;  
    }
    if(begin > end ) return 0 ;   
    return middle;
}

/**
  * @brief Enter the serial number value of the table to obtain the temperature value
  * @param num table number
  * @retval float Temperature value
  */
float numToTemperature(unsigned char num)
{
    float  data;
    data = NTC_TABLE_Minimum_Temperature+NTC_TABLE_Temperature_Tnterval*(num);
    return data;
}

/**
  * @brief calculation NTC Function
  * @param data Ntc ad value
  * @retval float Temperature value
  */
float calculationGetNtc(unsigned short data)
{
	unsigned char num = 0;
	float tx = 0.0, t1 = 0.0;
	if( (data<=AC2000_NTCTAB[0]) && (data>=AC2000_NTCTAB[NTC_TABLE_Length-1]) )
    {
        num=lookUpTable(AC2000_NTCTAB,NTC_TABLE_Length,data);
        t1=numToTemperature(num);
        tx=NTC_TABLE_Temperature_Tnterval*(data-AC2000_NTCTAB[num])/(AC2000_NTCTAB[num+1]-AC2000_NTCTAB[num])+t1;
//		printf("temp_ntc:%4.2f,%4.2f,%d,%x\r\n",tx,t1,num,data);    
    }
	return tx;
}




