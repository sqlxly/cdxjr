#include <rtthread.h>
#include <board.h>
#include "./tools/filter.h"
#include "./tools/common.h"
#include "stdio.h"
#include <stdlib.h>
#include <arm_math.h>

#define AVG_STABLE_LEN   40																						//称重均值稳定次数

#define STABLE_THRESHOLD_CNT  20 																			//稳定次数达到该值时认为信号已经稳定

#define CLEAR_STABLE_THRESHOLD_CNT  30 																//清零稳定次数达到该值时认为信号已经稳定

static uint8_t clear_stable_cnt;
static uint8_t savePos[WEIGH_CHANNEL];													 			//采集值当前存储位置
static uint8_t completed[WEIGH_CHANNEL];															//完成称重采集标志位
static uint32_t dynamicZero[WEIGH_CHANNEL]; 	  											//动态零位

static uint8_t tempStableCnt[WEIGH_CHANNEL]; 													//临时存储每个通道连续稳定次数
static double tempStableVector[WEIGH_CHANNEL][STABLE_THRESHOLD_CNT];  //临时存储每个通道的稳定值
static uint8_t stablePos[WEIGH_CHANNEL];															//稳定值当前存储位置
static double stableVector[WEIGH_CHANNEL];														//记录误差小的那一组平均值
static double prevLevel[WEIGH_CHANNEL]; 															//记录误差范围 与当前误差比较
static double totalLevel[WEIGH_CHANNEL]; 															//记录重量最大值

static uint8_t tempClearStableCnt[WEIGH_CHANNEL]; 										//临时存储每个通道连续稳定次数
static double tempClearStableVector[WEIGH_CHANNEL][CLEAR_STABLE_THRESHOLD_CNT]; //临时存储每个通道的稳定值
static uint8_t clearPos[WEIGH_CHANNEL];													 			//稳定值当前存储位置
static double stableClearVector[WEIGH_CHANNEL];												//记录误差小的那一组平均值
static double prevClearLevel[WEIGH_CHANNEL]; 													//记录误差范围 与当前误差比较

static double avgVector[WEIGH_CHANNEL][AVG_STABLE_LEN];								//每个通道的连续采集值

static KalmanInfo kalman[WEIGH_CHANNEL];															//卡尔曼滤波系数
static double eggWeight[WEIGH_CHANNEL] = {0.0f};											//测量的鸡蛋真实重量
static uint8_t weighThresholdCnt[WEIGH_CHANNEL] = {0};
static uint8_t reset_weigh[WEIGH_CHANNEL] = {0};

static uint8_t printFlag = 10;

static uint16_t pluseFlag = 0;

extern uint8_t calib_flag;																						//进行过称重校准的标志

extern uint8_t stop_flag;																							//机器暂停运行标志

extern uint8_t peelOutErr[6];
extern uint8_t weighSensorErr[6];

extern void setModbusHoldingReg(uint16_t addr, uint16_t v);

extern void setModbusHoldingRegU32(rt_uint16_t base_id, rt_uint32_t value);

extern uint16_t getModbusHoldingReg(uint16_t addr);

void get_eggs_weigh_online(double *pweight)
{
	rt_memcpy(pweight, &eggWeight[0], sizeof(double)*WEIGH_CHANNEL);
	for(int ch=0; ch<WEIGH_CHANNEL; ch++)
	{
		eggWeight[ch] = 0.0f;
	}
}

void software_filter_init(void)
{
	int ch, n;
	Weigh_Linear_Info *info;

	info = get_weigh_calibration();

	clear_stable_cnt = 10;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++)
	{
		for(n=0; n<AVG_STABLE_LEN; n++)
		{
			avgVector[ch][n] = 0.0f;
		}
		
		savePos[ch] = 0;
		stablePos[ch] = 0;
		clearPos[ch] = 0;
		
		eggWeight[ch] = 0.0f;
		prevLevel[ch] = 4000000000.0f;
		tempStableCnt[ch] = 0;

		prevClearLevel[ch] = 4000000000.0f;
		tempClearStableCnt[ch] = 0;

		for(n=0; n<STABLE_THRESHOLD_CNT; n++)
		{
			tempStableVector[ch][n] = 0.0f;
		}

		for(n=0; n<clear_stable_cnt; n++)
		{
			tempClearStableVector[ch][n] = 0.0f;
		}

		completed[ch] = 0;

		dynamicZero[ch] = info->ZeroAD[ch];

		InitKalmanInfo(&kalman[ch], 0.005, 0.06, dynamicZero[ch]);
	}
}

