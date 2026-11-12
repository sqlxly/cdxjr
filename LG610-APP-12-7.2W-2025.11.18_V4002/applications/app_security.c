

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "security_dat.h"
#include "app_security.h"
#include "crc32.h"
#include "at24cxx.h"
#include "modbus_screen.h"

#include "mdebug.h"

extern struct tm  *rtime;

#define ADDR_SECURITY_DAT  1169
#define EEP_SECURITY_FLAG  0XBA
uint8_t  check_security_result=0;   //用于屏幕输入校验显示


struct  security_dat
{
	uint8_t cmd;
	uint16_t year;
	uint8_t mon;
	uint8_t date;
};



static uint16_t ID_COUNT=0;


void refresh_operation_screen(uint16_t addr,uint16_t val)
{
	mb_mapping->tab_registers[addr] =val;
}


//函数名：eep_save_security
//作者: yaozusheng
//日期: 2021.9.10
//功能：授权码存储
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void eep_save_security(uint8_t *dat)
{
	uint32_t addr;
	uint16_t i,t_len;
	uint8_t r_buf;
	uint8_t wbuf[10];
	wbuf[0]=EEP_SECURITY_FLAG;
	rt_memcpy(&wbuf[1],dat,7);
	t_len=(ADDR_EEP_SECURITY_STOP - ADDR_EEP_SECURITY_START)/8;
	t_len = t_len-2;
	for(i=0;i<t_len;i++)
	{
		addr = ADDR_EEP_SECURITY_START+i*8;
		
		eeprom_read(addr,&r_buf,1); 
	//	DEBUG_CARE("eep_flag=%x\n",r_buf);
		if(r_buf != EEP_SECURITY_FLAG)
		{	
			eep_mutp_write(addr,wbuf,8); 
		    rt_memset(wbuf,0,8);
			eep_mutp_write(addr+8,wbuf,8); 
			
		//	DEBUG_CARE("保存授权码记录addr=%x\n",addr);
			return ;
		}
		if(i==(t_len-2))
		{
		//	DEBUG_INFO("存储已满保存授权码记录addr=%x\n",ADDR_EEP_SECURITY_START);
		  eep_mutp_write(ADDR_EEP_SECURITY_START,wbuf,8); 
		  rt_memset(wbuf,0,8);
		  eep_mutp_write(ADDR_EEP_SECURITY_START+8,wbuf,8); 
		  return ;
		}
	}
}
// 返回1 表示有记录

uint8_t cmp_buf(uint8_t *buf1,uint8_t *buf2,uint8_t len)
{
	uint8_t i,count=0;
	for(i=0;i<len;i++)
	{
		if(buf1[i]==buf2[i])
			count++;
	}
	if(count==len)
		return 1;
	else 
		return 0;
}

// 返回1表示有记录
uint8_t eep_cmp_security_hostory_mesg(uint8_t *dat)
{
	uint32_t addr;
	uint16_t i,t_len;
	uint8_t rbuf[10];
	t_len=(ADDR_EEP_SECURITY_STOP - ADDR_EEP_SECURITY_START)/8;
	for(i=0;i<t_len;i++)
	{
	  addr = ADDR_EEP_SECURITY_START+i*8;
	  eep_mutp_read(addr,rbuf,8); 
		
		if(cmp_buf(dat,&rbuf[1],7)>0)
		{
			return 1;
		}
	}
	return 0;
}
/*
static uint16_t t_buf[1024*2];
void test_sc()
{
	uint16_t i;
	for(i=0;i<1024;i++)
	t_buf[i]=i;
	eep_save_security((uint8_t *)t_buf);
	
	for(i=0;i<1024;i++)
	t_buf[i]=0;
	
	eep_mutp_read(ADDR_EEP_SECURITY_START,(uint8_t *)t_buf,1024); 
	for(i=0;i<600;i++)
	{
	  rt_kprintf("buf[%d]=%d\n",i,t_buf[i]);
	}
}
*/

