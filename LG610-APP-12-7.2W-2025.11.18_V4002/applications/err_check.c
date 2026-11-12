
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "E6B2.h"
#include "common.h"
#include "mdebug.h"
#include "drv_key.h"
#include "err_check.h"
#include "user_set.h"
#include "egg_control.h"
#include "device_alarm.h"
#include "math.h"
#include "move_control.h"
#include "product_hostroy.h"
#include "app_screen.h"

ALIGN(RT_ALIGN_SIZE)

//
extern uint32_t EMPTY_CUP_SENSER_OFFEST_PLUSEA ;	// 检测空蛋杯的脉冲位置
extern uint32_t EMPTY_CUP_SENSER_OFFEST_PLUSEB ;

extern uint8_t find_first_cup_flag;
extern alread_production_type alread_production;
static uint16_t electromagnet_err_count_CHA[43];  //电磁铁错误统计
static uint16_t electromagnet_err_count_CHB[43];  //电磁铁错误统计
static uint16_t cup_err_count_CHA[MAX_CUP_COUNT+1];	// A线每个蛋杯的错误
static uint16_t cup_err_count_CHB[MAX_CUP_COUNT+1];	// B线每个蛋杯的错误
static uint32_t up_pluse_A=0;
static uint32_t up_pluse_B=0;

static uint16_t throw_ch_per_min_eggg_count[9][60+10];  //最大10分钟 10*60



void count_per_min_egg(uint8_t ch) //通道
{
	rt_base_t level;
    rt_tick_t time;
	uint16_t pos;
	uint16_t what_time;
	
	throw_ch_product_type throw_ch_product;
	
	read_product_throw_ch_part(&throw_ch_product);
    if(ch>=8)
		return;
	

	what_time = 60;

	time = rt_tick_get();  
	time = time/1000;
	pos = time%what_time;
	level = rt_hw_interrupt_disable();
    throw_ch_per_min_eggg_count[ch][pos]++;
	rt_hw_interrupt_enable(level);
	//DEBUG_CARE("recive egg ch=%d pos=%d\n",ch,pos);
}
static uint16_t per_min_pos_flag[9]={0,0,0,0,0,0,0,0,0};
static uint8_t time_1s_flag=0;
static uint16_t stop_time[9]={0,0,0,0,0,0,0,0,0};
static uint32_t stop_egg_count[9]={0,0,0,0,0,0,0,0,0};

void clear_throw_ch_egg_err_count()
{

  rt_memset(throw_ch_per_min_eggg_count,0,sizeof(throw_ch_per_min_eggg_count));
}
// 运行的时候，会不停地计算踢蛋信息
// 往抛弃通道踢蛋的时候，该时刻的秒钟在60中的位置的抛弃数量加1
// 这个函数会每秒钟将该时刻的秒钟在60中的位置的抛弃数量置为0
// 这个置为0是置为到哪个60个的哪个位置呢，上面加1的位置，还是加1后面的位置，还是加1前面的位置

