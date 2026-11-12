#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "product_set.h"
#include "spi_flash_sfud.h"
#include "dat_backup.h"
#include "crc32.h"
#include "mdebug.h"
#include "at24cxx.h"
#include "device_alarm.h"
#include "egg_control.h"
#include "product_hostroy.h"

ALIGN(RT_ALIGN_SIZE)

static product_packge_type product_packge;      //产品包装
static product_attribute_type product_attribute;          //产品属性  
static supplier_management_type supplier_management;          //供应商管理
static production_type pre_production1;                  //预计生产管理
static throw_ch_product_type product_throw_ch_part;
static production_type pre_production;                  //预计生产管理
uint16_t get_pruduct_packge_count()
{
	return product_packge.count;
}


uint8_t  per_product_get_pruduct_packge_info(uint16_t *name)
{
	int result;
	uint8_t packge_id = 255;
	uint8_t i;
	
	for(i=0;i<product_packge.count;i++)
	{
	   result = rt_strcmp((const char *)name,(const char *)product_packge.info[i].name);
	   if(result==0)
	   {
	     packge_id = i;
	   }
	}
	return packge_id;
}
uint16_t packge_err_count=0;
// 获取某一个包装信息，如果目前的包装信息校验或者初始化不对，则需要从flash里面读取包装信息
// 哪些情况会不同呢，是屏幕设置了，主控没有保存或者保存不成功？
void get_pruduct_packge_info(uint16_t id,product_packge_info *dat)
{
	uint32_t crc1;
	crc1 = CRC32_file((unsigned char*)&product_packge,sizeof(product_packge_type)-4);
	
	if((crc1 !=product_packge.crc)&&(product_packge.init !=SPI_FLASH_DAT_IS_INIT_FLAG))
	{
		packge_err_count++;
		DEBUG_ERR("packge crc err read spi flash\r\n");
		if(packge_err_count>100)
		{
			packge_err_count=0;
			read_product_packge_type(&product_packge);
		}
	}
	else 
	{
	  packge_err_count=0;
	}
	rt_memcpy(dat,&product_packge.info[id],sizeof(product_packge_info));
	dat->crc_32 = CRC32_file((unsigned char*)dat,sizeof(product_packge_info)-4);
}
// 获取所有包装信息，如果目前的所有包装信息校验或者初始化不对，则需要从flash里面读取所有包装信息
// 哪些情况会不同呢，是屏幕设置了，主控没有保存或者保存不成功？
void get_product_packge_dat(product_packge_type *dat)
{
	uint32_t crc1,crc2;
	rt_base_t level;
	crc1 = CRC32_file((unsigned char*)&product_packge,sizeof(product_packge_type)-4);
	
	if((crc1 !=product_packge.crc)&&(product_packge.init !=SPI_FLASH_DAT_IS_INIT_FLAG))
	{
		packge_err_count++;
		DEBUG_ERR("packge crc err read spi flash\r\n");
	  if(packge_err_count>100)
		{
			packge_err_count=0;
			read_product_packge_type(&product_packge);
		}
	}
	else 
	{
	  packge_err_count=0;
	}

	rt_memcpy(dat,&product_packge,sizeof(product_packge_type));
}

uint8_t package_change_flag=0;

