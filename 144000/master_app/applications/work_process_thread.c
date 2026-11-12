#include <rtthread.h>
#include <drv_common.h>
#include "mainOrbitCacheQueue.h"
#include "mdbus_tcp_slaver.h"
#include "master402_od.h"
#include "dev_config.h"
#include "user_FSmachine.h"
#include "user_alarm.h"
#include "mdbus_tcp_master.h"
#include "user_kinco.h"
#include "user_encoder.h"
#include "production_param.h"

static struct rt_thread work_thread;
static char work_thread_stack[1024];

static uint8_t masterTakeTurn = 1;
static uint8_t slaverTakeTurn = 1;
			
static void hmi_page_switch_ctrol(void);

extern void reset_diff_pos(void);

//设置电动推缸私服电机参数
extern void set_cylinder_motor_param(enum eElectricCylinderState state, int32_t speed, uint8_t mode);

//重置伺服驱动器扩展状态值 用于伺服电机复位
extern void reset_servo_stateEx(void);
	
//电动推缸原点设置
extern void set_cylinder_motor_origen(int32_t pos, int32_t zero);
	
/* 设备管理-蛋杯状态 */
static void eggs_cup_show_page(void)
{
	uint16_t n, ob=0, pos=0;

	ob = (u_tab_registers[5720] - 1) % MAIN_ORBIT_NUM;
	for(n=0; n<48; n++){
		pos = (showPageQueue.front + n) % SHOW_PAGE_QUEUE_SIZE;
		u_tab_registers[5724+n] = 1;
		u_tab_registers[5772+n] = showPageQueue.items[pos].weigh[ob]; 		//重量
		u_tab_registers[5820+n] = showPageQueue.items[pos].level[ob]; 		//裂纹
		u_tab_registers[5868+n] = showPageQueue.items[pos].dirty[ob]; 		//脏蛋
		u_tab_registers[5916+n] = showPageQueue.items[pos].color[ob]; 		//颜色
		u_tab_registers[5964+n] = showPageQueue.items[pos].toChannel[ob]; //血斑
	}
}

void product_data_show_page(void)
{
	CacheCupStateDef *cache;
	int ch = 0, ob = 0, n;
	
	setModbusHoldingRegU32(52, proData.eggsTotal);
	
	setModbusHoldingRegU32(30, 100);
	
	for(ch=0; ch<sorterUsedNumber; ch++)
	{
		cache = get_cache_cup_state(ch);
		for(ob=0; ob<MAIN_ORBIT_NUM; ob++){
			for(n=0; n<6; n++){
				if(getbitv(cache->isFull[ob], n)){
					u_tab_registers[398+26*ch+ob*6+n] = 1;
				}else{
					u_tab_registers[398+26*ch+ob*6+n] = 0;
				}
			}
		}
		u_tab_registers[396+26*ch] = proData.packageState[ch];
	}
}

//更新系统时间
void updateSystemDateTime(void)
{
	time_t now;
	struct tm *p;
	
	now = time(RT_NULL);
	p = localtime(&now);

	u_tab_registers[6590] = p->tm_year+1900;
	u_tab_registers[6591] = p->tm_mon+1;
	u_tab_registers[6592] = p->tm_mday;
	u_tab_registers[6593] = p->tm_hour;
	u_tab_registers[6594] = p->tm_min;
	u_tab_registers[6595] = p->tm_sec;
	
	u_tab_registers[6600] = now&0xffff; 
	u_tab_registers[6601] = (now>>16)&0xffff;	
}

