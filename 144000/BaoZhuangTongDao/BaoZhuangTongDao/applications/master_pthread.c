#include <rtthread.h>
#include <rtdevice.h>
#include "sn74hc165pwr.h"
#include "devConfig.h"
#include "IOdriver.h"
#include "ecat_data_proc.h"
#include "arm_math.h"
#include "master402_od.h"
#include "servo_planner.h"
#include <stdlib.h>

// 注意：靠近蛋托出口位置为主轨道1
// 各层缓存之间的高度在105mm左右 根据自由落体运动时间计算为150ms
#define DCF_OPEN_TIME				20    //20x10 = 200ms
#define DCF_CLOSE_TIME			10
#define DCF_OPEN_TIME_EX1		15		//250ms  移动缓存和第三层缓存气缸开启时间
#define DCF_OPEN_TIME_EX2		15		//250ms  移动缓存和第三层缓存气缸开启时间
#define DCF_OP_TIMEOUT			100   //100x10 = 1000ms
#define CACHE_CHANNEL				4			//每一层4列通道缓存
#define LEN_RUN							43 		//GET_PIN(C, 11)

const int32_t clear_puu_throd = 2000000000;   //伺服电机PUU脉冲计数清零的门限值 20亿PUU单位

static rt_sem_t dynamic_sem = RT_NULL;

static struct rt_thread master_thread;
static char master_thread_stack[4096];
static rt_bool_t sem_flag = 0;

static ButtonOperateTypeDef btReset;  				//通道复位按钮
static ButtonOperateTypeDef btActive; 				//通道动作按钮

static uint8_t origin_sigal_bak[3] = {0};			//用于通道复位时原点信号备份

static uint8_t autoTray_sigal_bak = 0;				//用于自动发托时原点信号备份
	
ChanbelOperateTypeDef cop;

uint8_t channelClearFlag = 0;									//通道清空标志 需要发送给主控

extern int canopen_init(void);
extern void alta_motor_close(uint8_t id);
extern void alta_motor_open(uint8_t id);
extern uint8_t get_servo_state(uint8_t index);

// N3 移动缓存 基准满速
#define YDHC_BASE_SPD    50000
// N4 接收抓 基准满速
#define JSZ_BASE_SPD     21000
// N2 输送通道 两档基准速度
#define CONVEY_NORMAL_BASE_SPD  10000  // 单格前进基准
#define CONVEY_LOOP_BASE_SPD    25000   // 整盘循环基准
#define ACCELERATION_RATIO  10

static uint8_t target_egg_num = 0;
static uint8_t convey_speed_flag = 2;//变快需要输送通道先快
static uint8_t jsz_speed_flag = 2;////变慢需要接受爪先慢
static uint8_t egg_load_level = 0;
static uint8_t egg_load_level_bak = 0;
static uint8_t egg_load_level_next = 0;
// 电机倍率查表，放大十倍
static const float speed_ratio_table[4] = {0.6, 0.7, 0.8, 1.0};
static const uint8_t accelertation_ratio[4] = {10, 10, 10 ,10};//移动缓存，接受爪，蛋托单格速度，蛋托循环速度

// 全局总蛋数分级索引：0~2 /3~5 /6~8 /9~12
typedef enum
{
    EGG_LEVEL_LOW = 0,    // 0~2，低速
    EGG_LEVEL_MID1 = 1,   // 3~5，中低速
    EGG_LEVEL_MID2 = 2,   // 6~7，中高速
    EGG_LEVEL_HIGH = 3    // 8~12 满速
} EggLoadLevelDef;

void system_param_init(ChanbelOperateTypeDef *op)
{
	/* 初始化气缸状态 */
	op->dcfState = 0;
	updateDcfIO(op->dcfState);

	for(int i=0; i<12; i++){
		op->jthcOP[i].sta = 0;
		op->jthcOP[i].flag = 0;
		op->jthcOP[i].flag = 0;
	}
	
	op->ydhcOP.sta = 0;
	op->ydhcOP.flag = 0;
	op->ydhcOP.mark = 0; 
	op->ydhcOP.speed = 0;
	op->ydhcOP.count = 0;
	
	op->jszOP.sta = 0;
	op->jszOP.flag = 0;
	op->jszOP.speed = 0;
	
	op->sstdOP.sta = 0;
	op->sstdOP.flag = 0;
	op->sstdOP.type = 0;
	op->sstdOP.speed = 0;
	op->sstdOP.sstd_rst = 0;
	
	clrbitv(cop.dcfState, eXPSSout1);  //关闭斜坡输送电机
}

void getAllInputPortState(uint8_t *data, uint8_t len)
{
		rt_memcpy(data, cop.inputData, len);
}

//按钮电平状态读取函数
uint8_t Read_Button_Reset_Level(void)
{
  return get_input_port_sta(cop.inputData, eANdin3);
}
uint8_t Read_Button_Active_Level(void)
{
  return get_input_port_sta(cop.inputData, eANdin4);
}

static uint8_t ClearAllFlag = 0;
void Btn_Reset_Down_CallBack(void *btn)
{
	ClearAllFlag = 1;
}

void Btn_Reset_Up_CallBack(void *btn)
{
	// 1.空闲模式下才能按复位键 电机复位->进入空闲状态
	cop.keyBtn = keyResetBtn;
	ClearAllFlag = 0;
}
void Btn_Active_Up_CallBack(void *btn)
{
	// 1.空闲模式下才能按通道动作键 接受抓转一圈放下一排鸡蛋->进入空闲状态
	cop.keyBtn = keyActiveBtn;
}
void Btn_Active_Long_CallBack(void *btn)
{
	// 1.空闲模式下才能按通道清零键 将缓存通道的所有鸡蛋都放入蛋托->进入空闲状态
	if(1 == ClearAllFlag) cop.keyBtn = keyClearAllBtn;
	else cop.keyBtn = keyClearBtn;
}

//包装通道按键初始化函数
void channel_button_init(void)
{
	/* 每个按钮只绑定单击和长按两种状态 */
	Button_Create(&btReset.btn, Read_Button_Reset_Level, KEY_ON);
	Button_Attach(&btReset.btn, BUTTON_DOWM, Btn_Reset_Down_CallBack); 	 //单击
	Button_Attach(&btReset.btn, BUTTON_UP, Btn_Reset_Up_CallBack); 	 //单击
//	Button_Attach(&btReset.btn, BUTTON_LONG, Btn_Reset_Dowm_CallBack); 	//长按
	
	Button_Create(&btActive.btn, Read_Button_Active_Level, KEY_ON);
	Button_Attach(&btActive.btn, BUTTON_UP, Btn_Active_Up_CallBack); //单击
	Button_Attach(&btActive.btn, BUTTON_LONG, Btn_Active_Long_CallBack); //长按
}

static uint8_t get_egg_load_level(void)
{
	uint8_t all_egg = target_egg_num; //全部12缓存总蛋数

    if(all_egg >= 8)
        return EGG_LEVEL_HIGH;
    else if(all_egg >= 6)
        return EGG_LEVEL_MID2;
    else if(all_egg >= 3)
        return EGG_LEVEL_MID1;
    else
        return EGG_LEVEL_LOW;
}

//计算每一条主轨道下面阶梯缓存的鸡蛋缓存数量 找出缓存数量最多的那条通道优先控制移动缓存去接蛋
uint8_t calc_hurry_up_cache_floor(ChanbelOperateTypeDef *op)
{
	uint8_t floor[CACHE_CHANNEL+1], i, pos, temp = 0;

	target_egg_num = 0;
	for(i=0; i<CACHE_CHANNEL; i++)
	{
		floor[i] = 0;
		//1 floor
		pos = eDCFout1+i;
		if(0 != op->jthcOP[pos].flag){
			floor[i] += 1;
			target_egg_num += 1;
		}
		//2 floor
		pos = eDCFout5+i;
		if(0 != op->jthcOP[pos].flag){
			floor[i] += 1;
			target_egg_num += 1;
		}
		//3 floor
		pos = eDCFout9+i;
		if(0 != op->jthcOP[pos].flag){
			floor[i] += 1;
			target_egg_num += 1;
		}
	}


	//查找最大值
	temp = floor[0];
	for(i=1; i<CACHE_CHANNEL; i++){
		if(temp < floor[i]) temp = floor[i];
	}
	
	//优先选择最远的1、4轨道
	if(0 == temp) return 0;
	else if(floor[0] == temp) return 1;
	else if(floor[3] == temp) return 4;
	else if(floor[1] == temp) return 2;
	else if(floor[2] == temp) return 3;
	else return 0;
}

