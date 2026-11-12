
#include <rtthread.h>
#include "err_check.h"

#ifndef _PRODUCT_SET_H__
#define _PRODUCT_SET_H__

#define SPT_CH_LINE 18   //8+6


#define FLASH_RECODE_DATA_SIZE 100
#define PER_SECT_SAVE_MESG_COUNT  (4096/FLASH_RECODE_DATA_SIZE)
#define FLASH_RECODE_DAT_FLAG 0xa5


#define PRODUCT_PACKGE_INFO_SPI_ADDR      (1*4096)
#define PRODUCT_ATTRIBUTE_INFO_SPI_ADDR   (3*4096)
#define SUPPLIER_MANGEMENT_SPI_ADDR       (5*4096)

#define PER_THROW_ADDR_SPI_ADDR           (10*4096)

#define PER_PRUDUCT_ADDR_SPI_ADDR         (20*4096)



#define PRODUCT_HOSTRAY_LOGS_OFFEST_ADDR      (1000*1024)   //1MB 起始地址
#define PRODUCT_HOSTRAY_LOGS_SECT_COUNT        500   //(500*4096)    //2MB 长度

#define RECODE_PRODUCT_EGGS_INFO_START_ADDR (5000*1024)   //5MB 起始地址
#define RECODE_PRODUCT_EGGS_MEM_LEN          ((12*40)*4096)    //大约2MB 12个月  每个月预留40天的内存  每天存储在一个扇区里面(4096) 
#define PRODUCT_PACKGE_BACK_INFO_SPI_ADDR   (7000*4096)  //7MB 起始地址

#define RECODE_PRODUCT_EGGS_PER_SEC_LEN      4096   //
#define USE_FLAG           0XAA55


#define PACKGE_LINE_COUNT             12
#define TASK_PRUDUCTION_COUNT          12   //计划生产种类  10种
#define PRODUCT_PACKGE_INFO_COUNT ((4096-10)/sizeof(product_packge_info))       //用一页存放  擦写次数足够  // sizeof(product_packge_info)=52字节	// PRODUCT_PACKGE_INFO_COUNT 78 
	
#define PRODUCT_ATTRIBUTE_INFO_COUNT ((4096-10)/sizeof(product_attribute_info))       //用一页存放  擦写次数足够  // sizeof(product_attribute_info)=100字节	// PRODUCT_ATTRIBUTE_INFO_COUNT 40 
#define SUPPLIER_MANGEMENT_COUNT 50                         //供应商数量

#define SPI_FLASH_DAT_IS_INIT_FLAG 0X55A0

typedef struct
{
	uint16_t name[10];                         //包装盒名称
	uint16_t row;                              //包装盒行数
	uint16_t line;                             //包装盒列数即一行的个数    
	uint16_t auto_transmit_egg_carton_positon; //自动派托位置
	uint16_t check_egg_carton_position;        //蛋托检测位置
	uint16_t packge_line1_postion;          //首排落蛋位置
	uint16_t packge_line2_postion;
	uint16_t packge_line3_postion;
	uint16_t packge_line4_postion;
	uint16_t packge_line5_postion;
	uint16_t recive_claw_fall_dowm_position;   //接收爪下降位置
	uint16_t recive_claw_open_position;        //接收爪打开位置
	uint16_t recive_claw_close_position;       //接收爪关闭位置
	uint16_t transport_start_position;         //传输线启动位置
	uint16_t use_box_count;                    //占用分级线格数
	uint32_t crc_32;
}product_packge_info;



typedef struct
{
	uint16_t packge_id;     
	uint16_t product_name[10];
	uint16_t packge_name[10];
  uint16_t weight_leve[10];   //重量等级 8个 + 2个自定义
	uint16_t colour_leve[6];      //颜色等级 4+2 个
	uint16_t crack_leve[6];       //裂纹等级 4+2个
	uint16_t dirty_leve[5];       //脏蛋等级 3+2个
	uint16_t print_id;         //打印编号
	uint16_t bood_leve;
}product_attribute_info;                     //产品管理

typedef struct
{
	uint16_t name[10];
}supplier_management_info;  // 供应商信息

typedef struct
{
	uint16_t empty;
	uint16_t product_id;        //产品管理  里面有包装信息和ID
	uint16_t packge_id;     
	uint16_t product_name[10];
	uint16_t packge_name[10];
  uint16_t weight_leve[10];   //重量等级 8个 + 2个自定义
	uint16_t colour_leve[6];      //颜色等级 4+2 个
	uint16_t crack_leve[6];       //裂纹等级 4+2个
	uint16_t dirty_leve[5];       //脏蛋等级 3+2个
	uint16_t bood_leve;
	uint16_t print_id;         //打印编号
	uint16_t auto_clacure_weight[5];
	uint16_t task_complete_warn;  //任务完成时
	uint16_t packge_line;  // 按位表示一条线     低位为 1 高位 为6
	uint32_t task_box_count;     //任务盒数量
	uint32_t task_egg_count;     //任务数量
	uint16_t tray_row;   //  托盘行
	uint16_t tray_line;  //  托盘列
	uint16_t box_count;  //一箱多少盘鸡蛋

}pre_production_info;  // 预计生产信息


