#include "electricity_meter.h"
void electricity_meter_cmd1(uint8_t *Buff)
{
	Buff[0] = 0x02;//0x07;	//Device Address	电表传感器
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x00;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x06;	//DataLengthL
	Buff[6] = 0x70;	//CRCL
	Buff[7] = 0x3b;	//CRCH
}
void electricity_meter_cmd2(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器电流采集
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x08;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x06;	//DataLengthL
	Buff[6] = 0xf1;	//CRCL
	Buff[7] = 0xac;	//CRCH
}
void electricity_meter_cmd3(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器采集有功功率与无功功率
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x10;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x10;	//DataLengthL
	Buff[6] = 0xf0;	//CRCL
	Buff[7] = 0x65;	//CRCH
}
void electricity_meter_cmd4(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器的功率因素
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x2a;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x06;	//DataLengthL
	Buff[6] = 0x51;	//CRCL
	Buff[7] = 0xa6;	//CRCH
}
void electricity_meter_cmd5(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器的频率
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x36;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x02;	//DataLengthL
	Buff[6] = 0x91;	//CRCL
	Buff[7] = 0xa3;	//CRCH
}
void electricity_meter_cmd6(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器的总有功电量
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x01;	//Register Start Address
	Buff[3] = 0x00;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x02;	//DataLengthL
	Buff[6] = 0x70;	//CRCL
	Buff[7] = 0x51;	//CRCH
}
void electricity_meter_cmd7(uint8_t *Buff)
{
	Buff[0] = 0x07;	//Device Address	电表传感器的总无功电量
	Buff[1] = 0x04;	//Function Code
	Buff[2] = 0x04;	//Register Start Address
	Buff[3] = 0x00;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x02;	//DataLengthL
	Buff[6] = 0x70;	//CRCL
	Buff[7] = 0x9d;	//CRCH
}
void electricity_meter_cmd8(uint8_t *Buff)
{
	Buff[0] = 0x01;	//Device Address	水表传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x00;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x04;	//DataLengthL
	Buff[6] = 0x44;	//CRCL
	Buff[7] = 0x09;	//CRCH
}
void electricity_meter_process(uint8_t *data)
{
	uint32_t TmpData;
	uint16_t electricity_meter[3];	
	for(uint8_t i=0;i<3;i++)
	{
		//得到三相ABC电压
			electricity_meter[i] = 0x00;
			electricity_meter[i] = (electricity_meter[i] + data[6+i*4])<<24;

			TmpData = 0x00;
			TmpData  = (TmpData + data[5+i*4])<<16;
			electricity_meter[i]=electricity_meter[i]+TmpData;

			TmpData = 0x00;
			TmpData  = (TmpData + data[4+i*4])<<8;
			electricity_meter[i]=electricity_meter[i]+TmpData;

			TmpData = 0x00;
			TmpData  = (TmpData + data[3+i*4])<<0;
			electricity_meter[i]=electricity_meter[i]+TmpData;
		}
}
