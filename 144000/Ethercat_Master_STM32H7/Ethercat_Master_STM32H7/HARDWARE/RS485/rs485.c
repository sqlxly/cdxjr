#include "rs485.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F7开发板
//RS485驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/12/29
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

UART_HandleTypeDef USART3_RS485Handler;  //USART2句柄(用于RS485)
//接收缓存区 	
uint8_t RS485_RX_BUF[64];  	//接收缓冲,最大64个字节.
//接收到的数据长度
uint8_t RS485_RX_CNT=0; 
uint8_t RS485_Receive_Over=0;

#if EN_USART3_RX   		//如果使能了接收   	  
void USART3_IRQHandler(void)
{
    uint8_t res;	  
    if(__HAL_UART_GET_IT(&USART3_RS485Handler,UART_IT_RXNE)!=RESET)  //接收中断
	{	 	
        HAL_UART_Receive(&USART3_RS485Handler,&res,1,1000);
		if(RS485_RX_CNT<64)
		{
			RS485_RX_BUF[RS485_RX_CNT]=res;		//记录接收到的值
			RS485_RX_CNT++;						//接收数据增加1 
		} 
	} 
}    
#endif

//初始化IO 串口2
//bound:波特率
void RS485_Init(uint32_t bound)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
		
	__HAL_RCC_GPIOC_CLK_ENABLE();			//使能GPIOC时钟
	__HAL_RCC_USART3_CLK_ENABLE();			//使能USART3时钟
	
	GPIO_Initure.Pin=GPIO_PIN_10|GPIO_PIN_11; //PC10,11
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;//高速
	GPIO_Initure.Alternate=GPIO_AF7_USART3;	//复用为USART3
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	//初始化PC10,11
    
	GPIO_Initure.Pin=GPIO_PIN_12;        //PC12,RS485收发控制端
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;	//推挽输出
	GPIO_Initure.Pull=GPIO_PULLDOWN;			//上拉
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;//高速
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);	   	//初始化PC12
	
    //USART 初始化设置
	USART3_RS485Handler.Instance=USART3;			        //USART3
	USART3_RS485Handler.Init.BaudRate=bound;		        //波特率
	USART3_RS485Handler.Init.WordLength=UART_WORDLENGTH_8B;	//字长为8位数据格式
	USART3_RS485Handler.Init.StopBits=UART_STOPBITS_1;		//一个停止位
	USART3_RS485Handler.Init.Parity=UART_PARITY_NONE;		//无奇偶校验位
	USART3_RS485Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;	//无硬件流控
	USART3_RS485Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	HAL_UART_Init(&USART3_RS485Handler);			        //HAL_UART_Init()会使能USART3
    
    __HAL_UART_CLEAR_IT(&USART3_RS485Handler,UART_CLEAR_TCF);
#if EN_USART3_RX
	__HAL_UART_ENABLE_IT(&USART3_RS485Handler,UART_IT_RXNE);//开启接收中断
	HAL_NVIC_EnableIRQ(USART3_IRQn);				        //使能USART3中断
	HAL_NVIC_SetPriority(USART3_IRQn,3,3);			        //抢占优先级3，子优先级3
#endif	
    RS485_TX_Set(0);                                        //设置为接收模式	
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(uint8_t *buf,uint8_t len)
{
	RS485_TX_Set(1);			//设置为发送模式
    HAL_UART_Transmit(&USART3_RS485Handler,buf,len,1000);//串口2发送数据
	RS485_RX_CNT=0;	  
	RS485_TX_Set(0);			//设置为接收模式	
}
//RS485查询接收到的数据
//buf:接收缓存首地址
//len:读到的数据长度
void RS485_Receive_Data(uint8_t *buf,uint8_t *len)
{
	uint8_t rxlen=RS485_RX_CNT;
	uint8_t i=0;
	*len=0;				//默认为0
	Delay_ms(2);		//等待10ms,连续超过10ms没有接收到一个数据,则认为接收结束
	if(rxlen==RS485_RX_CNT&&rxlen)//接收到了数据,且接收完成了
	{
		for(i=0;i<rxlen;i++)
		{
			buf[i]=RS485_RX_BUF[i];	
		}		
		*len=RS485_RX_CNT;	//记录本次数据长度
		RS485_RX_CNT=0;		//清零
		RS485_Receive_Over=1;
	}
} 
//RS485模式控制.
//en:0,接收;1,发送.
void RS485_TX_Set(uint8_t en)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,en);
}
