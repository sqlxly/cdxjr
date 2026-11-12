#include "waterline.h"
#include "bsp_clkconfig.h"
#include "data_table.h"
const uint8_t waterline_cmd[WATERLINE_CMD_LENGTH]={0xee,0x48,0x02,0x05,0x74,0x79};
WATERLINE waterline_table;

void waterline_process(uint8_t *dataBuff,uint16_t length)
{
	
	//EE 48 32 00 00 00 00 00 01 11 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05 01 A7 3F
	unsigned int i=3;
	unsigned int water_alarm=0;
	unsigned char tongxingshibai=0;
	unsigned int crc1=server_crc_chk(dataBuff,(length-2));
	unsigned int crc2=dataBuff[length-2]*256+dataBuff[length-1];
	if(crc1==crc2&&Ac2000Data!=NULL)
	{	
		i=3;
		water_alarm=0;
		for(uint8_t k=0;k<48;k++)
		{
			if(dataBuff[i]==0x00)// 有水
			{
				if(ac2000_table.ac2000_3g_house!=0)//3G版
				{
					Ac2000Data[ADDR_3G_1867+k]=2;
				}
				else if(ac2000_table.ac2000_platinum_house!=0)//PLATINUM 版
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=2;
				}
				else if(ac2000_table.ac2000_pro_house!=0)
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=2;
				}
			}
			else if(dataBuff[i]==0x11)//无水已达到报警条件
			{
				water_alarm=stateBinValue;//饮水监测报警状态 有报警则为1
				if(ac2000_table.ac2000_3g_house!=0)//3G版
				{
					Ac2000Data[ADDR_3G_1867+k]=1;
				}
				else if(ac2000_table.ac2000_platinum_house!=0)//PLATINUM 版
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=1;
				}
				else if(ac2000_table.ac2000_pro_house!=0)
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=1;
				}
			}
			else if(dataBuff[i]==0x01)//无水未达到报警条件
			{
				if(ac2000_table.ac2000_3g_house!=0)//3G版
				{
					Ac2000Data[ADDR_3G_1867+k]=0;
				}
				else if(ac2000_table.ac2000_platinum_house!=0)//PLATINUM 版
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=0;
				}
				else if(ac2000_table.ac2000_pro_house!=0)
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=0;
				}
			}
			else if(dataBuff[i]==0x10)//灯板的采集板通信失败
			{
				if(ac2000_table.ac2000_3g_house!=0)//3G版
				{
					Ac2000Data[ADDR_3G_1867+k]=3;
				}
				else if(ac2000_table.ac2000_platinum_house!=0)//PLATINUM 版
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=3;
				}
				else if(ac2000_table.ac2000_pro_house!=0)
				{
					Ac2000Data[ADDR_PLATINUM_2486+k]=3;
				}
				tongxingshibai=1;
			}
			i++;
		}
	
		waterline_table.waterline_alarm_delay=dataBuff[i++];//报警延时
		waterline_table.waterline_alarm_on=dataBuff[i++];//报警开启
		waterline_table.waterline_alarm_state=water_alarm;
		
		if(ac2000_table.ac2000_3g_house!=0)//3G版
		{
			Ac2000Data[ADDR_3G_1924]=waterline_table.waterline_alarm_delay;//报警延时
			Ac2000Data[ADDR_3G_1925]=waterline_table.waterline_alarm_on;//报警开启
			Ac2000Data[ADDR_3G_1926]=tongxingshibai;////采集板和控制板的通信状态
			Ac2000Data[ADDR_3G_1866]=waterline_table.waterline_alarm_state;//饮水监测报警状态 有报警则为1
			
			for(i=0;i<48;i++)
			{
				if(Ac2000Data[ADDR_3G_1867+i]==0x01)
				{
					AlarmCode3g[ALARM_3G_11433+i][1]=1;
				}
				else
				{
					AlarmCode3g[ALARM_3G_11433+i][1]=0;
				}
				//RS485_1_DEBUG("Ac2000Data[ADDR_3G_1867+%d]=%02X\r\n",i,Ac2000Data[ADDR_PLATINUM_2486+i]);
			}
		}
		else if(ac2000_table.ac2000_platinum_house!=0||ac2000_table.ac2000_pro_house!=0)//PLATINUM 版
		{
			Ac2000Data[ADDR_PLATINUM_2534]=waterline_table.waterline_alarm_delay;//报警延时
			Ac2000Data[ADDR_PLATINUM_2535]=waterline_table.waterline_alarm_on;//报警开启
			Ac2000Data[ADDR_PLATINUM_2536]=tongxingshibai;////采集板和控制板的通信状态
			Ac2000Data[ADDR_PLATINUM_2485]=waterline_table.waterline_alarm_state;//饮水监测报警状态 有报警则为1
			for(i=0;i<48;i++)
			{
				if(Ac2000Data[ADDR_PLATINUM_2486+i]==0x01)
				{
					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=1;
				}
				else
				{
					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=0;
				}
				//RS485_1_DEBUG("Ac2000Data[ADDR_3G_1867+%d]=%02X\r\n",i,Ac2000Data[ADDR_PLATINUM_2486+i]);
			}
		//		RS485_1_DEBUG("饮水设备报警状态=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2485]);
		//		RS485_1_DEBUG("缺水设备延时=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2534]);
		//		RS485_1_DEBUG("缺水设备监测开启=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2535]);
		//		RS485_1_DEBUG("缺水采集盒通信=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2536]);
		}
		
		RS485_1_DEBUG("缺水设备延时=%02X\r\n",waterline_table.waterline_alarm_delay);
		RS485_1_DEBUG("缺水设备监测开启=%02X\r\n",waterline_table.waterline_alarm_on);//报警开启
		RS485_1_DEBUG("饮水设备报警状态=%02X\r\n",waterline_table.waterline_alarm_state);
		waterline_table.waterline_link++;
		waterline_table.waterline_mount=true;	
	}
	else
	{
		RS485_1_DEBUG("缺水CRC校验失败\r\n");
	}
}



