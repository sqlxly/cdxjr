#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "dat_backup.h"
#include "product_set.h"
#include "mdebug.h"
#include "crc32.h"
#include "anasis_egg.h"
#include "at24cxx.h"
#include "spi_flash_sfud.h"

ALIGN(RT_ALIGN_SIZE)

static production_type back_per_product_info;
static throw_ch_product_type throw_ch_info;
static uint8_t PER_PRODUCT_SAVE_OFFEST=0;
void read_bak_per_pruduct_dat(production_type *dat)
{
  rt_memcpy(dat,&back_per_product_info,sizeof(production_type));
}
void show_save_addr()
{
	
	DEBUG_INFO("already addr=%d size=%d\n",ALREAD_PRODUCT_DAT_OFFEST_ADDR,sizeof(alread_production_type));
	DEBUG_INFO("per pruduct addr=%d size=%d\n",PER_PRODUCT_DAT_OFFEST_ADDR,sizeof(production_type));
	DEBUG_INFO("throw addr=%d size=%d\n",PRODUCT_HTROW_CH_PART_OFFEST_ADDR,sizeof(throw_ch_product_type));
	DEBUG_INFO("ayaysisi addr=%d size=%d\n",RECODE_ANAYSIS_EGG_DAT_OFFEST_ADDR,sizeof(recode_analisys_product_type));
	DEBUG_INFO("ANAYSIS DAT LEN=%d\n",RECODE_ANAYSIS_EGG_DAT_LEN);
  DEBUG_INFO("already addr=%d per product addr=%d  throw addr=%d\n",ALREAD_PRODUCT_DAT_OFFEST_ADDR,PER_PRODUCT_DAT_OFFEST_ADDR,PRODUCT_HTROW_CH_PART_OFFEST_ADDR);
  DEBUG_INFO("supply egg count=%d\n",MAX_SUPPLY_COUNT);
}
void backup_save_alread_pruduct_dat(alread_production_type *dat)
{

  dat->flag = BACKUP_SRAM_FLAG;
	bkp_sram_write(ALREAD_PRODUCT_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(alread_production_type));
}
void backup_read_alread_pruduct_dat(alread_production_type *dat)
{
	bkp_sram_read(ALREAD_PRODUCT_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(alread_production_type));
	if(dat->flag != BACKUP_SRAM_FLAG)
	{
		rt_memset(dat,0,sizeof(alread_production_type));
		DEBUG_WARN("已经生产信息读取错误\n");
	}
}



extern void bkp_sram_write_NO_INT(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len);
void backup_read_per_pruduct_dat(production_type *dat)
{
	rt_err_t result;
    uint32_t crc;
  if(PER_PRODUCT_SAVE_OFFEST>12) PER_PRODUCT_SAVE_OFFEST=12;
	
	bkp_sram_read(PER_PRODUCT_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(production_type));
	crc = CRC32_file((uint8_t *)dat,sizeof(production_type)-4);
	if((dat->crc != crc) && (dat->crc != BACKUP_SRAM_FLAG))
	{
		rt_memset(dat,0,sizeof(production_type));

		DEBUG_WARN("SDRAM预生产信息读取错误\n");
		result = spi_flash_read(PER_PRUDUCT_ADDR_SPI_ADDR+(PER_PRODUCT_SAVE_OFFEST*4096),(uint8_t *)dat,sizeof(production_type));
		//result =  eep_mutp_read(EEP_PER_PRODUCT_ADDR,(uint8_t *)dat,sizeof(production_type));
		if(result != RT_EOK)
		{
			rt_memset(dat,0,sizeof(production_type));
			DEBUG_WARN("FLASH 预生产信息读取失败...\n");
		}
		else
		{
		  crc = CRC32_file((uint8_t *)dat,sizeof(production_type)-4);
		  if((dat->crc != crc) && (dat->crc != BACKUP_SRAM_FLAG))
			{
				rt_memset(dat,0,sizeof(production_type));
				DEBUG_WARN("FLASH 预生产信息校验失败...\n");
			}
			else 
			{
				 //bkp_sram_write_NO_INT(PER_PRODUCT_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(production_type));
			   DEBUG_INFO("FLASH预生产信息读取成功\n");
			}
			
		}
	}
	 else 
	 {
	   DEBUG_INFO("SDRAM预生产信息读取成功\n");
	 }
	 rt_memcpy(&back_per_product_info,dat,sizeof(production_type));
}

