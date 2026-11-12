#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "spi_flash.h"
#include "user_set.h"
#include "mdebug.h"
#include "product_set.h"
#include "app_screen.h"
#include "dat_backup.h"
#include "system_part.h"
#include "modbus_screen.h"
#include "product_control.h"
#include "product_egg_recode.h"


ALIGN(RT_ALIGN_SIZE)



const char weight_content[11][23]=
{
"异常大,%d,%d,%d,%d\r\n",
"超巨大,%d,%d,%d,%d\r\n",
"巨大,%d,%d,%d,%d\r\n",
"超大,%d,%d,%d,%d\r\n",
"大,%d,%d,%d,%d\r\n",
"中,%d,%d,%d,%d\r\n",
"小,%d,%d,%d,%d\r\n",
"特小,%d,%d,%d,%d\r\n",
"微小,%d,%d,%d,%d\r\n",
"异常小,%d,%d,%d,%d\r\n",
"总数,%d,%d,%d,%d\r\n",
};

const char crack_content[5][23]=
{
"优,%d,%d,%d,%d\r\n",
"隐纹,%d,%d,%d,%d\r\n",
"破口,%d,%d,%d,%d\r\n",
"漏液,%d,%d,%d,%d\r\n",
"总数,%d,%d,%d,%d\r\n",
};

const char dirty_content[4][23]=
{
"优,%d,%d,%d,%d\r\n",
"良,%d,%d,%d,%d\r\n",
"差,%d,%d,%d,%d\r\n",
"总数,%d,%d,%d,%d\r\n",
};

const char blood_content[3][23]=
{
"有,%d,%d,%d,%d\r\n",
"无,%d,%d,%d,%d\r\n",
"总数,%d,%d,%d,%d\r\n",
};
const char  cvs_head[]={"供应商名称,%s\r\n日期,%d.%d.%d"};
const char  cvs_title[]={"\r\n%s分级,数量,重量(千克),平均(g),百分比\r\n"};
static recode_analisys_product_type recode_analysis_eggs_buf;
static uint16_t disp_supply_neme[10];


