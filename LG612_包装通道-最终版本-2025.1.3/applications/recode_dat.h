
#ifndef _RECODE_DAT_H__
#define _RECODE_DAT_H__
#include <rtthread.h>


#define CFG_BKP_SRAM_SIZE 4096

uint8_t read_board_addr(void);
void set_board_addr(uint8_t board_addr);
#endif
