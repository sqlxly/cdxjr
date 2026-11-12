#include <rtthread.h>
#include "rtdevice.h"
#include "./tools/common.h"
#include "push_out.h"
#include "string.h"

#define FDCAN_DEV_NAME       "fdcan1"      /* CAN device name */

/*
	can 命令码：0x01 -- 主机广播发送一排鸡蛋的起始信号 同时获取一排鸡蛋的裂纹数据
							0x02 -- 从机信号处理板返回裂纹检测数据
							0x03 -- 主机广播发送编码器移动的位置信息
							0x04 -- 从机返回32个陶瓷压电传感器新增错误次数 返回格式为传感器编号(1byte)+次数(1byte)
*/
typedef struct{
	double weigh[WEIGH_CHANNEL];			//六个通道的重量数据
} Eggs_Line_Weighs;

typedef struct{
	uint16_t id;
	uint8_t cmd;
	uint8_t addr;
} Fdcan_Msg_Head;

static rt_device_t fdcan_dev;            /* CAN device handle */

static struct rt_mailbox mbCan;

static char mbCan_pool[128];

static uint16_t udp_msg_id = 0;
	
static Egg_Quality_Info Egg_q_vector[UDP_SND_PLUSE];

static Eggs_Line_Weighs line_weighs[MOVE_OUT_PLUSE];

static uint8_t line_mark[MOVE_OUT_PLUSE] = {0};

static uint8_t crack_level[WEIGH_CHANNEL] = {0};

static uint8_t push_out_cnt[WEIGH_CHANNEL] = {0};

static struct rt_can_msg rxmsg[8];

static uint32_t total_eggs_found = 0;

uint8_t stop_flag;									//机器暂停运行标志

extern Bkup_Sram_Alarm bkup_alarm;  //热备区保存压电陶瓷传感器当前连续错误次数
extern uint8_t canConnectedErr[6];

extern void udp_submit_work(rt_uint32_t addr);
extern rt_int32_t read_pulse_encoder(void);
extern void calc_current_weighing(void);
extern void add_eggs_level_info(Egg_Quality_Info *info);
extern void get_eggs_weigh_online(double *pweight);
extern uint16_t getModbusHoldingReg(uint16_t addr);
extern uint16_t getModbusHoldingRegEx(uint16_t addr);
extern void setModbusHoldingReg(uint16_t addr, uint16_t v);
extern void setModbusHoldingRegU32(rt_uint16_t base_id, rt_uint32_t value);
extern void reset_fdcan_read_status(void);
extern CrackParam getCrackSensitivityParam(void);

void fdcan_submit_work(rt_ubase_t cmd)
{
		rt_mb_send(&mbCan, cmd);
}

#define EMPTY_TRIGER		5    //连续漏敲次数门限值  超过此值可能是大裂纹蛋  和传感器漏敲了的情况有冲突

