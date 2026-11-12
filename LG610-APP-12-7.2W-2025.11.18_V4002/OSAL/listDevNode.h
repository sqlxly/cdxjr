#ifndef __LISTDEVNODE_H__
#define __LISTDEVNODE_H__

#include <rtthread.h>

typedef struct{
	rt_uint16_t type;							// 节点设备类型
	rt_uint16_t agreeLen;					// 协议长度 
	rt_uint16_t dataLen;					// 数据长度
	rt_uint16_t agreement[100];		// 节点设备协议数据
}DEVDataType;

typedef struct DEVListNode
{
	DEVDataType data;
	struct DEVListNode* next;
	struct DEVListNode* prev;
}DEVListNode;

// 初始化创建链表 返回链表的头节点
DEVListNode* DEVListCreate(void);
// 双向链表销毁
void DEVListDestory(struct DEVListNode* phead);
// 双向链表打印
void DEVListPrint(struct DEVListNode* phead);
// 双向链表判断是否为空
rt_bool_t DEVListEmpty(struct DEVListNode* phead);
// 双向链表表尾插入一条数据
void DEVListPushBack(struct DEVListNode* phead, rt_uint16_t type);
// 双向链表表尾删除一条数据
void DEVListPopBack(struct DEVListNode* phead);
// 双向链表表头插入一条数据
void DEVListPushFront(struct DEVListNode* phead, rt_uint16_t type);
// 双向链表表头删除一条数据
void ListPopFront(struct DEVListNode* phead);
// 双向链表查找数据
DEVListNode* DEVListFind(struct DEVListNode* phead, rt_uint16_t type);
// 双向链表在pos的前面插入一条数据
void DEVListInsert(struct DEVListNode* pos, rt_uint16_t type);
// 双向链表删除pos位置的节点
void DEVListErase(struct DEVListNode* pos);
// 计算链表有效节点数目
int DEVListSize(struct DEVListNode* phead);

#endif
