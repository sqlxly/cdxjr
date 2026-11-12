#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <user_set.h>
#include "modbus_screen.h"
#include "app_screen.h"
#include "at24cxx.h"
#include "crc32.h"
#include "mdebug.h"
#include "product_set.h"
#include "device_alarm.h"

ALIGN(RT_ALIGN_SIZE)

static device_offest_position devide_positon;
static user_set_part_type user_set_part;
static run_dat_type run_dat;
static uint8_t read_err_flag=0;
static mix_device_part mix_device;

// 获取某个包装通道是否安装
uint8_t get_packge_ch_config(uint8_t id)
{
  uint16_t *addr;
	uint8_t val;
	
	if((id%2)==0)
	{
		addr = &user_set_part.device_use_info.chA.packge_ch1;
		val = addr[id/2]&&1;
	}
	else 
	{
		addr = &user_set_part.device_use_info.chB.packge_ch1;
		val = addr[id/2]&&1;
	}

	return val;
}
// 获取用户参数，包含设备是否安装，设备是否使能，各种等级以及其他等
void get_user_set_dat(user_set_part_type *buf)
{
  rt_memcpy(buf,&user_set_part,sizeof(user_set_part_type));
}
// 获取用户参数地址
// 这个可以在获取后修改，然后user_set_part也修改了
void get_user_set_point(user_set_part_type **point)
{
  *point = &user_set_part;
}
// 获取用户参数，包含设备是否安装，设备是否使能，各种等级以及其他等
void get_user_set_part(user_set_part_type *dat)
{
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	rt_memcpy(dat,&user_set_part,sizeof(user_set_part_type));
	rt_hw_interrupt_enable(level);
}

// 获取混杂设备的参数
rt_err_t read_mix_device_part(mix_device_part *buf)
{
	rt_err_t result;
	uint16_t len;
	uint32_t crc32_dat;
	
	len = sizeof(mix_device_part);
	
	result= at24cxx_multy_read(MIX_DEVICE_PART_EEP_ADDR,(uint8_t*)buf,len);  
	crc32_dat = CRC32_file((uint8_t *)buf,len-4);  // 4为CRC32 
	if(crc32_dat !=buf->crc_32)
	{
		DEBUG_ERR("混杂设备参数读取失败 crc=%d read crc=%d\n",crc32_dat,buf->crc_32);
		set_device_alarm(SISTME_PART_READ_ERROR);
		rt_memset(buf,0,len);
		return RT_ERROR;
	}
	rt_kprintf("check crc32=%d recd crc=%d\n",crc32_dat,buf->crc_32);
	return RT_EOK;
}
// 保存混杂设备参数，先存数据在存校验，为什么不一次性存了呀
void save_mix_device_part(mix_device_part *dat)
{
	rt_err_t result;

	uint16_t i;
	uint16_t len;
	uint32_t crc32_dat;
	
	len = sizeof(mix_device_part);

	crc32_dat = CRC32_file((uint8_t *)dat,len-4);  // 4为CRC32 

	result= at24cxx_multy_write(MIX_DEVICE_PART_EEP_ADDR,(uint8_t*)&dat[0],len-4);  
	
  result= at24cxx_multy_write(MIX_DEVICE_PART_EEP_ADDR+(len-4),(uint8_t *)&crc32_dat,4);    //写CRC32
	rt_thread_mdelay(5);
	read_mix_device_part(&mix_device);
}
// 获取混杂设备的参数
void get_mix_device_part(mix_device_part *dat)
{
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	rt_memcpy(dat,&mix_device,sizeof(mix_device_part));
	rt_hw_interrupt_enable(level);
}
// 获取通道点位偏移参数
void get_senser_offest(device_offest_position *dat)
{
	rt_base_t level;
	
	level = rt_hw_interrupt_disable();
	rt_memcpy(dat,&devide_positon,sizeof(device_offest_position));
	rt_hw_interrupt_enable(level);
}
// 从eeprom中读取通道点位偏移参数
rt_err_t read_device_offest_from_eep(device_offest_position *buf)
{
	rt_err_t result;
	uint16_t len,i;
	uint16_t *addr;
	uint32_t crc32_dat;
	
	len = sizeof(device_offest_position);
	
	result= at24cxx_multy_read(DEVICE_POSITION_OFFEST_EEP_ADDR,(uint8_t*)buf,len);  
	crc32_dat = CRC32_file((uint8_t *)buf,len-4);  // 4为CRC32 
	if(crc32_dat !=buf->crc_32)
	{
		set_device_alarm(SISTME_PART_READ_ERROR);
		//DEBUG_ERR("位置信息参数读取失败 crc=%d read crc=%d\n",crc32_dat,buf->crc_32);
		rt_memset(buf,0,len);
		return RT_ERROR;
	}
	rt_kprintf("crc32=%d\n",crc32_dat);
	return RT_EOK;
}
// 保存通道点位偏移参数到eeprom中
void save_device_offest_to_eep(device_offest_position *dat )
{
	rt_err_t result;

	uint16_t i;
	uint16_t len;
	uint32_t crc32_dat;
	
	
	len = sizeof(device_offest_position);

	crc32_dat = CRC32_file((uint8_t *)dat,len-4);  // 4为CRC32 

	result= at24cxx_multy_write(DEVICE_POSITION_OFFEST_EEP_ADDR,(uint8_t*)&dat[0],len-4);  
	
  result= at24cxx_multy_write(DEVICE_POSITION_OFFEST_EEP_ADDR+(len-4),(uint8_t *)&crc32_dat,4);    //写CRC32
	rt_thread_mdelay(5);
	read_device_offest_from_eep(&devide_positon);
}