void reflash_per_pruduct()
{
	uint8_t i;
	uint8_t id;
	
	rt_base_t level;
   
    level = rt_hw_interrupt_disable();
	// 最多12预产品
	for(i=0;i<12;i++)
	{
		// 某个预生产的信息不为空即该预生产需要生产
		if(pre_production.pre_production[i].empty !=0)
		{
			// 根据该预生产信息的名称找到预产品在产品包装列表中的id(序号)信息
			// 这个包装信息里面就有重量等级，颜色范围等
			// 包装管理界面，建了包装的都有id(序号)和名称，相当于通过名称去到那个id(序号)
			// 肯定某个地方有按序号存储这些包装信息，相当于1个包装信息数组，根据屏幕设计，包装信息可能有很多个，因为还可以翻页
			id = per_product_get_pruduct_packge_info(pre_production.pre_production[i].packge_name);
			if(id==255)
			{
			  rt_memcpy(pre_production.pre_production[i].packge_name,"请重新选择包装",rt_strlen("请重新选择包装"));
			  DEBUG_WARN("not find packge id=%d name=%s\n",i,pre_production.pre_production[i].packge_name);
			}
			else 
			{
			  pre_production.pre_production[i].packge_id = id;
			  DEBUG_WARN("change packge id=%d\n",id);
			}
		}
	}
	 backup_save_per_pruduct_dat(&pre_production);
	 rt_hw_interrupt_enable(level);
}
// 将包装信息按照名称排序
// 是用flash的一页来存储的，能存多少个呢 最大能存 PRODUCT_PACKGE_INFO_COUNT 个
// 实际个数是count
void product_packge_paixu(product_packge_type *pk_type)
{
	
	uint8_t i,j;
	int result;
	 
	product_packge_info tmp;

	rt_base_t level;
	level = rt_hw_interrupt_disable();
	for(i=0;i<(pk_type->count-1);i++)
	{
		pk_type->info[i].name[9]=0;
	}
	for(i=0;i<(pk_type->count-1);i++)
	{
		for(j=0;j<(pk_type->count-1);j++)
		{
			// 这个把所有的包装信息按照名称来排序
			// 是按照字母先后顺序来排
			// result >0表示序号在前面的名称，字母在后面，所以要移到后面去
			// 也就是按照a,b,c...这样排列
	        result = rt_strcmp((const char *)pk_type->info[j].name,(const char *)pk_type->info[j+1].name);
		    if(result >0)
			{
			  package_change_flag=2;
			  rt_memcpy(&tmp,&pk_type->info[j],sizeof(product_packge_info));
			  rt_memcpy(&pk_type->info[j],&pk_type->info[j+1],sizeof(product_packge_info));
			  rt_memcpy(&pk_type->info[j+1],&tmp,sizeof(product_packge_info));
			}
		}
	}
	rt_hw_interrupt_enable(level);
	if(package_change_flag>0)
	{
	  package_change_flag =0;
		
	  reflash_per_pruduct();
	}
}
// 读取包装信息
rt_err_t read_product_packge_type(product_packge_type *pk_type)
{
	rt_err_t result;
  uint32_t crc;
	// 先读取正常位置的包装信息，如果是初始化了的则返回即说明读到了正确的包装信息
	result = spi_flash_read(PRODUCT_PACKGE_INFO_SPI_ADDR,(uint8_t *)pk_type,sizeof(product_packge_type));
	if(pk_type->init ==SPI_FLASH_DAT_IS_INIT_FLAG)
	{
		return result;
	}
	// 如果包装信息显示没有初始化好，但是读到了包装信息并且包装信息校验是对的，则也认为读到正确的包装信息
	crc = CRC32_file((unsigned char*)pk_type,sizeof(product_packge_type)-4);
	if(((result ==RT_EOK)&&(crc ==pk_type->crc)))
	{
	  return result;
	}
	
	// 如果都不对，则读取备份的包装产品信息
	// 读到备份的包装信息如果是显示初始化ok的，则返回备份的包装信息
	result = spi_flash_read(PRODUCT_PACKGE_BACK_INFO_SPI_ADDR,(uint8_t *)pk_type,sizeof(product_packge_type));
	if(pk_type->init ==SPI_FLASH_DAT_IS_INIT_FLAG)
	{
		return result;
	}
	// 如果备份的包装信息显示没有初始化好，但是读到了备份的包装信息并且包装信息校验是对的，则也认为读到正确的包装信息
	crc = CRC32_file((unsigned char*)pk_type,sizeof(product_packge_type)-4);
	if(((result ==RT_EOK)&&(crc ==pk_type->crc)))
	{
	  return result;
	}
	// 如果以上都不对，则没有读到正确的包装信息，则报警
	set_device_alarm(CHECK_PRODUCK_PACKGE_INFO_IS_OK);
	return result;
}

