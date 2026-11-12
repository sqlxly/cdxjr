#include "user_outputIO.h"
#include "dev_config.h"

// SRCLK PD7 移位寄存器时钟引脚 上升沿时 移位寄存器中的数据整体后移 并从SER引脚接受新的数据
#define SN74_SRCLK_H			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET)
#define SN74_SRCLK_L			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_RESET)

// RCLK PD6 存储寄存器时钟输入引脚 上升沿时 数据从移位寄存器转存至带存储的寄存器
#define SN74_RCLK_H				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_SET)
#define SN74_RCLK_L				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_6, GPIO_PIN_RESET)

// SER PD4 串行数据输入引脚
#define SN74_SER_H				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_SET)
#define SN74_SER_L				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4, GPIO_PIN_RESET)

// 短暂延时
#define DELAY_NOP()			  {__nop(); __nop(); __nop();__nop(); __nop(); __nop();}

// 初始化SN74HC595PWR GPIO_PIN
void hc595ConfigIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

		__HAL_RCC_GPIOE_CLK_ENABLE();

		// 配置SRCLK、RCLK、SER引脚
    GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_7, GPIO_PIN_SET);
}

// 同时发送数据到6个SN74HC595PWR芯片
void hc595WriteData(uint8_t* rData, uint8_t nByte)
{
	uint8_t chip, bit;
 
  for(chip = 0; chip < nByte; chip++){ 	// 选择芯片
    for(bit = 0; bit < 8; bit++){ 			// 移位数据，每次移位芯片的一位
      if(rData[chip] & (1 << (7 - bit))){
        SN74_SER_H;
      }else{
        SN74_SER_L;
      }
			SN74_SRCLK_L;											// 产生移位时钟脉冲
			DELAY_NOP();
			SN74_SRCLK_H;
			DELAY_NOP();			
    }
	}

  SN74_RCLK_H;													// 锁存所有芯片的数据
  DELAY_NOP();
  SN74_RCLK_L;
  DELAY_NOP();
}

#include <stdlib.h>

static void cmd_dout_ctrl(int argc, char **argv)
{
		int8_t port, sta;

		if(argc > 1) {
			port = atoi(argv[1]);
		}
    if(argc > 2) {
			sta = atoi(argv[2]);
    }
		
		set_output_io_sta(masterOperate.outputData, (enum EnumOutputIO)port, sta);
}

MSH_CMD_EXPORT_ALIAS(cmd_dout_ctrl, write_out_io, ctrl states of output);

// 继电器输出IO口映射 RLY1 ~ RLY7 PORT: B7 I4 E1 E0 B9 B8 I7
rt_base_t rly_map[RELAY_NUM] = {23, 132, 65, 64, 25, 24, 135};

// 继电器控制IO初始化
void configRelayIO(void)
{
	for(int n=0; n<RELAY_NUM; n++)
	{
		rt_pin_mode(rly_map[n], PIN_MODE_OUTPUT);
		rt_pin_write(rly_map[n], PIN_LOW);
	}
}
// 继电器输出控制 使用位操作7bit
void updateRelayIO(int8_t ctrl)
{
	rt_ssize_t val = 0;
	
	for(int n=0; n<RELAY_NUM; n++)
	{
		val = getbitv(ctrl, n) ? PIN_HIGH : PIN_LOW;
		rt_pin_write(rly_map[n], val);
	}
}

static int8_t rlyState = 0;
static void cmd_rly_ctrl(int argc, char **argv)
{
		int8_t port, sta;
	
		if(argc > 1) {
			port = atoi(argv[1]);
		}
    if(argc > 2) {
			sta = atoi(argv[2]);
    }
		
		if(0 == sta) {
			clrbitv(rlyState, port);
		}else{
			setbitv(rlyState, port);
		}
		
		updateRelayIO(rlyState);
}

MSH_CMD_EXPORT_ALIAS(cmd_rly_ctrl, write_rly_io, ctrl states of relay);

