#include "mdbus_tcp_master.h"
#include "agile_modbus.h"
#include "drv_tcp_common.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <sys/socket.h>
#include "netdb.h"
#include "string.h"		
#include "stdio.h"
#include "mdbus_tcp_slaver.h"
#include "user_outputIO.h"
#include "user_encoder.h"
#include "user_pid.h"
#include "user_alarm.h"

/* 施耐德变频器位置同步误差过大 因为主从变频器没有位置闭环，只有速度闭环；

		解决方案1：不使用主从控制模式 主从电机分别跑速度闭环控制 主控只需要跑一个位置环来同步位置误差 程序设计会简单一些
		
		解决方案2：不使用主从控制模式 主从分别跑速度开环控制 主控需要给主从变频器分别设定一个速度环 再设定一个位置环来实现主从同步 主控运算量增大
*/

// 0x6040控制字
#define CONTROL_WORD_DISABLE 						0x00 			// 禁止加电
#define CONTROL_WORD_QUICK_STOP 				0x02 			// 快速停机
#define CONTROL_WORD_SHUTDOWN 					0x06      // 停止
#define CONTROL_WORD_SWITCH_ON 					0x07			// 通电
#define CONTROL_WORD_ENABLE_OPERATION 	0x0F			// 允许运行
#define CONTROL_WORD_RESET 							0x80			// 故障复位 先发送 0x00 再发送 0x80 即数据上升沿复位

#define MOTOR_STOP_TIMEOUT							15000     //15000*20ms = 5min

#define DBG_ENABLE
#define DBG_COLOR
#define DBG_SECTION_NAME "mbtcp_master"
#define DBG_LEVEL        DBG_LOG
#include "drv_log.h"

static struct rt_thread mb_tcp_thread;
static char mb_tcp_thread_stack[4096];

static int master_sock = -1;
static int slave_sock = -1;

static MotorParamTypeDef motorParam;				//主从电机运行参数 状态返回参数

static MotorOperateTypeDef	m_motorOP;			//主电机控制参数

static MotorOperateTypeDef	s_motorOP;			//从电机控制参数

static PIDController position_pid;					//创建主从同步控制PID参数

static MotorInformation masterInfo;					//主电机同步控制参数

static MotorInformation slaveInfo;					//从电机同步控制参数

static uint8_t masterMotorPulseBak = 0, slaverMotorPulseBak = 0;

//获取主电机速度
int16_t get_main_motor_LFRD(void)
{
	return motorParam.m_LFRD;
//	return motorParam.m_RFRD;
}

//判断主电机是否已经停止
uint8_t get_main_motor_state(void)
{
	if((m_motorOP.eState == eMotorStop) && (m_motorOP.motorState == 0)) return 0;
	return 1;
}

//重置电机扩展状态值  用于电机复位
void reset_ms_motor_stateEx(void)
{
	m_motorOP.stateEx = 0;
	s_motorOP.stateEx = 0;
}

//设置主电机运行参数
void set_master_motor_param(enum MainMotorStatus state, int16_t target, uint8_t smooth)
{
	m_motorOP.eState = state;
	m_motorOP.targetLFRD = target;
	m_motorOP.speedSmooth = smooth;
}

//设置从电机运行参数
void set_slaver_motor_param(enum MainMotorStatus state, int16_t target, uint8_t smooth)
{
	s_motorOP.eState = state;
	s_motorOP.targetLFRD = target;
	s_motorOP.speedSmooth = smooth;
}

int modbus_write_holdregisters(agile_modbus_t *ctx, int _sock, uint16_t addr, int nb,  const uint16_t *value)
{
	int send_len = agile_modbus_serialize_write_registers(ctx, addr, nb, value);
	modbus_tcp_send(_sock, ctx->send_buf, send_len);
	int read_len = modbus_tcp_receive(_sock, ctx->read_buf, 12, 1000);
	if (read_len < 0) {
		LOG_E("Receive error, now exit.");
		return -1;
	}
	if (read_len == 0) {
		LOG_W("Receive timeout.");
		return -2;
	}
	int rc = agile_modbus_deserialize_write_registers(ctx, read_len);
	if (rc < 0) {
			LOG_W("Receive failed. %d", read_len);
			if (rc != -1)
					LOG_W("Error code:%d", -128 - rc);
			return -3;
	}

	return 0;
}

