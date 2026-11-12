
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "egg_control.h"
#include "screen_chat.h"
#include "app_screen.h"
#include "mail_box.h"
#include "uadc.h"
#include "common.h"
#include "mdebug.h"
#include "E6B2.h"
#include <user_set.h>
#include "move_control.h"
#include "product_control.h"
#include "product_set.h"
#include "device_alarm.h"
#include "drv_key.h"
#include "spt_egg.h"
#include "udp_weight.h"
#include "dat_backup.h"
#include "udac.h"
#include "relay.h"
#include "modbus_packge.h"
#include "modbus_transport.h"
#include "modbus_dirty.h"
#include "err_check.h"
#include "product_egg_recode.h"
#include "app_io_control.h"
#include "sync_egginfo.h"

#define forward_egg_len 3
#define back_egg_len 2
ALIGN(RT_ALIGN_SIZE)

# define STP_ABNORMAL_SPEED 300  //低于这个 300mm/s 都是不正常


static uint16_t weightA_offset_count=0;
static uint16_t weightB_offset_count=0;

static uint16_t crackA_offset_count=0;
static uint16_t crackB_offset_count=0;

//static uint16_t dirty_offest_count=0;
static uint16_t dirtyA_offest_count=0;
static uint16_t dirtyB_offest_count=0;

static uint16_t dirty_and_weight_sync_count=0;

static uint32_t recive_cup_empty_id[12]={0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234,0x1234};
static uint32_t servo_down_pluse[12];
static	uint16_t throw_cha_max_id;
static	uint16_t throw_cha_min_id;
static	uint16_t throw_chb_max_id;
static	uint16_t throw_chb_min_id;

//static uint16_t EGG_RECIVE_LINE1_START_POSTION;   //5010  //2269//   //编码器脉冲数  接收爪通道1——>原点位置   比如刚好100圈  =1000*100 
//static uint16_t EGG_RECIVE_LINE2_START_POSTION;    //6425    //编码器脉冲数 
//static uint16_t EGG_RECIVE_LINE3_START_POSTION;    //9005     //编码器脉冲数 
//static uint16_t EGG_RECIVE_LINE4_START_POSTION;    //10420    //编码器脉冲数    
//static uint16_t EGG_RECIVE_LINE5_START_POSTION;    //13000                                    //编码器脉冲数 
//static uint16_t EGG_RECIVE_LINE6_START_POSTION;    //14415     //编码器脉冲数 

static uint16_t THROW_LINEA1_POSTION         =      16320;
static uint16_t THROW_LINEA2_POSTION         =      16550;
static uint16_t THROW_LINEA3_POSTION         =      16780;

static uint16_t THROW_LINEB1_POSTION         =      16320;
static uint16_t THROW_LINEB2_POSTION         =      16550;
static uint16_t THROW_LINEB3_POSTION         =      16780;

uint32_t EMPTY_CUP_SENSER_OFFEST_PLUSEA =17085;
uint32_t EMPTY_CUP_SENSER_OFFEST_PLUSEB =17085;

typedef struct
{
	uint32_t ch[MAX_RECIVE_CUP_NUMB];
}ahead_info_type;	// 记录每个通道的6个接收抓是否有蛋？

// 这个是这么理解
// 接收爪5-0,6个，但是托盘可能不是6个
// 6(5*6)个时候，max_ch=5，min_ch=0
// 3(2*3)个时候，max_ch=5，min_ch=3；也有可能是max_ch=3，min_ch=1等，也有可能是max_ch=2，min_ch=0等，看包装的位置
typedef struct
{
	uint8_t kick_ch_pos; // 当前应该踢哪个通道(比如2*3总共3个格子蛋托，就要设置0-5电磁铁中哪些来提，比如2-4，然后kick_ch_pos指2-4中的哪个电磁铁踢)
	uint8_t min_ch;     // 用的通道的最小编号(0-5这6个电磁铁用的最小的是哪个，比如2*3总共3个格子蛋托，就要设置0-5电磁铁中哪些来提，比如2-4)
	uint8_t max_ch;     // 用的通道的最大编号(0-5这6个电磁铁用的最大的是哪个，比如2*3总共3个格子蛋托，就要设置0-5电磁铁中哪些来提，比如2-4)
}kick_egg_type;


// 这个没有使用
typedef struct
{
	uint8_t kick_ch_pos; // 当前应该踢哪个通道
	uint8_t min_ch;     // 根据包装盒 列数来定
	uint8_t max_ch;     // 根据包装盒 列数来定
}low_speed_egg_type;


typedef struct
{
	uint8_t egg_count[12];
	uint8_t line[12];// 12 条线
	uint8_t max_eggs;
	uint8_t line_pos;
}Avg_product_info;

typedef struct
{
	Avg_product_info product_chA;  
	Avg_product_info product_chB;  
}avg_product_line_info;

typedef struct
{
	uint8_t min_id;           // 
	uint8_t max_id;         // 
  uint16_t print_id;           // 
	uint16_t egg_id;         // 
	uint16_t PRINT_PRITION;
}Print_egg_info;	

typedef struct
{
  Print_egg_info cha;
	Print_egg_info chb;
}Print_device_info;

alread_production_type alread_production;		// 记录所有通道的已经生产的记录信息

uint8_t save_analisys_egg_recode_index=0xff;

static separte_egg_type spt_eg;     //分离线鸡蛋数据
static uint8_t message_buf[MESSAGE_DAT_LEN]; 
static ahead_info_type throw_ahead_info[MAX_SEPARATE_LINE];	// 存着每个通道的每个蛋杯的脉冲值
static uint16_t throw_egg_id[MAX_SEPARATE_LINE][2];         //记录要踢蛋的开始和结束的蛋杯id(即算出某个通道开始踢蛋例如为100，但是不能按照100来到在踢，要踢例如98-101这几个蛋杯，因为避免漏踢?)																		 
static production_type per_poduction_egg;                  //要生产产品的参数

// A传输通道设备发过来的蛋的信息
uint8_t transport_egg_dirty_infoA[MAX_TRANSPORT_LINE_COUNT][6];
static uint8_t transport_egg_crack_infoA[MAX_TRANSPORT_LINE_COUNT][6];
static uint8_t transport_egg_colour_infoA[MAX_TRANSPORT_LINE_COUNT][6];
static uint16_t transport_egg_weight_infoA[MAX_TRANSPORT_LINE_COUNT][6];
// B传输通道设备发过来的蛋的信息
uint8_t transport_egg_dirty_infoB[MAX_TRANSPORT_LINE_COUNT][6];
static uint8_t transport_egg_crack_infoB[MAX_TRANSPORT_LINE_COUNT][6];
static uint8_t transport_egg_colour_infoB[MAX_TRANSPORT_LINE_COUNT][6];
static uint16_t transport_egg_weight_infoB[MAX_TRANSPORT_LINE_COUNT][6];


static Print_device_info print_device_info;
static avg_product_line_info avg_product_line_recode[12];   

static uint32_t load_egg_count=0;              //记录 传送带移动的格子数量  传送线转到 分离线用

static kick_egg_type recode_kick_egg[12]={5,5,5,5,5,5,5,5,5,5,5,5};   // 记录每个通道用了电磁铁0-5中哪些电磁铁，以及当前用的哪个电磁铁踢的
static uint32_t start_throw_dist[MAX_SEPARATE_LINE]={0,0,0,0,0,0,0,0,0,0,0,0}; // 每个通道开始踢蛋的第一个蛋杯的脉冲位置
																					 
// 清除保存蛋品分析的记录索引
void clear_save_analisys_egg_recode_index()
{
	save_analisys_egg_recode_index =0xff;
}

void show_tick_egg_ch()
{

		DEBUG_CARE("tick egg ch=%d %d %d %d %d %d\n",recode_kick_egg[0].kick_ch_pos
	 ,recode_kick_egg[1].kick_ch_pos,recode_kick_egg[2].kick_ch_pos,recode_kick_egg[3].kick_ch_pos,recode_kick_egg[4].kick_ch_pos
	,recode_kick_egg[5].kick_ch_pos);
}
//计算每一通道 要踢蛋的实际偏移 ID

extern void init_recive_crack_delay_time(uint32_t pluse);

// 初始化每个通道开始踢蛋的第一个蛋杯的脉冲位置
void init_packge_line_start_position()
{
  device_offest_position position;
  get_senser_offest(&position);
	
	
  //clacute_blood_offest(position.boody_position);	
	
		// 每个包装通道通道开始踢蛋的第一个蛋杯的脉冲位置
      start_throw_dist[packge_map_ch_id[0]] = position.chA.line1;
	  start_throw_dist[packge_map_ch_id[1]] = position.chB.line1;
	  start_throw_dist[packge_map_ch_id[2]] = position.chA.line2;
	  start_throw_dist[packge_map_ch_id[3]] = position.chB.line2;
	  start_throw_dist[packge_map_ch_id[4]] = position.chA.line3;
	  start_throw_dist[packge_map_ch_id[5]] = position.chB.line3;
	  start_throw_dist[packge_map_ch_id[6]] = position.chA.line4;
	  start_throw_dist[packge_map_ch_id[7]] = position.chB.line4;
	
	  start_throw_dist[packge_map_ch_id[8]] = position.chA.line5;
	  start_throw_dist[packge_map_ch_id[9]] = position.chB.line5;
	  start_throw_dist[packge_map_ch_id[10]] = position.chA.line6;
	  start_throw_dist[packge_map_ch_id[11]] = position.chB.line6;
	
		// 抛弃通道开始踢蛋的脉冲位置
    THROW_LINEA1_POSTION = position.chA.throw_line1;
	  THROW_LINEA2_POSTION = position.chA.throw_line2;
	  THROW_LINEA3_POSTION = position.chA.throw_line3;
	
	  THROW_LINEB1_POSTION = position.chB.throw_line1;
	  THROW_LINEB2_POSTION = position.chB.throw_line2;
	  THROW_LINEB3_POSTION = position.chB.throw_line3;
	
	//EGG_PRINT_POSITION  = position.print_senser;
		// 每条分级线的蛋杯数量
	  SEPARATE_CHA_CUP_NUMBER = position.chA.totle_cup;
	  SEPARATE_CHB_CUP_NUMBER = position.chB.totle_cup;
		// 脏蛋，重量以及裂纹的延迟时间，这个延迟时间是指对应设备发数据后，蛋落到分级线的时间吗，如果是这个时间就和各个发送信息的位置到分级线的距离以及每条线的速度有关
	  init_recive_dirty_delay_time(position.chA.ai_divice*10);	// 这个在第一个蛋杯的时候，屏幕上设置的脉冲值，这里乘以10是什么意思
																															// 或者说计算这个延迟脉冲的依据是什么
																															// 或者说为什么要延迟一段时间才记录收到蛋的信息呢？
    init_recive_weight_delay_time(position.chA.weight_device*10);
		init_recive_crack_delay_time(position.chA.blood_device*10);
		
	//EMPTY_CUP_SENSER_OFFEST_PLUSE = position.check_empty_cup;
		// 空杯检测的位置
	  EMPTY_CUP_SENSER_OFFEST_PLUSEA = position.chA.empty_cup_check;
	  EMPTY_CUP_SENSER_OFFEST_PLUSEB = position.chB.empty_cup_check;
//	dirty_offest_count =  position.dirty_device/1000;

	// 或者说不是格子数，就是理解为用这么多缓存来存这些数据
	// 因为都以当前位置为参考点
	// 也即称重的weightA_offset_count和裂纹的crackA_offset_count和脏蛋的dirtyA_offest_count对于主控来说是一个位置
	// 也即称重从发送信号走weightA_offset_count这么多个数据
	// 裂纹从发送信号走crackA_offset_count这么多个数据
	// 脏蛋从发送信号走dirtyA_offest_count这么多个数据
	// 都刚好都到同一个位置也即本身就是同一排鸡蛋的数据？
	
	// 重量到分级线要走的格子数量？
    weightA_offset_count = position.chA.weight_device/100;
	  weightB_offset_count = position.chB.weight_device/100;
		// 裂纹到分级线要走的格子数量？
		crackA_offset_count = position.chA.blood_device/100;
		crackB_offset_count = position.chB.blood_device/100;
	// 脏蛋到分级线要走的格子数量？
		dirtyA_offest_count = position.chA.ai_divice/100;
		dirtyB_offest_count = position.chB.ai_divice/100;

}
// 计算每个通道第一个蛋杯的蛋杯号
// 计算每个通道的喷码的蛋杯号
// 这个蛋杯号和速度没有关系，只和开始设置的脉冲位置有关系