// 举例，当某个时刻有个抛弃通道的16位置有抛弃的蛋，16则为1
// 紧接着会调用这个函数，这个函数的时刻是加了1000即算的是蛋杯又跑了1s中的，可能是17位置？并将该17位置变为0，下一次就是第18个位置，并将该18位置变为0
// 这个就相当于每秒钟一次将0-59的位置清零，供前面抛弃通道又数据的时候来设置
// 推出了是提前清零了未来的位置，当时间来了的时候并且抛弃通道与抛弃数据，就可以置为1
// 并且这个清零的位置永远是最老的位置，如果要置位也是置位清零后的最老的位置
void per_min_egg_dat_move()
{
	uint16_t i;
    rt_tick_t time;
	uint16_t max_pos;
	
	uint32_t egg_count=0;
	
	throw_ch_product_type throw_ch_product;
	
	
    time = rt_tick_get()+1000;  //提前1S清除数据
	time = time/1000;		//得到多少秒
	time = time%2;			// 这个值可能：0 1 0 1 0 1...
	// 根据算出来，每秒钟就执行一次
	if(time_1s_flag !=time)	// flag	0 0	1	0 1
	{												// time 0 1	0 1 0
		time_1s_flag = time;	//				x	x	x x 				
		read_product_throw_ch_part(&throw_ch_product);		// 获取抛弃通道的数据？这个数据是什么时候设置的，中间有没有被修改，修改了哪些？
		time = rt_tick_get();
		time = time/1000;		// 得到当前系统的秒数

		stop_egg_count[0] = throw_ch_product.cha[0].abnormal_stop_count;
		stop_egg_count[1] = throw_ch_product.cha[1].abnormal_stop_count;
		stop_egg_count[2] = throw_ch_product.cha[2].abnormal_stop_count;
		stop_egg_count[3] = throw_ch_product.force_throwa_stop_count;
		
		stop_egg_count[4] = throw_ch_product.chb[0].abnormal_stop_count;
		stop_egg_count[5] = throw_ch_product.chb[1].abnormal_stop_count;
		stop_egg_count[6] = throw_ch_product.chb[2].abnormal_stop_count;
		stop_egg_count[7] = throw_ch_product.force_throwb_stop_count;
		
		max_pos = time% 60;	// 将秒数除以60，得到这个秒数在1个分钟数的哪个位置，举例，118秒，这个就该算在0-59的58这个位置
		throw_ch_per_min_eggg_count[0][max_pos]=0;
		throw_ch_per_min_eggg_count[1][max_pos]=0;
		throw_ch_per_min_eggg_count[2][max_pos]=0;
		throw_ch_per_min_eggg_count[3][max_pos]=0;  
		throw_ch_per_min_eggg_count[4][max_pos]=0;
		throw_ch_per_min_eggg_count[5][max_pos]=0;
		throw_ch_per_min_eggg_count[6][max_pos]=0;
		throw_ch_per_min_eggg_count[7][max_pos]=0;
		throw_ch_per_min_eggg_count[8][max_pos]=0;
		//DEBUG_CARE("CLEAR CH1=%d CH2=%d CH3=%d CH4=%d\n",max_pos[0],max_pos[1],max_pos[2],max_pos[3]);
		
		for(int j=0;j<8;j++)
		{
		  stop_time[j]=60;		// 就是设置了一个数字60，下面用来比较，要算60次
		}
		for(int j=0;j<8;j++)	// 计算某个通道
		{
			egg_count =0;
			for(i=0;i<stop_time[j];i++)		// 计算该通道1分钟的数据
			{
				egg_count += throw_ch_per_min_eggg_count[j][i];		// 计算1分钟内的抛弃通道抛弃的蛋数
				if(egg_count>stop_egg_count[j])										// 某个通道1分钟内抛弃的数量大于设定的数量则报警
				{
				  rt_kprintf("egg_count=%d stop_egg_count=%d\n",egg_count,stop_egg_count[j]);
			      set_device_alarm(THROW_CH_EGG_ERROR);
				}
			}
			//rt_kprintf("ch%d=%d\n",j,egg_count);
		}
	}
}

int init_check_err_dat()
{
   rt_memset(throw_ch_per_min_eggg_count,0,sizeof(throw_ch_per_min_eggg_count));
   rt_memset(electromagnet_err_count_CHA,0,sizeof(electromagnet_err_count_CHA));
	 rt_memset(electromagnet_err_count_CHB,0,sizeof(electromagnet_err_count_CHB));
   rt_memset(cup_err_count_CHA,0,sizeof(cup_err_count_CHA));
	 rt_memset(cup_err_count_CHB,0,sizeof(cup_err_count_CHB));
	 return 0;
}

void clear_electromagnet_err_count()
{
  rt_memset(electromagnet_err_count_CHA,0,sizeof(electromagnet_err_count_CHA));
	rt_memset(electromagnet_err_count_CHB,0,sizeof(electromagnet_err_count_CHB));
}
uint16_t *get_electromagnet_err_count(uint8_t ch)
{
	if(ch==0)
		return &electromagnet_err_count_CHA[0];
	else 
		return &electromagnet_err_count_CHB[0];
}

uint16_t *get_cup_err_count(uint8_t ch)
{
	if(ch==0)
		return &cup_err_count_CHA[0];
	else 
		return &cup_err_count_CHB[0];
}

void clear_one_electromagnet_err_count(uint8_t ch,uint8_t id)
{   
	if(id>42)
	{
			return;
	}
	if(ch==0)
	{
		electromagnet_err_count_CHA[id] =0;
	}
	else 
	{
	  electromagnet_err_count_CHB[id] =0;
	}
}

