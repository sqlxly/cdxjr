#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <dfs_posix.h>
#include <rtconfig.h>
#include "system_part.h"
#include "app_screen.h"
#include "at24cxx.h"
#include "user_set.h"
#include "common.h"
#include "spi_flash_sfud.h"

#define UPDATE_DISP_ADDR (6000-1)
#define UPDATE_MAX_LINE  20
#define UPDATE_DISP_LEN  (200*2) 
#define EEP_MEM_DAT_LEN    (1024*10)  //10K
#define SPI_FLASH_DAT_LEN (4096*10)   //40k

static char disp_buf[UPDATE_DISP_LEN];
static uint8_t disp_step=0;
static uint8_t scan_file_count=0;
static char systen_fine_name[20][30];
static char select_file_neme[40];
static uint8_t copy_file_bar=0;
void disp_system_part_mangemeng()
{
  uint8_t i;
	//rt_base_t level;
	uint8_t buf[50];
	
	rt_memset(disp_buf,0,sizeof(disp_buf));
	
	if(disp_step==0)
	{
	  rt_memset(disp_buf,0,sizeof(disp_buf));
		rt_memcpy(&disp_buf[0],"请插入U盘",strlen("请插入U盘"));
	  CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==1)
	{
		rt_memcpy(&disp_buf[0],"文件扫描中....",strlen("文件扫描中...."));
	  CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
  else if(disp_step==2)
	{
		for(i=0;i<scan_file_count;i++)
		{
	    snprintf((char*)&disp_buf[i*20],30,"%d、%s",(i+1),&systen_fine_name[i][0]);
			rt_kprintf("disp file nema id=%d name=%s\n",i+1,&systen_fine_name[i][0]);
     // rt_memcpy(disp_buf+(i*20),(uint8_t *)&update_fine_name[i][0],20);
			//rt_memcpy(disp_buf+(i*20),(uint8_t *)&update_fine_name[i][0],20);
			CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		}
		if(scan_file_count==0)
		{
		   rt_memset(disp_buf,0,sizeof(disp_buf));
			 rt_memcpy(&disp_buf[0],"未发现系统参数文件",strlen("未发现系统参数文件"));
			 CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		}
	}
	else if(disp_step==3)
	{
		  rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"正在复制系统参数....");
			CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
			rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"进度=%d%%",copy_file_bar);
			CopyToMBHotReg(UPDATE_DISP_ADDR+10,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==4)
	{
		  rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"正在复制系统参数....");
			CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
			rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"进度=%d%%",100);
			CopyToMBHotReg(UPDATE_DISP_ADDR+10,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
		  rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"正在复制历史数据....");
			CopyToMBHotReg(UPDATE_DISP_ADDR+20,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
			rt_memset(disp_buf,0,sizeof(disp_buf));
			snprintf((char*)&disp_buf[0],30,"进度=%d%%",copy_file_bar);
			CopyToMBHotReg(UPDATE_DISP_ADDR+30,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==5)
	{
		rt_memset(disp_buf,0,sizeof(disp_buf));
    snprintf((char*)&disp_buf[0],30,"正在导入参数请稍等....");
		CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
		rt_memset(disp_buf,0,sizeof(disp_buf));
    snprintf((char*)&disp_buf[0],30,"%s",&select_file_neme[5]);
		CopyToMBHotReg(UPDATE_DISP_ADDR+10,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
		
		rt_memset(disp_buf,0,sizeof(disp_buf));
    snprintf((char*)&disp_buf[0],30,"进度=%d%%",copy_file_bar);
		CopyToMBHotReg(UPDATE_DISP_ADDR+20,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==6)
	{
		 rt_memset(disp_buf,0,sizeof(disp_buf));
		 rt_memcpy(&disp_buf[0],"文件打开错误",strlen("文件打开错误"));
		 CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==7)
	{
		 rt_memset(disp_buf,0,sizeof(disp_buf));
		 rt_memcpy(&disp_buf[0],"升级完成",strlen("升级完成"));
		 CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==8)
	{
		 rt_memset(disp_buf,0,sizeof(disp_buf));
		 rt_memcpy(&disp_buf[0],"系统参数拷贝完成",strlen("系统参数拷贝完成"));
		 CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}
	else if(disp_step==9)
	{
		 rt_memset(disp_buf,0,sizeof(disp_buf));
		 rt_memcpy(&disp_buf[0],"系统参数拷贝失败",strlen("系统参数拷贝失败"));
		 CopyToMBHotReg(UPDATE_DISP_ADDR,(uint8_t *)&disp_buf[0],sizeof(disp_buf));
	}

	
	//rt_hw_interrupt_enable(level);
}

uint8_t check_mount_usb_divice()
{
  DIR *dir;

	dir = opendir(UDISK_MOUNTPOINT);
	if(dir == RT_NULL)
	{	

		return 0;
	}
  closedir(dir);
	return 1;
}

static uint8_t envent=0;
void set_envent_id(uint8_t id)
{
  envent = id;
}


int check_udisk()
{
	if(check_mount_usb_divice()==0)
	{
		rt_kprintf("not found udisk\n");
		disp_step =0;
		disp_system_part_mangemeng();
		return 0;
	}
	return 1;
}
int scan_systemprt_file()
{
	DIR *dir; 
	struct dirent *d;
	int len = 0;
	int max_len = 26*2;
	int offset;
  const char* title = "%02d ";
	
  if(check_udisk()==0)
	{
		return 0;
	}
	scan_file_count =0;
	rt_memset(systen_fine_name,0,sizeof(systen_fine_name));
	dir = opendir(UDISK_MOUNTPOINT);
	if(dir == RT_NULL)
	{	
		return RT_ERROR;
	} else 
	{
		disp_step =1;
		disp_system_part_mangemeng();
		int title_len = strlen(title);
		
		
		while((d=readdir(dir)) != RT_NULL)
		{
			if(strstr(d->d_name, "-e.dat"))
			{
				len = strlen(d->d_name);
				if(scan_file_count<UPDATE_MAX_LINE)
				{
					rt_memcpy(&systen_fine_name[scan_file_count][0],d->d_name,len);
				  scan_file_count++;
				}
				rt_kprintf("scan file %d=%s\n",scan_file_count,d->d_name);
			}
		}
		closedir(dir);
	}

	disp_step =2;
	rt_thread_mdelay(200);
	disp_system_part_mangemeng();
	rt_kprintf("扫描完毕!!!!!!!!!!!!!!!! file count=%d\n",scan_file_count);
	return scan_file_count;	
}

#define read_buf_size 128
int copy_systempart_to_udisk()
{
	int result;
	int loop=0;
	int i;
  rtc_info_t time;
	char *read_buf;
	int read_addr;
	char file_neme[30];
	FILE *fp = NULL;    //
	int write_byte=0;
  if(check_udisk()==0)
	{
		return 0;
	}
	
	
  get_local_time(&time);
	snprintf(file_neme,30,"/usb/%d-%d-%d-e.dat",time.year+1900,time.month,time.date);
  fp = fopen(file_neme,"w+");   //????,??
  if(fp == NULL) 
	{
		disp_step =9;
		disp_system_part_mangemeng();
		return 0;
	}
	fseek(fp,0,SEEK_SET);
	
	read_buf = rt_malloc(read_buf_size+10);
	if(read_buf == RT_NULL)
	{
		disp_step =9;
		//rt_free(read_buf);
		disp_system_part_mangemeng();
		return 0;
	}
	
	disp_step =3;
	disp_system_part_mangemeng();
	read_addr =0;
	loop = EEP_MEM_DAT_LEN/read_buf_size;
	for(i=0;i<loop;i++)
	{
		rt_memset(read_buf,0,read_buf_size);
		result = at24cxx_multy_read(read_addr,(unsigned char *)read_buf,read_buf_size);
		read_addr +=read_buf_size;
		if(result !=RT_EOK)
		{
			disp_step =9;
			rt_free(read_buf);
			disp_system_part_mangemeng();
			return 0;
		}
		fwrite(read_buf,1,read_buf_size,fp);     //?a?????fp??????
   
		
		disp_step =3;
		copy_file_bar = (i*100)/loop;
		disp_system_part_mangemeng();
		rt_thread_mdelay(5);
	}
	
	
  
	read_addr =0;
	loop = SPI_FLASH_DAT_LEN/read_buf_size;   //40k
	for(i=0;i<loop;i++)
	{
		rt_memset(read_buf,0,read_buf_size);
		result = spi_flash_read(read_addr,(unsigned char *)read_buf,read_buf_size);
		read_addr +=read_buf_size;
		if(result !=RT_EOK)
		{
			disp_step =9;
			rt_free(read_buf);
			disp_system_part_mangemeng();
			return 0;
		}
		fwrite(read_buf,1,read_buf_size,fp);     //?a?????fp??????
		disp_step =4;
		copy_file_bar = (i*100)/loop;
		disp_system_part_mangemeng();
		rt_thread_mdelay(5);
	}
	read_addr =0;
	loop = 4096/read_buf_size;
	for(i=0;i<loop;i++)
	{
		rt_memset(read_buf,0,read_buf_size);
	  bkp_sram_read(read_addr,(unsigned char *)read_buf,read_buf_size);
		read_addr +=read_buf_size;
		write_byte = fwrite(read_buf,1,read_buf_size,fp);     //?a?????fp??????
		rt_thread_mdelay(5);
	}
	disp_step =8;
	disp_system_part_mangemeng();
	fflush(fp);
  fclose(fp); //????
	rt_free(read_buf);
}


uint8_t check_part_file()
{
	uint8_t result;
	uint16_t id;
  struct stat buf;
	
	id = ReadMBHotRegU16(1283);
	if(id<=0)
	{
	  disp_step =6;
		disp_system_part_mangemeng();
		return 1;
	}
	else 
	{
	  id = id -1;
	}
	rt_memset(select_file_neme,0,sizeof(select_file_neme));
	snprintf(select_file_neme,40,"/usb/%s",&systen_fine_name[id][0]);
	
	int fd = open(select_file_neme, O_RDONLY);
	if(fd > 0)
	{
	  result =0;
		close(fd);
	}
	else 
	{
		disp_step =6;
		disp_system_part_mangemeng();
	  result =1;
	}
  rt_kprintf("要升级的文件 id=%d reuslt=%d name=%s\n",id,result,select_file_neme);
	return result;
}
int copy_systempart_to_system()
{
	//FILE *fp = NULL;    //
	int i,loop;
	char *read_buf;
	uint32_t writeaddr;
	uint32_t read_addr;
	int read_len;
	int result=0;
  if(check_udisk()==0)
	{
		return 0;
	}
	if(check_part_file()!=0)
	{
		return 0;
	}
	
	
  int fd = open(select_file_neme, O_RDONLY);
//	fp = fopen(select_file_neme, "r+");
//	fseek(fp,0,SEEK_SET);
	read_buf = rt_malloc(read_buf_size+10);
	if(read_buf == RT_NULL)
	{
		disp_step =9;
		rt_free(read_buf);
		disp_system_part_mangemeng();
		return 0;
	}
	if(fd > 0)
	{		
    loop = EEP_MEM_DAT_LEN/read_buf_size;
		writeaddr =0;
		for(i=0;i<loop;i++)
		{
			rt_memset(read_buf,0,read_buf_size);
			read_len = read(fd, read_buf, read_buf_size);
			rt_kprintf("read udisk ok\r\n");
			//fread(read_buf,1,read_buf_size,fp);     //?a?????fp??????
			result = at24cxx_multy_write(writeaddr,(unsigned char *)read_buf,read_len);
			rt_kprintf("write 24c02 ok\r\n");
			writeaddr +=read_len;
			if((result !=RT_EOK)||(read_len !=read_buf_size))
			{
				disp_step =9;
				rt_free(read_buf);
				disp_system_part_mangemeng();
				return 0;
			}
			disp_step =5;
			copy_file_bar = (i*50)/loop;
			disp_system_part_mangemeng();
			rt_thread_mdelay(5);
		}
		for(i=0;i<(SPI_FLASH_DAT_LEN/4096);i++)
		{
			spi_flash_ease((i*4096) +1);
		}
		writeaddr =0;
		loop = SPI_FLASH_DAT_LEN/read_buf_size;   //40k
		for(i=0;i<loop;i++)
		{
			rt_memset(read_buf,0,read_buf_size);
			read_len = read(fd, read_buf, read_buf_size);
			rt_kprintf("read udisk ok\r\n");
			//fread(read_buf,1,read_buf_size,fp);     //?a?????fp??????
			result = spi_flash_write(writeaddr,(unsigned char *)read_buf,read_buf_size);
			rt_kprintf("write spi ok\r\n");
			writeaddr +=read_len;
			if((result !=RT_EOK)||(read_len !=read_buf_size))
			{
				disp_step =9;
				rt_free(read_buf);
				disp_system_part_mangemeng();
				return 0;
			}
			disp_step =5;
			copy_file_bar = ((i*50)/loop)+50;
			disp_system_part_mangemeng();
			rt_thread_mdelay(5);
		}
		
		writeaddr =0;
		loop = 4096/read_buf_size;   //40k
		for(i=0;i<loop;i++)
		{
			rt_memset(read_buf,0,read_buf_size);
			read_len = read(fd, read_buf, read_buf_size);
			//fread(read_buf,1,read_buf_size,fp);     //?a?????fp??????
			bkp_sram_write(writeaddr,(unsigned char *)read_buf,read_len);
			writeaddr +=read_buf_size;
			rt_thread_mdelay(5);
		}
		close(fd);
	}
	else 
	{
		disp_step =6;
		rt_free(read_buf);
		disp_system_part_mangemeng();
		rt_kprintf("system part copy fall\n");
		return 0;
	}
	    read_addr = PRODUCT_PACKGE_INFO_SPI_ADDR;
	    writeaddr = EEP_BACKGE_PACKGE_ADDR;
	    loop = 4096/read_buf_size;   //40k
		for(i=0;i<loop;i++)
		{
			rt_memset(read_buf,0,read_buf_size);
			result = spi_flash_read(read_addr,(unsigned char *)read_buf,read_buf_size);
			rt_kprintf("read spi ok\r\n");
			result = at24cxx_multy_write(writeaddr,(unsigned char *)read_buf,read_buf_size);
			rt_kprintf("wirte 24c02 ok\r\n");
			writeaddr +=read_buf_size;
			read_addr += read_buf_size;
			if(result !=RT_EOK)
			{
				disp_step =9;
				rt_free(read_buf);
				disp_system_part_mangemeng();
				return 0;
			}
			disp_step =5;
			copy_file_bar = (i*50)/loop;
			disp_system_part_mangemeng();
			rt_thread_mdelay(5);
		}
	
	disp_step=8;
	disp_system_part_mangemeng();
	rt_kprintf("part copy to system is sucess\n");
	rt_free(read_buf);
	return 0;
}
void process_system_part_fun()
{
	switch(envent)
	{
		case 0:
		{
		
		}
		break;
		case 1:
		{
			scan_systemprt_file();
		}
		break;
		case 2:
		{
			copy_systempart_to_udisk();
		}
		break;
		case 3:
		{
			copy_systempart_to_system();
		}
		break;
		default:
		{
			 
		}
		break;
	}
  envent =0;
}