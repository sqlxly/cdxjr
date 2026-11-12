#ifndef __REPOSTCACHEQUEUE_H
#define __REPOSTCACHEQUEUE_H

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define 	REPOST_CACHE_LEN			4			//转载缓存共4排
#define 	REPOST_CACHE_WIDTH		12		//转载缓存宽度
#define 	s_WIDTH  							6			//单条检测线6个通道
// 鸡蛋所有特征的汇总信息
typedef struct
{
  uint16_t weigh[REPOST_CACHE_WIDTH];  	  //鸡蛋重量
	uint8_t  level[REPOST_CACHE_WIDTH];   	//裂纹等级
	uint8_t  dirty[REPOST_CACHE_WIDTH];   	//脏蛋等级
	uint8_t  color[REPOST_CACHE_WIDTH];   	//颜色等级
	uint8_t  toChannel[REPOST_CACHE_WIDTH];	//鸡蛋将被分配至哪一个通道
} EggsGatherInfo;

typedef struct
{
  EggsGatherInfo items[REPOST_CACHE_LEN];  //暂存转载缓存中4排鸡蛋的详细信息
	uint16_t  front;
	uint16_t  rear;
} EggsGatherQueue;

extern EggsGatherQueue gatherQueue;

extern uint8_t repost_complete_singal;
	 
//初始化转载缓存队列 开机启动时需要重置数据 转载缓存放入主轨道缓存后也需要重置数据
void initRepostCacheQueue(EggsGatherQueue *q);

//写入转载缓存队列
void enqueueRepostCacheQueue(EggsGatherQueue *q, EggsGatherInfo *v);

//从转载缓存队列读取数据
void dequeueRepostCacheQueue(EggsGatherQueue *q, EggsGatherInfo *v);

void addThreeLineEggsToRepostCache(void);

#endif
