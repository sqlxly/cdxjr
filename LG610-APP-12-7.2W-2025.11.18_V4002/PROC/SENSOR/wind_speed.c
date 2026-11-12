#include "wind_speed.h"
void wind_speed_cmd(uint8_t *Buff)
{
	Buff[0] = 0x02;	//Device Address	风速传感器
	Buff[1] = 0x03;	//Function Code
	Buff[2] = 0x06;	//Register Start Address
	Buff[3] = 0x00;	//Register Start Address
	Buff[4] = 0x00;	//DataLengthH
	Buff[5] = 0x01;	//DataLengthL
	Buff[6] = 0x84;	//CRCL
	Buff[7] = 0xb1;	//CRCH
}
