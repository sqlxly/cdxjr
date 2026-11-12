#ifndef __DIRTYANDCOLOREGGSQUEUE_H
#define __DIRTYANDCOLOREGGSQUEUE_H

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define DIRTY_QUEUE_VALID		30	 //队列有效长度为30

#define DIRTY_QUEUE_SIZE 		38   //队列长度为30 预留8个数据缓冲区

typedef struct
{
	uint16_t mesg_id;   //消息id
  uint8_t dirty[6]; 	//脏蛋等级
	uint8_t color[6];   //颜色等级
} DirtyAndColorInfo;

typedef struct {
    DirtyAndColorInfo items[DIRTY_QUEUE_SIZE];
    uint16_t front;
    uint16_t rear;
} DirtyCircularQueue;

extern DirtyCircularQueue dirtyQueueA;      //脏蛋颜色A组1~6通道
extern DirtyCircularQueue dirtyQueueB;			//脏蛋颜色B组1~6通道

// 初始化队列
void initDirtyQueue(DirtyCircularQueue *q);

// 数据写入队列
bool enqueueDirtyQueue(DirtyCircularQueue *q, DirtyAndColorInfo *v);
	
// 从队列读取数据
bool dequeueDirtyQueue(DirtyCircularQueue *q, DirtyAndColorInfo *v);

// modbus tcp 收到一帧脏蛋颜色检测设备的数据执行一次
void dirtyAndColorModbusDataProc(int index, int len);

#endif
