#ifndef __ECAT_DATA_PROC_H__
#define __ECAT_DATA_PROC_H__

#include <drv_common.h>

typedef struct {
	uint8_t pushTrig;
	uint8_t pushTrigBak;
} PushTrigSignal;

void ethercat_slave_recv_data_process(void);

void ethercat_slave_send_datat_process(uint16_t cmd);

extern PushTrigSignal tirgSignal[4];

#endif
