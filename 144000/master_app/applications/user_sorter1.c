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
static uint8_t channelCooldown[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};//通道掩码

/*****************************************************************************************************/


/* 
 *重量均分模式
 */
static uint32_t weight_avg_max_tolerance;  //每列重量最大误差
static uint32_t weight_avg_base_tolerance; //单蛋平均误差

#define TOLERANCE_PERCENT   2  			//差值百分比
#define WEIGHT_AVG_PRIORITY_REMAIN   2	//剩余多少个位置时，进入全通道择优模式 

/* 每条主轨道的每个包装通道一盘蛋已放总重量 */
static uint32_t weightChannelSum[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};
/* 每条主轨道的每个包装通道当前已放蛋数量 */
static uint8_t weightChannelCnt[REPOST_CACHE_LEN][SORTER_USED_MAX] = {0};

/*****************************************************************************************************/


void mode_cache_reset(void)
{
		rt_memset(packageLineRecord, 0, sizeof(packageLineRecord));
    rt_memset(weightChannelSum, 0, sizeof(weightChannelSum));
    rt_memset(weightChannelCnt, 0, sizeof(weightChannelCnt));
    rt_memset(packageChannelPostion, 0, sizeof(packageChannelPostion));
    rt_memset(oneChannelTickPostion, 0, sizeof(oneChannelTickPostion));
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


/*
 * 数量均分模式
 */
void get_channel_for_amount_average(SingleEggInfo *egg, WorkRecipeParam *param, uint8_t ruleLen)
{
	uint8_t n;
	uint8_t orbitIndex; //当前主轨道
	uint8_t lineIndex;
	uint8_t channel;
	uint8_t packageRow = 6;
	uint32_t line;
	PackageBoxInformation *box;

	orbitIndex = egg->orbitIndex;
	if(orbitIndex >= REPOST_CACHE_LEN){
		return;
	}
	rt_kprintf("111.\n");
	line = param->singleRecipe.sortRule[ruleLen].packageLine;
	if(get_package_line_channel_count(line) == 0){
		return;
	}

	rt_kprintf("222.\n");
	lineIndex = get_package_line_record_index(line);
	if(lineIndex >= RECIPE_PARAM_LIST_LEN){
		return;
	}
	rt_kprintf("333\n");
	box = getPackageBoxInformation(param->singleRecipe.sortRule[ruleLen].packageName);
	if((box != RT_NULL) && (box->row > 0)){
		packageRow = box->col;
	}
	for(n = 0; n < SORTER_USED_MAX; n++){
	//需要考虑中途关闭通道怎么处理
		channel = packageChannelPostion[orbitIndex][lineIndex];
		if(getbitv(line, channel)&&(1 == u_tab_registers[8319+channel])){
		
			egg->toChannel = channel + 1;
			egg->isAct = oneChannelTickPostion[orbitIndex][channel] + 1;
			rt_kprintf("toChannel=%d,isAct=%d\n",egg->toChannel,egg->isAct);
			oneChannelTickPostion[orbitIndex][channel] =
				(oneChannelTickPostion[orbitIndex][channel] + 1) % packageRow;
	
			if(0 == oneChannelTickPostion[orbitIndex][channel]){
				packageChannelPostion[orbitIndex][lineIndex] = (channel + 1) % SORTER_USED_MAX;
			}
	
			return;
		}else{
			packageChannelPostion[orbitIndex][lineIndex] = (channel + 1) % SORTER_USED_MAX;
		}
	}

}

// 重量均分动态容差分层阈值
#define TOL_STAGE_MORE     4   // 剩余空位大于该值，大容错
#define TOL_MULTI_MORE     4U
#define TOL_STAGE_MID      2   // 剩余空位大于该值，中等容错
#define TOL_MULTI_MID      2U
#define TOL_RETURN_MIN     1U  // 容差最小值，防止0容错

static uint16_t get_weight_average_tolerance(uint8_t remainCount)
{
    uint32_t max_tolerance = weight_avg_max_tolerance;
    uint32_t single_tolerance = weight_avg_base_tolerance;

	// 零保护：基础容差为0时强制最小容差，避免所有蛋匹配失败
    if(single_tolerance == 0)
    {
        single_tolerance = TOL_RETURN_MIN;
    }
    // 剩余蛋数越多，允许的单蛋误差越大
    if(remainCount > TOL_STAGE_MORE)
    {
        // 放大倍数不超过总列容差
        uint32_t tol = single_tolerance * TOL_MULTI_MORE;
        return (tol > max_tolerance) ? max_tolerance : tol;
    }
    else if(remainCount > TOL_STAGE_MID) // 剩余3~4个
    {
        uint32_t tol = single_tolerance * TOL_MULTI_MID;
        return (tol > max_tolerance) ? max_tolerance : tol;
    }
    // 剩余1~2个，直接使用单蛋容差
    return single_tolerance;
}


/**
 * @brief 计算当前蛋放入后的适配误差
 * @param eggWeight 当前蛋重量
 * @param curWeight 当前通道这一盘已放总重量
 * @param curCount 当前通道这一盘已放数量
 * @param eggCount 一盘目标蛋数
 * @param targetWeight 一盘目标总重量
 * @return uint32_t 误差，越小越适合
 */
static uint32_t get_weight_fit_error(uint16_t eggWeight,
									 uint32_t curWeight,
									 uint8_t curCount,
									 uint8_t eggCount,
									 uint32_t targetWeight)
{
	uint8_t remainCount;
	uint32_t afterWeight;
	uint32_t remainWeight;
	uint32_t needAvg;

	if(curCount >= eggCount){
		return 0xff;
	}

	remainCount = eggCount - curCount;
	afterWeight = curWeight + eggWeight;

	/*
	 * 最后一个位置，直接用放入后的总重误差判断。
	 * 这样能保证最后一个蛋不会因为平均重量不合适而卡死。
	 */
	if(remainCount <= 1){
		if(afterWeight > targetWeight){
			return afterWeight - targetWeight;
		}else{
			return targetWeight - afterWeight;
		}
	}

	/*
	 * 如果当前已经达到或超过目标重量，
	 * 不再计算 needAvg = 0，否则误差会退化成 eggWeight。
	 * 这里直接用放入后的总重超差作为误差。
	 */
	if(afterWeight >= targetWeight){
		return afterWeight - targetWeight;
	}
	/*
	 * 正常情况：根据剩余目标重量 / 剩余位置数，
	 * 判断当前蛋是否接近当前应该放入的平均重量。
	 */
	remainWeight = targetWeight - curWeight;
	needAvg = remainWeight / remainCount;

	if(eggWeight > needAvg){
		return eggWeight - needAvg;
	}else{
		return needAvg - eggWeight;
	}
}


static rt_bool_t weight_is_match(uint16_t eggWeight,
                                 uint32_t curWeight,
                                 uint8_t curCount,
                                 uint8_t eggCount,
                                 uint32_t targetWeight,
                                 uint32_t *err_out)
{
    uint8_t remainCount;
    uint16_t single_tolerance;   // 当前剩余蛋数下的单蛋容差
    uint32_t expectedWeight;     // 当前已放蛋数的理论重量
    uint32_t deviation;          // 当前已放重量与理论重量的偏差
    uint32_t allowedCumulativeDeviation; // 允许的累积偏差
    uint32_t err;
	uint32_t afterWeight;

    if(curCount >= eggCount){
        return RT_FALSE;
    }

    remainCount = eggCount - curCount;
    afterWeight = curWeight + eggWeight;  // 放入本蛋后的总重
    
    single_tolerance = get_weight_average_tolerance(remainCount);  // 动态容差

    // ========== 前置拦截：判断当前已放重量是否偏离理论值过多 ==========
    // 理论重量 = 目标总重 × 已放蛋数（加上本蛋） / 总蛋数
    expectedWeight = (targetWeight * (curCount + 1)) / eggCount;
    deviation = (afterWeight > expectedWeight) ? (afterWeight - expectedWeight) : (expectedWeight - afterWeight);

    // 允许的累积偏差 = 单蛋容差 × 已放蛋数（按当前动态容差计算）
    allowedCumulativeDeviation = (uint32_t)single_tolerance * (curCount + 1);
    if(allowedCumulativeDeviation > weight_avg_max_tolerance){
        allowedCumulativeDeviation = weight_avg_max_tolerance;
    }

    if (deviation > allowedCumulativeDeviation) {
        return RT_FALSE;  // 当前累积偏差超出允许范围 
    }

    // ========== 单蛋适配判断 ==========
    err = get_weight_fit_error(eggWeight, curWeight, curCount, eggCount, targetWeight);

    if (err_out != RT_NULL){
        *err_out = err;
    }

    return (err <= single_tolerance) ? RT_TRUE : RT_FALSE;
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
void get_channel_for_weight_average(SingleEggInfo *egg, WorkRecipeParam *param, uint8_t ruleLen)
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
	targetWeight = param->singleRecipe.sortRule[ruleLen].targetBox / box->row;
	// 单列最大总误差：单列目标重量 × 2%
	weight_avg_max_tolerance = targetWeight * TOLERANCE_PERCENT  / 100;
	//鸡蛋单位是0.1g，放大了十倍。这里可能有风险？会出现0
	weight_avg_base_tolerance = param->singleRecipe.sortRule[ruleLen].targetBox * TOLERANCE_PERCENT /100/(box->row*box->col
);
	if(targetWeight == 0){
		return;
	}
	for(n = 0; n < SORTER_USED_MAX; n++){ //鸡蛋冷却	
		if (channelCooldown[orbitIndex][n] > 0) {
			channelCooldown[orbitIndex][n]--;
		}
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
	if(bestChannel != 0xff){
		weight_commit_to_channel(egg, orbitIndex, lineIndex, bestChannel, packageRow);
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


//鸡蛋分类执行器 鸡蛋分类时 先从异常蛋开始分类 减少计算量
void eggs_sort_actuator(SingleEggInfo *egg, WorkRecipeParam *param)
{
	rt_bool_t ret=RT_TRUE;
	int16_t param_Low,param_High;
	uint8_t ruleLen, tag=0, n;
	ProductManageInformation *product;
	
	/* 遍历配方中20个规则参数 找到当前输入鸡蛋满足哪一条分级规则 */
	for(ruleLen=0; ruleLen<RECIPE_PARAM_LIST_LEN; ruleLen++){
		//根据产品名称获取参数
		product = getProductManageInformation(param->singleRecipe.sortRule[ruleLen].productName);

		//设备启用 血斑分类器筛选
		if((1==bloodDev)&&(product->bloodLevel>0)){  //这个功能可能做在主轨道位置  过了检测位置后对主轨道中的鸡蛋二次分配

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
				else{param_Low = getModbusHoldingRegU16(4333);;param_High = getModbusHoldingRegU16(4334);}
					ret = compare_range(param_Low, param_High, egg->level);
					//符合这个条件c
					if(RT_TRUE==ret) break;
				}
			}
//			rt_kprintf("111.\n");
		}else{
//				rt_kprintf("111+++\n");
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
							param_Low = getModbusHoldingRegU16(4335);
							param_High = getModbusHoldingRegU16(4336);
					}				
					ret = compare_range(param_Low, param_High, egg->dirty);
					//符合这个条件
					if(RT_TRUE==ret) break;					
				}
			}
//			rt_kprintf("222.\n");
		}else{
			//规则中没有配置脏蛋 则认为满足这条规则
			ret = RT_TRUE;
//			rt_kprintf("222+++.\n");
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
							param_Low = getModbusHoldingRegU16(4331);
							param_High = getModbusHoldingRegU16(4332);
					}		
					ret = compare_range(param_Low, param_High, egg->color);
					//符合这个条件
					if(RT_TRUE==ret) break;				
				}
			}
