
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "uadc.h"
#include "udac.h"
#include "relay.h"
#include "E6B2.h"
#include "user_set.h"
#include "move_control.h"
#include "egg_control.h"
#include "drv_key.h"
#include "mdebug.h"
#include "modbus_screen.h"
#include "common.h"
#include "product_control.h"

#define MOVE_CYCLE_TIME 5
#define MOVE_CYCLE_1S 1000/MOVE_CYCLE_TIME
#define expansion_link_min_res   2000  //欧姆
#define expansion_link_max_res   7000  //欧姆
#define expansion_link_len       50   //欧姆每mm
#define min_separate_speed 5
#define shake_filter_val  2  //上下抖动 单位 mm/s
#define expansion_link_shake_filter 2  // 有震动 必须滤波  单位mm
#define expansion_link_forward_speed 1440  //um  1440um/ms? 1.44mm/ms?  ---都不对，实际用的值是mm，并且扩大了100，所以这里实际是14.4mm/s，并且看推杆的资料也是13mm/s
																						// 但是为什么要乘以100或者扩大100，是因为单片机可以以ms运行，这样精度更高？
#define expansion_link_back_speed 1330  //um  1.33mm

#define EXPANSION_LINK_STOP 0
#define EXPANSION_LINK_FORWARD 1
#define EXPANSION_LINK_BACK    2

ALIGN(RT_ALIGN_SIZE)

static uint8_t product_speed=0;
static uint16_t expansion_link_dist=0;

typedef struct 
{
	float start_res;
	float stop_res;
	float mmmeter_per_res;     	// 伸缩1mm 对应的电阻
	rt_tick_t start_time;
	uint8_t dir;             		// 方向
	uint8_t excues_flag;    		// 启动标志
  uint32_t offest_dist;  	 		// 实际偏移位置
	uint32_t except_position; 	// 期望位置
}expansion_link_type;  				// 伸缩杆


static expansion_link_type expansion_link={
	expansion_link_min_res,
	expansion_link_max_res,
	expansion_link_len,
	0,
	0,
	0,
	0,
	0,
}; //伸缩杆


//3000 - 60CM=600MM
// 3103 最低刻度
//230 最高
//R=0,Vout=[3.3*10/(R+10+10)-3.3*10/30]*6=3.3V
uint16_t get_expansion_link_res()
{
	float votage=0;
	float res_tmp;
	uint16_t res=0;
	
	votage = get_adc_votage();	
	res_tmp = (198.0/(votage+6.6))-20;
	res =  (uint16_t) (res_tmp*1000); 
	return res;
}

uint16_t get_expansion_link_offeset()
{
  
  return expansion_link.offest_dist/100;		// 设置这个距离的时候扩大了100倍，这里要缩小100倍
}

uint8_t calculate_run_speed()
{

	uint8_t speed;
	uint32_t  egg_per_hour_count;
	
	egg_per_hour_count =  get_screen_set_val_type32(2823)/2;	// 获取到设置的生产速度多少枚/每个小时，这里除以2是因为72000的是2条线
	speed = egg_per_hour_count/1000;													// 为什么除以1000，难道哪里乘以了1000？
																														// 在调用这个函数得到的速度又乘以了一个1000
	if(speed==0) speed=10;
  return speed;
}

static uint16_t adj_speed_count=0;

static uint16_t set_product_speed=0;



