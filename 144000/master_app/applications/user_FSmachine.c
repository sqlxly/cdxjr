#include "user_FSmachine.h"
#include "user_spi_slaver.h"
#include "mdbus_tcp_slaver.h"
#include "repostCacheQueue.h"
#include "user_encoder.h"
#include "user_alarm.h"
#include "dirtyAndColorEggsQueue.h"
#include "weighAndCrackQueue.h"
#include "sort_process_thread.h"
#include "production_param.h"
#include "discard_channel.h"

__IO uint8_t u_spiTxBuf[1024];   // SPI发送数据缓存
__IO uint8_t u_spiRxBuf[1024];	 // SPI接收数据缓存

static uint8_t repostPulseBak = 0;
static uint8_t eggCupPulseBak = 0;

static PackageAlarmMaskTypeDef alarmMask;			//多通道报警掩膜

/**** 设备主轨道蛋杯位置掩膜 指示主轨道蛋杯与包装通道相对位置关系 数字表示包装通道编号 0表示空位 ****/

// 包装通道1A  	65-PLUSE
// 包装通道1B  	75-PLUSE
// 抛弃3 		 		98-PLUSE 最后一个抛弃通道电磁铁+1的位置

const uint16_t DevMainOrbitMask[SORTER_USED_MAX] = {63, 53, 33, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 	//包装通道设备物理位置在主轨道链条的第多少号蛋杯对齐 和结构相关 76 86

static CacheCupStateDef cacheState[SORTER_USED_MAX];					//主轨道蛋杯与第一层缓存蛋杯配合 进行踢蛋转载的相关操作参数

static TickEggStateDef tickState;															//主轨道踢蛋和喷码位置触发的编码器脉冲记录

static uint8_t cup_close_counter[4] = {0,0,0,0};							//蛋杯闭合信号检测消抖计数

static uint16_t obritRearBak = MAIN_ORBIT_QUEUE_VALID, checkPos = 12;

int32_t cDctAheadPluse;																				//偏移脉冲最大值 = (单个蛋杯宽度对应脉冲数166)/(单个蛋杯宽度63.5mm)*(电磁铁最大提前位置86mm)

uint8_t cup_per_singal = 0;																		//每个蛋杯的脉冲信号置位

const uint16_t egg_detect_postion = 5;												//定义待检测鸡蛋在主轨道蛋杯中的位置

static uint8_t eggs_cup_close_detect(void); 									//主轨道组件 主轨蛋杯闭合到位检测1~4

static uint8_t eggs_repost_malposition_detect(void);					//主轨道组件 主蛋杯接蛋检测1~4 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中

CacheCupStateDef *get_cache_cup_state(int ch)
{
	return &cacheState[ch];
}

PackageAlarmMaskTypeDef *get_package_alarm_mask_info(void)
{
	return &alarmMask;
}

void clear_package_alarm_mask(void)
{
	rt_memset(&alarmMask, 0, sizeof(PackageAlarmMaskTypeDef));
}

void mainOrbitMaskInit(void)
{
	for(int i=0; i<sorterUsedNumber; i++){
		cacheState[i].mask = DevMainOrbitMask[i];
	}
}

void allCacheQueueInit(void)
{
	initDirtyQueue(&dirtyQueueA);
	initDirtyQueue(&dirtyQueueB);
	
	initWeighQueue(&weighQueueA);
	initWeighQueue(&weighQueueB);
	
	initRepostCacheQueue(&gatherQueue);
	
	initShowPageQueue(&showPageQueue);

	initMainOrbitQueue(&mainOrbitQueue);
	
	mainOrbitMaskInit();
	
	rt_memset(&tickState, 0, sizeof(TickEggStateDef));

	firstPulseEncoderClear();
	tickState.tickEncoderAdd = 0;
	tickState.encoderRepostAdd = 0;
	weighsync.compareRealEncoder = 0;
	obritRearBak = MAIN_ORBIT_QUEUE_VALID;
	checkPos = 12;
}

void package_alarm_info_analysis(AcceptSlaveMessage *msg, PackageAlarmMaskTypeDef *mask, int ch)
{
	uint16_t *ptr;
		
	if(1 == u_tab_registers[8319+ch]){ //启用
		//第1--3层缓存蛋杯故障
		if((msg->alarm.alarmCache & 0x000f) > 0) setbitv(mask->cache1ErrorMask, ch);
		if((msg->alarm.alarmCache & 0x00f0) > 0) setbitv(mask->cache2ErrorMask, ch);
		if((msg->alarm.alarmCache & 0x0f00) > 0) setbitv(mask->cache3ErrorMask, ch);
		//移动缓存故障
		if(msg->alarm.alarmMoveServoConenct) setbitv(mask->moveCacheConnectErrorMask, ch);
		if(msg->alarm.alarmMoveServodevice) setbitv(mask->moveCacheDeviceErrorMask, ch);
		if(msg->alarm.alarmMoveCupError) setbitv(mask->moveCacheCupErrorMask, ch);
		//接收抓故障
		if(msg->alarm.alarmAcceptServoConnect) setbitv(mask->acceptConnectErrorMask, ch);
		if(msg->alarm.alarmAcceptServoDevice) setbitv(mask->acceptDeviceErrorMask, ch);
		if(msg->alarm.alarmAcceptCupError) setbitv(mask->acceptClearErrorMask, ch);
		//蛋托输送故障
		if(msg->alarm.alarmConveyServoConnect) setbitv(mask->conveyConnectErrorMask, ch);
		if(msg->alarm.alarmConveyServoDevice) setbitv(mask->conveyDeviceErrorMask, ch);
		//自动发托机故障
		if(msg->alarm.alarmAutoDispatchError) setbitv(mask->autoDispatchErrorMask, ch);
		//蛋托缺失
		if(msg->alarm.alarmTrayMiss) setbitv(mask->trayMissErrorMask, ch);
		//蛋托堆叠
		if(msg->alarm.alarmTrayStacke) setbitv(mask->trayStackedErrorMask, ch);

		ptr = (uint16_t *)&msg->alarm;
		if((*ptr > 0)||(*(ptr+1) > 0)){
			proData.packageState[ch] = 2;
		}else{
			proData.packageState[ch] = 1;
		}
	}else{	//设备离线
		//设置包装通道状态位报警
		setbitv(mask->packageConnectErrorMask, ch);
		clrbitv(mask->cache1ErrorMask, ch);
		clrbitv(mask->cache2ErrorMask, ch);
		clrbitv(mask->cache3ErrorMask, ch);
		clrbitv(mask->moveCacheConnectErrorMask, ch);
		clrbitv(mask->moveCacheDeviceErrorMask, ch);
		clrbitv(mask->moveCacheCupErrorMask, ch);
		clrbitv(mask->acceptConnectErrorMask, ch);
		clrbitv(mask->acceptDeviceErrorMask, ch);
		clrbitv(mask->acceptClearErrorMask, ch);
		clrbitv(mask->conveyConnectErrorMask, ch);
		clrbitv(mask->conveyDeviceErrorMask, ch);
		clrbitv(mask->autoDispatchErrorMask, ch);
		clrbitv(mask->trayMissErrorMask, ch);
		clrbitv(mask->trayStackedErrorMask, ch);
		proData.packageState[ch] = 0;
	}
}

/* 函数功能: 包装通道设备状态实时控制逻辑
 * queuePos：
 * channel：包装通道的通道序号
 * orbit：轨道编号
 * col：每排需要踢入几个蛋为放满
 */
static __IO uint8_t sndBuf[1024];
void sorter_control_realtime(uint16_t offset, uint8_t act, int ch, int col)
{
	int	pos = 0, n = 0, ob = 0, sta = 0, realQueue = 0;
	MasterSendMessageWork *work;
	MasterSendMessageStop *stop;
	AcceptSlaveMessage *accept;

	/**********踢蛋的时候需要记录是哪一个电磁铁踢的蛋 方便记录电磁铁漏踢次数*********/
	pos = 256 + ch*20;
	accept = (AcceptSlaveMessage *)&u_spiRxBuf[pos];
	package_alarm_info_analysis(accept, &alarmMask, ch);				//解析每个包装通道的报警状态
	
	pos = 256 + ch*16;
	if(0 == u_tab_registers[8319+ch]){ 	//通道停用
		stop = (MasterSendMessageStop *)&sndBuf[pos];
		stop->cmd = eMasterStopState;
	}else{															//通道启用
		work = (MasterSendMessageWork *)&sndBuf[pos];
		work->cmd = eMasterRunState;
		work->speed = 30;

		for(ob=0; ob<MAIN_ORBIT_NUM; ob++)
		{
			if((cacheState[ch].actBak != act)&&(1 == act)){ 					//触发开始踢蛋信号
				/* 到达电磁铁踢蛋限位:
					1.包装缓存第一层为就绪状态才可以踢蛋
					2.扫描主轨道缓存中蛋杯状态数据 
					3.需要计算每一个电磁铁是否需要打开
					4.鸡蛋位置根据队列来查找 抛弃通道那边开始是队列的第一号蛋杯	
				*/
				switch(ob){
					case 0: sta = accept->jthc_gd1; break;
					case 1: sta = accept->jthc_gd2; break;
					case 2: sta = accept->jthc_gd3; break;
					case 3: sta = accept->jthc_gd4; break;
					default: break;
				}
				if(0 == sta){										//主轨道ob第一层缓存为空
					for(n=0; n<col; n++){					//主轨道ob第一层缓存的6个蛋杯 isAct
						if(0 == getbitv(cacheState[ch].isFull[ob], n)){	//缓存蛋杯为空且当前蛋杯的蛋不满足踢蛋要求 则退出循环 为了实现踢蛋按照1,2,3,4,5,6的顺序 等待下一次可放入的鸡蛋到位
							realQueue = (mainOrbitQueue.front + cacheState[ch].mask + offset + n) % MAIN_ORBIT_QUEUE_SIZE;
							if((mainOrbitQueue.items[realQueue].toChannel[ob] == (ch+1))&&((n+1) == mainOrbitQueue.items[realQueue].isAct[ob])){  //满足主轨道蛋杯中的蛋刚好需要放入这一通道
								mainOrbitQueue.items[realQueue].isAct[ob] = 0;
								switch(ob)
								{
									case 0: work->dbFull1 = 1; setbitv(work->dctLine1, n); break;
									case 1: work->dbFull2 = 1; setbitv(work->dctLine2, n); break;
									case 2: work->dbFull3 = 1; setbitv(work->dctLine3, n); break;
									case 3: work->dbFull4 = 1; setbitv(work->dctLine4, n); break;
									default: break;
								}
								setbitv(cacheState[ch].isFull[ob], n);
							}else break;
						}
					}
				}
//				rt_kprintf("%d",work->dctLine1);
			}else if((cacheState[ch].actBak != act)&&(2 == act)){  //触发踢蛋结束信号
				switch(ob)
				{
					case 0: work->dctLine1 = 0; break;
					case 1: work->dctLine2 = 0; break;
					case 2: work->dctLine3 = 0; break;
					case 3: work->dctLine4 = 0; break;
					default: break;
				}
			}
		
			switch(cacheState[ch].state[ob]){
				case 0:    //判断第一层蛋杯缓存是否放满 如果放满则进入状态1 如果没放满则在当前状态继续等待
					for(n=0; n<col; n++){
						if(0 == getbitv(cacheState[ch].isFull[ob], n)){
							cacheState[ch].state[ob] = 0;
							break;
						} else {
							if(2 == act){
								cacheState[ch].orbitDelay[ob] = 0;
								cacheState[ch].state[ob] = 1;
							}
						}
					}
					break;
				case 1:    //延时135ms(根据鸡蛋自由落体时间得出) 通知包装通道将鸡蛋放入第二层缓存 进入状态4
					cacheState[ch].orbitDelay[ob]++;
					if(cacheState[ch].orbitDelay[ob] > 100){
						cacheState[ch].orbitDelay[ob] = 0;
						cacheState[ch].state[ob] = 2;
						switch(ob){
							case 0: work->dbFull1 = 2; break;
							case 1: work->dbFull2 = 2; break;
							case 2: work->dbFull3 = 2; break;
							case 3: work->dbFull4 = 2; break;
						}
					}
					break;
				case 2:    //延时100ms等待第一层缓存的蛋放入第二层缓存 进入状态0继续踢蛋
					cacheState[ch].orbitDelay[ob]++;
					if(cacheState[ch].orbitDelay[ob] > 50){
						cacheState[ch].orbitDelay[ob] = 0;
						cacheState[ch].state[ob] = 0;
						cacheState[ch].isFull[ob] = 0;
						switch(ob){
							case 0: work->dbFull1 = 0; break;
							case 1: work->dbFull2 = 0; break;
							case 2: work->dbFull3 = 0; break;
							case 3: work->dbFull4 = 0; break;
						}
					}
					break;
				default: cacheState[ch].state[0] = 0; break;
			}
		}
		
		cacheState[ch].actBak = act;					//记录踢蛋历史状态 用于信号变化时生成事件触发信号
	}
	
	rt_memcpy((uint8_t *)&u_spiTxBuf[pos], (uint8_t *)&sndBuf[pos], 16);
}

#include <stdio.h>
#include <stdlib.h>
uint32_t tick;
uint32_t mmm = 0, nnn = 0;
// 计算设备运行状态下发送的ecat数据  实时获取鸡蛋位置 分配鸡蛋 踢蛋控制 转载缓存数据加入主轨道队列等 需要周期调用 约4ms
void packag_channel_run_ctrol(void)
{
		int32_t dctOpenOffset = 0;
		int32_t dctCloseOffset = 0;
		int32_t dctEncoderTotal = 0;
		uint8_t status = 0;

		EggsClassifyInfo eggsClass;

		EggsGatherInfoEx gatEx;

		tickState.cPerCupPluse = PER_CUP_PLUSE;
	
		tickState.encoderCnt = firstPulseEncoderGet(); 					//读取编码器值
		
//		if(1 == weighsync.printflag){
//			weighsync.printflag = 0;
//			rt_kprintf("cod = %d\n", (tickState.encoderCnt+tickState.encoderRepostAdd) % 500);
//		}
		
		status = get_input_io_sta(masterOperate.inputData, eInputIO30);

		/* 如果收到了定向转载缓存向主轨道缓存放蛋的信号 每经过12个主轨道蛋杯触发一次脉冲信号 */
		if((1 == repostPulseBak)&&(0 == status)&&(tickState.encoderRepostAdd > 1000)){	//脉冲下降沿 并且脉冲间隔大于单个蛋杯脉冲的2/3才是有效脉冲
//			nnn++;
//			rt_kprintf("+++%d\n", nnn);
			srand(HAL_GetTick());
			tickState.encoderRepostAdd = 0;
			repost_complete_singal = 1;
			sort_sem_release();																		//通知蛋品分级计算线程读取转载缓存数据并计算鸡蛋要去的通道 耗时的计算需要异步处理 避免影响ethercat线程处理数据的实时性
		}

		repostPulseBak = status;

		status = get_input_io_sta(masterOperate.inputData, eInputIO16);

		/* 如果收到了主轨道缓存单个蛋杯脉冲 从主轨道缓存队列数据尾端移除一条数据 并查看数据鸡蛋是否被正常踢开 统计异常结果 */
		if((1 == eggCupPulseBak)&&(0 == status)&&(tickState.encoderCnt > 100)) //脉冲下降沿 并且脉冲间隔大于单个蛋杯脉冲的2/3才是有效脉冲
		{
			tickState.currentEncoder = tickState.cPerCupPluse;			//数据归一化处理 当一个蛋杯脉冲不足或超过设定值 需要对当前脉冲计数进行差值补全
			
			tickState.tickEncoderAdd += tickState.currentEncoder;		//踢蛋信号脉冲累计值

			tickState.encoderRepostAdd += tickState.encoderCnt;   	//转载信号的编码器累计值
			
			weighsync.compareRealEncoder += tickState.cPerCupPluse; //称重同步编码器累计值
			
//			mmm++;
//			rt_kprintf("---%d\n", tickState.encoderCnt);

			tickState.encoderCnt = 0;
			firstPulseEncoderClear();																//收到每个主轨道蛋杯脉冲信号时 编码器计数值清零
			
			weighsync.compareRealProx += 1;													//脉冲计数器
			
			cup_per_singal = 1;
			
			dequeueShowPageQueue(&showPageQueue, &gatEx);
		}else{
			tickState.currentEncoder = tickState.encoderCnt;
		}

		eggCupPulseBak = status;																	//用于检测单个蛋杯脉冲信号上升沿

		if((0 == repost_complete_singal)&&(1 == cup_per_singal)) {//检测插入蛋杯中的蛋是否掉入对应的主蛋杯中
			cup_per_singal = 0;
			malpositionDetectFlag = eggs_repost_malposition_detect();		
		}

		if(weighsync.isWaitForData == RT_TRUE){
			if((weighsync.compareRealEncoder + tickState.encoderCnt) >= weighsync.syncPosition){		//到达称重数据读取的位置  从modbus接收数据中读取一排鸡蛋的数据并加入称重裂纹数据队列中去
				sync_weigh_and_crack_data_to_queue();
				weighsync.isWaitForData = RT_FALSE;
			}
		}

		/* 主轨道蛋杯每移动3个蛋杯同步一次称重数据 */
		if(weighsync.compareRealProx >= 3){  
			weighsync.compareRealProx -= 3;
			weighsync.compareRealEncoder = 0;
			weighsync.isWaitForData = RT_TRUE;
		}

		/* 转载1号蛋杯偏移位置9 到达喷码机喷码位置 */
		if((tickState.encoderCnt >= tickState.tagPulseOffset) && (tickState.encoderCnt < tickState.tagRecoverOffset)){
			// 1.根据喷码信息控制喷码选择
			// 2.发送喷码使能脉冲
		} else if(tickState.encoderCnt >= tickState.tagRecoverOffset) { /* 到达喷码结束位置 关闭喷码使能 */
			// 1.结束喷码 关闭使能
		}

//		int16_t temp = u_tab_registers[5003];  //100
		int16_t temp  = 0;
		/* 踢蛋控制状态机 计算出踢蛋开始位置的脉冲计数值和结束值 调试的时候电动推缸的零位对应刚踢开蛋杯提前一点的位置 */
		switch(tickState.state)
		{
			case 0:
				dctOpenOffset = (tickState.cPerCupPluse + 60 + temp - cDctAheadPluse) % tickState.cPerCupPluse;
				dctEncoderTotal =  tickState.currentEncoder - dctOpenOffset;
				if((dctEncoderTotal >= 0)&&(dctEncoderTotal < 30)){		//准备踢蛋状态 满足条件时控制电磁铁打开
//					rt_kprintf("%d\n",dctEncoderTotal);
					tickState.pushrotStart = cDctAheadPluse;
					tickState.orbitStart = tickState.encoderCnt + tickState.tickEncoderAdd;
					tickState.orbitCupOffset = 0;
//					rt_kprintf("T%d %d %d %d %d\n", dctEncoderTotal, dctOpenOffset, tickState.orbitStart, cDctAheadPluse, tickState.orbitCupOffset);
//					rt_kprintf("T%d %d\n", tickState.tickEncoderAdd, tickState.encoderCnt);
//					tick = HAL_GetTick();
					tickState.state = 1;
					tickState.act = 1;
				}
				break;
			case 1:
				dctCloseOffset = tickState.orbitStart + 100;
				dctEncoderTotal = tickState.tickEncoderAdd + tickState.encoderCnt + cDctAheadPluse - tickState.pushrotStart;	//电磁铁踢蛋结束位置需要把电动推缸移动的位置加上去
				if(dctEncoderTotal >= dctCloseOffset){   							//踢蛋结束 控制电磁铁关闭
//					rt_kprintf("R%d %d %d %d %d\n\n", dctEncoderTotal, dctCloseOffset, tickState.tickEncoderAdd, tickState.encoderCnt, cDctAheadPluse);
//					rt_kprintf("R%d %d %d\n\n", tickState.tickEncoderAdd, tickState.encoderCnt, HAL_GetTick() - tick);
					if (tickState.tickEncoderAdd > 50000){
						tickState.tickEncoderAdd -= 40000;
						discard_channel_encoder_rebase(-40000);
					}
					tickState.state = 0;
					tickState.act = 2;
					dequeueMainOrbitQueue(&mainOrbitQueue, &eggsClass);
					if(eggsClass.isAct > 0){ 														//有鸡蛋没踢开  统计异常信息
						
					}
				}
				break;
			default: tickState.state = 0; break;
		}
		sorter_control_realtime(tickState.orbitCupOffset, tickState.act, 0, packageWorkList[0].col);   //如果通道需要单独设置偏移值  那么tickState.orbitCupOffset就改为通道蛋杯位置值 上面的踢蛋开始位置需要每个通道单独计算 主轨道数据队列用同一个就可以了 因为每个通道鸡蛋数据在轨道里标注有不同的信息 不会被错踢到其它通道 抛弃检测只需要用1通道的移除数据就可以了
		sorter_control_realtime(tickState.orbitCupOffset, tickState.act, 1, packageWorkList[1].col);
		sorter_control_realtime(tickState.orbitCupOffset, tickState.act, 2, packageWorkList[2].col);
		sorter_control_realtime(tickState.orbitCupOffset, tickState.act, 3, packageWorkList[3].col);
		
		cupDetectFlag = eggs_cup_close_detect(); 																	//蛋杯闭合报警检测
		discard_channel_process(tickState.tickEncoderAdd + tickState.encoderCnt);
}

int16_t calc_main_orbit_queue_offset(void)
{
	int diff = 0;
	
	if(mainOrbitQueue.rear != obritRearBak){
//		rt_kprintf("p=%d %d\n", mainOrbitQueue.rear, obritRearBak);
		diff = (mainOrbitQueue.rear >= obritRearBak) ? (mainOrbitQueue.rear - obritRearBak) : (mainOrbitQueue.rear + MAIN_ORBIT_QUEUE_SIZE - obritRearBak);
		obritRearBak = mainOrbitQueue.rear;
		checkPos += diff;
	}
	
	if(checkPos > 0) checkPos -= 1;

//	rt_kprintf("checkPos = %d\n", checkPos);
	
	return checkPos;
}

//偏移蛋杯数 主轨道组件 检测插入蛋杯中的蛋是否掉入对应的主蛋杯中 目前处理方法是停机报警 后期需要考虑把丢错位的数据清除 这样就不会将空杯踢到蛋托里面去
uint8_t eggs_repost_malposition_detect(void)
{
	uint16_t postion = 0, status = 0, ret = 0;
	
	int16_t temp = calc_main_orbit_queue_offset();

	//计算蛋的位置
	postion = (mainOrbitQueue.rear + MAIN_ORBIT_QUEUE_SIZE - egg_detect_postion - temp) % MAIN_ORBIT_QUEUE_SIZE;

//	if(u_tab_registers[5003] == 99) return 0;

	if(mainOrbitQueue.items[postion].toChannel[0] > 0){		//主轨道1这个位置本来应该有鸡蛋 让传感器去检查是否真有鸡蛋
		status = get_input_io_sta(masterOperate.inputData, eInputIO11); //主蛋杯接蛋检测1
		if(0 == status){		//主轨道1接蛋异常
			setbitv(ret, 0);
		}
	}

	if(mainOrbitQueue.items[postion].toChannel[1] > 0){		//主轨道2这个位置本来应该有鸡蛋 让传感器去检查是否真有鸡蛋
		status = get_input_io_sta(masterOperate.inputData, eInputIO12); //主蛋杯接蛋检测2
		if(0 == status){		//主轨道2接蛋异常
			setbitv(ret, 1);
		}
	}

	if(mainOrbitQueue.items[postion].toChannel[2] > 0){		//主轨道3这个位置本来应该有鸡蛋 让传感器去检查是否真有鸡蛋
		status = get_input_io_sta(masterOperate.inputData, eInputIO13); //主蛋杯接蛋检测3
		if(0 == status){		//主轨道3接蛋异常
			setbitv(ret, 2);
		}
	}

	if(mainOrbitQueue.items[postion].toChannel[3] > 0){		//主轨道4这个位置本来应该有鸡蛋 让传感器去检查是否真有鸡蛋
		status = get_input_io_sta(masterOperate.inputData, eInputIO14); //主蛋杯接蛋检测4
		if(0 == status){		//主轨道4接蛋异常
			setbitv(ret, 3);
		}
	}
	
//	if(ret > 0) rt_kprintf("%d--%04b\n", postion, ret);
	
	return ret;
}

// 主轨蛋杯闭合到位检测1~4
uint8_t eggs_cup_close_detect(void)
{
	uint8_t status = 0, ret = 0;

	status = get_input_io_sta(masterOperate.inputData, eInputIO20); //蛋杯闭合到位检测1
	if(1 == status){
		cup_close_counter[0]++;
		if(cup_close_counter[0] >= 5){
			cup_close_counter[0] = 5;
			setbitv(ret, 0);
		}
	}else{
		cup_close_counter[0] = 0;
	}

	status = get_input_io_sta(masterOperate.inputData, eInputIO21);	//蛋杯闭合到位检测2
	if(1 == status){
		cup_close_counter[1]++;
		if(cup_close_counter[1] >= 5){
			cup_close_counter[1] = 5;
			setbitv(ret, 1);
		}
	}else{
		cup_close_counter[1] = 0;
	}
	
	status = get_input_io_sta(masterOperate.inputData, eInputIO22);	//蛋杯闭合到位检测3
	if(1 == status){
		cup_close_counter[2]++;
		if(cup_close_counter[2] >= 5){
			cup_close_counter[2] = 5;
			setbitv(ret, 2);
		}
	}else{
		cup_close_counter[2] = 0;
	}
	
	status = get_input_io_sta(masterOperate.inputData, eInputIO23);	//蛋杯闭合到位检测4
	if(1 == status){
		cup_close_counter[3]++;
		if(cup_close_counter[3] >= 5){
			cup_close_counter[3] = 5;
			setbitv(ret, 3);
		}
	}else{
		cup_close_counter[3] = 0;
	}
	
	if(ret>0) rt_kprintf("~~~~~%04b\n", ret);
	
	return ret;
}

void packag_channel_stop_ctrol(void)
{
	int pos=0, ch = 0;
	uint16_t sndBuf[8];
	MasterSendMessageStop *stop;

	for(ch=0; ch<sorterUsedNumber; ch++){
		rt_memset((uint8_t *)sndBuf, 0, 16);
		stop = (MasterSendMessageStop *)&sndBuf;
		stop->cmd = eMasterStopState;
		if(RT_TRUE == alarmConfirmBtn){
			stop->alarmConfirm = 1;
		}else{
			stop->alarmConfirm = 0;
		}
		
		pos = 256 + ch*16;
		rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
	}
}

static uint8_t clearFlagBak[SORTER_USED_MAX] = {0};
void packag_channel_stop_response(void)
{
	int ch, pos, ob;
	AcceptSlaveMessage *accept;

	for(ch=0; ch<SORTER_USED_MAX; ch++){
		pos = 256 + ch*20;
		accept = (AcceptSlaveMessage *)&u_spiRxBuf[pos];
		if(accept->cmd == eMasterStopStateRsp){
			
			package_alarm_info_analysis(accept, &alarmMask, ch);		//解析每个包装通道的报警状态
			
			if((0 == clearFlagBak[ch])&&(1 == accept->clearFlag)){  //检查到通道清空触发信号 则清空踢蛋缓存数据
				for(ob=0; ob<MAIN_ORBIT_NUM; ob++){
					cacheState[ch].isFull[ob] = 0;
				}
//				rt_kprintf("clear ch%d\n", ch);
			}
			clearFlagBak[ch] = accept->clearFlag;
		}
	}
}

void set_package_param_1(void)
{
	int pos=0, ch = 0;
	uint16_t sndBuf[8];
	MasterSendMessageParam1 *param1;
	
	for(ch=0; ch<SORTER_USED_MAX; ch++){
		rt_memset((uint8_t *)sndBuf, 0, 16);
		param1 = (MasterSendMessageParam1 *)sndBuf;
		
		param1->cmd = eMasterSndParamState1;
		param1->row 					 = packageWorkList[ch].row;
		param1->col 					 = packageWorkList[ch].col;
		param1->autoTray 			 = packageWorkList[ch].autoTray;
		param1->trayCheck 		 = packageWorkList[ch].trayCheck;
		param1->transportStart = packageWorkList[ch].transportStart;
		param1->packBoxPost1 	 = packageWorkList[ch].packBoxPost1;
		param1->packBoxPost2 	 = packageWorkList[ch].packBoxPost2;
		
		pos = 256+ch*16;
		rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
	}
}

void set_package_param_2(void)
{
	int pos=0, ch = 0;
	uint16_t sndBuf[8];
	MasterSendMessageParam2 *param2;
	
	for(ch=0; ch<SORTER_USED_MAX; ch++){
		rt_memset((uint8_t *)sndBuf, 0, 16);
		param2 = (MasterSendMessageParam2 *)sndBuf;
		
		param2->cmd = eMasterSndParamState2;
		param2->packBoxPost3 = packageWorkList[ch].packBoxPost3;
		param2->packBoxPost4 = packageWorkList[ch].packBoxPost4;
		param2->packBoxPost5 = packageWorkList[ch].packBoxPost5;
		param2->packBoxPost6 = packageWorkList[ch].packBoxPost6;
		param2->packBoxPost7 = packageWorkList[ch].packBoxPost7;
		param2->packBoxPost8 = packageWorkList[ch].packBoxPost8;
		param2->chainLinkCnt = packageWorkList[ch].chainLinkCnt;
		
		pos = 256+ch*16;
		rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
	}
}

void set_package_param_3(void)
{
	int pos=0, ch = 0;
	uint16_t sndBuf[8];
	MasterSendMessageParam3 *param3;
	
	for(ch=0; ch<SORTER_USED_MAX; ch++){
		rt_memset((uint8_t *)sndBuf, 0, 16);
		param3 = (MasterSendMessageParam3 *)sndBuf;
		param3->cmd = eMasterSndParamState3;
		param3->alarmConfirm = 1;
		
		pos = 256+ch*16;
		rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
	}
}

void packag_channel_debug_ctrol(void)
{
	int i = 0, ch = -1, pos = -1;
	uint16_t sndBuf[8];
	uint8_t dctLine = 0;
	uint16_t jthcTemp =0;
	
	MasterSendMessageControl *ctrl;
	for(i=5580; i<5600; i++){
		if(1 == u_tab_registers[i]){
			ch = i-5580;
		}
	}
	
	if(ch < 0) return;

	rt_memset((uint8_t *)sndBuf, 0, 16);
	ctrl = (MasterSendMessageControl *)sndBuf;
	
	ctrl->cmd = eMasterControlState;					//命令码
	
	for(i=0; i<6; i++){												//电磁铁
		if(1 == u_tab_registers[5604+i]) setbitv(dctLine, i);
	}
	if(1 == u_tab_registers[5600])
		ctrl->dctLine1 = dctLine;
	if(1 == u_tab_registers[5601])
		ctrl->dctLine2 = dctLine;
	if(1 == u_tab_registers[5602])
		ctrl->dctLine3 = dctLine;
	if(1 == u_tab_registers[5603])
		ctrl->dctLine4 = dctLine;
	pos = -1;
	for(i=0; i<3; i++){												//阶梯缓存
		if(1==u_tab_registers[5611+i]) pos = i;
	}
	if(pos >= 0){
		jthcTemp = 0;
		for(i=0; i<4; i++){
			if(1 == u_tab_registers[5614+i])
				setbitv(jthcTemp, (pos*4+i));
		}
	}
	ctrl->jthcCtrl = jthcTemp;
	
	ctrl->ftjAct = u_tab_registers[5624]%2;		//发托机点动开关

	if(1 == u_tab_registers[5626]){  					//蛋拖输送线调试
		if(masterOperate.keyBtn == keyRunBtn) ctrl->dtssAct = 1;
		else if(masterOperate.keyBtn == keyRunBtnUp) ctrl->dtssAct = 0;
		if(1 == u_tab_registers[5627]) ctrl->dtssTurn = 1;
		else if(1 == u_tab_registers[5628]) ctrl->dtssTurn = 2;
		else ctrl->dtssTurn = 0;
	}else{
		ctrl->dtssAct = 0;
		ctrl->dtssTurn = 0;
	}

	if(1 == u_tab_registers[5633]){  					//接受抓调试
		if(masterOperate.keyBtn == keyRunBtn) ctrl->jszAct = 1;
		else if(masterOperate.keyBtn == keyRunBtnUp) ctrl->jszAct = 0;
		if(1 == u_tab_registers[5634]) ctrl->jszTurn = 1;
		else if(1 == u_tab_registers[5635]) ctrl->jszTurn = 2;
		else ctrl->jszTurn = 0;
	}else{
		ctrl->jszAct = 0;
		ctrl->jszTurn = 0;
	}

	if(1 == u_tab_registers[5640]){  						//移动缓存调试
		if(masterOperate.keyBtn == keyRunBtn) ctrl->ydhcAct = 1;
		else if(masterOperate.keyBtn == keyRunBtnUp) ctrl->ydhcAct = 0;	
		if(1 == u_tab_registers[5641]) ctrl->ydhcTurn = 1;
		else if(1 == u_tab_registers[5642]) ctrl->ydhcTurn = 2;
		else ctrl->ydhcTurn = 0;
		if(1 == u_tab_registers[5643]) ctrl->ydhcFlex = 1;
		else if(1 == u_tab_registers[5644]) ctrl->ydhcFlex = 2;
		else ctrl->ydhcFlex = 0;
	}else{
		ctrl->ydhcAct = 0;
		ctrl->ydhcTurn = 0;
		ctrl->ydhcFlex = 0;
	}
	
	ctrl->xpssAct = u_tab_registers[5653]%2; 		//斜坡输送电机开关控制
	
	pos = 256+ch*16; 													//通道1~20
	rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
}

void packag_channel_debug_response(void)
{
	int i = 0,ch = -1, pos = 0;
	
	AcceptSlaveMessage *accept;
	for(i=5580; i<5600; i++){
		if(1 == u_tab_registers[i]){
			ch = i-5580;
		}
	}
	if(ch < 0) return;
	pos = 256+ch*20; 															//通道1~20
	accept = (AcceptSlaveMessage *)&u_spiRxBuf[pos];

	if(accept->cmd == eMasterControlStateRsp){ 
		if(1 == u_tab_registers[5611]){							//第一层缓存蛋夹状态
			u_tab_registers[5618] = getbitv(accept->jthcSta, 0);
			u_tab_registers[5619] = getbitv(accept->jthcSta, 1);
			u_tab_registers[5620] = getbitv(accept->jthcSta, 2);
			u_tab_registers[5621] = getbitv(accept->jthcSta, 3);
		}else if(1 == u_tab_registers[5612]){ 			//第二层缓存蛋夹状态
			u_tab_registers[5618] = getbitv(accept->jthcSta, 4);
			u_tab_registers[5619] = getbitv(accept->jthcSta, 5);
			u_tab_registers[5620] = getbitv(accept->jthcSta, 6);
			u_tab_registers[5621] = getbitv(accept->jthcSta, 7);			
		}else if(1 == u_tab_registers[5613]){ 			//第三层缓存蛋夹状态
			u_tab_registers[5618] = getbitv(accept->jthcSta, 8);
			u_tab_registers[5619] = getbitv(accept->jthcSta, 9);
			u_tab_registers[5620] = getbitv(accept->jthcSta, 10);
			u_tab_registers[5621] = getbitv(accept->jthcSta, 11);			
		}

		u_tab_registers[5625] = accept->ftjPost; 		//发托机到位状态
		u_tab_registers[5629] = accept->dtss_dt;		//蛋托输送 蛋托检测
		u_tab_registers[5630] = accept->dtss_dl;		//蛋托输送 堆料检测
		u_tab_registers[5631] = accept->dtss_lw;		//蛋托输送 零位检测
		u_tab_registers[5636] = accept->jsz_dw;			//接受抓 接受抓到位
		u_tab_registers[5637] = accept->jsz_dj;			//接受抓 蛋夹关闭
		u_tab_registers[5638] = accept->jsz_db;			//接受抓 蛋杯检测
		u_tab_registers[5645] = accept->ydhc_dj;		//移动缓存 蛋夹关闭
		u_tab_registers[5646] = accept->ydhc_lw;		//移动缓存 零位检测
		u_tab_registers[5647] = accept->ydhc_xw1;		//移动缓存 限位开关1
		u_tab_registers[5648] = accept->ydhc_xw2;		//移动缓存 限位开关2
		u_tab_registers[5649] = accept->ydhc_jdw1;	//轨道1下面的第一层阶梯缓存状态 空或者有蛋
		u_tab_registers[5650] = accept->ydhc_jdw2;	//轨道2下面的第一层阶梯缓存状态 空或者有蛋
		u_tab_registers[5651] = accept->ydhc_jdw3;	//轨道3下面的第一层阶梯缓存状态 空或者有蛋
		u_tab_registers[5652] = accept->ydhc_jdw4;	//轨道4下面的第一层阶梯缓存状态 空或者有蛋
		
		u_tab_registers[5632] = accept->sstdServoPost;	//蛋托输送私服电机位置
		u_tab_registers[5639] = (accept->jszServoPost > 1000) ? 1000 : accept->jszServoPost; //接受抓私服电机位置
	}
}

void pipeline_debug_ctrol(void)
{
	//实现一些设备的手动开关功能
	//需要等输送线控制板到了才能控制
}

void extension_line_debug_ctrol(void)
{
//	uint16_t channel = 0;
	
//	channel = u_tab_registers[704];
//	switch(channel){   						//抛弃通道电磁铁控制
//		case 0:
//			break;
//		case 1:
//			break;
//		case 2:
//			break;
//		case 3:
//			break;
//		default: break;
//	}
	
	if(0 == u_tab_registers[5520]){			//抛弃通道驱动电机关
		
	}else if(1 == u_tab_registers[5520]){	//抛弃通道驱动电机开
		
	}
}

void extension_line_debug_response(void)
{
	
}

static uint16_t ecatSendCnt = 0;   //向ethercat发送数据是 计数延迟来确保从站已经收到数据
void edit_or_new_package_ctrol(void)
{
	int ch = -1, pos = -1, n;
	uint16_t sndBuf[8];
	MasterSendMessageControl *ctrl;
	
	ch = u_tab_registers[4702] % 20;
	for(n=0; n<SORTER_USED_MAX; n++)
	{
		rt_memset((uint8_t *)sndBuf, 0, 16);
		ctrl = (MasterSendMessageControl *)sndBuf;
		if(n == ch) {
			ctrl->cmd = eMasterEditState;
			if(u_tab_registers[5624]> 0){ 						//发托电机点动开关
				ctrl->ftjAct = 1;
			}else{
				ctrl->ftjAct = 0;
			}
			if(1 == u_tab_registers[5626]){  					 //蛋拖输送线调试
				if(masterOperate.keyBtn == keyRunBtn) ctrl->dtssAct = 1;
				else if(masterOperate.keyBtn == keyRunBtnUp) ctrl->dtssAct = 0;
				if(1 == u_tab_registers[5627]) ctrl->dtssTurn = 1;
				else if(1 == u_tab_registers[5628]) ctrl->dtssTurn = 2;
				else ctrl->dtssTurn = 0;
			}else{
				ctrl->dtssAct = 0;
				ctrl->dtssTurn = 0;
			}
			if(1 == u_tab_registers[5633]){  					//接受抓调试
				if(masterOperate.keyBtn == keyRunBtn) ctrl->jszAct = 1;
				else if(masterOperate.keyBtn == keyRunBtnUp) ctrl->jszAct = 0;
				if(1 == u_tab_registers[5634]) ctrl->jszTurn = 1;
				else if(1 == u_tab_registers[5635]) ctrl->jszTurn = 2;
				else ctrl->jszTurn = 0;
			}else{
				ctrl->jszAct = 0;
				ctrl->jszTurn = 0;
			}
			
			if(1 == u_tab_registers[4733]){
				ctrl->toOrigin = 1; //新建包装--回原点
				ecatSendCnt++;
				if(ecatSendCnt > 20) {
					ecatSendCnt = 0;
					u_tab_registers[4733] = 0;
				}
			}else ctrl->toOrigin = 0;
			
			if(1 == u_tab_registers[4734]){
				ctrl->toNext = 1;	//新建包装--下一步
				ecatSendCnt++;
				if(ecatSendCnt > 20) {
					ecatSendCnt = 0;
					u_tab_registers[4734] = 0;
				}
			}else ctrl->toNext = 0;
		}else {
			ctrl->cmd = eMasterStopState;
		}
		
		pos = 256+n*16;
		rt_memcpy((uint8_t *)&u_spiTxBuf[pos], sndBuf, 16);
	}
}

void edit_or_new_package_response(void)
{
	int ch = -1, pos = -1;
	
	AcceptSlaveMessage *accept;
	ch = u_tab_registers[4702] % 20;
	pos = 256+ch*20;
	accept = (AcceptSlaveMessage *)&u_spiRxBuf[pos];

	u_tab_registers[5625] = accept->ftjPost;   	//发托机到位状态
	
	u_tab_registers[5629] = accept->dtss_dt;		//蛋托输送 蛋托检测
	u_tab_registers[5630] = accept->dtss_dl;		//蛋托输送 堆料检测
	u_tab_registers[5631] = accept->dtss_lw;		//蛋托输送 零位检测
	
	u_tab_registers[5636] = accept->jsz_dw;			//接受抓 接受抓到位
	u_tab_registers[5637] = accept->jsz_dj;			//接受抓 蛋夹关闭
	u_tab_registers[5638] = accept->jsz_db;			//接受抓 蛋杯检测
	
	u_tab_registers[5632] = accept->sstdServoPost;	//蛋托输送私服电机位置
	u_tab_registers[5639] = (accept->jszServoPost > 1000) ? 1000 : accept->jszServoPost; //接受抓私服电机位置
}

/* 控制整机运行状态的有限状态机 */
void machine_work_process(void)
{
	switch(masterOperate.eSysState)
	{
		case eSystemIdle:  				//开机或者故障后恢复到初始状态
			break;
		case eSystemRun:  				//运行状态
			packag_channel_run_ctrol();  //在任何情况下只要机器在动就应该被执行
			break;
		case eSystemStop:  				//暂停状态
			packag_channel_stop_ctrol();
			packag_channel_stop_response();
			break;
		case eSystemReset:				//复位状态
			break;
		case eChannelStep: 				//包装通道调试 手动控制状态
			packag_channel_debug_ctrol();
			packag_channel_debug_response();
			break;
		case ePipelineStep: 			//输送线调试 手动控制状态
			pipeline_debug_ctrol();
			break;
		case eExtensionLineStep:	//分级线调试 手动控制状态
			extension_line_debug_ctrol();
			extension_line_debug_response();
			break;
		case ePackageEdit:				//包装管理中新建或者编辑包装盒的动作测试控制
			edit_or_new_package_ctrol();
			edit_or_new_package_response();
			break;
		case eSystemAlarm: 				//故障报警状态
			break;
		default:
			break;
	}
}

static void pluse_encoder(int argc, char **argv)
{
	rt_kprintf("= %d %d %d\n", firstPulseEncoderGet(), secondPulseEncoderGet(), thirdPulseEncoderGet());
}

MSH_CMD_EXPORT_ALIAS(pluse_encoder, pluse_encoder, show encoder pluse);