// 将包装信息排序后存入正常的包装信息flash地址和备份的包装信息flash地址
void paixu_product_packge()
{
  //rt_base_t level;
	//level = rt_hw_interrupt_disable();
	product_packge_paixu(&product_packge);
	product_packge.init = 0;
	product_packge.crc = CRC32_file((unsigned char*)&product_packge,sizeof(product_packge_type)-4);
	//rt_hw_interrupt_enable(level);
	
	spi_flash_ease(PRODUCT_PACKGE_INFO_SPI_ADDR);
	spi_flash_write(PRODUCT_PACKGE_INFO_SPI_ADDR,(uint8_t *)&product_packge,sizeof(product_packge_type));
	rt_thread_mdelay(20);
	spi_flash_ease(PRODUCT_PACKGE_BACK_INFO_SPI_ADDR); 
	spi_flash_write(PRODUCT_PACKGE_BACK_INFO_SPI_ADDR,(uint8_t *)&product_packge,sizeof(product_packge_type));
	rt_thread_mdelay(20);
	read_product_packge_type(&product_packge);
}
// 保存包装信息到正常的包装信息flash地址和备份的包装信息flash地址(将排序注销了)
rt_err_t save_product_packge_type(product_packge_type *pk_type)
{
  
	
//	product_packge_paixu(pk_type);
	pk_type->init = 0;
	spi_flash_ease(PRODUCT_PACKGE_INFO_SPI_ADDR);
	pk_type->crc = CRC32_file((unsigned char*)pk_type,sizeof(product_packge_type)-4);
	spi_flash_write(PRODUCT_PACKGE_INFO_SPI_ADDR,(uint8_t *)pk_type,sizeof(product_packge_type));
	rt_thread_mdelay(20);
	spi_flash_ease(PRODUCT_PACKGE_BACK_INFO_SPI_ADDR); 
	spi_flash_write(PRODUCT_PACKGE_BACK_INFO_SPI_ADDR,(uint8_t *)pk_type,sizeof(product_packge_type));
	rt_thread_mdelay(20);
	return read_product_packge_type(&product_packge);
}

// 创建包装信息
// 包装信息没有达到最大的包装信息数量则依次保存并存储
// 达到最大的数量，则覆盖最后一个包装信息并存储
void crate_product_packge_info(product_packge_info *dat)
{

  if(product_packge.count<PRODUCT_PACKGE_INFO_COUNT)
	{
	  product_packge.count++;
	}
	else 
	{
	 product_packge.count = PRODUCT_PACKGE_INFO_COUNT;
	}
	rt_kprintf("product_packge.count=%d totle=%d\n",product_packge.count,PRODUCT_PACKGE_INFO_COUNT);
	rt_memcpy(&product_packge.info[product_packge.count-1],dat,sizeof(product_packge_info));
	rt_kprintf("copy product_packge.count=%d totle=%d\n",product_packge.count,PRODUCT_PACKGE_INFO_COUNT);
    save_product_packge_type(&product_packge);
	rt_kprintf("save product packge info count =%d\n",product_packge.count);
	read_product_packge_type(&product_packge);
	rt_kprintf("read product packge info count =%d\n",product_packge.count);
}



// 删除包装信息
void delete_product_packge_info(uint16_t id)
{
  uint8_t i;
  if(id>PRODUCT_PACKGE_INFO_COUNT)
	{
		return ;
	}
  // 比如一共有10个包装信息，我要删除第8个(程序上是7)
	// 那么第9个覆盖第8个
	// 第10个覆盖第9个，后面的依次类推
	// 如果是删除最后一个，比如这里的第10，可以不用管，后面的数量减1不记录就是了
	if(id != product_packge.count)
	{
	  for(i=(id-1);i<product_packge.count;i++)
		{
			rt_memcpy(&product_packge.info[i],&product_packge.info[i+1],sizeof(product_packge_info));
		}
	}
	if( product_packge.count>0)
		 product_packge.count--;
	// 重新存储覆盖后(如果是删除最后1个则没有变化)的包装信息
  save_product_packge_type(&product_packge);
	rt_kprintf("save product packge info count =%d\n",product_packge.count);
	read_product_packge_type(&product_packge);
	rt_kprintf("read product packge info count =%d\n",product_packge.count);
	reflash_per_pruduct();
}
// 编辑包装信息
// id(序号)小于1不对，界面的序号至少从1开始
// 如果id(序号)大于记录的包装数量，说明编辑的这个包装信息是新增的
// 如果id(序号)小于等于记录的包装数量，说明编辑的这个包装信息是已经有的
void edit_product_packge_info(uint8_t id,product_packge_info *dat)
{
	if(id<1) return ;
  if(id>product_packge.count)  //新增
	{
		product_packge.count++;
		if(product_packge.count>PRODUCT_PACKGE_INFO_COUNT)
		{
			product_packge.count = PRODUCT_PACKGE_INFO_COUNT;
		}
		rt_memcpy(&product_packge.info[product_packge.count-1],dat,sizeof(product_packge_info));
	}
	else
	{
		rt_memcpy(&product_packge.info[id-1],dat,sizeof(product_packge_info));
	}
  save_product_packge_type(&product_packge);
	rt_kprintf("save product packge info count =%d\n",product_packge.count);
	read_product_packge_type(&product_packge);
	rt_kprintf("read product packge info count =%d\n",product_packge.count);
}
// 加载包装信息
rt_err_t load_product_packge_dat()
{
	rt_err_t result;
	uint16_t i;
	uint16_t loop;
	uint16_t read_buf_size;
	uint32_t read_addr;
	uint32_t write_addr;
	uint32_t crc;
	char *read_buf;
	read_buf_size =128;
	
	
	// 先读取包装信息
	result =   read_product_packge_type(&product_packge);
  crc =  CRC32_file((unsigned char*)&product_packge,sizeof(product_packge_type)-4);
	// 如果检验对的，则加载到了正确的包装信息
	if(crc ==product_packge.crc)
	{
	  return 0;
	}
	// 如果读取的数据校验不对，又显示是没有初始化的
	// 先报警
	// 从eeprom中读取备份的包装信息到flash的正常的包装信息的位置
	if((product_packge.init != SPI_FLASH_DAT_IS_INIT_FLAG)&&(result ==RT_EOK))
	{
		write_addr = PRODUCT_PACKGE_INFO_SPI_ADDR;
	  read_addr = EEP_BACKGE_PACKGE_ADDR;
		read_buf = rt_malloc(read_buf_size+10);
		set_device_alarm(SISTME_PART_READ_ERROR);
		if(read_buf == RT_NULL)
		{
		
			return 0;
		}
	    loop = 4096/read_buf_size;   
		for(i=0;i<loop;i++)
		{
			rt_memset(read_buf,0,read_buf_size);
			
			result = at24cxx_multy_read(read_addr,(unsigned char *)read_buf,read_buf_size);
			result = spi_flash_read(write_addr,(unsigned char *)read_buf,read_buf_size);
			write_addr +=read_buf_size;
			read_addr += read_buf_size;
			rt_thread_mdelay(5);
		}
		//rt_kprintf("产品包装数据初始化\n");
		//if(product_packge.count!=0)
		//{
	     // rt_memset(&product_packge,0,sizeof(product_packge_type));
		 // product_packge.init = SPI_FLASH_DAT_IS_INIT_FLAG;
		 // save_product_packge_type(&product_packge);
		//}
		
	}
	// 如果显示不成功则设置系统参数读取错误报警
	if(result !=RT_EOK)
	{
	   set_device_alarm(SISTME_PART_READ_ERROR);
	}
	
  return  result;
}