void set_expansion_link_move(uint8_t dir,uint16_t position)
{
  expansion_link.dir = dir;
	expansion_link.except_position = position;		// 设置的时候是毫米并且扩大了100倍
	expansion_link.excues_flag =1;
}
void expansion_link_move()
{
  	uint32_t run_time;
	  float offest_dist=0; 
	
  	if(expansion_link.excues_flag==1)
	  {
	   if(expansion_link.dir == EXPANSION_LINK_FORWARD)
		 {
			 DEBUG_INFO("伸缩杆前进\n");
		   expansion_link_down();
		 }
		 else if(expansion_link.dir == EXPANSION_LINK_BACK)	// 后退回到原点，对于伸缩杆来说原点的时候实际是伸开的而且是100%的位置
		 {
			 DEBUG_INFO("伸缩杆后退\n");
		   expansion_link_up();
		 }
		 else 
		 {
			 DEBUG_INFO("伸缩杆停止\n");
		   expansion_link_stop();
		 }
		 expansion_link.start_time = rt_tick_get();
		 expansion_link.excues_flag=2;
	  }
	  else 
	  {
			if(expansion_link.excues_flag==2)
			{
				run_time = count_use_time(expansion_link.start_time);	// 这个得到的移动的时间是多少ms
				expansion_link.start_time = rt_tick_get();
				if(expansion_link.dir == EXPANSION_LINK_FORWARD)
				{
					run_time = run_time * expansion_link_forward_speed;  // 时间乘以速度，这个速度是14.4mm/s得到距离，得到的是mm(但是时间的单位是s)
					offest_dist = run_time/1000;  //1000 ms								// 这个距离转换为mm
					//offest_dist = run_time/10;  // um  转为 mm
					//rt_kprintf("offest=%d add=%d\n",expansion_link.offest_dist,(uint16_t)offest_dist);
					expansion_link.offest_dist = expansion_link.offest_dist + ((uint16_t)offest_dist);	// 伸展，是当前的距离加上当前伸展的距离
					
				}
				else if(expansion_link.dir == EXPANSION_LINK_BACK)
				{
					run_time = run_time * expansion_link_back_speed;
					offest_dist = run_time/1000;  //1000 ms		// 得到回退的距离
					//offest_dist = run_time/10;  // um  转为 mm
					//rt_kprintf("offest=%d add=%d\n",expansion_link.offest_dist,(uint16_t)offest_dist);
					if(expansion_link.offest_dist>((uint16_t)offest_dist))	// 如果当前的距离大于回退了的距离，则当前的距离为上一次的距离减去回退的距离
					{
					  expansion_link.offest_dist = expansion_link.offest_dist - ((uint16_t)offest_dist);
					}
					else 
					{
					  expansion_link.offest_dist =0;		// 否则退到了最极限了
					}
				}
				// 如果当前伸展的位置到了期望的位置了，则停止伸缩了
				if((expansion_link.offest_dist  >= expansion_link.except_position)&&(expansion_link.dir ==EXPANSION_LINK_FORWARD))
				{
					// 则停止伸缩了
					 expansion_link_stop();
					 expansion_link.excues_flag =0;
					 DEBUG_CARE("伸长已到位\n");
				}
				// 如果当前退回的位置到了期望的位置(这个期望的位置不是起点)了，则停止伸缩了
				if((expansion_link.offest_dist <= expansion_link.except_position)&&
					(expansion_link.dir ==EXPANSION_LINK_BACK)&&(expansion_link.except_position >0))
				{
					 expansion_link_stop();
					 expansion_link.excues_flag =0;
					 DEBUG_CARE("收缩已到位\n");
				}
				// 如果当前是退回方向，并且希望退回到起点位置，则判断传感器是不是到了起点？
				if((expansion_link.dir ==EXPANSION_LINK_BACK)&&(expansion_link.except_position ==0))
				{
				  if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)
					{
						expansion_link_stop();
						expansion_link.offest_dist =0;
						expansion_link.excues_flag =0;
					  DEBUG_CARE("收缩已到原点\n");
					}
				}
		  }
	  }
}
// 设备在运行状态的时候调用
// 根据屏幕设置的生产速度计算得到伸缩杆应该的位置move_dist(主要是屏幕上的速度是有可能在生产的过程中被修改了，但是这个速度product_speed是检查环境的时候就设定的呀，也不是中间实时获取的呀)
// 然后获取伸缩杆在每次开始运行的时候设定的位置e_offest，但是这个位置指运行的

// 检查环境的时候根据屏幕设置的生产速度计算得到伸缩杆应该的位置move_dist
// 环境检测完后进入运行状态，设备会将这个屏幕设置的生产速度转换为伸缩杆应该到的位置，但是移动这个距离不是说等着一步到位，而是需要一个时间移动，所以e_offest这个实时位置可能和应该的位置move_dist不一样
// 所以这个应该的位置move_dist和当前实时位置e_offest可能不一样

