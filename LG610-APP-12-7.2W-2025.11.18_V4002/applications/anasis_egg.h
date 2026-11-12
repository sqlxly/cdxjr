
#include <rtthread.h>
#include "product_set.h"

#ifndef _ANASIS_COM_H__
#define _ANASIS_COM_H__

#define BACKUP_SRAM_FLAG 0x11223344
#define ALREAD_PRODUCT_DAT_OFFEST_ADDR  0
#define PER_PRODUCT_DAT_OFFEST_ADDR     (sizeof(alread_production_type) +4)
#define PRODUCT_HTROW_CH_PART_OFFEST_ADDR (PER_PRODUCT_DAT_OFFEST_ADDR + sizeof(production_type) + 4)
#define RECODE_ANAYSIS_EGG_DAT_LEN (4086 -(PRODUCT_HTROW_CH_PART_OFFEST_ADDR+sizeof(throw_ch_product_type)))
	
#define RECODE_ANAYSIS_EGG_DAT_OFFEST_ADDR (PRODUCT_HTROW_CH_PART_OFFEST_ADDR+sizeof(throw_ch_product_type)+4)
	


#endif  	// _USER_SET_H__