//			rt_kprintf("333\n");
		}else{
			//规则中没有配置颜色 则认为满足这条规则
			ret = RT_TRUE;
//			rt_kprintf("333+++\n");
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
					if(RT_TRUE==ret) break;			
				}
			}
//			rt_kprintf("param_Low=%d,param_High=%d,weigh=%d,ret=%d,n=%d\n",param_Low,param_High,egg->weigh,ret,n);
		}else{
//			rt_kprintf("444+++\n");
			//规则中没有配置重量 则认为满足这条规则
			ret = RT_TRUE;
		}
		//为鸡蛋找到了满足全部匹配规则的条件 退出当前循环
		if(RT_TRUE == ret){
			tag = product->printNumber; //喷码编号为自定义
			break;
		}			
	}
	rt_kprintf("param_Low=%d,param_High=%d,weigh=%d,ret=%d,ruleLen=%d,sortMode=%d\n",param_Low,param_High,egg->weigh,ret,ruleLen,sortMode);
	if(ruleLen<RECIPE_PARAM_LIST_LEN){
		/* 有通道可以接收这个鸡蛋 */
		switch(sortMode){
			case eAmountMode:		//数量均分模式
				get_channel_for_amount_average(egg, param, ruleLen);
				egg->toTag = tag;
				break;
			case eWeighMode:		//重量均分模式
				get_channel_for_weight_average(egg, param, ruleLen);
				egg->toTag = tag;
				break;
			case eFreeMode:			//自由分配模式
				break;
			default: break;
		}
	}else{
		/* 没有匹配规则能匹配上这个鸡蛋 鸡蛋被抛弃 */
		egg->toChannel = 100;	//100--表示没有通道可以接收鸡蛋
		egg->toTag = tag;
		egg->isAct = RT_TRUE;
	}
}