// 当实时的位置还没有到应该到的位置
// 则计算当前伸缩杆位置反推的分级线的蛋速
// 如果反推出的蛋速小于等于设置的蛋速，就将这个反推出的蛋速扩大10倍(我看好像根据伸缩杆位置算出的速度设置给电机都扩大了10倍，是因为电机设置函数需要扩大10吗)设置为主电机？让电机快速跑起来？

// 等到伸缩杆到了应该的位置move_dist，这个时候就要实时调整电机的速度，因为电机跑的速度才能真正体现出屏幕设置的速度
// 1	因为没有到的时候，电机是扩大了10倍在运行
// 2  设备在运行的时候可能并不是准确的按照设定的速度在运行？
void expansion_link_control_up()
{
	uint16_t c_speed=0;
	uint16_t e_speed=0;
	uint16_t e_offest=0;
	uint32_t run_time;
	int16_t set_speed;
	uint32_t spt_speed;
	uint32_t egg_speed;
	float move_dist=0;

	float dat_buf=0;
	rt_base_t level;
	
  spt_speed = product_speed*1000;			// product_speed这个速度是屏幕设置的速度，在获取的时候除以了1000，所以这里乘以1000？(这个速度是1条线的速度)
																			// 在计算product_speed这个速度的时候除以了1000
																			// 注意这个速度是屏幕设置的速度，有可能实际的产能速度比这个大或者小，所以实际速度要实时调整来满足设置的生产速度？
																			// product_speed的范围为0-36对应变频器频率0-36hz
	spt_speed = spt_speed*635;					// 一个蛋杯是63.5mm,这里乘以635是10个蛋杯
	spt_speed = spt_speed/10;						// 这里除以了一个10，是因为上面乘以的是10个蛋杯
	spt_speed = spt_speed/3600;  //毫米/s
	
	move_dist = (EGG_FALL_DOWN_TIME/1000.0)*spt_speed;		//算出当前落下时水平方向走的距离，单位为毫米mm
	// 这个速度是指踢蛋的时候，从踢到落到蛋托里面的时候这段时间，水平方向走的距离
	// 这个水平的距离也就是要提前踢的距离？
	
    expansion_link_move();	// 这里面就是如果有伸缩杆需要运行到指定位置的就会执行，没有就不执行
														
	e_offest = get_expansion_link_offeset();	// 获取上一次的或者之前保存的落下时水平方向走的距离
																						// 获取当前伸缩杆的位置，这个距离是通过运行时间乘以伸缩杆运行的速度计算得到的，不是屏幕的速度得到的
																						// 这个位置就是踢蛋的时候提前踢的位置？
																						// 比如一个蛋要提前16mm踢，那么伸缩杆带着电磁铁就要走到16mm的位置，这个时候来了蛋，就直接打开电磁铁踢蛋？
	// 等到伸缩杆到了应该的位置move_dist，这个时候就要实时调整电机的速度，因为电机跑的速度才能真正体现出屏幕设置的速度
	// 1	因为没有到的时候，电机是扩大了10倍在运行
	// 2  设备在运行的时候可能并不是准确的按照设定的速度在运行？
	if(e_offest>=((uint16_t)move_dist))  //修正速度		// 如果当前速度计算得到的提前提蛋距离小于等于伸缩杆的位置距离，说明伸缩杆跑远了？还是跑近了？
	{
		// 调整主电机的速度就要根据
		egg_speed = get_avg_per_hour_eggs();					// 获取实际计算出的速度
		uint32_t o_speed;
	  o_speed = get_screen_set_val_type32(2823)/2;	// 获取设置的速度
		adj_speed_count++;
		// 比较实际电机跑的速度和设置的速度
		if((egg_speed+200)<o_speed)	// 如果实际的产蛋速度的速度小于设置的速度
		{
			if(adj_speed_count>300)		// 连续300次实际的产蛋速度的速度小于设置的速度则进行调整
			{
				adj_speed_count =0;
			  set_product_speed+=1;		// 速度加1(这个是指0-10v这个加1，还是0-1000mm/s加1)
			  if(set_product_speed>500)
			  {
				set_product_speed=500;
			  }
				//DEBUG_WARN("ste adj speed=%d\n",set_product_speed);
		    set_main_moto_speed(set_product_speed);
			}
		}
		else if(egg_speed>(o_speed+200))	// 如果实际的速度大于设置的速度
		{
			if(adj_speed_count>300)					// 连续300次实际的速度大于设置的速度(小于和大于的次数都用同一个变量，有没有问题)
			{
				adj_speed_count =0;
				set_product_speed-=1;					// 速度减1(这个是指0-10v这个减1，还是0-1000mm/s减1)
				//DEBUG_WARN("ste adj speed=%d\n",set_product_speed);
		    set_main_moto_speed(set_product_speed);
			}
		}
	}
	// 当实时的位置还没有到应该到的位置
	// 则计算当前伸缩杆位置反推的分级线的蛋速
	// 如果反推出的蛋速小于等于设置的蛋速，就将这个反推出的蛋速扩大10倍设置为主电机？让电机快速跑起来？
	else 
	{
		c_speed = get_spt_speed();
		e_speed = (uint16_t)(e_offest / (EGG_FALL_DOWN_TIME/1000.0));  // mm/s  通过伸缩杆的位置和落的时间，得到分级线水平的运行速度(也就是这么大的速度乘以落的时间才得到落的时候水平移动的位置也即伸缩杆要达到的提前踢蛋的位置)
		dat_buf = e_speed/17.6;		// 这个速度除以17.6怎么就得到了设定的速度了
															// 是不是下面：// 速度 加减 1 对应  传输线 加减 17mm/s
															// 除以17.6得到就是有多少个17.6，因为速度加减1就是17.6，那么相当于得到就是多少速度
		set_speed  = (int)(dat_buf);
		if(set_speed<5) set_speed =5;
		//rt_kprintf("c_speed=%d e_speed=%d\n",c_speed,e_speed);
		if(set_speed<=product_speed)
		{
			set_main_moto_speed(set_speed*10);
			set_product_speed = set_speed*10;		// 记录下这个设置的主电机速度
		}
		//DEBUG_CARE("set moto speed=%d \n",set_speed);
		rt_thread_mdelay(50);
	}
}