uint8_t calc_crack_level_ex(struct rt_can_msg *msg, Eggs_Line_Weighs *weighs, uint8_t mark, uint8_t ch)
{
	uint8_t crack[32], crack_bak = 0;
	uint8_t n, mask = 0, isReal = 0;
	uint8_t levelExa = 0, levelExb = 0, levelExa_bak = 0, levelExb_bak = 0;
	uint16_t threshold = 0;
	
	threshold = getModbusHoldingReg(692);

	for(n=0; n<8; n++)
	{
		crack[n*4+0] = (msg->data[n]>>6)&0x03;
		crack[n*4+1] = (msg->data[n]>>4)&0x03;
		crack[n*4+2] = (msg->data[n]>>2)&0x03;
		crack[n*4+3] = msg->data[n]&0x03;
	}

	if((weighs->weigh[ch] > 0.0f)&&(mark == 1)){
		for(n=0; n<32; n++){
			if(crack[n] == 0x02){
				mask++;
				if(bkup_alarm.errTimes[ch][n] < threshold){
					bkup_alarm.errTimes[ch][n] = 0;
				}
			}else if((crack[n] == 0x01) || (crack[n] == 0x00)){
				crack_bak++;
				if(crack[n] == 0x01) isReal = 1;
				else if(crack_bak > EMPTY_TRIGER) {
					rt_kprintf("x\n");
					isReal = 1;
				}
				if(mask >= 2){
					if(n < 16){
						levelExa = 0;
					}else{
						levelExb = 0;
					}
				}
				mask = 0;
				if((getModbusHoldingReg(699+32*ch+n) == 0)&&(getModbusHoldingRegEx(499+32*ch+n) == 0)){ //传感器启用并且连续错误次数小于门限
					if(n < 16){
						levelExa += 1;
					}else{
						levelExb += 1;
					}
				}
				bkup_alarm.errTimes[ch][n] += 1;
			}

			// 连续异常的那一段判断为裂纹等级
			if(levelExa > levelExa_bak){
				levelExa_bak = levelExa;
			}
			if(levelExb > levelExb_bak){
				levelExb_bak = levelExb;
			}

			if(getModbusHoldingReg(699+32*ch+n)){  //传感器禁用
				setModbusHoldingReg(499+32*ch+n, 2);
			}else{
				if(bkup_alarm.errTimes[ch][n] >= threshold){
					bkup_alarm.errTimes[ch][n] = threshold;
					setModbusHoldingReg(499+32*ch+n, 1);
				}else{
					setModbusHoldingReg(499+32*ch+n, 0);
				}
			}
		}
	}

//#	if 1
	if(0 == isReal) return 0;   //没有一次错误敲击 可能是漏敲了信号 判断为正常鸡蛋
//#endif
	
	return (levelExa_bak + levelExb_bak);
}

#define OUT_WEIGH_L 299
#define OUT_WEIGH_H 300
#define OUT_WEIGH_ACTIVE 303

#define HIGH_LEVEL_L 301
#define HIGH_LEVEL_H 302
#define HIGH_LEVEL_ACTIVE 304
void high_crack_level_push_out(Egg_Quality_Info *info)
{
	int n = 0;
	uint16_t p_weigh_sw, p_level_sw;
	uint16_t level_low, level_high;
	uint16_t weigh_low, weigh_high;
	
	level_low = getModbusHoldingReg(HIGH_LEVEL_L);
	level_high = getModbusHoldingReg(HIGH_LEVEL_H);
	p_level_sw = getModbusHoldingReg(HIGH_LEVEL_ACTIVE);
	
	weigh_low = getModbusHoldingReg(OUT_WEIGH_L);
	weigh_high = getModbusHoldingReg(OUT_WEIGH_H);
	p_weigh_sw = getModbusHoldingReg(OUT_WEIGH_ACTIVE);
	
	for(n=0; n<WEIGH_CHANNEL; n++){
		if(info->weight[n] == 0.0){ //没有重量 可能是重启了设备 这时候不需要踢蛋
			continue;
		}
		if(((p_level_sw == 1)&&(info->level[n] >= level_low)&&(info->level[n] <= level_high)) ||
			((p_weigh_sw == 1)&&(info->weight[n] >= weigh_low)&&(info->weight[n] < weigh_high))){  //根据裂纹等级抛弃鸡蛋启用并且裂纹等级超过门限 或者 根据重量抛弃鸡蛋并且重量低于门限
			push_out_control(n, 1);
			push_out_cnt[n] = 1;
			info->level[n] = 0;
			info->weight[n] = 0;
			info->type[n] = 0;
		}
	}
}

void check_push_out_status(void)
{
	int n = 0;
	
	for(n=0; n<WEIGH_CHANNEL; n++)
	{
		if(push_out_cnt[n] > 0){
			push_out_cnt[n]++;
		}
		if(push_out_cnt[n] >= 8){
			push_out_cnt[n] = 0;
			push_out_control(n, 0);
		}
	}
}

