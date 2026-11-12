#include "ammonia.h"

void ammonia_cmd(uint8_t *Buff)
{
	Buff[0] = 0x02;  //0x08;	//Device Address	氨气传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x06;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x01;	//DataLengthL
	Buff[6] = 0x64;	//CRCL
	Buff[7] = 0x38;	//CRCH
}
void ammonia_process(uint8_t *data)
{
	//uint16_t NH3=(data[3]<<8)+data[4];
}