//速度 加减  1 对应  鸡蛋个数 加减 1000/小时   
// 速度 加减 1 对应  传输线 加减 17mm/s

// 在停止状态
// 设备在停止状态的时候调用
// 伸缩杆会一直伸开到原点，到了原点然后停下
// 伸缩杆停下的时候，主控电机在下面并没有停止，然后一直等待编码器的z相被检测到，即说明是等待到分级线的原点来到
// 检测到z相后，将主电机停下
void expansion_link_control_down()
{
	uint16_t c_key;
	uint16_t res=0;
	uint16_t inc_res;
	uint16_t c_speed=0;
	uint16_t e_speed=0;
	uint16_t e_offest =0;
	uint16_t set_speed;
	float offest_dist=0;  
	float forward_dist=0;   //
	float dat_buf;
  rt_base_t level;
	
	expansion_link_move();
	e_offest =  get_expansion_link_offeset();
	if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)
	{
		expansion_link_stop();
		
		c_key = (rt_pin_read(ENCODER3_PHASE_Z)&&1);		// 伸缩杆停下的时候，主控电机在下面并没有停止，然后一直等待编码器的z相被检测到，即说明是等待到分级线的原点来到
		if(c_key==0)
		{
		 // set_main_moto_speed(15);
		 // if(ge_key_val(EGG_COUNT_SENSER) !=PRESS)
		 // {
		    set_main_moto_speed(0);										// 检测到z相后，将主电机停下
																									// 编码器有个线程一直在算电机的实时速度
																									// product_control也就是系统控制中，停止状态下会一直检测主电机速度是不是为0，为0了则设备就如到空闲状态
		 // }
		}
		return;
	}
	else 
	{
	  expansion_link_up();
	}
	c_speed = get_spt_speed();	// 执行这个后没有用这个变量
	e_speed = (uint16_t)(e_offest / (EGG_FALL_DOWN_TIME/1000.0));  // mm/s		// EGG_FALL_DOWN_TIME/1000.0这个是毫秒转换为秒
																															// 伸缩杆的距离除以时间就是分级线的速度
																															// 因为伸缩杆的距离就是通过分级线的速度乘以落蛋时间得到的
																															// 举例92.075mm，距离除以时间0.145s得到速度635mm/s
																															// 那么这个速度减一加一对应的鸡蛋个数 加减 1000/小时  ？传输线 加减 17mm/s？
																															// 鸡蛋速度 	0 - 36000个/时
																															// 变频器速度	0 - 50hz，但是是不是虚拟的36hz就转换为实际的50hz？
																															// 电压				0 - 5v
																															// 传输线速度	0 - 635mm/s
																															// GP8401			0 - 4095(65520)实际设置的是扩大了16倍的数据65520
																															// 635/36=17.6，即1hz就是17.6
																															// 635/50=12.7，即1hz就是12.7
																															// 他是不是为了方便将36hz比例扩大到50hz，因为一个小时是36000，那么1hz对应就是36000/36=1000个鸡蛋
    dat_buf = e_speed/17.6;																		// 除以17.6得到就是有多少个17.6，因为1个速度对应1个17.6，那么相当于得到多少个17.6mm/s
	set_speed  = (int)(dat_buf);
	//rt_kprintf("c_speed=%d e_speed=%d\n",c_speed,e_speed);
	if(set_speed<6) set_speed =6;
	set_main_moto_speed(set_speed*10);
	//DEBUG_CARE("set moto speed=%d \n",set_speed);
	
	rt_thread_mdelay(5);
}

