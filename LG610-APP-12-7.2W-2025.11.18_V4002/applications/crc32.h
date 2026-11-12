#include <rtthread.h>

#ifndef __CRC32_H
#define __CRC32_H
#include "stdint.h"

rt_uint32_t hal_crc32(uint8_t *buf,uint16_t len);
unsigned int CRC32_file(unsigned char* buf, unsigned int len);


unsigned int CALCRC32_get(void);
unsigned int CALCRC32_set( unsigned int crc32);
unsigned int CALCRC32_file(unsigned char* buf, unsigned int len);

#endif /* __MAIN_H */