void set_electromagnet_err_count(uint8_t packge_line, uint8_t line,uint8_t ch)
{   
	uint8_t pos=0;
	
	pos = line*6;
	pos += ch;
	if(pos>42)
	{
			return;
	}
	if(packge_line==0)
		electromagnet_err_count_CHA[pos]++;
	else 
		electromagnet_err_count_CHB[pos]++;
}
// 保存某个蛋杯的错误计数
void set_cup_err_count(uint8_t ch,uint16_t id)
{
	if(id>SEPARATE_CHB_CUP_NUMBER)
	{
		return;
	}
	if(id>1)
		id = id-1;
	if(ch ==0)
	{
		cup_err_count_CHA[id]++;
		save_egg_cup_err_count(0,cup_err_count_CHA[id],id);
	}
	else 
	{
		cup_err_count_CHB[id]++;
		save_egg_cup_err_count(1,cup_err_count_CHB[id],id);
	}
}
// 清除所有蛋杯的错误计数
// 要先将当前所有蛋杯的错误计数先保存
// 然后在清零变量
void clear_all_cup_err_count()
{
	uint16_t i;
	
	
	for(i=0;i<SEPARATE_CHB_CUP_NUMBER;i++)
	{
		if(cup_err_count_CHA[i>0])
		save_egg_cup_err_count(0,cup_err_count_CHA[i],i);
		if(cup_err_count_CHB[i>0])
		save_egg_cup_err_count(1,cup_err_count_CHB[i],i);
	}
	
    rt_memset(cup_err_count_CHA,0,sizeof(cup_err_count_CHA));
	rt_memset(cup_err_count_CHB,0,sizeof(cup_err_count_CHB));
}
// 清除某个蛋杯的错误计数并将eeprom中该蛋杯的错误计数也清零
void clear_ont_cup_err_count(uint8_t ch,uint16_t id)
{
	if(id>=SEPARATE_CHB_CUP_NUMBER)  return;
	
	if(ch ==0)
	{
		cup_err_count_CHA[id]=0;
		save_egg_cup_err_count(0,cup_err_count_CHA[id],id);
	}
	else 
	{
		cup_err_count_CHB[id]=0;
		save_egg_cup_err_count(1,cup_err_count_CHB[id],id);
	}
	
}

// 光通道(光检通道)传感器错误检查
void check_light_err()
{
	uint8_t key;
	key =  get_immediately_key_val(CHA_LIGHT_SENSER_ERR);
  if(key ==0)
	{
			set_device_alarm(LIGHT_CH_ERR1);
	}
	key =  get_immediately_key_val(CHB_LIGHT_SENSER_ERR);
  if(key ==0)
	{
			set_device_alarm(LIGHT_CH_ERR2);
	}			
}


