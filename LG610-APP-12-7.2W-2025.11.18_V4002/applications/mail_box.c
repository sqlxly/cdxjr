
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <mail_box.h>

ALIGN(RT_ALIGN_SIZE)

struct rt_messagequeue masseg_p1;
struct rt_messagequeue masseg_p2;
struct rt_messagequeue masseg_p3;
struct rt_messagequeue masseg_p4;
struct rt_messagequeue masseg_p5;
struct rt_messagequeue masseg_p6;
struct rt_messagequeue masseg_t1;
struct rt_messagequeue masseg_throw1;
struct rt_messagequeue masseg_throw2;
struct rt_messagequeue masseg_weight;
struct rt_messagequeue masseg_ai;

static rt_uint8_t msg_p1_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_p2_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_p3_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_p4_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_p5_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_p6_pool[MESSAGE_DAT_LEN*20];
//static rt_uint8_t msg_t1_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_throw1_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_throw2_pool[MESSAGE_DAT_LEN*20];
static rt_uint8_t msg_weight_pool[MESSAGE_DAT_LEN*2];
static rt_uint8_t msg_ai_pool[AI_MESSAGE_DAT_LEN*2];
//发送
/* 
            result = rt_mq_send(&mq, &buf, 1);
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_send ERR\n");
            }
						
*/

//接收
/*
if (rt_mq_recv(&mq, &buf, sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("thread1: recv msg from msg queue, the content:%c\n", buf);
            if (cnt == 19)
            {
                break;
            }
        }
*/
static int masseg_box_init(void)
{
		rt_err_t result;
    result =0;
    result += rt_mq_init(&masseg_p1, "mp1",&msg_p1_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p1_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_p2, "mp2",&msg_p2_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p2_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_p3, "mp3",&msg_p3_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p3_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_p4, "mp4",&msg_p4_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p4_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_p5, "mp5",&msg_p5_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p5_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_p6, "mp6",&msg_p6_pool[0], MESSAGE_DAT_LEN,sizeof(msg_p6_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	
	 // result += rt_mq_init(&masseg_t1, "mt1",&msg_t1_pool[0], MESSAGE_DAT_LEN,sizeof(msg_t1_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_throw1, "mtw1",&msg_throw1_pool[0], MESSAGE_DAT_LEN,sizeof(msg_throw1_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
		result += rt_mq_init(&masseg_throw2, "mtw2",&msg_throw2_pool[0], MESSAGE_DAT_LEN,sizeof(msg_throw2_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
    result += rt_mq_init(&masseg_weight, "mtwe",&msg_weight_pool[0], MESSAGE_DAT_LEN,sizeof(msg_weight_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
	  result += rt_mq_init(&masseg_ai, "mai",&msg_ai_pool[0], AI_MESSAGE_DAT_LEN,sizeof(msg_ai_pool), RT_IPC_FLAG_PRIO);   
	  if ((result != RT_EOK))
    {
        rt_kprintf("init message queue failed.\n");
        return RT_ERROR;
    }
		return RT_EOK;

}
INIT_ENV_EXPORT(masseg_box_init);