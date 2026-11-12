#ifndef __LED_H
#define	__LED_H

#include "stm32h7xx.h"

//引脚定义
/*******************************************************/

//RUN
#define RUN_PIN                  		GPIO_PIN_6
#define RUN_GPIO_PORT            		GPIOG
#define RUN_GPIO_CLK_ENABLE()    		__GPIOG_CLK_ENABLE()
		
//ERR		
#define ERR_PIN                  		GPIO_PIN_12
#define ERR_GPIO_PORT            		GPIOC
#define ERR_GPIO_CLK_ENABLE()    		__GPIOC_CLK_ENABLE()
		
//ETH_RST		
#define ETH_RST                  		GPIO_PIN_10
#define ETH_RST_GPIO_PORT           GPIOB
#define ETH_RST_GPIO_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
/************************************************************/


/** 控制LED灯亮灭的宏，
	* LED低电平亮，设置ON=0，OFF=1
	* 若LED高电平亮，把宏设置成ON=1 ，OFF=0 即可
	*/
#define OFF  GPIO_PIN_RESET
#define ON GPIO_PIN_SET

/* 直接操作寄存器的方法控制IO */
#define	digitalHi(p,i)				{p->BSRR=i;}			  //设置为高电平		
#define digitalLo(p,i)				{p->BSRR=i<<16;}				//输出低电平
#define digitalToggle(p,i)		{p->ODR ^=i;}			//输出反转状态

#define RUN_TOGGLE			digitalToggle(RUN_GPIO_PORT,RUN_PIN)
#define RUN_ON					digitalHi(RUN_GPIO_PORT,RUN_PIN)
#define RUN_OFF					digitalLo(RUN_GPIO_PORT,RUN_PIN)

#define ERR_TOGGLE			digitalToggle(ERR_GPIO_PORT,ERR_PIN)
#define ERR_ON					digitalHi(ERR_GPIO_PORT,ERR_PIN)
#define ERR_OFF					digitalLo(ERR_GPIO_PORT,ERR_PIN)

#define ETH_RST_TOGGLE	digitalToggle(ETH_RST_GPIO_PORT,ETH_RST)
#define ETH_RST_ON			digitalHi(ETH_RST_GPIO_PORT,ETH_RST)
#define ETH_RST_OFF			digitalLo(ETH_RST_GPIO_PORT,ETH_RST)

/*******************************************************/

 /** 按键按下标置宏
	* 按键按下为高电平，设置 KEY_ON=1， KEY_OFF=0
	* 若按键按下为低电平，把宏设置成KEY_ON=0 ，KEY_OFF=1 即可
	*/
#define KEY_ON	1
#define KEY_OFF	0

void MX_GPIO_Init(void);

//void Key_GPIO_Config(void);
//uint8_t Key_Scan(GPIO_TypeDef* GPIOx,uint16_t GPIO_Pin);

#endif /* __LED_H */
