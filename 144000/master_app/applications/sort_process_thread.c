#include <rtthread.h>
#include <drv_common.h>
#include "mainOrbitCacheQueue.h"
#include "repostCacheQueue.h"
#include "production_param.h"
#include "user_sorter.h"
#include "mdbus_tcp_slaver.h"
#include "user_sorter.h"
/* 鸡蛋分类处理线程：
	1.从输送线队列读取4排鸡蛋数据到转载缓存
	2.根据重量 裂纹 脏蛋 血斑等信息确定鸡蛋可以去到那个通道
	3.根据包装信息和生产产品的任务信息确定鸡蛋要去到那个通道 当多个通道都满足条件时 需要均分合格的鸡蛋
	4.为每个鸡蛋打上通道标签 最后将标记好的鸡蛋信息加入4条主轨道缓冲队列中去
*/
static struct rt_thread sort_thread;
static char sort_thread_stack[2048];

static rt_sem_t sort_sem = RT_NULL;

int channelSel = 0;
void sort_sem_release(void)
{
	rt_sem_release(sort_sem);
}

static uint8_t packageChannelPostion[REPOST_CACHE_LEN] = {0};
static uint8_t oneChannelTickPostion[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};

void sort_sync_process_func_ex(void)
{
	int i, j;
	
	EggsGatherInfo gat[REPOST_CACHE_LEN];
	EggsClassifyInfo eggsClass;
	EggsGatherInfoEx gatEx;
	
	addThreeLineEggsToRepostCache();
	
	for(i=0; i<REPOST_CACHE_LEN; i++){  									//读取转载缓存数据 然后放入蛋杯信息展示缓存
		dequeueRepostCacheQueue(&gatherQueue, &gat[i]);
	}
	
	for(j=0; j<REPOST_CACHE_WIDTH; j++){ //12
		for(i=0; i<REPOST_CACHE_LEN; i++){ //4
			if(gat[i].weigh[j] > 0){
				proData.eggsTotal += 1;
				while(1){
					if(1 == u_tab_registers[8319+packageChannelPostion[i]]){	//这个通道是启用状态
						gat[i].toChannel[j] = packageChannelPostion[i] + 1; 
						eggsClass.toChannel[i] = packageChannelPostion[i] + 1;
						eggsClass.isAct[i] = oneChannelTickPostion[i][packageChannelPostion[i]] + 1;
						oneChannelTickPostion[i][packageChannelPostion[i]] = (oneChannelTickPostion[i][packageChannelPostion[i]] + 1) % 6;
//						if(packageChannelPostion[i]>1){
//							oneChannelTickPostion[i][packageChannelPostion[i]] = (oneChannelTickPostion[i][packageChannelPostion[i]] + 1) % 6;
//						}else{
//							oneChannelTickPostion[i][packageChannelPostion[i]] = (oneChannelTickPostion[i][packageChannelPostion[i]] + 1) % 5;
//						}
						if(0 == oneChannelTickPostion[i][packageChannelPostion[i]]){
							packageChannelPostion[i] = (packageChannelPostion[i] + 1) % sorterUsedNumber;
						}
						break;
					}else{
						packageChannelPostion[i] = (packageChannelPostion[i] + 1) % sorterUsedNumber;
					}
				}
			}else{

				gat[i].toChannel[j] = 0; //test
				eggsClass.isAct[i] = 0;
			}
			eggsClass.toTag[i] = 0;//test
			
			gatEx.weigh[i] = gat[i].weigh[j];
			gatEx.level[i] = gat[i].level[j];
			gatEx.dirty[i] = gat[i].dirty[j];
			gatEx.color[i] = gat[i].color[j];
			gatEx.toChannel[i] = gat[i].toChannel[j];
		}
		enqueueShowPageQueue(&showPageQueue, &gatEx);
		enqueueMainOrbitQueue(&mainOrbitQueue, &eggsClass);
	}
}