void package_channel_manual_control_func(ChanbelOperateTypeDef *op)
{
	if(mCtrl.ftjAct){  //发托机点动开关
		setbitv(op->dcfState, eFTDJout2);
	}else{
		clrbitv(op->dcfState, eFTDJout2);
	}

	if(mCtrl.dtssAct){ //蛋托输送 运行1/停止0
		cylinderState[0].eState = eCylinderOpen;
		if(1 == mCtrl.dtssTurn) {  //正转1/反转2/无效0
			if(5 == cylinderState[0].uStateEx){
					N2_pos_cmd1 += 2000;
			}
		}else if(2 == mCtrl.dtssTurn) {
			if(5 == cylinderState[0].uStateEx){
					N2_pos_cmd1 -= 2000;
			}
		}
	}else{
		cylinderState[0].eState = eCylinderClose;
		N2_pos_cmd1 = N2_position_actual_value_6064;
	}
	
	if(mCtrl.jszAct){ //接收爪 运行1/停止0
		cylinderState[2].eState = eCylinderOpen;
		if(1 == mCtrl.jszTurn) {  //正转1/反转2/无效0
			if(5 == cylinderState[2].uStateEx){
					N4_pos_cmd1 += 2000;
			}
		}else if(2 == mCtrl.jszTurn) {
			if(5 == cylinderState[2].uStateEx){
					N4_pos_cmd1 -= 2000;
			}
		}
	}else{
		cylinderState[2].eState = eCylinderClose;
		N4_pos_cmd1 = N4_position_actual_value_6064;
	}
	
	if(mCtrl.ydhcAct){ //移动缓存 运行1/停止0
		cylinderState[1].eState = eCylinderOpen;
		if(1 == mCtrl.ydhcTurn) {  //正转1/反转2/无效0
			if(5 == cylinderState[1].uStateEx){
					N3_pos_cmd1 += 1000;
			}
		}else if(2 == mCtrl.ydhcTurn) {
			if(5 == cylinderState[1].uStateEx){
					N3_pos_cmd1 -= 1000;
			}
		}
	}else{
		cylinderState[1].eState = eCylinderClose;
		N3_pos_cmd1 = N3_position_actual_value_6064;
	}
	
	if(1 == mCtrl.ydhcFlex){	//移动缓存 伸出1/收缩2/无效0
		setbitv(op->dcfState, eDCFout13);
	}else if(2 == mCtrl.ydhcFlex){
		clrbitv(op->dcfState, eDCFout13);
	}else{
		clrbitv(op->dcfState, eDCFout13);
	}
	
	if(mCtrl.xpssAct){  //斜坡输送电机开关控制
		setbitv(op->dcfState, eXPSSout1);
	}else{
		clrbitv(op->dcfState, eXPSSout1);
	}
}

static uint8_t toOriginBak = 0, toNextBak = 0;
//新建包装盒信息时 需要手动控制包装通道
void package_channel_new_edit_func(ChanbelOperateTypeDef *op)
{
	int no = 0;
	
	if(mCtrl.ftjAct){  //发托机点动开关
		setbitv(op->dcfState, eFTDJout2);
	}else{
		clrbitv(op->dcfState, eFTDJout2);
	}

	if(mCtrl.dtssAct){ //蛋托输送 运行1/停止0
		cylinderState[0].eState = eCylinderOpen;
		if(1 == mCtrl.dtssTurn) {  //正转1/反转2/无效0
			if(5 == cylinderState[0].uStateEx){
					N2_pos_cmd1 += 2000;
			}
		}else if(2 == mCtrl.dtssTurn) {
			if(5 == cylinderState[0].uStateEx){
					N2_pos_cmd1 -= 2000;
			}
		}
	}else{
		cylinderState[0].eState = eCylinderClose;
		N2_pos_cmd1 = N2_position_actual_value_6064;
	}

	if(mCtrl.jszAct){ //接收爪 运行1/停止0
		cylinderState[2].eState = eCylinderOpen;
		if(1 == mCtrl.jszTurn) {  //正转1/反转2/无效0
			if(5 == cylinderState[2].uStateEx){
					N4_pos_cmd1 += 2000;
			}
		}else if(2 == mCtrl.jszTurn) {
			if(5 == cylinderState[2].uStateEx){
					N4_pos_cmd1 -= 2000;
			}
		}
	}else{
		cylinderState[2].eState = eCylinderClose;
		N4_pos_cmd1 = N4_position_actual_value_6064;
	}

	if((1 == mCtrl.toOrigin)&&(toOriginBak == 0)&&(branchState != eResetState)){  //按下去原点按钮
		branchState = eResetState;
		op->ydhcOP.speed = 2000;
		op->ydhcOP.ydhc_rst = 0;
		op->jszOP.jsz_rst = 0;
		op->sstdOP.sstd_rst = 0;
		origin_sigal_bak[0] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin18));
		origin_sigal_bak[1] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin20));	
		origin_sigal_bak[2] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
		for(no=0; no<SERVO_MAX; no++){ 								//使能电机2 3 4
			cylinderState[no].eState = eCylinderOpen;
			cylinderState[no].uStateEx = 0;
		}
	}
	toOriginBak = mCtrl.toOrigin;

	if((1 == mCtrl.toNext)&&(toNextBak == 0)&&(branchState != eActiveState)){    	//按下去下一步按钮
		branchState = eActiveState;
		op->jszOP.flag = 1;
		for(no=0; no<SERVO_MAX; no++){ 								//使能电机2 3 4
			cylinderState[no].eState = eCylinderOpen;
			cylinderState[no].uStateEx = 0;
		}
	}
	toNextBak = mCtrl.toNext;
}

/* 函数功能：第一层缓存处理逻辑 主控需要检测到蛋杯状态为flag=0才能控制电磁铁踢蛋到第一层蛋杯中
 * 返回值：0表示第一层缓存暂停状态 1表示第一层缓存运行状态
 */
