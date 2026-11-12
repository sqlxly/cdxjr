#include "IOdriver.h"
#include "devConfig.h"

// 继电器输出IO口映射 RLY1 ~ RLY7 PORT: I5 I4 E1 E0 B9 I6 I7
rt_base_t rly_map[RELAY_NUM] = {133, 132, 65, 64, 25, 134, 135};

// 继电器控制IO初始化
void configRelayIO(void)
{
	for(int i=0; i<RELAY_NUM; i++)
	{
		rt_pin_mode(rly_map[i], PIN_MODE_OUTPUT);
		rt_pin_write(rly_map[i], PIN_LOW);
	}
}
// 继电器输出控制 使用位操作7bit
void updateRelayIO(uint8_t ctrl)
{
	rt_ssize_t val = 0;
	
	for(int i=0; i<RELAY_NUM; i++)
	{
		val = getbitv(ctrl, i) ? PIN_HIGH : PIN_LOW;
		rt_pin_write(rly_map[i], val);
	}
}

// 电磁铁输出IO口映射 DCT1 ~ DCT24  PORT: E2 E3 E4 E5 E6 I8 C13 I9 I10 I11 F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 C0 C1 C2
rt_base_t dct_map[DCT_NUM] = {	66, 67, 68, 69, 70, 136,	// DCT1 ~ DCT6
																45, 137, 138, 139, 80, 81,// DCT7 ~ DCT12
																82, 83, 84, 85, 86, 87,		// DCT13 ~ DCT18
																88, 89, 90, 32, 33, 34,		// DCT19 ~ DCT24
														 };

// 电磁铁控制IO初始化
void configDctIO(void)
{
	for(int i=0; i<DCT_NUM; i++)
	{
		rt_pin_mode(dct_map[i], PIN_MODE_OUTPUT);
		rt_pin_write(dct_map[i], PIN_LOW);
	}
}

// 电磁铁输出控制 使用位操作24bit
void updateDctIO(uint32_t ctrl)
{
	rt_ssize_t val = 0;
	
	for(int i=0; i<DCT_NUM; i++)
	{
		val = getbitv(ctrl, i) ? PIN_HIGH : PIN_LOW;
		rt_pin_write(dct_map[i], val);
	}
}

// 气缸开闭输出IO口映射 DCF1 ~ DCF20 包括13组气缸开闭 1组发托控制 1组主输送线控制 5组备用
//C3 H2 H3 H4 H5 C4 C5 B0 B1 B2 F11 F12 F13 F14 F15 G0 G1 E7 E8 E9 
rt_base_t dcf_map[DCF_NUM] = {	35, 114, 115, 116, 117,// DCF1 ~ DCF5(气缸开闭)
																36, 37, 16, 17, 18,		// DCF6 ~ DCF10(气缸开闭)
																91, 92, 93, 94, 95,		// DCF11 ~ DCF13(气缸开闭)、XPSS15(主输送线控制)、FTDJ14(发托控制)
																96, 97, 71, 72, 73, 	// RE16 ~ RE20(5组备用)
														 };
							 
// 气缸开闭控制IO初始化
void configDcfIO(void)
{
	for(int i=0; i<DCF_NUM; i++)
	{
		rt_pin_mode(dcf_map[i], PIN_MODE_OUTPUT);
		rt_pin_write(dcf_map[i], PIN_LOW);
	}
}
// 气缸开闭输出控制 使用位操作20bit
void updateDcfIO(uint32_t ctrl)
{
	rt_ssize_t val = 0;
	
	for(int i=0; i<DCF_NUM; i++)
	{
		val = getbitv(ctrl, i) ? PIN_HIGH : PIN_LOW;
		rt_pin_write(dcf_map[i], val);
	}
}