// 产品属性 <------------------------------------------>
// 获取一共有多少个产品信息
uint16_t get_pruduct_attribute_count()
{
	return product_attribute.count;
}
// 根据id(序号)获取该序号的产品信息
void get_pruduct_attribute_info(uint16_t id,product_attribute_info *dat)
{
	rt_memcpy(dat,&product_attribute.info[id],sizeof(product_attribute_info));
}
// 获取所有产品信息
void get_product_attribute_dat(product_attribute_type *dat)
{
	rt_memcpy(dat,&product_attribute,sizeof(product_attribute_type));
}
// 从flash中读取所有的产品信息
rt_err_t read_product_attribute_type(product_attribute_type *pa_type)
{
	 return spi_flash_read(PRODUCT_ATTRIBUTE_INFO_SPI_ADDR,(uint8_t *)pa_type,sizeof(product_attribute_type));
}
// 保存所有的产品信息到flash中
rt_err_t save_product_attribute_type(product_attribute_type *pa_type)
{
  spi_flash_ease(PRODUCT_ATTRIBUTE_INFO_SPI_ADDR);
	spi_flash_write(PRODUCT_ATTRIBUTE_INFO_SPI_ADDR,(uint8_t *)pa_type,sizeof(product_attribute_type));
	return read_product_attribute_type(&product_attribute);
}