void cache_floor1_process_flow(PushTrigSignal *signal, ChanbelOperateTypeDef *op, int ch)
{
		int pos;
	
		pos = eDCFout1 + ch;
		switch(op->jthcOP[pos].sta){
			case 0: 	/* 接收到主控发送的第一层某列满的脉冲信号 上升沿有效 */
				clrbitv(alarmMark.alarmCache, pos);
				if((2 == signal->pushTrig) && ((1 == signal->pushTrigBak)||(0 == signal->pushTrigBak))){
					op->jthcOP[pos].sta = 1;
					op->jthcOP[pos].flag = 1;
				}
				signal->pushTrigBak = signal->pushTrig;
				break;
			case 1:		/* 检测到第二层对应列蛋杯为空 打开气缸 */
				if(0 == op->jthcOP[eDCFout5+ch].flag){
					op->jthcOP[pos].sta = 2;
					op->jthcOP[pos].flag = 2;
					op->jthcOP[pos].count = 0;
					op->jthcOP[pos].countEx = 0;
					setbitv(op->dcfState, pos);
				}
				break;
			case 2:		/* 到达开启时间 关闭气缸 等待拉簧恢复 */
				if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin1+ch))){ /* 检测到气缸打开信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_OPEN_TIME){
						op->jthcOP[pos].sta = 3;
						op->jthcOP[pos].flag = 3;
						op->jthcOP[pos].count = 0;
						op->jthcOP[pos].countEx = 0;
						op->jthcOP[eDCFout5+ch].flag = 1;  					/* 已经缓存到了下一层 写入下一层蛋杯状态为满 */
						clrbitv(op->dcfState, pos);						
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						setbitv(alarmMark.alarmCache, pos);				/* 第一层缓存蛋杯打开故障 */
						rt_kprintf("F1 EO\n");
					}					
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_OPEN_TIME){
//					if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin1+ch))){ /* 检测到气缸打开信号 */
//						op->jthcOP[pos].sta = 3;
//						op->jthcOP[pos].flag = 3;
//						op->jthcOP[pos].count = 0;
//						op->jthcOP[eDCFout5+ch].flag = 1;  					/* 已经缓存到了下一层 写入下一层蛋杯状态为满 */
//						clrbitv(op->dcfState, pos);
//					}else{
//						if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){
//							op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//							setbitv(alarmMark.alarmCache, pos);				/* 第一层缓存蛋杯打开故障 */
//							rt_kprintf("F1 EO\n");
//						}
//					}
//				}
				break;
			case 3: 	/* 等待气缸复位信号 或者复位超时 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin1+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){
						op->jthcOP[pos].sta = 0;
						op->jthcOP[pos].flag = 0;
						op->jthcOP[pos].count = 0;		
						op->jthcOP[pos].countEx = 0;							
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){ 		/* 气缸复位超时 输出气缸开闭故障 */
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						op->jthcOP[pos].sta = 4;
						op->jthcOP[pos].flag = 4;
						setbitv(alarmMark.alarmCache, pos); 				/* 缓存蛋杯关闭启故障 */
						rt_kprintf("F1 EC\n");
					}
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){
//					if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin1+ch))){ /* 检测到气缸复位信号 */
//						op->jthcOP[pos].sta = 0;
//						op->jthcOP[pos].flag = 0;
//						op->jthcOP[pos].count = 0;
//					}
//				}else{
//					if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){ 		/* 气缸复位超时 输出气缸开闭故障 */
//						op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//						op->jthcOP[pos].sta = 4;
//						op->jthcOP[pos].flag = 4;
//						setbitv(alarmMark.alarmCache, pos); 				/* 缓存蛋杯关闭启故障 */
//						rt_kprintf("F1 EC\n");						
//					}
//				}
				break;
			case 4:		/* 进入气缸开闭故障状态 需要人工干预 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin1+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].sta = 0;
					op->jthcOP[pos].flag = 0;
					op->jthcOP[pos].count = 0;
					op->jthcOP[pos].countEx = 0;
				}
				break;
			default: break;
		}
}

/* 函数功能：第二层缓存处理逻辑 
 * 返回值：0表示第二层缓存暂停状态 1表示第二层缓存运行状态
 */
void cache_floor2_process_flow(ChanbelOperateTypeDef *op, int ch)
{
		int pos;
	
		pos = eDCFout5 + ch;
		switch(op->jthcOP[pos].sta){
			case 0:	/* 空闲状态 检测缓存中有无鸡蛋 */
				clrbitv(alarmMark.alarmCache, pos);
				op->jthcOP[pos].count = 0;
				op->jthcOP[pos].countEx = 0;
				if(1 == op->jthcOP[pos].flag){
					op->jthcOP[pos].sta = 1;
				}
				break;
			case 1: /* 检测到第三层对应列蛋杯为空 打开气缸 */
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_OPEN_TIME){   //鸡蛋落下来以后还要延时一下再放入下一层蛋杯
					if(0 == op->jthcOP[eDCFout9+ch].flag){
						op->jthcOP[pos].sta = 2;
						op->jthcOP[pos].flag = 2;
						op->jthcOP[pos].count = 0;
						op->jthcOP[pos].countEx = 0;
						setbitv(op->dcfState, pos);
					}
//					else{
//						op->jthcOP[pos].count = DCF_OPEN_TIME;
//					}
//				}
				break;
			case 2: /* 到达开启时间 关闭气缸 等待拉簧恢复 */
				if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin5+ch))){ /* 检测到气缸打开信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_OPEN_TIME){
						op->jthcOP[pos].sta = 3;
						op->jthcOP[pos].flag = 3;
						op->jthcOP[pos].count = 0;
						op->jthcOP[pos].countEx = 0;
						op->jthcOP[eDCFout9+ch].flag = 1;  		 		/* 已经缓存到了下一层 写入下一层蛋杯状态为满 */
						clrbitv(op->dcfState, pos);						
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						setbitv(alarmMark.alarmCache, pos); 		/* 第二层缓存蛋杯打开故障 */
						rt_kprintf("F2 EO\n");
					}					
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_OPEN_TIME){
//					if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin5+ch))){ /* 检测到气缸打开信号 */
//						op->jthcOP[pos].sta = 3;
//						op->jthcOP[pos].flag = 3;
//						op->jthcOP[pos].count = 0;
//						op->jthcOP[eDCFout9+ch].flag = 1;  		 		/* 已经缓存到了下一层 写入下一层蛋杯状态为满 */
//						clrbitv(op->dcfState, pos);
//					}else{
//						if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){
//							op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//							setbitv(alarmMark.alarmCache, pos); 		/* 第二层缓存蛋杯打开故障 */
//							rt_kprintf("F2 EO\n");
//						}
//					}
//				}
				break;			
			case 3: /* 等待气缸复位信号 或者复位超时 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin5+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){ 
						op->jthcOP[pos].sta = 0;
						op->jthcOP[pos].flag = 0;
						op->jthcOP[pos].count = 0;		
						op->jthcOP[pos].countEx = 0;						
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){ 	/* 气缸复位超时 输出气缸开闭故障 */
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						op->jthcOP[pos].sta = 4;
						op->jthcOP[pos].flag = 4;
						setbitv(alarmMark.alarmCache, pos);	 	/* 第二层缓存蛋杯关闭启故障 */
						rt_kprintf("F2 EC\n");
					}					
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){ 
//					if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin5+ch))){ /* 检测到气缸复位信号 */
//						op->jthcOP[pos].sta = 0;
//						op->jthcOP[pos].flag = 0;
//						op->jthcOP[pos].count = 0;	
//					}						
//				}else{
//					if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){ 	/* 气缸复位超时 输出气缸开闭故障 */
//						op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//						op->jthcOP[pos].sta = 4;
//						op->jthcOP[pos].flag = 4;
//						setbitv(alarmMark.alarmCache, pos);	 	/* 第二层缓存蛋杯关闭启故障 */
//						rt_kprintf("F2 EC\n");
//					}
//				}
				break;
			case 4:	/* 进入气缸开闭故障状态 需要人工干预 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin5+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].sta = 0;
					op->jthcOP[pos].flag = 0;
					op->jthcOP[pos].count = 0;
					op->jthcOP[pos].countEx = 0;
				}
				break;
			default: break;
		}
}

/* 函数功能：第三层缓存处理逻辑 
 * 返回值：0表示第三层缓存暂停状态 1表示第三层缓存运行状态
 */
void cache_floor3_process_flow(ChanbelOperateTypeDef *op, int ch)
{
		int pos;
	
		pos = eDCFout9 + ch;
		switch(op->jthcOP[pos].sta){
			case 0:	/* 空闲状态 检测缓存中有无鸡蛋 */
				clrbitv(alarmMark.alarmCache, pos);
				op->jthcOP[pos].count = 0;
				op->jthcOP[pos].countEx = 0;
				if(1 == op->jthcOP[pos].flag){
					op->jthcOP[pos].sta = 1;
				}
				break;
			case 1: /* 检测到移动缓存蛋杯为空 并且移动缓存停在对应列的位置 则打开气缸 */
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_OPEN_TIME){   //鸡蛋落下来以后还要延时一下再放入下一层蛋杯
					if((2 == op->ydhcOP.sta) && (0 == op->ydhcOP.flag) && (op->ydhcOP.mark == (ch+1))){
						op->jthcOP[pos].sta = 2;
						op->jthcOP[pos].flag = 2;
						op->jthcOP[pos].count = 0;
						op->jthcOP[pos].countEx = 0;
						setbitv(op->dcfState, pos);
					}
//					else{
//						op->jthcOP[pos].count = DCF_OPEN_TIME;
//					}
//				}
				break;
			case 2: /* 到达开启时间后 关闭气缸 等待拉簧恢复 */
				if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin9+ch))){ /* 检测到气缸打开信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_OPEN_TIME_EX1){
						op->jthcOP[pos].sta = 3;
						op->jthcOP[pos].flag = 3;
						op->jthcOP[pos].count = 0;
						op->jthcOP[pos].countEx = 0;
						op->ydhcOP.flag = 1;
						clrbitv(op->dcfState, pos);						
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						setbitv(alarmMark.alarmCache, pos); 			/* 第三层缓存蛋杯打开故障 */
						rt_kprintf("F3 EO\n");
					}
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_OPEN_TIME_EX1){
//					if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin9+ch))){ /* 检测到气缸打开信号 */
//						op->jthcOP[pos].sta = 3;
//						op->jthcOP[pos].flag = 3;
//						op->jthcOP[pos].count = 0;
//						op->ydhcOP.flag = 1;
//						clrbitv(op->dcfState, pos);
//					}else{
//						if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){
//							op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//							setbitv(alarmMark.alarmCache, pos); 			/* 第三层缓存蛋杯打开故障 */
//							rt_kprintf("F3 EO\n");
//						}
//					}
//				}
				break;		
			case 3: /* 等待气缸复位信号 或者复位超时 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin9+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].count++;
					if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){
						op->jthcOP[pos].sta = 0;
						op->jthcOP[pos].flag = 0;
						op->jthcOP[pos].count = 0;	
						op->jthcOP[pos].countEx = 0;						
					}
				}else{
					op->jthcOP[pos].countEx++;
					if(op->jthcOP[pos].countEx >= DCF_OP_TIMEOUT){ 		/* 气缸复位超时 输出气缸开闭故障 */
						op->jthcOP[pos].countEx = DCF_OP_TIMEOUT;
						op->jthcOP[pos].sta = 4;
						op->jthcOP[pos].flag = 4;
						setbitv(alarmMark.alarmCache, pos); /* 第三层缓存蛋杯关闭启故障 */
						rt_kprintf("F3 EC\n");
					}					
				}
//				op->jthcOP[pos].count++;
//				if(op->jthcOP[pos].count >= DCF_CLOSE_TIME){
//					if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin9+ch))){ /* 检测到气缸复位信号 */
//						op->jthcOP[pos].sta = 0;
//						op->jthcOP[pos].flag = 0;
//						op->jthcOP[pos].count = 0;	
//					}						
//				}else{
//					if(op->jthcOP[pos].count >= DCF_OP_TIMEOUT){ 		/* 气缸复位超时 输出气缸开闭故障 */
//						op->jthcOP[pos].sta = 4;
//						op->jthcOP[pos].count = DCF_OP_TIMEOUT;
//						op->jthcOP[pos].flag = 4;
//						setbitv(alarmMark.alarmCache, pos); /* 第三层缓存蛋杯关闭启故障 */
//						rt_kprintf("F3 EC\n");
//					}
//				}
				break;	
			case 4:	/* 进入气缸开闭故障状态 需要人工干预 */
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin9+ch))){ /* 检测到气缸复位信号 */
					op->jthcOP[pos].sta = 0;
					op->jthcOP[pos].flag = 0;
					op->jthcOP[pos].count = 0;
					op->jthcOP[pos].countEx = 0;
				}
				break;
			default: break;						
		}
}

// 初始化移动缓存电机控制参数
MotorParameters N3_motor = {
    .current_position = 0.0,
    .target_position 	= 0.0, 
    .max_velocity 		= 0.0,
    .max_acceleration = 0.0,
		.time_step				= 1,
		.sim_time					= 0,
//    .motion_completed = true
};

// 移动缓存电机规划参数结构体
TrapezoidalPlan N3_plan = {
		.t_acc 			= 0.0,
		.t_const		= 0.0,
		.t_dec 			= 0.0,
		.total_time = 0.0,
		.s_acc 			= 0.0,
		.s_const 		= 0.0,
		.s_dec 			= 0.0,
		.cruise_vel = 0.0
};

const int32_t orbit_postion_f[4] = {-400000, -133333, 133333, 400000};

/* 函数功能：移动缓存处理逻辑
 * 返回值：0表示移动缓存暂停状态 1表示移动缓存运行状态
 * +speed向第一列方向  -speed向第四列方向
 * 移动缓存伺服电机减速比1:5
 */
