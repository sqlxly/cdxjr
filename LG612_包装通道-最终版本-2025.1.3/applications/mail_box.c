
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <mail_box.h>

struct rt_messagequeue masseg_modbus_cmd;
struct rt_messagequeue send_control_state_cmd;

static rt_uint8_t msg_modbus_cmd_pool[MESSAGE_DAT_LEN*4];
static rt_uint8_t send_control_state_cmd_pool[MESSAGE_DAT_LEN*4];

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
		rt_err_t result1;

    result1 = rt_mq_init(&masseg_modbus_cmd, "mp1",&msg_modbus_cmd_pool[0], MESSAGE_DAT_LEN,sizeof(msg_modbus_cmd_pool), RT_IPC_FLAG_PRIO);       /* 如果有多个线程等待  优先级大的先得到*/
    result1 += rt_mq_init(&send_control_state_cmd, "mp2",&send_control_state_cmd_pool[0], MESSAGE_DAT_LEN,sizeof(send_control_state_cmd_pool), RT_IPC_FLAG_PRIO);  
    if (result1 != RT_EOK)
    {
        rt_kprintf("init message queue failed.\n");
        return RT_ERROR;
    }
		return RT_EOK;

}
INIT_ENV_EXPORT(masseg_box_init);