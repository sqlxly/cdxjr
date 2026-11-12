#include "mainOrbitCacheQueue.h"

/**** 实现1个高效数据队列 存放转载缓存放入主轨道缓存中的鸡蛋 ****/

MainOrbitCircularQueue mainOrbitQueue;     	//主轨道蛋杯数据缓存
ShowPageCircularQueue showPageQueue;				//主轨道展示前24个蛋杯详细信息的缓存


/* 主轨道缓存逻辑功能：
 * 1.根据转载缓存放入主轨道缓存的脉冲信号开始计算
 * 2.将转载缓存中计算好分配位置的鸡蛋放入主轨道缓存中
 * 3.同时根据主轨道蛋杯脉冲和编码器位置计算主轨道中当前鸡蛋的位置 是否到达需要分配的通道位置 如果到位 需要控制电磁铁打开并将鸡蛋踢入包装通道的第一层缓存 再根据编码器位置确定电磁铁收回的位置
 * 4.鸡蛋被踢出后 将鸡蛋信息从主轨道蛋杯中移除
 */

// 初始化主轨道展示蛋杯缓存队列
void initShowPageQueue(ShowPageCircularQueue *q) {
		int i;
    q->front = 0;
    q->rear = SHOW_PAGE_QUEUE_VALID;
		for(i=0; i<SHOW_PAGE_QUEUE_SIZE; i++){  //设置初始值
			rt_memset(&q->items[i], 0, sizeof(EggsGatherInfoEx));
		}
}

// 数据写入主轨道展示蛋杯缓存队列
void enqueueShowPageQueue(ShowPageCircularQueue *q, EggsGatherInfoEx *v) {
		rt_memcpy(&q->items[q->rear], v, sizeof(EggsGatherInfoEx));
    q->rear = (q->rear + 1) % SHOW_PAGE_QUEUE_SIZE; // 更新rear环形指针位置
}

// 从主轨道展示蛋杯缓存队列读取数据
void dequeueShowPageQueue(ShowPageCircularQueue *q, EggsGatherInfoEx *v) {
		rt_memcpy(v, &q->items[q->front], sizeof(EggsGatherInfoEx));
		rt_memset(&q->items[q->front], 0, sizeof(EggsGatherInfoEx));  	// 读完以后需要清除队列中原来存储的数据
    q->front = (q->front + 1) % SHOW_PAGE_QUEUE_SIZE; 					// 更新front环形指针位置
}

// 初始化主轨道缓存队列
void initMainOrbitQueue(MainOrbitCircularQueue *q) {
		int i, j;
		q->front = 0;
		q->rear = MAIN_ORBIT_QUEUE_VALID;
		for(i=0; i<MAIN_ORBIT_QUEUE_SIZE; i++){  //设置初始值
			for(j=0; j<MAIN_ORBIT_NUM; j++){
				q->items[i].isAct[j] = 0;
				q->items[i].toChannel[j] = 0;
				q->items[i].toTag[j] = 0;
			}
		}
}

// 数据写入主轨道缓存队列
void enqueueMainOrbitQueue(MainOrbitCircularQueue *q, EggsClassifyInfo *v) {
		rt_memcpy(&q->items[q->rear], v, sizeof(EggsClassifyInfo));
    q->rear = (q->rear + 1) % MAIN_ORBIT_QUEUE_SIZE; // 更新rear环形指针位置
}

// 从主轨道缓存队列读取数据
void dequeueMainOrbitQueue(MainOrbitCircularQueue *q, EggsClassifyInfo *v) {
		rt_memcpy(v, &q->items[q->front], sizeof(EggsClassifyInfo));
		rt_memset(&q->items[q->front], 0, sizeof(EggsClassifyInfo));	// 读完以后需要清除队列中原来存储的数据  考虑不清除数据  在尾端抛弃踢蛋的时候再清理数据
    q->front = (q->front + 1) % MAIN_ORBIT_QUEUE_SIZE; 						// 更新front环形指针位置
	
//		rt_kprintf("%d %d %d\n", q->front, q->rear, ((q->rear > q->front) ? (q->rear - q->front) : (q->rear + MAIN_ORBIT_QUEUE_SIZE - q->front)));
}

static void show_page(int argc, char **argv)
{
	int pos;
	for(int i=0; i<48; i++){
		pos = (showPageQueue.front + i) % SHOW_PAGE_QUEUE_SIZE;
		rt_kprintf("%d: %d %d %d %d\n", i, showPageQueue.items[pos].weigh[0], showPageQueue.items[pos].weigh[1], showPageQueue.items[pos].weigh[2], showPageQueue.items[pos].weigh[3]);
	}
}

MSH_CMD_EXPORT_ALIAS(show_page, show_page, show page);

static void main_page(int argc, char **argv)
{
	int pos,i;
	for(i=0; i<MAIN_ORBIT_QUEUE_VALID; i++){
		pos = (mainOrbitQueue.front + i) % MAIN_ORBIT_QUEUE_SIZE;
		rt_kprintf("%d: %d%d %d%d %d%d %d%d\n", i+1, mainOrbitQueue.items[pos].toChannel[0],mainOrbitQueue.items[pos].isAct[0], mainOrbitQueue.items[pos].toChannel[1],mainOrbitQueue.items[pos].isAct[1], mainOrbitQueue.items[pos].toChannel[2],mainOrbitQueue.items[pos].isAct[2], mainOrbitQueue.items[pos].toChannel[3],mainOrbitQueue.items[pos].isAct[3]);
	}
	
	rt_kprintf("%d %d %d\n", mainOrbitQueue.front, mainOrbitQueue.rear, 
	((mainOrbitQueue.rear > mainOrbitQueue.front) ? (mainOrbitQueue.rear - mainOrbitQueue.front) : (mainOrbitQueue.rear + MAIN_ORBIT_QUEUE_SIZE - mainOrbitQueue.front)));
}

MSH_CMD_EXPORT_ALIAS(main_page, main_page, show main orbit);