// 显示供应商名称
void set_disp_sypply_id(char *name)
{
	uint16_t len;
	len= rt_strlen((char *)name);
	if(len>20)
		len=20;
	DEBUG_INFO("set supply  name=%s\n",name);
	rt_memset(disp_supply_neme,0,sizeof(disp_supply_neme));
	rt_memcpy(disp_supply_neme,name,len);
    CopyToMBHotReg(4724,(uint8_t *)&disp_supply_neme[0],20);
	show_analisys_quality();
}
// 根据当前日期计算前一天或者后一天
uint32_t get_up_dat(uint32_t date,uint8_t dir)
{
	 uint8_t mon;
	 uint8_t day;
	 uint16_t year;
	 uint32_t new_day;
	 
	 year =  date/10000;
	 mon  =  (date%10000)/100;
	 day  =  date%100;
   if(dir==1)   //1 增加
	 {
     day = day +1;
		 
	   switch(mon)
		 {
			 case 4:
			 case 6:
			 case 9:
			 case 11:
	       if(day>30)
				 {
				   day =1;
					 mon=mon+1;
				 }
				 break;
		   case 1:
			 case 3:
			 case 5:
			 case 7:
			 case 8:
			 case 10:
			 case 12:
				 if(day>31)
				 {
				   day =1;
					 mon=mon+1;
					 if(mon>12)
					 {
						 mon =1;
						 year = year+1;
					 }
				 }
				 break;
			 case 2:
				 if((year%4)==0)
				 {
				   if(day>29)
					 {
						 day =1;
						 mon=mon+1;
					 }
				 }
				 else 
				 {
				   if(day>28)
					 {
						 day =1;
						 mon=mon+1;
					 }
				 }
				 break;
		 }
	 }
	 else if(dir==2)   //1 增加  2 减少
	 {
		 day = day -1; 
	   switch(mon)
		 {
			 case 4:
			 case 6:
			 case 9:
			 case 11:
	       if(day==0)
				 {
				   day =31;
					 mon=mon-1;
				 }
				 break;
		   case 1:
			 case 3:
			 case 5:
			 case 7:
			 case 8:
			 case 10:
			 case 12:
				 if(day==0)
				 {
				
					 if(mon==1)
					 {
						 day = 31;
						 mon = 12;
						 year = year-1;
					 }
					 else if(mon==3)
					 {
						 mon = 2;
						 if((year%4)==0)
						 {
						   day = 29;
						 }
						 else 
						 {
						   day = 28;
						 }
					 }
					 else if(mon==5)
					 {
						 mon =4;
						 day = 30;
					 }
					 else if(mon==7)
					 {
						 mon =6;
						 day = 30;
					 }
					 else if(mon==8)
					 {
						 mon =7;
						 day = 31;
					 }
					 else if(mon==10)
					 {
						 mon =9;
						 day = 30;
					 }
					 else if(mon==12)
					 {
						 mon =11;
						 day = 30;
					 }
				 }
				 break;
			 case 2:
				 if(day==0)
				 {
					 day =31;
					 mon=1;
				 }
				 break;
		 }
	 }
	 new_day = year*10000;
	 new_day = new_day + mon*100;
	 new_day = new_day + day;
	
	 return new_day;
}
extern void clear_save_analisys_egg_recode_index();
void clear_product_weight_info()
{
	rt_memset(&recode_analysis_eggs_buf,0,sizeof(recode_analisys_product_type));
	clear_save_analisys_egg_recode_index();
}
//mon 1-12
//day 1-31 
void save_egg_quality_recode(uint8_t mon,uint8_t day)
{
	uint8_t i,result;
	uint32_t save_addr=0;

	 if(mon>0)
		 mon = mon-1;
	 if(day>0)
		 day = day-1;
	 
	 save_addr =  mon*40;  //40 个片区为一个月的数据内存
	 save_addr += day;  //根据 天数得出偏移地址
	 save_addr = save_addr*RECODE_PRODUCT_EGGS_PER_SEC_LEN;
	 save_addr = save_addr + RECODE_PRODUCT_EGGS_INFO_START_ADDR; //加上起始地址
	 
	for(i=0;i<10;i++)
	{
		flash_erase(save_addr,4096);
		recode_analysis_eggs_buf.flag = USE_FLAG;
		result = flash_write(save_addr,4096,(uint8_t *)&recode_analysis_eggs_buf);
		if(result == RT_EOK)
		{
			break; 
		}
		rt_thread_mdelay(10);
	}
	clear_product_weight_info();
	day = day+1;
	mon = mon+1;
	DEBUG_INFO("save analisys eggs quity addr=%d mon=%d day=%d result=%d\n",save_addr,mon,day,result);
}
// 将要分级的鸡蛋添加到记录分析蛋品质量的数据中去
void recode_egg(recode_egg_dat *egg_dat, uint8_t arry_id)
{

	
	if(arry_id ==MAX_SUPPLY_COUNT-1)
		recode_analysis_eggs_buf.body[arry_id].supply_id = MIX_SYPPL_ID_FLAG;
	else 
		recode_analysis_eggs_buf.body[arry_id].supply_id = egg_dat->supply_id;
	if(recode_analysis_eggs_buf.body[arry_id].use_flag != USE_FLAG)
	{
		recode_analysis_eggs_buf.body[arry_id].use_flag = USE_FLAG;
		if(arry_id == MIX_SYPPL_ID_FLAG)
		{
		  rt_memcpy(&recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].supply_name[0],"未知供应商",strlen("未知供应商"));
		}
	}
	
	if(egg_dat->weight_leve<EGG_WEIGHT_LEVE_COUNT)
	{
		recode_analysis_eggs_buf.body[arry_id].weight_leve[egg_dat->weight_leve].egg_count++;											// 该等级的蛋个数加1
		recode_analysis_eggs_buf.body[arry_id].weight_leve[egg_dat->weight_leve].totle_weight += egg_dat->weight;	// 该等级的蛋总重量加上这个蛋的重量
	}
	
	if(egg_dat->colour_leve<EGG_COLOUR_LEVE_COUNT)
	{
	  recode_analysis_eggs_buf.body[arry_id].colour_leve[egg_dat->colour_leve].egg_count++;											// 该等级的蛋个数加1
		recode_analysis_eggs_buf.body[arry_id].colour_leve[egg_dat->colour_leve].totle_weight += egg_dat->weight;	// 该等级的蛋总重量加上这个蛋的重量
	}
	
	if(egg_dat->crack_leve<EGG_CRACK_LEVE_COUNT)
	{
	  recode_analysis_eggs_buf.body[arry_id].crack_leve[egg_dat->crack_leve].egg_count++;												// 该等级的蛋个数加1
		recode_analysis_eggs_buf.body[arry_id].crack_leve[egg_dat->crack_leve].totle_weight += egg_dat->weight;		// 该等级的蛋总重量加上这个蛋的重量
	}
	
	if(egg_dat->dity_leve<EGG_DITY_LEVE_COUNT)
	{
	  recode_analysis_eggs_buf.body[arry_id].dity_leve[egg_dat->dity_leve].egg_count++;													// 该等级的蛋个数加1
		recode_analysis_eggs_buf.body[arry_id].dity_leve[egg_dat->dity_leve].totle_weight += egg_dat->weight;			// 该等级的蛋总重量加上这个蛋的重量
	}
	
	if(egg_dat->boody_leve<EGG_BOODY_LEVE_COUNT)
	{
	  recode_analysis_eggs_buf.body[arry_id].boody_leve[egg_dat->boody_leve].egg_count++;												// 该等级的蛋个数加1
		recode_analysis_eggs_buf.body[arry_id].boody_leve[egg_dat->boody_leve].totle_weight += egg_dat->weight;		// 该等级的蛋总重量加上这个蛋的重量
	}
}
// 将要分级的鸡蛋添加到记录分析蛋品质量的数据中去，index是供应商的索引id
void recode_analysis_egg_info(recode_egg_dat *egg_dat,uint8_t index)  
{
	 
	 if(index>=MAX_SUPPLY_COUNT)
	 {
		 index = MAX_SUPPLY_COUNT-1;
	 }
	 recode_egg(egg_dat,index);
}
// 清除屏幕的蛋质量分析数据
void clear_anaylisis_egg_quity()
{
  uint8_t i;
	//重量

	for(i=0;i<(EGG_WEIGHT_LEVE_COUNT+1);i++)   //重量
	{
		WriteMBHotRegU32(3569+(i*7),0);
		WriteMBHotRegU32(3571+(i*7),0); //单位千克
		WriteMBHotRegU32(3573+(i*7),0);  //单位克
		WriteMBHotRegU16(3575+(i*7),0);
	}
	
	//颜色

	for(i=0;i<(EGG_COLOUR_LEVE_COUNT+1);i++)   
	{

		WriteMBHotRegU32(3653+(i*7),0);
		WriteMBHotRegU32(3655+(i*7),0);
		WriteMBHotRegU32(3657+(i*7),0);
		WriteMBHotRegU16(3659+(i*7),0);
	}

	 
	 //裂纹

	for(i=0;i<(EGG_CRACK_LEVE_COUNT+1);i++)   
	{
		WriteMBHotRegU32(3695+(i*7),0);
		WriteMBHotRegU32(3697+(i*7),0);
		WriteMBHotRegU32(3699+(i*7),0);
		WriteMBHotRegU16(3701+(i*7),0);
	}
	
	 
	 	 
	 //脏蛋
	
	for(i=0;i<(EGG_DITY_LEVE_COUNT+1);i++)   
	{
	
		
		WriteMBHotRegU32(3737+(i*7),0);
		WriteMBHotRegU32(3739+(i*7),0);
		WriteMBHotRegU32(3741+(i*7),0);
		WriteMBHotRegU16(3743+(i*7),0);
	}
	

	for(i=0;i<(EGG_BOODY_LEVE_COUNT+1);i++)   
	{
		WriteMBHotRegU32(3772+(i*7),0);
		WriteMBHotRegU32(3774+(i*7),0);
		WriteMBHotRegU32(3776+(i*7),0);
		WriteMBHotRegU16(3778+(i*7),0);
	}

}
// 显示蛋质量分析数据
void disp_egg_quality(recode_product_eggs_body *dat)
{
	uint8_t i;
	uint32_t egg_count=0;
	uint32_t weight_count=0;
	uint32_t avg_weight=0;
	uint32_t per_egg;
	egg_count=0;
	weight_count =0;
	
	//重量
	if(dat->use_flag !=USE_FLAG)
		return;
	for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)   
	{
		egg_count += dat->weight_leve[i].egg_count;
		weight_count += dat->weight_leve[i].totle_weight;
	}
	for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)   //重量
	{
		per_egg = dat->weight_leve[i].egg_count*1000;  //数量百分比
		per_egg = per_egg/egg_count;		// 这个等级的重量占所有等级的总重量的百分比
		
		avg_weight = dat->weight_leve[i].totle_weight/dat->weight_leve[i].egg_count;
		WriteMBHotRegU32(3569+(i*7),dat->weight_leve[i].egg_count);
		WriteMBHotRegU32(3571+(i*7),dat->weight_leve[i].totle_weight/1000); //单位千克
		WriteMBHotRegU32(3573+(i*7),avg_weight);  //单位克	// 该重量等级的鸡蛋的平均重量
		WriteMBHotRegU16(3575+(i*7),per_egg);
	}
   avg_weight = (weight_count)/egg_count;	// 所有等级的总重量除以所有等级的总数量得到该供应商的鸡蛋的平均重量
	 WriteMBHotRegU32(3646,egg_count);					// 总蛋数		
	 WriteMBHotRegU32(3648,weight_count/1000);	// 总重量
	 WriteMBHotRegU32(3650,avg_weight);					// 所有等级的总重量除以所有等级的总数量得到该供应商的鸡蛋的平均重量
	 if(egg_count>0)
	 {
	  WriteMBHotRegU32(3652,1000);
	 }
	 else 
	 {
	   WriteMBHotRegU32(3652,0);
	 }
	
	//颜色
	egg_count =0;
	weight_count =0;
	for(i=0;i<EGG_COLOUR_LEVE_COUNT;i++)   
	{
		egg_count += dat->colour_leve[i].egg_count;
		weight_count += dat->colour_leve[i].totle_weight;
	}
	for(i=0;i<EGG_COLOUR_LEVE_COUNT;i++)   
	{
		per_egg = dat->colour_leve[i].egg_count*1000;
		per_egg = per_egg/egg_count;
		avg_weight = dat->colour_leve[i].totle_weight/dat->colour_leve[i].egg_count;
		
		WriteMBHotRegU32(3653+(i*7),dat->colour_leve[i].egg_count);
		WriteMBHotRegU32(3655+(i*7),dat->colour_leve[i].totle_weight/1000);
		WriteMBHotRegU32(3657+(i*7),avg_weight);
		WriteMBHotRegU16(3659+(i*7),per_egg);
	}
	 avg_weight = weight_count/egg_count;
	 WriteMBHotRegU32(3688,egg_count);
	 WriteMBHotRegU32(3690,weight_count/1000);
	 WriteMBHotRegU32(3692,avg_weight);
	 if(egg_count>0)
	 {
		 WriteMBHotRegU32(3694,1000);
	 }
	 else 
	 {
	   WriteMBHotRegU32(3694,0);
	 }
	 
	 
	 
	 //裂纹
	egg_count =0;
	weight_count =0;
	for(i=0;i<EGG_CRACK_LEVE_COUNT;i++)   
	{
		egg_count += dat->crack_leve[i].egg_count;
		weight_count += dat->crack_leve[i].totle_weight;
	}
	for(i=0;i<EGG_CRACK_LEVE_COUNT;i++)   
	{
		per_egg = dat->crack_leve[i].egg_count*1000;
		per_egg = per_egg/egg_count;
		avg_weight = dat->crack_leve[i].totle_weight/dat->crack_leve[i].egg_count;
		
		WriteMBHotRegU32(3695+(i*7),dat->crack_leve[i].egg_count);
		WriteMBHotRegU32(3697+(i*7),dat->crack_leve[i].totle_weight/1000);
		WriteMBHotRegU32(3699+(i*7),avg_weight);
		WriteMBHotRegU16(3701+(i*7),per_egg);
	}
	 avg_weight = weight_count/egg_count;
	 WriteMBHotRegU32(3730,egg_count);
	 WriteMBHotRegU32(3732,weight_count/1000);
	 WriteMBHotRegU32(3734,avg_weight);
	 if(egg_count>0)
	 {
		 WriteMBHotRegU32(3736,1000);
	 }
	 else 
	 {
	   WriteMBHotRegU32(3736,0);
	 }
	 
	 	 
	 //脏蛋
	egg_count =0;
	weight_count =0;
	for(i=0;i<EGG_DITY_LEVE_COUNT;i++)   
	{
		egg_count += dat->dity_leve[i].egg_count;
		weight_count += dat->dity_leve[i].totle_weight;
	}
	for(i=0;i<EGG_DITY_LEVE_COUNT;i++)   
	{
		per_egg = dat->dity_leve[i].egg_count*1000;
		per_egg = per_egg/egg_count;
		avg_weight = dat->dity_leve[i].totle_weight/dat->dity_leve[i].egg_count;
		
		WriteMBHotRegU32(3737+(i*7),dat->dity_leve[i].egg_count);
		WriteMBHotRegU32(3739+(i*7),dat->dity_leve[i].totle_weight/1000);
		WriteMBHotRegU32(3741+(i*7),avg_weight);
		WriteMBHotRegU16(3743+(i*7),per_egg);
	}
	 avg_weight = weight_count/egg_count;
	 WriteMBHotRegU32(3765,egg_count);
	 WriteMBHotRegU32(3767,weight_count/1000);
	 WriteMBHotRegU32(3769,avg_weight);
	 if(egg_count>0)
	 {
		 WriteMBHotRegU32(3771,1000);
	 }
	 else 
	 {
	   WriteMBHotRegU32(3771,0);
	 }
	
	 
	  //血斑
	egg_count =0;
	weight_count =0;
	for(i=0;i<EGG_BOODY_LEVE_COUNT;i++)   
	{
		egg_count += dat->boody_leve[i].egg_count;
		weight_count += dat->boody_leve[i].totle_weight;
	}
	for(i=0;i<EGG_BOODY_LEVE_COUNT;i++)   
	{
		per_egg = dat->boody_leve[i].egg_count*1000;
		per_egg = per_egg/egg_count;
		avg_weight = dat->boody_leve[i].totle_weight/dat->boody_leve[i].egg_count;
		
		WriteMBHotRegU32(3772+(i*7),dat->boody_leve[i].egg_count);
		WriteMBHotRegU32(3774+(i*7),dat->boody_leve[i].totle_weight/1000);
		WriteMBHotRegU32(3776+(i*7),avg_weight);
		WriteMBHotRegU16(3778+(i*7),per_egg);
	}
	 avg_weight = weight_count/egg_count;
	 WriteMBHotRegU32(3786,egg_count);
	 WriteMBHotRegU32(3788,weight_count/1000);
	 WriteMBHotRegU32(3790,avg_weight);
	 if(egg_count>0)
	 {
		 WriteMBHotRegU32(3792,1000);
	 }
	 else 
	 {
	   WriteMBHotRegU32(3792,0);
	 }
}
// 获取当天的蛋质量分析数据
void get_today_quity(recode_analisys_product_type *dat)
{
   rt_memcpy(dat,&recode_analysis_eggs_buf,sizeof(recode_analisys_product_type));
}
// 显示供应商信息
// 供应商界面
void disp_sypply_info()
{

  uint8_t i;
	uint8_t totle;
	uint8_t line;
	uint8_t result;
	uint8_t mon,day;
	uint16_t supply_name[20];
	uint32_t date;
	uint32_t now_data;
	int buf;
	uint32_t read_addr;
	rtc_info_t time;
  recode_analisys_product_type *dat;
	
	date = ReadMBHotRegU32(3561);	// 获取屏幕想看的日期
	date = date%10000;
	mon = date/100;
	day = date%100;

	rt_kprintf("data=%d mon=%d day=%d\n",date,mon,day);
	line = 0;
	totle=1;
	dat = rt_malloc(sizeof(recode_analisys_product_type));
	if(dat==RT_NULL)
	{
		return;
	}
	
	  WriteMBHotRegU16(3989,10);
	  get_local_time(&time);		// 获取本地的日期
	  date = ReadMBHotRegU32(3561);
	  now_data = (time.year+1900)*10000;
		now_data = now_data+ (time.month*100);
		now_data = now_data + time.date;

		// 如果想看的日期比本地的当前日期更新
	 if(date>now_data)
	 {
		 // 更新的日期的数据肯定没有，生都没有生产
	    rt_memset(dat,0,sizeof(recode_analisys_product_type));
	 }
	 // 如果想看的日期是当天的
	 else if((time.month==mon)&&(time.date ==day))
	 {
	    rt_memcpy(dat,&recode_analysis_eggs_buf,sizeof(recode_analisys_product_type));
	 }
	 // 如果想看的是上一天的数据
	 else 
	 {
	    if(mon>0)
				mon = mon-1;
			if(day>0)
				day = day-1; 
			read_addr =  mon*40;  //40 个片区为一个月的数据内存
			read_addr += day;  //根据 天数得出偏移地址
			read_addr = read_addr*RECODE_PRODUCT_EGGS_PER_SEC_LEN;
			read_addr = read_addr + RECODE_PRODUCT_EGGS_INFO_START_ADDR; //加上起始地址
			for(i=0;i<10;i++)
			{
				result = flash_read(read_addr,sizeof(recode_analisys_product_type),(uint8_t *)dat);
				if(result == RT_EOK)
				{
					break; 
				}
				rt_thread_mdelay(10);
			}
	 }
	for(i=0;i<MAX_SUPPLY_COUNT;i++)
	{
	  if(dat->body[i].use_flag==USE_FLAG)
		{
		  totle++;
		}
	}
	if(totle>13)
	{
	  WriteMBHotRegU16(4738,2);	// 总共2页？
	}
	else 
	{
		WriteMBHotRegU16(4737,1);	// 当前第一页
	  WriteMBHotRegU16(4738,1);	// 总共2页？
	}
	// 供应商选择界面的供应商名称，程序里最多13个供应商，界面可以翻页
  for(i=0;i<13;i++)
	{
		rt_memset(supply_name,0,sizeof(supply_name));
	  CopyToMBHotReg(4739+(i*10),(uint8_t *)&supply_name[0],20);
	}
	
	// 如果供应商界面只有一个页面说明供应商小于等于13个
	// 相当于一个界面就可以显示完所有供应商
	if(ReadMBHotRegU16(4737)==1)
	{
		for(i=0;i<MAX_SUPPLY_COUNT;i++)
		{
			if(dat->body[i].use_flag==USE_FLAG)
			{
				CopyToMBHotReg(4739+(line*10),(uint8_t *)&dat->body[i].supply_name[0],20);
				line++;
			}
		}
		rt_memcpy(supply_name,"所有供应商",strlen("所有供应商"));
		CopyToMBHotReg(4739+(line*10),(uint8_t *)&supply_name[0],20);
	}
	else 
	{
	  for(i=0;i<MAX_SUPPLY_COUNT;i++)
		{
			if(dat->body[i].use_flag==USE_FLAG)
			{
				if(line>=13)
				{
					CopyToMBHotReg(4739+((line-13)*10),(uint8_t *)&dat->body[i].supply_name[0],20);
				}
				line++;
			}
		}
		if(line>=13)
		{
			rt_memcpy(supply_name,"所有供应商",strlen("所有供应商"));
			CopyToMBHotReg(4739+((line-13)*10),(uint8_t *)&supply_name[0],20);
		}
	}
	rt_free(dat);
}
// 显示蛋质量分析数据
void show_analisys_quality()
{
	uint8_t i,j,result;
	uint32_t read_addr=0;
  uint8_t mon;
	uint8_t day;
	uint8_t today_mon;
	uint8_t today_day;
	uint32_t now_date;
	uint32_t date;
	rtc_info_t time;
	
	recode_analisys_product_type *dat;
	recode_product_eggs_body *body;
	
	date = ReadMBHotRegU32(3561);
	date = date%10000;
	mon = date/100;
	day = date%100;
	dat = rt_malloc(sizeof(recode_analisys_product_type));
	if(dat==RT_NULL)
	{
		return;
	}
	clear_anaylisis_egg_quity();
	DEBUG_INFO("要清除上次的显示\n");
	 WriteMBHotRegU16(3989,10);

	 get_local_time(&time);
	 date = ReadMBHotRegU32(3561);
	 now_date = (time.year+1900)*10000;
	 now_date = now_date+ (time.month*100);
	 now_date = now_date + time.date;

	 if(date>now_date)
	 {
	    rt_memset(dat,0,sizeof(recode_analisys_product_type));
	 }
	 else if((time.month==mon)&&(time.date ==day))
	 {
	    rt_memcpy(dat,&recode_analysis_eggs_buf,sizeof(recode_analisys_product_type));
	 }
	 else 
	 {
	   if(mon>0)
			mon = mon-1;
		if(day>0)
			day = day-1; 
		read_addr =  mon*40;  //40 个片区为一个月的数据内存
		read_addr += day;  //根据 天数得出偏移地址
		read_addr = read_addr*RECODE_PRODUCT_EGGS_PER_SEC_LEN;
		read_addr = read_addr + RECODE_PRODUCT_EGGS_INFO_START_ADDR; //加上起始地址
		for(i=0;i<10;i++)
		{
			result = flash_read(read_addr,sizeof(recode_analisys_product_type),(uint8_t *)dat);
			if(result == RT_EOK)
			{
				break; 
			}
			rt_thread_mdelay(10);
		}
		if((result != RT_EOK)||(dat->flag !=USE_FLAG))
		{
			rt_free(dat);	
			return;
		}
	 }

	
	//if(disp_sypply_id ==ALL_SYPPL_ID_FLAG)
	 // 如果是显示所有的供应商的质量分析数据
	if(rt_memcmp("所有供应商",disp_supply_neme,strlen("所有供应商"))==0)
	{
		body= rt_malloc(sizeof(recode_product_eggs_body));
		if(body==RT_NULL)
		{
		 rt_free(dat);	
		 return;
		}
		  body->use_flag = USE_FLAG;
		  for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)   //重量
			{
				body->weight_leve[i].egg_count=0;
				body->weight_leve[i].totle_weight =0;
				for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
				{
					if(dat->body[j].use_flag ==USE_FLAG)
					{
						body->weight_leve[i].egg_count += dat->body[j].weight_leve[i].egg_count;
						body->weight_leve[i].totle_weight += dat->body[j].weight_leve[i].totle_weight;
					}
				}
			}
			for(i=0;i<EGG_COLOUR_LEVE_COUNT;i++)   //颜色
			{
				body->colour_leve[i].egg_count=0;
				body->colour_leve[i].totle_weight =0;
				
				for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
				{
					if(dat->body[j].use_flag ==USE_FLAG)
					{
						body->colour_leve[i].egg_count += dat->body[j].colour_leve[i].egg_count;
						body->colour_leve[i].totle_weight += dat->body[j].colour_leve[i].totle_weight;
					}

				}
			}
    for(i=0;i<EGG_CRACK_LEVE_COUNT;i++)   //裂纹 
		{
			body->crack_leve[i].egg_count=0;
			body->crack_leve[i].totle_weight =0;
				
			for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
			{
				if(dat->body[j].use_flag ==USE_FLAG)
				{
					body->crack_leve[i].egg_count += dat->body[j].crack_leve[i].egg_count;
					body->crack_leve[i].totle_weight += dat->body[j].crack_leve[i].totle_weight;
				}
			}
		}
	
		for(i=0;i<EGG_DITY_LEVE_COUNT;i++)     //脏蛋
		{
		  body->dity_leve[i].egg_count=0;
			body->dity_leve[i].totle_weight =0;
			for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
			{
				if(dat->body[j].use_flag ==USE_FLAG)
				{
					body->dity_leve[i].egg_count += dat->body[j].dity_leve[i].egg_count;
					body->dity_leve[i].totle_weight += dat->body[j].dity_leve[i].totle_weight;
				}
			}
		}
		for(i=0;i<EGG_BOODY_LEVE_COUNT;i++)   
		{
		  body->boody_leve[i].egg_count=0;
			body->boody_leve[i].totle_weight =0;
			for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
			{
				if(dat->body[j].use_flag ==USE_FLAG)
				{
					body->boody_leve[i].egg_count += dat->body[j].boody_leve[i].egg_count;
					body->boody_leve[i].totle_weight += dat->body[j].boody_leve[i].totle_weight;
				}
			}
		}
		disp_egg_quality(body);
		rt_free(body);
	}
	else 
	{
		// 显示某个供应商的
		for(i=0;i<(MAX_SUPPLY_COUNT);i++)   
		{
			uint16_t strlen1,strlen2;
			
			strlen1 = strlen((char *)disp_supply_neme);
			strlen2 = strlen((char *)&dat->body[i].supply_name[0]);
			if(strlen1<strlen2)
			strlen1 = strlen2;	
			if(strlen1>20)	strlen1 =20;		
			if(rt_memcmp(dat->body[i].supply_name,disp_supply_neme,strlen1)==0)
      {
			  DEBUG_INFO("供应商名称相同 name1=%s name2=%s\n",dat->body[i].supply_name,disp_supply_neme,disp_supply_neme);
			  disp_egg_quality(&dat->body[i]);
			}
		}
	}
		
	rt_free(dat);
}

