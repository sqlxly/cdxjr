
#include <rtthread.h>
#ifndef __DRV74HCXX_H
#define __DRV74HCXX_H

#define SN74HCXX_SCL GET_PIN(C, 2)
#define SN74HCXX_DAT GET_PIN(C, 3)
#define SN74HCXX_LD  GET_PIN(A, 0)

void sn74hcxx_get_dat(uint8_t *rx_dat,uint8_t len);

#endif  //__DRV74HCXX_H