uint8_t encode(uint8_t dat,uint8_t *encode,uint8_t index)
{
	uint8_t i;
	if((dat>15)&&(index>0xff))
		return 0;
	for(i=0;i<16;i++)
	{
		if(securry[index][i]==dat)
		{
		//	printf("dat=%d,encode=%d",dat,i);
			*encode= i;
			return 1;
		}
	}
	return 0;
}

uint8_t decode(uint8_t dat,uint8_t *dval,uint8_t index)
{

	if(dat>15)
		return 0;
	*dval = securry[index][dat];
	return 1;
}

//函数名：decode_uint8
//作者: yaozusheng
//日期: 2021.9.10
//功能：授权码解码
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
uint8_t decode_uint8(uint8_t dat,uint8_t *dval,uint8_t index)
{

	uint8_t buf1,val;

	buf1 = dat&0x0f;
	val = securry[index][buf1];
	*dval = val;
	*dval = *dval<<4;

	buf1 = dat>>4;
	val = securry[index][buf1];
	*dval = *dval + val;
	return 1;
}

//函数名：encode_uint8
//作者: yaozusheng
//日期: 2021.9.10
//功能：授权码编码 
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:

uint8_t encode_uint8(uint8_t dat,uint8_t *endat,uint8_t index)
{
	uint8_t buf1,val;

	buf1 = dat&0x0f;

	if(encode(buf1,&val,index)==0)
	{
		return 0;
	}
	*endat = val;
	*endat = *endat<<4;
	dat = dat>>4;
	buf1 = dat&0x0f;
	if(encode(buf1,&val,index)==0)
	{
		return 0;
	}
	*endat = *endat+val;
	return 1;
}

