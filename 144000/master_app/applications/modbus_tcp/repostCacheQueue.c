#include "repostCacheQueue.h"
#include "weighAndCrackQueue.h"
#include "dirtyAndColorEggsQueue.h"

/**** 实现1个高效数据队列 将输送通道的鸡蛋放入4排转载缓存中 ****/

EggsGatherQueue gatherQueue;  // 暂存转载缓存中4排鸡蛋的详细信息

uint8_t repost_complete_singal = 0;

/* 转载缓存逻辑功能：
 * 1.根据一排鸡蛋移入缓存的脉冲信号开始计算
 * 2.将脏蛋颜色队列和称重裂纹队列收集到的脏蛋 颜色 重量 裂纹等信息全部整合在一起(如果对应的检测设备是启用状态)
 * 3.根据分级配置文件 计算鸡蛋需要被分配到哪个包装通道中去 并为鸡蛋打上对应的标签
 * 4.根据将转载缓存中的鸡蛋放入主轨道缓存的信号 将转载缓存中的数据加入到主轨道队列中去
 */

//初始化转载缓存队列 开机启动时需要重置数据 转载缓存放入主轨道缓存后也需要重置数据
void initRepostCacheQueue(EggsGatherQueue *q)
{
	q->front = 0;
	q->rear = 0;
	for(int i=0; i<REPOST_CACHE_LEN; i++){
		rt_memset(&q->items[i], 0, sizeof(EggsGatherInfo));
	}
}

//写入转载缓存队列
void enqueueRepostCacheQueue(EggsGatherQueue *q, EggsGatherInfo *v)
{
	rt_memcpy(&q->items[q->rear], v, sizeof(EggsGatherInfo));
	q->rear = (q->rear + 1) % REPOST_CACHE_LEN;
}

//从转载缓存队列读取数据
void dequeueRepostCacheQueue(EggsGatherQueue *q, EggsGatherInfo *v)
{
	rt_memcpy(v, &q->items[q->front], sizeof(EggsGatherInfo));
	q->front = (q->front + 1) % REPOST_CACHE_LEN;
}

//定向转载输送通道收到一个脉冲信号执行一次
void addThreeLineEggsToRepostCache(void)
{
	uint16_t dirtyIsOpen = false;
	uint16_t colorIsOpen = false;
	uint16_t weighIsOpen = true;
	uint16_t crackIsOpen = false;
	uint8_t i = 0, index = 0;
	
	EggsGatherInfo gat;
	DirtyAndColorInfo dirtyA, dirtyB;
	WeighAndCrackInfo weighA, weighB;

	initRepostCacheQueue(&gatherQueue); //重新初始化转载缓存队列 等待下一次往缓存队列种加入数据

	// step 1.获取所有通道数据 整合后放入转载缓存中去
	for(index=0; index<REPOST_CACHE_LEN; index++) {
		if(true == dirtyIsOpen) { 	// 脏蛋启用
			dequeueDirtyQueue(&dirtyQueueA, &dirtyA);
			dequeueDirtyQueue(&dirtyQueueB, &dirtyB);
			for(i=0; i<REPOST_CACHE_WIDTH; i++){
				if(i<s_WIDTH)
					gat.dirty[i] = dirtyA.dirty[i];
				else
					gat.dirty[i] = dirtyB.dirty[i-s_WIDTH];
			}
			if(true == colorIsOpen) { // 颜色启用
				for(i=0; i<REPOST_CACHE_WIDTH; i++){
					if(i<s_WIDTH)
						gat.color[i] = dirtyA.color[i];
					else
						gat.color[i] = dirtyB.color[i-s_WIDTH];
				}
			}
		}

		if(true == weighIsOpen) {  // 称重启用
			dequeueWeighQueue(&weighQueueA, &weighA);
			dequeueWeighQueue(&weighQueueB, &weighB);
			for(i=0; i<REPOST_CACHE_WIDTH; i++){
				if(i<s_WIDTH)
					gat.weigh[i] = weighA.weigh[i];
				else
					gat.weigh[i] = weighB.weigh[i-s_WIDTH];
//				if(gat.weigh[i] > 0) rt_kprintf("repost:%d =%d index%d\n", gat.weigh[i], i, index);
			}
			if(true == crackIsOpen){ // 裂纹启用
				for(i=0; i<REPOST_CACHE_WIDTH; i++){
					if(i<s_WIDTH)
						gat.level[i] = weighA.level[i];
					else
						gat.level[i] = weighB.level[i-s_WIDTH];
				}
			}
		}
		enqueueRepostCacheQueue(&gatherQueue, &gat);
	}
}
