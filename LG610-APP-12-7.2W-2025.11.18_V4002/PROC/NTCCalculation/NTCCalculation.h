#ifndef NTCCALCULATION_H_
#define NTCCALCULATION_H_

#define NTC_TABLE_Length 16
#define NTC_TABLE_Minimum_Temperature -40
#define NTC_TABLE_Temperature_Tnterval 10.0

unsigned char lookUpTable(const unsigned short *a,unsigned char ArrayLong,unsigned short data);
float numToTemperature(unsigned char num);
float calculationGetNtc(unsigned short data);

#endif


