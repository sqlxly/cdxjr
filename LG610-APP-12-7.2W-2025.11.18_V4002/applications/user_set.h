
#include <rtthread.h>

#ifndef _USER_SET_H__
#define _USER_SET_H__


//时间信息
typedef struct
{   
	unsigned char year;
	unsigned char month;
	unsigned char date;
  unsigned char hour;
	unsigned char min;
	unsigned char sec;
}rtc_info_t;
typedef struct
{
	uint16_t reconnect_timeou_number;
	uint16_t tcp_alarm_count;
	uint16_t servo_moto_connect_number;
}communication_parameters ;


// 踢蛋点击是指电磁阀还是伸缩杆
// 蛋杯错误是指接收爪那个蛋杯吗
typedef struct
{
	uint16_t tick_egg_moto_err_count;
  uint16_t egg_cup_err_count;
	uint16_t dd;
	uint32_t crc_32;
}mix_device_part;

typedef struct
{
	uint16_t line1;																		// qianxuping X通道的第1条包装通道的蛋杯位置(脉冲数)
	uint16_t line2;																		// qianxuping X通道的第2条包装通道的蛋杯位置(脉冲数)
	uint16_t line3;																		// qianxuping X通道的第3条包装通道的蛋杯位置(脉冲数)
	uint16_t line4;																		// qianxuping X通道的第4条包装通道的蛋杯位置(脉冲数)
	uint16_t line5;																		// qianxuping X通道的第5条包装通道的蛋杯位置(脉冲数)
	uint16_t line6;																		// qianxuping X通道的第6条包装通道的蛋杯位置(脉冲数)
  uint16_t weight_device;														// qianxuping X通道的称重设备的位置(脉冲数)
	uint16_t ai_divice;																// qianxuping X通道的ai(指什么呢或者包含什么呢)设备的位置(脉冲数)
	uint16_t print_device;														// qianxuping X通道的喷码设备的位置(脉冲数)
	uint16_t blood_device;														// qianxuping X通道的血斑设备的位置(脉冲数)
	uint16_t first_cup_positon;												// qianxuping X通道的分级线第一个蛋杯的位置(脉冲数)
	uint16_t empty_cup_check;													// qianxuping X通道的空蛋杯的位置(脉冲数)
	uint16_t cup_check;       //蛋杯监测（未关闭）		// qianxuping X通道的蛋杯关闭检测的位置(脉冲数)
	uint16_t totle_cup;																// qianxuping X通道的总蛋杯数量的位置(脉冲数)
	uint16_t throw_line1;   //抛弃通道1								// qianxuping X通道的抛弃通道设备1的位置(脉冲数)
	uint16_t throw_line2;   //抛弃通道2								// qianxuping X通道的抛弃通道设备2的位置(脉冲数)
	uint16_t throw_line3;   //抛弃通道3								// qianxuping X通道的抛弃通道设备3的位置(脉冲数)
	uint16_t no_use1;
	uint16_t no_use2;
}offest_position;

typedef struct
{
	offest_position chA;
	offest_position chB;
	uint32_t crc_32;
}device_offest_position;