void eggs_line_update(void)
{
	int ch = 0, n = 0;
	rt_size_t rx_size = 0;
	Eggs_Line_Weighs new_line;
	struct rt_can_msg txmsg = {0};
	Egg_Quality_Info Egg_q;
	
	high_crack_level_push_out(&Egg_q_vector[UDP_SND_PLUSE-5]);
	
	get_eggs_weigh_online(&new_line.weigh[0]);

	for(n=1; n<MOVE_OUT_PLUSE; n++)
	{
		line_weighs[n-1] = line_weighs[n];
		line_mark[n-1] = line_mark[n];
	}	
	line_weighs[MOVE_OUT_PLUSE-1] = new_line;
	line_mark[MOVE_OUT_PLUSE-1] = 1;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		if(new_line.weigh[ch] > 0.0f){
			txmsg.data[ch] = 0x1;
			total_eggs_found += 1;
		}else{
			txmsg.data[ch] = 0x0;
		}
	}
	txmsg.id = 0x061601c0;
	txmsg.data[6] = 0;
	txmsg.data[7] = 0;

	rt_memset(&crack_level[0], 0, WEIGH_CHANNEL);

	//重置can接收数据状态
	reset_fdcan_read_status();

	//发送一排新蛋的信息 同时查询被溢出那一排蛋的裂纹信息
	if (rt_device_write(fdcan_dev, 0, &txmsg, sizeof(struct rt_can_msg)) != RT_EOK)
  {
      rt_kprintf("can dev write data failed!\n");
  }

	//信号量或者阻塞等待can总线接收数据处理板返回裂纹信息
	rx_size = rt_device_read(fdcan_dev, 0, &rxmsg[0], sizeof(struct rt_can_msg)*8);
//	rt_kprintf("rx_size = %d\n", rx_size);
	if(rx_size > 0){
		for(int n=0; n<WEIGH_CHANNEL; n++){
			if(rxmsg[n].id > 0){ 						//计算裂纹处理结果
				crack_level[n] = calc_crack_level_ex(&rxmsg[n], &line_weighs[0], line_mark[0], n);
				canConnectedErr[n] = 0;
			}else{
				canConnectedErr[n] += 1;
				if(canConnectedErr[n] >= 5){  //can总线通信故障报警
					canConnectedErr[n] = 5;
				}
			}
		}
	}
	
	rt_memset(&Egg_q, 0, sizeof(Egg_Quality_Info));

	Egg_q.command = 1001;
	Egg_q.mesg_id = udp_msg_id;
	udp_msg_id = (udp_msg_id + 1) % UDP_MSG_ID_LEN;
	
	for(ch=0; ch<WEIGH_CHANNEL; ch++){
		if(line_weighs[0].weigh[ch] > 0.0f){
			Egg_q.weight[ch] = line_weighs[0].weigh[ch] * 10;
//				rt_kprintf("egg_weigh: %d.%d\n", Egg_q.weight[ch]/10, Egg_q.weight[ch]%10);
			if(line_mark[0] == 1){
				Egg_q.level[ch] = crack_level[ch];
				if(crack_level[ch] > 0){
					Egg_q.type[ch] = 2;
				}else{
					Egg_q.type[ch] = 1;
				}
			}else{
				Egg_q.type[ch] = 1;
			}
		}
	}

	for(n=1; n<UDP_SND_PLUSE; n++)
	{
		Egg_q_vector[n-1] = Egg_q_vector[n];
	}	
	Egg_q_vector[UDP_SND_PLUSE-1] = Egg_q;

	add_eggs_level_info(&Egg_q_vector[0]);

	setModbusHoldingRegU32(1700, total_eggs_found);
	
	udp_submit_work((rt_ubase_t)&Egg_q_vector[0]);
}

static rt_int32_t encodeVector[20] = {0};
static rt_int16_t encodePos = 0;
static rt_int16_t encodeRecord = 0;

rt_int16_t get_encoder_avg(void)
{
	rt_int32_t totalEncode = 0;
	
	encodeVector[encodePos] = read_pulse_encoder();
	encodePos = (encodePos + 1) % 20;

	if(encodePos == 0){
		for(int i=0; i<20; i++){
			totalEncode += encodeVector[i];
		}
		encodeRecord = totalEncode / 20;
	}

	return encodeRecord;
}