int modbus_read_holdregisters(agile_modbus_t *ctx, int _sock, uint16_t addr, int nb, uint16_t *value)
{
	int send_len = agile_modbus_serialize_read_registers(ctx, addr, nb);
	modbus_tcp_send(_sock, ctx->send_buf, send_len);
	int read_len = modbus_tcp_receive(_sock, ctx->read_buf, 9+nb*2, 1000);
	if (read_len < 0) {
		LOG_E("Receive error, now exit.");
		return -1;						
	}
	if (read_len == 0) {
		LOG_W("Receive timeout. %d", _sock);
		return -2;
	}
	int rc = agile_modbus_deserialize_read_registers(ctx, read_len, value);
	if (rc < 0) {
		LOG_W("Receive failed.");
		if (rc != -1)
			LOG_W("Error code:%d", -128 - rc);
		return -3;
	}
	return 0;
}

//设置电机控制命令cmd
int set_control_word_CMD(agile_modbus_t *ctx, int _sock, const uint16_t *value)
{
	return modbus_write_holdregisters(ctx, _sock, 12761, 1, value);
}

//设置电机目标转速 value=转每分钟
int set_target_velocity_LFRD(agile_modbus_t *ctx, int _sock, const uint16_t *value)
{
	return modbus_write_holdregisters(ctx, _sock, 12762, 1, value);
}

//设置电机加速斜坡
int set_Accelerate_slope_ACC(agile_modbus_t *ctx, int _sock, const uint16_t *value)
{
	return modbus_write_holdregisters(ctx, _sock, 12763, 1, value);
}

//设置电机减速斜坡
int set_deceleration_slope_DEC(agile_modbus_t *ctx, int _sock, const uint16_t *value)
{
	return modbus_write_holdregisters(ctx, _sock, 12764, 1, value);
}

//读取电机状态字
int read_status_word_ETA(agile_modbus_t *ctx, int _sock, uint16_t *value)
{
	return modbus_read_holdregisters(ctx, _sock, 12741, 1, value); 
}

//读取电机实际控制速度
int read_actual_velocity_RFRD(agile_modbus_t *ctx, int _sock, uint16_t *value)
{
	return modbus_read_holdregisters(ctx, _sock, 12742, 1, value);
}

//读取电机电流
int read_motor_electric_current_LCR(agile_modbus_t *ctx, int _sock, uint16_t *value)
{
	return modbus_read_holdregisters(ctx, _sock, 12743, 1, value);
}

//电机启动加速过程
const int16_t smoothStep = 5;   //定义加速的步进值 加速周期为20ms 例如目标速度为(100b%)1428转每分钟 则加速时间≈5.7秒
//const int16_t smoothStep = 10;   //定义加速的步进值 加速周期为40ms 例如目标速度为(100b%)1428转每分钟 则加速时间≈5.7秒
void master_motor_run_smooth(void)
{
	if(motorParam.m_LFRD < m_motorOP.targetLFRD){
		motorParam.m_LFRD += smoothStep;
	}else{
		motorParam.m_LFRD = m_motorOP.targetLFRD;	
	}
}

//电机停止减速过程 需要停在原点位置
void master_motor_stop_smooth(void)
{
	if(motorParam.m_LFRD > smoothStep){
		motorParam.m_LFRD -= smoothStep;
	}else{
//		motorParam.CMD = 0x07;
		motorParam.m_LFRD = 0;
		m_motorOP.motorState = 0;
	}
}