// 通过供应商名字找到供应商的id
uint8_t find_supply_id_from_recode_dat1(char *supply_name)
{
  uint8_t i,j;
	uint16_t strlen1,strlen2;
	
	
	strlen1 = strlen((char *)supply_name);
	if(strlen1==0)   //未知供应商信息
	{
		rt_memcpy(&recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].supply_name[0],"未知供应商",strlen("未知供应商"));
		recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].use_flag = USE_FLAG;
		DEBUG_CARE("len=0 index=%d name=%s\n",MAX_SUPPLY_COUNT -1,&recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].supply_name[0]);
		return (MAX_SUPPLY_COUNT -1);
	}
	

  for(i=0;i<(MAX_SUPPLY_COUNT);i++)   //名称相同的找不到
	{
		if(recode_analysis_eggs_buf.body[i].use_flag == USE_FLAG)
		{   
			  strlen1 = strlen((char *)supply_name);
			  strlen2 = strlen((char *)&recode_analysis_eggs_buf.body[i].supply_name[0]);
				if(strlen1<strlen2)
				strlen1 = strlen2;
	      if(strlen1>20) strlen1=20;
				if(rt_memcmp(&recode_analysis_eggs_buf.body[i].supply_name[0],supply_name,strlen1)==0)
				{
					DEBUG_CARE("you index=%d name=%s\n",i,&recode_analysis_eggs_buf.body[i].supply_name[0]);
					return i;
				}
		}
	}	
	
	 for(i=0;i<(MAX_SUPPLY_COUNT);i++)   //找生产总数为0的
	{
		if(recode_analysis_eggs_buf.body[i].use_flag == USE_FLAG)
		{   
			 uint32_t egg_count=0;
			 for(j=0;j<EGG_WEIGHT_LEVE_COUNT;j++)
			{
			  	egg_count += recode_analysis_eggs_buf.body[i].weight_leve[j].egg_count;
			}
			if(egg_count==0)
			{
				 strlen1 = strlen(supply_name);
				 if(strlen1>20) strlen1=20;
			   rt_memcpy(&recode_analysis_eggs_buf.body[i].supply_name[0],supply_name,strlen1);
				 recode_analysis_eggs_buf.body[i].use_flag = USE_FLAG;
			   DEBUG_CARE("add index=%d name=%s\n",i,&recode_analysis_eggs_buf.body[i].supply_name[0]);
		     return i;
			}
		}
	}	
	
	for(i=0;i<(MAX_SUPPLY_COUNT-1);i++)    //找空白 
	{
		if(recode_analysis_eggs_buf.body[i].use_flag != USE_FLAG)
	  {
			  strlen1 = strlen(supply_name);
			  if(strlen1>20) strlen1=20;
			 rt_memcpy(&recode_analysis_eggs_buf.body[i].supply_name[0],supply_name,strlen1);
			 recode_analysis_eggs_buf.body[i].use_flag = USE_FLAG;
			 DEBUG_CARE("add index=%d name=%s\n",i,&recode_analysis_eggs_buf.body[i].supply_name[0]);
		   return i;
		}
	}
	
	rt_memcpy(&recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].supply_name[0],"未知供应商",strlen("未知供应商"));
	recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT-1].use_flag = USE_FLAG;
	DEBUG_CARE("add index=%d name=%s\n",MAX_SUPPLY_COUNT -1,&recode_analysis_eggs_buf.body[MAX_SUPPLY_COUNT -1].supply_name[0]);
	return (MAX_SUPPLY_COUNT -1);
}
// 通过供应商名字找到供应商的id(这个id是在所有的供应商列表中索引id)
uint8_t find_supply_id_from_recode_dat(char *supply_name)
{
   uint8_t index=0;
	 
   index = find_supply_id_from_recode_dat1(supply_name);
//   DEBUG_ERR("name=%s index=%d name1=%s   use flag=%02x\n",supply_name,index,&recode_analysis_eggs_buf.body[index].supply_name[0],recode_analysis_eggs_buf.body[index].use_flag);
	 return index;
}	
// 测试存储记录数据
void test_crate_eggs()
{
  uint16_t buf;
	uint32_t j,i;
	uint16_t count;
  recode_egg_dat egg;
  uint16_t index;
	uint16_t name_buf[10];
	count=0;
	for(j=0;j<20;j++)
	{
		
		rt_memset(name_buf,0,sizeof(name_buf));
		sprintf((char *)&name_buf[0],"hello=%d",j);
    index = find_supply_id_from_recode_dat((char *)name_buf);
		DEBUG_INFO("name=%s index=%d\n",name_buf,index);
		for(i=0;i<1000;i++)
		{
			egg.supply_id =j;
			buf = rand()%1100;
			egg.weight = buf+1;
			egg.weight_leve = egg.weight/101;
			buf = rand()%5;
			egg.colour_leve =buf;
			buf = rand()%5;
			egg.crack_leve =buf;
		
			buf = rand()%4;
			egg.dity_leve =buf;
			buf = rand()%2;
			egg.boody_leve =buf;
			recode_analysis_egg_info(&egg,index);
		}
	//	sprintf((char *)&name_buf[0],"n=%d\n",j);
	//	rt_kprintf("j=%d name=%s\n",j,&name_buf[0]);
	//	set_recode_egg_buf_supply_name(j,name_buf);
		rt_kprintf("j=%d name=%s\n",j,&recode_analysis_eggs_buf.body[j].supply_name[0]);
	}
}