//void waterline_process(uint8_t *dataBuff,uint16_t length)
//{
//	//EE 48 32 00 00 00 00 00 01 11 00 10 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05 01 A7 3F
//	unsigned int i=3;
//	unsigned char state=0;
//	unsigned char tongxingshibai=0;
//	unsigned int crc1=server_crc_chk(dataBuff,(length-2));
//	unsigned int crc2=dataBuff[length-2]*256+dataBuff[length-1];
//	//my_log(dataBuff,length);
//	if(crc1==crc2&&Ac2000Data!=NULL)
//	{
//		if(ac2000_table.ac2000_3g_house!=0)//3G版
//		{
//			//0x00有水正常   0x01缺水但是没有达到报警超时的条件   0x11缺水已经达到报警超时   0x10控制器与采集板通信失败
//			Ac2000Data[ADDR_3G_1867]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1868]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1869]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1870]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1871]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1872]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1873]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1874]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1875]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1876]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1877]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1878]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1879]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1880]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1881]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1882]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1883]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1884]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1885]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1886]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1887]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1888]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1889]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1890]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1891]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1892]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1893]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1894]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1895]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1896]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1897]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1898]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1899]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1900]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1901]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1902]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1903]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1904]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1905]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1906]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1907]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1908]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1909]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1910]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1911]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1912]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1913]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_3G_1914]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			
//			Ac2000Data[ADDR_3G_1924]=dataBuff[i++];//报警延时
//			Ac2000Data[ADDR_3G_1925]=dataBuff[i++];//报警开启
//			Ac2000Data[ADDR_3G_1926]=tongxingshibai;////采集板和控制板的通信状态
//			Ac2000Data[ADDR_3G_1866]=state;//饮水监测报警状态 有报警则为1
//												
//			for(i=0;i<48;i++)
//			{
//				if(Ac2000Data[ADDR_3G_1867+i]==0x11)
//				{
//					AlarmCode3g[ALARM_3G_11433+i][1]=1;
//				}
//				else
//				{
//					AlarmCode3g[ALARM_3G_11433+i][1]=0;
//				}
//				//RS485_1_DEBUG("Ac2000Data[ADDR_3G_1867+%d]=%02X\r\n",i,Ac2000Data[ADDR_PLATINUM_2486+i]);
//			}
//	//		RS485_1_DEBUG("饮水设备报警状态=%02X\r\n",Ac2000Data[ADDR_3G_1866]);
//	//		RS485_1_DEBUG("缺水设备连接状态=%02X\r\n",Ac2000Data[ADDR_3G_1923]);
//	//		RS485_1_DEBUG("缺水设备延时=%02X\r\n",Ac2000Data[ADDR_3G_1924]);
//	//		RS485_1_DEBUG("缺水设备监测开启=%02X\r\n",Ac2000Data[ADDR_3G_1925]);
//	//		RS485_1_DEBUG("缺水采集盒通信=%02X\r\n",Ac2000Data[ADDR_3G_1926]);
//			
//			waterline_table.waterline_link++;
//			waterline_table.waterline_mount=true;
//		}
//		else if(ac2000_table.ac2000_platinum_house!=0)//PLATINUM 版
//		{
//			//0x00有水正常   0x01缺水但是没有达到报警超时的条件   0x11缺水已经达到报警超时   0x10控制器与采集板通信失败
//			Ac2000Data[ADDR_PLATINUM_2486]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2487]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2488]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2489]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2490]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2491]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2492]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2493]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2494]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2495]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2496]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2497]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2498]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2499]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2500]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2501]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2502]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2503]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2504]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2505]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2506]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2507]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2508]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2509]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2510]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2511]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2512]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2513]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2514]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2515]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2516]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2517]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2518]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2519]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2520]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2521]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2522]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2523]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2524]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2525]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2526]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2527]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2528]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2529]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2530]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2531]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2532]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2533]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//																	 