#include "repostCacheQueue.h"
#include "dirtyAndColorEggsQueue.h"
#include "mainOrbitCacheQueue.h"
//主控电机和电动推缸复位控制
void extension_line_reset_ctrol_ex()
{
	if(0 == get_input_io_sta(masterOperate.inputData, eInputIO30)){  	//主电机复位完成信号
		masterOperate.mainMotorResetFlag = RT_TRUE;
		set_master_motor_param(eMotorStop, 0, 0);
	}else{																														//复位进行中
		set_master_motor_param(eMotorRun, 200, 0); //200rpm
	}

	if(0 == get_input_io_sta(masterOperate.inputData, eInputIO26)){  	//从电机复位完成信号
		masterOperate.slaveMotorResetFlag = RT_TRUE;
		set_slaver_motor_param(eMotorStop, 0, 0);
	}else{																														//复位进行中
		set_slaver_motor_param(eMotorRun, 200, 0);	
	}
	
	if(1 == get_input_io_sta(masterOperate.inputData, eInputIO39)){  	//到达缩回限位 电动推杆找到原点
		masterOperate.servoMotorResetFlag = RT_TRUE;
		set_cylinder_motor_param(eCylinderClose, 0, 2);
		set_cylinder_motor_origen(0, position_actual_value_6064); 			//电动推缸初始位置
	}else{
		set_cylinder_motor_param(eCylinderOpen, -6000, 2);		
	}

	/* 主从电机和伺服电缸复位完成或者再次点击复位按钮则停止复位 */
	if(((RT_TRUE == masterOperate.mainMotorResetFlag)&&(RT_TRUE == masterOperate.slaveMotorResetFlag)&&(RT_TRUE == masterOperate.servoMotorResetFlag))||(masterOperate.keyBtn == keyResetBtn)){
		masterOperate.eSysState = eSystemStop;													//回到系统暂停状态
		masterOperate.keyBtn = keyUnknowBtn;
		secondPulseEncoderClear();
		thirdPulseEncoderClear();
		reset_diff_pos();
		set_cylinder_motor_param(eCylinderClose, 0, 2);
		set_master_motor_param(eMotorStop, 0, 0);
		set_slaver_motor_param(eMotorStop, 0, 0);
		led_and_beep_ctrl(eGreen, 1);
		allCacheQueueInit();																						//重新初始化所有缓存队列
	}
}

void extension_line_debug_ctrol_ex(void)
{
	if(1 == u_tab_registers[5525]){   									 //选中主轨道输送辊条电机
		if(1 == u_tab_registers[5529]){  								 	//选择点动
			if(masterOperate.keyBtn == keyRunBtn){				 	//点动按钮按下运行
				if(2 == u_tab_registers[5526]){   						//单从机运行模式
					set_slaver_motor_param(eMotorRun, u_tab_registers[5550], 0);
				}else if(1 == u_tab_registers[5526]){					//单主机运行模式
					set_master_motor_param(eMotorRun, u_tab_registers[5550], 0);
				}else if(3 == u_tab_registers[5526]){					//主从模式
					set_slaver_motor_param(eMotorRun, u_tab_registers[5550], 0);
					set_master_motor_param(eMotorRun, u_tab_registers[5550], 0);
				}
				masterOperate.keyBtn = keyUnknowBtn;
			}else if(masterOperate.keyBtn == keyRunBtnUp){ 	//点动按钮松开停止主从电机
				set_master_motor_param(eMotorStop, 0, 0);
				set_slaver_motor_param(eMotorStop, 0, 0);
				masterOperate.keyBtn = keyUnknowBtn;
			}
		}else if(1 == u_tab_registers[5530]){ 						//选择持续动作
			if(masterOperate.keyBtn == keyRunBtn){
				if(2 == u_tab_registers[5526]){   						//单从机运行模式
					if(0 == slaverTakeTurn){								
						slaverTakeTurn = 1;												//运行
					}else if(1 == slaverTakeTurn){ 				
						slaverTakeTurn = 0;												//停止
					}
					if(1 == slaverTakeTurn){										//运行
						set_slaver_motor_param(eMotorRun, u_tab_registers[5550], 0);
					}else if(0 == slaverTakeTurn){ 							//停止
						set_slaver_motor_param(eMotorStop, 0, 0);
					}
				}else if(1 == u_tab_registers[5526]) {				//单主机模式
					if(0 == masterTakeTurn){								
						masterTakeTurn = 1;												//运行
					}else if(1 == masterTakeTurn){ 				
						masterTakeTurn = 0;												//停止
					}
					if(1 == masterTakeTurn){										//运行
						set_master_motor_param(eMotorRun, u_tab_registers[5550], 0);
					}else if(0 == masterTakeTurn){ 							//停止
						set_master_motor_param(eMotorStop, 0, 0);
					}
				}else if(3 == u_tab_registers[5526]){					//主从模式
					if(0 == masterTakeTurn){								
						masterTakeTurn = 1;												//运行
					}else if(1 == masterTakeTurn){ 				
						masterTakeTurn = 0;												//停止
					}
					if(1 == masterTakeTurn){										//运行
						set_master_motor_param(eMotorRun, u_tab_registers[5550], 0);
						set_slaver_motor_param(eMotorRun, u_tab_registers[5550], 0);
					}else if(0 == masterTakeTurn){ 							//停止
						set_master_motor_param(eMotorStop, 0, 0);
						set_slaver_motor_param(eMotorStop, 0, 0);
					}					
				}
				masterOperate.keyBtn = keyUnknowBtn;
			}
		}
	}else if(1 == u_tab_registers[5531]){ 							//选中电动推缸
		if(masterOperate.keyBtn == keyRunBtn){
			if(1 == u_tab_registers[5532]){ 								//伸出
				set_cylinder_motor_param(eCylinderOpen, 6000, 1);
			}else if(1 == u_tab_registers[5533]){ 					//缩回
				set_cylinder_motor_param(eCylinderOpen, -6000, 1);
			}
			masterOperate.keyBtn = keyUnknowBtn;
		}else if(masterOperate.keyBtn == keyRunBtnUp){		//电动推缸停止运行
			set_cylinder_motor_param(eCylinderClose, 0, 1);
			masterOperate.keyBtn = keyUnknowBtn;
		}
	}else{
		masterOperate.keyBtn = keyUnknowBtn;
	}
}