/* 主电机电机控制流程
	1.空闲状态
	2.正常运行状态
	3.停机状态
	4.故障报警状态
*/
void master_motor_state_process(void)
{
	//定义主从电机运行状态机
	switch(m_motorOP.eState){
		case eMotorIdle:	//初始状态
			m_motorOP.stateEx = 0;
			break;
		case eMotorRun:		//运行状态
			m_motorOP.stopTimeout = 0;
			m_motorOP.motorState = 1;
			if(motorParam.m_ETA.bit0==1 && motorParam.m_ETA.bit1==1 && motorParam.m_ETA.bit2==1 && motorParam.m_ETA.bit3==0){ //电机无异常 直接启动电机
				m_motorOP.stateEx = 5;
			}
			switch(m_motorOP.stateEx)
			{
				case 0: motorParam.m_CMD = 0; m_motorOP.stateEx++; break; 																	//298=01010011000 218=01000011000
				case 1: motorParam.m_CMD = 0x80; if(motorParam.m_ETA.bit3==0) m_motorOP.stateEx++; break;   //2d0=01011010000 250=01001010000
				case 2: motorParam.m_CMD = 0x06; if(motorParam.m_ETA.bit0==1) m_motorOP.stateEx++; break;	  //2b1=01010110001 231=01000110001
				case 3: motorParam.m_CMD = 0x07; if(motorParam.m_ETA.bit1==1) m_motorOP.stateEx++; break;   //2b3=01010110011 233=01000110011
				case 4: motorParam.m_CMD = 0x0F; if(motorParam.m_ETA.bit2==1) m_motorOP.stateEx++; break;   //637=11000110111 237=01000110111
				case 5:
					if(1 == m_motorOP.speedSmooth){		  	//需要平滑加速
						master_motor_run_smooth(); 
					}else{																//直接启动电机
						motorParam.m_LFRD = m_motorOP.targetLFRD;
					}
					break;
				default: m_motorOP.stateEx = 0; break;
			}
			break;
		case eMotorStop:	//暂停状态
			if(1 == m_motorOP.speedSmooth){						//需要平滑减速
				master_motor_stop_smooth();
			}else{																		//直接停止电机
//				motorParam.CMD = 0x07;
				motorParam.m_LFRD = 0;
				m_motorOP.motorState = 0;
			}
			m_motorOP.stateEx = 0;
			m_motorOP.stopTimeout++;
			if(m_motorOP.stopTimeout >= MOTOR_STOP_TIMEOUT){
				m_motorOP.stopTimeout = MOTOR_STOP_TIMEOUT;
				motorParam.m_CMD = 0x07;
			}
			break;
		case eMotorAlarm:	//故障报警状态
			motorParam.m_CMD = 0;
			motorParam.m_LFRD = 0;
			m_motorOP.stateEx = 7;
			break;
		default: break;
	}
}

/* 从电机控制流程
	1.空闲状态
	2.正常运行状态
	3.停机状态
	4.故障报警状态
*/
void slaver_motor_state_process(void)
{
	//定义主从电机运行状态机
	switch(s_motorOP.eState){
		case eMotorIdle:	//初始状态
			s_motorOP.stateEx = 0;
			break;
		case eMotorRun:		//运行状态
			s_motorOP.stopTimeout = 0;
			if(	motorParam.s_ETA.bit0==1 && motorParam.s_ETA.bit1==1 &&  motorParam.s_ETA.bit2==1 && motorParam.s_ETA.bit3==0 ){ //电机无异常 直接使能电机
				s_motorOP.stateEx = 5;
			}
			switch(s_motorOP.stateEx)
			{
				case 0: motorParam.s_CMD = 0; s_motorOP.stateEx++; break; 																	//298=01010011000 218=01000011000
				case 1: motorParam.s_CMD = 0x80; if(motorParam.s_ETA.bit3==0) s_motorOP.stateEx++; break;   //2d0=01011010000 250=01001010000
				case 2: motorParam.s_CMD = 0x06; if(motorParam.s_ETA.bit0==1) s_motorOP.stateEx++; break;	  //2b1=01010110001 231=01000110001
				case 3: motorParam.s_CMD = 0x07; if(motorParam.s_ETA.bit1==1) s_motorOP.stateEx++; break;   //2b3=01010110011 233=01000110011
				case 4: motorParam.s_CMD = 0x0F; if(motorParam.s_ETA.bit2==1) s_motorOP.stateEx++; break;   //637=11000110111 237=01000110111
				case 5: 	//从电机没有加减速控制 而是跟随主电机运动
					if(1 == s_motorOP.speedSmooth){
						motorParam.s_LFRD = slaveInfo.current_freq_hz;	//从电机位置闭环控制逻辑
					}else{
						motorParam.s_LFRD = s_motorOP.targetLFRD; 
					}
					break;
				default: s_motorOP.stateEx = 0; break;
			}
			break;
		case eMotorStop:	//暂停状态
			if(1 == s_motorOP.speedSmooth){						//需要平滑减速
				motorParam.s_LFRD = slaveInfo.current_freq_hz;   //从电机位置闭环控制逻辑
			}else{																		//直接停止电机
				motorParam.s_LFRD = 0;
			}
			s_motorOP.stateEx = 0;
			s_motorOP.stopTimeout++;
			if(s_motorOP.stopTimeout >= MOTOR_STOP_TIMEOUT){
				s_motorOP.stopTimeout = MOTOR_STOP_TIMEOUT;
				motorParam.s_CMD = 0x07;
			}
			break;
		case eMotorAlarm:	//故障报警状态
			motorParam.s_CMD = 0;
			motorParam.s_LFRD = 0;
			s_motorOP.stateEx = 7;
			break;
		default: break;
	}
}

