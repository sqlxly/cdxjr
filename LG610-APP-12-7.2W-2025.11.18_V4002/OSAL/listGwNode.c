#include "listGwNode.h"

/* 定义一个链表 */

// 申请节点
GWListNode* GWBuyNode(rt_uint16_t type)
{
	GWListNode* newnode = (GWListNode*)rt_malloc(sizeof(GWListNode));
	if (!newnode){
		return RT_NULL;
	}
	newnode->data.type = type;
	rt_memset(newnode->data.data, 0, sizeof(rt_uint16_t)*100);
	newnode->next = RT_NULL;
	newnode->prev = RT_NULL;
	return newnode;
}

// 创建链表 返回链表的头节点
GWListNode* ListCreate(void)
{
	GWListNode* head = GWBuyNode(0);
	head->next = head;
	head->prev = head;
	return head;
}

// 双向链表销毁
void GWListDestory(GWListNode* phead)
{
	GWListNode* cur = phead->next;
	while (cur != phead)
	{
		GWListNode* next = cur->next;
		rt_free(cur);
		cur = next;
	}
	rt_free(phead);
	phead = RT_NULL;
}

// 双向链表判断是否为空
rt_bool_t GWListEmpty(GWListNode* phead){
	return phead->next == phead;
}

// 双向链表打印
void GWListPrint(GWListNode* phead){
	GWListNode* cur = phead->next;
	while (cur != phead){
		rt_kprintf("LG%d ", cur->data.type);
		cur = cur->next;
	}
	rt_kprintf("\n");
}

// 双向链表表尾插入一条数据
void GWListPushBack(GWListNode* phead, rt_uint16_t type){
	GWListNode* cur = phead->prev;
	GWListNode* newnode = GWBuyNode(type);

	cur->next = newnode;
	newnode->prev = cur;
	newnode->next = phead;
	phead->prev = newnode;	
}

// 双向链表表尾删除一条数据
void GWListPopBack(GWListNode* phead){
	GWListNode* cur = phead;
	if (GWListEmpty(phead)){
		return;
	}
	GWListNode* tmp = cur->prev;
	cur->prev->prev->next = cur;
	cur->prev = cur->prev->prev;
	rt_free(tmp);
}

// 双向链表表头插入一条数据
void GWListPushFront(GWListNode* phead, rt_uint16_t type){
	GWListNode* cur = phead;
	GWListNode* newnode = GWBuyNode(type);
	newnode->next = cur->next;
	newnode->prev = cur;
	cur->next = newnode;
	newnode->next->prev = newnode;
}

// 双向链表表头删除一条数据
void GWListPopFront(GWListNode* phead){
	if (GWListEmpty(phead)){
		return;
	}
	GWListNode* delnode = RT_NULL;
	delnode = phead->next;
	delnode->next->prev = phead;
	phead->next = delnode->next;
	rt_free(delnode);
}

// 双向链表查找数据
GWListNode* GWListFind(GWListNode* phead, rt_uint16_t devNum){
	GWListNode* cur = phead->next;
	while (cur != phead){
		if (cur->data.devNum == devNum){
			return cur;
		}
		cur = cur->next;
	}
	return RT_NULL;
}

// 双向链表在pos的前面插入一条数据
void GWListInsert(GWListNode* pos, rt_uint16_t type){
	if (!pos){
		return;
	}
	GWListNode* newnode = GWBuyNode(type);
	newnode->prev = pos->prev;
	newnode->next = pos;
	pos->prev->next = newnode;
	pos->prev = newnode;
}

// 双向链表删除pos位置的节点
void GWListErase(GWListNode* pos){
	if (!pos){
		return;
	}
	pos->prev->next = pos->next;
	pos->next->prev = pos->prev;
	rt_free(pos);
}

// 计算链表有效节点数目
int GWListSize(GWListNode* phead)
{
	int count = 0;
	GWListNode* cur = phead->next;
	
	while (cur != phead)
	{
		count++;
		cur = cur->next;
	}
 
	return count;
}