void move_cache_process_flow(ChanbelOperateTypeDef *op)
{
	int32_t distance = 0;
	
	switch(op->ydhcOP.sta)
	{
		case 0: /* 移动缓存空闲状态 选择移动到哪一列去接蛋 电机速度匹配等操作 */
			alarmMark.alarmMoveCupError = 0;
			op->ydhcOP.mark = calc_hurry_up_cache_floor(op); /* 1.优先选择鸡蛋缓存数量最多的通道 */
			if(op->ydhcOP.mark > 0) {			/* 2.根据是否需要接蛋来确定是否移动缓存 如果没有蛋需要接 就停在接受抓上方的位置不动 */
				egg_load_level = get_egg_load_level();
				if((convey_speed_flag == 2)&&(jsz_speed_flag == 2))//同步完成
				{
					if(egg_load_level >= egg_load_level_bak)
					{
						convey_speed_flag = 1;
						jsz_speed_flag = 0;
					}
					else
					{
						convey_speed_flag = 0;
						jsz_speed_flag = 1;
					}
					egg_load_level_next = egg_load_level;
				}
				
				op->ydhcOP.speed = YDHC_BASE_SPD * speed_ratio_table[egg_load_level];
				N3_motor.max_velocity = op->ydhcOP.speed;
				N3_motor.max_acceleration = op->ydhcOP.speed / accelertation_ratio[0];
				N3_motor.current_position = N3_position_actual_value_6064;
				if(TURN_FORWORD){  //根据要去哪个通道控制电机正反转
					N3_motor.target_position = N3_pos_cmd1 + orbit_postion_f[(op->ydhcOP.mark-1)%4];
				}else{
					N3_motor.target_position = N3_pos_cmd1 - orbit_postion_f[(op->ydhcOP.mark-1)%4];
				}			
				distance = N3_motor.target_position - N3_motor.current_position;
				N3_plan = trapezoidal_velocity_planning(distance, N3_motor.max_velocity, N3_motor.max_acceleration);	// 梯形速度曲线规划计算
//				printf("N3time_%lfx10ms %d\n", N3_plan.total_time, distance);  //打印执行动作需要的时间
				N3_motor.time_step = 1; // 10ms控制周期
				N3_motor.sim_time = 0;
				op->ydhcOP.sta = 1;
			}
			break;
		case 1: /* 控制移动缓存去目标位置 到目标位置后电机停止 */
			if(N3_motor.sim_time <= N3_plan.total_time + N3_motor.time_step) {
				MotionState state = get_trapezoidal_motion_state(N3_motor.sim_time, N3_plan,
																			N3_motor.current_position, N3_motor.target_position,
																			N3_motor.max_acceleration);
				N3_motor.sim_time += N3_motor.time_step;
				N3_pos_cmd1 = state.position;
			}else{
				op->ydhcOP.sta = 2;
			}
			break;
		case 2: /* 等待第三层缓存放蛋结束 */
			if(1 == op->ydhcOP.flag) {
				op->ydhcOP.sta = 3;
			}
			break;
		case 3: /* 控制移动缓存回到接受抓上方位置 */
			op->ydhcOP.speed = YDHC_BASE_SPD * speed_ratio_table[egg_load_level];
			N3_motor.max_velocity = op->ydhcOP.speed;
			N3_motor.max_acceleration = op->ydhcOP.speed / accelertation_ratio[0];
			if(op->ydhcOP.mark > 0) {			/* 2.根据是否需要接蛋来确定是否移动缓存 如果没有蛋需要接 就停在接受抓上方的位置不动 */
				N3_motor.current_position = N3_position_actual_value_6064;
				if(TURN_FORWORD){  //根据要去哪个通道控制电机正反转
					N3_motor.target_position = N3_pos_cmd1 - orbit_postion_f[(op->ydhcOP.mark-1)%4];
				}else{
					N3_motor.target_position = N3_pos_cmd1 + orbit_postion_f[(op->ydhcOP.mark-1)%4];
				}				
				distance = N3_motor.target_position - N3_motor.current_position;
				N3_plan = trapezoidal_velocity_planning(distance, N3_motor.max_velocity, N3_motor.max_acceleration);	// 梯形速度曲线规划计算
//				printf("N3time_%lfx10ms %d\n", N3_plan.total_time, distance);  //打印执行动作需要的时间
				N3_motor.time_step = 1; // 10ms控制周期
				N3_motor.sim_time = 0;
			}
			op->ydhcOP.sta = 4;
			break;
		case 4: /* 移动缓存回到接受抓上方后 进入等待将鸡蛋放入接受抓的状态 */
			if(N3_motor.sim_time <= N3_plan.total_time + N3_motor.time_step) {
				MotionState state = get_trapezoidal_motion_state(N3_motor.sim_time, N3_plan,
																			N3_motor.current_position, N3_motor.target_position,
																			N3_motor.max_acceleration);
				N3_motor.sim_time += N3_motor.time_step;
				N3_pos_cmd1 = state.position;
			}else{
				op->ydhcOP.speed = 0;
				op->ydhcOP.sta = 5;
				op->ydhcOP.mark = 0;
			}
			break;
		case 5: /* 检查接受抓状态 如果允许放入鸡蛋 则控制移动缓存气缸打开 */
			if(0 == op->jszOP.flag){
				op->ydhcOP.count = 0;
				op->ydhcOP.countEx = 0;
				op->ydhcOP.flag = 2;
				op->ydhcOP.sta = 6;
				setbitv(op->dcfState, eDCFout13);
			}
			break;
		case 6: /* 到达开启时间 关闭气缸 等待拉簧恢复 */
			if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin13))){ /* 检测到气缸打开信号 */
				op->ydhcOP.count++;
				if(op->ydhcOP.count >= DCF_OPEN_TIME_EX2){
					op->ydhcOP.sta = 7;
					op->ydhcOP.flag = 3;
					op->ydhcOP.count = 0;
					op->ydhcOP.countEx = 0;
					op->jszOP.flag = 1;    /* 接受抓满 */
					clrbitv(op->dcfState, eDCFout13);					
				}
			}else{
				op->ydhcOP.countEx++;
				if(op->ydhcOP.countEx >= DCF_OP_TIMEOUT){
					op->ydhcOP.countEx = DCF_OP_TIMEOUT;
					alarmMark.alarmMoveCupError = 1;	/* 移动缓存气缸打开故障 */
					rt_kprintf("M EO\n");
				}				
			}
//			op->ydhcOP.count++;
//			if(op->ydhcOP.count >= DCF_OPEN_TIME_EX2){
//				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin13))){ /* 检测到气缸打开信号 */
//					op->ydhcOP.sta = 7;
//					op->ydhcOP.flag = 3;
//					op->ydhcOP.count = 0;
//					op->jszOP.flag = 1;    /* 接受抓满 */
//					clrbitv(op->dcfState, eDCFout13);
//				}else{
//					if(op->ydhcOP.count >= DCF_OP_TIMEOUT){
//						op->ydhcOP.count = DCF_OP_TIMEOUT;
//						alarmMark.alarmMoveCupError = 1;	/* 移动缓存气缸打开故障 */
//						rt_kprintf("M EO\n");
//					}
//				}
//			}
			break;
		case 7: /* 等待气缸复位信号 或者复位超时 */
			if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin13))){ /* 检测到气缸复位信号 */
//				op->ydhcOP.count++;
//				if(op->ydhcOP.count >= DCF_CLOSE_TIME){
					op->ydhcOP.sta = 0;
					op->ydhcOP.count = 0;
					op->ydhcOP.countEx = 0;
					op->ydhcOP.flag = 0;					
//				}
			}else{
				op->ydhcOP.countEx++;
				if(op->ydhcOP.countEx >= DCF_OP_TIMEOUT){ /* 气缸复位超时 输出气缸开闭故障 */
					op->ydhcOP.countEx = DCF_OP_TIMEOUT;
					op->ydhcOP.sta = 8;
					op->ydhcOP.flag = 4;
					alarmMark.alarmMoveCupError = 1;	/* 移动缓存气缸打开故障 */
					rt_kprintf("M EC\n");
				}				
			}
//			op->ydhcOP.count++;
//			if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin13))){ /* 检测到气缸复位信号 */
//				op->ydhcOP.sta = 0;
//				op->ydhcOP.count = 0;
//				op->ydhcOP.flag = 0;			
//			}else{
//				if(op->ydhcOP.count >= DCF_OP_TIMEOUT){ /* 气缸复位超时 输出气缸开闭故障 */
//					op->ydhcOP.count = DCF_OP_TIMEOUT;
//					op->ydhcOP.sta = 8;
//					op->ydhcOP.flag = 4;
//					alarmMark.alarmMoveCupError = 1;	/* 移动缓存气缸打开故障 */
//					rt_kprintf("M EC\n");
//				}
//			}
			break;
		case 8:	/* 进入气缸开闭故障状态 需要人工干预 */
			if(0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin13))){ /* 检测到气缸复位信号 */
				op->ydhcOP.sta = 0;
				op->ydhcOP.count = 0;
				op->ydhcOP.countEx = 0;
				op->ydhcOP.flag = 0;
			}
			break;
		default: break;
	}
}

// 初始化接收抓电机控制参数
MotorParameters N4_motor = {
    .current_position = 0.0,
    .target_position 	= 0.0, 
    .max_velocity 		= 0.0,
    .max_acceleration = 0.0,
		.time_step				= 1,
		.sim_time					= 0,
    .motion_completed = 0
};

// 接收抓电机规划参数结构体
TrapezoidalPlan N4_plan = {
		.t_acc 			= 0.0,
		.t_const		= 0.0,
		.t_dec 			= 0.0,
		.total_time = 0.0,
		.s_acc 			= 0.0,
		.s_const 		= 0.0,
		.s_dec 			= 0.0,
		.cruise_vel = 0.0
};

/* 函数功能：接受抓处理流程
 * 返回值：0表示接受抓暂停状态 1表示接受抓运行状态
 * 接受抓伺服电机减速比1:10
 */
