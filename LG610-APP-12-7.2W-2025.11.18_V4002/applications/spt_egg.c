#include <rtthread.h>
#include <rtdevice.h>
#include "spt_egg.h"
#include "mdebug.h"
#include "user_set.h"
#include "device_alarm.h"
#include "egg_control.h"
#include "product_egg_recode.h"
#include "app_screen.h"
#include "product_control.h"
ALIGN(RT_ALIGN_SIZE)



#define WEIGHT_DOR 0

#define AUTO_BOXFULL_BUF_LEN 90
typedef struct
{
	uint16_t pos;
	uint16_t egg_weight[AUTO_BOXFULL_BUF_LEN];
	uint16_t egg_count;
	uint16_t avg_min_weight;
	uint16_t avg_max_weight;
}avg_egg_dat;	

static uint16_t packge_id=0;
static avg_egg_dat egg_boxful_info[12];
static uint8_t product_weight_cell[MAX_LINE_NUMBER];
static uint8_t product_crack_cell[MAX_LINE_NUMBER];
static uint8_t product_colour_cell[MAX_LINE_NUMBER];
static uint8_t product_dirty_cell[MAX_LINE_NUMBER];
static uint8_t product_blood_cell[MAX_LINE_NUMBER];   //血斑
static uint8_t product_coincide_cell[MAX_LINE_NUMBER];  //集合
static throw_ch_product_type throw_ch_product ; 
static uint32_t avg_product_egg_count[MAX_LINE_NUMBER];
static uint8_t avg_product_egg_max_line_number[MAX_LINE_NUMBER]={6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6};
static uint8_t avg_product_egg_line_count[MAX_LINE_NUMBER];
extern uint8_t save_analisys_egg_recode_index;

void re_spt_egg_to_packge_line(uint8_t line);





void clear_avg_product_count()
{
	uint8_t i;
	for(i=0;i<MAX_LINE_NUMBER;i++)
	{
      avg_product_egg_count[i]=0;
	}
}	
// 初始化某个通道的一排最大可以接收的蛋数即设置某个通道的蛋托一排多少个
// 我看36000的设备没有这个函数
void init_avg_egg_line_max_count(uint8_t id,uint8_t line)
{
	uint8_t up_line;
     if(id>=MAX_LINE_NUMBER)
	 {
	   return;
	 }
	 up_line = avg_product_egg_max_line_number[id];	// 先获取原来的这个参数
	 avg_product_egg_max_line_number[id]=line;			// 然后将新设置的保存下来
	 
	 // 如果不一样说明蛋托一排的个数变了，需要重新设置
	 if(up_line !=line)															// 如果原来的数和新设置的数不一样
	 {
		 // 不一样的话，下面的操作是什么意思,是不是8个通道的设备，8,9是分级线A的抛弃通道，10,11是分级线B的抛弃通道?
		if((id==8)||(id==9))
		{
			re_spt_egg_to_packge_line(0);		// 重新分离spt_eg里面的蛋到相应的包装通道？
			rt_kprintf("re spt egg lineA ch=%d\n",id);
		}
		if((id==10)||(id==11))
		{
			re_spt_egg_to_packge_line(1);
			rt_kprintf("re spt egg lineB ch=%d\n",id);
		}
	 }
}


// 这个有什么意义呢，好像没有意义呀，逻辑对不上，而且最终的变量没有使用呀？
uint8_t calcure_avg_product(uint8_t *index,uint8_t *tick_ch)
{
  uint8_t i;
	uint8_t id;
	id =MAX_LINE_NUMBER;
	uint32_t count=0xfffffff1;
	for(i=0;i<MAX_LINE_NUMBER;i++)   
	{
	  if(index[i]>0)   //依次遍历各个通道
		{
			// 这个相当于是从通道0-17，依次按顺序找
			// 当找到的上一条线比本条线生产的多也即本条线更少，就记录下本条线
			if(count>avg_product_egg_count[i])		// 找相同生产线中数量少的，就往
			{
			  count= avg_product_egg_count[i];
			  id = i;
			}
		}
	}
	if(id<MAX_LINE_NUMBER)
	{
		avg_product_egg_count[id]++;	// 因为这个蛋是用找到的那条线来踢，所以这条线的总蛋数要加1
	}
  return id;
}


// 1 属于这个范围  0不属于
static uint8_t compare_dat(uint16_t min,uint16_t max,uint16_t dat)
{
	uint8_t result;
	
  if((dat>=min)&&(dat<max))
		result=1;
	else 
		result=0;
  return result;
}

// 1 属于这个范围  0不属于
static uint8_t compare_weight_dat(uint16_t min,uint16_t max,uint16_t dat)
{
	uint8_t result;
	
	if(WEIGHT_DOR==0)
	{
		min=min*10;
		max = max*10;
	}
  if((dat>=min)&&(dat<max))
		result=1;
	else 
		result=0;
  return result;
}