// 创建1个产品信息
// 产品数量小于PRODUCT_ATTRIBUTE_INFO_COUNT则直接将产品信息数量加1并将产品信息添加到该产品信息列表的后面
// 产品数量大于等于PRODUCT_ATTRIBUTE_INFO_COUNT则直接将产品信息覆盖产品信息列表中最后一个
void crate_product_attribute_info(product_attribute_info *dat)
{

  if(product_attribute.count<PRODUCT_ATTRIBUTE_INFO_COUNT)
	{
	  product_attribute.count++;
	}
	else 
	{
	 product_attribute.count = PRODUCT_ATTRIBUTE_INFO_COUNT;
	}
	rt_kprintf("product_attribute.count=%d totle=%d\n",product_attribute.count,PRODUCT_ATTRIBUTE_INFO_COUNT);
	rt_memcpy(&product_attribute.info[product_attribute.count-1],dat,sizeof(product_attribute_info));
	rt_kprintf("copy product_attribute.count=%d totle=%d\n",product_attribute.count,PRODUCT_ATTRIBUTE_INFO_COUNT);
  save_product_attribute_type(&product_attribute);
	rt_kprintf("save product attribute info count =%d\n",product_attribute.count);
	read_product_attribute_type(&product_attribute);
	rt_kprintf("read product attribute info count =%d\n",product_attribute.count);
}
// 删除产品信息
// 如果是删除最后一个，只需要将产品信息数量减1就可以了，不用移动数据
// 如果不是删除最后一个，则将当前要删除的后面的一个数据移动到当前删除的位置，后面的依次往前移动
void delete_product_attribute_info(uint16_t index)
{
	uint8_t i;
	
	if(index>PRODUCT_ATTRIBUTE_INFO_COUNT)
		return;
	DEBUG_INFO("delete index=%d name=%s\n",index,product_attribute.info[product_attribute.count-1].product_name);
	if(index !=product_attribute.count)
	{
	  for(i=(index-1);i<product_attribute.count;i++)
		{
			rt_memcpy(&product_attribute.info[i],&product_attribute.info[i+1],sizeof(product_attribute_info));
		}
	}
	if( product_attribute.count>0)
		 product_attribute.count--;
  save_product_attribute_type(&product_attribute);
	rt_kprintf("save product attribute info count =%d\n",product_attribute.count);
	read_product_attribute_type(&product_attribute);
	rt_kprintf("read product attribute info count =%d\n",product_attribute.count);
}

// 编辑产品信息
// id(序号)小于1不对，界面的序号至少从1开始
// 如果id(序号)大于记录的产品信息数量，说明编辑的这个产品信息是新增的
// 如果id(序号)小于等于记录的产品信息数量，说明编辑的这个产品信息是已经有的
void edit_product_attribute_info(uint8_t id,product_attribute_info *dat)
{
	if(id<1) return ;
  if(id>product_attribute.count)  //新增
	{
		product_attribute.count++;
		if(product_attribute.count>PRODUCT_ATTRIBUTE_INFO_COUNT)
		{
			product_attribute.count = PRODUCT_ATTRIBUTE_INFO_COUNT;
		}
		rt_memcpy(&product_attribute.info[product_attribute.count-1],dat,sizeof(product_attribute_info));
	}
	else
	{
		rt_memcpy(&product_attribute.info[id-1],dat,sizeof(product_attribute_info));
	}
  save_product_attribute_type(&product_attribute);
	rt_kprintf("save product attribute info count =%d\n",product_attribute.count);
	read_product_attribute_type(&product_attribute);
	rt_kprintf("read product attribute info count =%d\n",product_attribute.count);
}
// 读取产品信息
// 读取到的产品信息如果没有初始化，则直接初始化并存储？这样对不对

rt_err_t load_product_attribute_dat()
{
	rt_err_t result;
  result =   read_product_attribute_type(&product_attribute);
	if((product_attribute.init != SPI_FLASH_DAT_IS_INIT_FLAG)&&(result ==RT_EOK))
	{
		rt_kprintf("产品管理数据初始化\n");
	  rt_memset(&product_attribute,0,sizeof(product_attribute_type));
		product_attribute.init = SPI_FLASH_DAT_IS_INIT_FLAG;
		save_product_attribute_type(&product_attribute);
	}
	return result;
}

// 产品属性 <------------------------------------------>

// ************供应商管理*************/
// 获取供应商的数量
uint16_t get_supplier_count(void)
{
	return supplier_management.count;
}
// 获取所有供应商信息
void get_supplier_dat(supplier_management_type *dat)
{
	rt_memcpy(dat,&supplier_management,sizeof(supplier_management_type));
}
// 创建一个供应商信息
void crate_supplier_info(supplier_management_info *dat)
{
 if(supplier_management.count<SUPPLIER_MANGEMENT_COUNT)
	{
	  supplier_management.count++;
	}
	else 
	{
	 supplier_management.count = SUPPLIER_MANGEMENT_COUNT;
	}

	rt_memcpy(&supplier_management.info[supplier_management.count-1],dat,sizeof(supplier_management_info));
  save_supplier_type(&supplier_management);
	read_supplier_type(&supplier_management);
	rt_kprintf("supplier type count =%d\n",supplier_management.count);
}
// 从flash中读取所有的供应商信息
rt_err_t read_supplier_type(supplier_management_type *sm_type)
{
	return spi_flash_read(SUPPLIER_MANGEMENT_SPI_ADDR,(uint8_t *)sm_type,sizeof(supplier_management_type));
}
// 保存供应商信息并读到全局变量中
rt_err_t save_supplier_type(supplier_management_type *sm_type)
{
  spi_flash_ease(SUPPLIER_MANGEMENT_SPI_ADDR);
	spi_flash_write(SUPPLIER_MANGEMENT_SPI_ADDR,(uint8_t *)sm_type,sizeof(supplier_management_type));
	return read_supplier_type(&supplier_management);
}