static rt_int32_t total_encoder = 0;
static rt_int8_t total_cnt = 0;
static rt_int8_t step = 0;
int32_t encoder_pulse = 0;
void fdcan_synchronise_encoder(void)
{
		rt_int32_t encode = 0;
		struct rt_can_msg txmsg = {0};
		CrackParam cp;
		
//		encode = get_encoder_avg();
		encode = read_pulse_encoder(); 
		encoder_pulse += encode;
		if(encode < 0) encode = -encode; //增量编码器 如果AB相线接反了 只需要取绝对值
		if(encode < 10)  //如果编码器检测到传输带没有动或者速度太慢 可能是按了急停按键 这时需要清除裂纹传感器上鸡蛋的敲击数据 因为传送带在启停的那段时间敲击的结果不准确
		{
			for(int i=7; i>0; i--){
				line_mark[i] = 0;
			}
		}

		/********此处添加连续计算一段时间内的平均速度*********/
		total_cnt += 1;
		total_encoder += encode;
		if(total_cnt == 20){
//			if(total_encoder < 50){
//				stop_flag = RT_TRUE;
//			}
			step = total_encoder / 20;
			total_encoder = 0;
			total_cnt = 0;
			if(step < ENCODER_SPEED_MIN) step = ENCODER_SPEED_MAX - ENCODER_SPEED_MIN;
			else if(step > ENCODER_SPEED_MAX) step = 0;
			else step = ENCODER_SPEED_MAX - step;
		}

		cp = getCrackSensitivityParam();
//		rt_kprintf("%d %d\n", encode, step);
		/**********此处添加鸡蛋重量因素调整增益倍数**********/

		txmsg.id = 0x061603c0;
		txmsg.data[0] = (encode>>8);
		txmsg.data[1] = (encode&0xff);
		txmsg.data[2] = step;     						//增益设置
		txmsg.data[3] = cp.sensitivity_1;			//灵敏度参数1
		txmsg.data[4] = cp.sensitivity_2;			//灵敏度参数2
		txmsg.data[5] = cp.sensitivity_3;     //灵敏度参数3
		txmsg.data[6] = ((cp.addCnt>>8)&0xff);//禽蛋采样的平均数高8位
		txmsg.data[7] = (cp.addCnt&0xff);			//禽蛋采样的平均数低8位

		if (rt_device_write(fdcan_dev, 0, &txmsg, sizeof(struct rt_can_msg)) != RT_EOK)
    {
        rt_kprintf("can dev write data failed!\n");
    }
}

void fdcan_send_calib_status(void)
{
		struct rt_can_msg txmsg = {0};
		uint16_t channel = 0, status = 0;
		
		memset(txmsg.data, 0, 8);
		channel = getModbusHoldingRegEx(1599);
		status = getModbusHoldingRegEx(1601);
		switch(channel)
		{
			case 0: 
				txmsg.id = 0x061605c1;
				txmsg.data[0] = status;
				break;
			case 1: 
				txmsg.id = 0x061605c2;
				txmsg.data[1] = status;
				break;
			case 2:
				txmsg.id = 0x061605c3;
				txmsg.data[2] = status;
				break;
			case 3:
				txmsg.id = 0x061605c4;
				txmsg.data[3] = status;
				break;
			case 4:
				txmsg.id = 0x061605c5;
				txmsg.data[4] = status;
				break;
			case 5:
				txmsg.id = 0x061605c6;
				txmsg.data[5] = status;
				break;
			default: 
				txmsg.id = 0x061605c1;
				txmsg.data[0] = status;
				break;
		}

		if (rt_device_write(fdcan_dev, 0, &txmsg, sizeof(struct rt_can_msg)) != RT_EOK)
    {
        rt_kprintf("can dev write data failed!\n");
    }
}