//系统初始化状态
void system_work_idle_state(void)
{
	led_and_beep_ctrl(eGreen, 1);
	set_cylinder_motor_param(eCylinderClose, 0, 0);//伺服电机停止状态
	set_master_motor_param(eMotorStop, 0, 0);			 //主从电机都设置为停止状态
	set_slaver_motor_param(eMotorStop, 0, 0);
	masterOperate.eSysState = eSystemStop;  			 //系统切换到暂停状态
	masterOperate.keyBtn = keyUnknowBtn;					 //初始化按键状态
	masterOperate.mainMotorResetFlag = RT_FALSE;   //清除主电机复位标志
	masterOperate.slaveMotorResetFlag = RT_FALSE;  //清除从电机复位标志
	masterOperate.servoMotorResetFlag = RT_FALSE;  //清除伺服电机复位标志
}

void system_work_run_state(void)
{
	alarm_state_t alarmSta;
	
	alarmSta = get_alarm_action();

	if((masterOperate.keyBtn == keyStopBtn)||(masterOperate.keyBtn == keyRunBtn)||(alarmSta.action == ALARM_ACTION_SHUTDOWN)){
		set_master_motor_param(eMotorStop, 0, 1); 	 //需要做减速到原点位置再停止电机的控制
		set_slaver_motor_param(eMotorStop, 0, 1);
		set_cylinder_motor_param(eCylinderOpen, 12000, 3); //电动缸跟随主电机速度运动
		masterOperate.keyBtn = keyUnknowBtn;
	}

	if(alarmSta.action == ALARM_ACTION_SHUTDOWN){
		led_and_beep_ctrl(eRed, 1);
	}else{
		led_and_beep_ctrl(eGreen, 1);
	}
	
	if(0 == get_main_motor_state()){							 //电机完全停止以后才切换到暂停状态
		masterOperate.eSysState = eSystemStop;
		clrbitv(masterOperate.relayData, eRelayIO5);
	}
}