//			Ac2000Data[ADDR_PLATINUM_2534]=dataBuff[i++];//报警延时
//			Ac2000Data[ADDR_PLATINUM_2535]=dataBuff[i++];//报警开启
//			Ac2000Data[ADDR_PLATINUM_2536]=tongxingshibai;////采集板和控制板的通信状态
//			Ac2000Data[ADDR_PLATINUM_2485]=state;//饮水监测报警状态 有报警则为1
//												
//			for(i=0;i<48;i++)
//			{
//				if(Ac2000Data[ADDR_PLATINUM_2486+i]==0x11)
//				{
//					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=1;
//				}
//				else
//				{
//					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=0;
//				}
//				//RS485_1_DEBUG("Ac2000Data[ADDR_3G_1867+%d]=%02X\r\n",i,Ac2000Data[ADDR_PLATINUM_2486+i]);
//			}
//	//		RS485_1_DEBUG("饮水设备报警状态=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2485]);
//	//		RS485_1_DEBUG("缺水设备延时=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2534]);
//	//		RS485_1_DEBUG("缺水设备监测开启=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2535]);
//	//		RS485_1_DEBUG("缺水采集盒通信=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2536]);
//			
//			waterline_table.waterline_link++;
//			waterline_table.waterline_mount=true;			
//		}
//		else if(ac2000_table.ac2000_pro_house!=0)
//		{
//			//0x00有水正常   0x01缺水但是没有达到报警超时的条件   0x11缺水已经达到报警超时   0x10控制器与采集板通信失败
//			Ac2000Data[ADDR_PLATINUM_2486]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2487]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2488]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2489]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2490]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2491]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2492]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2493]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2494]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2495]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2496]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2497]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2498]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2499]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2500]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2501]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2502]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2503]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2504]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2505]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2506]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2507]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2508]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2509]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2510]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2511]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2512]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2513]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2514]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2515]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2516]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2517]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2518]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2519]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2520]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2521]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2522]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2523]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2524]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2525]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2526]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2527]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2528]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2529]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2530]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2531]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2532]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//			Ac2000Data[ADDR_PLATINUM_2533]=dataBuff[i++];if(dataBuff[i-1]==0x11)state=stateBinValue;if(dataBuff[i-1]==0x10)tongxingshibai=stateBinValue;
//																	 

//			Ac2000Data[ADDR_PLATINUM_2534]=dataBuff[i++];//报警延时
//			Ac2000Data[ADDR_PLATINUM_2535]=dataBuff[i++];//报警开启
//			Ac2000Data[ADDR_PLATINUM_2536]=tongxingshibai;////采集板和控制板的通信状态
//			Ac2000Data[ADDR_PLATINUM_2485]=state;//饮水监测报警状态 有报警则为1
//												
//			for(i=0;i<48;i++)
//			{
//				if(Ac2000Data[ADDR_PLATINUM_2486+i]==0x11)
//				{
//					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=1;
//				}
//				else
//				{
//					AlarmCodePlatinum[ALARM_PLATINUM_13201+i][1]=0;
//				}
//				//RS485_1_DEBUG("Ac2000Data[ADDR_3G_1867+%d]=%02X\r\n",i,Ac2000Data[ADDR_PLATINUM_2486+i]);
//			}
//	//		RS485_1_DEBUG("饮水设备报警状态=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2485]);
//	//		RS485_1_DEBUG("缺水设备延时=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2534]);
//	//		RS485_1_DEBUG("缺水设备监测开启=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2535]);
//	//		RS485_1_DEBUG("缺水采集盒通信=%02X\r\n",Ac2000Data[ADDR_PLATINUM_2536]);
//			
//			waterline_table.waterline_link++;
//			waterline_table.waterline_mount=true;		
//		}
//	}
//	else
//	{
//		RS485_1_DEBUG("缺水CRC校验失败\r\n");
//	}
//}




