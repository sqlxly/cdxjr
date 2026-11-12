#include <rtthread.h>
#include "devConfig.h"
#include "IOdriver.h"

#define DEBOUNCE_COUNT 			3   						// 连续采样次数
#define DEBOUNCE_DELAY 			20  						// 每次采样间隔（毫秒）
#define CHECKDELAY_COUNT		50							// 延时检测下一个蛋托位置
#define TIMEOUT_COUNT 			200 	 					// 拆托超时时间（约4秒）
#define STACKE_COUNT 				150 	 					// 堆料检测超时时间（约3秒）

/*****包装通道外部设备控制的线程 例如自动发托 网链电机等*****/

static uint8_t rly_status = 0;
static uint8_t autoTrayState = 3;				// 自动发托状态机 初始状态为3空闲
static int8_t debounce_counter = 0;    	// 消抖计数器
static int16_t output_open_counter = 0; // 输出开启时间

static int16_t stacke_counter = 0; 			// 堆料检测延时

static struct rt_thread dev_thread;
static char dev_thread_stack[1024];

extern ChanbelOperateTypeDef cop;

//自动发托机工作流程
void atuo_delivery_machine_work(void)
{
	uint8_t status = 0;

	status = get_input_port_sta(cop.inputData, (enum EnumInputPort)(eFSdin3));
	if((1 == cop.sstdOP.autoTrayFlag)&&(1 == cop.sstdOP.autoTrayEnable)){  //输送通道到达发托位置 并且使能了自动发托机
		autoTrayState = 0;
		cop.sstdOP.autoTrayFlag = 2;
		debounce_counter = 0;
		output_open_counter = 0;
	}

	switch(autoTrayState)
	{
		case 0:		//开始发托
			setbitv(cop.dcfState, eFTDJout2);// 开启输出端口
			output_open_counter++;
			if(output_open_counter >= TIMEOUT_COUNT){  //超时 发托故障
				autoTrayState = 8;
			}else{
				if(1 == status){	//检测到了蛋托 进入发托流程
					debounce_counter++;
					if(debounce_counter >= DEBOUNCE_COUNT){
						autoTrayState = 1;
						debounce_counter = 0;
						output_open_counter = 0;
					}
				}
			}
			break;
		case 1:		//延时检测
			output_open_counter++;
			if(output_open_counter >= CHECKDELAY_COUNT){  //延时1s 轮盘转1圈的位置
				autoTrayState = 2;
			}
			break;
		case 2: 	//等待下一个蛋托检测位置
			output_open_counter++;
			if(output_open_counter >= TIMEOUT_COUNT){     //延时4s还没检测到蛋托 拆托机故障
				autoTrayState = 8;
				alarmMark.alarmAutoDispatchError = 1;
			}else{
				if(1 == status){	//检测到了蛋托 本次发托完成
					debounce_counter++;
					if(debounce_counter >= DEBOUNCE_COUNT){
						autoTrayState = 3;
						cop.sstdOP.autoTrayFlag = 0;
						clrbitv(cop.dcfState, eFTDJout2);
					}
				}
			}
			break;
		case 3:		//空闲状态 等待下一个拆托信号
			if(1 == status){
				alarmMark.alarmAutoDispatchError = 0;
			}
			break;
		case 8:		//发托机故障状态
			alarmMark.alarmAutoDispatchError = 1;
			cop.sstdOP.autoTrayFlag = 0;
			autoTrayState = 3;
			clrbitv(cop.dcfState, eFTDJout2);
			break;
		default: autoTrayState = 3; 
			break;
	}
}

//堆料检测 蛋托再输送线上堆积后 需要延时报警
void materiial_stacke_inspection(void)
{
		uint8_t status = 0;
	
		status = get_input_port_sta(cop.inputData, (enum EnumInputPort)(eANdin6));		//蛋托输送 堆料检测
		if(1 == status){ 											//堆料了 则延时启动报警
			stacke_counter++;
			if(stacke_counter >= STACKE_COUNT){ //堆料时间大于3秒
				alarmMark.alarmTrayStacke = 1; 		//堆料报警
				stacke_counter = STACKE_COUNT;
			}
		}else{
			stacke_counter = 0;
			alarmMark.alarmTrayStacke = 0;
		}
}

//空托检测 用于检测蛋托输送轨道上是否却空蛋托
void empty_packet_check(void)
{
	uint8_t status = 0;

	status = get_input_port_sta(cop.inputData, (enum EnumInputPort)(eFSdin2));    //空托检测

	//需要再伺服转到固定的位置的时候再检测  所以需要读取伺服位置反馈值
}

static uint8_t led_switch_cntt = 0;

void led_and_beep_ctrl(void)
{
	uint16_t *ptr;
	
	ptr = (uint16_t *)&alarmMark;
	if((*ptr > 0)||(*(ptr+1) > 0)){
		clrbitv(rly_status, eRLYout3);
		if(led_switch_cntt < 15){
			setbitv(rly_status, eRLYout1);
		}else{
			clrbitv(rly_status, eRLYout1);
		}
//		setbitv(rly_status, eRLYout4);
	}else{
		setbitv(rly_status, eRLYout3);
		clrbitv(rly_status, eRLYout1);
//		clrbitv(rly_status, eRLYout4);
	}
	
	led_switch_cntt = (led_switch_cntt + 1) % 30;
}

static void rt_external_dev_thread_entry(void *parameter)
{
	configRelayIO();
	
	while(1)
	{
		if(mainState == eRunState)        //运行状态下自动发托机和网链电机才工作
		{
			alarmMark.alarmAutoDispatchError = 0;
			
			materiial_stacke_inspection();	//堆料检测报警
			
			empty_packet_check();						//空托检测报警
		}

		atuo_delivery_machine_work();			//运行自动发托机

		led_and_beep_ctrl();
		
		updateRelayIO(rly_status);
		
		rt_thread_mdelay(DEBOUNCE_DELAY);
	}
}

int external_dev_pthread_init(void)
{
		rt_thread_init(&dev_thread,
                   "devices",
                   rt_external_dev_thread_entry,
                   RT_NULL,
                   &dev_thread_stack[0],
                   sizeof(dev_thread_stack),
                   15, 10);
    rt_thread_startup(&dev_thread);

		return RT_EOK;
}

INIT_APP_EXPORT(external_dev_pthread_init);
