#ifndef __FOOD_WEIGHT_H
#define __FOOD_WEIGHT_H

#include "stm32h7xx.h"
#include <stdbool.h>
#define EN_WEIGHT_SENSOR    1
#define WEIGHT_CMD_LENGTH   8
#define WEIGHT_RECV_LENGTH  79

#define WEIGHT_LINK_RETYRY  2
typedef struct food_weight
{
	bool food_weight_mount;
	uint8_t food_weigh_link;
	uint8_t food_weigh_retry;
}FOOD_WEIGHT;

extern FOOD_WEIGHT food_weight_table;
extern const uint8_t weight_cmd[WEIGHT_CMD_LENGTH];

void food_weight_process(uint8_t *data,int length);

#endif
