#include "user_sorter.h"
#include "dev_config.h"
#include  "repostCacheQueue.h"
#include "mdbus_tcp_slaver.h"
#include "mainOrbitCacheQueue.h"
#include "user_kinco.h"

uint8_t bloodDev=0;   //设备是否启用 
uint8_t crackDev=0;
uint8_t dirtyDev=0;
uint8_t colorDev=0;
uint8_t weighDev=1;

enum SortModeEnum sortMode = eAmountMode;				//支持的分类模式



//RECIPE_PARAM_LIST_LEN 与 SORTER_USED_MAX 实际需要相等
/* 
 *数量均分模式
 */
static uint32_t packageLineRecord[RECIPE_PARAM_LIST_LEN] = {0};
static uint8_t packageChannelPostion[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};//通道号
static uint8_t oneChannelTickPostion[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};//踢蛋位置
static uint8_t channelCooldown[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};//通道冷却掩码
/* 同蛋品多规则数量均分：每个规则组、每条主轨道当前执行的规则下标 */
static uint8_t amountRulePosition[RECIPE_PARAM_LIST_LEN][REPOST_CACHE_LEN] = {0};
/*****************************************************************************************************/
/* 
 *重量均分模式
 */
 typedef struct
{
    uint8_t ruleLen;
    uint8_t lineIndex;
    uint8_t channel;
    uint8_t packageRow;
    uint8_t valid;
    uint32_t score;
} WeightAssignCandidate;

static uint32_t weight_avg_max_tolerance;  //每列重量最大误差
static uint32_t weight_max_base_tolerance; //单个蛋最大误差

#define TOLERANCE_PERCENT   2  			//差值百分比


/* 每条主轨道的每个包装通道一盘蛋已放总重量 */
static uint32_t weightChannelSum[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};
/* 每条主轨道的每个包装通道当前已放蛋数量 */
static uint8_t weightChannelCnt[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};

/*****************************************************************************************************/

//需要引用到user_FSmachine.c，如果换了配方的话需要重置
void mode_cache_reset(void)
{
	rt_memset(packageLineRecord, 0, sizeof(packageLineRecord));
    rt_memset(weightChannelSum, 0, sizeof(weightChannelSum));
    rt_memset(weightChannelCnt, 0, sizeof(weightChannelCnt));
    rt_memset(packageChannelPostion, 0, sizeof(packageChannelPostion));
    rt_memset(oneChannelTickPostion, 0, sizeof(oneChannelTickPostion));
	rt_memset(amountRulePosition, 0, sizeof(amountRulePosition));
}

/*
 * Line 代表一组可用包装通道
 */
static uint8_t get_package_line_record_index(uint32_t line)
{
	uint8_t i;

	if(line == 0){
		return 0xff;
	}

	/* 这组包装线之前出现过，直接复用原状态槽 */
	for(i = 0; i < RECIPE_PARAM_LIST_LEN; i++){
		if(packageLineRecord[i] == line){
			return i;
		}
	}

	/* 第一次出现的包装线集合，分配一个空状态槽 */
	for(i = 0; i < RECIPE_PARAM_LIST_LEN; i++){
		if(packageLineRecord[i] == 0){
			packageLineRecord[i] = line;
			return i;
		}
	}

	/* 状态槽用完 */
	return 0xff;
}

/* 获取 packageLine 中启用的包装通道数量 */
static uint8_t get_package_line_channel_count(uint32_t line)
{
	uint8_t i;
	uint8_t count = 0;

	for(i = 0; i < SORTER_USED_MAX; i++){
		if(getbitv(line, i) && (1 == u_tab_registers[8319+i])){
			count++;
		}
	}

	return count;
}

typedef enum
{
	eAmountAssignFailed = 0,       /* 当前规则没有成功分配 */
	eAmountAssignContinue,        /* 已分配，但当前规则本轮未完成 */
	eAmountAssignRuleRoundDone    /* 已分配，且当前规则本轮完成 */
} AmountAssignResult;
/*
 * 数量均分模式
 */