// 这个结构体是哪些板子(功能模块)有没有使能或者使用？
typedef struct
{
uint16_t packge_ch1;   //包装通道1									// qianxuping X通道的包装通道1是否安装
uint16_t packge_ch2; 																// qianxuping X通道的包装通道2是否安装
uint16_t packge_ch3; 																// qianxuping X通道的包装通道3是否安装
uint16_t packge_ch4; 																// qianxuping X通道的包装通道4是否安装
uint16_t packge_ch5; 																// qianxuping X通道的包装通道5是否安装
uint16_t packge_ch6; 																// qianxuping X通道的包装通道6是否安装		
uint16_t weight_samp;																// qianxuping X通道的称重设备是否安装
uint16_t crack_check;																// qianxuping X通道的裂纹检测设备是否安装
uint16_t dirty_recognize;														// qianxuping X通道的脏蛋检测设备是否安装
uint16_t colour_recognize;													// qianxuping X通道的蛋颜色识别设备是否安装
uint16_t blood_recognize;														// qianxuping X通道的血斑识别设备是否安装
uint16_t print_machine;															// qianxuping X通道的喷码设备是否安装
uint16_t throw_ch;																	// qianxuping X通道的抛弃通道设备是否安装
}mode_use_info;
// 说明除以上是A，B线不一样，其他像喷油，清洗等是开的话，都一起开的
// 设备安装
typedef struct
{
	mode_use_info chA;
  mode_use_info chB;
	uint16_t oil_system;         //喷油
	uint16_t uv_lamp;      //紫外杀菌
	uint16_t human_eye_recognize;   //人工肉眼识别
	uint16_t dry_system;            //风干
	uint16_t auto_add_medicine;     //自动加药
	uint16_t wash_system;       //清洗系统
}Device_use_set;
// 设备使能
// 逻辑是设备先要安装，然后才能使能启用
typedef struct
{
	uint16_t low_medicine_alarm;  //药位报警
	uint16_t oil_print;           //喷油
	uint16_t uv_lamp;         //紫外
	uint16_t dry_brush;       //干燥毛刷
	uint16_t dry_fan;  //干燥风机
	uint16_t wash_fan;     //清洗鼓风机
  uint16_t wash_brush;    //清洗毛刷
	uint16_t add_medicine;  //清洗加药
	uint16_t wash_moto;    //清洗抽水机
	uint16_t absorb_oil;   //吸油
	uint16_t light_lamp;   //照明灯
	uint16_t check_tray;   //托盘检测
}mode_en_type;
typedef struct
{
	uint16_t ch1_nb;          //每个脉冲代表X个鸡蛋 		// qianxuping 这个是原来有个手动来计坏蛋的功能，可能没有1个1个的计，是比如10个蛋记一次，计的时候转一下按钮产生一个脉冲，这个脉冲表示10个蛋？
	uint16_t ch2_nb;          //每个脉冲代表X个鸡蛋 
	uint16_t ch3_nb;          //每个脉冲代表X个鸡蛋
	uint16_t ch1_use;         //启停标记								// qianxuping 手动计蛋功能是否启动？
	uint16_t ch2_use;         //启停标记
	uint16_t ch3_use;         //启停标记
	uint16_t heap_delay;     //堆积延时									// qianxuping 中央集蛋堆积停止延迟
	uint16_t unheap_delay;   //缓解堆积延时							// qianxuping 中央集蛋缓解启动延迟
	uint16_t wash_delay;  //清洗延时										// qianxuping 清洗系统停机延迟
	uint16_t dry_delay;   //风干延时										// qianxuping 风干系统停机延迟
	uint16_t light_delay; //停止光检照明延时						// qianxuping 人工光检停机延迟(有些界面没有人工光检，该位置是清洗机器)
	uint16_t uv_lamp_delay;   //紫外灯关闭延时					// qianxuping 杀菌系统停机延迟
	uint16_t absorb_oil_delay;      //吸油机延时				// qianxuping 喷油系统停机延迟	
	uint16_t print_oil_delay;   //喷油延时							// qianxuping 喷码系统停机延迟
}transport_set_type;

typedef struct
{
	uint16_t device_open;
	uint16_t device_fault;
	uint16_t device_stop;
}voice_type;

typedef struct
{
	uint16_t unnormal_stop_count;     //异常停机数量
	uint16_t leve1;    //大到小
	uint16_t leve2;
	uint16_t leve3;
	uint16_t leve4;
	uint16_t leve5;
	uint16_t leve6;
	uint16_t leve7;
	uint16_t leve8;
	uint16_t leve9;
	uint16_t leve10;
}weight_leve_type;   // 鸡蛋等级信息

typedef struct
{
	uint16_t dat[8];    //破口
	
}colour_leve_type;   

typedef struct
{
  uint16_t dat[6]; 
}drty_leve_type;   // 鸡蛋等级信息

typedef struct
{
	uint16_t dat[8];    //破口
}crack_leve_type;   // 鸡蛋等级信息

typedef struct 
{
	voice_type voice;
	Device_use_set device_use_info;
	transport_set_type transport;
	communication_parameters commun_part;
	weight_leve_type weight_leve;       //鸡蛋等级信息
	colour_leve_type colour_leve;      // 
	drty_leve_type   drty_leve;      //脏蛋等级 //优 良 差
	crack_leve_type  crack_leve;
	mode_en_type mode_en;    //模块使能
	uint32_t crc_32;
}user_set_part_type;


//运行参数
typedef struct
{   
	uint8_t state;
	uint8_t frequency_speed;      //变频器速度   0-100						// qianxuping 这个是设置的变频器的速度？
	uint16_t transport_encode_speed;  //传输线编码器速度   m/s		// qianxuping 这个是读取到的传输线编码器速度？
	uint16_t separate_encode_speed;   //分级线编码器速度   m/s		// qianxuping 这个是读取到的分级线编码器速度？
}run_dat_type;


void get_senser_offest(device_offest_position *dat);
void save_device_offest_to_eep(device_offest_position *dat );

void get_mix_device_part(mix_device_part *dat);
void save_mix_device_part(mix_device_part *dat);

void get_run_dat(run_dat_type *dat);
void get_local_time(rtc_info_t *ptime);
void get_user_set_dat(user_set_part_type *buf);
void user_dat_init(void);
uint8_t get_packge_ch_config(uint8_t id);
void get_user_set_point(user_set_part_type **point);
void get_user_set_part(user_set_part_type *dat);
rt_err_t read_user_set_from_eep(user_set_part_type *buf);
void save_user_set_to_eep(user_set_part_type *dat);

void read_egg_cup_err_count(uint8_t ch,uint16_t *dat,uint16_t len);
void save_egg_cup_err_count(uint8_t ch,uint16_t dat,uint16_t index);  //从0 开始 蛋杯号数
	
#endif  	// _USER_SET_H__