// 保存蛋质量分析数据
// 如果当天的数据没有存储则存储当天的数据
void save_backups_anaysis_dat()
{
	rtc_info_t time;
	uint8_t mon;
	uint8_t day;
	
	get_local_time(&time);
	 
	mon = recode_analysis_eggs_buf.date/100;	
  day = recode_analysis_eggs_buf.date%100;	
	if((mon !=0)&&(day !=0))
	{
	
	  if((mon !=time.month)||(day != time.date))
		{
		   save_egg_quality_recode(mon,day);
			 clear_product_weight_info();
		}
	}
	recode_analysis_eggs_buf.date =time.month*100;
	recode_analysis_eggs_buf.date +=time.date;
  backup_save_analysis_egg_dat(&recode_analysis_eggs_buf);
	
}
// 加载当天的数据
void load_backups_anaysis_dat()
{
  backup_read_analysis_egg_dat(&recode_analysis_eggs_buf);
}

int test_recode(int argc, char **argv) //
{
	
	 uint8_t mon;
   uint8_t day;
	 mon = atoi(argv[1]);
	 day = atoi(argv[2]);
  
	if(argc ==3)
	{
		test_crate_eggs();
	}
	else if(argc ==4)
	{
	  save_egg_quality_recode(mon,day);
		clear_product_weight_info();
	}
	
	DEBUG_INFO("mesg count=%d\n",MAX_SUPPLY_COUNT);
}




	
static FILE *fp = NULL;    //
// 打开usb里面的某个文件
// 以写方式打开
int open_file(char *name)
{
	char select_file_neme[40];
	rt_memset(select_file_neme,0,sizeof(select_file_neme));
	snprintf(select_file_neme,40,"/usb/%s.csv",name);
	rt_kprintf("creat file name=%s select nema=%s\n",name,select_file_neme);
	
	for(int i=0;i<20;i++)
	{
	  rt_kprintf("i=%d\n",name[i]);
	}
	fp = fopen(select_file_neme,"w+");   //????,??
     if(fp == NULL) 
	{
		return 0;
	}
	fseek(fp,0,SEEK_SET);
	return 1;
}
// 创建脏蛋标题
void creat_dirty_title(char *write_buf)
{

    rt_memset(write_buf,0,1024);
	snprintf(write_buf,1024,cvs_title,"脏蛋");
	fwrite(write_buf,1,strlen(write_buf),fp);  
}
// 创建供应商标题
void crate_sypply_info(char *write_buf, char *name,uint16_t year,uint8_t mon,uint8_t dat)
{
	rt_memset(write_buf,0,1024);
	snprintf(write_buf,1024,cvs_head,name,year,mon,dat);
	fwrite(write_buf,1,strlen(write_buf),fp);  
}
// 创建重量标题
void creat_weight_title(char *write_buf)
{
	
    rt_memset(write_buf,0,1024);
	snprintf(write_buf,1024,cvs_title,"重量");
	fwrite(write_buf,1,strlen(write_buf),fp);  
}
// 创建裂纹标题
void creat_crack_title(char *write_buf)
{
    rt_memset(write_buf,0,1024);
	snprintf(write_buf,1024,cvs_title,"裂纹");
	fwrite(write_buf,1,strlen(write_buf),fp);  
}


