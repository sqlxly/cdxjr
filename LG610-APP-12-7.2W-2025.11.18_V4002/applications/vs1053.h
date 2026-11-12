
#include <rtthread.h>
#include <board.h>
#ifndef _VS1053_H_
#define _VS1053_H_


#define VS1053_CS_PORT GPIOA
#define VS1053_CS_PIN  GPIO_PIN_10

#define VS_WRITE_COMMAND 	0x02
#define VS_READ_COMMAND 	0x03
//VS10XX寄存器定义
#define SPI_MODE        	0x00   
#define SPI_STATUS      	0x01   
#define SPI_BASS        	0x02   
#define SPI_CLOCKF      	0x03   
#define SPI_DECODE_TIME 	0x04   
#define SPI_AUDATA      	0x05   
#define SPI_WRAM        	0x06   
#define SPI_WRAMADDR    	0x07   
#define SPI_HDAT0       	0x08   
#define SPI_HDAT1       	0x09 
  
#define SPI_AIADDR      	0x0a   
#define SPI_VOL         	0x0b   
#define SPI_AICTRL0     	0x0c   
#define SPI_AICTRL1     	0x0d   
#define SPI_AICTRL2     	0x0e   
#define SPI_AICTRL3     	0x0f   
#define SM_DIFF         	0x01   
#define SM_JUMP         	0x02   
#define SM_RESET        	0x04   
#define SM_OUTOFWAV     	0x08   
#define SM_PDOWN        	0x10   
#define SM_TESTS        	0x20   
#define SM_STREAM       	0x40   
#define SM_PLUSV        	0x80   
#define SM_DACT         	0x100   
#define SM_SDIORD       	0x200   
#define SM_SDISHARE     	0x400   
#define SM_SDINEW       	0x800   
#define SM_ADPCM        	0x1000   
#define SM_ADPCM_HP     	0x2000 		 

#define I2S_CONFIG			0XC040
#define GPIO_DDR			0XC017
#define GPIO_IDATA			0XC018
#define GPIO_ODATA			0XC019


__packed typedef struct 
{							  
	uint8_t mvol;		//主音量,范围:0~254
	uint8_t bflimit;		//低音频率限定,范围:2~15(单位:10Hz)
	uint8_t bass;		//低音,范围:0~15.0表示关闭.(单位:1dB)
	uint8_t tflimit;		//高音频率限定,范围:1~15(单位:Khz)
	uint8_t treble;		//高音,范围:0~15(单位:1.5dB)(原本范围是:-8~7,通过函数修改了);
	uint8_t effect;		//空间效果设置.0,关闭;1,最小;2,中等;3,最大.
	uint8_t speakersw;	//板载喇叭开关,0,关闭;1,打开 
	uint8_t saveflag; 	//保存标志,0X0A,保存过了;其他,还从未保存	   
}_vs10xx_obj;

void VS_Set_All(void);
void VS_Soft_Reset(void);
uint8_t VS_HD_Reset(void);
void VS_Reset_DecodeTime(void);
uint8_t VS_Send_MusicData(uint8_t* wbuf,uint16_t len);
void VS_Sine_Test(void);
uint16_t VS_Ram_Test(void);
void VS_Sine_Test(void);
rt_err_t vs1053_init(void);
uint8_t f_typetell(uint8_t *fname);
void inter_alarm(uint8_t dat);
void inter_alarm_stop(uint8_t dat);
void stop_play_music();
void set_play_music(uint8_t id,uint8_t replay);
void VS_Load_Patch(uint16_t *patch,uint16_t len);

#endif  	// #define _MDEBUG_H__