void isSignalStableEx(double val, int ch)
{
	int n = 0;
	double max = 0.0, min = 4000000000.0, total = 0.0, level = 0.0;
	
	tempStableVector[ch][stablePos[ch]] = val; 	 	// 记录连续稳定值
	stablePos[ch] = (stablePos[ch] + 1) % STABLE_THRESHOLD_CNT;
	tempStableCnt[ch]++; 													// 连续稳定个数增加
	if (tempStableCnt[ch] >= STABLE_THRESHOLD_CNT)
	{
		total = 0;
		tempStableCnt[ch] = STABLE_THRESHOLD_CNT;
		for(n=0; n<STABLE_THRESHOLD_CNT; n++)
		{
			if(tempStableVector[ch][n] > max) max = tempStableVector[ch][n];
			if(tempStableVector[ch][n] < min) min = tempStableVector[ch][n];
			total += tempStableVector[ch][n];
		}
		if(RT_TRUE == stop_flag){										//如果是停机启动 则找出最大均值那一段作为称重结果
			if(total > totalLevel[ch])
			{
				totalLevel[ch] = total;
				stableVector[ch] = total / STABLE_THRESHOLD_CNT;
			}
			if(ch == 5) rt_kprintf("t%d\n", total);
		}else{																			//正常运行状态下 找出称重最平稳的那一段作为称重结果
			level = fabs(max - min);
			if(level < prevLevel[ch])
			{
				prevLevel[ch] = level;
				stableVector[ch] = total / STABLE_THRESHOLD_CNT;
			}
		}
	}
}

void isClearSignalStableEx(double val, int ch)
{
	int n = 0;
	double max = 0.0, min = 4000000000.0, total = 0.0, level = 0.0;

	tempClearStableVector[ch][clearPos[ch]] = val; 	 				// 记录连续稳定值
	clearPos[ch] = (clearPos[ch] + 1) % clear_stable_cnt;
	tempClearStableCnt[ch]++; 														 	// 连续稳定个数增加
	if (tempClearStableCnt[ch] >= clear_stable_cnt)
	{
		total = 0;
		tempClearStableCnt[ch] = clear_stable_cnt;
		for(n=0; n<clear_stable_cnt; n++)
		{
			if(tempClearStableVector[ch][n] > max) max = tempClearStableVector[ch][n];
			if(tempClearStableVector[ch][n] < min) min = tempClearStableVector[ch][n];
			total += tempClearStableVector[ch][n];
		}
		
		level = fabs(max - min);
		if(level < prevClearLevel[ch])
		{
			prevClearLevel[ch] = level;
			stableClearVector[ch] = total / clear_stable_cnt;
		}
	}
}

void balance_signal_handler(rt_uint32_t adc, int ch)
{
	char buf[30];
	double val = 0;
	Weigh_Linear_Info *info;
	
	info = get_weigh_calibration();
	
	val = KalmanFilter(&kalman[ch], adc);

	if(ch == printFlag){
		sprintf(buf, "%.1f,%d,\n", val - info->ZeroAD[ch]+100, pluseFlag);
		if(pluseFlag > 0) pluseFlag = 0;
		rt_kprintf("%s", buf);
	}
	
	avgVector[ch][savePos[ch]] = val;

	/* 超过记重门限 记录重量参数 */
	if(val > (info->ZeroAD[ch] + info->weighThreshold[ch]))
	{
		weighThresholdCnt[ch] += 1;
		if(weighThresholdCnt[ch] > 3)
		{
			weighThresholdCnt[ch] = 3;
			if((completed[ch] == 0)||(completed[ch] == 1))
			{
				isSignalStableEx(val, ch);
				completed[ch] = 1;
			}
		}
	}else{
		if(completed[ch] == 1)
		{
//			sprintf(buf, "ch%d_%lf \n", ch, prevLevel[ch]);
//			rt_kprintf("%s ", buf);
			completed[ch] = 2;
			if(stop_flag) rt_kprintf("restart!\n");
			stop_flag = RT_FALSE;
		}

		tempStableCnt[ch] = 0;
		prevLevel[ch] = 4000000000.0f;
		weighThresholdCnt[ch] = 0;
	}

	/* 记录接近开关两个脉冲之间的最小值 */
	if(val < (info->ZeroAD[ch] + info->weighThreshold[ch]))
	{
		isClearSignalStableEx(val, ch);
	}

	//接近开关每接收一个脉冲便重置一下称重数据
	if(reset_weigh[ch] == 1)
	{
		reset_weigh[ch] = 0;
		
		completed[ch] = 0;
		tempStableCnt[ch] = 0;
		prevLevel[ch] = 4000000000.0f;
		weighThresholdCnt[ch] = 0;
		
		tempClearStableCnt[ch] = 0;
		prevClearLevel[ch] = 4000000000.0f;
		tempClearStableCnt[ch] = 0;
	}
	
	savePos[ch] = (savePos[ch] + 1) % AVG_STABLE_LEN;
}