// 计算每个大通道踢的蛋杯的范围(这个蛋杯往前以及往后多查了几个蛋杯)
// 计算了每条线喷码的范围
static void calculate_tick_egg_id()
{
	uint8_t ch;
	uint32_t spt_start_len;
	uint32_t spt_end_len;
	device_offest_position position;
  get_senser_offest(&position);
	
	// 计算每个通道的第一个蛋杯的蛋杯号(为了避免漏查，使用了一个范围)
	for(ch=0;ch<MAX_SEPARATE_LINE;ch++)
	{
		spt_start_len = start_throw_dist[ch];		// 某个通道的第一个蛋杯的位置(脉冲值)
		spt_end_len = spt_start_len + SEPARATE_RECIVE_CUP_LEN*MAX_RECIVE_CUP_NUMB;	// 某个通道的最后一个蛋杯的位置(脉冲值)(开始位置+多少个接收抓*每个接收抓距离的脉冲值)
		
		// back_egg_len 这个是个数，不是脉冲值
		// throw_egg_id[ch][0] 这个里面保存着该通道踢蛋的蛋杯id范围(即算出某个通道开始踢蛋例如为100，但是不能按照100来到在踢，要踢例如98-101这几个蛋杯，因为避免漏踢?)
		throw_egg_id[ch][0] = (spt_start_len/SEPARATE_CUP_LEN)-back_egg_len;                  //向前多查一个蛋杯  //向后多查2个蛋杯 避免漏查
		
		// forward_egg_len 这个是个数，不是脉冲值
		throw_egg_id[ch][1] = (spt_end_len/SEPARATE_CUP_LEN)+forward_egg_len;
		rt_kprintf("通道ID=%d 检测蛋杯起始ID=%d 终止ID=%d\n",ch,throw_egg_id[ch][0],throw_egg_id[ch][1]);       
		
		// throw_egg_id[x][0] 里面存储的是某个通道的开始的蛋杯id
		// throw_egg_id[x][1] 里面存储的是某个通道的结束的蛋杯id
	}	
  
	// 这个计算喷码的时候蛋杯id的范围
	rt_memset(&print_device_info,0,sizeof(Print_device_info));
	print_device_info.cha.egg_id=0xffff;
	print_device_info.chb.egg_id=0xffff;
	print_device_info.cha.print_id =0xffff;
	print_device_info.chb.print_id =0xffff;
	print_device_info.cha.PRINT_PRITION = position.chA.print_device;
	print_device_info.chb.PRINT_PRITION = position.chB.print_device;
	// 计算喷码的蛋杯号(为了避免漏查，使用了一个范围)
	print_device_info.cha.min_id= ((position.chA.print_device/SEPARATE_CUP_LEN)-3);
  print_device_info.cha.max_id= ((position.chA.print_device/SEPARATE_CUP_LEN)+2);
  print_device_info.chb.min_id= ((position.chB.print_device/SEPARATE_CUP_LEN)-3);
  print_device_info.chb.max_id= ((position.chB.print_device/SEPARATE_CUP_LEN)+2);
	//rt_kprintf("chA P=%d chB P=%d\n",position.chA.print_device,position.chB.print_device);
	//rt_kprintf("ID_A min_id=%d ID_A max_id=%d\n",print_device_info.cha.min_id,print_device_info.cha.max_id);
	//rt_kprintf("ID_B min_id=%d ID_B max_id=%d\n",print_device_info.chb.min_id,print_device_info.chb.max_id);
	
}
// 计算提前踢蛋距离   通道ID   几号杯
// 这个计数每个通道里面的每个蛋杯提前踢的距离脉冲值
// 因为一个接收爪的长度是SEPARATE_RECIVE_CUP_LEN，相当于每个通道的各个蛋杯的脉冲值依次增加SEPARATE_RECIVE_CUP_LEN

// 计算每个通道的所有接收爪的脉冲值
static inline void  calculate_throw_ahead_dist()
{
		uint8_t i,j;
    uint32_t dist;

		for(i=0;i<MAX_SEPARATE_LINE;i++)
	   {
			dist = start_throw_dist[i];         //提前的实际距离
			for(j=0;j<MAX_RECIVE_CUP_NUMB;j++)
			{
				throw_ahead_info[i].ch[j]= dist + j*SEPARATE_RECIVE_CUP_LEN;  //每通道提前距离
			//	rt_kprintf("line=%d ch=%d offest=%d\n",i,j,throw_ahead_info[i].ch[j]);
			}
		}
}
// 计算每个通道的每个接收爪的编码器脉冲值
// 计算每个通道第一个接收爪的对应的分级线的蛋杯号id(是个范围)
void reinit_position()
{
	
  calculate_throw_ahead_dist();
  calculate_tick_egg_id();
}
// 分离通道蛋杯数据初始化
void spt_egg_info_init()
{
	rt_memset(&spt_eg,0,sizeof(spt_eg));
	rt_kprintf("鸡蛋数据结构=%d 单个结构=%d\n",sizeof(spt_eg),sizeof(separate_egg_info_type));
	rt_memset(&message_buf,0,sizeof(message_buf));
	rt_memset(&throw_ahead_info,0,sizeof(ahead_info_type)*MAX_SEPARATE_LINE);
	rt_memset(&transport_egg_dirty_infoA,0,sizeof(transport_egg_dirty_infoA));
	rt_memset(&transport_egg_crack_infoA,0,sizeof(transport_egg_crack_infoA));
	rt_memset(&transport_egg_colour_infoA,0,sizeof(transport_egg_colour_infoA));
	rt_memset(&transport_egg_weight_infoA,0,sizeof(transport_egg_weight_infoA));
	
	rt_memset(&transport_egg_dirty_infoB,0,sizeof(transport_egg_dirty_infoB));
	rt_memset(&transport_egg_crack_infoB,0,sizeof(transport_egg_crack_infoB));
	rt_memset(&transport_egg_colour_infoB,0,sizeof(transport_egg_colour_infoB));
	rt_memset(&transport_egg_weight_infoB,0,sizeof(transport_egg_weight_infoB));
	
	init_packge_line_start_position();	// 初始化各个位置的脉冲位置
	calculate_tick_egg_id();						// 根据各个位置的脉冲值算出各个位置对应的分级线的蛋杯号id
	calculate_throw_ahead_dist();				// 计算每个通道的接收爪的脉冲值
}



//设置基于原点偏移X鸡蛋的动态数据  分离通道
// ch 0-1
// offeset	0-5
static void set_spt_egg_info(uint8_t ch,uint16_t offeset,separate_egg_info_type *dat)
{
	separate_egg_info_type *addr;
	uint16_t id;
	
	if(ch==0)
	{
		// 传入的offeset：0 1 2 3 4 5
		// spt_eg.cha_pos 0-269
		
		// 270/6=45
		// 当前spt_eg.cha_pos是第x个6的位置，则存储的位置为第45-x个6的位置
		// 当前spt_eg.cha_pos是第0也即第45个个6的位置，则存储的位置为第0个6的位置(这个比较特殊)
		// 相当于存的位置是先存第一个6个位置
		// 在存倒数第1个6个位置
		// 在存倒数第2个6个位置
		// 在存倒数第3个6个位置
		// 在存倒数第4个6个位置
		// 依次类推
		
		// 相当于从后面依次往前面存
		// 也就是最后面的是先来的数据，也就是是最老的数据
		// 还是有问题，好像不循环呀，不循环取的时候不就乱套了吗，不就不是先来的先取吗
		
		// spt_eg.cha_pos 0
		// id=(0,1,2,3,4,5)-0=0,1,2,3,4,5(就将数据放在这些地址上)
		
		// spt_eg.cha_pos 6
		// id=270-(6-(0,1,2,3,4,5))=264,265,266,267,268,269(就将数据放在这些地址上)
		
		// spt_eg.cha_pos 12
		// id=270-(12-(0,1,2,3,4,5))=258,259,260,261,262,263(就将数据放在这些地址上)
		
		// spt_eg.cha_pos 96
		// id=270-(96-(0,1,2,3,4,5))=174,175,176,177,178,179(就将数据放在这些地址上)
		
		// spt_eg.cha_pos 264
		// id=270-(264-(0,1,2,3,4,5))=6,7,8,9,10,11(就将数据放在这些地址上)
		
		
		// 通过分析 offeset 指的是当前蛋在这6个蛋当中的偏移，而且传入的只能是 0-5
	  if(offeset>=spt_eg.cha_pos)
	  {
		id = offeset-spt_eg.cha_pos;
	  }
	  else 
	  {
		id = SEPARATE_CHA_CUP_NUMBER-(spt_eg.cha_pos-offeset);
	  }
	
	  addr=&spt_eg.cha_info[id];
	  rt_memcpy(addr,dat,sizeof(separate_egg_info_type));
	}
	else 
	{
	  if(offeset>=spt_eg.chb_pos)
	  {
		id = offeset-spt_eg.chb_pos;
	  }
	  else 
	  {
		id = SEPARATE_CHB_CUP_NUMBER-(spt_eg.chb_pos-offeset);
	  }
	
	  addr=&spt_eg.chb_info[id];
	  rt_memcpy(addr,dat,sizeof(separate_egg_info_type));
	}
	//rt_kprintf("id=%d pluse=%d\n",dat->id,dat->pluse);
}




// 设置该蛋的踢蛋信息
// 包括踢蛋状态，打开时间，哪个踢蛋通道踢
static void recode_tick_egg_info_down(uint8_t ch,uint16_t offeset,uint16_t cup_open_time,uint8_t tick_ch,uint8_t state)
{
	separate_egg_info_type *addr;
	
	if(ch==0)
	{
	   if(offeset>=spt_eg.cha_pos)
		{
			addr=&spt_eg.cha_info[offeset-spt_eg.cha_pos];
		}
		else 
		{
			addr=&spt_eg.cha_info[SEPARATE_CHA_CUP_NUMBER-(spt_eg.cha_pos-offeset)];
		}
		addr->empty = state;
		addr->cup_open_time = cup_open_time;
		addr->tick_ch = tick_ch;
		addr->tick_time = rt_tick_get();
	}
	else 
	{
	   if(offeset>=spt_eg.chb_pos)
		{
			addr=&spt_eg.chb_info[offeset-spt_eg.chb_pos];
		}
		else 
		{
			addr=&spt_eg.chb_info[SEPARATE_CHB_CUP_NUMBER-(spt_eg.chb_pos-offeset)];
		}
		addr->empty = state;
		addr->cup_open_time = cup_open_time;
		addr->tick_ch = tick_ch;
		addr->tick_time = rt_tick_get();
	}
	
   
	//rt_memset(addr,0,sizeof(separate_egg_info_type));
}
//获取基于原点偏移X的动态数据
void get_pst_egg_info(uint8_t ch,uint16_t offeset,separate_egg_info_type **dat,uint8_t debug)
{
	uint16_t id;
	
	if(ch==0)
	{
		// offeset 200
		// spt_eg.cha_pos 0-269
		
		// offeset 200
		// spt_eg.cha_pos 0
		// id 200
		// offeset 200
		// spt_eg.cha_pos 10
		// id 190
		// offeset 200
		// spt_eg.cha_pos 20
		// id 180
		// offeset 200
		// spt_eg.cha_pos 200
		// id 0
		
		// offeset 200
		// spt_eg.cha_pos 201
		// id 270-201+200=269
		// offeset 200
		// spt_eg.cha_pos 260
		// id 270-260+200=210
		// offeset 200
		// spt_eg.cha_pos 269
		// id 270-269+200=201
		
		// 通过以上分析取的数据是先从200往0取，然后在从261往201取？
		
		// 存的时候又是0-5,264-269,258-263这样存的呀
		
		if(offeset>=spt_eg.cha_pos)
		{
			id = offeset-spt_eg.cha_pos;
		}
		else 
		{
			id= SEPARATE_CHA_CUP_NUMBER-(spt_eg.cha_pos-offeset);
		}
		*dat = &spt_eg.cha_info[id];
	}
	else
	{
		if(offeset>=spt_eg.chb_pos)
		{
			id = offeset-spt_eg.chb_pos;
		}
		else 
		{
			id= SEPARATE_CHB_CUP_NUMBER-(spt_eg.chb_pos-offeset);
		}
		
	  *dat = &spt_eg.chb_info[id];
	}
	if(debug)
	{
		//rt_kprintf("get dat id =%d ch=%d\n",id,spt_eg.info[id].separate_ch);
	}
}




