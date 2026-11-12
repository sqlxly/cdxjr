#include <rtthread.h>
#include <rtdevice.h>
#include "mdebug.h"
#include "at24cxx.h"
#include <user_set.h>
#include <spi_flash.h>
#include "product_set.h"
#include "product_hostroy.h"
#include "egg_control.h"

ALIGN(RT_ALIGN_SIZE)

#define EEP_ALREADY_PRODUCT_LEN (sizeof(already_produced_log_type)+2)
#define EEP_ALREADY_PRODUCT_COUNT 2048/EEP_ALREADY_PRODUCT_LEN

// 初始化产品历史信息
void product_logs_hostoy_factry_init()
{
      uint8_t *buf;
			uint8_t i;
	    buf = rt_malloc(10);
      rt_memset(buf,0,10);
			buf[0]=FLASH_RECODE_DAT_FLAG;
			buf[5]=FLASH_RECODE_DAT_FLAG;
			at24cxx_multy_write(PRODUCT_LOGS_COUNT_EEP_ADDR,buf,8);
			//flash_erase(PRODUCT_HOSTRAY_LOGS_OFFEST_ADDR,4096);
	        //rt_thread_mdelay(10);
	        //flash_erase(PRODUCT_HOSTRAY_LOGS_OFFEST_ADDR+4096,4096);
	    for(i=0;i<EEP_ALREADY_PRODUCT_COUNT;i++)
			{
			  buf[0]=0;
			  buf[1]=0;	
			  at24cxx_multy_write((EEP_ALREADY_PRODUCT_ADDR+(i*EEP_ALREADY_PRODUCT_LEN)),buf,1);
			}
			DEBUG_CARE("生产产品重新初始化\n");
      rt_free(buf);
}
// 获取与多少个产品历史信息
uint16_t get_product_hostroy_count()
{
  uint16_t count=0;	
  uint8_t i,result;
  uint8_t buf[2];
  for(i=0;i<EEP_ALREADY_PRODUCT_COUNT;i++)
  {
    result = at24cxx_multy_read((EEP_ALREADY_PRODUCT_ADDR+(i*EEP_ALREADY_PRODUCT_LEN)),buf,1);
    if(result == RT_EOK)
		{
			if(buf[0]==0xaa)
				count++;
		}
  }
  return count;
}
// 获取当前产品在产品历史数据的第几个索引
uint32_t  get_current_porduct_index()
{  
	uint8_t result; 
    uint8_t buf[10];
	uint32_t index=0;
	result = at24cxx_multy_read(PRODUCT_LOGS_COUNT_EEP_ADDR,buf,8);
	if(result == RT_EOK)
	{
		if((buf[0]==FLASH_RECODE_DAT_FLAG)&& (buf[5] ==FLASH_RECODE_DAT_FLAG)) //两边都没有检测到头
		{
			index = buf[1]<<8;
			index +=buf[2];
			if(index>=EEP_ALREADY_PRODUCT_COUNT)
			{
		      index=0;
			}
			
		}
	}
	return index;
}	
uint32_t count_product_recode_addr()  //存储使用
{
    uint8_t result;
	uint8_t *buf;
	uint32_t count;
    uint32_t addr=0; 
	count =0;
	buf = rt_malloc(10);
	addr=EEP_ALREADY_PRODUCT_ADDR;
	if(buf ==RT_NULL)
	{
	  addr = EEP_ALREADY_PRODUCT_ADDR;
	  return addr;
	}
	rt_memset(buf,0,10);
    result = at24cxx_multy_read(PRODUCT_LOGS_COUNT_EEP_ADDR,buf,8);
	if(result == RT_EOK)
	{
		if((buf[0]==FLASH_RECODE_DAT_FLAG)&& (buf[5] ==FLASH_RECODE_DAT_FLAG)) //两边都没有检测到头
		{
			count = buf[1]<<8;
			count +=buf[2];
			count ++;
			if(count>=EEP_ALREADY_PRODUCT_COUNT)
			{
		      count=0;
			}
			buf[0]=FLASH_RECODE_DAT_FLAG;
			buf[5]=FLASH_RECODE_DAT_FLAG;
			buf[1]=count>>8;
			buf[2]=(count&0x00ff);
			buf[3]=buf[1];
			buf[4]=buf[2];
			at24cxx_multy_write(PRODUCT_LOGS_COUNT_EEP_ADDR,buf,8);
		}
		else   //初始化 从0开始
		{
			count=0;
			buf[0]=FLASH_RECODE_DAT_FLAG;
			buf[5]=FLASH_RECODE_DAT_FLAG;
			buf[1]=count>>8;
			buf[2]=(count&0x00ff);
			buf[3]=buf[1];
			buf[4]=buf[2];
			at24cxx_multy_write(PRODUCT_LOGS_COUNT_EEP_ADDR,buf,8);
		    count=0;
		}
	}
	else
	{
	  count=0;
	}
	count = count*EEP_ALREADY_PRODUCT_LEN;
	addr = count + EEP_ALREADY_PRODUCT_ADDR;
    rt_free(buf);
    return addr;
 
}



