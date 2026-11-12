#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "app_recode_logs.h"
#include "spi_flash_sfud.h"


static void bkp_sram_write(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;

    if(len > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE;
    HAL_PWR_EnableBkUpAccess();
    rt_memcpy((rt_uint8_t *)(BKPSRAM_BASE + offset),(rt_uint8_t *)data,len);
    HAL_PWR_DisableBkUpAccess();
}



static void bkp_sram_read(rt_uint16_t offset,rt_uint8_t *data,rt_uint16_t len)
{
    if(offset >= CFG_BKP_SRAM_SIZE)
        return;

    if(len > CFG_BKP_SRAM_SIZE)
        len = CFG_BKP_SRAM_SIZE;
    rt_memcpy((rt_uint8_t *)data, (rt_uint8_t *)(BKPSRAM_BASE + offset), len);
}


void hot_reg_clear_all_dat()
{
	  uint16_t i;
	  uint8_t data=0;
		HAL_PWR_EnableBkUpAccess();
			for(i=0;i<CFG_BKP_SRAM_SIZE;i++)
				rt_memcpy((rt_uint8_t *)(BKPSRAM_BASE + i),&data,1);
    HAL_PWR_DisableBkUpAccess();

}
void check_ceggs_recode()
{
	 uint32_t crc;
	 ceggs_recode ceggs_log;
   bkp_sram_read(RECODE_CEGGS_OFFSET,(rt_uint8_t *)&ceggs_log,sizeof(ceggs_recode));
//	 crc = ceggs_log.direction;
	 crc = ceggs_log.date;
	 crc += ceggs_log.hour;
	 crc += ceggs_log.min;
	 crc += ceggs_log.mode;
	 crc += ceggs_log.mon;
	 crc += ceggs_log.runtime;
	 crc += ceggs_log.year;
	
	 if(crc !=ceggs_log.crc)
	 {
	//	 DEBUG_INFO("集蛋热寄存器校验失败\n");
	   rt_memset((rt_uint8_t *) &ceggs_log,0,sizeof(ceggs_recode));
		 bkp_sram_write(RECODE_CEGGS_OFFSET,(rt_uint8_t *)&ceggs_log,sizeof(ceggs_recode));
	 }
	 else
	 {
		if(ceggs_log.runtime>0)
		{
		//	DEBUG_INFO("集蛋过程中掉电了,没有保存\n");
			save_ceggs_recode(&ceggs_log);
			rt_memset(&ceggs_log,0,sizeof(feed_recode));
			recode_ceggs_to_hog_reg(&ceggs_log);   //  清除掉电保存寄存器
		}
	//	else 
	//		DEBUG_INFO("集蛋检验为0\n");
	 }
}

void check_feed_recode()
{
	 uint32_t crc;
	 feed_recode feed_log;
   bkp_sram_read(RECODE_CEGGS_OFFSET,(rt_uint8_t *)&feed_log,sizeof(feed_recode));
	 crc = feed_log.direction;
	 crc = feed_log.date;
	 crc += feed_log.hour;
	 crc += feed_log.min;
	 crc += feed_log.mode;
	 crc += feed_log.mon;
	 crc += feed_log.runtime;
	 crc += feed_log.year;
	
	 if(crc !=feed_log.crc)
	 {
	//	 DEBUG_INFO("喂料热寄存器校验失败\n");
	   rt_memset((rt_uint8_t *) &feed_log,0,sizeof(feed_recode));
		 bkp_sram_write(RECODE_FEED_OFFSET,(rt_uint8_t *)&feed_log,sizeof(feed_recode));
	 }
	 else
	 {
		if(feed_log.runtime>0)
		{
			//DEBUG_INFO("喂料过程中掉电了,没有保存\n");
			save_feed_recode(&feed_log);
			rt_memset(&feed_log,0,sizeof(feed_recode));
			recode_feed_to_hog_reg(&feed_log);   //  清除掉电保存寄存器
		}
		//else 
	//		DEBUG_INFO("喂料检验为0\n");
	 }
}

void check_pack_recode()
{
	 uint16_t crc;
	 pack_recode pack_log;
   bkp_sram_read(RECODE_UP_PACK_OFFSET,(rt_uint8_t *)&pack_log,sizeof(pack_recode));
	 crc = pack_log.carnumber;
	 crc += pack_log.date;
	 crc += pack_log.hour;
	 crc += pack_log.min;
	 crc += pack_log.mode;
	 crc += pack_log.mon;
	 crc += pack_log.runtime;
	 crc += pack_log.year;
	 if(crc !=pack_log.crc)
	 {
		// DEBUG_INFO("上料上层热寄存器校验失败\n");
	   rt_memset((rt_uint8_t *) &pack_log,0,sizeof(pack_recode));
		 bkp_sram_write(RECODE_UP_PACK_OFFSET,(rt_uint8_t *)&pack_log,sizeof(pack_recode));
	 }
	 else
	 {
		if(pack_log.runtime>0)
		{
		//	DEBUG_INFO("上层上料过程中掉电了,没有保存\n");
			save_pack_recode(&pack_log);
			rt_memset(&pack_log,0,sizeof(pack_recode));
			pack_log.carnumber=1;
			recode_pack_to_hog_reg(&pack_log);   //  清除掉电保存寄存器
		}
		//else 
		//	DEBUG_INFO("上层上料检验为0\n");
	 }
	 
	 bkp_sram_read(RECODE_DOWN_PACK_OFFSET,(rt_uint8_t *)&pack_log,sizeof(pack_recode));
	 crc = pack_log.carnumber;
	 crc += pack_log.date;
	 crc += pack_log.hour;
	 crc += pack_log.min;
	 crc += pack_log.mode;
	 crc += pack_log.mon;
	 crc += pack_log.runtime;
	 crc += pack_log.year;
	 if(crc !=pack_log.crc)
	 {
		// DEBUG_INFO("下层热寄存器校验失败\n");
	   rt_memset((rt_uint8_t *) &pack_log,0,sizeof(pack_recode));
		 bkp_sram_write(RECODE_DOWN_PACK_OFFSET,(rt_uint8_t *)&pack_log,sizeof(pack_recode));
	 }
	 else
	 {
		if(pack_log.runtime>0)
		{
		//	DEBUG_INFO("下层上料过程中掉电了,没有保存\n");
			save_pack_recode(&pack_log);
			rt_memset(&pack_log,0,sizeof(pack_recode));
			
			recode_pack_to_hog_reg(&pack_log);   //  清除掉电保存寄存器
		}
		//else 
		//	DEBUG_INFO("下层上料检验为0\n");
	 }
}


void recode_ceggs_to_hog_reg(ceggs_recode *dat)
{
	uint32_t crc;
//	crc = dat->direction;
	crc = dat->date;
	crc += dat->hour;
	crc += dat->min;
	crc += dat->mode;
	crc += dat->mon;
	crc += dat->runtime;
	crc += dat->year;
  dat->crc =crc;
	bkp_sram_write(RECODE_CEGGS_OFFSET,(rt_uint8_t *)dat,sizeof(feed_recode));
	//DEBUG_INFO("保存集蛋运行时间=%d\n",dat->runtime);
}

void recode_feed_to_hog_reg(feed_recode *dat)
{
	uint16_t crc;
	crc = dat->direction;
	crc += dat->date;
	crc += dat->hour;
	crc += dat->min;
	crc += dat->mode;
	crc += dat->mon;
	crc += dat->runtime;
	crc += dat->year;
  dat->crc =crc;
	bkp_sram_write(RECODE_FEED_OFFSET,(rt_uint8_t *)dat,sizeof(feed_recode));
//	DEBUG_INFO("保存喂料运行时间=%d\n",dat->runtime);
}

void recode_pack_to_hog_reg(pack_recode *dat)
{
	uint16_t crc;
	crc = dat->carnumber;
	crc += dat->date;
	crc += dat->hour;
	crc += dat->min;
	crc += dat->mode;
	crc += dat->mon;
	crc += dat->runtime;
	crc += dat->year;
  dat->crc =crc;
	if(dat->carnumber==1)
		bkp_sram_write(RECODE_UP_PACK_OFFSET,(rt_uint8_t *)dat,sizeof(pack_recode));
	else 
		bkp_sram_write(RECODE_DOWN_PACK_OFFSET,(rt_uint8_t *)dat,sizeof(pack_recode));
//	DEBUG_INFO("保存上料运行时间=%d\n",dat->runtime);
}





uint8_t get_recode_tab_status(uint32_t addr,uint8_t *result)
{
	uint16_t sector;

	uint8_t *data = rt_malloc(FLASH_RECODE_SECTOR_HEAD_SIZE);
	sector = addr/4096;
	*result=0;
	if (data) 
	{
		rt_memset(data,0,FLASH_RECODE_SECTOR_HEAD_SIZE);
		flash_read(sector*4096+FLASH_RECODE_SECTOR_HEAD_ADDR,FLASH_RECODE_SECTOR_HEAD_SIZE,data);
		if((data[0]==0xaa)&&(data[1]==0xaa)&&(data[2]==0xaa))
			*result=*result |0X01;
		flash_read(sector*4096+FLASH_RECODE_SECTOR_END_ADDR,FLASH_RECODE_SECTOR_END_SIZE,data);
		if((data[0]==0xaa)&&(data[1]==0xaa)&&(data[2]==0xaa))
			*result= *result |0X02;
		rt_free(data);
	}
	else
	{
		DEBUG_INFO("动态内存分配失败\n");
		return 0;
	}
	return 1;
}

void write_recode_tab_head(uint32_t addr)
{
	uint16_t sector;
	sector = addr/4096;
	uint8_t *data = rt_malloc(FLASH_RECODE_SECTOR_HEAD_SIZE);
	if (data) 
	{
		rt_memset(data,0xaa,FLASH_RECODE_SECTOR_HEAD_SIZE);
		flash_write(sector*4096+FLASH_RECODE_SECTOR_HEAD_ADDR,FLASH_RECODE_SECTOR_HEAD_SIZE,data);
		rt_free(data);
	}
	else
	{
		DEBUG_INFO("动态内存分配失败\n");
	}
}

void write_recode_tab_end(uint32_t addr)
{
	uint16_t sector;
	sector = addr/4096;
	uint8_t *data = rt_malloc(FLASH_RECODE_SECTOR_END_SIZE);
	if (data) 
	{
		rt_memset(data,0xaa,FLASH_RECODE_SECTOR_END_SIZE);
		flash_write(sector*4096+FLASH_RECODE_SECTOR_END_SIZE,FLASH_RECODE_SECTOR_END_SIZE,data);
		rt_free(data);
	}
	else
	{
		DEBUG_INFO("动态内存分配失败\n");
	}
}
uint8_t find_tab_index(uint32_t addr,uint16_t *index)
{
	uint16_t sector=0;
	uint8_t i;
	uint8_t buf[5];
	sector = addr/4096;
	*index=0;
	for(i=0;i<FLASH_RECODE_SECTOR_SAVE_TOTLE_NUMB;i++)
	{
		rt_memset(buf,0,sizeof(buf));
		flash_read(((sector*4096+FLASH_RECODE_SECTOR_START_ADDR)+(i*FLASH_RECODE_DATA_SIZE)),2,buf);
		if(buf[0] !=FLASH_RECODE_DAT_FLAG)
		{
			*index=i+1;
			//DEBUG_INFO("获取到序号=%d\n",*index);
			return 1;
		}
	}
	return 0;
}

// 0 失败  1 成功 
uint8_t write_tab_index(uint32_t addr,uint16_t index,uint8_t *buf,uint16_t size)
{
	uint16_t sector=0;
	sector = addr/4096;
	if((index>0)&&(index<=200)) 
	{
		index =index-1;
		flash_write(((sector*4096+FLASH_RECODE_SECTOR_START_ADDR)+(index*FLASH_RECODE_DATA_SIZE)),size,buf);
		DEBUG_INFO("已存一条记录 sector=%d index=%d\n",sector,index+1);
		return 1;
	}
	return 0;
}

// 0 失败  1 成功   index =0 数据不可存储
uint8_t find_recode_tab_empty(uint32_t addr,uint16_t *index)
{

	uint8_t result=0,head_flag=0;
	uint16_t val1;
	result=get_recode_tab_status(addr,&head_flag);
	if(result)
	{
		if(head_flag==0)   //没有头标记 从1开始写
		{
			*index=1;
		}
		else if(head_flag==1)   //已存有数据  查找序号
		{
			if(find_tab_index(addr,&val1))
			{
				*index=val1;
			}
			else 
			{
				*index=0;  
				return 1;
			}
		}
		else
		{
			*index=0;    //0 为数据以存满
			return 1;
		}
		//DEBUG_INFO("获取扇区%d 表头记号=%d 存储序号=%d\n",sector,head_flag,*index);
		return 1;
	}
	return 0;
}
//函数名：save_pack_recode
//作者: yaozusheng
//日期: 2021.9.10
//功能：存储上料记录
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void save_pack_recode(pack_recode *pack_log)
{
	uint8_t i,result;
	uint16_t index=0;
	uint8_t buf[FLASH_RECODE_DATA_SIZE];
	if(sizeof(pack_recode)>=(FLASH_RECODE_DATA_SIZE-1))
	{
		DEBUG_INFO("错误,保存的日志数据长度大于定义长度 该数据被丢弃\n");
		return ;
	}
	rt_memset(buf,0,FLASH_RECODE_DATA_SIZE);
	buf[0]=FLASH_RECODE_DAT_FLAG;               //数据占位标记位
	rt_memcpy(&buf[1],(uint8_t *)pack_log,sizeof(pack_recode));
	for(i=0;i<FLASH_RECODE_PACK_MAX_SECTOR;i++)
	{	
		result = find_recode_tab_empty(FLASH_RECODE_PACK_ADDR+(i*4096), &index);
		if(result)
		{
			if(index>0)   //0表示已存满
			{
				//DEBUG_INFO("当前空余的扇区=%d 序号=%d\n",(FLASH_RECODE_PACK_ADDR+(i*4096))/4096,index);
				if(index==1)
				{
					if(i==(FLASH_RECODE_PACK_MAX_SECTOR-1))
					{
						flash_erase((FLASH_RECODE_PACK_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_PACK_ADDR+0*4096),10);
					}
					else
					{
						flash_erase((FLASH_RECODE_PACK_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_PACK_ADDR+((i+1)*4096)),10);
					}
				  write_recode_tab_head((FLASH_RECODE_PACK_ADDR+(i*4096)));
					write_tab_index((FLASH_RECODE_PACK_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				else 
				{
					write_tab_index((FLASH_RECODE_PACK_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				return ;
			}
		}
		else 
		{
			DEBUG_INFO("查找序号失败\n");
			return ;
		}
	}
	{
		flash_erase((FLASH_RECODE_PACK_ADDR+0*4096),10);
		DEBUG_INFO("上料记录数据已存满\n");
	}
}
//函数名：save_feed_recode
//作者: yaozusheng
//日期: 2021.9.10
//功能：存储喂料记录
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void save_feed_recode(feed_recode *feed_log)
{
	uint8_t i,result;
	uint16_t index=0;
	uint8_t buf[FLASH_RECODE_DATA_SIZE];
	if(sizeof(feed_recode)>=(FLASH_RECODE_DATA_SIZE-1))
	{
		DEBUG_INFO("错误,保存的日志数据长度大于定义长度 该数据被丢弃\n");
		return ;
	}
	rt_memset(buf,0,FLASH_RECODE_DATA_SIZE);
	buf[0]=FLASH_RECODE_DAT_FLAG;               //数据占位标记位
	rt_memcpy(&buf[1],(uint8_t *)feed_log,sizeof(feed_recode));
	for(i=0;i<FLASH_RECODE_FEED_MAX_SECTOR;i++)
	{	
		result = find_recode_tab_empty(FLASH_RECODE_FEED_ADDR+(i*4096), &index);
		if(result)
		{
			if(index>0)   //0表示已存满
			{
				//DEBUG_INFO("当前空余的扇区=%d 序号=%d\n",(FLASH_RECODE_PACK_ADDR+(i*4096))/4096,index);
				if(index==1)
				{
					if(i==(FLASH_RECODE_FEED_MAX_SECTOR-1))
					{
						flash_erase((FLASH_RECODE_FEED_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_FEED_ADDR+0*4096),10);
					}
					else
					{
						flash_erase((FLASH_RECODE_FEED_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_FEED_ADDR+((i+1)*4096)),10);
					}
				  write_recode_tab_head((FLASH_RECODE_FEED_ADDR+(i*4096)));
					write_tab_index((FLASH_RECODE_FEED_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				else 
				{
					write_tab_index((FLASH_RECODE_FEED_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				return ;
			}
		}
		else 
		{
			DEBUG_INFO("查找序号失败\n");
			return ;
		}
	}
	{
		flash_erase((FLASH_RECODE_FEED_ADDR+0*4096),10);
		DEBUG_INFO("喂料记录数据已存满\n");
	}
}

//函数名：save_ceggs_recode
//作者: yaozusheng
//日期: 2021.9.10
//功能：存储集蛋记录
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void save_ceggs_recode(ceggs_recode *ceggs_log)
{
	uint8_t i,result;
	uint16_t index=0;
	uint8_t buf[FLASH_RECODE_DATA_SIZE];
	if(sizeof(ceggs_recode)>(FLASH_RECODE_DATA_SIZE-1))
	{
		DEBUG_INFO("错误,保存的日志数据长度大于定义长度 该数据被丢弃长度=%d\n",sizeof(ceggs_recode));
		return ;
	}
	rt_memset(buf,0,FLASH_RECODE_DATA_SIZE);
	buf[0]=FLASH_RECODE_DAT_FLAG;               //数据占位标记位
	rt_memcpy(&buf[1],(uint8_t *)ceggs_log,sizeof(ceggs_recode));
	for(i=0;i<FLASH_RECODE_CEGGS_MAX_SECTOR;i++)
	{	
		result = find_recode_tab_empty(FLASH_RECODE_CEGGS_ADDR+(i*4096), &index);
		if(result)
		{
			if(index>0)   //0表示已存满
			{
				//DEBUG_INFO("当前空余的扇区=%d 序号=%d\n",(FLASH_RECODE_PACK_ADDR+(i*4096))/4096,index);
				if(index==1)
				{
					if(i==(FLASH_RECODE_CEGGS_MAX_SECTOR-1))
					{
						flash_erase((FLASH_RECODE_CEGGS_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_CEGGS_ADDR+0*4096),10);
					}
					else
					{
						flash_erase((FLASH_RECODE_CEGGS_ADDR+(i*4096)),10);
						flash_erase((FLASH_RECODE_CEGGS_ADDR+((i+1)*4096)),10);
					}
				  write_recode_tab_head((FLASH_RECODE_CEGGS_ADDR+(i*4096)));
					write_tab_index((FLASH_RECODE_CEGGS_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				else 
				{
					write_tab_index((FLASH_RECODE_CEGGS_ADDR+(i*4096)),index,buf,FLASH_RECODE_DATA_SIZE);
				}
				return ;
			}
		}
		else 
		{
			DEBUG_INFO("查找序号失败\n");
			return ;
		}
	}
	{
		flash_erase((FLASH_RECODE_CEGGS_ADDR+0*4096),10);
		DEBUG_INFO("喂料记录数据已存满\n");
	}
}



//1 返回数据有效
//0 返回数据无效

uint8_t get_history_recode_mesg(uint32_t addr, uint16_t index, uint8_t *dat)
{

  uint16_t sector = addr/4096;

	flash_read(((sector*4096+FLASH_RECODE_SECTOR_START_ADDR)+(index*FLASH_RECODE_DATA_SIZE)),FLASH_RECODE_DATA_SIZE,dat);
	
	//DEBUG_INFO("当前扇区=%d index=%d flag=%x\n",sector,index,dat[0]);
	if(dat[0] !=FLASH_RECODE_DAT_FLAG)
	{
		return 0;
	}
	else 
	{
		return 1;
	}
	
}


// 1 上料  2喂料   3集蛋
void get_recode_count( uint16_t  *count,uint8_t type)
{
	uint16_t i=0,j=0;
	uint8_t *data;
	uint32_t addr;
	*count =0;
	data = rt_malloc(FLASH_RECODE_DATA_SIZE);
	if (!data) 
	{
		DEBUG_INFO("动态内存分配失败\n");
		return;
	}
	 if(type==1)
		 addr=FLASH_RECODE_PACK_ADDR;
	 else if(type==2)
		 addr=FLASH_RECODE_FEED_ADDR;
	 else 
		 addr=FLASH_RECODE_CEGGS_ADDR;
		 
	for(i=0;i<FLASH_RECODE_PACK_MAX_SECTOR;i++)
	{
		for(j=0;j<200;j++)
		{
			rt_memset(data,0,FLASH_RECODE_DATA_SIZE);
			
			if(get_history_recode_mesg(addr+(i*4096),j,data)==1)
			{
				*count = *count+1;
			//	DEBUG_INFO("ture=%d\n",*count);
			}
		}
	}
  rt_free(data);
}
//函数名：get_recode_end_recode
//作者: yaozusheng
//日期: 2021.9.10
//功能：获取一页的数据
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void get_recode_end_recode(uint16_t *val,uint8_t type)
{
	uint8_t i,result;
	uint32_t addr; 
	if(type==1)
		 addr=FLASH_RECODE_PACK_ADDR;
	 else if(type==2)
		 addr=FLASH_RECODE_FEED_ADDR;
	 else 
		 addr=FLASH_RECODE_CEGGS_ADDR;
	 
	for(i=0;i<FLASH_RECODE_PACK_MAX_SECTOR;i++)
	{	
		result = find_recode_tab_empty(addr+(i*4096),val);
		if(result)
		{
			if(*val>0)
			{
				*val = (i*200+*val)-1;
				return ;
			}
		}
		else 
		{
			*val=0;
			return ;
		}
	}
}
//函数名：get_opage_recode_message
//作者: yaozusheng
//日期: 2021.9.10
//功能：获取一页的数据
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void get_opage_recode_message(uint8_t *buf, uint16_t page,uint8_t type)
{
	  uint16_t i,k;
		uint16_t totle_count=0,end_mesg=0,nub;
	  uint16_t tab[SCREEN_DISP_MESG_NUMBER];
	  uint32_t base_addr,addr;
	  
	  if(type==1)
		  base_addr=FLASH_RECODE_PACK_ADDR;
	  else if(type==2)
		  base_addr=FLASH_RECODE_FEED_ADDR;
	  else 
		  base_addr=FLASH_RECODE_CEGGS_ADDR;
	
		get_recode_count(&totle_count,type);
		get_recode_end_recode(&end_mesg,type);
	  
		if((page*SCREEN_DISP_MESG_NUMBER)<=end_mesg)
		{
		
		  for(i=0;i<SCREEN_DISP_MESG_NUMBER;i++)
			{
				tab[i]=end_mesg-(page*SCREEN_DISP_MESG_NUMBER)+i+1;
			}
		}
		else if((page*SCREEN_DISP_MESG_NUMBER)<(end_mesg+SCREEN_DISP_MESG_NUMBER))
		{
		  k =end_mesg%SCREEN_DISP_MESG_NUMBER;
			for(i=0;i<(end_mesg%SCREEN_DISP_MESG_NUMBER);i++)
			{
				tab[SCREEN_DISP_MESG_NUMBER-(i+1)]=k-i;
			}
			 k=0;
			for(i=(end_mesg%SCREEN_DISP_MESG_NUMBER);i<SCREEN_DISP_MESG_NUMBER;i++)
			{
				tab[SCREEN_DISP_MESG_NUMBER-(i+1)]=FLASH_RECODE_PACK_MAX_SECTOR*200-k;
				k++;
			}
		}
		else 
		{
		   nub = (page*SCREEN_DISP_MESG_NUMBER) -end_mesg;
		  
			for(i=0;i<SCREEN_DISP_MESG_NUMBER;i++)
			{
				tab[i]=FLASH_RECODE_PACK_MAX_SECTOR*200-nub + i+1;
			}
		}
		for(i=0;i<SCREEN_DISP_MESG_NUMBER;i++)
		{
		//	DEBUG_INFO("tab[%d]=%d\n",i,tab[i]);
			
			if(tab[i]>0)
			{
				addr = base_addr + (((tab[i]-1)/200)*4096);
			}
			else 
			{
				addr = base_addr;
			}
			if(tab[i]>0)
				tab[i] =tab[i]-1;
			get_history_recode_mesg(addr,tab[i]%200,&buf[i*FLASH_RECODE_DATA_SIZE]);
			
//			DEBUG_INFO("获取第%d条消息 扇区=%d index=%d flag=%x\n ",tab[i],addr/4096,tab[i]%200,buf[i*FLASH_RECODE_DATA_SIZE]);
			
		}
		
	
}



