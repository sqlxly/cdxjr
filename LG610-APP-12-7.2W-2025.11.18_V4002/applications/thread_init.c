#include <rtthread.h>
#include <rtdevice.h>

ALIGN(RT_ALIGN_SIZE)

static char mp3_stack[2048];
static struct rt_thread mp3_thread;



extern void mp3_thread_main(void *arg);



 rt_err_t app_thread_init()
{

   rt_thread_init(&mp3_thread,
                   "mp3",
                   mp3_thread_main,
                   RT_NULL,
                   &mp3_stack[0],
                   sizeof(mp3_stack),
                   RT_THREAD_PRIORITY_MAX - 1, 10);
    rt_thread_startup(&mp3_thread);

};