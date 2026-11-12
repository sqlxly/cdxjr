#include "sn74hc165pwr.h"

#define SN74_LD_H			HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_SET)
#define SN74_LD_L			HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_RESET)

#define SN74_CLK_H		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET)
#define SN74_CLK_L		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_RESET)

#define SN74_Q				HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_10)

#define DELAY_NOP		 { __nop(); __nop(); __nop();__nop(); __nop(); __nop(); __nop(); __nop(); __nop();__nop(); }

// 初始化SN74HC165PWR GPIO
void hc165ConfigIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		__HAL_RCC_GPIOE_CLK_ENABLE();
		__HAL_RCC_GPIOI_CLK_ENABLE();
	
		// SN74_CLK 
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, GPIO_PIN_SET);
		
		// SN74_LD
		GPIO_InitStruct.Pin = GPIO_PIN_2;
		HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOI, GPIO_PIN_2, GPIO_PIN_SET);
		
		// SN74_Q
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

// 读取74HC165的端口状态
void hc165ReadData(uint8_t* rData, uint8_t nByte)
{
	rt_base_t level;
	int8_t len=8, n=0;
  uint8_t temp = 0;
	
	SN74_LD_L;																			//SHLD拉低,165并口数据传入到移位寄存器中
	DELAY_NOP;
	SN74_LD_H;																			//SHLD拉高,禁止165并口数据传入到移位寄存器中
	DELAY_NOP;
	for(n=nByte-1; n>=0; n--)
	{
		while(len--)
		{
      temp |= (uint8_t)SN74_Q << len;							//左移一位,先读取数据高位,共读取8个数据位
			SN74_CLK_L; 
			DELAY_NOP;
			SN74_CLK_H;																	//时钟上升沿读取数据
			DELAY_NOP;
    }
		level = rt_hw_interrupt_disable();
		*(rData+n) = temp;
		rt_hw_interrupt_enable(level);
    len = 8;
		temp = 0;
  }
}

