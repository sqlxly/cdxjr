#ifndef __ELECTRIC_CURRENT_H
#define __ELECTRIC_CURRENT_H

#include "stm32h7xx.h"
#include <stdbool.h>

#define EN_ELECTRIC_CURRENT_SENSOR    1

#define ELECTRIC_CURRENT_CMD_NUM      4
#define ELECTRIC_CURRENT_CMD_LEN      8

#define ELECTRIC_CURRENT_RECV_LEN     37

#define ELECTRIC_CURRENT_RETRY        2
typedef struct electric_current
{
	bool electric_current_mount[ELECTRIC_CURRENT_CMD_NUM];
	uint8_t electric_current_link[ELECTRIC_CURRENT_CMD_NUM];
	uint8_t electric_current_retry[ELECTRIC_CURRENT_CMD_NUM];
	uint16_t electric_current_value[ELECTRIC_CURRENT_CMD_NUM][16];

}ELECTRIC_CURRENT;

extern ELECTRIC_CURRENT electric_current_table;
extern const uint8_t electric_current_cmd[ELECTRIC_CURRENT_CMD_NUM][ELECTRIC_CURRENT_CMD_LEN];
extern void electric_current_process(uint8_t *data,int len);
#endif