//读取电机状态参数
int8_t read_motor_work_param(agile_modbus_t *ctx, int id, int _sock)
{
	uint16_t hold_register[10] = {0,0,0,0,0,0,0,0,0,0};
	
	agile_modbus_set_slave(ctx, id);
	if(modbus_read_holdregisters(ctx, _sock, 12741, 3, hold_register) < 0){
		return -1;
	}else{
		switch(id)
		{
			case 1:
				memcpy((void *)&motorParam.m_ETA, (void *)&hold_register[0], 2);
				motorParam.m_RFRD = hold_register[1];
				motorParam.m_LCR = hold_register[2];
				break;
			case 2:
				memcpy((void *)&motorParam.s_ETA, (void *)&hold_register[0], 2);
				motorParam.s_RFRD = hold_register[1];
				motorParam.s_LCR = hold_register[2];
				break;
			default: break;
		}
	}	
	
	return 0;
}

int8_t write_motor_work_param(agile_modbus_t *ctx, int id, int _sock)
{
	uint16_t hold_register[10] = {0,0,0,0,0,0,0,0,0,0};
	
	if((1 != motorParam.s_ETA.bit2)&&(0 == motorParam.s_ETA.bit3)) { }; //异常处理 返回结果
	
	switch(id)
	{
		case 1:
			hold_register[0] = motorParam.m_CMD;
			hold_register[1] = motorParam.m_LFRD;	
			break;
		case 2:
			hold_register[0] = motorParam.s_CMD; 
			hold_register[1] = motorParam.s_LFRD;
			break;
		default: break;
	}
	hold_register[2] = 30;
	hold_register[3] = 30;
	
	agile_modbus_set_slave(ctx, id);
	
	if(modbus_write_holdregisters(ctx, _sock, 12761, 4,  hold_register)<0) {  //设置主电机控制参数
		return -1;
	}
	
	return 0;
}

void motor_state_init(MotorInformation* motor, int8_t sta)
{
//    motor->raw_encoder_count = 0;
    motor->absolute_position = 0;
    motor->last_home_count = 0;
    motor->home_detected = sta;
    motor->current_freq_hz = 0;
}

void update_master_motor_position(MotorInformation* motor, int32_t pulses_per_rev)
{
	int32_t master_encoder = 0;
	uint8_t status = 0;
	
	master_encoder = secondPulseEncoderGet(); //获取主电机编码器值
	status = get_input_io_sta(masterOperate.inputData, eInputIO25);
	if((1 == masterMotorPulseBak)&&(0 == status)&&(master_encoder > 300)){  //处理接近开关信号 脉冲间隔需大于300才有效
		// 主编码器计数清零
		secondPulseEncoderClear();
		
		if(motor->home_detected == RT_FALSE){
			// 第一次检测到参考点,初始化绝对位置基准
			// 假设参考点对应绝对位置为 0 或某个固定值,将当前绝对位置修正为raw_count 的整数倍
			motor->home_detected = RT_TRUE;

			// 将绝对位置设置为与当前raw_count最接近的pulses_per_rev的整数倍 避免累计误差
			motor->absolute_position = 0;
		}	else {
			// 后续每次经过参考点 进行误差校正
			// 计算理论上的参考点位置
			int32_t expected_home_pos = motor->last_home_count + master_encoder;
			
			// 计算实际应该对齐到的整圈位置
			int32_t target_abs_pos = (int32_t)round((double)expected_home_pos / pulses_per_rev) * pulses_per_rev;
			
			// 计算校准偏差
//			int32_t correction = target_abs_pos - expected_home_pos;

			// 校准后的绝对位置
			motor->absolute_position = target_abs_pos;
		}
		
		motor->last_home_count = motor->absolute_position;
	} else {
    // 未收到接近开关信号
    if (motor->home_detected == RT_TRUE) {
			motor->absolute_position = motor->last_home_count + master_encoder;
    } else {
			// 未复位时 暂时master_encoder代替或者置0
			motor->absolute_position = master_encoder; 
    }		
	}

	masterMotorPulseBak = status;
}

