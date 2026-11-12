#include "electric_current.h"
#include "bsp_clkconfig.h"
#include "data_table.h"
ELECTRIC_CURRENT electric_current_table;
const uint8_t electric_current_cmd[ELECTRIC_CURRENT_CMD_NUM][ELECTRIC_CURRENT_CMD_LEN]=/*命令码表*/
{
	{0x01,  0x16,	0x00,	0x00,	0x00,	0x20,	0x49,	0xd1},
	{0x02,	0x16,	0x00,	0x00,	0x00,	0x20,	0x49,	0xe2},	
	{0x03,	0x16,	0x00,	0x00,	0x00,	0x20,	0x48,	0x33},
  {0x04,	0x16,	0x00,	0x00,	0x00,	0x20,	0x49,	0x84}
};
void electric_current_process(uint8_t *data,int len)
{
	//01 16 20 0b dc 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 50 04 
	unsigned int crc1=server_crc_chk(data,(len-2));
	unsigned int crc2=data[len-2]*256+data[len-1];
	if(crc1==crc2)
	{
			switch (data[0])
			{
				case 0x01:
					for(uint8_t i=0;i<16;i++)
					{
						electric_current_table.electric_current_value[0][i] = (data[3+i*2]<<8) + (data[4+i*2]<<0);
						electric_current_table.electric_current_link[0]++;
						electric_current_table.electric_current_mount[0]=true;
						//RS485_1_DEBUG("电流板1 %d=%d\r\n",i,electric_current_table.electric_current_value[0][i]);
					}			
					break;
				case 0x02:
					for(uint8_t i=0;i<16;i++)
					{
						electric_current_table.electric_current_value[1][i] = (data[3+i*2]<<8) + (data[4+i*2]<<0);
						electric_current_table.electric_current_link[1]++;
						electric_current_table.electric_current_mount[1]=true;
						//RS485_1_DEBUG("电流板2 %d=%d\r\n",i,electric_current_table.electric_current_value[1][i]);
					}		
					break;
				case 0x03:
					for(uint8_t i=0;i<16;i++)
					{
						electric_current_table.electric_current_value[2][i] = (data[3+i*2]<<8) + (data[4+i*2]<<0);
						electric_current_table.electric_current_link[2]++;
						electric_current_table.electric_current_mount[2]=true;
						//RS485_1_DEBUG("电流板3 %d=%d\r\n",i,electric_current_table.electric_current_value[2][i]);
					}					
				break;
				case 0x04:
					for(uint8_t i=0;i<16;i++)
					{
						electric_current_table.electric_current_value[3][i] = (data[3+i*2]<<8) + (data[4+i*2]<<0);
						electric_current_table.electric_current_link[3]++;
						electric_current_table.electric_current_mount[3]=true;
						//RS485_1_DEBUG("电流板3 %d=%d\r\n",i,electric_current_table.electric_current_value[2][i]);
					}					
				break;
				default:break;
			}

	}
	else
	{
		RS485_1_DEBUG("电流板CRC校验失败\r\n");
	}
}