void clear_dynamic_zero_vector(void)
{
	int ch;
	rt_base_t lock;
	double temp = 0, tEggWeight = 0;
	Weigh_Linear_Info *info;
	
	info = get_weigh_calibration();
	
	pluseFlag = 6000;
	
	stop_flag = RT_FALSE;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++)
	{
		tempClearStableCnt[ch] = 0;
		lock = rt_hw_interrupt_disable();
		//*********动态误差prevClearLevel小于某个阀值时才进行动态清零***********
		if(prevClearLevel[ch] < WEIGH_CLEAR_LEVEL)
		{
			dynamicZero[ch] = stableClearVector[ch];
//			rt_kprintf("winding up weigh!\n");
		}
		prevClearLevel[ch] = 4000000000.0f;
//		dynamicZero[ch] = info->ZeroAD[ch]; //动态零位等于校准零位 用于不能动态清零的情况
		rt_hw_interrupt_enable(lock);
				
		//清除称重校准的标志 解决称重校准后第一排鸡蛋没有重量的问题
		if((completed[ch] == 2)||(completed[ch] == 1)||(getbitv(calib_flag, ch) == 1))  //当鸡蛋是被后面的挡杆推过去的时候 completed还在1这个状态 此时称重不准确 
		{
			temp = 0.0;
			if(stableVector[ch] > 0.0)
			{
				lock = rt_hw_interrupt_disable();
				temp = stableVector[ch];
				stableVector[ch] = 0.0;
				rt_hw_interrupt_enable(lock);
			}

			if(getbitv(calib_flag, ch) == 1)
			{
				clrbitv(calib_flag, ch);
				eggWeight[ch] = 0.0f;
			}else{
				if(temp > dynamicZero[ch])
				{
					tEggWeight = (temp - dynamicZero[ch]) * info->linearK[ch];
					if(tEggWeight > eggWeight[ch])
					{
						eggWeight[ch] = tEggWeight + 0.05;
					}
					//重量超过200g 认为称重传感器采集数据异常
					if(tEggWeight < 200.0f){
						weighSensorErr[ch] = 0;
					}else{
						weighSensorErr[ch] += 1;
						if(weighSensorErr[ch] >= 3){
							weighSensorErr[ch] = 3;
						}
					}
//					rt_kprintf("dynamicZero:%d ", dynamicZero[ch]);
				}else{
					eggWeight[ch] = 0.0f;
				}
			}
		}
		reset_weigh[ch] = 1;
	}
}

uint32_t get_adc_average_value(uint8_t ch)
{
	uint16_t i = 0, turn = 0;
	double total = 0.0f, ret = 0.0f;
	
	for(i=0; i<AVG_STABLE_LEN; i++)
	{
		if(avgVector[ch][i] > 0.0f)
		{
			total += avgVector[ch][i];
			turn += 1;
		}
	}

	if(turn > 0) 
	{
		ret = total / turn;
	}
	
	return (uint32_t)ret;
}

void calc_current_weighing(void)
{
		uint32_t ch, ad = 0, disp = 0;
		double tempWeigh = 0, tempWeighEx = 0, peelWeigh = 0;
		Weigh_Linear_Info *info;
		
		info = get_weigh_calibration();
		for(ch=0; ch<WEIGH_CHANNEL; ch++)
		{
			ad = get_adc_average_value(ch);
			setModbusHoldingRegU32(149+ch*2, ad);

			tempWeigh = ((int32_t)dynamicZero[ch] - (int32_t)info->ZeroAD[ch]) * info->linearK[ch] * 10.0f;
			disp = tempWeigh;
			setModbusHoldingReg(121+ch, disp);

			tempWeighEx = ((int32_t)ad - (int32_t)info->ZeroAD[ch]) * info->linearK[ch] * 10.0f + 0.5f - tempWeigh;
			disp = tempWeighEx;
			setModbusHoldingReg(103+ch, disp);

			//累计去皮报警
			peelWeigh = getModbusHoldingReg(128);
			peelWeigh = peelWeigh / 10.0f;
			if(tempWeigh <= peelWeigh){
				peelOutErr[ch] = 0;
			}else{
				peelOutErr[ch] += 1;
				if(peelOutErr[ch] >= 3){
					peelOutErr[ch] = 3;
				}
			}
		}
}

void clear_current_weighing(uint8_t ch, uint32_t ad)
{
	dynamicZero[ch] = ad;
}

void weigh(int argc, char *argv[])
{
	printFlag = atoi(argv[1]);
	if(printFlag > 10){
		clear_stable_cnt = printFlag;
	}
}

MSH_CMD_EXPORT(weigh, weigh function);

void kalmanFilter(int argc, char *argv[])
{
	int ch = 0;
	double Q = 0, R = 0;
	
	Q = atof(argv[1]);
	R = atof(argv[2]);
	for(ch=0; ch<WEIGH_CHANNEL; ch++)
	{
		InitKalmanInfo(&kalman[ch], Q, R, dynamicZero[ch]);
	}
}

MSH_CMD_EXPORT(kalmanFilter, kalman filter param);