void update_slaver_motor_position(MotorInformation* motor, int32_t pulses_per_rev)
{
	int32_t slaver_encoder = 0;
	uint8_t status = 0;
	
	slaver_encoder = thirdPulseEncoderGet(); 	//获取从电机编码器值
	status = get_input_io_sta(masterOperate.inputData, eInputIO26);
	if((1 == slaverMotorPulseBak)&&(0 == status)&&(slaver_encoder > 300)){  //处理接近开关信号 脉冲间隔需大于300才有效
		// 从编码器计数清零
		thirdPulseEncoderClear();
		
		if(motor->home_detected == RT_FALSE){
			// 第一次检测到参考点,初始化绝对位置基准
			// 假设参考点对应绝对位置为 0 或某个固定值,将当前绝对位置修正为raw_count 的整数倍
			motor->home_detected = RT_TRUE;

			// 将绝对位置设置为与当前raw_count最接近的pulses_per_rev的整数倍 避免累计误差
			motor->absolute_position = 0;
		}	else {
			// 后续每次经过参考点 进行误差校正
			// 计算理论上的参考点位置
			int32_t expected_home_pos = motor->last_home_count + slaver_encoder;
			
			// 计算实际应该对齐到的整圈位置
			int32_t target_abs_pos = (int32_t)round((double)expected_home_pos / pulses_per_rev) * pulses_per_rev;
			
			// 计算校准偏差
//			int32_t correction = target_abs_pos - expected_home_pos;

			// 校准后的绝对位置
			motor->absolute_position = target_abs_pos;
		}
		
		motor->last_home_count = motor->absolute_position;
	} else {
    // 未收到接近开关信号
    if (motor->home_detected == RT_TRUE) {
			motor->absolute_position = motor->last_home_count + slaver_encoder;
    } else {
			// 未复位时 暂时master_encoder代替或者置0
			motor->absolute_position = slaver_encoder; 
    }		
	}

	slaverMotorPulseBak = status;
}

//主从同步控制算法的另一种实现 更安全
int16_t apply_control_outputs_externel(MotorInformation* master, MotorInformation* slave, PIDController *pid, int16_t base_freq)
{
	int16_t diff = 100;
	
	update_master_motor_position(&masterInfo, 400);
	
	update_slaver_motor_position(&slaveInfo, 400);
	
	//清理累计值 防止数据溢出
	if((master->last_home_count > 12000)&&(slave->last_home_count > 12000)){
		master->last_home_count -= 10000;
		slave->last_home_count -= 10000;
	}

  // 计算位置误差
  double position_error = (double)(master->absolute_position - slave->absolute_position);	
	
  // 计算速度修正量
  double speed_correction = pid_compute(pid, 0.0, position_error, 0.1);
	
  // 计算最终频率
  double target_freq = base_freq + speed_correction;
	
	if(base_freq <= 0) target_freq = 0;
	
  // 输出频率安全限制
  if (target_freq < 0.0) target_freq = 0.0;
  if (target_freq > 1428.0) target_freq = 1428.0;
	
	slave->current_freq_hz = target_freq;

//	diff = u_tab_registers[718];
	diff = 200;
	if(((position_error < -diff)||(position_error > diff))&&(diff != 400)){		//主从电机同步位置误差过大
		trigger_alarm(ALARM_LEVEL_ERROR, eMotorSyncError, "NULL");
	}

//	static int8_t cnt =0;
//	cnt = (cnt + 1) % 3;
//	if(0 == cnt){
//		char buf[50];
//		sprintf(buf, "%d %.1f %d-%d=%.1f", base_freq, target_freq,  master->absolute_position, slave->absolute_position, position_error);
//		rt_kprintf("diff: %s\n",  buf);
//	}

	return (int16_t)target_freq;
}

void reset_diff_pos(void)
{
    motor_state_init(&masterInfo, RT_TRUE);
    motor_state_init(&slaveInfo, RT_TRUE);
}

