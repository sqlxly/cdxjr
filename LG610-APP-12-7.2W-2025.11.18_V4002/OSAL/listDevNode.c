#include "listDevNode.h"

/* 定义一个链表 */

// 申请节点
DEVListNode* DEVBuyNode(rt_uint16_t type)
{
	DEVListNode* newnode = (DEVListNode*)rt_malloc(sizeof(DEVListNode));
	if (!newnode){
		return RT_NULL;
	}
	newnode->data.type = type;
	rt_memset(newnode->data.agreement, 0, sizeof(rt_uint16_t)*101);
	newnode->next = RT_NULL;
	newnode->prev = RT_NULL;
	return newnode;
}

// 初始化创建链表 返回链表的头节点
DEVListNode* DEVListCreate(void)
{
	DEVListNode* head = DEVBuyNode(106); //创建LG106设备为链表头
	head->next = head;
	head->prev = head;
	
	return head;
}

// 双向链表销毁
void DEVListDestory(struct DEVListNode* phead)
{
	DEVListNode* cur = phead->next;
	while (cur != phead)
	{
		DEVListNode* next = cur->next;
		rt_free(cur);
		cur = next;
	}
	rt_free(phead);
	phead = RT_NULL;
}

// 双向链表判断是否为空
rt_bool_t DEVListEmpty(struct DEVListNode* phead)
{
	return phead->next == phead;
}

// 双向链表打印
void DEVListPrint(struct DEVListNode* phead)
{
	DEVListNode* cur = phead->next;
	while (cur != phead){
		rt_kprintf("LG%d ", cur->data.type);
		cur = cur->next;
	}
	rt_kprintf("\n");
}

// 双向链表表头插入一条数据
void DEVListPushFront(struct DEVListNode* phead, rt_uint16_t type)
{
	DEVListNode* cur = phead;
	DEVListNode* newnode = DEVBuyNode(type);
	newnode->next = cur->next;
	newnode->prev = cur;
	cur->next = newnode;
	newnode->next->prev = newnode;
}

// 双向链表表尾插入一条数据
void DEVListPushBack(struct DEVListNode* phead, rt_uint16_t type)
{
	DEVListNode* cur = phead->prev;
	DEVListNode* newnode = DEVBuyNode(type);

	cur->next = newnode;
	newnode->prev = cur;
	newnode->next = phead;
	phead->prev = newnode;	
}

// 双向链表表头删除一条数据
void DEVListPopFront(struct DEVListNode* phead)
{
	if (DEVListEmpty(phead)){
		return;
	}
	DEVListNode* delnode = RT_NULL;
	delnode = phead->next;
	delnode->next->prev = phead;
	phead->next = delnode->next;
	rt_free(delnode);
}

// 双向链表表尾删除一条数据
void DEVListPopBack(struct DEVListNode* phead)
{
	DEVListNode* cur = phead;
	if (DEVListEmpty(phead)){
		return;
	}
	DEVListNode* tmp = cur->prev;
	cur->prev->prev->next = cur;
	cur->prev = cur->prev->prev;
	rt_free(tmp);
}

// 双向链表查找数据
DEVListNode* DEVListFind(struct DEVListNode* phead, rt_uint16_t type)
{
	DEVListNode* cur = phead->next;
	while (cur != phead){
		if (cur->data.type == type){
			return cur;
		}
		cur = cur->next;
	}
	return RT_NULL;
}

// 双向链表在pos的前面插入一条数据
void DEVListInsert(struct DEVListNode* pos, rt_uint16_t type)
{
	if (!pos){
		return;
	}
	DEVListNode* newnode = DEVBuyNode(type);
	newnode->prev = pos->prev;
	newnode->next = pos;
	pos->prev->next = newnode;
	pos->prev = newnode;
}

// 双向链表删除pos位置的节点
void DEVListErase(struct DEVListNode* pos)
{
	if (!pos){
		return;
	}
	pos->prev->next = pos->next;
	pos->next->prev = pos->prev;
	rt_free(pos);
}

// 计算链表有效节点数目
int DEVListSize(struct DEVListNode* phead)
{
	DEVListNode* cur = phead->next;
	int count = 0;
	while (cur != phead)
	{
		count++;
		cur = cur->next;
	}
 
	return count;
}