// 没有使用这个函数
void check_empty_cup()
{
	
	 uint8_t key;
	 uint8_t i,j;
	 uint32_t offest_pluse;
	 uint32_t add_pluse;
	 uint32_t cpluse;
	 int32_t cmp_pluse;
	 int32_t adj_pluse;
	 rt_base_t level;
	 uint16_t find_start_id;
	 float expansion_move_pluse=0;
	 separate_egg_info_type *egg_info;
	
	
	

	 return;			// ？？？？？？？？？？
	 
	 expansion_move_pluse = SEPARATE_CUP_LEN/SEPARATE_CUP_distem;
	 expansion_move_pluse = expansion_move_pluse*get_expansion_link_offeset();
     offest_pluse =  EMPTY_CUP_SENSER_OFFEST_PLUSEA - (uint16_t)expansion_move_pluse;
	 find_start_id = (EMPTY_CUP_SENSER_OFFEST_PLUSEA/SEPARATE_CUP_LEN)-4;
	 //DEBUG_INFO("find =%d offest=%d\n",find_start_id,offest_pluse);
	 if(find_start_id>=SEPARATE_CHA_CUP_NUMBER)
	 {
		 find_start_id = SEPARATE_CHA_CUP_NUMBER-1;
	 }
	 key =  get_immediately_key_val(EGG_CUP_EMPTY_KEYA);
	 if(key==1) //上升源
	 {
		 //rt_kprintf("up_pluse\r\n");
		 cpluse = get_spt_encode_pluse();
		 for(i=0;i<6;i++)
		 {
			 get_pst_egg_info(0,find_start_id+i,&egg_info,0);
			 if(egg_info->wetght>0)
			 {
				//rt_kprintf("i=%d weight=%d cp=%d ep=%d add=%d\n",i,egg_info->wetght,cpluse,egg_info->pluse,cpluse-egg_info->pluse);
				
			 }
			 cmp_pluse = egg_info->pluse+offest_pluse;
			 cmp_pluse = cpluse-cmp_pluse;
			 add_pluse = cpluse - up_pluse_A;
			 adj_pluse = abs(cmp_pluse);
			 //rt_kprintf("egg id=%d adj=%d add=%d\n",find_start_id+i,adj_pluse,add_pluse);
			 if((adj_pluse<30)&&(add_pluse>150))
			 {
				 up_pluse_A = cpluse;
                 //rt_kprintf("egg id=%d adj=%d add=%d\n",i,adj_pluse,add_pluse);
				 if((egg_info->empty ==TICK_EGG_DOWN)||(egg_info->empty ==TICK_EGG_IS_OVER))
				 {
					  //set_electromagnet_err_count(0,egg_info->separate_ch,egg_info->tick_ch);
					  if(find_first_cup_flag>0)
						{
							//set_cup_err_count(0,SEPARATE_CHA_CUP_NUMBER-egg_info->id);
							//DEBUG_WARN("force empty=%d egg_id=%d\n",egg_info->empty,egg_info->tick_ch);
						}
				 }
				 
				  level = rt_hw_interrupt_disable();
				  if(egg_info->empty ==NEED_TICK_EGG)
					{
					  alread_production.force_chA.need_tick_count++;
					}
					else if(egg_info->empty ==TICK_EGG_DOWN)
					{
					   alread_production.force_chA.tick_over_count++;
					}
					else if(egg_info->empty ==TICK_EGG_IS_OVER)
					{
					  alread_production.force_chA.tick_over_count++;
					}
				     else if(egg_info->empty ==TICK_EGG_IS_BUSY)
					{
					  alread_production.force_chA.recive_busy_count++;
					}
					else if(egg_info->empty ==TICK_EGG_IS_OVERFLOW)
					{
					   alread_production.force_chA.over_flow_count++;
					} 
					else  //未知的
					{
						 if(egg_info->wetght==0)
						 {
							 alread_production.force_chA.no_weight++;
						 }
						 {
							 alread_production.force_chA.empty_cup_count++;
						 }
					}
					count_per_min_egg(3);
					alread_production.force_chA.totle_count++;
					rt_hw_interrupt_enable(level);
				  //DEBUG_WARN("HAVE EGG i=%d egg id=%d cmp=%d weight=%d,line=%d,ch=%d state=%d\n",i,egg_info->id,cmp_pluse,egg_info->wetght,egg_info->separate_ch,egg_info->tick_ch,egg_info->empty);
				  //DEBUG_WARN("HAVE EGG i=%d egg id=%d cmp=%d egg plue=%d,cpluse=%d\n",i,egg_info->id,cmp_pluse,egg_info->pluse,cpluse);
				  break;
			 }
			 
		 }
	 }
	 
	 
	 expansion_move_pluse = SEPARATE_CUP_LEN/SEPARATE_CUP_distem;
	 expansion_move_pluse = expansion_move_pluse*get_expansion_link_offeset();
     offest_pluse =  EMPTY_CUP_SENSER_OFFEST_PLUSEB - (uint16_t)expansion_move_pluse;
	 find_start_id = (EMPTY_CUP_SENSER_OFFEST_PLUSEB/SEPARATE_CUP_LEN)-4;
	 key =  get_immediately_key_val(EGG_CUP_EMPTY_KEYB);
	 if(find_start_id>=SEPARATE_CHB_CUP_NUMBER)
	 {
		 find_start_id = SEPARATE_CHB_CUP_NUMBER-1;
	 }
	 if(key==1) //上升源
	 {
		 cpluse = get_spt_encode_pluse();
		 for(i=0;i<6;i++)
		 {
			 get_pst_egg_info(1,find_start_id+i,&egg_info,0);
			 cmp_pluse = egg_info->pluse+offest_pluse;
			 cmp_pluse = cpluse-cmp_pluse;
			 add_pluse = cpluse - up_pluse_B;
			 adj_pluse = abs(cmp_pluse);
			 if((adj_pluse<30)&&(add_pluse>150))
			 {
				 up_pluse_B = cpluse;

				 if((egg_info->empty ==TICK_EGG_DOWN)||(egg_info->empty ==TICK_EGG_IS_OVER))
				 {
						//set_electromagnet_err_count(1,egg_info->separate_ch,egg_info->tick_ch);
					  if(find_first_cup_flag>0)
						{
							//set_cup_err_count(1,SEPARATE_CHB_CUP_NUMBER-egg_info->id);
							//DEBUG_WARN("force empty=%d egg_id=%d\n",egg_info->empty,egg_info->tick_ch);
						}
				 }
				 
				  level = rt_hw_interrupt_disable();
				  if(egg_info->empty ==NEED_TICK_EGG)
					{
					  alread_production.force_chB.need_tick_count++;
					}
					else if(egg_info->empty ==TICK_EGG_DOWN)
					{
					   alread_production.force_chB.tick_over_count++;
					}
					else if(egg_info->empty ==TICK_EGG_IS_OVER)
					{
					  alread_production.force_chB.tick_over_count++;
					}
				  else if(egg_info->empty ==TICK_EGG_IS_BUSY)
					{
					  alread_production.force_chB.recive_busy_count++;
					}
					else if(egg_info->empty ==TICK_EGG_IS_OVERFLOW)
					{
					   alread_production.force_chB.over_flow_count++;
					} 
					else  //未知的
					{
						 if(egg_info->wetght==0)
						 {
							 alread_production.force_chB.no_weight++;
						 }
						 {
							 alread_production.force_chB.empty_cup_count++;
						 }
					}
					count_per_min_egg(7);
					alread_production.force_chB.totle_count++;
					rt_hw_interrupt_enable(level);
				  //DEBUG_WARN("HAVE EGG i=%d egg id=%d cmp=%d weight=%d,line=%d,ch=%d state=%d\n",i,egg_info->id,cmp_pluse,egg_info->wetght,egg_info->separate_ch,egg_info->tick_ch,egg_info->empty);
					//DEBUG_WARN("HAVE EGG i=%d egg id=%d cmp=%d egg plue=%d,cpluse=%d\n",i,egg_info->id,cmp_pluse,egg_info->pluse,cpluse);
				  break;
			 }
		 }
	 }
}
// 清空强制踢蛋信息
// 设置刷新频率
static uint16_t show_egg_time_count=0;
void set_show_force_egg_info_ch_time(uint8_t time)
{
	clear_update_text_buf();				// 清空强制踢蛋信息
	show_egg_time_count = time*500;	
}
// 主函数会每隔一段时间调用一次
// 
void show_force_ch_egg_info()
{
	if(show_egg_time_count>0) show_egg_time_count--;
	if(show_egg_time_count<2)
	{
	 disp_force_egg_info(&alread_production);
	}
}
// 清除强制踢蛋记录
// 保存每个小通道的产品记录
// 清除已经生产蛋的数据
// 清除每条包装通道已经生产蛋的计数
void clear_totle_eggs_count()
{
  uint8_t i;
	
	
    alread_production.today_egg_count =0;
	alread_production.force_chA.totle_count =0;
	alread_production.force_chA.empty_cup_count =0;
	alread_production.force_chA.need_tick_count =0;
	alread_production.force_chA.over_flow_count =0;
	alread_production.force_chA.recive_busy_count =0;
	alread_production.force_chA.tick_over_count =0;
	alread_production.force_chA.no_weight =0;
	
	alread_production.force_chB.totle_count =0;
	alread_production.force_chB.empty_cup_count =0;
	alread_production.force_chB.need_tick_count =0;
	alread_production.force_chB.over_flow_count =0;
	alread_production.force_chB.recive_busy_count =0;
	alread_production.force_chB.tick_over_count =0;
	alread_production.force_chB.no_weight =0;
	
	for(i=0;i<12;i++)
	{
		save_product_hostroy(i);
		clear_already_pruduct_egg_dat(i);
		alread_production.packge_ch_egg_count[i]=0;
	}
	for(i=0;i<SPT_CH_LINE;i++)
	{
		alread_production.packge_ch_egg_count[i]=0;
	}
}

// 上电后从eeprom中读取分级线所有蛋杯的错误计数记录
void power_up_read_cup_err_count_from_epp()
{
     read_egg_cup_err_count(0,&cup_err_count_CHA[0],SEPARATE_CHA_CUP_NUMBER*2);
	 read_egg_cup_err_count(1,&cup_err_count_CHB[0],SEPARATE_CHB_CUP_NUMBER*2);
}


INIT_ENV_EXPORT(init_check_err_dat);



