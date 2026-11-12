#include "dirtyAndColorEggsQueue.h"
#include "mdbus_tcp_slaver.h"

/**** 实现2个高效数据队列 分别对应脏蛋颜色A和脏蛋颜色B ****/

DirtyCircularQueue dirtyQueueA;     //脏蛋颜色A组1~6通道
DirtyCircularQueue dirtyQueueB;			//脏蛋颜色B组1~6通道

// 初始化队列
void initDirtyQueue(DirtyCircularQueue *q) {
    q->front = 0;
    q->rear = DIRTY_QUEUE_VALID;
		for(int i=0; i<DIRTY_QUEUE_SIZE; i++){  //设置初始值9999 表示没有鸡蛋 缓存空闲状态
			q->items[i].mesg_id = 9999;
		}
}

// 判断队列是否同步异常 正常情况下front和rear的位置差值正好是队列的长度
bool isDirtyQueueExpect(DirtyCircularQueue *q) {
	int diff = 0;
	if(q->front < q->rear) {
		diff = q->rear - q->front;
	}else{
		diff = q->rear + DIRTY_QUEUE_SIZE - q->front;
	}
	if(diff < DIRTY_QUEUE_VALID -2) 
		return false;
	
	return true;
}

// 判断队列为空
bool isDirtyQueueEmpty(DirtyCircularQueue *q) {
    return q->front == q->rear;
}

// 判断队列是否已满
bool isDirtyQueueFull(DirtyCircularQueue *q) {
    return (q->rear + 1) % DIRTY_QUEUE_SIZE == q->front;
}

// 数据写入队列
bool enqueueDirtyQueue(DirtyCircularQueue *q, DirtyAndColorInfo *v) {
    if (isDirtyQueueFull(q)) {
        return false; // 若队列已满,无法写入元素
    }
		rt_memcpy(&q->items[q->rear], v, sizeof(DirtyAndColorInfo));
    q->rear = (q->rear + 1) % DIRTY_QUEUE_SIZE; // 更新rear环形指针位置
    return true;
}

// 从队列读取数据
bool dequeueDirtyQueue(DirtyCircularQueue *q, DirtyAndColorInfo *v) {
    if (isDirtyQueueEmpty(q)) {
        return false; // 若队列为空,无法读取元素
    }
		rt_memcpy(v, &q->items[q->front], sizeof(DirtyAndColorInfo));
		rt_memset(&q->items[q->front], 0, sizeof(DirtyAndColorInfo));  				// 读完以后需要清除队列中原来存储的数据
    q->front = (q->front + 1) % DIRTY_QUEUE_SIZE; 											  // 更新front环形指针位置
    return true;
}

// modbus tcp 收到一帧脏蛋颜色检测设备的数据执行一次
void dirtyAndColorModbusDataProc(int index, int len)
{
	if((10200 == index)||(10300 == index)) {
		DirtyAndColorInfo vx;
		
		vx.mesg_id  = u_tab_registers[index+0];
		// 脏蛋信息
		vx.dirty[0] = u_tab_registers[index+1];
		vx.dirty[1] = u_tab_registers[index+2];
		vx.dirty[2] = u_tab_registers[index+3];
		vx.dirty[3] = u_tab_registers[index+4];
		vx.dirty[4] = u_tab_registers[index+5];
		vx.dirty[5] = u_tab_registers[index+6];
		// 颜色信息
		vx.color[0] = u_tab_registers[index+7];
		vx.color[1] = u_tab_registers[index+8];
		vx.color[2] = u_tab_registers[index+9];
		vx.color[3] = u_tab_registers[index+10];
		vx.color[4] = u_tab_registers[index+11];
		vx.color[5] = u_tab_registers[index+12];
		
		switch(index){
			case 10200:		// 收到脏蛋颜色A的同步采集数据
				enqueueDirtyQueue(&dirtyQueueA, &vx);
				break;
			case 10300:		// 收到脏蛋颜色B的同步采集数据
				enqueueDirtyQueue(&dirtyQueueB, &vx);
				break;
			default: break;
		}
	}
}
