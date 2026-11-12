#include "weighAndCrackQueue.h"
#include "mdbus_tcp_slaver.h"

/**** 实现2个高效数据队列 分别对应裂纹A和裂纹B ****/

WeighCircularQueue weighQueueA;     //称重裂纹A组1~6通道
WeighCircularQueue weighQueueB;			//称重裂纹B组1~6通道
WeighSyncInfo weighsync;						//称重信息同步

static WeighAndCrackInfo recvA, recvB;
static uint8_t recvFlagA = 0, recvFlagB = 0;

// 初始化队列
void initWeighQueue(WeighCircularQueue *q) {
		int16_t temp = 0, s = 0, c = 0, diff = 0;
	
//		temp = u_tab_registers[999];
		temp = 300;
		c = temp / 500;
		s = temp % 500;
		if(temp < 0){
			if(s != 0) diff = c - 1;
			else diff = c;
			weighsync.syncPosition = (s + 500 + 300) % 500;
		} else {
			diff = c;
			weighsync.syncPosition = (s + 300) % 500;
		}
		weighsync.printflag = 0;
		weighsync.compareRealProx = 0;
		weighsync.weighQueueLen = WEIGH_QUEUE_VALID + diff;
    q->front = 0;
    q->rear = weighsync.weighQueueLen;
		for(int i=0; i<WEIGH_QUEUE_SIZE; i++){  //设置初始值9999 表示没有鸡蛋 缓存空闲状态
			q->items[i].mesg_id = 9999;
		}
}

// 数据写入队列
bool enqueueWeighQueue(WeighCircularQueue *q, WeighAndCrackInfo *v) {
		rt_memcpy(&q->items[q->rear], v, sizeof(WeighAndCrackInfo));
    q->rear = (q->rear + 1) % WEIGH_QUEUE_SIZE; 											// 更新rear环形指针位置
    return true;
}

// 从队列读取数据
bool dequeueWeighQueue(WeighCircularQueue *q, WeighAndCrackInfo *v) {
		rt_memcpy(v, &q->items[q->front], sizeof(WeighAndCrackInfo));
		rt_memset(&q->items[q->front], 0, sizeof(WeighAndCrackInfo));  		// 读完以后需要清除队列中原来存储的数据
    q->front = (q->front + 1) % WEIGH_QUEUE_SIZE; 										// 更新front环形指针位置
    return true;
}

// modbus tcp 收到一帧称重裂纹检测设备的数据执行一次
void weighAndCrackModbusDataProc(int index, int len)
{
	if((10013 == index)||(10113 == index)){
		//心跳报文 包含故障报警信息
	} else if((10000 == index)||(10100 == index)) {
		WeighAndCrackInfo vx;
		
		vx.mesg_id = u_tab_registers[index+0];
		// 重量信息
		vx.weigh[0] = u_tab_registers[index+1];
		vx.weigh[1] = u_tab_registers[index+2];
		vx.weigh[2] = u_tab_registers[index+3];
		vx.weigh[3] = u_tab_registers[index+4];
		vx.weigh[4] = u_tab_registers[index+5];
		vx.weigh[5] = u_tab_registers[index+6];
		// 裂纹信息
		vx.level[0] = u_tab_registers[index+7];
		vx.level[1] = u_tab_registers[index+8];
		vx.level[2] = u_tab_registers[index+9];
		vx.level[3] = u_tab_registers[index+10];
		vx.level[4] = u_tab_registers[index+11];
		vx.level[5] = u_tab_registers[index+12];

		switch(index){
			case 10000:		// 收到称重A的同步数据
				recvFlagA = RT_TRUE;
				rt_memcpy(&recvA, &vx, sizeof(WeighAndCrackInfo));
				weighsync.printflag = 1;
				break;
			case 10100:		// 收到称重B的同步数据
				recvFlagB = RT_TRUE;
				rt_memcpy(&recvB, &vx, sizeof(WeighAndCrackInfo));
				break;
			default: break;
		}
	}
}

void sync_weigh_and_crack_data_to_queue(void)
{
	if(recvFlagA == RT_TRUE){
		recvFlagA = RT_FALSE;
		enqueueWeighQueue(&weighQueueA, &recvA);
	}else{
		//同步异常
		rt_memset(&recvA, 0, sizeof(WeighAndCrackInfo));
		recvA.mesg_id = 9999;
		enqueueWeighQueue(&weighQueueA, &recvA);
	}
	
	if(recvFlagB == RT_TRUE){
		recvFlagA = RT_FALSE;
		enqueueWeighQueue(&weighQueueB, &recvB);
	}else{
		//同步异常
		rt_memset(&recvB, 0, sizeof(WeighAndCrackInfo));
		recvB.mesg_id = 9999;
		enqueueWeighQueue(&weighQueueB, &recvB);
	}
}

static void weigh_page(int argc, char **argv)
{
	int pos,i;
	
	for(i=0; i<weighsync.weighQueueLen+4; i++){
		pos = (weighQueueA.front + i) % WEIGH_QUEUE_SIZE;
		rt_kprintf("%d: %d %d %d %d %d %d", i+1, weighQueueA.items[pos].weigh[0],weighQueueA.items[pos].weigh[1], weighQueueA.items[pos].weigh[2],weighQueueA.items[pos].weigh[3], weighQueueA.items[pos].weigh[4],weighQueueA.items[pos].weigh[5]);
		pos = (weighQueueB.front + i) % WEIGH_QUEUE_SIZE;
		rt_kprintf(" %d %d %d %d %d %d\n", weighQueueB.items[pos].weigh[0],weighQueueB.items[pos].weigh[1], weighQueueB.items[pos].weigh[2],weighQueueB.items[pos].weigh[3], weighQueueB.items[pos].weigh[4],weighQueueB.items[pos].weigh[5]);
	}
}

MSH_CMD_EXPORT_ALIAS(weigh_page, weigh_page, show weigh orbit);
