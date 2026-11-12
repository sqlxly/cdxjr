#include "terminal_proc.h"

//mbus1消息队列控制块
static struct rt_messagequeue mqttb1;
static rt_uint8_t msg_pool1[1024];

void mbus_mq_init(void)
{
	rt_err_t result;
	
	result = rt_mq_init(&mqttb1, "mqttb1", &msg_pool1[0], 42, sizeof(msg_pool1), RT_IPC_FLAG_PRIO);    
  if (result != RT_EOK)
  {
      rt_kprintf("init message queue failed.\n");
      while(1){}
  }
}

rt_err_t _operate_mbus1_mq_send(char *send, rt_int32_t len)
{
	return rt_mq_send(&mqttb1, (char *)send, len);
}

rt_err_t _operate_mbus1_mq_recv(char *recv, int len, rt_int32_t timeout)
{
	return rt_mq_recv(&mqttb1, recv, len, timeout);
}