// 1 重量属于这个范围  0不属于
// 称重设备发过来的称重数据扩大了10倍
// 比较蛋的重量是不是在屏幕设置的该包装通道的重量范围
// 先判断称重设备是不是使用
// 然后获取该产品的重量设置范围
// 先判断是不是使用了自定义模式，自定义模式是weight_buf[8]-weight_buf[9]，如果是自定义模式的话，weight_buf[9]一定大于0
// 如果是自定义模式，在直接判断weight_buf[8]-weight_buf[9]之间即可
// 如果是其他8个等级则依次判断
uint8_t compare_weigt_packge_leve(production_type *product_part,uint8_t per_product_id,uint16_t egg_weight,uint16_t spt_ch)
{

	uint8_t i,result;
	uint16_t *weight_buf;
	uint16_t *weight_leve;
	user_set_part_type *user_set;
	result = 0;

	get_user_set_point(&user_set);
  if(spt_ch==0X555)
		{
		  if(user_set->device_use_info.chA.weight_samp==0)  //设备未启用
	    {
		    return 1;
	    }
		}
		else 
		{
		  if(user_set->device_use_info.chB.weight_samp==0)  //设备未启用
	    {
		    return 1;
	    }
		}
	weight_leve = &user_set->weight_leve.leve1;
	weight_buf = &product_part->pre_production[per_product_id].weight_leve[0];
	if(weight_buf[9]>0)  //自定义模式
	{
		 result = compare_weight_dat(weight_buf[8],weight_buf[9],egg_weight);
		 if(result>0) 
		 {
			 return 1;
		 }
	}
	else    //常规8种等级模式                            
	{
		for(i=0;i<8;i++)
		{
			// weight_leve[i]这个只是表示有没有选中该等级？还是该等级的重量信息也包含？
		  if(product_part->pre_production[per_product_id].weight_leve[i]>0)  //启用这个等级  一共8个   等级1最大
			{
			  result = compare_weight_dat(weight_leve[i+2],weight_leve[i+1],egg_weight);
				if(result>0)
				{
					 return 1;
				}
			}
		}
	}
	return 0;
}


// 1 重量属于这个范围  0不属于
// 比较蛋的重量是不是在屏幕设置的该抛弃通道的重量范围
uint8_t compare_weigt_throw_leve(uint16_t *weight_buf,uint16_t egg_weight,uint16_t spt_ch)
{

	uint8_t i,result;
	uint16_t *weight_leve;
	user_set_part_type user_set;
	
	get_user_set_part(&user_set);
	result = 0;
    if(spt_ch==0X555)
	{
	   if(user_set.device_use_info.chA.weight_samp==0)  //设备未启用
	   {
		    return 1;
	   }
	}
	else 
	{
		if(user_set.device_use_info.chB.weight_samp==0)  //设备未启用
	    {
		    return 1;
	    }
	}
	weight_leve = &user_set.weight_leve.leve1;

	// 没有自定义模式
	if(0)//(weight_buf[9]>0)  //自定义模式
	{
		 result += compare_weight_dat(weight_buf[8],weight_buf[9],egg_weight);
	}
	else    //常规8种等级模式                            
	{
		for(i=0;i<9;i++)
		{
		  if(weight_buf[i]>0)  //启用这个等级  一共8个   等级1最大
			{
			  result += compare_weight_dat(weight_leve[i+1],weight_leve[i],egg_weight);   //重量 BUF[0] 最大
				if(result>0)
				{
					 return 1;
				}
			}
		}
	}
	/*
	 result = compare_weight_dat(weight_leve[1],weight_leve[0],egg_weight);  //蛋品分析要记录  异常大
	 if(result>0)
	 {
	    *cmp_leve = 0;    //异常大
	 }
	 
	 result = compare_weight_dat(0,weight_leve[9],egg_weight);  //蛋品分析要记录  异常大
	 if(result>0)
	 {
	    *cmp_leve = 9;    //异常小
	 }
	 if(egg_weight<10)
		 *cmp_leve =100;  
	 */
	return 0;
}








// 1 属于这个范围  0不属于
uint8_t compart_crack_leve(uint8_t line,uint16_t *product_leve,uint16_t crack_dat,uint16_t spt_ch)
{
   uint8_t j;
	 uint8_t result;
	 uint32_t count=0;
	 result = 0;
	 user_set_part_type user_set;
	
	 get_user_set_part(&user_set);
   
	  if(spt_ch==0X555)
		{
		  if(user_set.device_use_info.chA.crack_check==0)  //设备未启用
	    {
		    return 1;
	    }
		}
		else 
		{
		  if(user_set.device_use_info.chB.crack_check==0)  //设备未启用
	    {
		    return 1;
	    }
		}
  	
	 for(j=0;j<4;j++) //4个等级+ 2个自定义   优良中 差
	 {
     if(product_leve[j]>0)  //启用这个等级  一共4个   
		 {
			 result = compare_dat(user_set.crack_leve.dat[j*2],user_set.crack_leve.dat[j*2+1],crack_dat);
			 if(result>0)
			 {
					 return 1;
			 }
			}
	 }
	 if(line<12)
	 {
	   result = compare_dat(product_leve[4],product_leve[5],crack_dat);
		 if(result>0)
			{
					 return 1;
			}
	 }
	 return 0;
}

