#include <rtthread.h>


#ifndef __PRODUCT_HOSTROY_H
#define __PRODUCT_HOSTROY_H





typedef struct
{
	
	uint16_t start_time;
	uint16_t stop_time;
	uint16_t product_name[10];
	uint16_t packge_name[10];
	uint16_t paqckge_line;
	uint16_t product_bar;
	uint32_t date;       
	uint32_t task_box;
	uint32_t product_box;
	uint32_t task_count;
	uint32_t product_count;
	
}already_produced_log_type;

#define EEP_ALREADY_PRODUCT_LEN (sizeof(already_produced_log_type)+2)	// 44+2=46
#define EEP_ALREADY_PRODUCT_COUNT 2048/EEP_ALREADY_PRODUCT_LEN				// 2048/46=44

void save_product_hostroy(uint8_t index);
void product_logs_hostoy_factry_init();
uint16_t get_product_hostroy_count(void);
uint16_t get_product_recode_current_offest(void);
void save_product_recode(already_produced_log_type *already_produced_log);
void get_pruduct_hostroy_log(already_produced_log_type *already_produced_log,uint16_t index);
uint32_t  get_current_porduct_index(void);
#endif  //__PRODUCT_HOSTROY_H