//调试便频器

static rt_tick_t up_run_time=0;
void debug_frequency_motor(uint8_t mode,uint8_t speed,uint8_t state)
{
	// 点动
	if(mode==0)
	{
		// 两次点动的点击间隔大于等于100ms
		if((rt_tick_get() - up_run_time)<100)
		{
		  return;
		}
		frequency_moto_enable();				// 主电机使能
		set_main_moto_speed(speed*10);	// 主电机运行(0-33)*10
		rt_thread_mdelay(800);					// 运行800ms
		frequency_moto_disable();				// 主电机关闭
		set_main_moto_speed(0);					// 主电机速度设置为0
		up_run_time = rt_tick_get();
	}
	else 
	{
		if(state>0)
		{
			frequency_moto_enable();				// 主电机使能
			set_main_moto_speed(speed*12);	// 主电机运行(0-33)*12
		}
	  else 
		{
		  frequency_moto_disable();				// 主电机关闭
			set_main_moto_speed(0);					// 主电机速度设置为0
		}
	}
}

static uint8_t expansion_to_origin_setp=0;
// 伸缩杆到原点，先左右伸缩，最后才确定为原点
void expansion_link_to_orgin()
{
	 if(expansion_to_origin_setp==0)
	 {
		if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)	// 伸缩杆最远的时候(100%的时候)是原点
		{
			expansion_to_origin_setp=1;											// 如果检测到到了100%的位置，进入步骤1
		}
		else 
		{
		  expansion_link_up();														// 如果没有检测到到了原点则伸展伸缩杆，这个一次只走30ms，并且步骤还是不变，下次还是执行这个步骤
		}
	 }
	 else if(expansion_to_origin_setp==1)
	 {
		if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)	// 这个步骤，怎么又检测原点，而且检测到还回收呢
		{																									// 回收到刚刚检测不到的位置，就进入步骤2
			expansion_link_down();
		}
		else 
		{
		  expansion_to_origin_setp=2;
		}
	 }
	 else if(expansion_to_origin_setp==2)												// 进入步骤2，又伸开
	 {
		if(ge_key_val(EXPONSION_LINK_TOP_SENSER) ==PRESS)					// 伸开到检测到原点则伸缩杆停止，置为原点标志为3，并且清空伸缩杆数据
		{
			expansion_link_stop();
			rt_thread_mdelay(300);
			expansion_link.start_res = get_expansion_link_res();		// 这个没有使用
			expansion_to_origin_setp=3;
			
			expansion_link.dir =0;
			expansion_link.excues_flag =0;
			expansion_link.offest_dist=0;
			expansion_link.except_position=0;
		}
		else 
		{
		  expansion_link_up();
		}
	 }
}
uint8_t get_expansion_link_to_orgin_state(void)
{
   if(expansion_to_origin_setp==3)
		 return 1;
	 else 
		 return 0;
}