static AmountAssignResult get_channel_for_amount_average(SingleEggInfo *egg, WorkRecipeParam *param, uint8_t ruleLen)
{
	uint8_t n;
	uint8_t orbitIndex; //当前主轨道
	uint8_t lineIndex;
	uint8_t channel;
	uint8_t packageRow = 6;
	uint32_t line;
	PackageBoxInformation *box;
	uint8_t lastChannel = 0xff;

	orbitIndex = egg->orbitIndex;
	if(orbitIndex >= REPOST_CACHE_LEN){
//		rt_kprintf("111.\n");
		return eAmountAssignFailed;
	}
	line = param->singleRecipe.sortRule[ruleLen].packageLine;
	if(get_package_line_channel_count(line) == 0){
//		rt_kprintf("222\n");
		return eAmountAssignFailed;
	}
	/* 查找当前包装线中编号最大的映射且启用通道。 */
	for(n = 0; n < SORTER_USED_MAX; n++){
		if(getbitv(line, n) && (1 == u_tab_registers[8319 + n])){
			lastChannel = n;
		}
	}
	/* 理论上已由 get_package_line_channel_count() 检查过，此处作为保护。 */
	if(lastChannel == 0xff){
		return eAmountAssignFailed;
	}
	//rt_kprintf("222.\n");
	lineIndex = get_package_line_record_index(line);
	if(lineIndex >= RECIPE_PARAM_LIST_LEN){
//		rt_kprintf("333\n");
		return eAmountAssignFailed;
	}
	//rt_kprintf("333\n");
	box = getPackageBoxInformation(param->singleRecipe.sortRule[ruleLen].packageName);
	if((box != RT_NULL) && (box->col > 0)){
		packageRow = box->col;
	}
	for(n = 0; n < SORTER_USED_MAX; n++){
	//需要考虑中途关闭通道怎么处理
		channel = packageChannelPostion[orbitIndex][lineIndex];		
		if(getbitv(line, channel)&&(1 == u_tab_registers[8319+channel])){	
			egg->toChannel = channel + 1;
			egg->isAct = oneChannelTickPostion[orbitIndex][channel] + 1;
//			rt_kprintf("toChannel=%d,isAct=%d\n",egg->toChannel,egg->isAct);
			oneChannelTickPostion[orbitIndex][channel] =
				(oneChannelTickPostion[orbitIndex][channel] + 1) % packageRow;
			if(0 == oneChannelTickPostion[orbitIndex][channel]){
				packageChannelPostion[orbitIndex][lineIndex] =
					(channel + 1U) % SORTER_USED_MAX;

				/* 当前规则的最后一个有效通道完成一列，本规则一轮结束。 */
				if(channel == lastChannel){
					packageChannelPostion[orbitIndex][lineIndex] = 0;
					return eAmountAssignRuleRoundDone;
				}
			}
			return eAmountAssignContinue;
		}
		else{
			packageChannelPostion[orbitIndex][lineIndex] = (channel + 1) % SORTER_USED_MAX;
		}
	}	
	return eAmountAssignFailed;
}
	
static uint32_t get_weight_average_tolerance(uint8_t afterCount,
											 uint8_t eggCount)
{
	uint32_t multiplier;
	uint32_t tolerance;

	if((afterCount == 0) || (eggCount == 0)){
		return weight_avg_max_tolerance;
	}

	/*
	 * 前期允许更大的累计偏差，给重轻蛋补偿留空间；
	 * 第一颗：130%
	 * 最后一颗：100%
	 */
	if(afterCount >= eggCount){
		multiplier = 100U;
	}else{
		multiplier = 100U +
			((uint32_t)(eggCount - afterCount) * 30U) /
			(eggCount - 1U);
	}

	tolerance =
		((uint64_t)weight_avg_max_tolerance * multiplier + 99U) /
		100U;

	return tolerance;
}




/**
 * @brief 计算当前蛋放入后的适配误差
 * @param eggWeight 鸡蛋重量
 * @param curWeight 当前已放总重量
 * @param curCount 当前已放数量
 * @param eggCount 目标蛋数
 * @param targetWeight 目标总重
 */