// 从eeprom中获取用户参数，包含设备是否安装，设备是否使能，各种等级以及其他等
rt_err_t read_user_set_from_eep(user_set_part_type *buf)
{
	rt_err_t result;
	uint16_t len;
	uint32_t crc32_dat;
	
	read_err_flag =0;
	len = sizeof(user_set_part_type);
	result= at24cxx_multy_read(USER_SET_EEP_ADDR,(uint8_t*)buf,len);  
	crc32_dat = CRC32_file((uint8_t *)buf,len-4);  // 4为CRC32 
	if(crc32_dat !=buf->crc_32)
	{
		read_err_flag=1;
		set_device_alarm(SISTME_PART_READ_ERROR);
		//DEBUG_ERR("参数校验失败 crc=%d read crc=%d\n",crc32_dat,buf->crc_32);
		rt_memset(buf,0,len);
		return RT_ERROR;
	}
	rt_kprintf("crc32=%d\n",crc32_dat);
	return RT_EOK;
}
// 保存用户参数，包含设备是否安装，设备是否使能，各种等级以及其他等到eeprom中
void save_user_set_to_eep(user_set_part_type *dat )
{
	rt_err_t result;
	uint8_t *old_dat;
	uint8_t *new_dat;
	uint16_t i;
	uint16_t len;
	uint32_t crc32_dat;
	
	len = sizeof(user_set_part_type);
	old_dat = (uint8_t *)&user_set_part;
	new_dat = (uint8_t *)dat;
	crc32_dat = CRC32_file((uint8_t *)new_dat,len-4);  // 4为CRC32 
	for(i=0;i<(len-4);i++)
	{
		if((old_dat[i]!=new_dat[i])||(read_err_flag>0))
		{
			result= at24cxx_multy_write(USER_SET_EEP_ADDR+i,&new_dat[i],1);  
			//rt_thread_mdelay(5);
		}
	}
  result= at24cxx_multy_write(USER_SET_EEP_ADDR+i,(uint8_t *)&crc32_dat,4);    //写CRC32
	//rt_memcpy(old_dat,new_dat,sizeof(user_set_part_type));
	DEBUG_INFO("SAVE LEN=%d crc=%d\n",sizeof(user_set_part_type),crc32_dat);
	rt_thread_mdelay(5);
	read_user_set_from_eep(&user_set_part);
}
// 获取时间
void get_local_time(rtc_info_t *ptime)
{
	 time_t rawtime;
   struct tm *info;
	 struct tm rtime;
   time( &rawtime );
   info = localtime( &rawtime );
   rt_memset(&rtime,0,sizeof(struct tm));
	 rt_memcpy(&rtime,info,sizeof(struct tm));
	 //rtime.tm_year +=1900;
	 rtime.tm_mon  +=1;
	 ptime->year = rtime.tm_year;
   ptime->month = rtime.tm_mon;
   ptime->date = rtime.tm_mday;
   ptime->hour = rtime.tm_hour;
   ptime->min = rtime.tm_min;
   ptime->sec = rtime.tm_sec;
} 
// 获取运行参数(变频器设置的速度和编码器的速度)
// 这个函数没有使用，相当于这个运行参数没有使用
void get_run_dat(run_dat_type *dat)
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_memcpy(dat,&run_dat,sizeof(run_dat_type));
	rt_hw_interrupt_enable(level);
}


