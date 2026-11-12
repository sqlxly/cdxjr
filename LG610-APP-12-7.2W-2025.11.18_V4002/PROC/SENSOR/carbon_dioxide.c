#include "carbon_dioxide.h"

void carbon_dioxide_cmd(uint8_t *Buff)
{
	Buff[0] = 0x01;//0x04;	//Device Address	二氧化碳传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x05;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x01;	//DataLengthL
	Buff[6] = 0x94;	//CRCL
	Buff[7] = 0x0b;	//CRCH
}
void carbon_dioxide_process(uint8_t *data)
{
	//uint16_t carbon_dioxide=(data[3]<<8)+data[4];
}
