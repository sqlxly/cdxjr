#ifndef __LISTGWNODE_H__
#define __LISTGWNODE_H__

#include <rtthread.h>

typedef struct GWDataType{
	rt_uint8_t devNum;					// 设备序号 即modbus地址
	rt_uint16_t type;						// 节点设备类型
//	rt_uint8_t isOnline;				// 是否在线
	rt_uint16_t iemi[6];				// 24位设备IEMI编码
	rt_uint8_t alarmBit[8]; 		// 64种报警状态 通过读取modbus线圈状态获得 8*8bit=64bit
	rt_uint16_t data[120];			// 读取modbus输入寄存器并备份 用于设备数据变化时的比较
}GWDataType;

typedef struct GWListNode
{
	GWDataType data;
	struct GWListNode* next;
	struct GWListNode* prev;
}GWListNode;

// 创建链表 返回链表的头节点
GWListNode* ListCreate(void);
// 双向链表销毁
void GWListDestory(GWListNode* phead);
// 双向链表打印
void GWListPrint(GWListNode* phead);
// 双向链表判断是否为空
rt_bool_t GWListEmpty(GWListNode* phead);
// 双向链表表尾插入一条数据
void GWListPushBack(GWListNode* phead, rt_uint16_t type);
// 双向链表表尾删除一条数据
void GWListPopBack(GWListNode* phead);
// 双向链表表头插入一条数据
void GWListPushFront(GWListNode* phead, rt_uint16_t type);
// 双向链表表头删除一条数据
void GWListPopFront(GWListNode* phead);
// 双向链表查找数据
GWListNode* GWListFind(GWListNode* phead, rt_uint16_t devNum);
// 双向链表在pos的前面插入一条数据
void GWListInsert(GWListNode* pos, rt_uint16_t type);
// 双向链表删除pos位置的节点
void GWListErase(GWListNode* pos);
// 计算链表有效节点数目
int GWListSize(GWListNode* phead);

#endif