uint16_t throw_part_err_count=0;
void read_thorw_ch_ionfo(throw_ch_product_type *dat)
{
  throw_part_err_count+=2;
  rt_memcpy(dat,&throw_ch_info,sizeof(throw_ch_product_type));
}

void backup_read_throw_ch_part(throw_ch_product_type *dat)
{
	 uint32_t crc;
	
	 bkp_sram_read(PRODUCT_HTROW_CH_PART_OFFEST_ADDR,(uint8_t *)dat,sizeof(throw_ch_product_type));
	
	 crc = CRC32_file((uint8_t *)dat,sizeof(throw_ch_product_type)-4);
	
     if(dat->crc != crc)
	 {
	   throw_part_err_count++;
		 
	   spi_flash_read(PER_THROW_ADDR_SPI_ADDR,(uint8_t *)dat,sizeof(throw_ch_product_type));
	   DEBUG_ERR("抛弃通道设置读取失败\n");
	   //rt_memset(dat,0,sizeof(throw_ch_product_type));
	 }
	 else 
	 {
	   DEBUG_INFO("抛弃通道设置读取成功\n");
	 }
	 rt_memcpy(&throw_ch_info,dat,sizeof(throw_ch_product_type));
}

void backup_save_throw_ch_part(throw_ch_product_type *dat)
{
	 uint32_t crc;
	
	 crc = CRC32_file((uint8_t *)dat,sizeof(throw_ch_product_type)-4);
	 dat->crc = crc;
	 bkp_sram_write(PRODUCT_HTROW_CH_PART_OFFEST_ADDR,(uint8_t *)dat,sizeof(throw_ch_product_type));
	 spi_flash_write(PER_THROW_ADDR_SPI_ADDR,(uint8_t *)dat,sizeof(throw_ch_product_type));
}
void copy_dat(uint8_t oid,uint8_t nid,production_type *dat)
{
	pre_production_info buf;
	rt_memcpy(&buf,&dat->pre_production[nid],sizeof(pre_production_info));
	rt_memcpy(&dat->pre_production[nid],&dat->pre_production[oid],sizeof(pre_production_info));
	rt_memcpy(&dat->pre_production[oid],&buf,sizeof(pre_production_info));
}
void paixue_per_product(production_type *dat)
{
	
	uint8_t id;
	uint8_t  i,k;
	uint16_t line;
	uint16_t index;
	int result;
	

	for(k=0;k<6;k++)
	{
	  id = k*2;
	  index = 1<<id;
	  for(i=0;i<12;i++)
	  {
		dat->pre_production[i].auto_clacure_weight[0]=0;
		if((dat->pre_production[i].packge_line & index)&&(dat->pre_production[i].empty !=0))
        {
	     if(i!=k)
		 {
		    copy_dat(i,k,dat);
		 }
	    }
	  }
	}
	
	for(k=0;k<6;k++)
	{
	  id = k*2+1;
	  index = 1<<id;
	  for(i=0;i<12;i++)
	  {
		dat->pre_production[i].auto_clacure_weight[0]=0;
		if((dat->pre_production[i].packge_line & index)&&(dat->pre_production[i].empty !=0))
        {
	     if(i!=(k+6))
		 {
		    copy_dat(i,k+6,dat);
		 }
	    }
	  }
	}
}
void paixue_per_product1(production_type *dat)
{
	uint8_t i,j;
	uint8_t count;
	int result;
	pre_production_info buf;
	count=0;
	for(i=0;i<12;i++)
	{
		if(dat->pre_production[i].empty !=0)
		{
		  count++;
		}
	}
	for(i=0;i<count;i++)
	{
		for(j=0;j<(count-1);j++)
		{
		  if(dat->pre_production[j].packge_line>dat->pre_production[j+1].packge_line)
		  {
		    rt_memcpy(&buf,&dat->pre_production[j+1],sizeof(pre_production_info));
			rt_memcpy(&dat->pre_production[j+1],&dat->pre_production[j],sizeof(pre_production_info));
			rt_memcpy(&dat->pre_production[j],&buf,sizeof(pre_production_info));
		  }	
		}
	}
}