static uint32_t get_weight_fit_error(uint16_t eggWeight,
                                     uint32_t curWeight,
                                     uint8_t curCount,
                                     uint8_t eggCount,
                                     uint32_t targetWeight)
{
    uint32_t afterWeight;
    uint32_t expectedWeight;

    if(curCount >= eggCount){
        return 0xffffffff;
    }

    afterWeight = curWeight + eggWeight;
    expectedWeight =
        ((uint64_t)targetWeight * (curCount + 1U)) / eggCount;

    if(afterWeight > expectedWeight){
        return afterWeight - expectedWeight;
    }

    return expectedWeight - afterWeight;
}


static rt_bool_t weight_is_match(uint16_t eggWeight,
								 uint32_t curWeight,
								 uint8_t curCount,
								 uint8_t eggCount,
								 uint32_t targetWeight,
								 uint32_t *err_out)
{
	uint32_t err;
	uint32_t stageTolerance;

	if(curCount >= eggCount){
		return RT_FALSE;
	}

	err = get_weight_fit_error(eggWeight,
							   curWeight,
							   curCount,
							   eggCount,
							   targetWeight);

	if(err_out != RT_NULL){
		*err_out = err;
	}

	/*
	 * curCount 是放入前数量；
	 * 所以当前蛋放入后数量是 curCount + 1。
	 */
	stageTolerance =
		get_weight_average_tolerance(curCount + 1, eggCount);

	return (err <= stageTolerance) ? RT_TRUE : RT_FALSE;
}




static void weight_commit_to_channel(SingleEggInfo *egg,
									 uint8_t orbitIndex,
									 uint8_t lineIndex,
									 uint8_t channel,
									 uint8_t packageRow)
{
	egg->toChannel = channel + 1;
	egg->isAct = oneChannelTickPostion[orbitIndex][channel] + 1;

	weightChannelSum[orbitIndex][channel] += egg->weigh;
	weightChannelCnt[orbitIndex][channel]++;

	oneChannelTickPostion[orbitIndex][channel] =
		(oneChannelTickPostion[orbitIndex][channel] + 1) % packageRow;

	if((0 == oneChannelTickPostion[orbitIndex][channel]) && (weightChannelCnt[orbitIndex][channel] >= packageRow)){
		channelCooldown[orbitIndex][channel] =  packageRow;
		weightChannelSum[orbitIndex][channel] = 0;
		weightChannelCnt[orbitIndex][channel] = 0;
		packageChannelPostion[orbitIndex][lineIndex] = (channel + 1) % SORTER_USED_MAX;
	}
}




