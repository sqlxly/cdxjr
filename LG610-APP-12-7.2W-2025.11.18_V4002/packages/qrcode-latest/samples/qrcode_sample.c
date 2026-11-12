#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdio.h>

#include "qrcode.h"
#include "font.h"

#define UID_BASEADDR 		0x1FFF7A10

#define DEFAULT_QR_VERSION 3
#define DEFAULT_QR_STRING "XIAO JU REN"

void readStm32UniqueID(void) 
{
	rt_uint32_t uid[3];

  uid[0] = *(__IO uint32_t *)(UID_BASEADDR);
  uid[1] = *(__IO uint32_t *)(UID_BASEADDR + 0x04);
  uid[2] = *(__IO uint32_t *)(UID_BASEADDR + 0x08);
	sprintf(stm32_uniqueId, "%08x%08x%08x", uid[0], uid[1], uid[2]);
	rt_kprintf("UID:%s\n", stm32_uniqueId);
}

void qrcode_bmp_to_lcd_jpg(void)
{
		QRCode qrc;
		uint8_t i = 0;
    uint8_t x, y, *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION)); //size = 106
    int8_t result;
    char *qrstr = stm32_uniqueId;
		
		readStm32UniqueID();
	
    if (qrcodeBytes)
    {
        result = qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, qrstr);

        if (result >= 0)
        {
            for (y = 0; y < qrc.size; y++)
            {
                for (x = 0; x < qrc.size; x++)  //qrc.size = 29
                {
                    if (qrcode_getModule(&qrc, x, y))
                    {
                       if((y%2)==0){
													for(i=0; i<4; i++){
														qrcodeData[(y/2)*116+x*4+i] = 0xf0;
													}
											 }else{
													for(i=0; i<4; i++){
														qrcodeData[(y/2)*116+x*4+i] |= 0x0f;
													}
											 }
                    }
                    else
                    {
                       if((y%2)==0){
													for(i=0; i<4; i++){
														qrcodeData[(y/2)*116+x*4+i] = 0x00;
													}												
											 }else{
													for(i=0; i<4; i++){
														qrcodeData[(y/2)*116+x*4+i] |= 0x00;
													}												
											 }
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

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void qrcode(uint8_t argc, char **argv)
{
    QRCode qrc;
    uint8_t x, y, *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION)); //size = 106
    int8_t result;
    char *qrstr = DEFAULT_QR_STRING;

    if (qrcodeBytes)
    {
        if (argc > 1)
        {
            qrstr = argv[1];
        }

        result = qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, qrstr);

        if (result >= 0)
        {
            for (y = 0; y < qrc.size; y++)
            {
                for (x = 0; x < qrc.size; x++)  //qrc.size = 29
                {
                    if (qrcode_getModule(&qrc, x, y))
                    {
                        rt_kprintf("\xdb\xdb");
                    }
                    else
                    {
                        rt_kprintf("  ");
                    }
                }
                rt_kprintf("\n");
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
MSH_CMD_EXPORT(qrcode, qrcode generator: qrcode [string]);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