// 1 属于这个范围  0不属于
uint8_t compart_colour_leve(uint8_t line,uint16_t *product_leve,uint16_t colour_dat,uint16_t spt_ch)
{
   uint8_t j;
	 uint8_t result;
	 uint32_t count=0;
	 result = 0;
	 user_set_part_type user_set;
	
	 get_user_set_part(&user_set);
	 if(spt_ch==0X555)
	 {
		 if(user_set.device_use_info.chA.colour_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
	 }
	else 
	{
		if(user_set.device_use_info.chB.colour_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
	}
	
	 for(j=0;j<4;j++) //4个等级+ 2个自定义   优良中 差
	 {
       if(product_leve[j]>0)  //启用这个等级  一共4个   
		 {
			 result = compare_dat(user_set.colour_leve.dat[j*2],user_set.colour_leve.dat[j*2+1],colour_dat);
			 if(result>0)
			 {
					 return 1;
			 }
			}
	 }
	 if(line<12)
	 {
	   result = compare_dat(product_leve[4],product_leve[5],colour_dat);
		 if(result>0)
			{
					 return 1;
			}
	 }
	 return 0;
}


// 1 属于这个范围  0不属于
uint8_t compart_blood_leve(uint8_t line,uint16_t blood_leve,uint16_t blood_dat,uint16_t spt_ch)
{
	 uint8_t j;
	 uint8_t result;
	 uint32_t count=0;
	 result = 0;
	 user_set_part_type user_set;
	
	 get_user_set_part(&user_set);
	
	 if(spt_ch==0X555)
	 {
		  if(user_set.device_use_info.chA.blood_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
		}
		else 
		{
		  if(user_set.device_use_info.chB.blood_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
		}
	 if(blood_leve==1)  //设备未启用以及不区分血斑蛋
	 {
		 return 1;
	 }
	 else if(blood_dat ==2)  //是血斑蛋
	 { 
	   return 0;
	 }
	 else 
	 {
		return 1;
	 }
}

uint8_t compart_dirty_leve(uint8_t line,uint16_t *product_leve,uint16_t dirty_dat,uint16_t spt_ch)
{
   uint8_t j;
	 uint8_t result;
	 uint32_t count=0;
	 result = 0;
	 user_set_part_type user_set;
	
	 get_user_set_part(&user_set);
	 if(spt_ch==0X555)
	 {
		  if(user_set.device_use_info.chA.dirty_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
		}
		else 
		{
		  if(user_set.device_use_info.chB.dirty_recognize==0)  //设备未启用
	    {
		    return 1;
	    }
		}
	 for(j=0;j<3;j++) //4个等级+ 2个自定义   优良中 差
	 {
     if(product_leve[j]>0)  //启用这个等级  一共4个   
		 {
			 result = compare_dat(user_set.drty_leve.dat[j*2],user_set.drty_leve.dat[j*2+1],dirty_dat);
			 if(result>0)
			 {
					 return 1;
			 }
			}
	 }
	 if(line<12)
	 {
	  result = compare_dat(product_leve[3],product_leve[4],dirty_dat);
		if(result>0)
		{
			return 1;
		}
	 }
	 return 0;
}

void show_crack_info(production_type *dat)
{
	uint8_t i;
	
  for(i=0;i<6;i++)
	{
		if(dat->pre_production[i].empty !=0)   //要生产的产品
		{
			DEBUG_INFO("index=%d buf[0]=%d buf[1]=%d buf[2]=%d buf[3]=%d buf[6]=%d buf[7]=%d \n",i,dat->pre_production[i].crack_leve[0]
			,dat->pre_production[i].crack_leve[1],dat->pre_production[i].crack_leve[2],dat->pre_production[i].crack_leve[3]
			,dat->pre_production[i].crack_leve[4],dat->pre_production[i].crack_leve[5]);
		}
	}
}

// 计算同时满足重量，裂纹，颜色，脏蛋，血斑等级的蛋
// 如果同时满足则product_coincide_cell[i]=1;
uint8_t  clacure_coincide_cell()
{
  uint8_t i;
	uint8_t result=0;
	//for(i=0;i<MAX_LINE_NUMBER;i++)
	for(i=0;i<18;i++)	// 18个产品，这样号理解一些
	{
		if((product_weight_cell[i]>0)&&(product_crack_cell[i]>0)&&(product_colour_cell[i]>0)&&(product_dirty_cell[i]>0)&&(product_blood_cell[i]>0))   //重量 和 裂纹
		{
			result =1;
		  product_coincide_cell[i]=1;
		}
	}
	return result;
}



// 根据蛋重量计算出是否满足设置的重量等级
uint8_t compare_egg_weight_quility_leve(uint16_t egg_weight,uint16_t spt_ch)
{
	uint8_t i;
	uint8_t result =0;
	uint16_t *weight_leve;
	user_set_part_type user_set;
	
	get_user_set_part(&user_set);
  if(spt_ch==0)
	{
		  if(user_set.device_use_info.chA.weight_samp==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	else 
	{
		  if(user_set.device_use_info.chB.weight_samp==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	
	weight_leve = &user_set.weight_leve.leve1;
  for(i=0;i<9;i++)
	{
		// 循环找，从最大开始找
		// 如果这个蛋重量在这个范围
		// 则返回i(i表示找的次数，也相当于等级了)
		// 这样就知道了这个蛋在设置当中是属于哪个等级的蛋
		// 因为可能不会全部循环完，因为可能提前找到属于哪个等级就提前退出了
		result += compare_weight_dat(weight_leve[i+1],weight_leve[i],egg_weight);   //重量 BUF[0] 最大
		if(result>0)
		{
			return i+1;
		}
	}

		// 如果上面都不属于上面的等级，说明这个蛋超出了设置的等级的重量范围，则判断是不是超过1000g
	 result = compare_weight_dat(weight_leve[0],1000,egg_weight);  //蛋品分析要记录  异常大
	 if(result>0)
	 {
	    return 0;    //异常大
	 }
	 // 如果超过了1000g，则看看是不是属于自定义的重量范围？
	 result = compare_weight_dat(0,weight_leve[9],egg_weight);  //蛋品分析要记录  异常大
	 if(result>0)
	 {
	   return 9;    //异常小
	 }
	 // 如果也不是自定义的重量范围，并且该蛋有重量，则属于未定义的重量
	 if(egg_weight>0)  //未知的 自定义
	 {
		 return 10; 
	 }
	 return 100;
}

uint8_t compare_egg_crack_quility_leve(uint16_t egg_crack,uint16_t spt_ch)
{
	uint8_t i;
	uint8_t result =0;
	uint16_t *crack_leve;
	user_set_part_type user_set;
	
	get_user_set_part(&user_set);
  if(spt_ch==0XAAA)
	{
		  if(user_set.device_use_info.chA.crack_check==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	else 
	{
		  if(user_set.device_use_info.chB.crack_check==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	
	crack_leve = &user_set.crack_leve.dat[0];
  for(i=0;i<4;i++)
	{
		// 看看这个裂纹是哪个等级，没有判断自定义等级？或者不需要判断自定义等级？
		result += compare_dat(crack_leve[i*2],crack_leve[i*2+1],egg_crack);   //重量 BUF[0] 最大
		if(result>0)
		{
			return i;
		}
	}
	 return 4;
}

uint8_t compare_egg_colour_quility_leve(uint16_t egg_colour,uint16_t spt_ch)
{
	uint8_t i;
	uint8_t result =0;
	uint16_t *colour_leve;
	user_set_part_type user_set;
	
	get_user_set_part(&user_set);
	
	if(spt_ch==0XAAA)
	{
		  if(user_set.device_use_info.chA.colour_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	else 
	{
		  if(user_set.device_use_info.chB.colour_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}

	
	colour_leve = &user_set.colour_leve.dat[0];
    for(i=0;i<4;i++)
	{
		// 判断这个蛋的颜色是哪个等级的颜色
		result += compare_dat(colour_leve[i*2],colour_leve[i*2+1],egg_colour);   //重量 BUF[0] 最大
		if(result>0)
		{
			return i;
		}
	}
	 return 4;
}

uint8_t compare_egg_dirty_quility_leve(uint16_t egg_dirty,uint16_t spt_ch)
{
	uint8_t i;
	uint8_t result =0;
	uint16_t *dirty_leve;
	user_set_part_type user_set;
	
	get_user_set_part(&user_set);
  if(spt_ch==0XAAA)
	{
		  if(user_set.device_use_info.chA.dirty_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	else 
	{
		  if(user_set.device_use_info.chB.dirty_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	
	dirty_leve = &user_set.drty_leve.dat[0];
    for(i=0;i<3;i++)
	{
		// 判断该蛋的脏蛋等级是哪个等级
		result += compare_dat(dirty_leve[i*2],dirty_leve[i*2+1],egg_dirty);   //重量 BUF[0] 最大
		if(result>0)
		{
			return i;
		}
	}
	return 3;
}
// 血斑没有使用?
uint8_t compare_egg_blood_quility_leve(uint16_t egg_blood,uint16_t spt_ch)
{

	user_set_part_type user_set;
	get_user_set_part(&user_set);
 
	if(spt_ch==0XAAA)
	{
		if(user_set.device_use_info.chA.blood_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	else 
	{
		if(user_set.device_use_info.chB.blood_recognize==0)  //设备未启用
	    {
		    return 100;
	    }
	}
	return egg_blood;
}
// 记录已经生产的蛋的质量分析
// 包括总重量，总个数以及，这个蛋是什么等级并记录
void analisys_egg_quility(separate_egg_info_type *egg,uint16_t spt_line)
{
        recode_egg_dat recode_egg;
		
	    recode_egg.weight = egg->wetght;
		recode_egg.colour_leve =compare_egg_colour_quility_leve(egg->colour,spt_line);
	    recode_egg.boody_leve =compare_egg_blood_quility_leve(0,spt_line);
		recode_egg.dity_leve =compare_egg_dirty_quility_leve(egg->dity,spt_line);
		recode_egg.weight_leve = compare_egg_weight_quility_leve(egg->wetght,spt_line);
		recode_egg.crack_leve = compare_egg_crack_quility_leve(egg->crack,spt_line);      
		recode_analysis_egg_info(&recode_egg,save_analisys_egg_recode_index);   //函数里面有判断 是否有重量
}

// 自动配重的最大和最小重量初始化
void init_egg_boxfull_info(uint8_t id,uint16_t min_weight,uint16_t max_weight)
{
    if(id>5) return ;
	rt_memset(&egg_boxful_info[id],0,sizeof(avg_egg_dat));

	egg_boxful_info[id].avg_max_weight = max_weight;
	egg_boxful_info[id].avg_min_weight = min_weight;
	DEBUG_INFO("自动配盘重量 INDEX=%d min=%d max=%d\n",id,min_weight,max_weight);
}
void count_egg_boxfull_dat(uint8_t id,uint16_t egg_weight)
{
	
     egg_boxful_info[id].egg_weight[egg_boxful_info[id].pos] = egg_weight;
	 if(egg_boxful_info[id].egg_count<AUTO_BOXFULL_BUF_LEN)
	 {
			egg_boxful_info[id].egg_count ++;
	 }
	 else
	 {
	   egg_boxful_info[id].egg_count = AUTO_BOXFULL_BUF_LEN;
	 }
	 egg_boxful_info[id].pos++;
	 if(egg_boxful_info[id].pos>=AUTO_BOXFULL_BUF_LEN)
	 {
			 egg_boxful_info[id].pos =0;
	 }
}

uint8_t compare_auto_weight(uint8_t ch,uint16_t egg_weight)  //自动分配重量
{
	 uint8_t i;
	 uint32_t totle_weight;
	 uint32_t agg_weight;
   if(egg_boxful_info[ch].egg_count< (AUTO_BOXFULL_BUF_LEN/3))
	 {
	   return 1;     //才开始 不算平均值
	 } 
	 else 
	 {
		 totle_weight =0;
		 for(i=0;i<egg_boxful_info[ch].egg_count;i++)  //计算当前 AUTO_BOXFULL_BUF_LEN 这个阶段的平均重量
		 {
		  totle_weight += egg_boxful_info[ch].egg_weight[i];
		 }
		 agg_weight = totle_weight/egg_boxful_info[ch].egg_count;
		 
		 if((agg_weight<egg_boxful_info[ch].avg_min_weight)&&(egg_weight<egg_boxful_info[ch].avg_min_weight))
		 {
		   return 0;
		 }
		 if((agg_weight>egg_boxful_info[ch].avg_max_weight)&&(egg_weight>egg_boxful_info[ch].avg_max_weight))
		 {
		   return 0;
		 }
	 }
   return 1;
}




// 计算某一个蛋(已经通过分离计算分离到spt_eg里面的蛋)
// 依次判断这个蛋是不是该条通道设置的重量，脏蛋，裂纹等等级范围
// 如果重量，脏蛋，裂纹等等级范围满足该条线设置的等级范围则将该等级数据的该通道置为1，表示满足该通道的等级设置
// 然后判断每个通道所有等级设置是不是都满足，如果这个蛋的所有等级都满足该通道所有等级的设置，则该蛋应该让该通道来踢
void calculate_egg_info(production_type * per_poduction_egg,separate_egg_info_type *egg_info,uint16_t spt_line)
{
	uint8_t i;
	uint8_t result;
	uint8_t spt_tick_ch;
	uint8_t is_packge_line=0;
	uint8_t spt_product_id =0;
	
	
	for(i=0;i<MAX_LINE_NUMBER;i++)
	{
			product_weight_cell[i]=0;
			product_crack_cell[i]=0;
			product_colour_cell[i]=0;
			product_dirty_cell[i]=0;
		  product_blood_cell[i]=0;
			product_coincide_cell[i]=0;
	}
	for(i=0;i<12;i++)    //是12个产品 不是12条线
	{
																																																// packge_line & spt_line
																																																// spt_line 
																																																// A 0x555 010101010101
																																																// B 0xAAA 101010101010
																																																// 2条线的话，某个产品肯定是在一条线上？
																																																// 
		// 某个产品用的通道在这条分级线上
		if((per_poduction_egg->pre_production[i].empty !=0)&&(per_poduction_egg->pre_production[i].packge_line & spt_line))   //要生产的产品
		{
			result = compart_crack_leve(i,&per_poduction_egg->pre_production[i].crack_leve[0],egg_info->crack,spt_line );
			if(result>0)
			{
				 product_crack_cell[i]=1;
			}
			//result = compart_blood_leve(i,per_poduction_egg->pre_production[i].bood_leve,egg_info->blood,spt_line);
			//if(result>0)
			//{
				 product_blood_cell[i]=1;
			//}
			result = compare_weigt_packge_leve(per_poduction_egg,i,egg_info->wetght,spt_line);
			if(result>0)
			{
				if(per_poduction_egg->pre_production[i].auto_clacure_weight[0]>0)   //启动自动配盘
				{
					uint8_t cmp;
					cmp = compare_auto_weight(i,egg_info->wetght);
					if(cmp>0)
					{
						product_weight_cell[i]=1;
					}
					else 
					{
					  product_weight_cell[i]=0;
					}
				}
				else	// 不是自动配重，则直接标记重量满足该等级
				{
					product_weight_cell[i]=1;
				}
			}
					
			result = compart_colour_leve(i,&per_poduction_egg->pre_production[i].colour_leve[0],egg_info->colour,spt_line);
			if(result>0)
			{
				product_colour_cell[i]=1;
			}
					
			result = compart_dirty_leve(i,&per_poduction_egg->pre_production[i].dirty_leve[0],egg_info->dity,spt_line);
			if(result>0)
			{
					product_dirty_cell[i]=1;
			}
		}
	}
	
	// 上面分析出了，这个蛋是不是这12个产品的各种等级，某些等级满足，则对应该产品的对应等级信息会被置1
	
	// 上面是12个产品？
	// 下面是18条线？
	// 怎么对应，还是说上面就是12条线？
	
	// 其实 clacure_coincide_cell 里面后6个永远都不会满足条件，相当于只判断了前12条通道？
	
	
	is_packge_line = clacure_coincide_cell();	// 计算这个蛋12个包装通道是不是满足设置
	if(spt_line==0X555) //  通道A
	{
	   if(is_packge_line ==0)  //前面12通道 都不匹配
		{
			read_product_throw_ch_part(&throw_ch_product);
			for(i=0;i<2;i++)
			{
				// 则判断是不是满足抛弃通道的各种等级
				product_blood_cell[i+12]=1;
		
				result = compare_weigt_throw_leve(&throw_ch_product.cha[i].weight_leve[0],egg_info->wetght,spt_line );
				if(result>0)
				{
					product_weight_cell[i+12]=1;
				}
					result = compart_crack_leve(i+12,&throw_ch_product.cha[i].crack_leve[0],egg_info->crack,spt_line );
				if(result>0)
				{
					product_crack_cell[i+12]=1;
				}
					
				result = compart_colour_leve(i+12,&throw_ch_product.cha[i].colour_leve[0],egg_info->colour,spt_line );
				if(result>0)
				{
					product_colour_cell[i+12]=1;
				}
					
				result = compart_dirty_leve(i+12,&throw_ch_product.cha[i].dirty_leve[0],egg_info->dity,spt_line );
				if(result>0)
				{
					product_dirty_cell[i+12]=1;
				}
			}
			// 这个时候13-14就有抛弃通道的数据了
			// 再次调用clacure_coincide_cell就可能有通道了
			is_packge_line = clacure_coincide_cell();	
		}
	}
	
	
	if(spt_line==0XAAA) //  通道B
	{
	   if(is_packge_line ==0)  //前面8通道 都不匹配
		{
			read_product_throw_ch_part(&throw_ch_product);
			for(i=0;i<2;i++)
			{
		
				product_blood_cell[i+15]=1;
			
				result = compare_weigt_throw_leve(&throw_ch_product.chb[i].weight_leve[0],egg_info->wetght,spt_line);
				if(result>0)
				{
					product_weight_cell[i+15]=1;
				}
					result = compart_crack_leve(i+12,&throw_ch_product.chb[i].crack_leve[0],egg_info->crack,spt_line);
				if(result>0)
				{
					product_crack_cell[i+15]=1;
				}
					
				result = compart_colour_leve(i+12,&throw_ch_product.chb[i].colour_leve[0],egg_info->colour,spt_line );
				if(result>0)
				{
					product_colour_cell[i+15]=1;
				}
					
				result = compart_dirty_leve(i+12,&throw_ch_product.chb[i].dirty_leve[0],egg_info->dity,spt_line );
				if(result>0)
				{
					product_dirty_cell[i+15]=1;
				}
			}
			is_packge_line = clacure_coincide_cell();	
		}
	}

	// clacure_coincide_cell 这个计算出了这个蛋是不是都满足这18条线，也就是说有些通道设置的生产信息一样，这个蛋可以往这些满足的通道跑
	// calcure_avg_product 这个函数就看看都满足的通道中的比较少的蛋的通道，然后就往这个通道跑
	
	
	// 如果需要通道来接收这个蛋了
	if(is_packge_line>0)
	{
		// 则计算用哪个通道来接收(通过该产品中蛋数量最少得通道来算)
		spt_product_id = calcure_avg_product(product_coincide_cell,&spt_tick_ch);
		if(spt_product_id<12)	// 如果是该用包装通道来踢
		{
			//if(per_poduction_egg->pre_production[spt_product_id].auto_clacure_weight[0]>0)   //启动自动配盘
			//{
				//count_egg_boxfull_dat(spt_product_id,egg_info->wetght);    //自动配盘数据
			//}
			egg_info->print_id = per_poduction_egg->pre_production[spt_product_id].print_id;	// 找到该包装通道应该哒喷码的id
			egg_info->family_ch = spt_product_id;   //属于哪个产品	// 是哪个通道
			egg_info->separate_ch = caculate_spt_ch(spt_product_id,spt_line);    // 平均分配这个鸡蛋
			egg_info->spt_ch_pos = spt_tick_ch;	// spt_tick_ch这个在上面调用的时候没有赋值呀，没有使用吗？
			egg_info->separate_ch = packge_map_ch_id[egg_info->separate_ch];
		}
		else 
		{
			if(spt_line==0X555)   //通道A
			{
			   egg_info->print_id = throw_ch_product.cha[spt_product_id-12].print_id;
			}
			else     //通道B
			{
			   egg_info->print_id = throw_ch_product.chb[spt_product_id-15].print_id;
			}
			
			egg_info->family_ch = spt_product_id;
			egg_info->separate_ch = spt_product_id;      //抛弃通道前面有平均分配
		}
		//rt_kprintf("ch=%d pid=%d\n",egg_info->separate_ch,egg_info->print_id);

	}
	else 
	{
		//DEBUG_CARE("sptch=%d weight=%d crack=%d dirty=%d\n",egg_info->separate_ch,egg_info->wetght,egg_info->crack,egg_info->dity);
	}
	

} 
extern uint8_t get_blood_dat();


static uint16_t blood_ofesset_id=0;
// 这个函数没有调用
// 可能是因为血斑没有使用，所以没有调用这个函数，使用的时候就要调用？说明还是有意义?
void clacute_blood_offest(uint16_t pluse)
{
 uint16_t offest;
 offest = pluse;	
 if(offest>1000)
 { 
   blood_ofesset_id = offest/1000;
   blood_ofesset_id = blood_ofesset_id *6;
   offest = offest %1000;
   offest = offest /166;
   blood_ofesset_id = blood_ofesset_id + offest;

 }
 else 
 {
	blood_ofesset_id =0;
 }
   DEBUG_CARE("blood offest id=%d pluse=%d\n",blood_ofesset_id,pluse);
}
void add_blood_info()
{
	
	//separate_egg_info_type *egg_info;
	//user_set_part_type user_set;
	//get_user_set_part(&user_set);
	
//	if(user_set.use_mode.egg_blood_recognize)
//	{
	//	get_pst_egg_info(blood_ofesset_id,&egg_info,0);
//		egg_info->blood = get_blood_dat();
		//rt_kprintf("weight=%d blood=%d\n",egg_info->wetght,egg_info->blood);
//	}
}
uint16_t lose_info[2]={0,0};
uint16_t nospt_info[2]={0,0};
uint32_t tspt_info[2]={0,0};

extern alread_production_type alread_production;
void spt_egg_to_packge_line()
{
	  user_set_part_type user_set;
	

      separate_egg_info_type *egg_infoA;
	  separate_egg_info_type *egg_infoB;
	  production_type per_poduction_egg;
	  add_blood_info();
	
	  get_user_set_part(&user_set);
		// 从spt_eg分级线数据中的第200-个位置去取A线的数据
	  get_pst_egg_info(0,200,&egg_infoA,0);	// 这里获取到了地址，修改egg_infoA里面的东西，获取位置的数据也被修改了
	  if(egg_infoA->empty !=CUP_IS_EMPTY)	// 该蛋不为空，tspt_info这个是分级线蛋数量加1？
	  {
	    tspt_info[0]++;
	  }
		// 没有信息的蛋？这个是怎么判断的呢？
	  if((egg_infoA->empty ==NEED_TICK_EGG)&&(egg_infoA->dity==100)&&(user_set.device_use_info.chA.dirty_recognize))
	  {
		  lose_info[0]++;
	    //DEBUG_CARE("A weigh=%d fch=%d  ch=%d\n",egg_infoA->wetght,egg_infoA->family_ch,egg_infoA->separate_ch);
	  }
		// 未分配通道的蛋
	  if((egg_infoA->empty ==NEED_TICK_EGG)&&(egg_infoA->separate_ch==30))
	  {
		  nospt_info[0]++;
	    //DEBUG_CARE("A weigh=%d fch=%d  ch=%d\n",egg_infoA->wetght,egg_infoA->family_ch,egg_infoA->separate_ch);
	  }
		
		
		
		/****************************/
		// 如果没有踢但是需要踢，则强制踢数量加1
		if(egg_infoA->empty ==NEED_TICK_EGG)
		{
				alread_production.force_chA.need_tick_count++;
		}
		// 如果没有踢但是需要踢，但是强制提又忙则强制忙数量加1
		else if(egg_infoA->empty ==TICK_EGG_IS_BUSY)
		{
				alread_production.force_chA.recive_busy_count++;
		}
		// 如果没有踢但是需要踢，但是不在强制提的脉冲(位置)范围则强制脉冲超数数量加1
		else if(egg_infoA->empty ==TICK_EGG_IS_OVERFLOW)
		{
				alread_production.force_chA.over_flow_count++;
		} 
		/****************************/
	  
	  
	  get_pst_egg_info(1,200,&egg_infoB,0);
		
		if(egg_infoB->empty ==NEED_TICK_EGG)
		{
				alread_production.force_chB.need_tick_count++;
		}
		else if(egg_infoB->empty ==TICK_EGG_IS_BUSY)
		{
				alread_production.force_chB.recive_busy_count++;
		}
		else if(egg_infoB->empty ==TICK_EGG_IS_OVERFLOW)
		{
				alread_production.force_chB.over_flow_count++;
		}
		
	  if(egg_infoB->empty !=CUP_IS_EMPTY)
	  {
	    tspt_info[1]++;
	  }
	  if((egg_infoB->empty ==NEED_TICK_EGG)&&(egg_infoB->dity==100)&&(user_set.device_use_info.chB.dirty_recognize))
	  {
		  lose_info[1]++;
	    //DEBUG_CARE("B weigh=%d fch=%d  ch=%d\n",egg_infoB->wetght,egg_infoB->family_ch,egg_infoB->separate_ch);
	  }
	  if((egg_infoB->empty ==NEED_TICK_EGG)&&(egg_infoB->separate_ch==30))
	  {
		  nospt_info[1]++;
	    //DEBUG_CARE("B weigh=%d fch=%d  ch=%d\n",egg_infoB->wetght,egg_infoB->family_ch,egg_infoB->separate_ch);
	  }
		
		
		// 从spt_eg分级线数据中的第12个位置去取A线的数据
	  get_pst_egg_info(0,12,&egg_infoA,0);
	  get_pst_egg_info(1,12,&egg_infoB,0);
//	  DEBUG_INFO("要改\n");
	  get_production_mange_set(&per_poduction_egg);
	
	/*
	 if(ReadMBHotRegU16(1283)==999)
	 {
		egg_infoA->empty = NEED_TICK_EGG;
		egg_infoA->print_id = 0;
		egg_infoA->family_ch = 0;
		egg_infoA->separate_ch = 0; 
		egg_infoB->empty = NEED_TICK_EGG;
		egg_infoB->print_id = 0;
		egg_infoB->family_ch = 0;
		egg_infoB->separate_ch = 4; 
		return;		
	 }
	 */
	//0XAA,0X55;
	 //1010   
	 //0101
	// 10101010
	// 01010101
	 
	  if(egg_infoA->empty ==NEED_TICK_EGG)
		{
			calculate_egg_info(&per_poduction_egg,egg_infoA,0X555);		// 根据要分离的蛋的信息，依据各种等级并计算出需要哪一条通道来踢蛋
			//DEBUG_CARE("CHA weight=%d  ch=%d \n"
			//,egg_infoA->wetght,egg_infoA->separate_ch);
		}
		
		if(egg_infoB->empty ==NEED_TICK_EGG)
		{
			// 根据计算得到的信息会存在egg_infoB，而egg_infoB其实是spt_eg里面的某个数据
			// egg_infoB 下面函数里面修改了东西，体现在egg_infoB这个倍修改了，而这个是get_pst_egg_info从spt_eg获取的，相当于spt_eg里面的数据倍修改了
			calculate_egg_info(&per_poduction_egg,egg_infoB,0XAAA);		// 根据要分离的蛋的信息，依据各种等级并计算出需要哪一条通道来踢蛋
			//DEBUG_INFO("CHB weight=%d ch=%d \n"
			//,egg_infoB->wetght,egg_infoB->separate_ch);
		}
		
		if(egg_infoA->empty !=CUP_IS_EMPTY)													// 如果某条线的某个蛋杯不为空，说明有蛋
		{
			// 获取供应商对于的ID
			if(save_analisys_egg_recode_index>=MAX_SUPPLY_COUNT)			// 如果要分析的id大于等于供应商的最大数量
			{
		      get_save_ayaylisis_egg_index((char *)per_poduction_egg.egg_source);	// 重新依据供应商的名称获取该供应商的id
			}
			analisys_egg_quility(egg_infoA,0X555);										// 依据供应商的id，记录这个蛋的各种等级信息以及对应等级的数量和重量信息
		}
		if(egg_infoB->empty !=CUP_IS_EMPTY)
		{
			// 获取供应商对于的ID
			if(save_analisys_egg_recode_index>=MAX_SUPPLY_COUNT)
			{
		      get_save_ayaylisis_egg_index((char *)per_poduction_egg.egg_source);
			}
			analisys_egg_quility(egg_infoB,0XAAA);
		}
}


// 重新分离蛋到包装通道
// 经一路分析，是不是没有意义？
void re_spt_egg_to_packge_line(uint8_t line)
{
	  uint8_t i;
	  user_set_part_type user_set;
	
      separate_egg_info_type *egg_infoA;
	  separate_egg_info_type *egg_infoB;
	  production_type per_poduction_egg;
	  add_blood_info();
	  get_user_set_part(&user_set);
	
	  get_production_mange_set(&per_poduction_egg);
	  for(i=200;i>11;i--)		// 为什么是循环200次
	  {
			if(line==0)
			{
				 get_pst_egg_info(0,i,&egg_infoA,0);		// 这个i是表示传输线上的某一排格子的距离id吗？这个表示获取某一排的要踢蛋的信息数据
				 if(egg_infoA->empty ==NEED_TICK_EGG)
				 {
						calculate_egg_info(&per_poduction_egg,egg_infoA,0X555);	// 然后如果这一排的鸡蛋不为空，需要提，则计算这个鸡蛋由哪一个通道来提？最终生成号的踢蛋信息保存在egg_info中
				 }
			}
			else
			{
				get_pst_egg_info(1,i,&egg_infoB,0);
				if(egg_infoB->empty ==NEED_TICK_EGG)
				{
					calculate_egg_info(&per_poduction_egg,egg_infoB,0XAAA);
				}
			}
	 }
}

void show_product(int argc, char *argv[])
{
	int addr;
	int val,product_id;
	production_type per_poduction_egg;
  get_production_mange_set(&per_poduction_egg);
	for(val=0;val<12;val++)
	{
	  rt_kprintf("product id=%d  line=%04x\n",val,per_poduction_egg.pre_production[val].packge_line);	// 打印这个蛋品用了哪些线？
	}
	for(product_id=0;product_id<12;product_id++)    //6个产品
	 {
		 if(per_poduction_egg.pre_production[product_id].empty !=0)  //不为空
		 {
			  init_avg_spt_egg_info(product_id,per_poduction_egg.pre_production[product_id].packge_line,per_poduction_egg.pre_production[product_id].tray_line);
		 }
	 }
}

MSH_CMD_EXPORT(show_product,libmodbus wite buf);
