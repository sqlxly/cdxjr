#include "food_weight.h"
#include "data_table.h"
#include "bsp_clkconfig.h"
const uint8_t weight_cmd[WEIGHT_CMD_LENGTH]={0x01,0x03,0x00,0x1B,0x00,0x25,0xf4,0x16};
//28        29      30              64
//今日耗料 今日上料 当前料重(分度)  累计耗料  37字节
//01 03 4A 00 00 00 00 00 05 00 00 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 AA CC 

FOOD_WEIGHT food_weight_table;
void food_weight_process(uint8_t *data,int length)
{
	/*
	01 03 4A 
	00 00 
	00 00 
	00 05 
	00 00 00 05 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
	00 00 
	AA CC 
	*/
	unsigned int crc1=server_crc_chk(data,(length-2));
	unsigned int crc2=data[length-2]*256+data[length-1];
	
	if(crc1==crc2&&Ac2000Data!=NULL)
	{
		if(ac2000_table.ac2000_3g_house!=0)//3G版
		{	
				int32_t x=data[3]*256+data[4];//料塔今日用量
				int32_t y=data[5]*256+data[6];//料塔今日上量
				int32_t z=data[9]*256*256*256+data[10]*256*256+data[7]*256+data[8];//料塔当前料重
				Ac2000Data[ADDR_3G_1928] = x;//料塔今日用量
				RS485_1_DEBUG("料塔今日用量=%d\r\n",Ac2000Data[ADDR_3G_1928]);
				//DEBUG("料塔今日用量=%d\r\n",Ac2000Data[ADDR_1928]);
				
				Ac2000Data[ADDR_3G_1929] = y;//料塔今日上量
				RS485_1_DEBUG("料塔今日上量=%d\r\n",Ac2000Data[ADDR_3G_1929]);
				//DEBUG("料塔今日上量=%d\r\n",Ac2000Data[ADDR_1929]);
				
				Ac2000Data[ADDR_3G_1927] = z;//料塔当前料重
				RS485_1_DEBUG("料塔当前料重=%d\r\n",Ac2000Data[ADDR_3G_1927]);
				//DEBUG("料塔当前料重=%d\r\n",Ac2000Data[ADDR_1927]);
				
				int32_t m=data[75]*256+data[76];
				Ac2000Data[ADDR_3G_1930] = m;//料塔累计耗料
				RS485_1_DEBUG("料塔累计耗料=%d\r\n",Ac2000Data[ADDR_3G_1930]);
				//DEBUG("料塔累计耗料=%d\r\n",Ac2000Data3G[ADDR_1930]);
				food_weight_table.food_weigh_link++;
				food_weight_table.food_weight_mount=true;
		}
		else if(ac2000_table.ac2000_platinum_house!=0||ac2000_table.ac2000_pro_house!=0)//PLATINUM 版
		{
				int32_t x=data[3]*256+data[4];//料塔今日用量
				int32_t y=data[5]*256+data[6];//料塔今日上量
				int32_t z=data[9]*256*256*256+data[10]*256*256+data[7]*256+data[8];//料塔当前料重
				Ac2000Data[ADDR_PLATINUM_2477] = x;//料塔今日用量
				RS485_1_DEBUG("料塔今日用量=%d\r\n",Ac2000Data[ADDR_PLATINUM_2477]);
				//DEBUG("料塔今日用量=%d\r\n",Ac2000Data[ADDR_1928]);
				
				Ac2000Data[ADDR_PLATINUM_2478] = y;//料塔今日上量
				RS485_1_DEBUG("料塔今日上量=%d\r\n",Ac2000Data[ADDR_PLATINUM_2478]);
				//DEBUG("料塔今日上量=%d\r\n",Ac2000Data[ADDR_1929]);
				
				Ac2000Data[ADDR_PLATINUM_2476] = z;//料塔当前料重
				RS485_1_DEBUG("料塔当前料重=%d\r\n",Ac2000Data[ADDR_PLATINUM_2476]);
				//DEBUG("料塔当前料重=%d\r\n",Ac2000Data[ADDR_1927]);
				
				int32_t m=data[75]*256+data[76];
				Ac2000Data[ADDR_PLATINUM_2479] = m;//料塔累计耗料
				RS485_1_DEBUG("料塔累计耗料=%d\r\n",Ac2000Data[ADDR_PLATINUM_2479]);
				//DEBUG("料塔累计耗料=%d\r\n",Ac2000Data[ADDR_1930]);
				food_weight_table.food_weigh_link++;
				food_weight_table.food_weight_mount=true;
		}
	}
	else
	{
		RS485_1_DEBUG("称重CRC校验失败\r\n");
	}
}