static void modbus_cycle_thread_entry(void *param)
{
		static uint8_t sync_motor_cnt = 0;
    uint8_t ctx_send_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint8_t ctx_read_buf[AGILE_MODBUS_MAX_ADU_LENGTH];
		int8_t reconnect_cnt = 100;
	
		//初始化电机控制参数
		memset(&motorParam, 0, sizeof(MotorParamTypeDef));
		motorParam.ACC = 30;
		motorParam.DEC = 30;

		//初始化位置同步PID
		pid_init(&position_pid, 0.5f, 0.3f, 0.05f, -100.0f, 100.0f);
	
		//初始化主从电机
    motor_state_init(&masterInfo, RT_FALSE);
    motor_state_init(&slaveInfo, RT_FALSE);

		//初始化主电机编码器2 + 从电机编码器3
		secondPulseEncoderInit();
		thirdPulseEncoderInit();
	
		//初始化modbus协议
    agile_modbus_tcp_t ctx_tcp;
    agile_modbus_t *ctx = &ctx_tcp._ctx;
    agile_modbus_tcp_init(&ctx_tcp, ctx_send_buf, sizeof(ctx_send_buf), ctx_read_buf, sizeof(ctx_read_buf));
    agile_modbus_set_slave(ctx, 1);

    LOG_I("Running.");
		masterOperate.eSysState = eSystemIdle;

		//启动主从机电机
		setbitv(masterOperate.relayData, eRelayIO6);
		setbitv(masterOperate.relayData, eRelayIO7);
		updateRelayIO(masterOperate.relayData);
		rt_thread_mdelay(2000);

		while(1) 
		{
			updateRelayIO(masterOperate.relayData);

			reconnect_cnt--;
			if(reconnect_cnt <= 0) {
				trigger_alarm(ALARM_LEVEL_ERROR, eATV930ConnectError, "NULL");
			}

			if(master_sock < 0){
				master_sock = modbus_tcp_connect("10.10.0.10", 502);
				if (master_sock == -1) {
//						LOG_E("Connect %s:%d failed!", "10.10.0.10", 502);
						rt_thread_mdelay(200);
						continue;
				}
			}
			if(slave_sock < 0){
				slave_sock = modbus_tcp_connect("10.10.0.11", 502);
				if (slave_sock == -1) {
//						LOG_E("Connect %s:%d failed!", "10.10.0.11", 502);
						rt_thread_mdelay(200);
						continue;
				}
			}

			rt_kprintf("Master-slave motor initialization successful.\n");

			while (1)
			{
					//PID控制参数计算 这个需要更快的速度去读取编码器和接近开关状态 所以每10ms执行一次
					apply_control_outputs_externel(&masterInfo, &slaveInfo, &position_pid, motorParam.m_LFRD);
				
					sync_motor_cnt = (sync_motor_cnt + 1) % 5;
					//与电机的modbus读通讯时间应设置在50~200ms之间 否则容易丢包
					if(0 == sync_motor_cnt)
					{
							//读取主电机当前状态并切换运行状态
							if(read_motor_work_param(ctx, 1, master_sock) != 0) break;
							else master_motor_state_process();
							
							//读取从电机当前状态并切换运行状态
							if(read_motor_work_param(ctx, 2, slave_sock) != 0) break;
							else slaver_motor_state_process();

							//两次通讯之间延时
							rt_thread_mdelay(10);
					
							//设置主电机控制参数
							if(write_motor_work_param(ctx, 1, master_sock) != 0) break;
		
							//设置从电机控制参数
							if(write_motor_work_param(ctx, 2, slave_sock) != 0) break;

							//主从电机故障报警 停止电机 触发故障报警信息
							if(motorParam.m_ETA.bit3==1 || motorParam.s_ETA.bit3==1){
								trigger_alarm(ALARM_LEVEL_ERROR, eATV930DeviceError, "NULL");
							}
					}

					updateRelayIO(masterOperate.relayData);							//继电器输出为低速设备 放在modbus主线程中更新输出端口状态

					rt_thread_mdelay(10);
				}

				modbus_tcp_close(slave_sock);  //关闭socket连接
				modbus_tcp_close(master_sock);
				master_sock = -1;
				slave_sock = -1;
				reconnect_cnt = 10;
				//出现断网直接报变频器通故障
				trigger_alarm(ALARM_LEVEL_ERROR, eATV930ConnectError, "NULL");
		}
}

int modbus_tcp_startup(void)
{
		rt_thread_init(&mb_tcp_thread,
                   "mb_master",
                   modbus_cycle_thread_entry,
                   RT_NULL,
                   &mb_tcp_thread_stack[0],
                   sizeof(mb_tcp_thread_stack),
                   13, 10);
    rt_thread_startup(&mb_tcp_thread);
									 
		return RT_EOK;
}

INIT_APP_EXPORT(modbus_tcp_startup);

#include <stdlib.h>
static void show_status(int argc, char **argv)
{
	rt_kprintf("slave ETA:%04x %04x %04x\n\r\n", motorParam.s_ETA, motorParam.s_RFRD, motorParam.s_LCR);
	rt_kprintf("master ETA:%04x %04x %04x\n", motorParam.m_ETA, motorParam.m_RFRD, motorParam.m_LCR);
	rt_kprintf("masterOperate.mainState=%d\n", masterOperate.eSysState);
}
MSH_CMD_EXPORT_ALIAS(show_status, show_status, show status of motor);
