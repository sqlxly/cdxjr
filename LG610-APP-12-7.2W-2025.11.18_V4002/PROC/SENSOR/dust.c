#include "dust.h"
void dust_cmd(uint8_t *Buff)
{
	Buff[0] = 0x04;//0x06;	//Device Address	粉尘传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x04;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x06;	//DataLengthL
	Buff[6] = 0x84;	//CRCL
	Buff[7] = 0x5c;	//CRCH
}