void accept_arrest_process_flow(ChanbelOperateTypeDef *op)
{
	int32_t distance = 0;
	
	switch(op->jszOP.sta){
		case 0: /* 空闲状态 检测接受抓里是否有鸡蛋 */
			alarmMark.alarmAcceptCupError = 0;
			switch(N4_motor.motion_completed)
			{
				case 0:
					if(1 == op->jszOP.flag){
						op->jszOP.sta = 1;
					}
					break;
				case 1:
					if(1 == getbitv(N4_statusword_6041, 10)){
						N4_modes_of_operation_6060 = 0x06;		//进入原点复归模式
						N4_control_word_6040 = 0x0F;
						N4_motor.motion_completed++;
					}
					break;
				case 2:
					N4_motor.motion_completed++;
					break;
				case 3:
					N4_control_word_6040 = 0x1F;						//正缘触发以当前位置为原点
					N4_pos_cmd1 = 0;
					N4_motor.motion_completed++;
					break;
				case 4:
					N4_motor.motion_completed++;
					break;
				case 5:
					N4_modes_of_operation_6060 = 0x07;			//恢复伺服为补件位置控制模式
				  N4_control_word_6040 = 0x0F;
					N4_motor.motion_completed = 0;
					break;
			}
			break;
		case 1: /* 等待输送蛋托为可以放入鸡蛋 */
			if(0 == op->sstdOP.flag){
				op->jszOP.sta = 2;
			}
			break;
		case 2: /* 控制电机旋转 匹配速度 将鸡蛋放入蛋托 */
			if(jsz_speed_flag == 1)
			{
				jsz_speed_flag = 2;
				op->jszOP.speed = JSZ_BASE_SPD * speed_ratio_table[egg_load_level_next];
			}
			else
			{
				if(convey_speed_flag == 2)
				{
					op->jszOP.speed = JSZ_BASE_SPD * speed_ratio_table[egg_load_level_next];
					jsz_speed_flag = 2;
					egg_load_level_bak = egg_load_level_next;
				}
				else
				{
					op->jszOP.speed = JSZ_BASE_SPD * speed_ratio_table[egg_load_level_bak];
				}
			}		
			op->jszOP.flag = 2;
			op->jszOP.sta = 3;
			N4_motor.current_position = N4_position_actual_value_6064;
			if(TURN_FORWORD) {
				N4_motor.target_position = N4_pos_cmd1 + 100000*10;
			} else {
				N4_motor.target_position = N4_pos_cmd1 - 100000*10;
			}
			N4_motor.max_velocity = op->jszOP.speed;
			N4_motor.max_acceleration = op->jszOP.speed / accelertation_ratio[1];
//			motor.motion_completed = true;

			distance = N4_motor.target_position - N4_motor.current_position;
			N4_plan = trapezoidal_velocity_planning(distance, N4_motor.max_velocity, N4_motor.max_acceleration);	// 梯形速度曲线规划计算
//			printf("N4time_%lfx10ms\n", N4_plan.total_time);  //打印执行动作需要的时间 0.72s
			N4_motor.time_step = 1; // 10ms控制周期
			N4_motor.sim_time = 0;
			break;
		case 3:	/* 控制电机再次旋转到原点位置 */
			if (N4_motor.sim_time <= N4_plan.total_time + N4_motor.time_step) 
			{
				MotionState state = get_trapezoidal_motion_state(N4_motor.sim_time, N4_plan,
																			N4_motor.current_position, N4_motor.target_position,
																			N4_motor.max_acceleration);
				N4_motor.sim_time += N4_motor.time_step;
				N4_pos_cmd1 = state.position;
				if(N4_motor.sim_time > N4_plan.total_time * 0.7)
				{
					op->sstdOP.flag = 1;
					op->jszOP.sta = 4;
				}
				else
				{
					op->jszOP.flag = 3;
				}
			} 
			break;
		case 4: /* 报警状态 需要人工干预 等待接受抓蛋杯闭合 且空蛋杯反射传感器为空 */
			if (N4_motor.sim_time <= N4_plan.total_time + N4_motor.time_step) 
			{
				MotionState state = get_trapezoidal_motion_state(N4_motor.sim_time, N4_plan,
																			N4_motor.current_position, N4_motor.target_position,
																			N4_motor.max_acceleration);
				N4_motor.sim_time += N4_motor.time_step;
				N4_pos_cmd1 = state.position;
			} 
			else 
			{ /* 检测到电机再次旋转到接近开关位置 */
				op->jszOP.speed = 0;
				if((1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin19))) && (0 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eFSdin1))))
				{
					op->jszOP.sta = 0;
					op->jszOP.flag = 0;
					if((N4_position_actual_value_6064 > clear_puu_throd) || (N4_position_actual_value_6064 < -clear_puu_throd))
					{
						N4_motor.motion_completed = 1;		//伺服向一个方向运行的计数脉冲PUU值大于20亿  需要对计数值清零
					}
				}
				else
				{
					op->jszOP.sta = 4;
					if(alarmMark.alarmAcceptCupError == 0)
					alarmMark.alarmAcceptCupError = 1;
				}
			}

			break;
		default: break;
	}
}

// 初始化输送通道电机控制参数
MotorParameters N2_motor = {
    .current_position = 0.0,
    .target_position 	= 0.0, 
    .max_velocity 		= 0.0,
    .max_acceleration = 0.0,
		.time_step				= 1,
		.sim_time					= 0,
    .motion_completed = 0,
		.package_postion = {123982, 270760, 270692, 270545, 273045, 0},
		.row = 5,
		.chainLinkCnt = 0,
};

// 输送通道电机规划参数结构体
TrapezoidalPlan N2_plan = {
		.t_acc 			= 0.0,
		.t_const		= 0.0,
		.t_dec 			= 0.0,
		.total_time = 0.0,
		.s_acc 			= 0.0,
		.s_const 		= 0.0,
		.s_dec 			= 0.0,
		.cruise_vel = 0.0
};

/*
原点  -2871199

一排  -2925181  -53982

二排  -3174941  -249760

三排  -3445633  -270692

四排  -3711178  -265545

五排  -3984223  -273045

*/
//const int32_t package_postion[6] = {123982, 270760, 270692, 270545, 273045, 0};
int8_t package_index = 0;
/* 函数功能：输送通道处理流程
 * 返回值：0表示输送通道暂停状态 1表示输送通道运行状态
 * 输送通道伺服电机减速比1:10
 */
uint8_t convey_channel_process_flow(ChanbelOperateTypeDef *op)
{
	uint8_t res = 0, i;
	int32_t distance = 0;
	int32_t t_target_position = 0;

	switch(op->sstdOP.sta){
		case 0:	/* 空闲状态 检测是否放入鸡蛋到托盘中 */
			switch(N2_motor.motion_completed)
			{
				case 0:
					if(1 == op->sstdOP.flag){
						if(1 == op->sstdOP.sstd_rst){ // 按下复位按钮后 第一排需要移动的位置
							op->sstdOP.sstd_rst = 0;
							t_target_position = N2_motor.package_postion[0];
							N2_motor.max_velocity = 15000;
							N2_motor.max_acceleration = 1000;
							package_index = 0;
						}else{
							package_index = (package_index + 1) % N2_motor.row;
							if(0 == package_index){     //转一圈换一个容器的信号
//								t_target_position = 2000000 - N2_motor.package_postion[0] - N2_motor.package_postion[1] - N2_motor.package_postion[2] - N2_motor.package_postion[3] - N2_motor.package_postion[4] + N2_motor.package_postion[0];
								t_target_position = 2000000 + N2_motor.chainLinkCnt*1000000;   //计算换一个容器的伺服电机位置
								for(i=0; i<N2_motor.row; i++){
									t_target_position -= N2_motor.package_postion[i];
								}
								t_target_position += N2_motor.package_postion[0];
								if(convey_speed_flag == 1)
								{
									convey_speed_flag = 2;
									N2_motor.max_velocity = CONVEY_LOOP_BASE_SPD * speed_ratio_table[egg_load_level_next];
								}
								else
								{
									if(jsz_speed_flag == 2)
									{
										N2_motor.max_velocity = CONVEY_LOOP_BASE_SPD * speed_ratio_table[egg_load_level_next];
										convey_speed_flag = 2;
										egg_load_level_bak = egg_load_level_next;
									}
									else
									{
										N2_motor.max_velocity = CONVEY_LOOP_BASE_SPD * speed_ratio_table[egg_load_level_bak];
									}
								}
								
								N2_motor.max_acceleration = N2_motor.max_velocity / accelertation_ratio[2];//原来1000

							}else{											//前进一个蛋格
								t_target_position = N2_motor.package_postion[package_index];
								if(convey_speed_flag == 1)
								{
									convey_speed_flag = 2;
									N2_motor.max_velocity = CONVEY_NORMAL_BASE_SPD * speed_ratio_table[egg_load_level_next];
								}
								else
								{
									if(jsz_speed_flag == 2)
									{
										N2_motor.max_velocity = CONVEY_NORMAL_BASE_SPD * speed_ratio_table[egg_load_level_next];
										convey_speed_flag = 2;
										egg_load_level_bak = egg_load_level_next;
									}
									else
									{
										N2_motor.max_velocity = CONVEY_NORMAL_BASE_SPD * speed_ratio_table[egg_load_level_bak];
									}
								}								
								N2_motor.max_acceleration = N2_motor.max_velocity / accelertation_ratio[3];
							}
						}

						N2_motor.current_position = N2_position_actual_value_6064;
						if(TURN_FORWORD){
							N2_motor.target_position = N2_pos_cmd1 - t_target_position;
						}else{
							N2_motor.target_position = N2_pos_cmd1 + t_target_position;
						}
					
						distance = N2_motor.target_position - N2_motor.current_position;
						N2_plan = trapezoidal_velocity_planning(distance, N2_motor.max_velocity, N2_motor.max_acceleration);	// 梯形速度曲线规划计算
//						printf("time_%lfx10ms\n", N2_plan.total_time);  //打印执行动作需要的时间 0.72s
						N2_motor.time_step = 1; // 10ms控制周期
						N2_motor.sim_time = 0;
						op->sstdOP.sta = 1;  //移动一个蛋格
					}
					break;
				case 1:
					if(1 == getbitv(N2_statusword_6041, 10)){
						N2_modes_of_operation_6060 = 0x06;		//进入原点复归模式
						N2_control_word_6040 = 0x0F;
						N2_motor.motion_completed++;
					}
					break;
				case 2:
					N2_motor.motion_completed++;
					break;
				case 3:
					N2_control_word_6040 = 0x1F;						//正缘触发以当前位置为原点
					N2_pos_cmd1 = 0;
					N2_motor.motion_completed++;
					break;
				case 4:
					N2_motor.motion_completed++;
					break;
				case 5:
					N2_modes_of_operation_6060 = 0x07;			//恢复伺服为补件位置控制模式
				  N2_control_word_6040 = 0x0F;
					N2_motor.motion_completed = 0;
					break;
				}
			break;
		case 1: /* 控制电机移动到下一个蛋格 */
			if(N2_motor.sim_time <= N2_plan.total_time + N2_motor.time_step){
				MotionState state = get_trapezoidal_motion_state(N2_motor.sim_time, N2_plan,
																			N2_motor.current_position, N2_motor.target_position,
																			N2_motor.max_acceleration);
				N2_motor.sim_time += N2_motor.time_step;
				N2_pos_cmd1 = state.position;
				if(N2_motor.sim_time > N2_plan.total_time * 0.7){
					op->sstdOP.flag = 0;
				}else{
					op->sstdOP.flag = 2;
				}
			}else{
				op->sstdOP.sta = 0;
				op->sstdOP.flag = 0;
				if((N2_position_actual_value_6064 > clear_puu_throd) || (N2_position_actual_value_6064 < -clear_puu_throd)){
					N2_motor.motion_completed = 1;		//伺服向一个方向运行的计数脉冲PUU值大于20亿  需要对计数值清零
				}
			}
			break;
		case 2: /* 输送通道故障 通道堆积 蛋托缺失 及其它故障*/
			res = 0;
			break;
		default: break;
	}
	return res;
}