void sys_start_clalure_expansion_link_offest()
{

	uint32_t spt_speed;
	uint16_t offest_dist=0;
	float buf;
  spt_speed = get_screen_set_val_type32(2823)/2;	// 一条线的速度
	spt_speed = spt_speed*635;
	spt_speed = spt_speed/10;												// 速度乘以每个蛋杯的距离
	spt_speed = spt_speed/3600;  //毫米/s
	buf = EGG_FALL_DOWN_TIME/1000.0;								// 鸡蛋掉落时间除以1000是什么意思,这个是将毫秒转换为妙
	buf = buf*spt_speed;
	offest_dist = (uint16_t)buf;
	
	// 伸缩的距离=时间乘以速度
	// 正常的计算
	// 例如屏幕设置72000个每小时
	// 72000/2是一条线的速度
	// 72000/2/3600是一条线的速度，这个是多少枚鸡蛋枚秒
	// 一枚鸡蛋的蛋杯距离是63.5mm，那么多少枚鸡蛋每秒换算为距离就是72000/2/3600*63.5这么多mm/s
	// 下降的时间 EGG_FALL_DOWN_TIME(为什么是EGG_FALL_DOWN_TIME/1000，因为EGG_FALL_DOWN_TIME这个是ms要算成秒)
	// 时间乘以速度就是开始落的时刻到落下的时刻这段时间水平方向走的距离，单位为mm
	
	// 那么举个例子，看看他算的和正常算的是不是一样
	// 36000*635/10/3600*(145/1000)=92.075
	// 36000/3600*63.5*(145/1000)=92.075
	
	
	
	// offest_dist 距离范围值
	// 0 -- 72000/2/3600即最大速度每秒10个格子，格子转换为距离即 10*63.5=635mm/s，时间145/1000=0.145s，那么这个距离是635*0.145=92.075mm，即按照我们产品的速度
	// 伸缩杆最大移动92.075mm
	
	
  set_expansion_link_move(1,offest_dist*100);	// 设置这个距离的时候扩大了100倍，
	DEBUG_INFO("伸缩杆偏移量为%d  spt_speed=%d offest=%d\n",offest_dist,spt_speed,(uint16_t)(offest_dist*10));
}
// 伸缩杆根据状态动态调整位置
// 这个会在产品控制里面每隔10ms调用一次
void expansion_link_control(uint8_t state)
{
		switch(state)
		{
			case SYSTEM_IS_IDLE:
			//	expansion_link_move();
				break;
			case SYS_IS_CHECK_ENV:
				product_speed = calculate_run_speed();					// 检查环境的时候是一直在检查，一直要等伸缩杆回到原点了才会进入到运行状态
			  //set_expansion_link_move(1,product_speed);
				expansion_link_to_orgin();
				break;
			case SYSTEM_IS_RUING:
				expansion_link_control_up();
				break;
			case SYSTEM_IS_STOP:
        expansion_link_control_down();
				break;
			case SYSTEM_IS_EMERGENCY_STOP:
				expansion_link_stop();
				break;
		  default:
				break;
		}
		if(state !=SYS_IS_CHECK_ENV)
		{
			expansion_to_origin_setp=0;
		}
}


void test_moto_fun(uint16_t time,uint8_t dir)
{

   if(dir==0)
	 {
	   expansion_link_down();
	 }
	 else 
	 {
	   expansion_link_up();
	 }
   rt_thread_mdelay(time);
   expansion_link_stop();
}



