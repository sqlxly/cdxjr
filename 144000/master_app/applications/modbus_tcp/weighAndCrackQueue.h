#ifndef __WEIGHANDCRACKQUEUE_H
#define __WEIGHANDCRACKQUEUE_H

#include <rtthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WEIGH_QUEUE_VALID		31	 //队列有效长度为32  展示蛋杯位置靠后这个数字增加  展示蛋杯位置提前这个数字减小

#define WEIGH_QUEUE_SIZE    50   //可用队列长度为32 预留数据缓冲区

typedef struct
{
	uint16_t mesg_id;    //消息id
  uint16_t weigh[6];   //鸡蛋重量
	uint8_t  level[6];   //裂纹等级
} WeighAndCrackInfo;

typedef struct {
  WeighAndCrackInfo items[WEIGH_QUEUE_SIZE];
  uint16_t front;
  uint16_t rear;
} WeighCircularQueue;

typedef struct {
	uint16_t weighQueueLen;				//重新计算过的称重队列长度
	int32_t syncPosition;					//触发称重同步数据写入队列的脉冲位置
	uint8_t isWaitForData;				//等待称重信息同步标志 为1表示主控已经走到数据同步位置等待从机发送数据 收到从机数据后清除标志
	int32_t compareRealEncoder;   //相对编码器计数
  int32_t compareRealProx;      //相对接近开关计数  每3个脉冲需要接收一次称重数据
	uint8_t printflag;
} WeighSyncInfo;

extern WeighCircularQueue weighQueueA;      //称重裂纹A组1~6通道
extern WeighCircularQueue weighQueueB;			//称重裂纹B组1~6通道
extern WeighSyncInfo weighsync;							//称重信息同步

// 初始化队列
void initWeighQueue(WeighCircularQueue *q);

// 数据写入队列
bool enqueueWeighQueue(WeighCircularQueue *q, WeighAndCrackInfo *v);
	
// 从队列读取数据
bool dequeueWeighQueue(WeighCircularQueue *q, WeighAndCrackInfo *v);

void weighAndCrackModbusDataProc(int index, int len);

//触发同步裂纹称重数据到队列中
void sync_weigh_and_crack_data_to_queue(void);

#endif