typedef struct
{

  uint16_t weight_leve[10];   //重量等级 8个 + 2个自定义
	uint16_t colour_leve[6];      //颜色等级 4+2 个
	uint16_t crack_leve[6];       //裂纹等级 4+2个
	uint16_t dirty_leve[5];       //脏蛋等级 3+2个
	uint16_t bood_leve;
	uint16_t abnormal_stop_count;	// 该抛弃通道设置的异常蛋数就停机值
	uint16_t abnormal_stop_time;	// 异常停机的时候停的分钟数？
	uint16_t print_id;
}throw_ch_product_info;  // 预计生产信息


typedef struct
{
  throw_ch_product_info cha[3];
	throw_ch_product_info chb[3];
	uint32_t force_throwa_stop_count;	// A线强抛设置的异常蛋数就停机值
	uint32_t force_throwa_stop_time;	// 异常停机的时候停的分钟数？
	uint32_t force_throwb_stop_count;
	uint32_t force_throwb_stop_time;
	uint32_t crc;
}throw_ch_product_type;  // 预计生产信息

typedef struct
{
	uint16_t empty;
	uint16_t prpduct_bar;      //生产进度条
	uint32_t pruduct_box_count;  //生产盒数量
	uint32_t pruduct_egg_count;  //生产数量
	uint32_t date;
	uint32_t start_time;
}alread_production_info;  //已经生产信息

typedef struct
{
	alread_production_info pruduct_info[TASK_PRUDUCTION_COUNT];
	uint32_t packge_ch_egg_count[SPT_CH_LINE+1];	
	throw_ch_eggs_type force_chA;
	throw_ch_eggs_type force_chB;
	uint32_t today_egg_count;
	uint32_t flag;
}alread_production_type;  //已经生产信息




typedef struct
{
	pre_production_info pre_production[TASK_PRUDUCTION_COUNT];
	uint16_t speed;            // 每小时多少个鸡蛋
	uint16_t egg_source[10];
	uint16_t egg_sourceid;
	uint32_t date;
	uint32_t crc;
}production_type;

typedef struct
{
	product_packge_info info[PRODUCT_PACKGE_INFO_COUNT];
	uint16_t count;
	uint16_t init;
	uint32_t crc;
}product_packge_type;

typedef struct
{
	product_attribute_info info[PRODUCT_ATTRIBUTE_INFO_COUNT];
	uint16_t count;
	uint16_t init;
	uint32_t crc;
}product_attribute_type;      //产品管理



typedef struct
{
	supplier_management_info info[SUPPLIER_MANGEMENT_COUNT];
	uint16_t count;
	uint16_t init;
	uint32_t crc;
}supplier_management_type;    //供应商管理

uint16_t get_pruduct_packge_count();

void get_product_packge_dat(product_packge_type *dat);
void crate_product_packge_info(product_packge_info *dat);
void edit_product_packge_info(uint8_t id,product_packge_info *dat);
void get_pruduct_packge_info(uint16_t id,product_packge_info *dat);
void delete_product_packge_info(uint16_t id);
	
void save_product_throw_ch_part(throw_ch_product_type *dat);
void read_product_throw_ch_part(throw_ch_product_type *dat);

uint16_t get_pruduct_attribute_count(void);
void get_product_attribute_dat(product_attribute_type *dat);
void crate_product_attribute_info(product_attribute_info *dat);
rt_err_t read_product_attribute_type(product_attribute_type *pa_type);
rt_err_t save_product_attribute_type(product_attribute_type *pa_type);
void get_pruduct_attribute_info(uint16_t id,product_attribute_info *dat);
void edit_product_attribute_info(uint8_t id,product_attribute_info *dat);
void delete_product_attribute_info(uint16_t index);
	
rt_err_t read_product_packge_type(product_packge_type *pk_type);

uint16_t get_supplier_count(void);
void get_supplier_dat(supplier_management_type *dat);
void crate_supplier_info(supplier_management_info *dat);
rt_err_t read_supplier_type(supplier_management_type *sm_type);
rt_err_t save_supplier_type(supplier_management_type *sm_type);
void get_supplier_info(uint16_t id,supplier_management_info *dat);
void edit_supplier_info(uint8_t id,supplier_management_info *dat);
void delete_supplier_info(uint8_t id);

void init_per_production_dat(void);
rt_err_t load_supplier_dat(void);           //加载供应商数据
rt_err_t load_product_packge_dat(void);
rt_err_t load_product_attribute_dat(void);
void get_pre_production_info(uint8_t id,pre_production_info *dat);
void save_pre_production_info(uint8_t id,pre_production_info *dat);
void get_production_mange_set(production_type *dat);
void save_production_mange_set(production_type *dat);
void delete_pre_production_info(uint8_t id);
uint16_t get_use_packge_line();
#endif  	// _PRODUCT_SET_H__