//根基生产需求，计算鸡蛋流经哪个通道






uint32_t check_empty_cup_and_cup_move()
{
	uint32_t pluse;
	uint32_t cpluse;
	uint32_t pos;
	uint32_t posA;
    uint32_t posB;
	int16_t move;

	// 获取分级线的编码器脉冲值
	pluse = get_spt_encode_pluse();
	cpluse = pluse;
	pos = (pluse/1000);	// 走了多少圈
	pos = pos*6;				// 一圈走6个蛋杯，得到一整圈的共走了多少个蛋杯
	pluse = pluse%1000;	// 这个得到不到一圈的脉冲数
	pos = pos +(pluse/SEPARATE_CUP_LEN); // 整圈数走的蛋杯数加上不足一圈走的蛋杯数得到一共走了多少个蛋杯(相对于原点)
	posA = pos %SEPARATE_CHA_CUP_NUMBER;	// 其实可以不用这一步，也可以使用这一步
																				// 这一步这么理解，比如整个分级线实际的蛋杯数是360，那么算到的pos这个值肯定在0-359之间
																				// 如果在以270求模，这个数字就会限制在0-269之间，那么270-359之间就会和前面的0-80重了
																				// 但是重了没有关系，因为又从0开始了
	posB = pos %SEPARATE_CHB_CUP_NUMBER;
	
	move = posA - spt_eg.cha_pos;	// posA这个是蛋杯走了的时候个数的编号
																// 举个例子，开始的时候spt_eg.cha_pos是0
																// 分级线走，然后得到走个posA个蛋杯
																// 走的这个蛋杯数量有变化
																		// 正走的时候，比如posA=2，spt_eg.cha_pos=0，这个move就表示走了2个蛋杯
																		// 在一整圈的时候posA由于循环可能变得很小，比如开始posA=269，spt_eg.cha_pos=269
																		// 下一次走就是(posA=0)-(spt_eg.cha_pos=269)=-269,spt_eg.cha_pos=0
																		// 下一次走就是(posA=1)-(spt_eg.cha_pos=0)=1
																
	// posA 在0-269之间		
	// spt_eg.cha_pos 也在0-269之间	
	// 会出现当上一次spt_eg.cha_pos是269
	// 本次posA算出来是0
	// 那么 move=0-269=-269，但是其实不能只判断大于0即走了一格，-269这种情况也是走了一格
	
	
	
	if((move>0)||(move<(-2)))			
	{
		spt_eg.cha_pos = posA;		// 实时的将最新的蛋杯位置给到这个分离数据spt_eg
		spt_eg.chb_pos = posB;
		spt_egg_to_packge_line();    //蛋杯走了一格或者多格的时候，也计算要分离的蛋属于哪个通道
	}
	return cpluse;
}

// 计算蛋杯打开时间
// 蛋杯打开的时间原理上要怎么计算呢
// 是整个电磁铁在某个蛋经过的时候都要打开，该蛋杯过了然后在关闭电磁铁？
// 并且这个时间要发给分级线的各个通道，告诉他们这个速度下每次电磁铁打开时间？
// 如果是这样的，那么时间等于一个蛋杯的距离除以分级线的速度，但是得到的是秒，要转换为毫秒
uint16_t calculate_cup_open_time()
{
	 uint16_t cup_open_time=0;
	 uint16_t move_dist;
	 float buf1;
	
	 move_dist = get_spt_speed();	// 这个是mm/s
	 buf1 =  (float)move_dist/SEPARATE_CUP_distem;
	// buf1 +=2;
	 buf1 = (int)buf1*10; //扩大10倍
	 cup_open_time = 10000/buf1;
	// rt_kprintf("cup open move dist=%d time=%d\n",move_dist,cup_open_time);

	// rt_kprintf("move_dist=%d\n",move_dist);
	

	// 上面计算蛋杯打开时间，举个例子，比如600mm/s
	// 10000/(600/63.5*10)=10000/94.48=105.84ms?
	// 63.5/600=0.1058s=105.8ms？
	
	
	
	
		// 算出来的这个时间为什么要调整呢，是因为实际当中有点差别？
	  if(move_dist<STP_ABNORMAL_SPEED)
	 {
	    cup_open_time = (cup_open_time*10)/7; //一半的时间  1.5
	 }
	 else  if(move_dist<400)
	 {
	    cup_open_time = (cup_open_time*10)/14; //一半的时间  1.5
	 }
	 else  
	 {
	   cup_open_time = (cup_open_time*10)/15; //一半的时间  1.5
	 }
	 

	 if(cup_open_time>1000)
	 {
		 //rt_kprintf("cup open time=%d\n",cup_open_time);
		 cup_open_time =1000;	// 最大1s
	 }
   return cup_open_time;
}



static uint8_t product_alarm_dat[12]={1,1,1,1,1,1,1,1,1,1,1,1};
// 产品生产完后是否需要报警，是否需要停机
void product_complete_alarm(uint8_t i,uint8_t bar,uint8_t alarm)
{

   if(alarm==1)   //不提示
	 {
			return ;
	 }

   if(bar>=100)
	 {
		 if(product_alarm_dat[i]==0)
		 {
				product_alarm_dat[i]=1;
			 
			 if(alarm==2)						// 只报警不停机
			 {
				 set_tast_complete_alarm();
			  // DEBUG_CARE("任务=%d 已生产完成报警提示\n",i);
			 }
			 else if(alarm==3)			// 报警并停机
			 {
				 set_device_alarm(PRODUCT_COMPLETE);
				 set_system_state(SYSTEM_IS_STOP);
				 set_tast_complete_alarm();
				 set_expansion_link_move(2,0);
			  // DEBUG_CARE("任务=%d 已生产完成报警提示 并停机\n",i);
			 }
		 }
	 }
	 else 
	 {
	     product_alarm_dat[i]=0;
	 }
}
// 计算生产的进度
void clacure_alread_pruduct_bar()
{  
	uint8_t per_box_egg;
	uint8_t i;
	uint32_t egg_count=0;
	uint32_t box_count=0;
	for(i=0;i<12;i++)	// 最多12个产品
	{
		if(per_poduction_egg.pre_production[i].empty  !=0)	// 依次判断这些产品哪些在用
		{
			per_box_egg = per_poduction_egg.pre_production[i].tray_line * per_poduction_egg.pre_production[i].tray_row;	// 根据盒子计算每个盒子装蛋数量
      egg_count = alread_production.pruduct_info[i].pruduct_egg_count;							// 已经生产的蛋的枚数
		  box_count = alread_production.pruduct_info[i].pruduct_egg_count/per_box_egg;	// 已经生产的蛋的合数 (根据已经生产的枚数除以每盒可以装的枚数)
		  alread_production.pruduct_info[i].pruduct_box_count = box_count;							// 记录一下已经生产的盒数
		  if(per_poduction_egg.pre_production[i].task_egg_count>0)
			{
				alread_production.pruduct_info[i].prpduct_bar = (egg_count*100)/per_poduction_egg.pre_production[i].task_egg_count;	// 根据已经生产的枚数除以计划生产的枚数得到进度
				if(alread_production.pruduct_info[i].prpduct_bar>100) alread_production.pruduct_info[i].prpduct_bar =100;
			}
			else 
			{
			  alread_production.pruduct_info[i].prpduct_bar =0;	// 计划生产的枚数为0，进度肯定也就为0
			}
			alread_production.flag =1;	// 这个flag有用吗？
			product_complete_alarm(i,alread_production.pruduct_info[i].prpduct_bar,per_poduction_egg.pre_production[i].task_complete_warn);
		}
		else 
		{
		  alread_production.flag =0;
		}
	}
}

// 清除某个蛋品的信息
// 注意这个函数没有先保存，所以调用这个函数之前需要先保存这个蛋品的信息？
// 清除所有已经生产的产品信息
void clear_already_pruduct_egg_dat(uint16_t index)
{
	rtc_info_t time;
  rt_base_t level;
	
	get_local_time(&time);
	if(index<12)	// 按照12个产品来清除
	{
		level = rt_hw_interrupt_disable();
		alread_production.pruduct_info[index].pruduct_egg_count=0;
		alread_production.pruduct_info[index].date = (time.year+1900)*10000;
	  alread_production.pruduct_info[index].date  += (time.month*100);
	  alread_production.pruduct_info[index].date  += time.date;
		alread_production.pruduct_info[index].start_time  = (time.hour *100)+time.min ;
		rt_hw_interrupt_enable(level);
	}
//	DEBUG_CARE("清除记录时必须要保存历史\n");
}


//一共 18条线   0-11 包装通道 12-17 抛弃通道
// 清除某条线(包括抛弃通道)已经生产的蛋的数量
void clear_packge_ch_already_pruduct_egg_dat(uint16_t index)
{
  rt_base_t level;
	if(index<SPT_CH_LINE)
	{
		level = rt_hw_interrupt_disable();
		alread_production.packge_ch_egg_count[index]=0;
		rt_hw_interrupt_enable(level);
	}
}
// 获取所有已经生产的蛋品的信息
void get_already_product_egg(alread_production_type *dat)
{
	 clacure_alread_pruduct_bar();
	 rt_memcpy(dat,&alread_production,sizeof(alread_production_type));
}
// 0-17 
// 包装通道0-11
// 抛弃通道   12-17 
// 每生产一个鸡蛋就记录一下个数
// 这个鸡蛋有可能是要走抛弃通道，有可能要走正常的包装通道，所以需要判断是哪个通道
// 然后判断是哪个蛋品family_id，相应的蛋品数量也要加1
// 然后今天的总生产数也要加1
void count_product_egg(uint8_t ch,uint8_t family_id)  
{
	if(ch<SPT_CH_LINE)
	{
		alread_production.packge_ch_egg_count[ch]++;
	}
	if(family_id<TASK_PRUDUCTION_COUNT)  
	{
	  alread_production.pruduct_info[family_id].pruduct_egg_count++;
	}
	alread_production.today_egg_count++;
}
// 获取某个通道(包括包装通道)的已经生产的数量
uint32_t get_packge_ch_alread_product_egg(uint8_t ch)
{
	return alread_production.packge_ch_egg_count[ch];
}
// 根据供应商的名字找到记录列表中的供应商存储序号或者id
void get_save_ayaylisis_egg_index(char *name)
{
  save_analisys_egg_recode_index = find_supply_id_from_recode_dat(name);
}
static void sys_check_env()   //启动前 更新下预生产信息 
{
	clear_avg_product_count();
  get_production_mange_set(&per_poduction_egg);
	//save_analisys_egg_recode_index = find_supply_id_from_recode_dat((char *)per_poduction_egg.egg_source);
}