//通道重量均分模式
//此模式必须为同种蛋进行重量均分
//一盘蛋重量固定=规格平均值*蛋托蛋数
//此模式最好开放一条通道作为数量均分模式，分配失败时进行数量均分兜底
void get_channel_for_weight_average(SingleEggInfo *egg, WorkRecipeParam *param, uint8_t ruleLen,WeightAssignCandidate *best)
{
	uint8_t n;
	uint8_t orbitIndex;
	uint8_t lineIndex;  //映射对象
	uint8_t channel;
	uint8_t packageRow = 6;// 默认单行列数6，后面用包装盒配置覆盖
	uint8_t remainCount;
	uint8_t bestChannel = 0xff; // 0xff代表无最优通道
	uint32_t line;
	uint32_t targetWeight;
	uint32_t err;
	uint32_t bestScore = 0xffffffff;// 初始最大分数
	uint32_t score; 	//通道打分
	PackageBoxInformation *box;

	orbitIndex = egg->orbitIndex;
	if(orbitIndex >= REPOST_CACHE_LEN){
		return;
	}

	line = param->singleRecipe.sortRule[ruleLen].packageLine;
	if(get_package_line_channel_count(line) == 0){
		return;
	}

	lineIndex = get_package_line_record_index(line);
	if(lineIndex >= RECIPE_PARAM_LIST_LEN){
		return;
	}

	box = getPackageBoxInformation(param->singleRecipe.sortRule[ruleLen].packageName);
	if((box == RT_NULL) || (box->row == 0) || (box->col == 0)){
		return;
	}
	packageRow = box->col;
	
	targetWeight = param->singleRecipe.sortRule[ruleLen].targetBox / box->row;//得到每列的重量
	if(targetWeight == 0){
		return;
	}
	// 单列最大总误差：单列目标重量 × 2%
	weight_avg_max_tolerance = ((uint64_t)targetWeight * TOLERANCE_PERCENT + 99U)  / 100;
	/* 最终列容差不能小于一个蛋可能带来的最大波动 */
	if(weight_avg_max_tolerance < weight_max_base_tolerance){
		weight_avg_max_tolerance = weight_max_base_tolerance;
	}
	/* 扫描同一包装线所有可用通道，选择评分最优的，最后一个通道作为兜底数量均分 */
	for(n = 0; n < SORTER_USED_MAX; n++){
		channel = n;
		if(!getbitv(line, channel)){
			continue;
		}

		if(1 != u_tab_registers[8319+channel]){
			continue;
		}
		// 如果通道处于冷却期，跳过
		if (channelCooldown[orbitIndex][channel] > 0) {
			continue;
		}
		if(weightChannelCnt[orbitIndex][channel] >= packageRow){
			continue;
		}

		remainCount = packageRow - weightChannelCnt[orbitIndex][channel];

		if(weight_is_match(egg->weigh,
						   weightChannelSum[orbitIndex][channel],
						   weightChannelCnt[orbitIndex][channel],
						   packageRow,
						   targetWeight,
						   &err)){

			/* 评分 = 误差 * 100 + 剩余位置（误差优先，剩余位置越少分数越低） */
			//score = (uint32_t)err * 100 + (uint32_t)remainCount * 1000;//剩余位置权重更大
			score = err * 1000 + remainCount;  // 误差权重更大

			if(score < bestScore){
				bestScore = score;
				bestChannel = channel;
			}
		}
	}

	/* 找到最佳通道，提交 */
	// if(bestChannel != 0xff){
	// 	weight_commit_to_channel(egg, orbitIndex, lineIndex, bestChannel, packageRow);
	// }
	if((bestChannel != 0xff) &&((best->valid == 0) || (bestScore < best->score))){ //同规则下的最佳通道
		best->ruleLen = ruleLen;
		best->lineIndex = lineIndex;
		best->channel = bestChannel;
		best->packageRow = packageRow;
		best->score = bestScore;
		best->valid = 1;
	}

/*	// 兜底通道：本组最后一路通道，纯重量均分模式，跳过重量匹配
	else 
	{
		uint8_t backup_ch = SORTER_USED_MAX - 1;
		// 校验兜底通道有效、未满
		if(getbitv(line, backup_ch) && (1 == u_tab_registers[8319+backup_ch])
		   && (weightChannelCnt[orbitIndex][backup_ch] < packageRow))//这里需要看是否踢蛋完成
		{
			weight_commit_to_channel(egg, orbitIndex, lineIndex, backup_ch, packageRow);
		}
		else
		{
			// 兜底通道也满，只能去抛弃通道
			//send_egg_to_recycle(egg);
		}
	}
*/		
}
static uint8_t get_amount_rule_group_index(WorkRecipeParam *param, uint8_t ruleLen)
{
	uint8_t i;

	for(i = 0; i < RECIPE_PARAM_LIST_LEN; i++){
		if(RT_TRUE == compare_names(param->singleRecipe.sortRule[i].productName,
									param->singleRecipe.sortRule[ruleLen].productName,
									5)){
			return i;
		}
	}

	return ruleLen;
}