// 获取某一个供应商信息
void get_supplier_info(uint16_t id,supplier_management_info *dat)
{
	rt_memcpy(dat,&supplier_management.info[id],sizeof(supplier_management_info));
}
// 编辑供应商信息
// id(序号)小于1不对，界面的序号至少从1开始
// 如果id(序号)大于记录的供应商信息数量，说明编辑的这个供应商信息是新增的
// 如果id(序号)小于等于记录的供应商信息数量，说明编辑的这个供应商信息是已经有的
void edit_supplier_info(uint8_t id,supplier_management_info *dat)
{
	if(id<1) return ;
  if(id>supplier_management.count)  //新增
	{
		supplier_management.count++;
		if(supplier_management.count>SUPPLIER_MANGEMENT_COUNT)
		{
			supplier_management.count = SUPPLIER_MANGEMENT_COUNT;
		}
		rt_memcpy(&supplier_management.info[supplier_management.count-1],dat,sizeof(supplier_management_info));
	}
	else
	{
		rt_memcpy(&supplier_management.info[id-1],dat,sizeof(supplier_management_info));
	}
  save_supplier_type(&supplier_management);
	read_supplier_type(&supplier_management);
	rt_kprintf("supplier count =%d\n",supplier_management.count);
}



// 删除供应商信息
// 如果是删除最后一个，只需要将供应商信息数量减1就可以了，不用移动数据
// 如果不是删除最后一个，则将当前要删除的后面的一个数据移动到当前删除的位置，后面的依次往前移动
void delete_supplier_info(uint8_t id)
{
  uint8_t i;
	
  if(id>SUPPLIER_MANGEMENT_COUNT)  //新增
	{
		return ;
	}
  if(id != supplier_management.count)
	{
	  for(i=(id-1);i<supplier_management.count;i++)
		{
			rt_memcpy(&supplier_management.info[i],&supplier_management.info[i+1],sizeof(supplier_management_info));
		}
	}  
	 if( supplier_management.count>0)
		 supplier_management.count--;

  save_supplier_type(&supplier_management);
	read_supplier_type(&supplier_management);
	rt_kprintf("supplier count =%d\n",supplier_management.count);
}

// 加载供应商信息
rt_err_t load_supplier_dat()
{
	rt_err_t result;

  result =   read_supplier_type(&supplier_management);
	if((supplier_management.init != SPI_FLASH_DAT_IS_INIT_FLAG)&&(result ==RT_EOK))
	{
		rt_kprintf("供应商管理数据初始化\n");
	  rt_memset(&supplier_management,0,sizeof(supplier_management_type));
		supplier_management.init = SPI_FLASH_DAT_IS_INIT_FLAG;
		save_supplier_type(&supplier_management);
	}
	
  return  result;
}
// ************供应商管理*************/


// 保存预生产信息
void save_pre_production_info1(uint8_t id,pre_production_info *dat)
{
	uint8_t i;
	uint8_t result=0;
	rtc_info_t time;
	// 如果预生产信息id大于等于最大的数量则直接返回
  if(id>=TASK_PRUDUCTION_COUNT)
	{
		return;
	}
	// 如果要保存的预生产信息在列表中不为空，则说明已经存在是修改的
	if(pre_production.pre_production[id].empty>0)  //修改  
	{
		// 修改了产品id，即重新选择了产品(产品中包含各种等级，相当于重新修改了各种等级)
		if(dat->product_id != pre_production.pre_production[id].product_id)
		{
		  result += 0x01;
			//DEBUG_INFO("now product id=%d old id=%d\n",dat->product_id ,pre_production.pre_production[id].product_id);
		}
		// 修改了包装id，即重新选择了包装(包装中包含包装信息，相当于重新修改了包装信息)
		if(dat->packge_id != pre_production.pre_production[id].packge_id)
		{
		  result += 0x02;
		}
		// 如果修改了产品，则需要清空已经生产的蛋的信息
		if((result&0x01)>0)
		{
			//save_product_hostroy(id);
			clear_already_pruduct_egg_dat(id);	// 将已经生产了的蛋的信息清零
		}
		rt_memcpy(&pre_production.pre_production[id],dat,sizeof(pre_production_info));
		pre_production.pre_production[id].empty=1;
	}
	else                                         //新增 地址不一定是+1 按照顺序点
	{
	  for(i=0;i<TASK_PRUDUCTION_COUNT;i++)
		{
			// 如果是新增的话，预生产没有序号，可以编辑任意一排，只有不超过最大的数量即可
			// 就循环遍历预生产信息列表中空的位置然后存进去
		  if(pre_production.pre_production[i].empty==0)
			{
				result +=0x04;
				rt_memcpy(&pre_production.pre_production[i],dat,sizeof(pre_production_info));
				pre_production.pre_production[i].empty=1;
				clear_already_pruduct_egg_dat(i);
				break;
			}		
		}
	}
	// 预生产信息存储在sram
	backup_save_per_pruduct_dat(&pre_production);

	//DEBUG_INFO("保存 改动result=%02x  0 不变  1=产品  2=包装 4=新增\n",result);
}