uint8_t caculate_spt_ch(uint8_t product_id,uint16_t spt_ch)
{
  uint8_t ch;
	
	int8_t i=0;
	uint8_t pos;
	uint8_t egg_count=0;
	uint8_t max_eggs;
	uint8_t line;
	if(spt_ch==0x555)
	{
		pos = avg_product_line_recode[product_id].product_chA.line_pos;
		max_eggs = avg_product_line_recode[product_id].product_chA.max_eggs;
		if(0)
		//if(avg_product_line_recode[product_id].product_chA.egg_count[pos]<max_eggs)  //当前通道没有接满
		{
		    avg_product_line_recode[product_id].product_chA.egg_count[pos]++;
			ch = pos;
		}
		else                          //寻找下一个通道
		{
		  avg_product_line_recode[product_id].product_chA.egg_count[pos]=1;
		  for(i=0;i<12;i++)   //
		  { 
				line= (pos +1) +i;
				if(line>11) line = line-12;
				if(avg_product_line_recode[product_id].product_chA.line[line]>0) //分配到该通道
				{
					avg_product_line_recode[product_id].product_chA.line_pos = line;
					avg_product_line_recode[product_id].product_chA.egg_count[line]=1;  //分配了一个鸡蛋
				  ch = line;
					break;
				}
		 }
		}
		//rt_kprintf("ch=%d max=%d count=%d\n",ch,max_eggs,avg_product_line_recode[product_id].product_chA.egg_count[ch]);
	}
	else 
	{
	  pos = avg_product_line_recode[product_id].product_chB.line_pos;
		max_eggs = avg_product_line_recode[product_id].product_chB.max_eggs;
		if(0)
		//if(avg_product_line_recode[product_id].product_chB.egg_count[pos]<max_eggs)  //当前通道没有接满
		{
		    avg_product_line_recode[product_id].product_chB.egg_count[pos]++;
			  ch = pos;
		}
		else                          //寻找下一个通道
		{
		  avg_product_line_recode[product_id].product_chB.egg_count[pos]=1;
			for(i=0;i<12;i++)   //
			{ 
				line= (pos +1) +i;
				if(line>11) line = line-12;
				if(avg_product_line_recode[product_id].product_chB.line[line]>0) //分配到该通道
				{
					avg_product_line_recode[product_id].product_chB.line_pos = line;
					avg_product_line_recode[product_id].product_chB.egg_count[line]=1;  //分配了一个鸡蛋
				  ch = line;
					break;
				}
			}
		}
	}
	
	return ch;
}


// product_id 0 -5   6种产品 ID
// falimy  1-6列包装线


void init_avg_spt_egg_info(uint8_t product_id,uint16_t falimy,uint8_t max_eggs)
{
	uint8_t i,j;
	uint8_t eggs;
	uint8_t egg_count;
	uint16_t lineA,lineB;

	if(product_id>11)
	{
		return ;
	}
	
	lineA = falimy&0x555;
	lineB = falimy&0xaaa;
	rt_memset(&avg_product_line_recode[product_id],0,sizeof(avg_product_line_info));
	
	for(i=0;i<12;i++)
	{
	  if(lineA & (1<<i))
		{
			eggs = get_packge_recive_cup_eggs(i);	// 获取对应通道的6个接收爪有误蛋的情况(6个接收爪，低位对应1高位对应6，1真为有蛋0无蛋)
			
			for(j=0;j<6;j++)
			{
			  if(eggs&(1<<j))	// 依次检查这6个接收爪是不是有蛋，有蛋则该包装通道蛋数量加1
				{
				  egg_count++;
				}
			}
			
			avg_product_line_recode[product_id].product_chA.line_pos=i;	// line_pos是什么意思，应该是哪一条线即哪个通道
			
			if(egg_count<max_eggs)		//  max_eggs传入的是托盘列，说明就是计算出的6个接收爪的蛋数小于蛋托一排能够装的蛋数(也就是有些蛋托一排有可能没有6个格子，比如3*2)
			{
			  avg_product_line_recode[product_id].product_chA.egg_count[i]=egg_count;	// 该通道的蛋数设置为
			}
			else 
			{
			  avg_product_line_recode[product_id].product_chA.egg_count[i]=1;	// 如果算出的接收爪的蛋数大于等于拖每排最大可以接收的蛋数，则该通道的蛋数置为1
			}
		  
			avg_product_line_recode[product_id].product_chA.line[i]=1;						// 该通道这个变量置为1
			avg_product_line_recode[product_id].product_chA.max_eggs = max_eggs;	// 保存该通道的拖每排最大可以接收的蛋数
		}
	}
	
	for(i=0;i<12;i++)
	{
		
		
			
	  if(lineB & (1<<i))
		{
			
			eggs = get_packge_recive_cup_eggs(i);
			
			for(j=0;j<6;j++)
			{
			  if(eggs&(1<<j))
				{
				  egg_count++;
				}
			}
			
			if(egg_count<max_eggs)
			{
			  avg_product_line_recode[product_id].product_chB.egg_count[i]=egg_count;
			}
			else 
			{
			  avg_product_line_recode[product_id].product_chB.egg_count[i]=1;
			}
			
			
			avg_product_line_recode[product_id].product_chB.line_pos=i;
		  
			avg_product_line_recode[product_id].product_chB.line[i]=1;
			avg_product_line_recode[product_id].product_chB.max_eggs = max_eggs;
		}
	}
}



// 同步某条通道下次踢蛋用哪个电磁铁踢
void sync_packge_recive_cup_eggs()
{
  uint8_t i,j,pos;
	uint8_t d;
	uint8_t eggs=0;
	

	for(i=0;i<12;i++)
	{
	  eggs = get_packge_recive_cup_eggs(i);		// 获取所有通道6个接收爪是不是有蛋
		DEBUG_WARN("recive cup ch=%d state=%02x\n",i,eggs);
		pos = 6;
		for(j=6;j>0;j--)
		{
			d = 1<<(j-1);
			if((eggs & d)>0)		// 依次判断5-0这6个位置有没有蛋，有蛋记录下该位置
			{
			  pos = 	j-1;			// 最后得到的是有蛋的最小位置
			}
		}
		if(pos>recode_kick_egg[i].min_ch)					// 这个有蛋的位置大于最小的位置，表明这一排鸡蛋要接收的数量还没有接收满
		{
			recode_kick_egg[i].kick_ch_pos = pos-1;	// 那么接收的位置要从有蛋的下一次即没有接蛋的位置的第一个开始接收
		}
		else
		{
			recode_kick_egg[i].kick_ch_pos = recode_kick_egg[i].max_ch;	// 接收满了(一般相等的时候就是接收满了)，从新开始
		}
		if(recode_kick_egg[i].kick_ch_pos>recode_kick_egg[i].max_ch)
		{
			recode_kick_egg[i].kick_ch_pos=recode_kick_egg[i].max_ch;
		}
	}
}


//设置包装盒有几列  踢蛋只提这几列。奇数靠左 偶数靠中间
// ch 0 -5
// line 列数  1-6列

// 设置没有踢到的蛋，重新踢的时候的电磁铁的id？
// 就是有可能分级线冲过头了，没有踢到？
// 设置某条通道用0-5电磁铁中哪些电磁铁来踢蛋
void init_recode_kick_egg_info(uint8_t ch,uint16_t line)
{
	if(ch>11)
		return;
	
	 if(line==0)
	 {
		recode_kick_egg[ch].min_ch=0;
		recode_kick_egg[ch].max_ch=5;
	 }
	 else if(line==1)
	 {
		recode_kick_egg[ch].min_ch=0;
		recode_kick_egg[ch].max_ch=0;
	 }
	 else if(line==2)
	 {
		recode_kick_egg[ch].min_ch=2;
		recode_kick_egg[ch].max_ch=3;
	 }
	 else if(line==3)
	 {
		recode_kick_egg[ch].min_ch=0;
		recode_kick_egg[ch].max_ch=2;
	 }
	 else if(line==4)
	 {
		recode_kick_egg[ch].min_ch=1;
		recode_kick_egg[ch].max_ch=4;
	 }
	 else if(line==5)
	 {
		recode_kick_egg[ch].min_ch=0;
		recode_kick_egg[ch].max_ch=4;
	 }
	 else if(line==6)
	 {
		recode_kick_egg[ch].min_ch=0;
		recode_kick_egg[ch].max_ch=5;
	 }
	 if(recode_kick_egg[ch].kick_ch_pos>recode_kick_egg[ch].max_ch)
	 {
		recode_kick_egg[ch].kick_ch_pos=recode_kick_egg[ch].max_ch;
	 }
	 if(recode_kick_egg[ch].kick_ch_pos<recode_kick_egg[ch].min_ch)
	 {
		recode_kick_egg[ch].kick_ch_pos=recode_kick_egg[ch].max_ch;
	 }
	 
	 DEBUG_CARE("line=%d 包装盒行数=%d max ch=%d min ch=%d pos=%d\n",ch,line,recode_kick_egg[ch].max_ch,recode_kick_egg[ch].min_ch,recode_kick_egg[ch].kick_ch_pos);
}




#define TICK_EGG_PLUSE 150  //电磁铁打下去最大距离   电磁铁打下去和抬起来 有时间 以及通信延迟  导致距离增大
// 关闭跑题通道所有的电磁铁
void close_throw_all_pin()
{
	message_buf[0]=20;      //命令
  send_message_to_throw_device(0,message_buf); 
	send_message_to_throw_device(1,message_buf); 
}


void start_retick_eggs(uint8_t line,uint8_t ch,uint16_t id,uint32_t current_pluse) //启动的时候重复踢一遍
{

	uint16_t cup_open_time;
	uint32_t throw_dist,egg_offest;
	separate_egg_info_type *spt_egg;
	
	// id 这个是相对于第一个蛋杯的蛋杯数
	// id 这个是相对于第一个蛋杯的蛋杯数
	// id 这个是相对于第一个蛋杯的蛋杯数
	
	// 然后根据这个蛋杯数去分离数据spt_eg中取出该蛋杯号的数据spt_egg
	// 然后判断该蛋踢的分离通道和本次判断的通道是不是一样
	// 一样并且是需要踢则判断
	
	if(line>1) return;
	if(ch<8)
	{
		get_pst_egg_info(line,id,&spt_egg,0);
	}
	else if((ch==8)||(ch==9))
	{
	   get_pst_egg_info(0,id,&spt_egg,0);		// A线的抛弃通道
	}
	else if((ch==10)||(ch==11))
	{
	   get_pst_egg_info(1,id,&spt_egg,0);		// B线的抛弃通道
	}
	
	// 得到如果该蛋是该当前分离道踢，并且是需要踢的状态
	if((spt_egg->separate_ch==ch)&&(spt_egg->empty ==TICK_EGG_DOWN))      //此蛋属于当前通道要踢的蛋  
	{
		// 获取该踢蛋通道的脉冲值
		throw_dist = throw_ahead_info[ch].ch[spt_egg->tick_ch] ;   //减去速度的偏移，提前踢蛋	// spt_egg->tick_ch 值6个电磁铁中的哪一个
		// 这个脉冲值在加150个脉冲，大概一个蛋杯166的距离
		throw_dist = throw_dist + 150;
		
		// spt_egg->pluse是分离的时候的脉冲值
		// current_pluse - spt_egg->pluse，当前脉冲减去这个脉冲得到的是什么呢
		// 上面的throw_dist这个脉冲可是相对于第一个蛋杯的觉得脉冲值呀
		// 一个相对一个觉得怎么能够相比呢
		egg_offest = current_pluse - spt_egg->pluse;    	// spt_egg->pluse这个是计算分离的时候的脉冲值
																											// 那么得到的这个egg_offest是当前脉冲值与计算分离的时候的脉冲值的差值
																											
   // DEBUG_INFO("ch=%d egg ID=%d cpluse=%d throw dist=%d offest=%d\n",spt_egg->separate_ch,spt_egg->tick_ch,current_pluse,throw_dist,egg_offest);
		
		if(throw_dist>egg_offest)
		{
			  cup_open_time = 3000;
			   message_buf[0]=1;      //命令提蛋
				message_buf[1]=ch%2;      //蛋带通道通道				  0-1
				message_buf[2]=spt_egg->tick_ch;    //蛋杯通道   0-5
				message_buf[3]=1;      //开关状态      开
				message_buf[4]=cup_open_time>>8;      //开启时间 高8位
				message_buf[5]=cup_open_time&0xff;      //开启时间 低8位
				message_buf[6]=get_spt_speed()/10;
			  message_buf[7] = 0;
			  send_message_to_packge_device(ch,message_buf); 
			  //DEBUG_INFO("蛋杯提前开启 line=%d ch=%d offest=%d throw dist=%d\n",ch,spt_egg->tick_ch,egg_offest,throw_dist);
			
				// 记录该蛋的电磁铁打开时间，踢蛋通道，踢蛋状态(踢蛋中)
				recode_tick_egg_info_down(line,id,cup_open_time,spt_egg->tick_ch,TICK_EGG_DOWN);
		}
		else  //脉冲冲过头了
		{
		    if(spt_egg->tick_ch==recode_kick_egg[ch].min_ch)
		    {
					message_buf[0]=7;      //调试命令
					message_buf[1]=ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=spt_egg->tick_ch;   //蛋杯通道   0-5
					message_buf[3]=0;      //开关状态      开
					message_buf[6]=get_spt_speed()/10;
					message_buf[7]=message_buf[1]+1;   //接受抓下降标记
					send_message_to_packge_device(ch,message_buf); 
					// 记录该蛋的电磁铁打开时间，踢蛋通道，踢蛋状态(踢蛋完成)
					recode_tick_egg_info_down(line,id,cup_open_time,spt_egg->tick_ch,TICK_EGG_IS_OVER);
					DEBUG_INFO("漏网之鱼 line=%d\n",ch);
				}
		}
	}
}