// 复位模式 控制三台伺服电机复位
uint8_t channel_reset_operate(ChanbelOperateTypeDef *op)
{
	uint8_t res = 0, origin_sigal = 0;

	//控制移动缓存复位 回到接受抓上方的位置
	origin_sigal = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin18));
	if((0 == origin_sigal_bak[0])&&(1 == origin_sigal)){  //按下复位按钮 检查复位信号上升沿
		op->ydhcOP.sta = 0;
		op->ydhcOP.count = 0;
		op->ydhcOP.flag = 0;
		op->ydhcOP.mark = 0;
		op->ydhcOP.speed = 0;
		op->ydhcOP.ydhc_rst = 1;
		clrbitv(op->dcfState, eDCFout13);  //关闭蛋夹
	}else{
		if(0 == op->ydhcOP.ydhc_rst){

			setbitv(op->dcfState, eDCFout13); //打开蛋夹

			if(TURN_FORWORD){
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eXCdin2))){
					op->ydhcOP.speed = 2000;
				}else if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eXCdin1))){
					op->ydhcOP.speed = -2000;
				}
				N3_pos_cmd1 += op->ydhcOP.speed;
			}else{
				if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eXCdin2))){
					op->ydhcOP.speed = -2000;
				}else if(1 == get_input_port_sta(op->inputData, (enum EnumInputPort)(eXCdin1))){
				op->ydhcOP.speed = 2000;
				}
				N3_pos_cmd1 -= op->ydhcOP.speed;
			}
			res += 1;
		}
	}
	origin_sigal_bak[0] = origin_sigal;
	
	//控制接受抓电机复位 检测到电机旋转到接近开关位置
	origin_sigal = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin20));	
	if((0 == origin_sigal_bak[1])&&(1 == origin_sigal)){ //按下复位按钮 检查复位信号上升沿
		op->jszOP.sta = 0;
		op->jszOP.speed = 0;
		op->jszOP.jsz_rst = 1;
	}else{
		if(0 == op->jszOP.jsz_rst){
			if(TURN_FORWORD){
				N4_pos_cmd1 += 2000;
			}else{
				N4_pos_cmd1 -= 2000;
			}
			res += 1;
		}
	}
	origin_sigal_bak[1] = origin_sigal;
	
	//控制输送通道电机复位
	origin_sigal = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
	if((0 == origin_sigal_bak[2])&&(1 == origin_sigal)){
		op->sstdOP.sta = 0;
		op->sstdOP.type = 0;
		op->sstdOP.flag = 1;
		op->sstdOP.sstd_rst = 1;
	}else{
		if(0 == op->sstdOP.sstd_rst){
			if(TURN_FORWORD){
				N2_pos_cmd1 -= 6000; 
			}else{
				N2_pos_cmd1 += 6000; 
			}
			res += 1;
		}
	}
	origin_sigal_bak[2] = origin_sigal;
	
	return res;
}

//通道单次动作控制流程
void channel_active_operate(ChanbelOperateTypeDef *op)
{
	/* 通道单次动作：
		1.输送蛋托向前移动一格
		2.接收抓向下放一排蛋
	*/
	
	accept_arrest_process_flow(op);
	
	convey_channel_process_flow(op);
}

int channel_clear_operate(ChanbelOperateTypeDef *op)
{
	/* 通道清空动作：
	 1.第一层缓存中如果有蛋 将第一层缓存设定为满蛋状态 在按下按钮的时候已经完成了这个操作
	 2.控制通道进入运行状态
	 3.等待所有缓存为空 移动缓存为空 接收抓为空 输送蛋托为空 然后完成通道清空
	 4.通知主控程序通道已经被清空
	*/
	int ch = 0, res = 0;
		
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		cache_floor1_process_flow(&tirgSignal[ch], op, ch);
		if(0 != op->jthcOP[eDCFout1+ch].flag) res += 1;
	}
	
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		cache_floor2_process_flow(op, ch);
		if(0 != op->jthcOP[eDCFout5+ch].flag) res += 1;
	}
	
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		cache_floor3_process_flow(op, ch);
		if(0 != op->jthcOP[eDCFout9+ch].flag) res += 1;
	}
	
	move_cache_process_flow(op);
	if(0 != op->ydhcOP.flag) res += 1;
	
	accept_arrest_process_flow(op);
	if(0 != op->jszOP.flag) res += 1;
	
	convey_channel_process_flow(op);
	if(0 != op->sstdOP.flag) res += 1;
	
	return res;
}

// 包装通道运行状态下程序处理流程
int caches_run_process_flow(ChanbelOperateTypeDef *op, enum EnumMainStateManchine stateNext)
{
	int ch = 0, res = 0;
	uint8_t state;
	
	/* 第1层阶梯缓存控制逻辑 */
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		if(stateNext == eStopState){ 	//按下暂停键
			if((0 == op->jthcOP[eDCFout1+ch].sta)||(1 == op->jthcOP[eDCFout1+ch].sta)||(4 == op->jthcOP[eDCFout1+ch].sta)){ 	//气缸没有在开闭过程中 则暂停这一层缓存运行
				continue;
			}else{
				res += 1;
			}
		}
		cache_floor1_process_flow(&tirgSignal[ch], op, ch);
	}
	
	/* 第2层阶梯缓存控制逻辑 */
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		if(stateNext == eStopState){ 	//按下暂停键
			if((0 == op->jthcOP[eDCFout5+ch].sta)||(1 == op->jthcOP[eDCFout5+ch].sta)||(4 == op->jthcOP[eDCFout5+ch].sta)){ 	//气缸没有在开闭过程中 则暂停这一层缓存运行
				continue;
			}else{
				res += 1;
			}
		}
		cache_floor2_process_flow(op, ch);
	}
	
	/* 第3层阶梯缓存控制逻辑 */
	for(ch=0; ch<CACHE_CHANNEL; ch++)
	{
		if(stateNext == eStopState){ //按下暂停键
			if((0 == op->jthcOP[eDCFout9+ch].sta)||(1 == op->jthcOP[eDCFout9+ch].sta)||(4 == op->jthcOP[eDCFout9+ch].sta)){ //气缸没有在开闭过程中 则暂停这一层缓存运行
				continue;
			}else{
				res += 1;
			}
		}
		cache_floor3_process_flow(op, ch);
	}
	
	/* 移动缓存控制逻辑 */
	if(stateNext == eStopState){ //按下暂停键
		if((0 == op->ydhcOP.sta)||(2 == op->ydhcOP.sta)||(5 == op->ydhcOP.sta)||(8 == op->ydhcOP.sta)){ 		//移动缓存可以停在接受抓上方位置 或者故障状态 则暂停这一层缓存运行
			//rt_kprintf("move_cache_stop\n");
		}else{
			res += 1;
			move_cache_process_flow(op);
		}
	}else{
		move_cache_process_flow(op);
	}

	/* 接受抓控制逻辑 */
	if(stateNext == eStopState){ //按下暂停键
		if((0 == op->jszOP.sta) || (1 == op->jszOP.sta) ||(4 == op->jszOP.sta)){ //空闲状态、等待输送蛋托状态时可以停机 暂停运行
			//rt_kprintf("accept_arrest_stop\n");
		}else{
			res += 1;
			accept_arrest_process_flow(op);
		}
	}else{
		accept_arrest_process_flow(op);
	}
	
	/* 输送通道控制逻辑 */
	if(stateNext == eStopState){ 		//按下暂停键
		if(0 == op->sstdOP.sta){ 					//暂停运行
			//rt_kprintf("convey_channel_stop\n");
		}else{
			res += 1;
			convey_channel_process_flow(op);
		}
	}else{
		convey_channel_process_flow(op);
	}
	
	setbitv(cop.dcfState, eXPSSout1); 									//机器运行则开启斜坡输送电机  增加故障报警状态也要停止网链电机
	
	state = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
	if((1 == state)&&(0 == autoTray_sigal_bak)){				//自动发托信号触发
		op->sstdOP.autoTrayFlag = 1;
	}
	autoTray_sigal_bak = state;
	
	return res;
}