void fdcan_get_calib_value(void)
{
		struct rt_can_msg txmsg = {0};
		uint16_t channel = 0;
		static uint8_t sensor_pos = 0;
		
		memset(txmsg.data, 0, 8);
		channel = getModbusHoldingRegEx(1599);
		switch(channel)
		{
			case 0: 
				txmsg.id = 0x061607c1;
				txmsg.data[0] = sensor_pos;
				break;
			case 1: 
				txmsg.id = 0x061607c2;
				txmsg.data[1] = sensor_pos;
				break;
			case 2:
				txmsg.id = 0x061607c3;
				txmsg.data[2] = sensor_pos;
				break;
			case 3:
				txmsg.id = 0x061607c4;
				txmsg.data[3] = sensor_pos;
				break;
			case 4:
				txmsg.id = 0x061607c5;
				txmsg.data[4] = sensor_pos;
				break;
			case 5:
				txmsg.id = 0x061607c6;
				txmsg.data[5] = sensor_pos;
				break;
			default: 
				txmsg.id = 0x061607c1;
				txmsg.data[0] = sensor_pos;
				break;
		}

		if (rt_device_write(fdcan_dev, 0, &txmsg, sizeof(struct rt_can_msg)) != RT_EOK)
    {
        rt_kprintf("can dev write data failed!\n");
    }
		
		sensor_pos = (sensor_pos + 1) % 32;
}

static void fdcan_tx_thread(void *parameter)
{
		rt_ubase_t command = 0;
		
	  while (1)
    {
        if (rt_mb_recv(&mbCan, (rt_ubase_t *)&command, RT_WAITING_FOREVER) == RT_EOK)
        {
//            rt_kprintf("command:%04x\n", command);
						switch(command)
						{
							case 8001: //发送编码器位置
								fdcan_synchronise_encoder();
								rt_thread_mdelay(5);
								check_push_out_status();
								break;
							case 8002: //发送接近开关位置 并接收数字处理板采集的裂纹数据
								eggs_line_update();
								rt_thread_mdelay(5);
								break;
							case 8003: //计算称重校准页面参数
								calc_current_weighing();
								break;
							case 8005: //裂纹传感器校准相关命令
								fdcan_send_calib_status();
								break;
							case 8007: //读取传感器校准值
								fdcan_get_calib_value();
								break;
							default:break;
						}
        }
		}
}

int fdcan_auto_init(void)
{
    rt_err_t res;
	  int m, n;

		/* 初始化邮箱 */
    res = rt_mb_init(&mbCan, "mbCan",  &mbCan_pool[0], sizeof(mbCan_pool) / 4, RT_IPC_FLAG_FIFO);       
    if (res != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }

		//开机发送称重传感器到裂纹模块输出这段的假数据 防止主控这段时间收不到数据而产生通信故障报警
		for(n=0; n<UDP_SND_PLUSE; n++){
			Egg_q_vector[n].command = 1001; 
			Egg_q_vector[n].mesg_id = udp_msg_id;
			udp_msg_id = (udp_msg_id + 1) % UDP_MSG_ID_LEN;
			for(m=0; m<6; m++){
				Egg_q_vector[n].level[m] = 0;
				Egg_q_vector[n].type[m] = 0;
				Egg_q_vector[n].weight[m] = 0;
			}
		}
		
		/* 初始化备份SRAM */
		BackupSRAM_config();
		
		/* 读取安全码 如果电池掉电了 清除历史记录 */
		
    /* 查找 CAN 设备 */
    fdcan_dev = rt_device_find(FDCAN_DEV_NAME);
    if (!fdcan_dev)
    {
        rt_kprintf("find %s failed!\n", FDCAN_DEV_NAME);
        return RT_ERROR;
    }

    /* 打开 CAN 设备 */
    res = rt_device_open(fdcan_dev, RT_DEVICE_OFLAG_RDWR);
    RT_ASSERT(res == RT_EOK);

    /* 创建数据收发线程 */
		rt_thread_t thread;

    thread = rt_thread_create("fdcan_tx", fdcan_tx_thread, RT_NULL, 1024, 10, 10);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        rt_kprintf("create can_rx thread failed!\n");
    }

    return res;
}
