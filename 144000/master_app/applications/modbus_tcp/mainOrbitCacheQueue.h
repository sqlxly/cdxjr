#ifndef __MAINORBITCACHEQUEUE_H
#define __MAINORBITCACHEQUEUE_H

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dev_config.h"

#define 	MAIN_ORBIT_QUEUE_VALID		140	 	//主轨道缓存队列有效长度为20 110

#define 	MAIN_ORBIT_QUEUE_SIZE			160	 	//主轨道缓存可用队列长度为140 预留20个数据缓冲区 150

#define 	SHOW_PAGE_QUEUE_VALID			48	 	//主轨道缓存展示数据队列有效长度为48

#define 	SHOW_PAGE_QUEUE_SIZE			60	 	//主轨道缓存可用展示数据队列长度为60 预留12个数据缓冲区

#define		MAIN_ORBIT_NUM						4		 	//共4条主轨道

typedef struct
{
	uint16_t weigh;  		//鸡蛋重量
	uint8_t  level;   	//裂纹等级
	uint8_t  dirty;   	//脏蛋等级
	uint8_t  color;   	//颜色等级
	uint8_t  toChannel;	//鸡蛋将被分配至哪一个通道
	uint8_t	 toTag;			//鸡蛋喷码序号
	uint8_t  isAct;			//蛋杯里面有蛋时为1 蛋被踢开或者空杯时为0
	uint8_t  orbitIndex;
} SingleEggInfo;

// 主轨道缓存展示蛋杯中鸡蛋所有特征的汇总信息
typedef struct
{
	uint16_t weigh[MAIN_ORBIT_NUM];  		//鸡蛋重量
	uint8_t  level[MAIN_ORBIT_NUM];   	//裂纹等级
	uint8_t  dirty[MAIN_ORBIT_NUM];   	//脏蛋等级
	uint8_t  color[MAIN_ORBIT_NUM];   	//颜色等级
	uint8_t  toChannel[MAIN_ORBIT_NUM];	//鸡蛋将被分配至哪一个通道
} EggsGatherInfoEx;

// 主轨道缓存展示蛋杯中当前鸡蛋信息的数据结构 只需要展示24组数据
typedef struct {
	EggsGatherInfoEx items[SHOW_PAGE_QUEUE_SIZE];	//主轨道展示前24个蛋杯鸡蛋信息的缓存
    uint16_t front;
    uint16_t rear;
} ShowPageCircularQueue;

// 主轨道缓存鸡蛋分类信息
typedef struct{
	uint8_t isAct[MAIN_ORBIT_NUM];    		//4条主轨道蛋杯里面有蛋时为1 蛋被踢开或者空杯时为0
	uint8_t toChannel[MAIN_ORBIT_NUM];   	//4条主轨道蛋杯中鸡蛋应该被放入哪一个通道
	uint8_t	toTag[MAIN_ORBIT_NUM];				//4条主轨道蛋杯中鸡蛋喷码序号
} EggsClassifyInfo;

// 主轨道缓存循环队列数据结构
typedef struct {
	EggsClassifyInfo items[MAIN_ORBIT_QUEUE_SIZE];   //主轨道缓存
    uint16_t front;
    uint16_t rear;
} MainOrbitCircularQueue;

typedef struct {	
	uint8_t state[MAIN_ORBIT_NUM]; 					//包装通道每个主轨道运行状态机
	uint16_t orbitDelay[MAIN_ORBIT_NUM];		//缓存蛋杯操作延时
	uint8_t mask;  													//实际用于踢蛋控制的掩膜 根据设置的包装盒为几行几列进行重新计算
	uint8_t isFull[MAIN_ORBIT_NUM]; 				//主轨道1 2 3 4下面的缓存蛋杯掩膜位置下面的第一层缓存蛋杯对应位置是否有鸡蛋
	uint8_t actBak;													//通道踢蛋状态扩展字 记录前一个踢蛋状态
} CacheCupStateDef;

extern MainOrbitCircularQueue mainOrbitQueue; 
extern ShowPageCircularQueue showPageQueue;			

// 初始化主轨道展示蛋杯缓存队列
void initShowPageQueue(ShowPageCircularQueue *q);

// 数据写入主轨道展示蛋杯缓存队列
void enqueueShowPageQueue(ShowPageCircularQueue *q, EggsGatherInfoEx *v);

// 从主轨道展示蛋杯缓存队列读取数据
void dequeueShowPageQueue(ShowPageCircularQueue *q, EggsGatherInfoEx *v);

// 初始化主轨道缓存队列
void initMainOrbitQueue(MainOrbitCircularQueue *q);

// 数据写入主轨道缓存队列
void enqueueMainOrbitQueue(MainOrbitCircularQueue *q, EggsClassifyInfo *v);

// 从主轨道缓存队列读取数据
void dequeueMainOrbitQueue(MainOrbitCircularQueue *q, EggsClassifyInfo *v);

#endif
