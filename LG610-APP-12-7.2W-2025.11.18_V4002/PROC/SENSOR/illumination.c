#include "illumination.h"
void illumination_cmd(uint8_t *Buff)
{
	Buff[0] = 0x03;//0x05;	//Device Address	光照明度传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x00;	//Register Start Address
	Buff[3] = 0x07;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x02;	//DataLengthL
	Buff[6] = 0x74;	//CRCL
	Buff[7] = 0x28;	//CRCH
}