void get_pruduct_hostroy_log(already_produced_log_type *already_produced_log,uint16_t index)
{
	 uint8_t result;
     uint32_t addr;
	 uint8_t *buf;
	
	 buf = rt_malloc(EEP_ALREADY_PRODUCT_LEN);
	 if(buf ==RT_NULL)
	 {
	   return ;
	 }
	 if(index>=EEP_ALREADY_PRODUCT_COUNT)
	 {
			rt_memset(already_produced_log,0,sizeof(already_produced_log_type));
		    return ;
	  }
	
	 addr =index*EEP_ALREADY_PRODUCT_LEN;
	 addr +=EEP_ALREADY_PRODUCT_ADDR;
	 
	 rt_kprintf("read addr=%d index=%d\n",addr,index);
     result = at24cxx_multy_read(addr,buf,EEP_ALREADY_PRODUCT_LEN);
	 if((result == RT_EOK)&&(buf[0]=FLASH_RECODE_DAT_FLAG))
	 {
		 rt_memcpy((uint8_t *)already_produced_log,&buf[1],sizeof(already_produced_log_type));
	 }
	 else 
	 {
	   rt_memset((uint8_t *)already_produced_log,0,sizeof(already_produced_log_type));
	 }
	 rt_free(buf);
}
//13312

// 将当前的生产的信息保存为历史信息
void save_product_recode(already_produced_log_type *already_produced_log)
{
	uint8_t result;
	uint32_t save_addr=0;
	uint8_t *buf;
	
   // rt_kprintf("name=%s,line=%d bar=%d\n",already_produced_log->packge_name,already_produced_log->paqckge_line
	//,already_produced_log->product_bar);
	buf = rt_malloc(sizeof(already_produced_log_type)+10);
	if(buf ==RT_NULL)
	{
		DEBUG_ERR("内存分配失败\n");
		return ;
	}
	// 获取当前产品的存储索引
	save_addr = count_product_recode_addr();
	if((save_addr<EEP_ALREADY_PRODUCT_ADDR)||(save_addr>EEP_ALREADY_PRODUCT_ADDR+2048))
	{
	  rt_free(buf);
	  return;
	}
	buf[0]=0xaa;	// 表示该索引的数据是有用的历史信息
	rt_memcpy(&buf[1],already_produced_log,sizeof(already_produced_log_type));

	result = at24cxx_multy_write(save_addr,buf,sizeof(already_produced_log_type)+1);
	//rt_kprintf("保存生产历史到EEP ADDR=%d result=%d\n",save_addr,result);
	rt_free(buf);
	rt_thread_mdelay(100);
	
}


int test_log(void *args)
{
  already_produced_log_type *dat;
	
	rtc_info_t time;
	
	dat = rt_malloc(sizeof(already_produced_log_type));
	rt_memset(dat,0,sizeof(already_produced_log_type));
    get_local_time(&time);
	
  //dat->date = (time.year+1900)*10000;
	//dat->date  += (time.month*100);
	//dat->date  += time.date;
	
	dat->date = time.hour*10000;
	dat->date +=time.min*100;
	dat->date +=time.sec;
  //dat->start_time = (time.hour *100)+time.min;
	dat->start_time = (time.min*100)+ time.sec;
	dat->stop_time  = (time.hour *100)+time.min ;
	rt_kprintf("date=%d start time=%d\n",dat->date,dat->start_time);
	dat->paqckge_line=2;
	rt_memcpy(dat->product_name,"精品蛋",strlen("精品蛋"));
	rt_memcpy(dat->packge_name,"6*5包装盒",strlen("6*5包装盒"));
	dat->product_count = 100;
    save_product_recode(dat);
	rt_free(dat);
}

// 将当前已经生产的产品信息存为生产历史信息
void save_product_hostroy(uint8_t index)  //保存生产历史 总的生产信息
{
	 rtc_info_t time;
	 production_type *product_dat;
	 already_produced_log_type *hostory_dat;
	 alread_production_type alread_dat;
	 uint8_t packge_str[12];
	 get_local_time(&time);
	 hostory_dat = rt_malloc(sizeof(already_produced_log_type));
	 product_dat = rt_malloc(sizeof(production_type));
	 if((hostory_dat ==RT_NULL)||(product_dat == RT_NULL))
	 {
			return;
	 }
   rt_memset(product_dat,0,sizeof(production_type));
	 rt_memset(hostory_dat,0,sizeof(already_produced_log_type));
 
	 if(index<12)
	 {
	     get_production_mange_set(product_dat);
		 get_already_product_egg(&alread_dat);
		 hostory_dat->date  = alread_dat.pruduct_info[index].date;
		 hostory_dat->start_time = alread_dat.pruduct_info[index].start_time;
	     hostory_dat->stop_time  = (time.hour *100)+time.min ;
		 

		 rt_memcpy(hostory_dat->product_name,product_dat->pre_production[index].product_name,20);
		 rt_memcpy(hostory_dat->packge_name,product_dat->pre_production[index].packge_name,20);
		 hostory_dat->paqckge_line = product_dat->pre_production[index].packge_line;
		 
		 hostory_dat->task_box = product_dat->pre_production[index].task_box_count;
		 hostory_dat->task_count = product_dat->pre_production[index].task_egg_count;
		 hostory_dat->product_box = alread_dat.pruduct_info[index].pruduct_box_count;
		 hostory_dat->product_count = alread_dat.pruduct_info[index].pruduct_egg_count;
		 hostory_dat->product_bar = alread_dat.pruduct_info[index].prpduct_bar;
		 
		 if(hostory_dat->product_count>0)
		 {
		   //rt_kprintf("index=%d egg count=%d\n",index,hostory_dat->product_count);
		   save_product_recode(hostory_dat);
		 }
   }
   rt_free(product_dat);
   rt_free(hostory_dat);
   //DEBUG_CARE("保存生产历史 ID=%d\n",index);
}

//MSH_CMD_EXPORT(test_log, test SPI Flash log.);