extern void show_per_product_spi_addr_offest(uint8_t dat);
void check_per_pruduct_save_addr()
{
	  uint8_t offest_addr;
	  uint8_t read_buf[2];
		rt_err_t result;
	  rtc_info_t time;
	  
	  result= at24cxx_multy_read(EEP_SAVE_PER_PRODUCT_INFO_ADDR,(uint8_t*)read_buf,2);  
		get_local_time(&time);
	  if(read_buf[0]==0xa5)
		{
		  offest_addr = time.month%12;
		  if(offest_addr != read_buf[1])
			{
				 PER_PRODUCT_SAVE_OFFEST = read_buf[1];
				 backup_read_per_pruduct_dat(&back_per_product_info);
				 read_buf[0]=0xa5;
				 read_buf[1] = offest_addr;
			   at24cxx_multy_write(EEP_SAVE_PER_PRODUCT_INFO_ADDR,(uint8_t*)read_buf,2);  
				 backup_save_per_pruduct_dat(&back_per_product_info);
			}
			PER_PRODUCT_SAVE_OFFEST = offest_addr;
		}
		else 
		{
			PER_PRODUCT_SAVE_OFFEST = 0;
			backup_read_per_pruduct_dat(&back_per_product_info);

			PER_PRODUCT_SAVE_OFFEST = time.month%12;
			read_buf[0]=0xa5;
			read_buf[1] = time.month%12;
			at24cxx_multy_write(EEP_SAVE_PER_PRODUCT_INFO_ADDR,(uint8_t*)read_buf,2);  
      backup_save_per_pruduct_dat(&back_per_product_info);
		}
		
    show_per_product_spi_addr_offest(PER_PRODUCT_SAVE_OFFEST);
   
	//DEBUG_INFO("read time=%d-%d-%d   %d:%d-%d\n",time.year+1900,time.month,time.date,
}


void backup_save_per_pruduct_dat(production_type *dat)
{
	rt_err_t result;
	rtc_info_t time;
	uint32_t crc;
    paixue_per_product(dat);
    get_local_time(&time);
	dat->date  = (time.month*10);
    dat->date  += time.date;
	crc = CRC32_file((uint8_t *)dat,sizeof(production_type)-4);
    dat->crc= crc;
	bkp_sram_write(PER_PRODUCT_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(production_type));
	//DEBUG_WARN("sdram预生产信息存储完成...\n");
	if(PER_PRODUCT_SAVE_OFFEST>12) PER_PRODUCT_SAVE_OFFEST=12;
	spi_flash_ease(PER_PRUDUCT_ADDR_SPI_ADDR+(PER_PRODUCT_SAVE_OFFEST*4096));
	result = spi_flash_write(PER_PRUDUCT_ADDR_SPI_ADDR+(PER_PRODUCT_SAVE_OFFEST*4096),(uint8_t *)dat,sizeof(production_type));
	//result = eep_mutp_write(EEP_PER_PRODUCT_ADDR,(uint8_t *)dat,sizeof(production_type));
	if(result == RT_EOK)
	{
		//DEBUG_WARN("flash 预生产信息存储完成...\n");
	}
	rt_memcpy(&back_per_product_info,dat,sizeof(production_type));
}


void backup_save_analysis_egg_dat(recode_analisys_product_type *dat)
{
  dat->flag = BACKUP_SRAM_FLAG;
	bkp_sram_write(RECODE_ANAYSIS_EGG_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(recode_analisys_product_type));
}
void backup_read_analysis_egg_dat(recode_analisys_product_type *dat)
{
	bkp_sram_read(RECODE_ANAYSIS_EGG_DAT_OFFEST_ADDR,(uint8_t *)dat,sizeof(recode_analisys_product_type));
	if(dat->flag != BACKUP_SRAM_FLAG)
	{
		rt_memset(dat,0,sizeof(recode_analisys_product_type));
		DEBUG_WARN("read anaysis is fall\n");
	}
	else
	{
		DEBUG_INFO("read anaysis is ok\n");
	}
}