// 从eeprom中读取每个蛋杯统计到的错误计数
// 1个蛋杯2个字节存储，长度不超过800即表示最大读取400个蛋杯的数据
// 程序里面又定义#define MAX_CUP_COUNT 360，最大360个蛋杯
// uint16_t SEPARATE_CHA_CUP_NUMBER = 270;
// uint16_t SEPARATE_CHB_CUP_NUMBER = 270; 有些地方又用了这个变量
void read_egg_cup_err_count(uint8_t ch,uint16_t *dat,uint16_t len)
{
  uint8_t result;
	uint16_t is_init;
	uint32_t addr=0;
	if(ch==0)
		addr = EGG_CHA_CUP_ERR_COUNT_EEP_ADDR;
	else 
		addr = EGG_CHB_CUP_ERR_COUNT_EEP_ADDR;
	
	at24cxx_multy_read(addr-2,(uint8_t*)&is_init,2); 
  if(len>800) return;
  if(is_init == 0x1122)	
	{
		result= at24cxx_multy_read(addr,(uint8_t*)dat,len);  
	}
	else 
	{
		 is_init = 0x1122; 
		 rt_memset(dat,0,len);
		 result= at24cxx_multy_write(addr-2,(uint8_t *)&is_init,2);    //写CRC32
		 result= at24cxx_multy_write(addr,(uint8_t*)dat,len);  
	}
}
// 保存蛋杯错误统计
// 从0号蛋杯开始统计，每个蛋杯都要统计并存储？最大的蛋杯数不能超过500个蛋杯，每一个蛋杯用2个字节存储
void save_egg_cup_err_count(uint8_t ch,uint16_t dat,uint16_t index)  //从0 开始 蛋杯号数
{
  uint8_t result;
  uint16_t val;
  uint16_t id;
	uint32_t addr=0;
	
  if(index>500) return;
  id = index*2;
  val = dat;
	
	if(ch==0)
		addr = EGG_CHA_CUP_ERR_COUNT_EEP_ADDR;
	else 
		addr = EGG_CHB_CUP_ERR_COUNT_EEP_ADDR;
	result= at24cxx_multy_write(addr+id,(uint8_t *)&val,2);    //写CRC32
}

void user_dat_init()
{
    rt_kprintf("user set len=%d\n",sizeof(user_set_part_type));
	rt_memset(&mix_device,0,sizeof(mix_device_part));
	rt_memset(&user_set_part,0,sizeof(user_set_part_type));
	read_mix_device_part(&mix_device);	// 踢蛋电机错误，蛋杯错误信息等
	read_user_set_from_eep(&user_set_part);	// 传输通道的io使能情况，各种等级设置，各个模块使能配置
	read_device_offest_from_eep(&devide_positon);//通道的位置脉冲值信息
	power_up_read_cup_err_count_from_epp();
	load_supplier_dat();					// 加载供应商信息
	load_product_packge_dat();		// 加载包装信息
	load_product_attribute_dat();	// 加载产品信息
	init_per_production_dat();		// 加载预生产信息
	if(devide_positon.chA.totle_cup<MIN_CUP_COUNT) devide_positon.chA.totle_cup = MIN_CUP_COUNT;
	if(devide_positon.chA.totle_cup>MAX_CUP_COUNT) devide_positon.chA.totle_cup = MAX_CUP_COUNT;
	
	if(devide_positon.chB.totle_cup<MIN_CUP_COUNT) devide_positon.chB.totle_cup = MIN_CUP_COUNT;
	if(devide_positon.chB.totle_cup>MAX_CUP_COUNT) devide_positon.chB.totle_cup = MAX_CUP_COUNT;
}