// 写脏蛋数据
void create_dirty_contont(recode_product_eggs_body *dat,char *write_buf)
{
	
	uint8_t i;
	uint32_t egg_count=0;
	uint32_t weight_count=0;
	uint32_t per_egg;
	uint32_t avg_weight;
	egg_count=0;
	weight_count =0;
	
	//重量
	if(dat->use_flag !=USE_FLAG)
	{
		for(i=0;i<4;i++)
		{
			rt_memset(write_buf,0,1024);
			snprintf(write_buf,1024,&dirty_content[i][0],0,0,0,0);
			fwrite(write_buf,1,strlen(write_buf),fp);  
		}
		return;
	}
	for(i=0;i<3;i++)   
	{
		egg_count += dat->dity_leve[i].egg_count;
		weight_count += dat->dity_leve[i].totle_weight;
	}
	for(i=0;i<4;i++)
	{
		per_egg = dat->dity_leve[i].egg_count*100;  //数量百分比
		per_egg = per_egg/egg_count;
		avg_weight = dat->dity_leve[i].totle_weight/dat->dity_leve[i].egg_count;
		rt_memset(write_buf,0,1024);
		snprintf(write_buf,1024,&dirty_content[i][0],dat->dity_leve[i].egg_count
		,dat->dity_leve[i].totle_weight/1000,avg_weight,per_egg);
		fwrite(write_buf,1,strlen(write_buf),fp);  
	}
	avg_weight = (weight_count)/egg_count;
	rt_memset(write_buf,0,1024);
	if(egg_count>0)
	{
		snprintf(write_buf,1024,&dirty_content[10][0],egg_count,weight_count/1000,avg_weight,100);
	}
	else 
	{
		snprintf(write_buf,1024,&dirty_content[10][0],egg_count,weight_count/1000,avg_weight,avg_weight,0);
	}
	fwrite(write_buf,1,strlen(write_buf),fp);    
}
// 写裂纹数据
void create_crack_contont(recode_product_eggs_body *dat,char *write_buf)
{
	
	uint8_t i;
	uint32_t egg_count=0;
	uint32_t weight_count=0;
	uint32_t per_egg;
	uint32_t avg_weight;
	egg_count=0;
	weight_count =0;
	
	//重量
	if(dat->use_flag !=USE_FLAG)
	{
		for(i=0;i<5;i++)
		{
			rt_memset(write_buf,0,1024);
			snprintf(write_buf,1024,&crack_content[i][0],0,0,0,0);
			fwrite(write_buf,1,strlen(write_buf),fp);  
		}
		return;
	}
	for(i=0;i<4;i++)   
	{
		egg_count += dat->crack_leve[i].egg_count;
		weight_count += dat->crack_leve[i].totle_weight;
	}
	for(i=0;i<4;i++)
	{
		per_egg = dat->crack_leve[i].egg_count*100;  //数量百分比
		per_egg = per_egg/egg_count;
		avg_weight = dat->crack_leve[i].totle_weight/dat->crack_leve[i].egg_count;
		rt_memset(write_buf,0,1024);
		snprintf(write_buf,1024,&crack_content[i][0],dat->crack_leve[i].egg_count
		,dat->crack_leve[i].totle_weight/1000,avg_weight,per_egg);
		fwrite(write_buf,1,strlen(write_buf),fp);  
	}
	avg_weight = (weight_count)/egg_count;
	rt_memset(write_buf,0,1024);
	if(egg_count>0)
	{
		snprintf(write_buf,1024,&crack_content[10][0],egg_count,weight_count/1000,avg_weight,100);
	}
	else 
	{
		snprintf(write_buf,1024,&crack_content[10][0],egg_count,weight_count/1000,avg_weight,avg_weight,0);
	}
	fwrite(write_buf,1,strlen(write_buf),fp);    
}
// 写重量数据
void create_weight_contont(recode_product_eggs_body *dat,char *write_buf)
{
	
	uint8_t i;
	uint32_t egg_count=0;
	uint32_t weight_count=0;
	uint32_t per_egg;
	uint32_t avg_weight;
	egg_count=0;
	weight_count =0;
	
	//重量
	if(dat->use_flag !=USE_FLAG)
	{
		for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)
		{
			rt_memset(write_buf,0,1024);
			snprintf(write_buf,1024,&weight_content[i][0],0,0,0,0);
			fwrite(write_buf,1,strlen(write_buf),fp);  
		}
		return;
	}
	for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)   
	{
		egg_count += dat->weight_leve[i].egg_count;
		weight_count += dat->weight_leve[i].totle_weight;
	}
	for(i=0;i<10;i++)
	{
		per_egg = dat->weight_leve[i].egg_count*100;  //数量百分比
		per_egg = per_egg/egg_count;
		avg_weight = dat->weight_leve[i].totle_weight/dat->weight_leve[i].egg_count;
		rt_memset(write_buf,0,1024);
		snprintf(write_buf,1024,&weight_content[i][0],dat->weight_leve[i].egg_count
		,dat->weight_leve[i].totle_weight/1000,avg_weight,per_egg);
		fwrite(write_buf,1,strlen(write_buf),fp);  
	}
	avg_weight = (weight_count)/egg_count;
	rt_memset(write_buf,0,1024);
	if(egg_count>0)
	{
		snprintf(write_buf,1024,&weight_content[10][0],egg_count,weight_count/1000,avg_weight,100);
	}
	else 
	{
		snprintf(write_buf,1024,&weight_content[10][0],egg_count,weight_count/1000,avg_weight,avg_weight,0);
	}
	fwrite(write_buf,1,strlen(write_buf),fp);    
}
// 拷贝蛋质量数据到u盘
int copy_egg_quitiy_to_udisk()
{
	
	uint8_t index;
	uint8_t mon;
	uint8_t day;
	uint8_t i,j,result;
	uint16_t loop;
	uint16_t year;
	uint16_t per_egg;
	uint16_t export_day;
	uint32_t read_addr=0;
    uint32_t writeaddr;
	uint32_t date;
	rtc_info_t time;
	char name[20];
	char *write_buf;

	recode_analisys_product_type *dat;
	recode_product_eggs_body *recode_body;
	

    if(check_mount_usb_divice()==0)
    {
		CopyToMBHotReg(3991,"请插入U盘",strlen("请插入U盘"));
		return 0;
    }
	 rt_memset(name,0,20);
     CopyFromMBHotReg(4724,(uint8_t *)&name[0],20);
	 name[19]=0;
	if(open_file(name)==0)
	{
		CopyToMBHotReg(3991,"文件创建失败",strlen("文件创建失败"));
		return 0;
	}
	write_buf = rt_malloc(1024);
	if(write_buf == RT_NULL)
	{
		CopyToMBHotReg(3991,"内存不足",strlen("内存不足"));
		return 0;
	}
	dat = rt_malloc(sizeof(recode_analisys_product_type));
	if(dat==RT_NULL)
	{
		CopyToMBHotReg(3991,"内存不足",strlen("内存不足"));
		rt_free(write_buf);	
		return 0;
	}
	recode_body = rt_malloc(sizeof(recode_product_eggs_body));
	if(dat==RT_NULL)
	{
		CopyToMBHotReg(3991,"内存不足",strlen("内存不足"));
		rt_free(dat);	
		rt_free(write_buf);		
		return 0;
	}
	CopyToMBHotReg(3991,"正在导出...",strlen("正在导出..."));
	rt_thread_mdelay(500);
	get_local_time(&time);
    date = (time.year+1900)*10000;
	date = date+ (time.month*100);
	date = date + time.date;
	
	//从这里循环 get_up_dat(date,2);
	
	export_day =ReadMBHotRegU16(3989);
	if(export_day>200) export_day=200;
	if(export_day<10) export_day=10;
	for(loop=0;loop<export_day;loop++)
	{
		
		year =  date/10000;
		mon  =  (date%10000)/100;
		day  =  date%100;
		rt_kprintf("i=%d year=%d mon=%d day=%d \n",loop,year,mon,day);
		
		if(mon>0)
			mon = mon-1;
		if(day>0)
			day = day-1; 
		read_addr =  mon*40;  //40 个片区为一个月的数据内存
		read_addr += day;  //根据 天数得出偏移地址
		read_addr = read_addr*RECODE_PRODUCT_EGGS_PER_SEC_LEN;
		read_addr = read_addr + RECODE_PRODUCT_EGGS_INFO_START_ADDR; //加上起始地址
		for(i=0;i<10;i++)
		{
			result = flash_read(read_addr,sizeof(recode_analisys_product_type),(uint8_t *)dat);
			if(result == RT_EOK)
			{
				break; 
			}
			
			rt_thread_mdelay(10);
		}
	    if(loop==0)
		{
		  get_today_quity(dat);
		}
		////这里寻找供应商
		uint8_t sypply_id=0;
		
		if(dat->body[sypply_id].use_flag !=USE_FLAG)
		{
			rt_kprintf("mon=%d date=%d 没有记入退出\n",mon+1,day+1);
            recode_body->use_flag = !USE_FLAG;
				
						crate_sypply_info(write_buf,name,year,mon+1,day+1);
						creat_weight_title(write_buf);
						create_weight_contont(recode_body,write_buf);
		
						creat_crack_title(write_buf);
						create_crack_contont(recode_body,write_buf);
		
						creat_dirty_title(write_buf);
						create_dirty_contont(recode_body,write_buf);
						rt_memset(write_buf,0,1024);
						snprintf(write_buf,1024,"\r\n\r\n");
						fwrite(write_buf,1,strlen(write_buf),fp);  
		}
		else 
		{
			CopyFromMBHotReg(4724,(uint8_t *)&name[0],20);
			name[19]=0;
			rt_kprintf("name=%s\n",name);
			if(rt_memcmp("所有供应商",&name[0],strlen("所有供应商"))==0)
			{
				rt_kprintf("所有供应商\n");
				rt_memset(recode_body,0,sizeof(recode_product_eggs_body));
				for(i=0;i<EGG_WEIGHT_LEVE_COUNT;i++)   //重量
				{
					recode_body->weight_leve[i].egg_count=0;
					recode_body->weight_leve[i].totle_weight =0;
					for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
					{
						if(dat->body[j].use_flag ==USE_FLAG)
						{
							recode_body->weight_leve[i].egg_count += dat->body[j].weight_leve[i].egg_count;
							recode_body->weight_leve[i].totle_weight += dat->body[j].weight_leve[i].totle_weight;
						}
					}
				}
				for(i=0;i<EGG_COLOUR_LEVE_COUNT;i++)   //颜色
				{
					recode_body->colour_leve[i].egg_count=0;
					recode_body->colour_leve[i].totle_weight =0;
					for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
					{
						if(dat->body[j].use_flag ==USE_FLAG)
						{
							recode_body->colour_leve[i].egg_count += dat->body[j].colour_leve[i].egg_count;
							recode_body->colour_leve[i].totle_weight += dat->body[j].colour_leve[i].totle_weight;
						}
					}
				}
				for(i=0;i<EGG_CRACK_LEVE_COUNT;i++)   //裂纹 
				{
					recode_body->crack_leve[i].egg_count=0;
					recode_body->crack_leve[i].totle_weight =0;
					for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
					{
						if(dat->body[j].use_flag ==USE_FLAG)
						{
							recode_body->crack_leve[i].egg_count += dat->body[j].crack_leve[i].egg_count;
							recode_body->crack_leve[i].totle_weight += dat->body[j].crack_leve[i].totle_weight;
						}
					}
				}
	
				for(i=0;i<EGG_DITY_LEVE_COUNT;i++)     //脏蛋
				{
					recode_body->dity_leve[i].egg_count=0;
					recode_body->dity_leve[i].totle_weight =0;
					for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
					{
						if(dat->body[j].use_flag ==USE_FLAG)
						{
							recode_body->dity_leve[i].egg_count += dat->body[j].dity_leve[i].egg_count;
							recode_body->dity_leve[i].totle_weight += dat->body[j].dity_leve[i].totle_weight;
						}
					}
				}
				for(i=0;i<EGG_BOODY_LEVE_COUNT;i++)   
				{
					recode_body->boody_leve[i].egg_count=0;
					recode_body->boody_leve[i].totle_weight =0;
					for(j=0;j<(MAX_SUPPLY_COUNT);j++)   
					{
						if(dat->body[j].use_flag ==USE_FLAG)
						{
							recode_body->boody_leve[i].egg_count += dat->body[j].boody_leve[i].egg_count;
							recode_body->boody_leve[i].totle_weight += dat->body[j].boody_leve[i].totle_weight;
						}
					}
				}
				        recode_body->use_flag =USE_FLAG;
				
						crate_sypply_info(write_buf,name,year,mon+1,day+1);
						creat_weight_title(write_buf);
						create_weight_contont(recode_body,write_buf);
		
						creat_crack_title(write_buf);
						create_crack_contont(recode_body,write_buf);
		
						creat_dirty_title(write_buf);
						create_dirty_contont(recode_body,write_buf);
						rt_memset(write_buf,0,1024);
						snprintf(write_buf,1024,"\r\n\r\n");
						fwrite(write_buf,1,strlen(write_buf),fp);  
			}
			else 
			{
				for(i=0;i<(MAX_SUPPLY_COUNT);i++)   
				{
					uint16_t strlen1,strlen2;
			
					strlen1 = strlen((char *)name);
					strlen2 = strlen((char *)&dat->body[i].supply_name[0]);
					if(strlen1<strlen2)
					strlen1 = strlen2;	
					if(strlen1>20)	strlen1 =20;		
					if(rt_memcmp(dat->body[i].supply_name,name,strlen1)==0)
					{
						sypply_id = i;
						DEBUG_INFO("供应商名称相同 name1=%s name2=%s\n",dat->body[i].supply_name,name,disp_supply_neme);
						crate_sypply_info(write_buf,name,year,mon+1,day+1);
						creat_weight_title(write_buf);
						create_weight_contont(&dat->body[sypply_id],write_buf);
		
						creat_crack_title(write_buf);
						create_crack_contont(&dat->body[sypply_id],write_buf);
		
						creat_dirty_title(write_buf);
						create_dirty_contont(&dat->body[sypply_id],write_buf);
						rt_memset(write_buf,0,1024);
						snprintf(write_buf,1024,"\r\n\r\n");
						fwrite(write_buf,1,strlen(write_buf),fp);  
						break;
					}
				}
				
				rt_kprintf("找不到供应商 name=%s\n",name);
			}	
		}
		date = get_up_dat(date,2);
		rt_kprintf("date=%d\n",date);
		
	}
	
	
	
	fflush(fp);
    fclose(fp); //????
	rt_free(write_buf);
	rt_free(dat);
	rt_free(recode_body);
	rt_kprintf("导出完毕\n");
	return 1;
}


static uint8_t export_flag=0;
void set_export_recode()
{
  export_flag=1;
}
void eudisk()
{
	if(export_flag==0)
		return ;
	if(copy_egg_quitiy_to_udisk()>0)
	{
		CopyToMBHotReg(3991,"导出成功",strlen("导出成功"));
		rt_kprintf("导出成功\n");
	}
	else
	{
	  rt_kprintf("导出失败\n");
	}
	export_flag=0;
}
// 清除flash中保存的产品生产历史数据
void ceqr()
{
	uint16_t i;
	uint32_t save_addr=0;

	for(i=0;i<(12*40);i++)
	{
	    save_addr = i*RECODE_PRODUCT_EGGS_PER_SEC_LEN;
		save_addr +=RECODE_PRODUCT_EGGS_INFO_START_ADDR;
	    flash_erase(save_addr,4096);
		rt_thread_mdelay(20);
	}
	
	DEBUG_INFO("清除质量历史记录完毕\n");
}

MSH_CMD_EXPORT(test_recode, test SPI Flash log.);

