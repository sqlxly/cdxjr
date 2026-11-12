#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <user_set.h>
#include "app_screen.h"
#include "screen_chat.h"
#include "modbus_screen.h"
#include "modbus_packge.h"
#include "modbus_transport.h"
#include "modbus_throw.h"
#include "move_control.h"
#include "product_set.h"
#include "product_set.h"
#include "device_alarm.h"
#include "modbus_packge.h"
#include "egg_control.h"
#include "ckeck_env.h"
#include "mdebug.h"
#include "E6B2.h"
#include "math.h"
#include "relay.h"
#include "egg_control.h"
#include "err_check.h"
#include "spt_egg.h"
#include "product_control.h"
#include "product_hostroy.h"
#include "common.h"
#include "udp_weight.h"
#include "product_egg_recode.h"

ALIGN(RT_ALIGN_SIZE)
// 刷新屏幕参数

#define SOFT_WARE 4000
static modbus_mapping_t *mb_reg;
static user_set_part_type reflash_dat;
static product_packge_type  disp_product_packge;
static product_attribute_type  disp_product_attribute;
static supplier_management_type disp_supplier_management; 
static production_type disp_production;                  //预计生产管理
static uint8_t spi_offest_addr=100;
void show_per_product_spi_addr_offest(uint8_t dat)
{

  spi_offest_addr = dat;
}
void show_soft_ware()
{
  mb_reg->tab_registers[1282] = SOFT_WARE;
}
void disp_debug_number1(uint16_t a)
{
  mb_reg->tab_registers[29] = a;
}
void disp_debug_number2(uint16_t a)
{
  mb_reg->tab_registers[30] = a;
}
void disp_debug_number3(uint16_t a)
{
  mb_reg->tab_registers[31] = a;
}

