#ifndef _USER_SORTER_H_
#define _USER_SORTER_H_

#include <rtthread.h>
#include "production_param.h"
#include "mainOrbitCacheQueue.h"

//定义分类器工作模式
enum SortModeEnum{
	eUnknowMode 	= 0,	//未知模式
	eAmountMode 	= 1,  //数量均分模式
	eWeighMode		= 2,	//重量均分模式
	eFreeMode 		= 3,  //自由分配模式
};

extern void eggs_sort_actuator(SingleEggInfo *egg, WorkRecipeParam *param);	//鸡蛋分类执行器 鸡蛋分类时 先从异常蛋开始分类 减少计算量

#endif