//CH = 12 13 14 15 16 17
// 这个计算抛弃通道踢蛋的信息
// 并且将这个信息发给抛弃通道的板子
// 并且记录这个抛弃通道抛弃的数量，然后会按照每一分钟计算，当一分钟内通过抛弃通道抛弃的数量大于设定的数量则停机？
void throw_ch_tick_egg(uint8_t line,uint8_t ch,uint16_t id,uint16_t offeset_pluse,uint32_t tick_pluse,uint16_t cup_open_time)
{

  int less;

	uint16_t spt_speed;
	uint32_t time;

	uint32_t current_pluse;
	uint32_t throw_dist,egg_offest;
	separate_egg_info_type *spt_egg;
	
	if(line>1) return;
	
	current_pluse = get_spt_encode_pluse();
	get_pst_egg_info(line,id,&spt_egg,0);

	if((spt_egg->separate_ch==ch)&&(spt_egg->empty ==NEED_TICK_EGG))      //此蛋属于当前通道要踢的蛋  
	{
		throw_dist =  tick_pluse -offeset_pluse;   //减去速度的偏移，提前踢蛋
		egg_offest = current_pluse - spt_egg->pluse;    
        less = egg_offest - throw_dist; 	
       //if(ch==9)rt_kprintf("offest=%d  td=%d   add=%d\n",egg_offest,throw_dist,less);		
		if((less<20)&&(less>0))
		{
			  
			  spt_speed = get_spt_speed();
			  message_buf[0]=1;      //命令
			  message_buf[1]=ch -11;  //1开始
				message_buf[2]=1;    // 开关
				message_buf[3] = 0xaa;
				message_buf[4]=cup_open_time>>8;      //开启时间 高8位
				message_buf[5]=cup_open_time&0xff;      //开启时间 低8位
				send_message_to_throw_device(0,message_buf); 
				count_per_min_egg((ch-12)+(line*1));	// 0 1 2 4 5 6 (3,7是强制抛弃)，设置当前通道里面1分钟60s的当前时间应在的位置的抛弃数量加1
																							// 这样分析强制抛弃的数量就不算了
				count_product_egg(ch,ch);							// 统计蛋数，包括该产品的蛋数，该通道的蛋数，该天总数

			  if(spt_speed>=STP_ABNORMAL_SPEED)
				{
					recode_tick_egg_info_down(line,id,1,ch,TICK_EGG_IS_OVER);
				}
				else
				{
					recode_tick_egg_info_down(line,id,cup_open_time,ch,TICK_EGG_DOWN);
				}
			//	rt_kprintf("tick egg ch=%d\n",ch);
		}
	}
	else if((spt_egg->separate_ch==ch)&&(spt_egg->empty ==TICK_EGG_DOWN))      //此蛋属于当前通道要踢的蛋  
	{
		throw_dist =  tick_pluse -offeset_pluse;   //减去速度的偏移，提前踢蛋
		throw_dist = throw_dist + TICK_EGG_PLUSE;
		egg_offest = current_pluse - spt_egg->pluse;    
    less = egg_offest - throw_dist; 				
		time = count_use_time(spt_egg->tick_time);
		
		if((less>0)&&(time<spt_egg->cup_open_time))
		{
			  message_buf[0]=1;      //命令
			  message_buf[1]=ch -12;  //1开始
				message_buf[2]=0;    // 开关
				message_buf[3] = 0xaa;
				message_buf[4]=cup_open_time>>8;      //开启时间 高8位
				message_buf[5]=cup_open_time&0xff;      //开启时间 低8位
				send_message_to_throw_device(0,message_buf); 
				recode_tick_egg_info_down(line,id,cup_open_time,spt_egg->tick_ch,TICK_EGG_IS_OVER);
		}
	}
}
//line 哪条分级线 A 或者B
// CH 通道
//id 鸡蛋ID
uint8_t recive_busy_count[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
void calculate_spt_egg_down(uint8_t line,uint8_t ch,uint16_t id,uint32_t current_pluse,uint16_t cup_open_time,uint16_t offeset_pluse)

{
	int less;

	uint16_t spt_speed;
	uint32_t time;
	uint32_t empty_id;
	uint32_t throw_dist,egg_offest;
	separate_egg_info_type *spt_egg;
	get_pst_egg_info(line,id,&spt_egg,0);
	if((spt_egg->separate_ch==ch)&&(spt_egg->empty ==NEED_TICK_EGG))      // 此蛋属于当前通道要踢的蛋  
	{																																			// 次鸡蛋的踢蛋通道是当前通道
		throw_dist = throw_ahead_info[ch].ch[recode_kick_egg[ch].kick_ch_pos] -offeset_pluse;   //减去速度的偏移，提前踢蛋
		egg_offest = current_pluse - spt_egg->pluse;    
		
		// throw_dist 是转载处到该电磁铁处的距离脉冲
		// spt_egg->pluse 是在转载处存该蛋的时候的脉冲
		// current_pluse 是当前分级线走的脉冲
		// current_pluse - spt_egg->pluse 表示该鸡蛋走了多少脉冲
		// less = egg_offest - throw_dist 走的脉冲和电磁铁举例脉冲作差
		// 大于0，说明走过了电磁铁，走过了，不超过20个脉冲则认为可以踢，否则是真的走过了
		// 小于0，说明还没有走到
		
    less = egg_offest - throw_dist; 		
		//		less = abs(less);
		if((less<20)&&(less>0))
		{
		  empty_id = get_recive_cup_empty_id(ch);		// 获取包装通道给给主控的状态的信息编号，当完成一个接收伺服下降并恢复后就会发信息给主控，里面的empty_id就会加1
																								// 这样和本地recive_cup_empty_id[ch]就会不一样
																								// 如果一样，并且踢的位置是最大编号的电磁铁，说明一直放的有问题，所以没有回复，就认为很忙？
		//if((recive_cup_empty_id[ch] == empty_id)&&(recode_kick_egg[ch].kick_ch_pos == recode_kick_egg[ch].max_ch)
		//&&((ch==0)||(ch==1)||(ch==4)||(ch==5)))  //踢第一通道有蛋时候检测接受抓是否清空
			
			// recive_cup_empty_id[ch]中保存着
			// kick_ch_pos从max_ch开始到min_ch
			// 这里面包含2个信息：1 踢的位置kick_ch_pos等于max_ch，说明这个蛋托一排的数量满了
			//										2 踢的位置kick_ch_pos不等于max_ch，说明这个蛋托一排的数量没有满，则要继续踢，直到kick_ch_pos等于min_ch该拖的一排满了，并且接收伺服要下降，然后将kick_ch_pos置为max_ch		
			//											举个例子:某个包装是5*6，最大是5，最小是0，那等等于5的时候是表示蛋托重新一排开始了，这个时候就要计算踢蛋信息
			//																等到kick_ch_pos等于0的时候，接收伺服下降，并将kick_ch_pos置为5
			//											举个例子:某个包装是2*3，最大是5，最小是3(后面0-2永远不踢，因为蛋托一排就只有3个)，那等等于5的时候是表示蛋托重新一排开始了，这个时候就要计算踢蛋信息
			//																等到kick_ch_pos等于3的时候，接收伺服下降，并将kick_ch_pos置为5
			//															:某个包装是2*3，最大是3，最小是1(后面0,4,5永远不踢，因为蛋托一排就只有3个)，那等等于3的时候是表示蛋托重新一排开始了，这个时候就要计算踢蛋信息
			//																等到kick_ch_pos等于1的时候，接收伺服下降，并将kick_ch_pos置为3
	  if((recive_cup_empty_id[ch] == empty_id)&&(recode_kick_egg[ch].kick_ch_pos == recode_kick_egg[ch].max_ch))  //踢第一通道有蛋时候检测接受抓是否清空
		{
	    //recive_cup_empty_id[ch] = empty_id+100;
			//DEBUG_WARN("接受抓busy ch=%d emptyid=%d eggs=%02x 漏蛋\n",ch,recive_cup_empty_id[ch],get_packge_recive_cup_eggs(ch));
			recive_busy_count[ch]++;		// 如果踢的位置变成了最大，因为满了才变为最大，满了接收伺服就会下降，下降就会忙，下降完成后empty_id会变
			if(recive_busy_count[ch]>5)	// 连续5次，接收伺服都还没有完成则报错
			{
				recive_busy_count[ch]=0;
				set_device_alarm(PACKGE_RECIVE1_ERR+ ch);
			}
			recode_tick_egg_info_down(line,id,1000,10,TICK_EGG_IS_BUSY);
		}
		else 
		{
			  recive_busy_count[ch]=0;		// 踢蛋接收伺服没有动作，显然忙状态要变为0
				recive_cup_empty_id[ch] = empty_id;
				count_product_egg(ch,spt_egg->family_ch);

				spt_speed = get_spt_speed();
				if(spt_speed>=STP_ABNORMAL_SPEED)
					{
					message_buf[0]=1;      //命令提蛋
					message_buf[1]=ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=recode_kick_egg[ch].kick_ch_pos;    //蛋杯通道   0-5
					message_buf[3]=1;      //开关状态      开
					message_buf[4]=cup_open_time>>8;      //开启时间 高8位
					message_buf[5]=cup_open_time&0xff;      //开启时间 低8位
					message_buf[6]=spt_speed/10;
					if(recode_kick_egg[ch].kick_ch_pos==recode_kick_egg[ch].min_ch)
					{
						message_buf[7]=message_buf[1]+1;   //接受抓下降标记
					}
					else 
					{
					  message_buf[7]=0;
					}
					//rt_kprintf("正常踢蛋\n");
					
					// 下面这个函数主要根据  spt_eg
					recode_tick_egg_info_down(line,id,cup_open_time,recode_kick_egg[ch].kick_ch_pos,TICK_EGG_IS_OVER);
				}
				else
				{
				  message_buf[0]=1;      //命令提蛋
					message_buf[1]=ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=recode_kick_egg[ch].kick_ch_pos;    //蛋杯通道   0-5
					message_buf[3]=1;      //开关状态      开
					message_buf[4]=cup_open_time>>8;      //开启时间 高8位
					message_buf[5]=cup_open_time&0xff;      //开启时间 低8位
					message_buf[6]=spt_speed/10;
				  message_buf[7] = 0;		// 速度慢接收伺服一直不下降？
					recode_tick_egg_info_down(line,id,cup_open_time,recode_kick_egg[ch].kick_ch_pos,TICK_EGG_DOWN);
					//rt_kprintf("low speed line=%d ch=%d\n",ch,recode_kick_egg[ch].kick_ch_pos);
					
					if(recode_kick_egg[ch].kick_ch_pos==recode_kick_egg[ch].min_ch)
					{
						servo_down_pluse[ch] = current_pluse;
						//DEBUG_CARE("low soeed line=%d pos=%d egg id=%d   !!!!!!!!!!!!!!!!!!\n",ch,recode_kick_egg[ch].min_ch,spt_egg->id);
					}

				}
				send_message_to_packge_device(ch,message_buf); 

				// kick_ch_pos是用6个中的哪个电磁铁踢
				// 从最大max_ch到min_ch最小间不断轮询
				if(recode_kick_egg[ch].kick_ch_pos>recode_kick_egg[ch].min_ch)
					recode_kick_egg[ch].kick_ch_pos--;
				else 
					recode_kick_egg[ch].kick_ch_pos= recode_kick_egg[ch].max_ch;
		 }
		}
		else if(less>0)
		{
			//DEBUG_ERR("溢出 cpluse=%d line=%d pos=%d o=%d t=%d\n",current_pluse,ch,recode_kick_egg[ch].kick_ch_pos,egg_offest,throw_dist);
			//show_pluse(current_pluse,offeset_pluse);
		  recode_tick_egg_info_down(line,id,1000,10,TICK_EGG_IS_OVERFLOW);     //清除数据
		}
	}
	// 低于300mm/s就会走这个分支
	// 这个分支，电磁铁不踢
	// 满了6个则接收伺服下降,但是接收伺服下降是要在电磁铁踢的状态才能下降，说明也执行不了，接收伺服下降不了？
	else if((spt_egg->separate_ch==ch)&&(spt_egg->empty ==TICK_EGG_DOWN))      //此蛋属于当前通道要踢的蛋  
	{
		throw_dist = throw_ahead_info[ch].ch[spt_egg->tick_ch] -offeset_pluse;   //减去速度的偏移，提前踢蛋
		throw_dist = throw_dist + TICK_EGG_PLUSE;
		egg_offest = current_pluse - spt_egg->pluse;    
    
        less = egg_offest - throw_dist; 		
		time = count_use_time(spt_egg->tick_time);
		if((less>0)&&(time<spt_egg->cup_open_time))
		{
			  message_buf[0]=7;      //调试命令
				message_buf[1]=ch%2;      //蛋带通道通道				  0-1
				message_buf[2]=spt_egg->tick_ch;   //蛋杯通道   0-5
				message_buf[3]=0;      //开关状态      关		// 其实包装通道的电磁铁没有处理关这个，包装通道电磁铁关是根据打开时间到了自己就关了（不对，这个发的是调试命令7）
				message_buf[6]=get_spt_speed()/10;
			  if(spt_egg->tick_ch==recode_kick_egg[ch].min_ch)
				{
						message_buf[7]=message_buf[1]+1;   //接受抓下降标记
					  rt_kprintf("recive down egg line=%d id=%d\n",ch,spt_egg->id);
				}
				else 
				{
					  message_buf[7]=0;
				}
				send_message_to_packge_device(ch,message_buf); 
			  //DEBUG_INFO("蛋杯提前关闭 ch=%d offest=%d throw dist=%d\n",spt_egg->tick_ch,egg_offest,throw_dist);
				recode_tick_egg_info_down(line,id,cup_open_time,spt_egg->tick_ch,TICK_EGG_IS_OVER);
		}
	}
}

// 检查接收伺服是否放蛋完成
// 原理是kick_ch_pos == max_ch，表示接收伺服应该是在处理放蛋过程
// 并且放蛋完成后empty_id会变，和本地的就不一样了
// 如果kick_ch_pos == max_ch并且empty_id一直没有变，说明有问题
// 最多等分级线跑300个脉冲这么久的时间，还是有问题
// 则发一次关闭电磁铁，并让接收伺服在放一次
// 经分析这个是慢速的时候，检查接收伺服是否已经放下了
void check_recive_servo_egg_is_down()
{
   uint8_t ch;
   uint32_t empty_id;
	 uint32_t current_pluse;
	
	 current_pluse = get_spt_encode_pluse();
	 for(ch=0;ch<12;ch++)
	 {
	   empty_id = get_recive_cup_empty_id(ch);
		 if((recive_cup_empty_id[ch] == empty_id)&&(recode_kick_egg[ch].kick_ch_pos == recode_kick_egg[ch].max_ch))
		 {
			 if(current_pluse>(servo_down_pluse[ch] +300))
			 {
			    message_buf[0]=7;      //调试命令
					message_buf[1]=ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=recode_kick_egg[ch].min_ch;   //蛋杯通道   0-5
					message_buf[3]=0;      //开关状态      开
					message_buf[6]=10;
					message_buf[7]=message_buf[1]+1;   //接受抓下降标记
					send_message_to_packge_device(ch,message_buf); 
				  DEBUG_WARN("脉冲超出范围 接受抓放蛋 ch=%d\n",ch);
			 }
		 }
	 }
}




// 踢蛋


void tick_egg()
{
	uint8_t i;
	uint8_t expansion_move_cup;
	uint16_t egg_id;
	uint16_t cup_open_time;
	uint32_t current_pluse;
	uint32_t throw_max_id;
	uint32_t throw_min_id;
  float expansion_move_pluse=0;
 
  throw_cha_max_id = THROW_LINEA3_POSTION/SEPARATE_CUP_LEN;		// 通过抛弃通道位置脉冲偏移以及一个蛋的脉冲得到从原点走到抛弃通道要走的蛋杯数也即蛋杯id
	throw_cha_min_id = THROW_LINEA1_POSTION/SEPARATE_CUP_LEN;
	
	throw_chb_max_id = THROW_LINEB3_POSTION/SEPARATE_CUP_LEN;
	throw_chb_min_id = THROW_LINEB1_POSTION/SEPARATE_CUP_LEN;
	
	throw_cha_max_id +=2;		// 往后2个蛋杯
	throw_cha_min_id -=6;		// 往前6个蛋杯
	
	throw_chb_max_id +=2;
	throw_chb_min_id -=6;
	
	cup_open_time = calculate_cup_open_time();	// 计算每个电磁铁踢蛋的时候，打开电磁铁的时间
	
	
	expansion_move_pluse = SEPARATE_CUP_LEN/SEPARATE_CUP_distem;	//	166/63.5怎么理解
	expansion_move_pluse = expansion_move_pluse*get_expansion_link_offeset();
	expansion_move_cup = expansion_move_pluse/SEPARATE_CUP_LEN;
	// 这个主要想计算伸缩杆移动了多少个蛋杯
	// 应该是移动的距离get_expansion_link_offeset
	// 除以一个蛋杯的距离SEPARATE_CUP_distem
	// 得到移动了几个蛋杯
	// 然后乘以一个蛋杯的脉冲SEPARATE_CUP_LEN得到移动了多少个脉冲
	// 然后除以一个蛋杯的脉冲得到多少个蛋杯
	
	// 其实分析以及带进入看
	// 就是移动距离除以一个蛋杯的距离就是移动了多少个蛋杯，get_expansion_link_offeset/SEPARATE_CUP_distem
	// expansion_move_pluse 但是下面要用这个移动脉冲，这里这样算也是有必要？
	
	
	// expansion_move_cup = (SEPARATE_CUP_LEN/SEPARATE_CUP_distem)*get_expansion_link_offeset()/SEPARATE_CUP_LEN;
	// expansion_move_cup = (get_expansion_link_offeset()/SEPARATE_CUP_distem)
	
	// 通过分析得到伸缩杆从原点(100%)的位置移动到现在的位置，移动的蛋杯数
	
	
	expansion_move_cup = expansion_move_cup +1;
	if(expansion_move_cup>10)	// 算出来移动了10个蛋杯的距离，就报错，意思是不能移动这么多的距离
	{  
	  set_device_alarm(EXPANSION_LINK_ERR);
	}
	for(i=0;i<10;i++)		// 新绛设备是10条线线
	{
		// 举个例子
		// 某个通道要踢的蛋的蛋杯范围为
		for(egg_id=(throw_egg_id[i][1]-expansion_move_cup);egg_id>=(throw_egg_id[i][0]-expansion_move_cup);egg_id--)
		{
			 current_pluse = get_spt_encode_pluse();
			 if((i%2)==0)
			 {
				 // 通道A
				 calculate_spt_egg_down(0,i,egg_id,current_pluse,cup_open_time ,(uint16_t)expansion_move_pluse);
			 }
			 else 
			 {
				 // 通道B
			   calculate_spt_egg_down(1,i,egg_id,current_pluse,cup_open_time ,(uint16_t)expansion_move_pluse);
			 }
		}
	}
	

	

	for(egg_id=throw_cha_min_id;egg_id<=throw_cha_max_id;egg_id++)  //抛弃通道
	{
			 current_pluse = get_spt_encode_pluse();
		   throw_ch_tick_egg(0,12,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEA1_POSTION,cup_open_time);
			 throw_ch_tick_egg(0,13,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEA2_POSTION,cup_open_time);
			 throw_ch_tick_egg(0,14,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEA3_POSTION,cup_open_time);
	}
	
	for(egg_id=throw_chb_min_id;egg_id<=throw_chb_max_id;egg_id++)  //抛弃通道
	{
			 current_pluse = get_spt_encode_pluse();
		     throw_ch_tick_egg(1,15,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEB1_POSTION,cup_open_time);
			 throw_ch_tick_egg(1,16,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEB2_POSTION,cup_open_time);
			 throw_ch_tick_egg(1,17,egg_id,(uint16_t)expansion_move_pluse,THROW_LINEB3_POSTION,cup_open_time);
	}
}
void start_tick_eggs()
{
	uint8_t i;
	uint16_t egg_id;

	uint32_t current_pluse;
	
	throw_cha_max_id = THROW_LINEA3_POSTION/SEPARATE_CUP_LEN;
	throw_cha_min_id = THROW_LINEA1_POSTION/SEPARATE_CUP_LEN;
	
	throw_chb_max_id = THROW_LINEB3_POSTION/SEPARATE_CUP_LEN;
	throw_chb_min_id = THROW_LINEB1_POSTION/SEPARATE_CUP_LEN;
	
	throw_cha_max_id +=2;
	throw_cha_min_id -=6;
	
	throw_chb_max_id +=2;
	throw_chb_min_id -=6;
	
	close_throw_all_pin();
	rt_thread_mdelay(3);
	for(i=0;i<8;i++)   //通道A
	{
		// 轮询8个通道，有些是6个，有些是10个，有些是12个
		// 每个通道的踢蛋范围内，都踢一下？
		// throw_egg_id[i][0]	保存着该通道的开始踢蛋的分级线蛋杯号
		// throw_egg_id[i][1] 保存着该通道的结束踢蛋的分级线蛋杯号
		// 也就是分级线的蛋杯号走到了自己的范围内才踢
		for(egg_id=throw_egg_id[i][1];egg_id>=throw_egg_id[i][0];egg_id--)
		{
			  current_pluse = get_spt_encode_pluse();	// 注意，这个时候是在检查环境，分级线还没有跑起来
			 
			  start_retick_eggs((i%2),i,egg_id,current_pluse);		// egg_id这个是该通道相对于第一个蛋杯的蛋杯数
																														// current_pluse这个可以计算出当前相对于第一个蛋杯走的蛋杯数
		}
	}
  for(i=8;i<10;i++)
	{
	  for(egg_id=throw_egg_id[i][1];egg_id>=throw_egg_id[i][0];egg_id--)
		{
			  current_pluse = get_spt_encode_pluse();
			  start_retick_eggs(0,i,egg_id,current_pluse);
		}
	}
	for(i=10;i<12;i++)
	{
		for(egg_id=throw_egg_id[i][1];egg_id>=throw_egg_id[i][0];egg_id--)
		{
			  current_pluse = get_spt_encode_pluse();
			  start_retick_eggs(1,i,egg_id,current_pluse);
		}
	}
	for(egg_id=throw_cha_min_id;egg_id<=throw_cha_max_id;egg_id++)  //抛弃通道A
	{
			 current_pluse = get_spt_encode_pluse();
		     //throw_ch_start_retick_eggs(0,9,egg_id,THROW_LINEA1_POSTION,current_pluse);
			 //throw_ch_start_retick_eggs(0,10,egg_id,THROW_LINEA2_POSTION,current_pluse);
			 //throw_ch_start_retick_eggs(0,11,egg_id,THROW_LINEA3_POSTION,current_pluse);
	}
	
	for(egg_id=throw_chb_min_id;egg_id<=throw_chb_max_id;egg_id++)  //抛弃通道B
	{
			 current_pluse = get_spt_encode_pluse();
		 //  throw_ch_start_retick_eggs(1,13,egg_id,THROW_LINEB1_POSTION,current_pluse);
		//	 throw_ch_start_retick_eggs(1,14,egg_id,THROW_LINEB2_POSTION,current_pluse);
			// throw_ch_start_retick_eggs(1,15,egg_id,THROW_LINEB3_POSTION,current_pluse);
	}
}

 uint32_t printA_egg_count=0;
 uint32_t printB_egg_count=0;
void egg_print()
{

	 uint16_t id;
	 uint16_t eggid;
	 uint32_t current_pluse;
	 uint32_t egg_offest;
	 int less;
	 separate_egg_info_type *spt_egg;
	
	 current_pluse = get_spt_encode_pluse();
	 for(id=(print_device_info.cha.min_id);id<print_device_info.cha.max_id;id++)
	 {
	   get_pst_egg_info(0,id,&spt_egg,0);
	   if((spt_egg->wetght>0)&&(spt_egg->print_id>0)) //需要打印
		 {
		   egg_offest = current_pluse - spt_egg->pluse;    
			 less = print_device_info.cha.PRINT_PRITION - egg_offest;
			 if((less>0)&&(less<20))
			 {
				  if((print_device_info.cha.egg_id !=spt_egg->id))
					{
						print_device_info.cha.egg_id =spt_egg->id;
						if(print_device_info.cha.print_id !=spt_egg->print_id)
						{
							print_device_info.cha.print_id = spt_egg->print_id;
							set_print_id(0,spt_egg->print_id);
						}
						printA_egg_count++;
						set_en_print_egg(0);
					//	DEBUG_INFO("print cup id=%d print id=%d\n",print_egg_id,spt_egg->print_id);
					}
				  break;
			 }
		 }
	 }
	 current_pluse = get_spt_encode_pluse();

	 for(id=(print_device_info.chb.min_id);id<print_device_info.chb.max_id;id++)
	 {
	   get_pst_egg_info(1,id,&spt_egg,0);
	   if((spt_egg->wetght>0)&&(spt_egg->print_id>0)) //需要打印
		 {
		   egg_offest = current_pluse - spt_egg->pluse;    
			 less = print_device_info.chb.PRINT_PRITION - egg_offest;
			 //rt_kprintf("1=%d 2=%d  3=%d \n",EGG_PRINT_POSITION,egg_offest,spt_egg->id);
			 if((less>0)&&(less<20))
			 {
			    //DEBUG_CARE("print egg eid=%d  pid=%d  ch=%d\n",spt_egg->id,spt_egg->print_id,spt_egg->separate_ch);
				  if((print_device_info.chb.egg_id !=spt_egg->id))
					{
						print_device_info.chb.egg_id =spt_egg->id;
						if(print_device_info.chb.print_id !=spt_egg->print_id)
						{
							print_device_info.chb.print_id = spt_egg->print_id;
							set_print_id(1,spt_egg->print_id);
						}
						printB_egg_count++;
						set_en_print_egg(1);
					//	DEBUG_INFO("print cup id=%d print id=%d\n",print_egg_id,spt_egg->print_id);
					}
				  break;
			 }
		 }
	 }
}


#define POSITION_NL 1 											// 2026.04.09 qianxuping
#define POSITION_L 0  //l型  反L						// 2026.04.09 qianxuping
#define DEVICE_POSITION POSITION_NL					// 2026.04.09 qianxuping

uint16_t dirty_a_id[6]={0,0,0,0,0,0};
uint16_t dirty_b_id[6]={0,0,0,0,0,0};

extern uint32_t get_crack_info(uint8_t ch,uint8_t *dat);
void transport_egg_move(uint32_t count)
{
	uint8_t j;
	uint16_t i;
  //uint8_t crack_cha_dat[6];	// qianxuping 裂纹是和重量一起发过来的,这里将光检裂纹信息注释
	//uint8_t crack_chb_dat[6];	// qianxuping 裂纹是和重量一起发过来的,这里将光检裂纹信息注释
	ai_dat_type dirty_infoA,dirty_infoB;
	weight_and_crack_info weight_infoA,weight_infoB;
	
	get_dirty_info(0,&dirty_infoA);
	get_weight_info(0,&weight_infoA);
	//get_crack_info(0,crack_cha_dat);	// qianxuping 裂纹是和重量一起发过来的,这里将光检裂纹信息注释
	
	get_dirty_info(1,&dirty_infoB);
	get_weight_info(1,&weight_infoB);
  //get_crack_info(1,crack_chb_dat);	// qianxuping 裂纹是和重量一起发过来的,这里将光检裂纹信息注释
  for(i=(MAX_TRANSPORT_LINE_COUNT-1);i>0;i--)	// 将0--(MAX_TRANSPORT_LINE_COUNT-1)的数据移动到1--(MAX_TRANSPORT_LINE_COUNT)
	{													// 实际算的时候是weight_offset_count或者dirty_offest_count，他们两个是同一个数据，都是转载处的数据
														// 那么0-MAX_TRANSPORT_LINE_COUNT中那个才是转载的数据呢
														// 或者说不能这么理解，这4组数据都是单独的，只是称重的weight_offset_count位置和脏蛋的dirty_offest_count位置是同一个蛋的数据
														// 那么MAX_TRANSPORT_LINE_COUNT就至少要大于最大的偏移量的数据，而且只能大不能小
														// MAX_TRANSPORT_LINE_COUNT大了，相当于有后面一截是没有实际用处的，如果MAX_TRANSPORT_LINE_COUNT小了，最大的偏移量的数据就会丢失，因为dirty_offest_count位置就会始终没有数据，数据
														// 到MAX_TRANSPORT_LINE_COUNT就结束了
	
		for(j=0;j<6;j++)
		{
			// 第0排移动到第1排
			// 第1排移动到第2排，依次类推
			// 第0排不断地在接收新的数据
			// 将数据往后保存
			transport_egg_weight_infoA[i][j] = transport_egg_weight_infoA[i-1][j];
			transport_egg_crack_infoA[i][j] =  transport_egg_crack_infoA[i-1][j];
			transport_egg_colour_infoA[i][j] = transport_egg_colour_infoA[i-1][j];
			transport_egg_dirty_infoA[i][j] =  transport_egg_dirty_infoA[i-1][j];
			
			transport_egg_weight_infoB[i][j] = transport_egg_weight_infoB[i-1][j];
			transport_egg_crack_infoB[i][j] =  transport_egg_crack_infoB[i-1][j];
			transport_egg_colour_infoB[i][j] = transport_egg_colour_infoB[i-1][j];
			transport_egg_dirty_infoB[i][j] =  transport_egg_dirty_infoB[i-1][j];
		}
	}
	#if 0
	for(j=0;j<6;j++)
	{
			transport_egg_weight_infoA[0][j] = weight_infoA.weight[j];
		
			if(crack_cha_dat[j]==0)
			{
			  transport_egg_crack_infoA[0][j] =  0;
			}
			else 
			{
			  transport_egg_crack_infoA[0][j] =  crack_cha_dat[j];
			}
		 
		  if(dirty_infoA.body[j].is_egg ==0)
			{
				transport_egg_colour_infoA[0][j] = 100;
				transport_egg_dirty_infoA[0][j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
			  transport_egg_colour_infoA[0][j] = dirty_infoA.body[j].level;
				transport_egg_dirty_infoA[0][j] = dirty_infoA.body[j].dirty;
			}
	}
	
	for(j=0;j<6;j++)
	{
			transport_egg_weight_infoB[0][j] = weight_infoB.weight[j];
		  if(crack_chb_dat[j]==0)
			{
			  transport_egg_crack_infoB[0][j] =  0;
			}
			else 
			{
			  transport_egg_crack_infoB[0][j] =  crack_chb_dat[j];
			}
			
		 
			if(dirty_infoB.body[j].is_egg ==0)
			{
				transport_egg_colour_infoB[0][j] = 100;
				transport_egg_dirty_infoB[0][j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
			  transport_egg_colour_infoB[0][j] = dirty_infoB.body[j].level;
				transport_egg_dirty_infoB[0][j] = dirty_infoB.body[j].dirty;
			}
	}
	#else		// 2026.04.09 qianxuping
	for(j=0;j<6;j++)				// 然后将最新的鸡蛋信息存放在第0的位置处
														// 注意最新数据的鸡蛋实际还没有走到转载处，要分别走对应偏移位置才能走到转载
														// 举例，称重的偏移是50，脏蛋的偏移是80
														// 实际肯定脏蛋在距离转载80的地方就发出数据了，然后称重在距离转载50的地方就发出数据了
														// 脏蛋走了30到称重，称重然后发出在走50，就到转载了，对于转载来说，脏蛋的80和称重的50是同一个鸡蛋
	{
		if(DEVICE_POSITION==POSITION_L)
		{
			transport_egg_weight_infoA[0][j] = weight_infoA.weight[j];
			transport_egg_crack_infoA[0][j] = weight_infoA.crack[j];
			
		  if(dirty_infoA.body[j].is_egg ==0)				// 脏蛋没有
			{
				transport_egg_colour_infoA[0][j] = 100;	// 颜色和脏蛋都赋值为100
				transport_egg_dirty_infoA[0][j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
					// 如果有蛋则获取颜色等级和脏蛋等级
			    transport_egg_colour_infoA[0][j] = dirty_infoA.body[j].level;
				transport_egg_dirty_infoA[0][j] = dirty_infoA.body[j].dirty;
			}
		}
		else   //反向L型
		{
			transport_egg_weight_infoA[0][5-j] = weight_infoA.weight[j];
			transport_egg_crack_infoA[0][5-j] = weight_infoA.crack[j];
		 // rt_kprintf("i=%d chA is egg=%d dirty=%d clour=%d\n",i,dirty_infoA.body[j].is_egg,dirty_infoA.body[j].dirty,dirty_infoA.body[j].level);
			if(dirty_infoA.body[j].is_egg ==0)
			{
				transport_egg_colour_infoA[0][5-j] = 100;
				transport_egg_dirty_infoA[0][5-j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
			    transport_egg_colour_infoA[0][5-j] = dirty_infoA.body[j].level;
				transport_egg_dirty_infoA[0][5-j] = dirty_infoA.body[j].dirty;
			}
		}
	}
	
	for(j=0;j<6;j++)
	{
		if(DEVICE_POSITION==POSITION_L)
		{
		
			transport_egg_weight_infoB[0][j] = weight_infoB.weight[j];
			transport_egg_crack_infoB[0][j] = weight_infoB.crack[j];
		 
			if(dirty_infoB.body[j].is_egg ==0)
			{
				transport_egg_colour_infoB[0][j] = 100;
				transport_egg_dirty_infoB[0][j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
			  transport_egg_colour_infoB[0][j] = dirty_infoB.body[j].level;
				transport_egg_dirty_infoB[0][j] = dirty_infoB.body[j].dirty;
			}
		}
		else 
		{
		 
			transport_egg_weight_infoB[0][5-j] = weight_infoB.weight[j];
			transport_egg_crack_infoB[0][5-j] = weight_infoB.crack[j];
		  //rt_kprintf("i=%d chB is egg=%d dirty=%d clour=%d\n",i,dirty_infoB.body[j].is_egg,dirty_infoB.body[j].dirty,dirty_infoB.body[j].level);
			if(dirty_infoB.body[j].is_egg ==0)
			{
				transport_egg_colour_infoB[0][5-j] = 100;
				transport_egg_dirty_infoB[0][5-j] = 100;
			}
			else 
			{
				//DEBUG_INFO("recive dirty pluse=%d\n",get_spt_encode_pluse());
			    transport_egg_colour_infoB[0][5-j] = dirty_infoB.body[j].level;
				transport_egg_dirty_infoB[0][5-j] = dirty_infoB.body[j].dirty;
			}
		}
	}
	#endif
}
uint32_t spt_egg_count[2]={0,0};
//鸡蛋从传输线转到分离线 
//注意要修改每个鸡蛋的距离
//基准点是 0开始 1-6
static void transfer_egg_to_separate_line(uint32_t egg_pluse)  // 走到第三格子才上蛋
{
	uint8_t i;
	uint16_t posA;
	uint16_t posB;
	uint32_t line_count=0;
	separate_egg_info_type egg_info1;

	rt_memset(&egg_info1,0,sizeof(separate_egg_info_type));

    line_count = egg_pluse/1000;	// 根据当前编码器脉冲得到当前分级线是哪个6个
																	// 这样理解，在输送通道一排转到分级线就是6个蛋杯
																	// 把编码器所有脉冲值除以6个蛋杯的脉冲值1000，得到分级线有多少个6个蛋杯
																	// 这里算出来得到当前算的这6个输送通道的蛋是分级线哪个6个的蛋杯
	
	
	
	posA = spt_eg.cha_pos;
	posB = spt_eg.chb_pos;
	if(posA==0) posA =SEPARATE_CHA_CUP_NUMBER;
	if(posB==0) posB =SEPARATE_CHB_CUP_NUMBER;
	// 经过下面if( line_count == (load_egg_count +1))这么运算以及if(posA==0) posA =SEPARATE_CHA_CUP_NUMBER;if(posB==0) posB =SEPARATE_CHB_CUP_NUMBER;这样运算
	// 那么进入下面条件里面的posA只能是6,12，...，264,270(spt_eg.cha_pos只能是0-269，当是0的时候，这里有变为270了)
	
	
	if( line_count == (load_egg_count +1))	// 这个一定是要等在走了1000个脉冲才执行下面的
																					// 当读到还没有走到这么远说明还没有转一圈也就是还不能分线
	{
		transport_egg_move(line_count);				// 将蛋的信息往后移动，并且将最新的一排蛋信息存到最新的位置
																					// 下面会读取最老的数据并计算成为分离的数据保存在分蛋信息spt_eg里面
		//set_weight_and_crack_comm_err();
		load_egg_count = line_count;
		egg_pluse = line_count *1000;
		for(i=0;i<6;i++)
		{
			egg_info1.family_ch =10;			// 这里赋的值后面可能倍修改，calculate_egg_info函数会根据产品等等计算用哪个通道，是什么供应商等
			egg_info1.separate_ch =30;		// 这里赋的值后面可能倍修改，calculate_egg_info函数会根据产品等等计算用哪个通道，是什么供应商等
																		// 这个值也可能不会被修改，最后抛弃通道就会判断这个是不是30，因为不是30不会往抛弃通道跑

			// transport_egg_weight_infoA里面保存着输送通道发过来的称重信息
			// transport_egg_weight_infoA里面最大是 #define MAX_TRANSPORT_LINE_COUNT 110  这么多排的数据
			// 但是只从weightA_offset_count取数据(举个例子，称重数据新来保存在0,0往1移动，1往2，依次类推，越往后面来的时间越早数据越旧
			// 																		比如存了0-199个，我只要来得及我从100取数据也可以，实际相当于只用了前100个缓存区
			//                                    但是这个weightA_offset_count算出来比MAX_TRANSPORT_LINE_COUNT这个大的话，不是越界了吗？)
			// 依次读取这一排的6个位置看看是不是有重量，有重量说明有蛋
			if(transport_egg_weight_infoA[weightA_offset_count][i]>0)		// 这个是传输通道称重脉冲位置算出来的weightA_offset_count，意思就是称重信息发出来的位置的称重数据
			{																														// 保存的称重信息是#define MAX_TRANSPORT_LINE_COUNT 110  //最大200根这么多格子的数据
																																	// 要取的位置
				
				// 有蛋的话，A线的总蛋数量加1
				spt_egg_count[0]++;
				egg_info1.empty =NEED_TICK_EGG;		// 标记这个需要踢
				egg_info1.wetght = transport_egg_weight_infoA[weightA_offset_count][i];	// 并将该蛋的重量记下来，注意这个重量梁老师发过来扩大了10倍
				egg_info1.crack =  transport_egg_crack_infoA[crackA_offset_count][i];		// 并将该蛋的裂纹信息记下来
				
				egg_info1.colour = transport_egg_colour_infoA[dirtyA_offest_count][i];	// 并将该蛋的颜色信息记下来
				egg_info1.dity = transport_egg_dirty_infoA[dirtyA_offest_count][i];			// 并将该蛋的脏蛋信息记下来
			}
			else 
			{
				// 如果没有重量就表示该排的该位置没有蛋
			  egg_info1.empty =CUP_IS_EMPTY;
			  egg_info1.wetght = 0;
				egg_info1.crack =  transport_egg_crack_infoA[crackA_offset_count][i];
				egg_info1.colour = transport_egg_colour_infoA[dirtyA_offest_count][i];
				egg_info1.dity = transport_egg_dirty_infoA[dirtyA_offest_count][i];	
			}
			
			
			// 为什么这里 weightA_offset_count crackA_offset_count dirtyA_offest_count 这几个就是对应的输送通道同一排的蛋呢
			// 是什么原理呢？
			// 举个例子
			// 在设置的时候
			// weight_device=50000		// weightA_offset_count=500
			// ai_divice=81000				// dirtyA_offest_count=810
			// blood_device=81000			// dirtyA_offest_count=810
			// 那么我取数据的时候，对于我来说称重走500个到我这里和脏蛋走810个到我这里是一样的也即是同一个数据
			
			
			
			
			
			// 如果文件编号设置为99，表示调试信息，这个信息是随便写的
			// 比如跑机的时候没有真实的蛋，可以设置为99，主控就会认为有蛋，包装通道也会去踢蛋
			if(ReadMBHotRegU16(1283)==99)
			{
				egg_info1.empty =NEED_TICK_EGG;
				egg_info1.wetght = 500;
				egg_info1.crack = 3;
				egg_info1.colour = 20;
				egg_info1.dity = i;	
			}
			
		  egg_info1.id=posA -i;	//posA照道理应该是6，12，.... 270
			// 这个举例
			// 6	5	4	3	2	1
			// 270 269 268 267 226 265
		  egg_info1.pluse = egg_pluse - SEPARATE_CUP_LEN*(i+1);	// 这6个蛋是同时落在分级线的
																														// 那么此时这6个蛋在分级线上的脉冲是相当于当前脉冲值egg_pluse的位置不一样
																														// 也就是此时这6个蛋落在分级线的蛋的脉冲值不一样，第6列在后面
																														// 每个蛋依次差一个蛋杯的脉冲
		  set_spt_egg_info(0,i,&egg_info1);											// 将传输线一排的鸡蛋即6个鸡蛋算好的数据存到spt_eg分离鸡蛋数据中去
		}
		for(i=0;i<6;i++)
		{
			egg_info1.family_ch =10;
			egg_info1.separate_ch =30;
			
			if(transport_egg_weight_infoB[weightB_offset_count][i]>0)
			{
				spt_egg_count[1]++;
				egg_info1.empty =NEED_TICK_EGG;
				egg_info1.wetght = transport_egg_weight_infoB[weightB_offset_count][i];
				egg_info1.crack = transport_egg_crack_infoB[crackB_offset_count][i];
				egg_info1.colour = transport_egg_colour_infoB[dirtyB_offest_count][i];
				egg_info1.dity = transport_egg_dirty_infoB[dirtyB_offest_count][i];	
			}
			else 
			{
			   egg_info1.empty =CUP_IS_EMPTY;
				 egg_info1.wetght = 0;
				 egg_info1.crack = transport_egg_crack_infoB[crackB_offset_count][i];
				 egg_info1.colour = transport_egg_colour_infoB[dirtyB_offest_count][i];
				 egg_info1.dity = transport_egg_dirty_infoB[dirtyB_offest_count][i];	
			}
			if(ReadMBHotRegU16(1283)==99)
			{
				egg_info1.empty =NEED_TICK_EGG;
				egg_info1.wetght = 600;
				egg_info1.crack = 6;
				egg_info1.colour = 30;
				egg_info1.dity = i;	
			}
			
			  egg_info1.id=posB -i;
		    egg_info1.pluse = egg_pluse - SEPARATE_CUP_LEN*(i+1);
		    set_spt_egg_info(1,i,&egg_info1);
		}
    
		//DEBUG_INFO("load egg count=%d weight[1]=%d weight[2]=%d weight[3]=%d weight[4]=%d weight[5]=%d weight[6]=%d\n");
	}
}

static void spearte_timeout(void *parameter)
{

	 uint8_t  system_state;
	 uint8_t  product_id;
	 uint16_t count=0;
	 uint32_t cpluse;
	 uint16_t bad_egg;
	 rt_thread_mdelay(20);
	 get_production_mange_set(&per_poduction_egg);
	 load_backups_anaysis_dat();

   backup_read_alread_pruduct_dat(&alread_production);
	 for(product_id=0;product_id<12;product_id++)    //12个产品
	 {
		 if(per_poduction_egg.pre_production[product_id].empty !=0)  //不为空 这个产品要做
		 {
			 // 记录所有产品，用的是哪些包装线，以及这些包装线用电磁铁
			  init_avg_spt_egg_info(product_id,per_poduction_egg.pre_production[product_id].packge_line,per_poduction_egg.pre_production[product_id].tray_line);
		 }
	 }
	 while(1)
	 {
		 count++;
		 
		 // 还有个隐形的规律，里面是要到了每走一个蛋杯才会运行一次
		 cpluse = check_empty_cup_and_cup_move();   //先检查蛋杯位移在上蛋  要不然上蛋的位置会有错误
																								// 这里面会修改spt_eg.cha_pos的值
																								// 这个函数会计算当前走的蛋杯数，并赋值给spt_eg.cha_pos
																								// 然后里面还会从spt_eg分级线数据中的第200个位置去取的数据计算好后是哪个通道来踢等信息存回到spt_eg里面
																								// 然后里面还会从spt_eg分级线数据中的第12个位置去取的数据计算好后是哪个通道来踢等信息存回到spt_eg里面
		// 还有个隐形的规律，里面是要到了每走6个蛋杯才会运行一次	
			// 作用是传输线每走一格，分级线走了6格，所以是将传输线一格(一格6排共6个蛋)的蛋放到分级线上
		 transfer_egg_to_separate_line(cpluse);  // 这个会将脏蛋，裂纹，重量数据等里面的数据转换为当前输送通道要分离的数据并保存在spt_eg里面
		 
		 // 也就是算6个，转换一个输送通道的数据(输送通道转换一次就有6个数据)
		 
		 
		 if(count==1000)	// 3s保存一下
		 {
			 bad_egg = get_bad_egg_count();
			 alread_production.today_egg_count += bad_egg;
			 save_backups_anaysis_dat();	// 保存一下已经分析好的质量分析信息
			 backup_save_alread_pruduct_dat(&alread_production);  //存储已经生产的历史
			 count=0;
		 }
		 egg_print();
		 system_state = get_system_state();
		 switch(system_state)
		 {
			case SYS_IS_CHECK_ENV:
				sys_check_env();		// 启动前都要从新读取预生产信息
			  clear_throw_ch_egg_err_count();
				break;
			case SYSTEM_IS_RUING:
				tick_egg();								// 计算踢蛋信息，并将踢蛋信息发送到相应的通道
			  per_min_egg_dat_move();		// 并且计算1分钟内抛弃通道的抛弃的数量是不是大于屏幕设定的抛弃数量(1分钟内超过设定允许的抛弃数量则停机)
				break;
			case SYSTEM_IS_STOP:
				tick_egg();				// 计算踢蛋信息，并将踢蛋信息发送到相应的通道
				break;
			default:			// 像空闲状态则什么都不做
				break;
		 }
		 rt_thread_mdelay(3);
	 }
}



static int spearte_egg_init(void)
{
    rt_thread_t  tid1;
	  spt_egg_info_init();
	  
		tid1 = rt_thread_create("SPT",spearte_timeout, RT_NULL,4096,9, 20);
    if (tid1 == RT_NULL) return RT_ERROR;
		rt_thread_startup(tid1);
    return RT_EOK;
}
INIT_APP_EXPORT(spearte_egg_init);




