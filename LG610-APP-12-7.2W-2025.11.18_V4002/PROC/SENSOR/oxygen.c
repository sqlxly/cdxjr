#include "oxygen.h"

void oxygen_cmd(uint8_t *Buff)
{
	Buff[0] = 0x03;//0x01;	//Device Address	氧气传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x06;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x01;	//DataLengthL
	Buff[6] = 0x65;	//CRCL
	Buff[7] = 0xe9;	//CRCH
}
void oxygen_process(uint8_t *data)
{
	//uint16_t oxygen=(data[3]<<8)+data[4];
}