void caches_branch_process_flow(ChanbelOperateTypeDef *op)
{
	uint8_t rest = 0, no, state = 0;
	uint16_t *ptr;
	
	switch(branchState)
	{
		/* 通道空闲状态 */
		case eBranchIdleState:
			if(op->keyBtn == keyResetBtn){
				branchState = eResetState;
				op->ydhcOP.speed = 2000;
				op->ydhcOP.ydhc_rst = 0;
				op->jszOP.jsz_rst = 0;
				op->sstdOP.sstd_rst = 0;
				op->sstdOP.autoTrayFlag = 0;
				op->sstdOP.autoTrayEnable = 1;
				autoTray_sigal_bak = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
				origin_sigal_bak[0] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin18));
				origin_sigal_bak[1] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin20));	
				origin_sigal_bak[2] = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
			}else if(op->keyBtn == keyActiveBtn){
				branchState = eActiveState;
				op->jszOP.flag = 1;
			}else if(op->keyBtn == keyClearBtn){
				if(tirgSignal[0].pushTrig > 0){   //如果顶层缓存有鸡蛋 则清空缓存中的鸡蛋
					op->jthcOP[eDCFout1].sta = 1;
					op->jthcOP[eDCFout1].flag = 1;
					tirgSignal[0].pushTrig = 0;
				}
				if(tirgSignal[1].pushTrig > 0){
					op->jthcOP[eDCFout2].sta = 1;
					op->jthcOP[eDCFout2].flag = 1;
					tirgSignal[1].pushTrig = 0;
				}
				if(tirgSignal[2].pushTrig > 0){
					op->jthcOP[eDCFout3].sta = 1;
					op->jthcOP[eDCFout3].flag = 1;
					tirgSignal[2].pushTrig = 0;					
				}
				if(tirgSignal[3].pushTrig > 0){
					op->jthcOP[eDCFout4].sta = 1;
					op->jthcOP[eDCFout4].flag = 1;
					tirgSignal[3].pushTrig = 0;
				}
				branchState = eClearState;
				channelClearFlag = 1; 							//通知主控程序这边已经对通道清空操作
				rt_kprintf("1 clr ph\n");
			}else if(op->keyBtn == keyClearAllBtn){ //强制清空所有缓存
				for(no=0; no<CACHE_CHANNEL; no++){
					op->jthcOP[eDCFout1+no].sta = 1;
					op->jthcOP[eDCFout1+no].flag = 1;
					op->jthcOP[eDCFout5+no].sta = 1;
					op->jthcOP[eDCFout5+no].flag = 1;
					op->jthcOP[eDCFout9+no].sta = 1;
					op->jthcOP[eDCFout9+no].flag = 1;					
				}
				branchState = eClearAllState;
				channelClearFlag = 1;				 				//通知主控程序这边已经对通道清空操作
			}
			if((op->keyBtn == keyResetBtn)||(op->keyBtn == keyActiveBtn)||(op->keyBtn == keyClearBtn)){
				for(no=0; no<SERVO_MAX; no++){ 			//使能电机2 3 4
					cylinderState[no].eState = eCylinderOpen;
					cylinderState[no].uStateEx = 0;
				}
				op->keyBtn = keyUnknowBtn;  				//清除按键值
			}
			break;
		/* 通道复位	控制伺服电机复位 */
		case eResetState:
			clear_alarm_mark();
			rest = channel_reset_operate(op);
			ptr = (uint16_t *)&alarmMark;
			if((*ptr > 0)||(*(ptr+1) > 0)){				//故障报警 切换回空闲状态
				branchState = eBranchIdleState;
			}
			if((0 == rest)||(op->keyBtn == keyResetBtn)) {	//三台伺服电机复位完成或者再次按下复位键 进入暂停状态
				branchState = eBranchIdleState; 		//主状态修改为暂停中
			}
			op->keyBtn = keyUnknowBtn;  					//执行命令过程中 清除按键值
			break;
		/* 通道动作  控制接受抓与蛋托输送电机动作 按一次完成一次放蛋动作 */
		case eActiveState:
			clear_alarm_mark();
			channel_active_operate(op);
			ptr = (uint16_t *)&alarmMark;
			if((*ptr > 0)||(*(ptr+1) > 0)){				//故障报警 切换回空闲状态
				branchState = eBranchIdleState;
			}
			if((0 == op->jszOP.flag)&&(0 == op->sstdOP.sta)){
				branchState = eBranchIdleState; 		//主状态修改为暂停中
			}
			op->keyBtn = keyUnknowBtn;  					//执行命令过程中 清除按键值
			break;
		/* 通道清空  清空缓存中的所有鸡蛋 全部装入蛋托 */
		case eClearState:
			clear_alarm_mark();
			rest = channel_clear_operate(op); 		//需要配合主控统计到那一列有蛋 然后对那一列做清空操作
			ptr = (uint16_t *)&alarmMark;
			if((*ptr > 0)||(*(ptr+1) > 0)){				//故障报警 切换回空闲状态
				branchState = eBranchIdleState;
			}
			if(0 == rest){
				branchState = eBranchIdleState; 		//主状态修改为暂停中
				channelClearFlag = 0;
			}
			op->keyBtn = keyUnknowBtn;  					//执行命令过程中 清除按键值
			break;
		case eClearAllState:
			clear_alarm_mark();
			rest = channel_clear_operate(op); 		//强制清空所有缓存中的鸡蛋
			ptr = (uint16_t *)&alarmMark;
			if((*ptr > 0)||(*(ptr+1) > 0)){				//故障报警 切换回空闲状态
				branchState = eBranchIdleState;
			}
			if(0 == rest){
				branchState = eBranchIdleState; 		//主状态修改为暂停中
				channelClearFlag = 0;
			}
			ClearAllFlag = 0;
			op->keyBtn = keyUnknowBtn;  					//执行命令过程中 清除按键值
			break;
		default: break;
	}
	
	clrbitv(cop.dcfState, eXPSSout1); 				//机器停止则斜坡输送电机
	
	state = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
	if((1 == state)&&(0 == autoTray_sigal_bak)){				//自动发托信号触发
		op->sstdOP.autoTrayFlag = 1;
	}
	autoTray_sigal_bak = state;
}

// 主流程
void caches_process_flow(ChanbelOperateTypeDef *op)
{
	uint8_t rest = 0, no, startupFlag = 0;
	uint16_t *ptr;
		
	/* 三台伺服电机通信故障状态获取 */
	alarmMark.alarmMoveServoConenct = get_servo_state(0);
	alarmMark.alarmAcceptServoConnect = get_servo_state(1);
	alarmMark.alarmConveyServoConnect = get_servo_state(2);
	
	/* 三台伺服电机驱动器故障状态获取 */
	alarmMark.alarmMoveServodevice = getbitv(N2_statusword_6041, 3);
	alarmMark.alarmAcceptServoDevice = getbitv(N3_statusword_6041, 3);
	alarmMark.alarmConveyServoDevice = getbitv(N4_statusword_6041, 3);

	/* 几种操作模式 */
	switch(mainState)
	{
		case eIdleState:		/* 1.空闲状态 */
			system_param_init(op);  														//初始化参数
			mainState = eStopState;
			break;
		case eRunState:			/* 2.运行状态 */
			rest = caches_run_process_flow(op, mainStateNext);
			if(mainStateNext == eStopState){
				if(0 == rest){  																	//按下暂停并且所有缓存器都已暂停
					mainState = mainStateNext; 											//主状态修改为暂停中
					for(no=0; no<SERVO_MAX; no++){									//关闭电机2 3 4
						cylinderState[no].eState = eCylinderClose;
					}
				}else{
					ptr = (uint16_t *)&alarmMark;
					if((*ptr > 0)||(*(ptr+1) > 0)){
						clrbitv(cop.dcfState, eXPSSout1); 						//增加故障报警状态也要停止坡输送电机
					}
				}
			}
			op->keyBtn = keyUnknowBtn;  												//清除按键值 运行状态下 所有通道单独控制的按钮失效
			break;
		case eStopState:		/* 3.暂停状态 */
//			rt_kprintf("eStopState\n");
			caches_branch_process_flow(op);											//暂停状态下 执行包装通道上的按钮控制动作

			if((mainStateNext == eRunState)&&(branchState == eBranchIdleState)){
				startupFlag = 1;
				for(no=0; no<SERVO_MAX; no++){										//使能电机2 3 4
					cylinderState[no].eState = eCylinderOpen;
					if(5 != cylinderState[no].uStateEx) {
						startupFlag = 0;
					}
				}
				if(startupFlag) {                                 //所有伺服都启动完成 系统进入运行状态
					mainState = mainStateNext;
				}
			}
			if((mainStateNext == eControlState)||(mainStateNext == eEditState)){ //切换到ethercat主机手动控制状态 或者新建包装盒控制状态
				mainState = mainStateNext;
			}
			break;
		case eControlState:	/* 4.手动控制状态 */
//			rt_kprintf("eControlState\n");
			if(mainStateNext == eStopState){
				mainState = mainStateNext;
				autoTray_sigal_bak = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21)); //手动控制不使用发托信号
			}
			package_channel_manual_control_func(op);
			break;
		case eEditState:		/*新建包装控制状态 */
			rt_kprintf("eEditState\n");
			if(mainStateNext == eStopState){
				mainState = mainStateNext;
				autoTray_sigal_bak = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21)); //手动控制不使用发托信号
			}
			package_channel_new_edit_func(op);
			break;
		case eUnknowState:	/* 6.未知状态 */
			break;
		default: break;
	}

	updateDcfIO(op->dcfState);
}

