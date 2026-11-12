
#include <user_set.h>
#include <rtthread.h>
#include "common.h"
#include "product_set.h"
#include "product_egg_recode.h"

#ifndef __DAT_BACKUPH__
#define __DAT_BACKUPH__


	

void backup_save_analysis_egg_dat(recode_analisys_product_type *dat);
void backup_read_analysis_egg_dat(recode_analisys_product_type *dat);	
void read_bak_per_pruduct_dat(production_type *dat);
void backup_save_throw_ch_part(throw_ch_product_type *dat);
void backup_read_throw_ch_part(throw_ch_product_type *dat);
void backup_save_alread_pruduct_dat(alread_production_type *dat);
void backup_read_alread_pruduct_dat(alread_production_type *dat);
void backup_read_per_pruduct_dat(production_type *dat);
void backup_save_per_pruduct_dat(production_type *dat);
void show_save_addr(void);
#endif /*__DAT_BACKUPH__ */