void sort_sync_test(void)
{
	int i, j;
	
	EggsGatherInfo gat[REPOST_CACHE_LEN];
	EggsClassifyInfo eggsClass;
	EggsGatherInfoEx gatEx;
	
	addThreeLineEggsToRepostCache();
	
	for(i=0; i<REPOST_CACHE_LEN; i++){  									//读取转载缓存数据 然后放入蛋杯信息展示缓存
		dequeueRepostCacheQueue(&gatherQueue, &gat[i]);
	}

	for(j=0; j<REPOST_CACHE_WIDTH; j++){ //12
		for(i=0; i<REPOST_CACHE_LEN; i++){ //4
//			if(gat[i].weigh[j] > 0){
				proData.eggsTotal += 1;
				while(1){
					if(1 == u_tab_registers[7519+packageChannelPostion[i]]){	//这个通道是启用状态
						gat[i].toChannel[j] = packageChannelPostion[i] + 1; 
						eggsClass.toChannel[i] = packageChannelPostion[i] + 1;
						eggsClass.isAct[i] = oneChannelTickPostion[i][packageChannelPostion[i]] + 1;
						oneChannelTickPostion[i][packageChannelPostion[i]] = (oneChannelTickPostion[i][packageChannelPostion[i]] + 1) % 6;
						if(0 == oneChannelTickPostion[i][packageChannelPostion[i]]){
							packageChannelPostion[i] = (packageChannelPostion[i] + 1) % sorterUsedNumber;
						}
						break;
					}else{
						packageChannelPostion[i] = (packageChannelPostion[i] + 1) % sorterUsedNumber;
					}
				}
//			}else{
//				gat[i].toChannel[j] = 0; //test
//				eggsClass.isAct[i] = 0;
//			}
			eggsClass.toTag[i] = 0;//test
			
			gatEx.weigh[i] = gat[i].weigh[j];
			gatEx.level[i] = gat[i].level[j];
			gatEx.dirty[i] = gat[i].dirty[j];
			gatEx.color[i] = gat[i].color[j];
			gatEx.toChannel[i] = gat[i].toChannel[j];
		}
		enqueueShowPageQueue(&showPageQueue, &gatEx);
		enqueueMainOrbitQueue(&mainOrbitQueue, &eggsClass);
	}
}

void eggs_sort_sync(void)
{
		int i, j;
		EggsGatherInfo gat[REPOST_CACHE_LEN];
		EggsClassifyInfo eggsClass;
		EggsGatherInfoEx gatEx;
		SingleEggInfo oneEgg;
	
		addThreeLineEggsToRepostCache();
	
		for(i=0; i<REPOST_CACHE_LEN; i++){//读取转载缓存数据 然后放入蛋杯信息展示缓存
			dequeueRepostCacheQueue(&gatherQueue, &gat[i]);
		}
		for(j=0; j<REPOST_CACHE_WIDTH; j++){
			for(i=0; i<REPOST_CACHE_LEN; i++){
				//添加分级计算的代码 需要计算每个蛋分配到哪个通道 ***************
			
				// step 1.获取这个鸡蛋的特征信息
				oneEgg.weigh = gat[i].weigh[j];
				oneEgg.level = gat[i].level[j];
				oneEgg.dirty = gat[i].dirty[j];
				oneEgg.color = gat[i].color[j];
				oneEgg.orbitIndex = i;
				oneEgg.toChannel = 0;
				oneEgg.toTag = 0;
				oneEgg.isAct = 0;
				
				// step 2.获取分级配置文件(需等屏幕程序开发出来) 根据分级配置文件计算出每个鸡蛋要去的位置
				if(oneEgg.weigh > 0){		//这个蛋杯有鸡蛋
					proData.eggsTotal += 1;
					eggs_sort_actuator(&oneEgg, &workRecipe);
				}
				
				eggsClass.toChannel[i] = oneEgg.toChannel;
				eggsClass.isAct[i] = oneEgg.isAct;
				eggsClass.toTag[i] = oneEgg.toTag;

				gatEx.weigh[i] = oneEgg.weigh;
				gatEx.level[i] = oneEgg.level;
				gatEx.dirty[i] = oneEgg.dirty;
				gatEx.color[i] = oneEgg.color;
				gatEx.toChannel[i] = oneEgg.toChannel;
			}
			enqueueShowPageQueue(&showPageQueue, &gatEx);
			enqueueMainOrbitQueue(&mainOrbitQueue, &eggsClass);
		}
}

void sort_process_thread_entry(void *param)
{
	static rt_err_t result;
	mode_cache_reset();
	while(RT_TRUE)
	{
			result = rt_sem_take(sort_sem, RT_WAITING_FOREVER);
      if(result == RT_EOK){
//          rt_kprintf("sortting...\n");
//					sort_sync_test();
					eggs_sort_sync();
					//sort_sync_process_func_ex();
				
					repost_complete_singal = 0;
      }
	}
}

int sort_process_thread_init(void)
{
    sort_sem = rt_sem_create("sort_sem", 0, RT_IPC_FLAG_PRIO);
    if (sort_sem == RT_NULL){
        rt_kprintf("create sort_sem semaphore failed.\n");
        return -1;
    } else {
        rt_kprintf("create done. sort_sem semaphore value = 0.\n");
    }

		rt_thread_init(&sort_thread,
                   "sort_proc",
                   sort_process_thread_entry,
                   RT_NULL,
                   &sort_thread_stack[0],
                   sizeof(sort_thread_stack),
                   19, 10);
    rt_thread_startup(&sort_thread);
									 
		return RT_EOK;
}

//INIT_APP_EXPORT(sort_process_thread_init);