//系统暂停状态下的用户操作处理逻辑
void system_work_stop_state(void)
{
	alarm_state_t alarmSta;
	
	if(masterOperate.keyBtn == keyRunBtn){
		masterOperate.eSysState = eSystemReady;
		masterOperate.sysStateEx = 0;  //切换到启动延时的第一个状态
		alarmConfirmBtn = RT_TRUE;     //一键确认报警
		masterOperate.keyBtn = keyUnknowBtn;
		getPackageWorkInformation(&workRecipe, packageWorkList);	//先计算出每个包装通道的运行参数
	}else if(masterOperate.keyBtn == keyResetBtn){ //按下复位键后 先将所有电机恢复到RUN状态 清除故障报警
		masterOperate.eSysState = eSystemReset;
		masterOperate.mainMotorResetFlag = RT_FALSE;
		masterOperate.slaveMotorResetFlag = RT_FALSE;  //清除从电机复位标志
		masterOperate.servoMotorResetFlag = RT_FALSE;
		reset_ms_motor_stateEx();
		set_master_motor_param(eMotorRun, 0, 0);
		set_slaver_motor_param(eMotorRun, 0, 0);
		reset_servo_stateEx();
		set_cylinder_motor_param(eCylinderOpen, 0, 2);
		rt_thread_mdelay(20);
		masterOperate.keyBtn = keyUnknowBtn;
		pos_cmd1 = position_actual_value_6064; 			//电动推缸当前位置
	}else{
		hmi_page_switch_ctrol();
		alarmSta = get_alarm_action();
		if(alarmSta.action == ALARM_ACTION_SHUTDOWN){
			led_and_beep_ctrl(eRed, 1);
		}else{
			led_and_beep_ctrl(eGreen, 1);
		}
	}
}

static int sys_ready_delay = 0;
//启动准备状态 发出声光报警 发送包装参数到每个通道等操作
void system_work_ready_state(void)
{
	/* 1.启动声光报警
	 * 2.发送通道配置参数1
	 * 3.发送通道配置参数2
	 * 4.停止声光报警 启动设备
	 * 5.启动过程中用户按暂停 需要停止当前启动过程
	 */
	switch(masterOperate.sysStateEx)
	{
		case 0:  //初始状态
			sys_ready_delay = 0;
			masterOperate.sysStateEx = 1;
			set_package_param_1();
			led_and_beep_ctrl(eYellow, 1);
			break;
		case 1: //发送参数1的延时过程 1秒
			if(sys_ready_delay == 25){
				led_and_beep_ctrl(eYellow, 0);
			}else if(sys_ready_delay == 50){
				led_and_beep_ctrl(eYellow, 1);
				set_package_param_2();
				masterOperate.sysStateEx = 2;
			}
			break;
		case 2: //发送参数2的延时过程 1秒
			if(sys_ready_delay == 75){
				led_and_beep_ctrl(eYellow, 0);
			}else if(sys_ready_delay == 100){
				led_and_beep_ctrl(eYellow, 1);
				set_package_param_3();
				masterOperate.sysStateEx = 3;
			}
			break;
		case 3: //延时过程 1秒
			if(sys_ready_delay == 125){
				led_and_beep_ctrl(eYellow, 0);
			}else if(sys_ready_delay == 150){
				masterOperate.sysStateEx = 4;
			}
			break;
		case 4: //启动设备 主状态切换到运行状态
			if(workRecipe.workSpeed == 6){
				masterOperate.masterSetSpeed = 12 * workRecipe.workSpeedEx;
			}else if(workRecipe.workSpeed > 0){
				masterOperate.masterSetSpeed = 600 + 120 * workRecipe.workSpeed; //speed = 12 * (50 + 10 * workRecipe.workSpeed);
			}else{
				masterOperate.masterSetSpeed = 600;
			}
			setModbusHoldingRegU32(50, (uint32_t)masterOperate.masterSetSpeed*100);
			set_master_motor_param(eMotorRun, masterOperate.masterSetSpeed, 1);  //需要做加速控制
			set_slaver_motor_param(eMotorRun, masterOperate.masterSetSpeed, 1);
			set_cylinder_motor_param(eCylinderOpen, 12000, 3); //电动缸跟随主电机速度运动
			masterOperate.eSysState = eSystemRun;
			setbitv(masterOperate.relayData, eRelayIO5);
			break;
		default: sys_ready_delay = 0; break;
	}
	sys_ready_delay++;
}

