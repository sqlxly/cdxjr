#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

#include "qrcode.h"
#include "font.h"
#include "common.h"
#define DEFAULT_QR_VERSION 3



void create_qrcode_bmp(void)
{
		QRCode qrc;
		uint8_t i = 0, pos = 0, ok = 0, ch = 0;
    uint8_t x, y, *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION)); //size = 106
    int8_t result;
    char *qrstr = stm32_uniqueId;
		
		readStm32UniqueID();

    if (qrcodeBytes)
    {
        result = qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, qrstr);
		rt_kprintf("qrc.size: %d\n",qrc.size);
        if (result >= 0)
        {
            for (y = 0; y < qrc.size; y++)
            {
                for (x = 0; x < qrc.size; x++)  //qrc.size = 29
                {
										pos = y%4;
										ok = qrcode_getModule(&qrc, x, y);
									  switch(pos){
									  	case 0:
									  		if(ok) ch = 0x03;
									  		else ch = 0;
									  		for(i=0; i<2; i++){
									  			qrcodeData[(y/4)*58+x*2+i] = ch;
									  		}
									  		break;
									  	case 1:
									  		if(ok) ch = 0x0C;
									  		else ch = 0;
									  		for(i=0; i<2; i++){
									  			qrcodeData[(y/4)*58+x*2+i] |= ch;
									  		}
									  		break;
									  	case 2:
									  		if(ok) ch = 0x30;
									  		else ch = 0;
									  		for(i=0; i<2; i++){
									  			qrcodeData[(y/4)*58+x*2+i] |= ch;
									  		}
									  		break;
									  	case 3:
									  		if(ok) ch = 0xC0;
									  		else ch = 0;
									  		for(i=0; i<2; i++){
									  			qrcodeData[(y/4)*58+x*2+i] |= ch;
									  		}
									  		break;
									  }
                }
            }
        }
        else
        {
            rt_kprintf("QR CODE(%s) General FAILED(%d)\n", qrstr, result);
        }
        rt_free(qrcodeBytes);
    }
    else
    {
        rt_kprintf("Warning: no memory!\n");
    }
}
#include <u8g2_port.h>
extern u8g2_t u8g2;
void create_u8g2_qrcode_bmp(uint8_t index_x, uint8_t index_y)
{
	QRCode qrc;
    uint8_t *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION)); //size = 106
//    int8_t result;
    char *qrstr = stm32_uniqueId;
	qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, qrstr);
    uint8_t x0 = index_x;
    uint8_t y0 = index_y;
    
    // get QR code pixels in a loop
    for (uint8_t y = 0; y < qrc.size; y++) {
      for (uint8_t x = 0; x < qrc.size; x++) {
        // Check this point is black or white
        if (qrcode_getModule(&qrc, x, y)) {
          u8g2_SetDrawColor(&u8g2,1);
        } else {
          u8g2_SetDrawColor(&u8g2,0);
        }
        // Double the QR code pixels
        u8g2_DrawPixel(&u8g2,x0 + x * 2, y0 + y * 2);
        u8g2_DrawPixel(&u8g2,x0 + 1 + x * 2, y0 + y * 2);
        u8g2_DrawPixel(&u8g2,x0 + x * 2, y0  + 1 + y * 2);
        u8g2_DrawPixel(&u8g2,x0 + 1 + x * 2, y0 + 1 + y * 2);
      }
    }
	rt_free(qrcodeBytes);
}