//函数名：fun_en_paixu
//作者: yaozusheng
//日期: 2021.9.10
//功能：授权码编码 排序
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void fun_en_paixu(uint8_t *odat,uint8_t *ndat,uint8_t index)
{
	uint8_t serial,i,buf;

	for(i=0;i<securry_number/2;i++)
	{
		
			serial = paixu[index][i*2];
			buf = (odat[i]) &0xf0;              //??
		//	printf("serial=%d\n",serial);
			if(serial%2==0)
			{
				ndat[serial/2] = ndat[serial/2]&0x0f;	
				ndat[serial/2] = ndat[serial/2] | buf;

			//	printf("i=%d hi ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}
			else 
			{
				ndat[serial/2] = ndat[serial/2]&0xf0;	  //LOW 
				buf=buf>>4;
				ndat[serial/2] = ndat[serial/2] | buf;

			//	printf("i=%d low ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}


			serial = paixu[index][i*2+1];
		//	printf("serial=%d\n",serial);
			buf = odat[i]&0x0f;              //LOW
			if(serial%2==0)
			{
				buf=buf<<4;
				ndat[serial/2] = ndat[serial/2]&0x0f;	
				ndat[serial/2] = ndat[serial/2] | buf;

			//	printf("i=%d hi ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}
			else 
			{
				ndat[serial/2] = ndat[serial/2]&0xf0;	  //LOW 
				ndat[serial/2] = ndat[serial/2] | buf;


			}
	}
}

//函数名：fun_de_paixu
//作者: yaozusheng
//日期: 2021.9.10
//功能：授权码解码 排序
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void fun_de_paixu(uint8_t *odat,uint8_t *ndat,uint8_t index)
{
	uint8_t serial,i,buf;
	for(i=0;i<securry_number/2;i++)
	{
		
			serial = paixu[index][i*2];
			buf = (odat[serial/2]) ;
			if(serial%2==0)
			{
				buf = buf&0xf0;
				ndat[i] = ndat[i]&0x0f;	
				ndat[i] = ndat[i] | buf;

			//	printf("i=%d hi ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}
			else 
			{
				buf = buf&0x0f;
				buf = buf<<4;
				ndat[i] = ndat[i]&0x0f;	
				ndat[i] = ndat[i] | buf;

			//	printf("i=%d low ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}

			serial = paixu[index][i*2+1];
			buf = odat[serial/2];             //LOW
			if(serial%2==0)
			{
				buf=buf&0xf0;
				buf=buf>>4;
				ndat[i] = ndat[i]&0xf0;	
				ndat[i] = ndat[i] | buf;

			//	printf("i=%d hi ndat[%d]=%d\n",i,serial/2,ndat[serial/2]);
			}
			else 
			{
				buf=buf&0x0f;
				ndat[i] = ndat[i]&0xf0;	
				ndat[i] = ndat[i] | buf;
			}
	}
}

uint16_t crc_count(uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint16_t count=0;
	for(i=0;i<len;i++)
	{
	 count +=buf[i];
	}
	return count;
}

//函数名：get_stm32_id
//作者: yaozusheng
//日期: 2021.9.10
//功能： 获取单片机ID号
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:

extern void HAL_GetUID(uint32_t *UID);
void get_stm32_id(uint32_t *uid)
{
	HAL_GetUID(uid);
	//DEBUG_INFO("\nUID0=%08x,\nUID1=%08x,\nUID2=%08x\n",uid[0],uid[1],uid[2]);
}

//函数名：fun_jiami
//作者: yaozusheng
//日期: 2021.9.10
//功能： 密钥加密
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void fun_jiami(uint8_t *dat ,uint8_t *endat,uint8_t len)
{
	uint8_t i;

	for(i=0;i<len;i++)
	{
		if(encode_uint8(dat[i],&endat[i],ID_COUNT)==0)
		{
			DEBUG_INFO("encode falll\n");
		}
	}
	fun_en_paixu(endat,dat,ID_COUNT);
}

//函数名：fun_jiema
//作者: yaozusheng
//日期: 2021.9.10
//功能： 密钥解码
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void fun_jiema(uint8_t *dat ,uint8_t *dedat,uint8_t len)
{
	uint8_t i;
	
	fun_de_paixu(dat,dedat,ID_COUNT);
	for(i=0;i<len;i++)
	{
		if(decode_uint8(dedat[i],&dat[i],ID_COUNT)==0)
		{
			DEBUG_INFO("decode falll\n");
		}
	//	DEBUG_INFO("%x ",dat[i]);
	}
}




//函数名：load_security_dat
//作者: yaozusheng
//日期: 2021.9.10
//功能： 获取密钥数据
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void load_security_dat(uint8_t *dat)
{
	eeprom_read(ADDR_EEP_SECURITY,dat,10);
}
void save_security_dat(uint8_t *dat)
{
	eeprom_write(ADDR_EEP_SECURITY,dat,10);
}



int isLeapYear(int year)
{
	return ((year%4==0 && year%100!=0) || year%400==0);
}

int getDays(int year, int month, int day)
{
	int m[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
	int result = 0;
	int i;

	if(isLeapYear(year))
		m[2]++;
	
	for( i = 1;i < year;i++)
	{
		result += 365;
		if(isLeapYear(i))
			result ++;
	}
	for( i = 1;i < month;i++)
	{
		result += m[i];
	}
	result += day;
	
	return result;
}
int dayDis (int year1, int month1, int day1,
			int year2, int month2, int day2)
{
	if(year1<2021)
		return -3;
	else 
		return (getDays(year2, month2, day2) - getDays(year1, month1, day1));
}
	
// 0 无分期   1有分期
void disp_pay_money_date(int16_t less_dat,uint8_t type)
{
	if(type==1)
	{
		if(less_dat>20)
		{
			refresh_operation_screen(549,30);
		}
		else if((less_dat<21)&&(less_dat>0))
		{
			refresh_operation_screen(549,less_dat+1);
		}
		else 
		{
			refresh_operation_screen(549,0);
		}
	
		if(less_dat>0)
		{
			refresh_operation_screen(536,less_dat);
		}
		else 
		{
			refresh_operation_screen(536,0);
		}
	}
	else 
	{
		refresh_operation_screen(549,30);
		refresh_operation_screen(536,10005);
	}
}

//函数名：cmp_security_date
//作者: yaozusheng
//日期: 2021.9.10
//功能： 计算授权码到期时间
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
int16_t cmp_security_date(struct security_dat *sdat)
{
	int less;
	//DEBUG_INFO("当前日期=%d-%d-%d,  授权码到期日期=%d-%d-%d\n",rtime->tm_year+1900,rtime->tm_mon+1,rtime->tm_mday,sdat->year,sdat->mon,sdat->date);
	
	less = dayDis(rtime->tm_year+1900,rtime->tm_mon+1,rtime->tm_mday,sdat->year,sdat->mon,sdat->date);
	DEBUG_INFO("授权码到期剩余时间=%d\n",less);
	disp_pay_money_date(less,1);
	return less;
}


//函数名：check_screen_security_dat
//作者: yaozusheng
//日期: 2021.9.10
//功能： 校验授权码是否有效
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void check_screen_security_dat(uint8_t *dat)
{

	uint8_t i,k,buf;
    uint32_t id[4];
	uint32_t crc32_val;
	uint8_t dedat[10];
	uint8_t crc_result[6];
	uint8_t temp[10];
	struct security_dat security;
	
	rt_memcpy(temp,dat,10);

    get_stm32_id(id);
	ID_COUNT = crc_count((uint8_t *)id,12);
	ID_COUNT = ID_COUNT&0x00ff;
	fun_jiema(dat,dedat,7);
	CALCRC32_set(0xFFFFFFFF);
	CALCRC32_file((uint8_t *)id,12);
	CALCRC32_file(&dat[4],3);
	crc32_val = CALCRC32_get();
	rt_memcpy(crc_result,(uint8_t *)&crc32_val,4);
	k=0;
	for(i=0;i<4;i++)
	{
		if(crc_result[i]==dat[i])
			k++;
	}

	if(k==4)
	{
		security.cmd=(dat[4]&0xf0)>>4;
		buf = (dat[4]&0x0f)<<4;
		buf = buf + ((dat[5]&0xf0)>>4);
		security.year = buf+2000; 
		security.mon = dat[5]&0x0f;
		security.date = dat[6];
		DEBUG_INFO("授权码校验通过\ncmd=%d year=%x,mon=%x,date=%x\n",security.cmd,security.year,security.mon,security.date);
		check_security_result=1;
		
		if((security.cmd==1)||(security.cmd==3))
		{
			save_security_dat(temp);
		}
		else if(security.cmd==2)
		{
			eep_save_security(temp);
			set_date( security.year,security.mon, security.date);
		}
		else if(security.cmd==4)
		{
			eep_save_security(temp);
			set_time_hour(security.date);
		}
	}
	else 
	{
		check_security_result=2;
	  DEBUG_WARN("授权码数据校验失败\n");
	}
	hid_security_dat();
}

// 检验本地授权码是否到期
// 0 不是分期  1 分期未到期 2 分期已到期  3 识别密钥失败
uint8_t check_local_security_dat()
{
	uint8_t i,k,buf,result=3;
  uint32_t id[4];
	uint32_t crc32_val;
  uint8_t security_dat[10];
	uint8_t jm_dat[10];
	uint8_t crc_result[6];
	struct security_dat security;
	
  get_stm32_id(id);
	ID_COUNT = crc_count((uint8_t *)id,12);
	ID_COUNT = ID_COUNT&0x00ff;
	rt_thread_mdelay(20);
	load_security_dat(security_dat);
	//fun_jiami(security_dat,jm_dat,7);
	//DEBUG_INFO("\r\n加密后的数据=\n");
	//for(i=0;i<7;i++)
	//{
	//	DEBUG_INFO("%02x ",security_dat[i]);
	//}
	fun_jiema(security_dat,jm_dat,7);
	//DEBUG_INFO("\r\n解密\r\n");
	//for(i=0;i<7;i++)
	//{
	//	DEBUG_INFO("%02x ",security_dat[i]);
	//}
	CALCRC32_set(0xFFFFFFFF);
	CALCRC32_file((uint8_t *)id,12);
	CALCRC32_file(&security_dat[4],3);
	crc32_val = CALCRC32_get();
	rt_memcpy(crc_result,(uint8_t *)&crc32_val,4);
	k=0;
	for(i=0;i<4;i++)
	{
		if(crc_result[i]==security_dat[i])
			k++;
	}
	if(k==4)
	{
		security.cmd=(security_dat[4]&0xf0)>>4;
		buf = (security_dat[4]&0x0f)<<4;
		buf = buf + ((security_dat[5]&0xf0)>>4);
		security.year = buf+2000; 
		security.mon = security_dat[5]&0x0f;
		security.date = security_dat[6];
		DEBUG_INFO("授权码校验通过 cmd=%d 日期=%d-%d-%d\n",security.cmd,security.year,security.mon,security.date);
		if(security.cmd==1)
		{
			if(cmp_security_date(&security)>=0)
			{
				result=1;
			}
			else 
			{
				result=2;
			}
		}
		else if(security.cmd==3)
		{
			disp_pay_money_date(0,0);
			result=0;
		}
		else
		{
			disp_pay_money_date(-1,1);
			result=3;
		}
	}
	else 
	{
		result=3;
		disp_pay_money_date(-1,1);
	  DEBUG_WARN("授权码数据校验失败\n");
	}
	return result;
	//hid_security_dat();
}

//函数名：rting_to_hex
//作者: yaozusheng
//日期: 2021.9.10
//功能： 字符串转十六进制
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
static uint8_t rting_to_hex(uint8_t str)
{
	if(str =='0')
		return 0;
	else if(str =='1')
		return 1;
	else if(str =='2')
		return 2;
	else if(str =='3')
		return 3;
	else if(str =='4')
		return 4;
	else if(str =='5')
		return 5;
	else if(str =='6')
		return 6;
	else if(str =='7')
		return 7;
	else if(str =='8')
		return 8;
	else if(str =='9')
		return 9;
	else if((str =='A')||(str =='a'))
		return 10;
	else if((str =='B')||(str =='b'))
		return 11;
	else if((str =='C')||(str =='c'))
		return 12;
	else if((str =='D')||(str =='d'))
		return 13;
	else if((str =='E')||(str =='e'))
		return 14;
	else if((str =='F')||(str =='f'))
		return 15;
	else 
		return 0;
}



void disp_help_page()
{
	uint32_t uid[4];
	uint8_t buf[30];
	uint8_t i;
	uint16_t addr;
	HAL_GetUID(uid);
	rt_memset(buf,0,30);
	rt_sprintf((char *)buf,"%08x%08x%08x",uid[0],uid[1],uid[2]);
	//DEBUG_INFO("序列号=%s\n",buf);
	addr = 539;
	//refresh_operation_screen(510,SOFT_VERSION);
	for(i=0;i<12;i++)
	{
		refresh_operation_screen(addr+i,buf[i*2]| buf[i*2+1]<<8);
	}
}
//函数名：get_secreen_security_dat
//作者: yaozusheng
//日期: 2021.9.10
//功能： 从屏幕获取授权码
//全局变量： 
//输入参数：NULL
//返回值： 无
//修改记录:
void get_secreen_security_dat()
{
	uint8_t i,result;
  uint8_t dat[15];
	rt_memset(dat,0,15);
	for(i=0;i<7;i++)
	{
		dat[i*2]= usSRegHoldBuf[ADDR_SECURITY_DAT+i]&0x00ff;
		dat[i*2+1]= usSRegHoldBuf[ADDR_SECURITY_DAT+i]>>8;
		dat[i*2]=rting_to_hex(dat[i*2]);
		dat[i*2+1]=rting_to_hex(dat[i*2+1]);
		dat[i*2]=dat[i*2]<<4;
		dat[i]=dat[i*2];
		dat[i] = dat[i] + dat[i*2+1];
	}
	//DEBUG_INFO("\r授权码=\n");
	//for(i=0;i<7;i++)
	//{
	//	DEBUG_INFO("%02x",dat[i]);
	//}
	//DEBUG_INFO("\n");
	
	result = eep_cmp_security_hostory_mesg(dat);
	if(result)
	{
		check_security_result=3;
		DEBUG_WARN("此命令已失效\n");
		return ;
	}
	check_screen_security_dat(dat);
}