void disp_debug_srt(uint8_t *str,uint8_t len)
{
	rt_memcpy(&mb_reg->tab_registers[19],str,len);
}
void CopyFromMBHotReg(uint16_t addr,uint8_t *dat,uint16_t len)
{
  rt_memcpy(dat,&mb_reg->tab_registers[addr],len);
}
// 清空强制踢蛋信息
void clear_update_text_buf()
{
	rt_memset(&mb_reg->tab_registers[5999],0,200*2);
}
static uint32_t pluse_adjust_val=0;
void set_pluse_adjust_val(uint32_t val)
{
  pluse_adjust_val=val;
}
extern uint32_t spt_egg_count[2];
extern uint16_t throw_part_err_count;
extern uint16_t nospt_info[2];
extern uint16_t per_pruduct_err_count;
extern uint32_t printA_egg_count;
extern uint32_t printB_egg_count;
extern uint32_t totle_egg[2];
extern uint16_t recive_dirty_info_pluse[5];
extern uint16_t lose_info[2];
extern uint32_t tspt_info[2];
uint32_t print_count[2]={0,0};
// 显示强制踢蛋信息
void disp_force_egg_info(alread_production_type *dat)
{
	char buf[20];
	uint8_t i;
	uint32_t err_index;
	uint32_t up_index;
	uint32_t up_mesg_id;
	uint32_t cmesg_id;
	uint32_t BUF[2];
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A总计=%d",dat->force_chA.totle_count);
	rt_memcpy(&mb_reg->tab_registers[5999],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A没重量=%d",dat->force_chA.no_weight);
	rt_memcpy(&mb_reg->tab_registers[5999+10],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A未踢=%d",dat->force_chA.need_tick_count);
	rt_memcpy(&mb_reg->tab_registers[5999+20],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A未踢开=%d",dat->force_chA.tick_over_count);
	rt_memcpy(&mb_reg->tab_registers[5999+30],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A接收爪忙=%d",dat->force_chA.recive_busy_count);
	rt_memcpy(&mb_reg->tab_registers[5999+40],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"A脉冲溢出=%d",dat->force_chA.over_flow_count);
	rt_memcpy(&mb_reg->tab_registers[5999+50],buf,20);
	rt_memset(buf,0,20);
	
	
	
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B总计=%d",dat->force_chB.totle_count);
	rt_memcpy(&mb_reg->tab_registers[5999+60],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B没重量=%d",dat->force_chB.no_weight);
	rt_memcpy(&mb_reg->tab_registers[5999+70],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B未踢=%d",dat->force_chB.need_tick_count);
	rt_memcpy(&mb_reg->tab_registers[5999+80],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B未踢开=%d",dat->force_chB.tick_over_count);
	rt_memcpy(&mb_reg->tab_registers[5999+90],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B接收爪忙=%d",dat->force_chB.recive_busy_count);
	rt_memcpy(&mb_reg->tab_registers[5999+100],buf,20);
	
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"B脉冲溢出=%d",dat->force_chB.over_flow_count);
	rt_memcpy(&mb_reg->tab_registers[5999+110],buf,20);
	rt_memset(buf,0,20);
	
	
	rt_snprintf(buf,20,"SA=%d,SB=%d",tspt_info[0],tspt_info[1]);
	rt_memcpy(&mb_reg->tab_registers[5999+120],buf,20);
	
	
	  rt_memset(buf,0,20);
	  rt_snprintf(buf,20,"A喷印次数=%d",print_count[0]);
	  rt_memcpy(&mb_reg->tab_registers[5999+130],buf,20);
	  
	  rt_memset(buf,0,20);
	  rt_snprintf(buf,20,"B喷印次数=%d",print_count[1]);
	  rt_memcpy(&mb_reg->tab_registers[5999+140],buf,20);
	
	for(i=0;i<2;i++)
	{
	  rt_memset(buf,0,20);
	  rt_snprintf(buf,20,"没信息[%d]=%d",i,lose_info[i]);
	  rt_memcpy(&mb_reg->tab_registers[5999+150+(i*10)],buf,20);
	}
	for(i=0;i<2;i++)
	{
	  rt_memset(buf,0,20);
	  rt_snprintf(buf,20,"未分配[%d]=%d",i,nospt_info[i]);
	  rt_memcpy(&mb_reg->tab_registers[5999+170+(i*10)],buf,20);
	}
	rt_memset(buf,0,20);
	rt_snprintf(buf,20,"SDRAM=%d,%d",per_pruduct_err_count,throw_part_err_count);
	rt_memcpy(&mb_reg->tab_registers[5999+190],buf,20);
}
uint16_t ReadMBHotRegU16(uint16_t addr)
{
  uint16_t value;
	
	value = mb_reg->tab_registers[addr];
	return value;
}
uint32_t ReadMBHotRegU32(uint16_t addr)
{

	uint32_t val;

	val=mb_reg->tab_registers[addr+1]<<16;
	val +=mb_reg->tab_registers[addr];

	return val;
}
void WriteMBHotRegU16(uint16_t addr,uint16_t value)
{

	mb_reg->tab_registers[addr] = value;

}

extern uint8_t transport_egg_dirty_infoA[MAX_TRANSPORT_LINE_COUNT][6];
extern uint8_t transport_egg_dirty_infoB[MAX_TRANSPORT_LINE_COUNT][6];
extern uint16_t dirty_a_id[6];
extern uint16_t dirty_b_id[6];
// 设备管理->蛋杯状态界面显示
void shoe_spt_cup_egg_info()
{
	uint8_t id;
	separate_egg_info_type *spt_egg;
	// a线蛋杯状态界面显示，为什么空闲的时候才实时显示呢，是不是搞错了？
	if((get_current_screen_page()==13)&&(get_system_state()==0))
	{
		for(id=0;id<72;id++)
		{
		   get_pst_egg_info(0,id,&spt_egg,0);
		   if(spt_egg->empty==NEED_TICK_EGG)
		   {
						 mb_reg->tab_registers[7299+id] =1;  //蛋杯状态
						 mb_reg->tab_registers[7379+id] =spt_egg->wetght;
             mb_reg->tab_registers[7459+id] =spt_egg->crack;
             mb_reg->tab_registers[7539+id] =spt_egg->print_id;
             mb_reg->tab_registers[7619+id] =spt_egg->spt_ch_pos;
             mb_reg->tab_registers[7699+id] = spt_egg->separate_ch;			   
		   }
		   else  
		   {
						 mb_reg->tab_registers[7299+id] =0;  //蛋杯状态
						 mb_reg->tab_registers[7379+id] =0;
             mb_reg->tab_registers[7459+id] =spt_egg->crack;
             mb_reg->tab_registers[7539+id] =spt_egg->print_id;
             mb_reg->tab_registers[7619+id] =spt_egg->colour;
             mb_reg->tab_registers[7699+id] =0;		
		   }
		}
	}
	// b线蛋杯状态界面显示
	else if((get_current_screen_page()==14)&&(get_system_state()==0))
	{
		for(id=0;id<72;id++)
		{
		   get_pst_egg_info(1,id,&spt_egg,0);
		   if(spt_egg->empty==NEED_TICK_EGG)
		   {
						 mb_reg->tab_registers[7299+id] =1;  //蛋杯状态
						 mb_reg->tab_registers[7379+id] =spt_egg->wetght;
             mb_reg->tab_registers[7459+id] =spt_egg->crack;
             mb_reg->tab_registers[7539+id] =spt_egg->print_id;
             mb_reg->tab_registers[7619+id] =spt_egg->spt_ch_pos;
             mb_reg->tab_registers[7699+id] =spt_egg->separate_ch;			  
		   }
		   else  
		   {
						mb_reg->tab_registers[7299+id] =0;  //蛋杯状态
						mb_reg->tab_registers[7379+id] =0;
            mb_reg->tab_registers[7459+id] =spt_egg->crack;
            mb_reg->tab_registers[7539+id] =spt_egg->print_id;
            mb_reg->tab_registers[7619+id] =spt_egg->colour;
            mb_reg->tab_registers[7699+id] =0;		
		   }
		}
	}
	// 其他不为空闲的时候，蛋杯状态都为0吗？
	else if(get_current_screen_page()==13)
	{
		for(id=0;id<72;id++)
		{
					  mb_reg->tab_registers[7299+id] =0;  //蛋杯状态
						mb_reg->tab_registers[7379+id] =0;
            mb_reg->tab_registers[7459+id] =0;
            mb_reg->tab_registers[7539+id] =0;
            mb_reg->tab_registers[7619+id] =0;
            mb_reg->tab_registers[7699+id] =0;	
		}
	}
	else if(get_current_screen_page()==14)
	{
		for(id=0;id<72;id++)
		{
					  mb_reg->tab_registers[7299+id] =0;  //蛋杯状态
						mb_reg->tab_registers[7379+id] =0;
            mb_reg->tab_registers[7459+id] =0;
            mb_reg->tab_registers[7539+id] =0;
            mb_reg->tab_registers[7619+id] =0;
            mb_reg->tab_registers[7699+id] =0;	
		}
	}
	int lop=0;
	if(get_current_screen_page()==197)
	{
		for(id=0;id<6;id++)
		{
			for(lop=0;lop<6;lop++)
		 {
			 if(transport_egg_dirty_infoB[id][lop]<100)
			 {
				 mb_reg->tab_registers[2909+(id*6)+lop]=1;
			 }
			 else 
			 {
				 mb_reg->tab_registers[2909+(id*6)+lop]=0;
			 }
		 }
      
			mb_reg->tab_registers[2869+(id*6)+0] =transport_egg_dirty_infoB[id][0];  //蛋杯状态
			mb_reg->tab_registers[2869+(id*6)+1] =transport_egg_dirty_infoB[id][1];
      mb_reg->tab_registers[2869+(id*6)+2] =transport_egg_dirty_infoB[id][2];
      mb_reg->tab_registers[2869+(id*6)+3] =transport_egg_dirty_infoB[id][3];
      mb_reg->tab_registers[2869+(id*6)+4] =transport_egg_dirty_infoB[id][4];
      mb_reg->tab_registers[2869+(id*6)+5] =transport_egg_dirty_infoB[id][5];
		}
		 for(id=0;id<6;id++)
		{
			mb_reg->tab_registers[2949+id]=dirty_b_id[id];
		}
	}
	if(get_current_screen_page()==203)
	{
		for(id=0;id<6;id++)
		{

			for(lop=0;lop<6;lop++)
		  {
			 if(transport_egg_dirty_infoA[id][lop]<100)
			 {
				 mb_reg->tab_registers[2909+(id*6)+lop]=1;
			 }
			 else 
			 {
				 mb_reg->tab_registers[2909+(id*6)+lop]=0;
			 }
		 }
			mb_reg->tab_registers[2869+(id*6)+0] =transport_egg_dirty_infoA[id][0];  //蛋杯状态
			mb_reg->tab_registers[2869+(id*6)+1] =transport_egg_dirty_infoA[id][1];
      mb_reg->tab_registers[2869+(id*6)+2] =transport_egg_dirty_infoA[id][2];
      mb_reg->tab_registers[2869+(id*6)+3] =transport_egg_dirty_infoA[id][3];
      mb_reg->tab_registers[2869+(id*6)+4] =transport_egg_dirty_infoA[id][4];
      mb_reg->tab_registers[2869+(id*6)+5] =transport_egg_dirty_infoA[id][5];
		}
		for(id=0;id<6;id++)
		{
			mb_reg->tab_registers[2949+id]=dirty_a_id[id];
		}
	}
}

void WriteMBHotRegU32(uint16_t addr, uint32_t value)
{
	mb_reg->tab_registers[addr] = value&0xffff;
	mb_reg->tab_registers[addr+1] = (value>>16)&0xffff;
}
void CopyToMBHotReg(uint16_t addr,uint8_t *dat,uint16_t len)
{
  rt_memcpy(&mb_reg->tab_registers[addr],dat,len);
}

void disp_egg_quility()
{
	uint8_t i;
	uint32_t crack_leve1=0;
	uint32_t crack_leve2=0;
	uint32_t crack_leve3=0;
	uint32_t crack_leve4=0;
	
	uint32_t diryt_leve1=0;
	uint32_t diryt_leve2=0;
	uint32_t diryt_leve3=0;

  recode_analisys_product_type *dat;

  dat= rt_malloc(sizeof(recode_analisys_product_type));
	if(dat==RT_NULL)
	{
		return;
	}
	get_today_quity(dat);
	
	for(i=0;i<MAX_SUPPLY_COUNT;i++)
	{
	   crack_leve1+=dat->body[i].crack_leve[0].egg_count;
	   crack_leve2+=dat->body[i].crack_leve[1].egg_count;
	   crack_leve3+=dat->body[i].crack_leve[2].egg_count;
	   crack_leve4+=dat->body[i].crack_leve[3].egg_count;
	}
	
	for(i=0;i<MAX_SUPPLY_COUNT;i++)
	{
	   diryt_leve1+=dat->body[i].dity_leve[0].egg_count;
	   diryt_leve2+=dat->body[i].dity_leve[1].egg_count;
	   diryt_leve3+=dat->body[i].dity_leve[2].egg_count;

	}
    rt_free(dat);
	
	WriteMBHotRegU32(2251,crack_leve1);
	WriteMBHotRegU32(2253,crack_leve2);
	WriteMBHotRegU32(2255,crack_leve3);
	WriteMBHotRegU32(2257,crack_leve4);
	
	WriteMBHotRegU32(2261,diryt_leve1);
	WriteMBHotRegU32(2263,diryt_leve2);
	WriteMBHotRegU32(2265,diryt_leve3);
	


}
// 高级调试界面中脉冲偏移显示
void show_decive_offest()
{
	uint8_t i;
	
	device_offest_position device_position;
  get_senser_offest(&device_position);
  //rt_memcpy(&mb_reg->tab_registers[1287],&device_position,(sizeof(device_offest_position)-8));
	/*
	mb_reg->tab_registers[1309] = device_position.chA.line1;
	mb_reg->tab_registers[1310] = device_position.chA.line2;
	mb_reg->tab_registers[1311] = device_position.chA.line3;
	mb_reg->tab_registers[1312] = device_position.chA.line4;
	
	mb_reg->tab_registers[1315] = device_position.chA.weight_device;
	mb_reg->tab_registers[1316] = device_position.chA.ai_divice;
	mb_reg->tab_registers[1317] = device_position.chA.print_device;
	mb_reg->tab_registers[1318] = device_position.chA.blood_device;
	mb_reg->tab_registers[1319] = device_position.chA.first_cup_positon;
	mb_reg->tab_registers[1320] = device_position.chA.empty_cup_check;
	mb_reg->tab_registers[1321] = device_position.chA.cup_check;
	mb_reg->tab_registers[1322] = device_position.chA.totle_cup;
	mb_reg->tab_registers[1323] = device_position.chA.throw_line1;
	mb_reg->tab_registers[1324] = device_position.chA.throw_line2;
	mb_reg->tab_registers[1325] = device_position.chA.throw_line3;
	mb_reg->tab_registers[1326] = device_position.chA.no_use1;
	*/
	rt_memcpy(&mb_reg->tab_registers[1309],&device_position.chA,sizeof(offest_position));
	rt_memcpy(&mb_reg->tab_registers[1329],&device_position.chB,sizeof(offest_position));
	
}
// 包装信息显示
// 产品信息显示
// 预生产信息显示
// 供应商信息显示
void disp_product_set_info()
{
	uint8_t line[16];
	uint16_t j,i,page,index;
	mb_reg = mb_mapping;
	
	if(mb_reg->tab_registers[2400]<1) 
	{
		mb_reg->tab_registers[2400]=1;
	}
	if(mb_reg->tab_registers[1800]<1) 
	{
		mb_reg->tab_registers[1800]=1;
	}
	
	if(mb_reg->tab_registers[3400]<1) 
	{
		mb_reg->tab_registers[3400]=1;
	}
	
	page = mb_reg->tab_registers[2400]-1;
	index = page*13;
	get_supplier_dat(&disp_supplier_management);
	get_product_packge_dat(&disp_product_packge);
	get_product_attribute_dat(&disp_product_attribute);
	get_production_mange_set(&disp_production);
	mb_reg->tab_registers[2401]=(disp_product_packge.count/13)+((disp_product_packge.count%13)&&1);
	mb_reg->tab_registers[1801]=(disp_product_attribute.count/13)+((disp_product_attribute.count%13)&&1);
	mb_reg->tab_registers[3401]=(disp_supplier_management.count/13)+((disp_supplier_management.count%13)&&1);
	
	//预生产管理界面
	
	
	for(i=0;i<TASK_PRUDUCTION_COUNT;i++)
	{
		uint8_t k=0;
		rt_memset(line,0,sizeof(line));
		for(j=0;j<12;j++)
		{
			if(disp_production.pre_production[i].packge_line&(1<<j))
			{
				if((j%2)==0)
				{
					line[k*2]='A';
					line[k*2+1]='1'+j/2;
					k++;
				}
				else 
				{
				    line[k*2]='B';
					line[k*2+1]='1'+j/2;
					k++;
				}
			}
		}
		if(disp_production.pre_production[i].empty !=0)
		{
		   mb_reg->tab_registers[2829+i]=1;
		}
		else 
		{
		   mb_reg->tab_registers[2829+i]=0;
		}
		mb_reg->tab_registers[7820+(i*40)] = disp_production.pre_production[i].task_egg_count&0xffff;
		mb_reg->tab_registers[7820+(i*40+1)] = disp_production.pre_production[i].task_egg_count>>16;
		
		mb_reg->tab_registers[7816+(i*40)] = disp_production.pre_production[i].task_box_count&0xffff;
		mb_reg->tab_registers[7816+(i*40+1)] = disp_production.pre_production[i].task_box_count>>16;
		mb_reg->tab_registers[7815+(i*40)] = disp_production.pre_production[i].print_id;
		rt_memcpy(&mb_reg->tab_registers[7809+(i*40)],line,12);
		rt_memcpy(&mb_reg->tab_registers[7789+(i*40)],&disp_production.pre_production[i].product_name[0],20);
    rt_memcpy(&mb_reg->tab_registers[7799+(i*40)],&disp_production.pre_production[i].packge_name[0],20);
	}
	
	
	//产品包装页面
	for(i=index;i<(index+13);i++)
	{
		if(i<disp_product_packge.count)
		{
			// 包装管理页面显示所有包装信息
			mb_reg->tab_registers[2539+(i-index)] = disp_product_packge.info[i].row;
			mb_reg->tab_registers[2552+(i-index)] = disp_product_packge.info[i].line;
			mb_reg->tab_registers[2565+(i-index)] = disp_product_packge.info[i].auto_transmit_egg_carton_positon;
			mb_reg->tab_registers[2578+(i-index)] = disp_product_packge.info[i].check_egg_carton_position;
			mb_reg->tab_registers[2591+(i-index)] = disp_product_packge.info[i].packge_line1_postion;
			mb_reg->tab_registers[2604+(i-index)] = disp_product_packge.info[i].packge_line5_postion;
			mb_reg->tab_registers[2617+(i-index)] = disp_product_packge.info[i].recive_claw_fall_dowm_position;
			mb_reg->tab_registers[2630+(i-index)] = disp_product_packge.info[i].recive_claw_open_position;
			mb_reg->tab_registers[2643+(i-index)] = disp_product_packge.info[i].recive_claw_close_position;
			mb_reg->tab_registers[2656+(i-index)] = disp_product_packge.info[i].transport_start_position;
			// 生产产品编辑->包装盒选择页面显示所有包装信息
		  rt_memcpy(&mb_reg->tab_registers[2409+((i-index)*10)],&disp_product_packge.info[i],20);
		}
		else
		{
			// 包装管理页面显示所有包装信息
		  mb_reg->tab_registers[2539+(i-index)] = 0;
			mb_reg->tab_registers[2552+(i-index)] = 0;
			mb_reg->tab_registers[2565+(i-index)] = 0;
			mb_reg->tab_registers[2578+(i-index)] = 0;
			mb_reg->tab_registers[2591+(i-index)] = 0;
			mb_reg->tab_registers[2604+(i-index)] = 0;
			mb_reg->tab_registers[2617+(i-index)] = 0;
			mb_reg->tab_registers[2630+(i-index)] = 0;
			mb_reg->tab_registers[2643+(i-index)] = 0;
			mb_reg->tab_registers[2656+(i-index)] = 0;
			// 生产产品编辑->包装盒选择页面显示所有包装信息
		  rt_memcpy(&mb_reg->tab_registers[2409+((i-index)*10)],"       未设置       ",20);
		}			
	}
	//产品属性页面
	
	page = mb_reg->tab_registers[1800]-1;
	index = page*13;
	for(i=index;i<(index+13);i++)
	{
		if(i<disp_product_attribute.count)
		{
			
		//	mb_reg->tab_registers[2069+(i-index)] = disp_product_attribute.info[i].weight_leve;
		//	mb_reg->tab_registers[2082+(i-index)] = disp_product_attribute.info[i].colour_leve;
		//	mb_reg->tab_registers[2095+(i-index)] = disp_product_attribute.info[i].crack_leve;
		//	mb_reg->tab_registers[2108+(i-index)] = disp_product_attribute.info[i].dirty_leve;
			mb_reg->tab_registers[2121+(i-index)] = disp_product_attribute.info[i].print_id;
		  rt_memcpy(&mb_reg->tab_registers[1809+((i-index)*10)],&disp_product_attribute.info[i].product_name,20);	// 产品名称
			rt_memcpy(&mb_reg->tab_registers[1939+((i-index)*10)],&disp_product_attribute.info[i].packge_name,20);	// 产品包装
		}
		else
		{
		  mb_reg->tab_registers[2121+(i-index)] = 0;
		  rt_memcpy(&mb_reg->tab_registers[1809+((i-index)*10)],"       未设置       ",20);	// 产品名称
		  rt_memcpy(&mb_reg->tab_registers[1939+((i-index)*10)],"                    ",20);	// 产品包装
		}			
	}
	mb_reg->tab_registers[1498] = reflash_dat.weight_leve.unnormal_stop_count;
	rt_memcpy(&mb_reg->tab_registers[1499],&reflash_dat.weight_leve.leve2,9*2);     //鸡蛋等级
	
	
	//供应商管理页面
	
	page = mb_reg->tab_registers[3400]-1;
	index = page*13;
	for(i=index;i<(index+13);i++)
	{
		if(i<disp_supplier_management.count)
		{
		  rt_memcpy(&mb_reg->tab_registers[3409+((i-index)*10)],&disp_supplier_management.info[i],20);
		}
		else
		{
			rt_memcpy(&mb_reg->tab_registers[3409+((i-index)*10)],"       未设置       ",20);
		}			
	}
}
// 显示系统运行状态
void disp_system_state()
{

  uint8_t  state;
	uint16_t c_page;
	mb_reg = mb_mapping;
	
	state = get_system_state();
	if(state ==5)
		state = 3;
	else if(state ==8)
		state = 6;
	else if(state ==0)
	{
	  c_page = get_current_screen_page();
		
		if(((c_page>49)&&(c_page<63))||(c_page == 108) ||(c_page == 37))
		{
	     state = 7;
		}
		else 
		{
	     state =0;
		}
	}
	mb_reg->tab_registers[60] = state;
}
// 包装通道的io状态显示
void disp_packge_control_io_state()
{
	uint8_t base;
	uint16_t ch;
  uint32_t io_dat;
	uint32_t buf;
	uint32_t val;
	
  ch = ReadMBHotRegU16(SCREEN_PACKGE_CH_ADDR);
  io_dat = get_packge_io_state(ch/2);
	
	if((ch%2)==0)   //一共 24个IO口  0-11为 通道1  12-23为通道2
	{
	  base = 0;
	}
	else
	{
	  base = 12;
	}
	
	buf = 1<<(CH1_EGG_CUP_OPEN_SENSER_PIN+base);
	val = buf & io_dat;
	WriteMBHotRegU16(1230,val&&1);
	
	buf = 1<<(CH1_RECIVE_RESET_ORIGIN_PIN+base);
	val = buf & io_dat;
	WriteMBHotRegU16(1228,val&&1);
	
	buf = 1<<(CH1_CHECK_TRAY_PIN+base);
	val = buf & io_dat;
	WriteMBHotRegU16(1224,!(val&&1));
	
	buf = 1<<(CH1_PACKGE_RESET_ORIGIN_PIN+base);
	val = buf & io_dat;
	WriteMBHotRegU16(1225,val&&1);
}


void init_screen_mb_dat()
{
  mb_reg = mb_mapping;
}
void refresh_dat_to_screen()
{

	mb_reg = mb_mapping;
	uint32_t avg_pluse;
	uint32_t cup_pluse;
	
	get_user_set_part(&reflash_dat);
	mb_reg->tab_registers[193] = reflash_dat.voice.device_open;
	mb_reg->tab_registers[195] = reflash_dat.voice.device_fault;
	mb_reg->tab_registers[197] = reflash_dat.voice.device_stop;
	mb_reg->tab_registers[1018] = reflash_dat.transport.wash_delay;
	mb_reg->tab_registers[1021] = reflash_dat.transport.dry_delay;
	mb_reg->tab_registers[1023] = reflash_dat.transport.light_delay;
	mb_reg->tab_registers[1025] = reflash_dat.transport.uv_lamp_delay;
	mb_reg->tab_registers[1028] = reflash_dat.transport.absorb_oil_delay;
	mb_reg->tab_registers[1029] = reflash_dat.transport.print_oil_delay;
	
	mb_reg->tab_registers[1279] = reflash_dat.commun_part.tcp_alarm_count;
	mb_reg->tab_registers[1280] = reflash_dat.commun_part.reconnect_timeou_number;
	mb_reg->tab_registers[1281] = reflash_dat.commun_part.servo_moto_connect_number;
  mb_reg->tab_registers[1285] =  spi_offest_addr;
	
	
	rt_memcpy(&mb_reg->tab_registers[1529],&reflash_dat.colour_leve,8*2);  //显示颜色分级设置
	
  rt_memcpy(&mb_reg->tab_registers[1537],&reflash_dat.drty_leve,6*2);  //显示脏蛋分级设置
	
	rt_memcpy(&mb_reg->tab_registers[1543],&reflash_dat.crack_leve,8*2);  //裂纹分级设置
	
	
	mb_reg->tab_registers[799] = get_packge_ch_err_count(0);
	mb_reg->tab_registers[800] = get_packge_ch_err_count(1);
	mb_reg->tab_registers[801] = get_packge_ch_err_count(2);
	mb_reg->tab_registers[802] = get_packge_ch_err_count(3);
	mb_reg->tab_registers[803] = get_throw_ch_err_count(0);
	mb_reg->tab_registers[804] = get_throw_ch_err_count(1);
	
	mb_reg->tab_registers[1226] = get_packge_servo_offest(mb_reg->tab_registers[1188]);  //包装伺服电机位移
    mb_reg->tab_registers[1229] = get_recive_servo_offest(mb_reg->tab_registers[1188]);  //接收爪距离
	
	rt_memcpy(&mb_reg->tab_registers[999],&reflash_dat.transport.ch1_nb,16);
//	rt_memcpy(&mb_reg->tab_registers[499],&reflash_dat.use_mode,sizeof(mode_use_type));
	
	rt_memcpy(&mb_reg->tab_registers[479],&reflash_dat.device_use_info.oil_system,6*2);
	
	rt_memcpy(&mb_reg->tab_registers[489],&reflash_dat.device_use_info.chA,sizeof(mode_use_info));
	rt_memcpy(&mb_reg->tab_registers[509],&reflash_dat.device_use_info.chB,sizeof(mode_use_info));
	
	avg_pluse = get_spt_encode_pluse();							// 这个脉冲，分级线的脉冲值运行的时候一直再加
	cup_pluse =((SEPARATE_CHA_CUP_NUMBER/6)*1000);	// 分级线走一圈的脉冲数据
	avg_pluse = avg_pluse%cup_pluse;								// 得到当前脉冲在一圈的脉冲的位置
	mb_reg->tab_registers[827] = avg_pluse;
}


/*
*\刷新时间
*/
void refresh_real_time(void)
{
    rtc_info_t time;
	  mb_reg = mb_mapping;
    get_local_time(&time);
    mb_reg->tab_registers[67] = time.year+1900;
	  mb_reg->tab_registers[66] = time.month;
	  mb_reg->tab_registers[65] = time.date;
	  mb_reg->tab_registers[64] = time.hour;
	  mb_reg->tab_registers[63] = time.min;
	  mb_reg->tab_registers[62] = time.sec;
}

// 当每个蛋杯和每个电磁铁错误数大于设定的最大次数，则显示为黄色状态并报警
// 正常的时候不报警并显示未灰色状态
void check_electromagnet_and_egg_cup_alarm()
{ 
	 uint16_t i;
	 uint16_t *electromagnet_err_count;
	 uint16_t *egg_cug_err_count;
   mix_device_part mix_device;
	 uint16_t c_page;
	
	
	 electromagnet_err_count = get_electromagnet_err_count(0);
   get_mix_device_part(&mix_device);
	
	
	 c_page = get_current_screen_page();

   for(i=0;i<42;i++)
	 {
	   if(electromagnet_err_count[i]>=mix_device.tick_egg_moto_err_count)
		 {
			if(c_page == 253)      //显示电磁铁状态CHA
			{
			  WriteMBHotRegU16(4238 +i,1);
			}
		   set_device_alarm(TICK_EGG_MOTO_ERROR);
		 }
		 else 
		 {
			 if(c_page == 253)      //显示电磁铁状态CHA
			{
				WriteMBHotRegU16(4238 +i,0);
			}
		 }
	 }
	 electromagnet_err_count = get_electromagnet_err_count(1);
	 for(i=0;i<42;i++)
	 {
	   if(electromagnet_err_count[i]>=mix_device.tick_egg_moto_err_count)
		 {
			if(c_page == 256)      //显示电磁铁状态CHA
			{
			  WriteMBHotRegU16(4238 +i,1);
			}
		   set_device_alarm(TICK_EGG_MOTO_ERROR);
		 }
		 else 
		 {
			if(c_page == 256)      //显示电磁铁状态CHA
			{
				WriteMBHotRegU16(4238 +i,0);
			}
		 }
	 }
	 
	 egg_cug_err_count = get_cup_err_count(0);
	 for(i=0;i<270;i++)
	 {
	   if(egg_cug_err_count[i]>=mix_device.egg_cup_err_count)
		 {
			  if((c_page == 62) ||(c_page == 63)) 
				{
					WriteMBHotRegU16(6299 +i,1);
				}
		   set_device_alarm(EGG_CUP_ERR);
		 }
		 else 
		 {
		   if((c_page == 62) ||(c_page == 63)) 
				{
					WriteMBHotRegU16(6299 +i,0);
				}
		 }
	 }
	 
	 egg_cug_err_count = get_cup_err_count(1);
	 for(i=0;i<270;i++)
	 {
	   if(egg_cug_err_count[i]>=mix_device.egg_cup_err_count)
		 {
			  if((c_page == 65) ||(c_page == 66)) 
				{
					WriteMBHotRegU16(6299 +i,1);
				}
		   set_device_alarm(EGG_CUP_ERR);
		 }
		 else 
		 {
		   if((c_page == 65) ||(c_page == 66)) 
				{
					WriteMBHotRegU16(6299 +i,0);
				}
		 }
	 }
}

// 显示抛弃通道的设置
void show_throw_ch_set()
{
	
	 uint16_t *tab_reg;
	 throw_ch_product_type *throw_part;
	 
	 tab_reg = mb_mapping->tab_registers;
	 throw_part = malloc(sizeof(throw_ch_product_type));
	 if(throw_part == RT_NULL)
	 {
		  return;
	 }
	 read_product_throw_ch_part(throw_part);
	 
  
	 
	 rt_memcpy(&tab_reg[5600-1],&throw_part->cha[0].weight_leve,10*2);  //重量
	 rt_memcpy(&tab_reg[5640-1],&throw_part->cha[1].weight_leve,10*2);  //重量
	 rt_memcpy(&tab_reg[5680-1],&throw_part->cha[2].weight_leve,10*2);  //重量
	 
	 rt_memcpy(&tab_reg[5730-1],&throw_part->chb[0].weight_leve,10*2);  //重量
	 rt_memcpy(&tab_reg[5770-1],&throw_part->chb[1].weight_leve,10*2);  //重量
	 rt_memcpy(&tab_reg[5810-1],&throw_part->chb[2].weight_leve,10*2);  //重量
  
	 rt_memcpy(&tab_reg[5612-1],&throw_part->cha[0].colour_leve,4*2);  //颜色
	 rt_memcpy(&tab_reg[5652-1],&throw_part->cha[1].colour_leve,4*2);  //
	 rt_memcpy(&tab_reg[5692-1],&throw_part->cha[2].colour_leve,4*2);  //
	 
	 rt_memcpy(&tab_reg[5742-1],&throw_part->chb[0].colour_leve,4*2);  //颜色
	 rt_memcpy(&tab_reg[5782-1],&throw_part->chb[1].colour_leve,4*2);  //
	 rt_memcpy(&tab_reg[5822-1],&throw_part->chb[2].colour_leve,4*2);  //
	 
	 
	 rt_memcpy(&tab_reg[5618-1],&throw_part->cha[0].crack_leve,4*2);  //裂纹
	 rt_memcpy(&tab_reg[5658-1],&throw_part->cha[1].crack_leve,4*2);  //
	 rt_memcpy(&tab_reg[5698-1],&throw_part->cha[2].crack_leve,4*2);  //
	
	 rt_memcpy(&tab_reg[5748-1],&throw_part->chb[0].crack_leve,4*2);  //裂纹
	 rt_memcpy(&tab_reg[5788-1],&throw_part->chb[1].crack_leve,4*2);  //
	 rt_memcpy(&tab_reg[5828-1],&throw_part->chb[2].crack_leve,4*2);  //
	
	 rt_memcpy(&tab_reg[5624-1],&throw_part->cha[0].dirty_leve,3*2);  //脏蛋
	 rt_memcpy(&tab_reg[5664-1],&throw_part->cha[1].dirty_leve,3*2);  //
	 rt_memcpy(&tab_reg[5704-1],&throw_part->cha[2].dirty_leve,3*2);  //
	 
	 rt_memcpy(&tab_reg[5754-1],&throw_part->chb[0].dirty_leve,3*2);  //脏蛋
	 rt_memcpy(&tab_reg[5794-1],&throw_part->chb[1].dirty_leve,3*2);  //
	 rt_memcpy(&tab_reg[5834-1],&throw_part->chb[2].dirty_leve,3*2);  //
	 
	  tab_reg[5629-1] = throw_part->cha[0].bood_leve;
	  tab_reg[5629-1] = throw_part->cha[1].bood_leve ;
	  tab_reg[5709-1] = throw_part->cha[2].bood_leve ;
	 
	 tab_reg[5759-1] = throw_part->chb[0].bood_leve ;
	 tab_reg[5799-1] = throw_part->chb[1].bood_leve ;
	 tab_reg[5839-1] = throw_part->chb[2].bood_leve ;
	 
	 tab_reg[5630-1] = throw_part->cha[0].print_id ;
	 tab_reg[5670-1] = throw_part->cha[1].print_id ;
	 tab_reg[5710-1] = throw_part->cha[2].print_id ;
	 
	 tab_reg[5760-1] = throw_part->chb[0].print_id ;
	 tab_reg[5800-1] = throw_part->chb[1].print_id ;
	 tab_reg[5840-1] = throw_part->chb[2].print_id ;
	 
	 tab_reg[5631-1] = throw_part->cha[0].abnormal_stop_count ;
	 tab_reg[5671-1] = throw_part->cha[1].abnormal_stop_count ;
	 tab_reg[5711-1] = throw_part->cha[2].abnormal_stop_count ;
	 
	 tab_reg[5761-1] = throw_part->chb[0].abnormal_stop_count ;
	 tab_reg[5801-1] = throw_part->chb[1].abnormal_stop_count  ;
	 tab_reg[5841-1] = throw_part->chb[2].abnormal_stop_count ;
	 
	 
	 tab_reg[5632-1] = throw_part->cha[0].abnormal_stop_time ;// ;
	 tab_reg[5672-1] = throw_part->cha[1].abnormal_stop_time ;//tab_reg[1410];
	 tab_reg[5712-1] = throw_part->cha[2].abnormal_stop_time ;//tab_reg[1478];
	 
	 tab_reg[5762-1] = throw_part->chb[0].abnormal_stop_time;// tab_reg[1404];
	 tab_reg[5802-1] = throw_part->chb[1].abnormal_stop_time;//tab_reg[1410];
	 tab_reg[5842-1] = throw_part->chb[2].abnormal_stop_time;//tab_reg[1478];
	 
	 tab_reg[5713-1] =  throw_part->force_throwa_stop_count;
	 tab_reg[5714-1] =  throw_part->force_throwa_stop_time  ;//tab_reg[1412];
	 
	 tab_reg[5843-1] = throw_part->force_throwb_stop_count;
	 tab_reg[5844-1] = throw_part->force_throwb_stop_time  ;//tab_reg[1412];
	 
	 
	 rt_free(throw_part);
}
// 显示每个蛋杯和每个电磁铁错误计数
// 在屏幕刷新函数中调用
void disp_cup_and_electromagnet_err_count()
{
  uint8_t i,j;
	uint16_t *dat;
  uint16_t c_page;
	mb_reg = mb_mapping;
	
  c_page = get_current_screen_page();
	
	if(c_page == 253)      //显示电磁铁状态CHA
	{
		dat = get_electromagnet_err_count(0);
		rt_memcpy(&mb_reg->tab_registers[4199],dat,39*2);	// 但是屏幕上的地址是4239(4238)
	}
	if(c_page == 256)      //显示电磁铁状态CHB
	{
		dat = get_electromagnet_err_count(1);
		rt_memcpy(&mb_reg->tab_registers[4199],dat,39*2);	// b通道的电磁铁地址不能和a通道的一样吧
	}																										// 在这个界面，往这个地址写就是b通道的电磁铁
	
	if((c_page == 62)||(c_page == 63)) 
	{
	  dat = get_cup_err_count(0);
	  rt_memcpy(&mb_reg->tab_registers[6569],dat,270*2);// a通道的每个蛋杯的错误计数显示
	}
	if((c_page == 65)||(c_page == 66)) 
	{
	  dat = get_cup_err_count(1);
	  rt_memcpy(&mb_reg->tab_registers[6569],dat,270*2);// b通道的每个蛋杯的错误计数显示
	}
	
}
	

static uint8_t alarm_flash_flag=0;

extern alread_production_type alread_production;

// 清除每条线的箱数以及蛋数计数
void clear_product_egg()
{
	for(int j=0;j<12;j++)
	{		
		WriteMBHotRegU32((2689+(j*2)),0);
		mb_reg->tab_registers[2729+j] = 0;
	}
}
// 显示生产的蛋的数量
// 屏幕刷新函数会调用
void disp_product_egg_count()
{
	 uint8_t i,j;
	 uint32_t egg_count;
	 uint32_t ch_egg_count=0;
	 uint16_t packge_line;
	 production_type  *production;						// 预生产信息
   alread_production_type  already_product;	// 已经生产了的信息
	

   production = rt_malloc(sizeof(production_type));
	 if(production ==RT_NULL)
	 {
		return;
	 }
   rt_memset(production,0,sizeof(production_type));
	 get_production_mange_set(production);
   get_already_product_egg(&already_product);
	 WriteMBHotRegU32(2823,production->speed*2);	// 乘以2是因为有2条大线
	// WriteMBHotRegU32(2791,already_product.packge_ch_egg_count[8]);
	// WriteMBHotRegU32(2793,already_product.packge_ch_egg_count[7]);
	// WriteMBHotRegU32(2795,already_product.packge_ch_egg_count[6]);
	 
	 // 如果从其他板子(称重什么的)收到的和大于记录到的已生产的总但是
	  if((totle_egg[0]+totle_egg[1])>already_product.today_egg_count)
		{
			WriteMBHotRegU32(2747,already_product.today_egg_count);	// 这个是预计要生产的蛋数吗
			WriteMBHotRegU32(2723,totle_egg[0]+totle_egg[1]);				// 累计已经生产的总蛋数
		}
		else 
		{
		  WriteMBHotRegU32(2747,already_product.today_egg_count);
			WriteMBHotRegU32(2723,already_product.today_egg_count);
		
		}
	// WriteMBHotRegU32(2789,already_product.force_ch.totle_count);
	 
	 rt_memcpy(&mb_reg->tab_registers[3039],production->egg_source,20);	// d蛋品来源
	 packge_line = get_use_packge_line();
	 for(i=0;i<12;i++)
	 {
		 if(packge_line&(1<<i))
		 {
		 	 mb_reg->tab_registers[2669+packge_map_ch_id[i]] =1;	// 这个显示每条线是否使用
		 }
		 else 
		 {
		    mb_reg->tab_registers[2669+packge_map_ch_id[i]] =0; // 这个显示每条线是否使用
		 }
		// rt_kprintf("i=%d dat=%d\n",i,mb_reg->tab_registers[2679+i]);
	 }
	 for(i=0;i<12;i++)
	 {
		 if(production->pre_production[i].empty !=0 )
		 {
			 mb_reg->tab_registers[2829+ i] =1;
			 
			 mb_reg->tab_registers[6219+i]=production->pre_production[i].packge_id+1;
			 WriteMBHotRegU32(7818+(i*40),already_product.pruduct_info[i].pruduct_box_count);	// 已经生产的箱数
			 WriteMBHotRegU32(7822+(i*40),already_product.pruduct_info[i].pruduct_egg_count);	// 已经生产的枚数
			 mb_reg->tab_registers[7824 + i*40] = already_product.pruduct_info[i].prpduct_bar;// 生产进度	
			 for(j=0;j<12;j++)
		   {
		     if(production->pre_production[i].packge_line&(1<<j))
				 {
					 egg_count = get_packge_ch_alread_product_egg(packge_map_ch_id[j]);
					 WriteMBHotRegU32((2689+(packge_map_ch_id[j]*2)),egg_count);	// 该条线已经生产了多少枚蛋
					 mb_reg->tab_registers[2729+packge_map_ch_id[j]] = egg_count/(production->pre_production[i].tray_line * production->pre_production[i].tray_row); // 该条线已经生产了多少盒蛋
				 }
		   }
		  }
	 }
	 
	 ch_egg_count =0;
	 for(i=0;i<6;i++)
	 {
	   ch_egg_count +=get_packge_ch_alread_product_egg(packge_map_ch_id[i*2]);
	 
	 }
	 WriteMBHotRegU32(2849,ch_egg_count);			// A线大线生产了多少枚蛋
	 
	 ch_egg_count =0;
	 for(i=0;i<6;i++)
	 {
	   ch_egg_count +=get_packge_ch_alread_product_egg(packge_map_ch_id[(i*2)+1]);
	 
	 }
	 WriteMBHotRegU32(2851,ch_egg_count);			// B线大线生产了多少枚蛋
	 

   egg_count = get_packge_ch_alread_product_egg(12);
	 egg_count += get_packge_ch_alread_product_egg(13);
	 WriteMBHotRegU32(2713,egg_count);	// 抛弃通道1生产了多少枚
	 
	 egg_count = get_packge_ch_alread_product_egg(15);
	 egg_count += get_packge_ch_alread_product_egg(16);
     WriteMBHotRegU32(2715,egg_count);// 抛弃通道2生产了多少枚
	 
	 egg_count = alread_production.force_chA.totle_count;
	 WriteMBHotRegU32(2717,egg_count);	// 抛弃通道3生产了多少枚
	 
	 egg_count = alread_production.force_chB.totle_count;
	 WriteMBHotRegU32(2719,egg_count);	// 抛弃通道4生产了多少枚
	 rt_free(production);
}
// 显示伸缩杆到0和100位置的状态
// 如果0或者100位置的传感器检测到了，就显示1
// 屏幕刷新函数会调用
void show_senser_state()
{
  uint8_t status=0;
	
	status =ge_key_val(EXPONSION_LINK_TOP_SENSER);
	mb_reg->tab_registers[819]=(status==PRESS)? 1:0;
  
	status =ge_key_val(EXPONSION_LINK_BUTTOM_SENSER);
	mb_reg->tab_registers[820]=(status==PRESS)? 1:0;
	


}
// 显示某个报警
// 3820是报警事件等级表中的开始地址
void disp_alarm(uint8_t offset,uint8_t state)
{
	if(offset<ALARM_COUNT)
	{
		mb_reg->tab_registers[3819+offset]=state;
	}
}
// 主页显示每个小通道接收抓有无蛋状态
void disp_recive_cup_eggs()
{
	uint8_t i,j;
	uint8_t eggs;
	uint8_t use_flag;
	uint8_t d;
	uint16_t addr;
	mb_reg = mb_mapping;
    
  

	for(i=0;i<12;i++)   //
	{
		    addr = 2749 + (i*6);								// 主页显示每个小通道接收抓有无蛋状态
			eggs = get_packge_recive_cup_eggs(i);	// 获取每个小通道的接收抓有多少个蛋
			use_flag = get_packge_ch_config(i);		// 获取该小通道是否安装
			if(use_flag==0) eggs =0;
			for(j=0;j<6;j++)   //
			{
		      d = 1<<j;
			  if((eggs & d)>0)
			  {
			    mb_reg->tab_registers[addr +(j)] =1;
			  }
				else 
				{ 
				  mb_reg->tab_registers[addr +(j)] =0;
				}
			}
	}

}
void set_alarm_flag(uint8_t flag)
{
  alarm_flash_flag = flag;
}
// 报警图标刷新
void device_alarm_flash()
{
  if(alarm_flash_flag)
	{
		if(mb_reg->tab_registers[61]>0)
		{
			mb_reg->tab_registers[61]=0;
		}
		else 
		{
			mb_reg->tab_registers[61]=1;
		}
	}
	else 
	{
		mb_reg->tab_registers[61]=0;
	}
}


// 输送通道的io状态显示，有些没有写
void disp_transport_io()
{
	uint16_t i;
	uint16_t *base_addr;
  transport_io_dat *transport_io;
	

	transport_io = get_transport_io_info();


	 mb_reg->tab_registers[1014]=transport_io->output.gpio[AUTO_WASH_PUMP]&&1;
	 mb_reg->tab_registers[1015]=transport_io->output.gpio[WASH_BRUSH]&&1;
	 mb_reg->tab_registers[1016]=transport_io->output.gpio[WASH_MOTO]&&1;
	 mb_reg->tab_registers[1017]=transport_io->output.gpio[ADD_MEDICINE]&&1;
	
   mb_reg->tab_registers[1019]=transport_io->output.gpio[DRY_FAN]&&1;
	 mb_reg->tab_registers[1020]=transport_io->output.gpio[DRY_BRUSH]&&1;
	 mb_reg->tab_registers[1030]=transport_io->output.gpio[WASH_FAN]&&1;
	 mb_reg->tab_registers[1031]=transport_io->output.gpio[AUTO_WASH_MOTO]&&1;
	
	 mb_reg->tab_registers[1022]=transport_io->output.gpio[AUTO_WASH_BRUSH]&&1;
	 mb_reg->tab_registers[1024]=transport_io->output.gpio[UV_LAMP]&&1;
	
	 mb_reg->tab_registers[1026]=transport_io->output.gpio[OIL_PRINT]&&1;
	 mb_reg->tab_registers[1027]=transport_io->output.gpio[OIL_ABSORB]&&1;
}


extern uint32_t disp_dirty_id[20];
extern uint32_t disp_dirty_pluse[20];

// 显示脏蛋信息(脏蛋的id和脏蛋的脉冲值)
// 但是215页，脏蛋同步数据界面怎么跳转过去，在使用没有(找到了在颜色分级->同步信息里面)
// 下面屏幕刷新函数调用了
void show_dirty_info()
{
	uint8_t i=0;

	for(i=0;i<20;i++)
	{
	
	  mb_reg->tab_registers[6859+i]=disp_dirty_id[i];
	  mb_reg->tab_registers[6879+i]=disp_dirty_pluse[i];
	}
}

extern void disp_help_page();
static uint8_t time_count=0;
static rt_tick_t up_cycle_time=0;


void refresh_screen()
{
	uint32_t time=0;
	
	time = count_use_time(up_cycle_time);
    if(time>200)
	{
		up_cycle_time =  rt_tick_get();
		time_count++;
		refresh_real_time();
		if(time_count%3==0)
		{
			disp_egg_quility();
			show_soft_ware();
			disp_mix_device_part(&mb_reg->tab_registers[0]);
			disp_transport_io();
		}
		check_electromagnet_and_egg_cup_alarm();
		disp_system_state();
		show_senser_state();
		refresh_dat_to_screen();
		disp_product_set_info();
		device_alarm_flash();
		disp_product_egg_count();
		disp_recive_cup_eggs();
		disp_cup_and_electromagnet_err_count();
		disp_packge_control_io_state();
		shoe_spt_cup_egg_info();
		show_dirty_info();
		//disp_help_page();
	}

}
/*
void refresh_screen_dat(void *args)
{
	uint8_t time_count=0;
	rt_thread_mdelay(1000);
	show_soft_ware();
	disp_mix_device_part(&mb_reg->tab_registers[0]);
	while(1)
	{
		time_count++;
		refresh_real_time();
		if(time_count%5==0)
		{
		  disp_transport_io();
		}
		check_electromagnet_and_egg_cup_alarm();
		disp_system_state();
		show_senser_state();
		refresh_dat_to_screen();
		disp_product_set_info();
		device_alarm_flash();
		disp_product_egg_count();
		disp_recive_cup_eggs();
		disp_cup_and_electromagnet_err_count();
		disp_packge_control_io_state();
		rt_thread_mdelay(100);
	}
}
static int refresh_screen_thread(void)
{
    rt_thread_t  tid1;

		tid1 = rt_thread_create("rscreen",refresh_screen_dat, RT_NULL,2048,25, 20);
    if (tid1 == RT_NULL) return RT_ERROR;
		rt_thread_startup(tid1);
    return RT_EOK;
}
INIT_APP_EXPORT(refresh_screen_thread);
*/