//鸡蛋分类执行器 鸡蛋分类时 先从异常蛋开始分类 减少计算量
void eggs_sort_actuator(SingleEggInfo *egg, WorkRecipeParam *param)
{
	rt_bool_t ret=RT_TRUE;
	int16_t param_Low,param_High;
	uint8_t ruleLen, tag=0, n;
	uint8_t matchedRuleList[RECIPE_PARAM_LIST_LEN] = {0};//规则组
	uint32_t matchedRuleWeightTolerance[RECIPE_PARAM_LIST_LEN] = {0};//规则组对应误差
	uint32_t currentRuleWeightTolerance = 0;
	uint8_t matchedRuleCount = 0;
	uint8_t orbitIndex;
	uint8_t groupIndex;
	uint8_t startIndex;
	uint8_t tryIndex;
	uint8_t selectRule;
	uint8_t i;
	static int16_t egg_num = 0,egg_num1 = 0,egg_num2 = 0;
	
	AmountAssignResult assignRet;
	ProductManageInformation *product;
	WeightAssignCandidate best = {0};

	orbitIndex = egg->orbitIndex;
	egg_num ++;
	/* 遍历配方中20个规则参数 找到当前输入鸡蛋满足哪些分级规则 */
	for(ruleLen=0; ruleLen<RECIPE_PARAM_LIST_LEN; ruleLen++){
		currentRuleWeightTolerance = 0;
		//根据产品名称获取参数
		product = getProductManageInformation(param->singleRecipe.sortRule[ruleLen].productName);
		if(product == RT_NULL){
			continue;
		}

		//设备启用 血斑分类器筛选
		if((1==bloodDev)&&(product->bloodLevel>0)){  

		}

		//设备启用 裂纹分类器筛选
		if((1==crackDev)&&(product->crackLevel>0)){
			ret = RT_FALSE;
			for(n=0; n<5; n++){
				if(getbitv(product->crackLevel, n)){ 
				if(n<4){//从标准库获取
					param_Low = getModbusHoldingRegU16(8150+3*n);
					param_High = getModbusHoldingRegU16(8150+3*n+1);
				}
				//自定义值从初始化去获取
				else{
					param_Low = product->crackLow;
					param_High = product->crackHigh;
				}
					ret = compare_range(param_Low, param_High, egg->level);
					//符合这个条件c
					if(RT_TRUE==ret) break;
				}
			}
		}else{
			//规则中没有配置裂纹 则认为满足这条规则
			ret = RT_TRUE;
		}
		if(RT_FALSE == ret) continue;
		
		//设备启用 脏蛋分类器筛选
		if((1==dirtyDev)&&(product->dirtyLevel>0)){
			ret = RT_FALSE;
			for(n=0; n<4; n++){
				if(getbitv(product->dirtyLevel, n)){
					if(n<3){
							param_Low = getModbusHoldingRegU16(8100+3*n);
							param_High = getModbusHoldingRegU16(8100+3*n+1);
					}//自定义值从初始化去获取
					else{
						param_Low = product->dirtyLow;
						param_High = product->dirtyHigh;
					}				
					ret = compare_range(param_Low, param_High, egg->dirty);
					//符合这个条件
					if(RT_TRUE==ret) break;					
				}
			}
		}else{
			//规则中没有配置脏蛋 则认为满足这条规则
			ret = RT_TRUE;
		}
		if(RT_FALSE == ret) continue;

		//设备启用 颜色分类器筛选
		if((1==colorDev)&&(product->colorLevel>0)){
			ret = RT_FALSE;
			for(n=0; n<5; n++){
				if(getbitv(product->colorLevel, n)){
				if(n<4){
					param_Low = getModbusHoldingRegU16(8050+2*n);
					param_High = getModbusHoldingRegU16(8050+2*n+1);
				}//自定义值从初始化去获取
					else{
						param_Low = product->colorLow;
						param_High = product->colorHigh;
					}		
					ret = compare_range(param_Low, param_High, egg->color);
					//符合这个条件
					if(RT_TRUE==ret) break;				
				}
			}
		}else{
			//规则中没有配置颜色 则认为满足这条规则
			ret = RT_TRUE;
		}
		if(RT_FALSE == ret) continue;
		
		//设备启用 重量分类器筛选
		if((1==weighDev)&&(product->weighLevel>0)){
			ret = RT_FALSE;
			for(n=0; n<6; n++){
				if(getbitv(product->weighLevel, n)){
					if(n<5){
							param_Low = getModbusHoldingRegU16(8000+2*n);
							param_High = getModbusHoldingRegU16(8000+2*n+1);
					}//自定义值从初始化去获取
					else{
						param_Low = product->weighLow;
						param_High = product->weighHigh;
					}
					ret = compare_range(param_Low, param_High, egg->weigh);
					//符合这个条件
					if(RT_TRUE == ret){
						currentRuleWeightTolerance  =
							param_High - ((param_High + param_Low) / 2);
						break;
					}
//						else rt_kprintf("555\n");
				}
			}
		}else{
		
			//规则中没有配置重量 则认为满足这条规则
			currentRuleWeightTolerance = 10000U;
			ret = RT_TRUE;
		}

		//找到了满足全部匹配规则的条件
		if(RT_TRUE == ret){
			matchedRuleList[matchedRuleCount] = ruleLen;
			matchedRuleWeightTolerance[matchedRuleCount] = currentRuleWeightTolerance;
			matchedRuleCount++;
		}			
	}

//	rt_kprintf("param_Low=%d,param_High=%d,weigh=%d,ret=%d,matchedRuleCount=%d,sortMode=%d\n",
//			   param_Low,param_High,egg->weigh,ret,matchedRuleCount,sortMode);
	if(matchedRuleCount > 0){
		/* 有通道可以接收这个鸡蛋 */
		switch(sortMode){
			case eAmountMode:		//数量均分模式	
				if(orbitIndex >= REPOST_CACHE_LEN){
				break;
				}			
				groupIndex = get_amount_rule_group_index(param, matchedRuleList[0]); //查找第一条规则号
				/*
				 * 从上次记录的规则游标开始。
				 * 如果上次规则这次不匹配，则从匹配列表第0条开始。
				 */
				startIndex = 0;
				for(i=0; i<matchedRuleCount; i++){
					if(matchedRuleList[i] == amountRulePosition[groupIndex][orbitIndex]){//获取当前运行的规则号
						startIndex = i;
						break;
					}
				}

				/*
				 * 尝试当前蛋匹配到的所有规则。
				 * 当前规则无可用通道时，尝试下一条规则。
				 * 当前规则完成所有通道一轮后，才切换到下一条规则。
				 */
				for(i=0; i<matchedRuleCount; i++){
					tryIndex = (startIndex + i) % matchedRuleCount;//从当前规则开始尝试
					selectRule = matchedRuleList[tryIndex]; //拿到规则的下标

					assignRet = get_channel_for_amount_average(egg, param, selectRule);
					if(assignRet != eAmountAssignFailed){
						egg_num1++;
						product = getProductManageInformation(param->singleRecipe.sortRule[selectRule].productName);
						if(product != RT_NULL){
							egg->toTag = product->printNumber;
						}else{
							egg->toTag = tag;
						}

						if(assignRet == eAmountAssignRuleRoundDone){
							tryIndex = (tryIndex + 1) % matchedRuleCount; //切换下一条规则
							amountRulePosition[groupIndex][orbitIndex] = matchedRuleList[tryIndex]; 	
						}else{
							amountRulePosition[groupIndex][orbitIndex] = selectRule; //保持原规则
						}
//		rt_kprintf("%d,%d,%d,%d,%d\n",egg_num,egg_num1,egg_num2,egg->toChannel,egg->isAct);
						return;
					}
					else
					{
						egg_num2++;
					}
				}
				break;

			case eWeighMode:
			
				for(i = 0; i < SORTER_USED_MAX; i++){
					if(channelCooldown[orbitIndex][i] > 0){
						channelCooldown[orbitIndex][i]--;
					}
				}

				rt_memset(&best, 0, sizeof(best));
				for(i = 0; i < matchedRuleCount; i++){ //遍历所有通道进行匹配
   					weight_max_base_tolerance = matchedRuleWeightTolerance[i];
					get_channel_for_weight_average(egg,
												param,
												matchedRuleList[i],
												&best);
				}
				if(best.valid != 0){   //分蛋
					weight_commit_to_channel(egg,
											orbitIndex,
											best.lineIndex,
											best.channel,
											best.packageRow);

					product = getProductManageInformation(
						param->singleRecipe.sortRule[best.ruleLen].productName);
					egg->toTag = (product != RT_NULL) ? product->printNumber : tag;
					return;
				}
				break;


			case eFreeMode:			//自由分配模式
				break;

			default: break;
		}
	}
	/* 没有匹配规则能匹配上这个鸡蛋，或者匹配规则无可用通道，鸡蛋被抛弃 */
	egg->toChannel = 100;	//100--表示没有通道可以接收鸡蛋
	egg->toTag = tag;
	egg->isAct = RT_TRUE;
}
