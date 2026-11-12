
#ifndef _APP_RECODE_LOGS_H__
#define _APP_RECODE_LOGS_H__
#include <rtthread.h>


#define CFG_BKP_SRAM_SIZE 4096
#define RECODE_UP_PACK_OFFSET 0
#define RECODE_DOWN_PACK_OFFSET 100
#define RECODE_FEED_OFFSET 200
#define RECODE_CEGGS_OFFSET 300

#define FLASH_RECODE_PACK_ADDR 1*4096
#define FLASH_RECODE_PACK_MAX_SECTOR 7
#define FLASH_RECODE_FEED_ADDR 11*4096
#define FLASH_RECODE_FEED_MAX_SECTOR 7
#define FLASH_RECODE_CEGGS_ADDR 21*4096
#define FLASH_RECODE_CEGGS_MAX_SECTOR 7

#define FLASH_RECODE_SECTOR_START_ADDR 48          //扇区数据起始地址    
#define FLASH_RECODE_DATA_SIZE 16                  //每条数据大小
#define FLASH_RECODE_SECTOR_SAVE_TOTLE_NUMB 200    //每个扇区存储 200条记录
#define FLASH_RECODE_SECTOR_HEAD_ADDR 20           //数据头  记录当前扇区是否存储有数据  标记
#define FLASH_RECODE_SECTOR_HEAD_SIZE 20           //数据头长度
#define FLASH_RECODE_SECTOR_END_ADDR (4096-40)     //数据尾  记录当前扇区是否存满 标记
#define FLASH_RECODE_SECTOR_END_SIZE 20            //数据尾长度
#define FLASH_RECODE_DAT_FLAG  0X5C
#define SCREEN_DISP_MESG_NUMBER 9

typedef struct
{
	uint16_t crc;
	uint16_t year;         //日期 2021
	uint16_t runtime;      //分钟
	uint8_t hour;         //上料时间  8：58
	uint8_t min;
	uint8_t mon;
	uint8_t date;
	uint8_t mode;         //模式 手动  自动
	uint8_t carnumber;    //上下层喂料

}pack_recode;

typedef struct
{
	uint16_t crc;
	uint16_t year;         //日期 2021
	uint16_t runtime;      //分钟
	uint8_t hour;         //当前喂料时间 时
	uint8_t min;          ////当前喂料时间 分
	uint8_t mon;
	uint8_t date;          //日期
	uint8_t mode;         //模式 手动  自动
	uint8_t direction;    // 方向 正转/反转
}feed_recode;

typedef struct
{
	uint16_t runtime;      //分钟
	uint16_t crc;
	uint16_t year;         //日期 2021
	
	uint8_t hour;         //当前喂料时间 时
	uint8_t min;          ////当前喂料时间 分
	uint8_t mon;
	uint8_t date;          //日期
	uint8_t mode;         //模式 手动  自动

}ceggs_recode;

void check_pack_recode(void);
void check_feed_recode(void);
void check_ceggs_recode(void);
void hot_reg_clear_all_dat(void);

void recode_ceggs_to_hog_reg(ceggs_recode *dat);
void recode_feed_to_hog_reg(feed_recode *dat);
void recode_pack_to_hog_reg(pack_recode *dat);
void save_pack_recode(pack_recode *pack_log);
void save_feed_recode(feed_recode *feed_log);
void save_ceggs_recode(ceggs_recode *ceggs_log);
void get_recode_count( uint16_t  *count,uint8_t type);
void get_opage_recode_message(uint8_t *buf, uint16_t page,uint8_t type);

#endif