void master_dynamic_sem_release(void)
{
	if(1 == sem_flag){
		rt_sem_release(dynamic_sem);
	}
}

/* 根据伺服电机反馈位置和零点位置计算电机相对位置
 * 模运算自动处理数据移除 相对位移 = (当前位置-前次位置+模数/2)%模数-模数/2
 * 模数 = 4294967296; 
 */
void servo_postion_feedback(ChanbelOperateTypeDef *op)
{
	uint8_t sta = 0;

	/* 输送通道伺服电机位置记录 */
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin21));
	if((1 == sta)&&(0 == sstd_servo.staBak)){ //输送通道伺服电机原点信号 记录伺服当前位置
		sstd_servo.zero = N2_position_actual_value_6064;
	}
	sstd_servo.distance = (N2_position_actual_value_6064 - sstd_servo.zero + 2147483648) % 4294967296 - 2147483648;
	sstd_servo.staBak = sta;

	/* 接受抓伺服电机位置记录 */
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin20));
	if((1 == sta)&&(0 == jsz_servo.staBak)){ //接受抓伺服电机原点信号 记录伺服当前位置
		jsz_servo.zero = N4_position_actual_value_6064;
	}
	jsz_servo.distance = (N4_position_actual_value_6064 - jsz_servo.zero + 2147483648) % 4294967296 - 2147483648;
	jsz_servo.staBak = sta;

	/* 移动缓存伺服电机位置记录 */
/*	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin18));
	if((1 == sta)&&(0 == ydhc_servo.staBak)){ //移动缓存接受抓上方信号 记录伺服当前位置
		ydhc_servo.zero = N3_position_actual_value_6064;
	}
	ydhc_servo.staBak = sta;
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin14));
	if((1 == sta)&&(0 == ydhc_servo.staBak1)){ //移动缓存接蛋位置1信号 记录伺服当前位置
		ydhc_servo.distance1 = (N3_position_actual_value_6064 - ydhc_servo.zero + 2147483648) % 4294967296 - 2147483648;
	}
	ydhc_servo.staBak1 = sta;
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin15));
	if((1 == sta)&&(0 == ydhc_servo.staBak2)){ //移动缓存接蛋位置2信号 记录伺服当前位置
		ydhc_servo.distance2 = (N3_position_actual_value_6064 - ydhc_servo.zero + 2147483648) % 4294967296 - 2147483648;
	}
	ydhc_servo.staBak2 = sta;
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin16));
	if((1 == sta)&&(0 == ydhc_servo.staBak3)){ //移动缓存接蛋位置3信号 记录伺服当前位置
		ydhc_servo.distance3 = (N3_position_actual_value_6064 - ydhc_servo.zero + 2147483648) % 4294967296 - 2147483648;
	}
	ydhc_servo.staBak3 = sta;
	sta = get_input_port_sta(op->inputData, (enum EnumInputPort)(eJJdin17));
	if((1 == sta)&&(0 == ydhc_servo.staBak4)){ //移动缓存接蛋位置4信号 记录伺服当前位置
		ydhc_servo.distance4 = (N3_position_actual_value_6064 - ydhc_servo.zero + 2147483648) % 4294967296 - 2147483648;
	}
	ydhc_servo.staBak4 = sta;
*/
}

void global_variable_init(void)
{
	rt_memset(&cop, 0, sizeof(ChanbelOperateTypeDef));
	
	rt_sem_control(dynamic_sem, RT_IPC_CMD_RESET, RT_NULL);
	
	sem_flag = 1;
}

static void rt_master_thread_entry(void *parameter)
{
	static uint8_t sta_cnt = 0, btn_cnt = 0;
	static uint16_t  wait_time = 10000;
	
	hc165ConfigIO();

	configDcfIO();
	
	mainState = eIdleState;  				//整机运行状态机
	
	branchState = eBranchIdleState; //包装通道状态机
	
	channel_button_init();					//按钮初始化
	
	rt_thread_mdelay(2000);					//等待私服驱动器启动
	
	canopen_init();									//初始化CANOPEN协议

	global_variable_init();

	while(RT_TRUE)
	{
//		rt_thread_mdelay(10);
			if(rt_sem_take(dynamic_sem, wait_time) == RT_EOK)
			{
					// 1.读取6片移位寄存器芯片输入IO的状态
					hc165ReadData(cop.inputData, 6);
				
					// 2.每30ms周期调用按钮处理函数 检测通道按钮状态并处理按键值
					btn_cnt = (btn_cnt+1)%3;
					if(2 == btn_cnt) Button_Process();
				
					// 3.执行主控制流程
					caches_process_flow(&cop);
				
					// 4.反馈电机实时位置
					servo_postion_feedback(&cop);
				
					sta_cnt = (sta_cnt + 1) % 40;
					if(sta_cnt < 20) rt_pin_write(LEN_RUN, PIN_HIGH);
					else rt_pin_write(LEN_RUN, PIN_LOW);
				
					wait_time = 1000;
			}else{
					/* 三台伺服电机通信故障状态获取 */
				alarmMark.alarmMoveServoConenct = get_servo_state(0);
				alarmMark.alarmAcceptServoConnect = get_servo_state(1);
				alarmMark.alarmConveyServoConnect = get_servo_state(2);	
				clrbitv(cop.dcfState, eXPSSout1);		//关闭斜坡输送
				clrbitv(cop.dcfState, eFTDJout2);   //关闭自动发托电机
			}
	}
}

int master_pthread_init(void)
{
		rt_pin_mode(LEN_RUN, PIN_MODE_OUTPUT);
		rt_pin_write(LEN_RUN, PIN_HIGH);

		dynamic_sem = rt_sem_create("dsem", 0, RT_IPC_FLAG_PRIO);
    if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }

		rt_thread_init(&master_thread,
                   "master",
                   rt_master_thread_entry,
                   RT_NULL,
                   &master_thread_stack[0],
                   sizeof(master_thread_stack),
                   12, 10);
    rt_thread_startup(&master_thread);

		return RT_EOK;
}

INIT_APP_EXPORT(master_pthread_init);

static void cmd_read_input(void)
{
	rt_kprintf("input IO:\n");
	for(int i=0; i<6; i++){
		rt_kprintf("%08b ", cop.inputData[i]);
	}
	rt_kprintf("\n");
}
MSH_CMD_EXPORT_ALIAS(cmd_read_input, read_input_io, print states of input);

static int32_t dcfState = 0;
static void cmd_dcf_ctrl(int argc, char **argv)
{
		int32_t port, sta;
	
		if(argc > 1) {
			port = atol(argv[1]);
		}
    if(argc > 2) {
			sta = atol(argv[2]);
    }
		
		if(0 == sta) {
			clrbitv(dcfState, port);
		}else{
			setbitv(dcfState, port);
		}
		
		updateDcfIO(dcfState);
}
MSH_CMD_EXPORT_ALIAS(cmd_dcf_ctrl, write_dcf_io, ctrl states of dcf);

static int32_t dctState = 0;
static void cmd_dct_ctrl(int argc, char **argv)
{
	int32_t port, sta;
	
	if(argc > 1) {
		port = atol(argv[1]);
	}
  if(argc > 2) {
		sta = atol(argv[2]);
  }	
	
	if(0 == sta) {
		clrbitv(dctState, port);
	}else{
		setbitv(dctState, port);
	}
		
	updateDctIO(dctState);
}
MSH_CMD_EXPORT_ALIAS(cmd_dct_ctrl, write_dct_io, ctrl states of dct);

static int8_t rlyState = 0;
static void cmd_rly_ctrl(int argc, char **argv)
{
	int32_t port, sta;
	
	if(argc > 1) {
		port = atol(argv[1]);
	}
  if(argc > 2) {
		sta = atol(argv[2]);
  }	
	
	if(0 == sta) {
		clrbitv(rlyState, port);
	}else{
		setbitv(rlyState, port);
	}
		
	updateRelayIO(rlyState);
}
MSH_CMD_EXPORT_ALIAS(cmd_rly_ctrl, write_rly_io, ctrl states of rly);

static void cmd_trig_ctrl(int argc, char **argv)
{
	int32_t port, sta;
	
	if(argc > 1) {
		port = atol(argv[1]);
	}
  if(argc > 2) {
		sta = atol(argv[2]);
  }	
	
	tirgSignal[port].pushTrig = sta;
}

MSH_CMD_EXPORT_ALIAS(cmd_trig_ctrl, push_trig_io, ctrl states of trig);