void hmi_show_status_page_switch(void)
{
	uint16_t page = 0;
	
	page = u_tab_registers[0];
	switch(page)
	{
		case 114:	/* 设备管理-蛋杯状态 */
			eggs_cup_show_page();
			break;
		case 115:	/* 设备管理-蛋杯预警 */
			
			break;
		case 105:	//包装线调试
			
			break;
		default: 
			
			break;
	}
}

//用于界面切换时 在手动控制和自动控制之间切换设备状态
void hmi_page_switch_ctrol(void)
{
	uint16_t page = 0;
	
	if(eSystemIdle == masterOperate.eSysState) return;
	
	page = u_tab_registers[0];
	switch(page)
	{
		case 103:	//分级线调试
			masterOperate.eSysState = eExtensionLineStep;
			break;
		case 104:	//输送线调试
			masterOperate.eSysState = ePipelineStep;
			break;
		case 105:	//包装线调试
			masterOperate.eSysState = eChannelStep;
			break;
		case 61:	//新建or编辑包装盒
			masterOperate.eSysState = ePackageEdit;
			break;
		default: 
			masterOperate.eSysState = eSystemStop;
			break;
	}
}

//每20ms执行一次 用于控制整机运行状态
void system_work_process(void)
{
	switch(masterOperate.eSysState)
	{
		case eSystemIdle:  				//开机或者故障后恢复到初始状态
			system_work_idle_state();
			break;
		case eSystemRun:  				//运行状态
			system_work_run_state();
			break;
		case eSystemStop:  				//暂停状态
			system_work_stop_state();
			break;
		case eSystemReady:				//暂停状态按下启动按钮 增加启动延时步骤 控制声光报警 向包装通道发送参数等逻辑
			system_work_ready_state();
			break;
		case eSystemReset:				//主控设备复位状态
			extension_line_reset_ctrol_ex();
			break;
		case eChannelStep: 				//包装通道调试 手动控制状态
			hmi_page_switch_ctrol();
			break;
		case ePipelineStep: 			//输送线调试 手动控制状态
			hmi_page_switch_ctrol();
			break;
		case eExtensionLineStep:	//分级线调试 手动控制状态
			extension_line_debug_ctrol_ex();
			hmi_page_switch_ctrol();
			break;
		case ePackageEdit:				//包装管理中新建或者编辑包装盒的动作测试控制
			hmi_page_switch_ctrol();
			break;
		case eSystemAlarm: 				//故障报警状态
			break;
		default: masterOperate.eSysState = eSystemIdle;
			break;
	}
}

void work_process_thread_entry(void *param)
{
	int updateCnt = 0;
	
	while(RT_TRUE)
	{
		updateCnt = (updateCnt + 1) % 3;
		if(0 == updateCnt){
			hmi_show_status_page_switch();
			
			product_data_show_page();
			
			updateSystemDateTime();
		}
		
		system_work_process();       //系统运行 停止 加减速等处理逻辑的控制
		
		system_alarm_process();
		
		rt_thread_mdelay(20);
	}
}

int work_process_thread_init(void)
{
		rt_thread_init(&work_thread,
                   "work_proc",
                   work_process_thread_entry,
                   RT_NULL,
                   &work_thread_stack[0],
                   sizeof(work_thread_stack),
                   18, 10);
    rt_thread_startup(&work_thread);

		return RT_EOK;
}

INIT_APP_EXPORT(work_process_thread_init);
