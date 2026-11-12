#ifndef _USER_FSMACHINE_H_
#define _USER_FSMACHINE_H_

#include "mainOrbitCacheQueue.h"

extern void mainOrbitMaskInit(void);

extern void allCacheQueueInit(void);

extern void machine_work_process(void);

extern void set_package_param_1(void);

extern void set_package_param_2(void);

extern void set_package_param_3(void);

extern CacheCupStateDef *get_cache_cup_state(int ch);

#endif