void save_pre_production_info(uint8_t id,pre_production_info *dat1)
{
	uint8_t i;
	uint8_t result=0;
	rtc_info_t time;
	rt_base_t level;
	
  if(id>=TASK_PRUDUCTION_COUNT)
	{
		return;
	}
	
	 production_type * pre_production_dat;
	 pre_production_dat =rt_malloc(sizeof(production_type));
	 if(pre_production_dat !=RT_NULL)
	 {
		 
		get_production_mange_set(pre_production_dat);

		if(pre_production.pre_production[id].empty>0)  //修改  
		{
		if(dat1->product_id != pre_production.pre_production[id].product_id)
		{
		  result += 0x01;
			//DEBUG_INFO("now product id=%d old id=%d\n",dat->product_id ,pre_production.pre_production[id].product_id);
		}
		if(dat1->packge_id != pre_production.pre_production[id].packge_id)
		{
		  result += 0x02;
		}
		if((result&0x01)>0)
		{
			// 先保存产品历史
			save_product_hostroy(id);
			// 然后清空已经生产的信息
			clear_already_pruduct_egg_dat(id);
		}
		
		rt_memcpy(&pre_production_dat->pre_production[id],dat1,sizeof(pre_production_info));
		pre_production_dat->pre_production[id].empty=1;
	}
	else                                         //新增 地址不一定是+1 按照顺序点
	{
	  for(i=0;i<TASK_PRUDUCTION_COUNT;i++)
		{
		  if(pre_production.pre_production[i].empty==0)
			{
				result +=0x04;
				rt_memcpy(&pre_production_dat->pre_production[i],dat1,sizeof(pre_production_info));
				pre_production.pre_production[i].empty=1;
				clear_already_pruduct_egg_dat(i);
				break;
			}		
		}
	}

	  backup_save_per_pruduct_dat(pre_production_dat);
	  level = rt_hw_interrupt_disable();
	  read_bak_per_pruduct_dat(&pre_production);
	  rt_hw_interrupt_enable(level);
		rt_free(pre_production_dat);
	}
}
// 获取某一个预生产信息
void get_pre_production_info(uint8_t id,pre_production_info *dat)
{
  if(id>=TASK_PRUDUCTION_COUNT)
	{
		dat =RT_NULL;
		return;
	}
	rt_memcpy(dat,&pre_production.pre_production[id],sizeof(pre_production_info));
}
// 删除某一个预生产信息
void delete_pre_production_info(uint8_t id)
{
	uint8_t i;
	for(i=id;i<(TASK_PRUDUCTION_COUNT-1);i++)
	{
		rt_memcpy(&pre_production.pre_production[i],&pre_production.pre_production[i+1],sizeof(pre_production_info));
	}
	if(id<TASK_PRUDUCTION_COUNT)
	{
	  rt_memset(&pre_production.pre_production[TASK_PRUDUCTION_COUNT-1],0,sizeof(pre_production_info));
	}
	
	backup_save_per_pruduct_dat(&pre_production);
}
uint16_t per_pruduct_err_count=0;
// 获取预生产信息
void get_production_mange_set(production_type *dat)
{
	uint32_t crc;
	rt_base_t level;
	

	level = rt_hw_interrupt_disable();
	rt_memcpy(dat,&pre_production,sizeof(production_type));
	rt_hw_interrupt_enable(level);
	
	crc = CRC32_file((uint8_t *)dat,sizeof(production_type)-4);
	if((dat->crc != crc) && (dat->crc != BACKUP_SRAM_FLAG))
	{
		per_pruduct_err_count++;
		if(per_pruduct_err_count>10)
		{
		  set_device_alarm(PER_PROTUCE_ERR);
		}
		if(per_pruduct_err_count>3)
		{
			//DEBUG_CARE("RELOAD per pruduct\r\n");
			read_bak_per_pruduct_dat(dat);
			read_bak_per_pruduct_dat(&pre_production);
		}
	}
	else 
	{
	  per_pruduct_err_count=0;
	}
}
// 保存预生产信息
void save_production_mange_set(production_type *dat)
{
	rt_memcpy(&pre_production,dat,sizeof(production_type));
	backup_save_per_pruduct_dat(&pre_production);
}
// 保存抛弃通道的参数
void save_product_throw_ch_part(throw_ch_product_type *dat)
{
	rt_memcpy(&product_throw_ch_part,dat,sizeof(throw_ch_product_type));
	backup_save_throw_ch_part(&product_throw_ch_part);
	backup_read_throw_ch_part(&product_throw_ch_part);
}
// 获取抛弃通道的参数
extern void read_thorw_ch_ionfo(throw_ch_product_type *dat);
void read_product_throw_ch_part(throw_ch_product_type *dat)
{
	uint32_t crc;
	rt_memcpy(dat,&product_throw_ch_part,sizeof(throw_ch_product_type));
	
	crc = CRC32_file((uint8_t *)dat,sizeof(throw_ch_product_type)-4);
	if(crc !=dat ->crc)
	{
	  read_thorw_ch_ionfo(dat);
	}
	if(dat->force_throwa_stop_count==0) dat->force_throwa_stop_count=100;
	if(dat->force_throwa_stop_time==0) dat->force_throwa_stop_time=1;
	
	if(dat->force_throwb_stop_count==0) dat->force_throwb_stop_count=100;
	if(dat->force_throwb_stop_time==0) dat->force_throwb_stop_time=1;
	
	if(dat->cha[0].abnormal_stop_count==0) dat->cha[0].abnormal_stop_count=100;
	if(dat->cha[0].abnormal_stop_time==0) dat->cha[0].abnormal_stop_time=1;
	if(dat->chb[0].abnormal_stop_count==0) dat->chb[0].abnormal_stop_count=100;
	if(dat->chb[0].abnormal_stop_time==0) dat->chb[0].abnormal_stop_time=1;
	
	if(dat->cha[1].abnormal_stop_count==0) dat->cha[1].abnormal_stop_count=100;
	if(dat->cha[1].abnormal_stop_time==0) dat->cha[1].abnormal_stop_time=1;
	if(dat->chb[1].abnormal_stop_count==0) dat->chb[1].abnormal_stop_count=100;
	if(dat->chb[1].abnormal_stop_time==0) dat->chb[1].abnormal_stop_time=1;
	
	if(dat->cha[2].abnormal_stop_count==0) dat->cha[2].abnormal_stop_count=100;
	if(dat->cha[2].abnormal_stop_time==0) dat->cha[2].abnormal_stop_time=1;
	if(dat->chb[2].abnormal_stop_count==0) dat->chb[2].abnormal_stop_count=100;
	if(dat->chb[2].abnormal_stop_time==0) dat->chb[2].abnormal_stop_time=1;
}
// 初始化预生产参数
// 读取抛弃通道的参数
// 读取预生产的参数
extern void check_per_pruduct_save_addr();
void init_per_production_dat()
{

	backup_read_throw_ch_part(&product_throw_ch_part);
	check_per_pruduct_save_addr();
	backup_read_per_pruduct_dat(&pre_production);
}
// 获取已经使用的包装通道
uint16_t get_use_packge_line()
{

	 uint16_t j;
	 uint16_t packge_line;

   packge_line = 0;
	 for(j=0;j<12;j++)    //12个产品
	 {
		 if(pre_production.pre_production[j].empty !=0)  //不为空
		 {
			  packge_line = packge_line | pre_production.pre_production[j].packge_line;// 一个预生产的产品可能不止一条通道，说明这个是8421的形式表示的吗，不然不易单或怎么搞
		 }
	 }
	 return packge_line;
}


static int showproduct(int argc, char **argv) //
{
	uint8_t i;
	uint8_t flag;
	for(i=0;i<12;i++)
	{
	  DEBUG_INFO("empty[%d]=%d packge id=%d name=%s\n",i,pre_production.pre_production[i].empty,
		pre_production.pre_production[i].packge_id,pre_production.pre_production[i].packge_name);
	}
}
MSH_CMD_EXPORT(showproduct,show  porduct info);

