#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <modbus.h>
#include "mdebug.h"
#include <mail_box.h>
#include "user_set.h"
#include "drv_key.h"
#include "vs1053.h"
#include "app_screen.h"
#include "modbus_screen.h"
#include "modbus_transport.h"
#include "screen_chat.h"
#include "product_set.h"
#include "move_control.h"
#include "relay.h"
#include "common.h"
#include "device_alarm.h"
#include "product_control.h"
#include "product_hostroy.h"
#include "egg_control.h"
#include "product_egg_recode.h"
#include "err_check.h"
#include "system_part.h"
#include "udp_weight.h"
#include "test_fun.h"

ALIGN(RT_ALIGN_SIZE)
#define PACKGE_BOX_LEN      355 //mm 毫米  包装通道每个格子长度
#define PACKGE_SERVO_CYCLE_SETP  20000 //伺服电机走多少个步为一个盒子长度

static user_set_part_type user_set_buf;

typedef enum
{
	SET_TIME=0,
	SET_TRANSPORT_PART,
	SET_USE_MODE,
	SET_MODE_EN,
	SET_VOICE,
	DEBUG_THROW_LINE,
	DEBUG_PACKGE_LINE,
	DEBUG_TRANSPORT_LINE,
	DEBUG_EXPANSION_LINK,
	DEBUG_PACKGE_BOX,
	DEBUG_MAST_CONTROL,
	CHANGE_PAGE,
	EDIT_PRODUCT_PACK_INFO,
	EDIT_PRODUCT_SPEED,
	EDIT_PRODUCT_ATTRIBUTE_INFO,
	EDIT_PER_PRODUCTION_INFO,
	EDIT_SUPPLIER_INFO,
	EDIT_DEVICE_POSITION,
	SELECT_WEIGHT_INFO,
	SET_EGG_LEVE_INFO,
	SET_COLOUR_LEVE_INFO,
	SET_DIRTY_LEVE_INFO,  //脏蛋
	SET_CRACK_LEVE_INFO,  //裂纹
	SIMPLE_FUN,
	SET_PRUDUCT_PACKGE,
	EDIT_PRODUCT_EGG_SPEED,
	SHOW_PRODUCT_HOSTROY,
	DISP_ANALISYS_QUILITY,
	SET_MIX_DEVICE_PART,
	FUNCTION_COUNT,
}mb_function_id;



static uint16_t packge_id=0;

static void set_packge_id(uint16_t id)
{
	packge_id = id;
}
static uint16_t get_packge_id( )
{
	return packge_id ;
}

uint16_t get_current_screen_page()
{
	return mb_mapping->tab_registers[59];
}

//获取处理该数据的函数
// 获取处理函数
uint8_t get_mb_write_reg_function(uint16_t addr)
{
	uint8_t i;
	
	if((addr>=82)&&(addr<=87))
	{
			return SET_TIME;
	}
	else if((addr==3561)||(addr==3563)||((addr>=4734)&&(addr<=4736)))
	{
			return DISP_ANALISYS_QUILITY;
	}
	else if((addr==1234)||(addr==1231))
	{
			return DEBUG_PACKGE_BOX;
	}
	else if((addr>=3190)&&(addr<=3197))
	{
			return EDIT_PRODUCT_SPEED;
	}
	else if(((addr>=1309)&&(addr<=1346))||((addr>=1305)&&(addr<=1308)))
	{
			return EDIT_DEVICE_POSITION;
	}
	
	
	else if(((addr>=999)&&(addr<=1006))||(addr==1018)||(addr==1021)||(addr==1023)||(addr==1025)||(addr==1028)||(addr==1029))         
	{
		return SET_TRANSPORT_PART;
	}
	else if(((addr>=823)&&(addr<=826))||((addr>=623)&&(addr<=636))||((addr>=821)&&(addr<=832)))
	{
			return DEBUG_THROW_LINE;
	}
	else if(((addr>=1235)&&(addr<=1239)) || ((addr>=1209)&&(addr<=1214)) || ((addr>=1222)&&(addr<=1223)) || ((addr>=1217)&&(addr<=1220)))
	{
			return	DEBUG_PACKGE_LINE;
	}
	else if((addr>=479)&&(addr<=521))
	{
			return	SET_USE_MODE;
	}
	else if((addr>=3799)&&(addr<=3810))
	{
			return	SET_MODE_EN;
	}
	else if((addr>=809)&&(addr<=818))
	{
			return	DEBUG_MAST_CONTROL;
	}
	else if((addr>=193)&&(addr<=198))
	{
			return SET_VOICE;
	}
	else if((addr>=1498)&&(addr<=1507))
	{
			return SET_EGG_LEVE_INFO;
	}
	else if((addr>=1529)&&(addr<=1536))
	{
			return SET_COLOUR_LEVE_INFO;
	}
	else if((addr>=1537)&&(addr<=1542))
	{
			return SET_DIRTY_LEVE_INFO;
	}
	else if((addr>=1543)&&(addr<=1550))
	{
			return SET_CRACK_LEVE_INFO;
	}
	else if(((addr>=1014)&&(addr<=1027))||((addr>=599)&&(addr<=608))||((addr>=1030)&&(addr<=1033)))
	{
			return DEBUG_TRANSPORT_LINE;
	}
	else if((addr==2399)||(addr == 1799)||(addr == 3399)||(addr == 2201)||(addr == 4405))
	{
			return SIMPLE_FUN;
	}
	else if((addr==1482)||(addr == 5637)||(addr == 5638)||(addr == 4198)||(addr == 3564))
	{
			return SIMPLE_FUN;
	}
	else if((addr==2744)||(addr == 4709)||(addr == 1284)||(addr == 1245))
	{
			return SIMPLE_FUN;
	}
	else if((addr==2745)||(addr == 2746)||(addr == 2407))
	{
			return SIMPLE_FUN;
	}
	else if((addr==3990)||(addr==1483))
	{
			return SIMPLE_FUN;
	}
	else if((addr==1404)||(addr == 1410)||(addr == 1478)||(addr == 1412))
	{
			return SIMPLE_FUN;
	}
	else if((addr>=1205)&&(addr<=1208))
	{
			return SIMPLE_FUN;
	}
	else if(((addr>=610)&&(addr<=622))||((addr>=1240)&&(addr<=1244)))
	{
			return DEBUG_PACKGE_LINE;
	}
	else if((addr>=1231)&&(addr<=1231))
	{
			return SET_PRUDUCT_PACKGE;
	}
	else if(addr==59)
	{
			return CHANGE_PAGE;
	}
	else if((addr==5584)||(addr==5585)||(addr==4996)||(addr==4997))
	{
		return SHOW_PRODUCT_HOSTROY;
	}
	else if(((addr>=3050)&&(addr<=3052))||(addr==2199)||((addr>=1615)&&(addr<=1625))||((addr>=1649)&&(addr<=1681)))
	{
			return EDIT_PER_PRODUCTION_INFO;
	}
	else if((addr>=1279)&&(addr<=1281))
	{
			return DEBUG_EXPANSION_LINK;
	}
	else if((addr==1259)||(addr==1260)||(addr==2402)||(addr==2200)||(addr==1216))   //2200 包装选择
	{
			return EDIT_PRODUCT_PACK_INFO;
	}
	else if((addr==1802)||(addr==1726)||(addr==1727))
	{
			return EDIT_PRODUCT_ATTRIBUTE_INFO;
	}
	else if((addr==3402)||(addr==3560)||(addr==3549))
	{
			return EDIT_SUPPLIER_INFO;
	}
	else if(addr==2823)
	{
			return EDIT_PRODUCT_EGG_SPEED;
	}
	else if((addr>=1516)&&(addr<=1523))
	{
			return SELECT_WEIGHT_INFO;
	}
	else if((addr==4195)||(addr==4197)||(addr==4400)||(addr==4401)||(addr==4407))
	{
			return SET_MIX_DEVICE_PART;
	}
	else 
	{
	  return FUNCTION_COUNT;
	}
}





// 发送信息到抛弃通道
void send_message_to_throw_device(uint8_t ch,uint8_t *buf)
{
	 rt_err_t result;			
		
	if(ch==0)
	{
	 result= rt_mq_send(&masseg_throw1, buf, MESSAGE_DAT_LEN);
	 if (result != RT_EOK)
   {
    // DEBUG_ERR("throw ch send_message ERR ch=%d\n");
   }
	}
	else 
	{
	  result= rt_mq_send(&masseg_throw2, buf, MESSAGE_DAT_LEN);
		if (result != RT_EOK)
		{
			// DEBUG_ERR("throw ch send_message ERR ch=%d\n");
		}
	}
}



// 分级线界面调试
// 52分级线调试
void debug_mast_control_moto(uint8_t key_val)
{
  uint16_t *reg_buf;
  uint16_t speed;
	
	set_debug_mode_index(DEBUG_MAIN_MOTO);
	reg_buf =(uint16_t *)&mb_mapping->tab_registers[0];
	if(reg_buf[810]>0)  //选中主电机
	{
		speed = reg_buf[815];   //速度寄存器,最大是99(0-99)
		if(speed>100)
		{
			speed =10;
			reg_buf[815]=speed;
		}
	  if(reg_buf[813]>0)  //点动
		{
		   debug_frequency_motor(0,(speed/3),key_val);  //限制为设置速度的 1/3;
		}
		if(reg_buf[814]>0)  //持续
		{
		   debug_frequency_motor(1,(speed/3),key_val);  //限制为设置速度的 1/3;
		}
	}
	else if(reg_buf[811]>0)  //选中推杆
	{
		 if(key_val>0)
		 {
			if(reg_buf[817]>0)				// 界面上伸长
			{
				expansion_link_up();
			}
			else if(reg_buf[818]>0)		// 界面上缩短
			{
		    expansion_link_down();
			}
		 }
		 else 
		 {
		   expansion_link_stop();
		 }
	}
}


// 包装通道的包装电机正反转调试
// 包装通道的接收电机上下调试
// 包装通道的蛋夹步进电机开合调试
void ext_key_debug_device(uint8_t key_val)
{

	 uint8_t message_buf[MESSAGE_DAT_LEN]; 
	 uint16_t *reg_buf;
	
	 reg_buf =(uint16_t *)&mb_mapping->tab_registers[0];

	 if(reg_buf[1218]>0)
	 {
				  
						message_buf[0]=2;      //传输线控制
						message_buf[1]=reg_buf[SCREEN_PACKGE_CH_ADDR]%2;      //蛋带通道通道				  0-1
						message_buf[2]= (key_val==PRESS) ? 1:0;      //启动/停止
				    if(reg_buf[1222]>0)
						{
							message_buf[3]=1;      //正转
							send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
						}
						else if(reg_buf[1223]>0)
						{
							message_buf[3]=2;      //反转
							send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
						}
						rt_kprintf("message_buf dat=%d\n",message_buf[3]);
		}
		else if(reg_buf[1219]>0)
		{
						message_buf[0]=3;      //传输线控制
						message_buf[1]=reg_buf[SCREEN_PACKGE_CH_ADDR]%2;      //蛋带通道通道				  0-1
						message_buf[2]= (key_val==PRESS) ? 1:0;      //启动/停止
				    if(reg_buf[1236]>0)
						{
							message_buf[3]=1;      //上升
							send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
						}
						else if(reg_buf[1237]>0)
						{
							message_buf[3]=2;      //下降
							send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
						}
		}
		else if(reg_buf[1220]>0)
		{
						message_buf[0]=4;      //传输线控制
						message_buf[1]=reg_buf[SCREEN_PACKGE_CH_ADDR]%2;      //蛋带通道通道				  0-1
				    if(reg_buf[1238]>0)
						{
							message_buf[2]=1;      //打开
							if(key_val==PRESS)
							{
								send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
							}
						}
						else if(reg_buf[1239]>0)
						{
							message_buf[2]=2;      //关闭
							if(key_val==PRESS)
							{
								send_message_to_packge_device(reg_buf[SCREEN_PACKGE_CH_ADDR],message_buf);     
							}
						}
			
	 }
}
// 发给包装通道设备清除报警信息
void start_clear_warn_alarm()
{
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	clear_warn_alarm();
		
	message_buf[0]=CLEAR_SLAVE_SERVO_ALARM;      //清除报警信息
	send_message_to_packge_device(1,message_buf);     
	send_message_to_packge_device(3,message_buf);     
	send_message_to_packge_device(5,message_buf);
    send_message_to_packge_device(7,message_buf); 
    send_message_to_packge_device(9,message_buf);
    send_message_to_packge_device(11,message_buf);	
}
// 确认报警信息
void simple_set(uint16_t *tab_registers,uint16_t addr )
{
	uint8_t i;
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	
	if(addr==1413)
	{
		for(i=0;i<8;i++)
		{
		  tab_registers[1508+i]=0;
		}
		if((tab_registers[1610]<9)&&(tab_registers[1610]>0))
		{
			tab_registers[(1507 + tab_registers[1610])] = 1;
		}
	}
	else if(addr==69)	// 报警确认，确认后就是要清除报警信息
	{
		clear_warn_alarm();
		message_buf[0]=CLEAR_SLAVE_SERVO_ALARM;      //清除报警信息
		send_message_to_packge_device(1,message_buf);     
	    send_message_to_packge_device(3,message_buf);     
		send_message_to_packge_device(5,message_buf);     
		send_message_to_packge_device(7,message_buf);   
		send_message_to_packge_device(9,message_buf); 
		send_message_to_packge_device(11,message_buf); 
		
		
		message_buf[0]=GET_SERVO_RUN_PART;      //清除报警信息
		send_message_to_packge_device(1,message_buf);     
	    send_message_to_packge_device(3,message_buf);     
		send_message_to_packge_device(5,message_buf);     
		send_message_to_packge_device(7,message_buf);   
		send_message_to_packge_device(9,message_buf); 
		send_message_to_packge_device(11,message_buf); 

	}
	else 
	{
		rt_kprintf("write addr=%d val=%d\n",addr,tab_registers[addr]);
		rt_kprintf("命令未识别\n");
	}
}

static void mb_clear_reg(uint16_t *tab_registers,uint16_t start,uint16_t stop)
{
	uint16_t i;
	for(i=start;i<(stop+1);i++)
	{
		tab_registers[i]=0;	
	}
}
// 输送线调试界面53的设置(设置运行时候输送线的运行参数)
void dispose_transport_line_setting(uint16_t *tab_registers,uint16_t addr )
{
	  get_user_set_part(&user_set_buf);
		rt_memcpy(&user_set_buf.transport,&tab_registers[999],8*2);
		user_set_buf.transport.wash_delay = tab_registers[1018];
		user_set_buf.transport.dry_delay = tab_registers[1021];
		user_set_buf.transport.light_delay = tab_registers[1023];
		user_set_buf.transport.uv_lamp_delay = tab_registers[1025];
		user_set_buf.transport.absorb_oil_delay = tab_registers[1028];
		user_set_buf.transport.print_oil_delay = tab_registers[1029];
		save_user_set_to_eep(&user_set_buf);
}
// 设备安装界面50的设置(设置哪些设备要安装，安装后(包装通道安装后就表示使能了，不用在单独使能)才能使能使用)
void dispose_use_mode_setting(uint16_t *tab_registers,uint16_t addr )
{
	  get_user_set_part(&user_set_buf);
		rt_memcpy(&user_set_buf.device_use_info.chA,&tab_registers[489],sizeof(mode_use_info));
	  rt_memcpy(&user_set_buf.device_use_info.chB,&tab_registers[509],sizeof(mode_use_info));
	  user_set_buf.device_use_info.oil_system = tab_registers[479];
	  user_set_buf.device_use_info.uv_lamp = tab_registers[480];
	  user_set_buf.device_use_info.human_eye_recognize = tab_registers[481];
	  user_set_buf.device_use_info.dry_system = tab_registers[482];
	  user_set_buf.device_use_info.auto_add_medicine = tab_registers[483];
	  user_set_buf.device_use_info.wash_system = tab_registers[484];
		save_user_set_to_eep(&user_set_buf);
}

// 运行管理界面250的设置(设置哪些设备使能)
void displsy_mode_en_setting(uint16_t *tab_registers,uint16_t addr )
{
	  get_user_set_part(&user_set_buf);
		rt_memcpy(&tab_registers[3799],&user_set_buf.mode_en,sizeof(mode_en_type));
}
// 获取界面的设备使能并保存在eeprom中
void dispose_mode_en_setting(uint16_t *tab_registers,uint16_t addr )
{
	  get_user_set_part(&user_set_buf);
		rt_memcpy(&user_set_buf.mode_en,&tab_registers[3799],sizeof(mode_en_type));
		save_user_set_to_eep(&user_set_buf);
	  displsy_mode_en_setting(tab_registers,addr);
}
//read_user_set_from_eep
uint8_t dispose_voice_setting(uint16_t *tab_registers,uint16_t addr )
{
	if((addr==194)||(addr==196)||(addr==198))
	{
	  set_play_music((tab_registers[addr-1]),0);
		return 1;
	}
	if((addr==193)||(addr==195)||(addr==197))
	{
		get_user_set_part(&user_set_buf);
		user_set_buf.voice.device_open=tab_registers[193];
		user_set_buf.voice.device_fault=tab_registers[195];
		user_set_buf.voice.device_stop=tab_registers[197];
		save_user_set_to_eep(&user_set_buf);
		return 1;
	}
	return 0;
}
// 产品质量分析
void disp_analisys_quility(uint16_t *tab_registers,uint16_t addr)
{
	 uint8_t mon;
	 uint8_t day;
	 uint16_t year;
	 uint16_t tmp;
   uint32_t buf;
	 rtc_info_t time;
	
  if(addr==3561)
	{
     buf = ReadMBHotRegU32(3561);
		 year= buf/10000;
		 mon = (buf%10000)/100;
		 day = (buf%100);
		 if((mon==0)||(mon>12))
		 {
		    get_local_time(&time);
			  buf = (time.year+1900)*10000;
		    buf = buf+ (time.month*100);
			  buf = buf + time.date;
			  WriteMBHotRegU32(3561,buf);  
			  DEBUG_INFO("date=%d\n",buf);
		 }
		 else 
		 {
		    switch(mon)
				{
					case 1:
				  case 3:
					case 5:
					case 7:
					case 8:
					case 10:
					case 12:
						if(day>31)
							day=31;
						break;
					case 4:
				  case 6:
					case 9:
					case 11:
						if(day>30)
							day=30;
						break;
				  case 2:
						if(year%4==0)
						{
						  if(day>29)
							day=29;
						}
						else 
						{
							if(day>28)
							day=28;
						}
						break;
				}
		 
		   
		    buf = year*10000;
		    buf = buf+ (mon*100);
			  buf = buf + day;
			  WriteMBHotRegU32(3561,buf);
        DEBUG_INFO("date=%d\n",buf);				
		 }
	}
	else if((addr==3563)&&(tab_registers[3563]==1))
	{
		buf = ReadMBHotRegU32(3561);
		buf = get_up_dat(buf,2);
	  WriteMBHotRegU32(3561,buf);
		show_analisys_quality();
	}

	else if((addr==3563)&&(tab_registers[3563]==2))
	{
		get_local_time(&time);
		buf = (time.year+1900)*10000;
		buf = buf+ (time.month*100);
		buf = buf + time.date;
		WriteMBHotRegU32(3561,buf);  
		show_analisys_quality();
	}
  else if((addr==3563)&&(tab_registers[3563]==3))
  {
		buf = ReadMBHotRegU32(3561);
		buf = get_up_dat(buf,1);
	  WriteMBHotRegU32(3561,buf);
		show_analisys_quality();
	}
	else if(addr==4736)
	{
		if(ReadMBHotRegU16(4738)==1)
		{
			  WriteMBHotRegU16(4737,1);
		}
		else 
		{
			tmp = ReadMBHotRegU16(4737);
		  WriteMBHotRegU16(4738,2);
			if(tmp==1)
				WriteMBHotRegU16(4737,2);
			else 
				WriteMBHotRegU16(4737,1);
		}
		disp_sypply_info();
	}
	else if(addr==4734)
	{
		WriteMBHotRegU16(4737,1);
		disp_sypply_info();
	}
	else if(addr==4735)
	{
		tmp = ReadMBHotRegU16(4737);
		buf = ReadMBHotRegU16(4735);  //id
		if(tmp>1)
		{
			set_disp_sypply_id((char *)&tab_registers[4729+(buf*10)]);
		}
		else 
		{
		  set_disp_sypply_id((char *)&tab_registers[4729+(buf*10)]);
		}
	}
}
// 产品历史记录
void disp_product_host_log(uint16_t *tab_registers,uint16_t addr)
{
	uint16_t mesg_count;
	uint16_t page_count;
	uint16_t i,index;
	uint16_t disp_offest;
	already_produced_log_type  *disp_buf;
	
	
	if((addr==4996)&&(tab_registers[4996]==1))
	{
		tab_registers[5585]=0;
		tab_registers[5586]=0;
	    product_logs_hostoy_factry_init();
	}
	
	mesg_count = get_product_hostroy_count();
	DEBUG_CARE("hostroy log count=%d\n",mesg_count);
	page_count = mesg_count/15;
	if(mesg_count%15 !=0)
	{
	  page_count++;
	}
	tab_registers[5586] = page_count;
	
	
	if(addr ==5584)
	{
		if(tab_registers[addr]==1)
		{
			tab_registers[5585]++;
			if(tab_registers[5585]>tab_registers[5586])
			{
		  tab_registers[5585] = tab_registers[5586];
			}
		}
	  else 
		{
		  if(tab_registers[5585]>1)
			{
				tab_registers[5585]--;
			}
		}
	}
	else if(addr ==4997)
	{
		if(mesg_count>0)
		{
			tab_registers[5585] = 1;
		}
		else 
		{
		  tab_registers[5585] = 0;
		}
	}
	else if(addr ==5585)
	{
	  if(tab_registers[5585]>tab_registers[5586])
		{
		  tab_registers[5585] = tab_registers[5586];
		}
	}
	if(tab_registers[5585]==0)  //没有数据 不显示
	{
		disp_offest =0;
	}
	else 
	{
		disp_offest = (tab_registers[5585]-1)*15;
	}
	
	if((disp_offest+15)<=mesg_count)  //显示几行控制
	{
		  tab_registers[4998] = 15;
	}
	else 
	{
		  tab_registers[4998] = mesg_count%15;
	}
	
	disp_buf = rt_malloc(sizeof(already_produced_log_type));
	if(disp_buf ==RT_NULL)
	{
	   return ;
	}
        uint16_t disp_index=0;
	    index = get_current_porduct_index()+1;
		for(i=1;i<16;i++)
		{
	
                
				disp_index = disp_offest+i;
			    if(disp_index>=EEP_ALREADY_PRODUCT_COUNT)
				{
				  rt_memset(disp_buf,0,sizeof(already_produced_log_type));
				}
				
				
				if(index>=disp_index)
				{
				   disp_index = index-i;
				}
				else 
				{
				   disp_index = EEP_ALREADY_PRODUCT_COUNT -(disp_index-index);
				}
				rt_kprintf("disp_index=%d index=%d i=%d\n",disp_index,index,i);
				
				if(tab_registers[5585]==0)  //没有数据 不显示
				{
					rt_memset(disp_buf,0,sizeof(already_produced_log_type));
				}
				else 
				{
					get_pruduct_hostroy_log((already_produced_log_type*)disp_buf,disp_index);
				}
				
				uint8_t j,k;

		    k=0;
				for(j=0;j<6;j++)
		    {
					 tab_registers[ (5013 +((i-1)*39) + j)] = 0;
		    }
		    for(j=0;j<8;j++)
		    {
			   if(disp_buf->paqckge_line &(1<<j))
			   {
					 tab_registers[ (5013 +((i-1)*39) + k)] = '1'+j;
                     k++;
			   }
		    }
				tab_registers[5029 +((i-1)*39)]= disp_buf->task_box &0x0000ffff;
				tab_registers[5030 +((i-1)*39)]= disp_buf->task_box >>16;
				
				tab_registers[5031 +((i-1)*39)]= disp_buf->product_box &0x0000ffff;
				tab_registers[5032 +((i-1)*39)]= disp_buf->product_box >>16;
				
				tab_registers[5033 +((i-1)*39)]= disp_buf->task_count &0x0000ffff;
				tab_registers[5034 +((i-1)*39)]= disp_buf->task_count >>16;
				
				tab_registers[5035 +((i-1)*39)]= disp_buf->product_count &0x0000ffff;
				tab_registers[5036 +((i-1)*39)]= disp_buf->product_count >>16;
				
				tab_registers[5037 +((i-1)*39)]= disp_buf->product_bar; 
				
				tab_registers[4999 +((i-1)*39)]=  disp_buf->date &0x0000ffff;
				tab_registers[5000 +((i-1)*39)]=  disp_buf->date >>16;
				
				tab_registers[5001 +((i-1)*39)]=  disp_buf->start_time;
				tab_registers[5002 +((i-1)*39)]=  disp_buf->stop_time;
				
				rt_memcpy(&tab_registers[5003 +((i-1)*39)],disp_buf->product_name,sizeof(disp_buf->product_name));
				rt_memcpy(&tab_registers[5019 +((i-1)*39)],disp_buf->packge_name,sizeof(disp_buf->packge_name));
		}
	rt_free(disp_buf);
	
}


 // 返回0 数据不属于此范围
// 输送线调试界面 调试
uint8_t dispose_debug_transport_lind(uint16_t *tab_registers,uint16_t addr )
{
  uint8_t val;
	val = tab_registers[addr]&&1;
	
	if((addr>=1014)&&(addr<=1017))               //清洗系统
	{
		if(addr==1014)
			set_transport_io(AUTO_WASH_PUMP,val);
		else if(addr==1015)
			set_transport_io(WASH_BRUSH,val);
		else if(addr==1016)
			set_transport_io(WASH_MOTO,val);
		else if(addr==1017)
			set_transport_io(ADD_MEDICINE,val);
		return 1;    
	}
	else if((addr>=1019)&&(addr<=1020))          //风干系统
	{
		if(addr==1019)
			set_transport_io(DRY_FAN,val);
		else if(addr==1020)
			set_transport_io(DRY_BRUSH,val);
    return 1;    
	}
	else if(addr==1030)          //风干系统
	{
		set_transport_io(WASH_FAN,val);
		return 1;    
	}
	else if(addr==1031)          //风干系统
	{
		set_transport_io(AUTO_WASH_MOTO,val);
		return 1;    
	}
	else if(addr==1032)          //风干系统
	{
		uint8_t stste;
		uint16_t speed;
		stste = get_system_state();
		if((stste==7)||(stste==0))
		{
		  if(tab_registers[addr]==1)
			{
			  set_transport_io(AUTO_WASH_BRUSH,1);
				set_transport_io(AUTO_WASH_PUMP,1);
				set_transport_io(AUTO_WASH_MOTO,1);
				set_transport_io(DRY_BRUSH,1);
				set_transport_io(WASH_MOTO,1);
				if(tab_registers[815]==0)  //选中主电机
				{
					tab_registers[815]=20;
					speed=20;
				}
				speed = tab_registers[815];   //速度寄存器
				if(speed>100)
				{
					speed =10;
					tab_registers[815]=speed;
				}
				debug_frequency_motor(1,(speed/3),1);  //限制最高速度的 1/3;
			}
			else
			{
				
				set_transport_io(DRY_BRUSH,0);
			  set_transport_io(AUTO_WASH_BRUSH,0);
				set_transport_io(AUTO_WASH_PUMP,0);
				set_transport_io(AUTO_WASH_MOTO,0);
				set_transport_io(WASH_MOTO,0);
				debug_frequency_motor(1,0,0);  //限制最高速度的 1/3;
			}
			return 1;   
		}			
	}
	else if((addr==1022)||(addr==1024))          //人工光检  杀菌系统
	{
		if(addr==1022)
		{
			//set_transport_io(LED_LIGHT,val);
			set_transport_io(AUTO_WASH_BRUSH,val);
		}
		else if(addr==1024)
			set_transport_io(UV_LAMP,val);
    return 1;    
	}
	else if((addr==1026)||(addr==1027))          //人工光检  杀菌系统
	{
		if(addr==1026)
			set_transport_io(OIL_PRINT,val);
		else if(addr==1027)
			set_transport_io(OIL_ABSORB,val);
    return 1;    
	}
	else if((addr>=599)||(addr<=608))          //人工光检  杀菌系统
	{
		DEBUG_INFO("改\n");
    return 1;    
	}

	return 0;
}
 // 返回0 数据不属于此范围
// 分级线界面调试抛弃通道
void dispose_debug_throw_line(uint16_t *tab_registers,uint16_t addr )
{
	uint8_t result;
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	rt_memset(message_buf,0,sizeof(message_buf));
	
	if((addr>=831)&&(addr<=832))	// 选中抛弃通道A或者B
	{
		tab_registers[831] =0;
		tab_registers[832] =0;
		tab_registers[addr] =1;			// 然后将选中的通道设置为1
	}
	else if((addr>=623)&&(addr<=629))
	{
		// 通道调试里面的抛弃通道的电磁阀调试
		// B通道电磁阀3	629	
		// B通道电磁阀2	628	
		// B通道电磁阀1	627
		// A通道电磁阀3	626	
		// A通道电磁阀2	625	
		// A通道电磁阀1	624	
		// message_buf[1]相当于这个字节存的是哪个电磁阀
		// 整个发送的是 命令	哪个电磁阀	该电磁阀的值	电磁阀持续动作
		// 但是程序的地址要比界面的地址少1，为什么程序的地址和界面的一样，是这个界面的调试没有用吗？？？？？？
	  if(addr==629)
		{
		   message_buf[1]=6+1;
		}
		else if((addr>=623)&&(addr<=628))
		{
			message_buf[1]=(addr-623)+1; 
		}
		
		message_buf[0]=1;      //命令
		message_buf[2]=tab_registers[addr];    // 开关
		message_buf[3] = ALWAYS_TYPE;
		send_message_to_throw_device(0,message_buf); 
	}
	else if((addr>=630)&&(addr<=636))	// 这个分支的地址，界面上没有搜索到
	{
	  if(addr==636)
		{
		   message_buf[1]=6+1;
		}
		else if((addr>=630)&&(addr<=635))
		{
			message_buf[1]=(addr-630)+1; 
		}
		
		message_buf[0]=1;      //命令
		message_buf[2]=tab_registers[addr];    // 开关
		message_buf[3] = ALWAYS_TYPE;
		send_message_to_throw_device(1,message_buf); 
	}
	else if((addr>=823)&&(addr<=826))
	{
		// 分级线调试里面的抛弃通道的电磁阀调试
		// 电磁阀1	824	
		// 电磁阀2	825	
		// 电磁阀3	826
		// 驱动电机	827	
		// 通道A		832	
		// 通道B		833	
		// message_buf[1]相当于这个字节存的是哪个电磁阀
		// 整个发送的是 命令	哪个电磁阀	该电磁阀的值	电磁阀持续动作
		
	  if(addr==826)						// 抛弃通道的驱动电机地址
		{
			message_buf[1]=6+1;		// 驱动电机只有1个，不分抛弃a或者b通道，编号为7
		}
		else 
		{
		  message_buf[1]=(addr-823)+1;      //A通道 823 824 825-->1 2 3  
			if(tab_registers[832]==1)					// 通道B的
			{
			  message_buf[1] = message_buf[1] +3;	// 通道B的地址都加了一个3，就是 4 5 6
			}
		}
	  message_buf[0]=1;      //命令
		message_buf[2]=tab_registers[addr];    // 开关
		message_buf[3] = ALWAYS_TYPE;
		send_message_to_throw_device(0,message_buf); 
	}
}


void set_rtc(uint16_t *tab_registers,uint16_t addr )
{
	rtc_info_t time;
	get_local_time(&time);


	//DEBUG_INFO("read time=%d-%d-%d   %d:%d-%d\n",time.year+1900,time.month,time.date,
	//time.hour,time.min,time.sec);

	//DEBUG_INFO("set time=%d-%d-%d   %d:%d-%d\n",tab_registers[87],tab_registers[86],tab_registers[85],
	//tab_registers[84],tab_registers[83],tab_registers[82]);
	if(((time.year+1900) !=tab_registers[87]) ||(time.month !=tab_registers[86]) ||(time.date !=tab_registers[85]))
	{
	  set_date(tab_registers[87], tab_registers[86],tab_registers[85]);
	 //DEBUG_INFO("set date\n");
	}
	if((time.hour !=tab_registers[84]) ||(time.min !=tab_registers[83]))
	{
	   set_time(tab_registers[84],tab_registers[83],tab_registers[82]); 
	   //DEBUG_INFO("set time\n");
	}
}

// 新增或者创建供应商信息
static void dispose_crate_supplier_info(uint16_t *tab_registers,uint16_t addr )   //新增或者修改确认键触发
{
	uint16_t index;
  supplier_management_info supplier_info;
	



	if((addr==3402)&&(tab_registers[3402]==0))                        //新建按钮
	{
		rt_memset(&tab_registers[3539],0,sizeof(supplier_management_info));
	}
	else if((addr==3402)&&(tab_registers[3402]>0))                        //编辑按钮
	{
		if(tab_registers[3400]>0)
			index = (tab_registers[3400]-1)*13;
		else 
			index =0;
		if(tab_registers[3402]>0)           //编辑新增键
		{
			index += (tab_registers[3402]-1);
			get_supplier_info(index,&supplier_info);
		}
		else 
		{
			rt_memset(&supplier_info,0,sizeof(supplier_management_info));
		}
		rt_kprintf("supplier_management_info id=%d name=%s\n",index,supplier_info.name);
		rt_memcpy(&tab_registers[3539],&supplier_info,sizeof(supplier_management_info));
	}
	else if((addr==3549) &&(tab_registers[3549]==1))                              //保存界面
	{
			rt_memcpy(&supplier_info,(uint8_t*)&tab_registers[3539],sizeof(supplier_management_info));
			if(tab_registers[3402]==0)
			{
				crate_supplier_info(&supplier_info);
			}
			else
			{
				if(tab_registers[3400]>0)
				{
					index = (tab_registers[3400]-1)*13+tab_registers[3402];
				}
				else 
				{
					index = tab_registers[3402];
				}
				edit_supplier_info(index,&supplier_info);
			}
	}
	else if((addr==3560) &&(tab_registers[3560]==1))                       //删除
	{
		
		if(tab_registers[3400]>0)
			index = (tab_registers[3400]-1)*13;
		else 
			index =0;
		if(tab_registers[3403]>0)           //编辑新增键
		{
			index += (tab_registers[3403]);
			delete_supplier_info(index);
		}
	}
}
static uint16_t pruduct_packge_id=0;

// 显示包装编辑页面
void disp_screen_packge_set_dat(product_packge_info *dat,uint16_t *tab_registers)
{
   rt_memcpy(&tab_registers[1189],&dat->name[0],20);
	 tab_registers[1199] = dat->row;
	 tab_registers[1200] = dat->line;
	 tab_registers[1201] = dat->auto_transmit_egg_carton_positon;
	 tab_registers[1202] = dat->check_egg_carton_position;
	 tab_registers[1240] = dat->packge_line1_postion;
	 tab_registers[1241] = dat->packge_line2_postion;
	 tab_registers[1242] = dat->packge_line3_postion;
	 tab_registers[1243] = dat->packge_line4_postion;
	 tab_registers[1244] = dat->packge_line5_postion;

	 tab_registers[1205] = dat->recive_claw_fall_dowm_position;
	 tab_registers[1206] = dat->recive_claw_open_position;
	 tab_registers[1207] = dat->recive_claw_close_position;
	 tab_registers[1208] = dat->transport_start_position;
	 if(dat->use_box_count==0) dat->use_box_count =1;
	 tab_registers[1245] = dat->use_box_count;
}
// 将包装编辑页面的数据存下来
void copy_screen_packge_set_dat(product_packge_info *dat,uint16_t *tab_registers)
{
   rt_memcpy(&dat->name[0],&tab_registers[1189],20);
	 dat->row = tab_registers[1199];
	 dat->line = tab_registers[1200];
	 dat->auto_transmit_egg_carton_positon = tab_registers[1201];
	 dat->check_egg_carton_position = tab_registers[1202];
	 dat->packge_line1_postion = tab_registers[1240];
	 dat->packge_line2_postion = tab_registers[1241];
	 dat->packge_line3_postion = tab_registers[1242];
	 dat->packge_line4_postion = tab_registers[1243];
	 dat->packge_line5_postion = tab_registers[1244];
   dat->use_box_count = tab_registers[1245] ;
	 dat->recive_claw_fall_dowm_position = tab_registers[1205];
	 dat->recive_claw_open_position = tab_registers[1206];
	 dat->recive_claw_close_position = tab_registers[1207];
	 dat->transport_start_position = tab_registers[1208];
}
// 包装新增或者修改
void dispose_crate_prodict_packge_info(uint16_t *tab_registers,uint16_t addr )   //新增或者修改确认键触发
{
	
	uint16_t idex;
  product_packge_info packge_info;
	
	if((addr==1259)&&(tab_registers[1259]==0)) return;  //取消操作

	if(addr==2200)
	{

		
			uint16_t index;
			product_packge_info info;
			index = (tab_registers[2400]-1)*13;
			if(tab_registers[2200]>0)
			{
				index += (tab_registers[2200]-1);
			}
			get_pruduct_packge_info(index,&info);
			rt_kprintf("edit id=%d name=%s\n",index,info.name);
			if(tab_registers[1187]==1)  //调试界面的选择按钮
			{
				tab_registers[1232] = 0;
				tab_registers[1233] = 0;
				//rt_memcpy(&tab_registers[1189],&info,sizeof(product_packge_info));
				disp_screen_packge_set_dat(&info,tab_registers);
			}
			else if(tab_registers[1187]==2)  //产品管理 选择包装界面
			{
				set_packge_id(index);
				rt_memcpy(&tab_registers[1709],&info.name,10);
			}
			else if(tab_registers[1187]==3)  //预生产 选择包装界面
			{
				pruduct_packge_id = index;
				rt_memcpy(&tab_registers[1599],&info.name,20);
			}
		
	}
	else if(addr==2402)
	{
		
		product_packge_info info;
		idex = (tab_registers[2400]-1)*13;
		if(tab_registers[2402]>0)
		{
			idex += (tab_registers[2402]-1);
			get_pruduct_packge_info(idex,&info);
		}
		else 
		{
			get_pruduct_packge_info(0,&info);
			//rt_memset(&info,0,sizeof(product_packge_info));
		}
		rt_kprintf("edit id=%d name=%s\n",idex,info.name);
		//rt_memcpy(&tab_registers[1189],&info,sizeof(product_packge_info));
		disp_screen_packge_set_dat(&info,tab_registers);
	}
	else if(addr==1259)
	{
			//rt_memcpy(&packge_info,(uint8_t*)&tab_registers[1189],sizeof(product_packge_info));
		  copy_screen_packge_set_dat(&packge_info,tab_registers);
		  if(tab_registers[59]==57)  //当前属于那个页面   按照调试界面
			{
				if(tab_registers[2200]==0)
				{
					crate_product_packge_info(&packge_info);
				}
				else 
				{
					if(tab_registers[2400]>0)
					{
						idex = (tab_registers[2400]-1)*13+tab_registers[2200];
					}
					else 
					{
						idex = tab_registers[2200];
					}
					edit_product_packge_info(idex,&packge_info);
				}
			}
			else if(tab_registers[59]==108)  //当前属于那个页面   新建包装界面
			{
				if(tab_registers[2402]==0)
				{
					crate_product_packge_info(&packge_info);
				}
				else
				{
					
					if(tab_registers[2400]>0)
					{
						idex = (tab_registers[2400]-1)*13+tab_registers[2402];
					}
					else 
					{
						idex = tab_registers[2402];
					}
					edit_product_packge_info(idex,&packge_info);
				}
			}
	}
	else if(addr==1186)
	{
		product_packge_info info;
		tab_registers[2200] =0;
		rt_memset(&info,0,sizeof(product_packge_info));
		//rt_memcpy(&tab_registers[1189],&info,sizeof(product_packge_info));
		disp_screen_packge_set_dat(&info,tab_registers);
	}
	else if((addr==1260)&&(tab_registers[1260]==1))
	{
		idex = (tab_registers[2400]-1)*13;
		idex += tab_registers[2403];
    delete_product_packge_info(idex);
	}
}


// 设置重量等级
static void dispose_set_egg_leve_info(uint16_t *tab_registers,uint16_t addr)  
{
  uint16_t i;
	if((addr>=1498)&&(addr<=1507))
	{
		for(i=1507;i>1499;i--)
		{
			if(tab_registers[i-1]<tab_registers[i])	
			{
			  tab_registers[i-1] = tab_registers[i];
			}
			rt_kprintf("weight i=1=%d  i=%d\n", tab_registers[i-1],tab_registers[i]);
		}
	  get_user_set_part(&user_set_buf);
		user_set_buf.weight_leve.leve1=1000;
		user_set_buf.weight_leve.unnormal_stop_count = tab_registers[1498];
		rt_memcpy(&user_set_buf.weight_leve.leve2,&tab_registers[1499],9*2);
		save_user_set_to_eep(&user_set_buf);
	}
}
// 设置颜色等级
static void dispose_set_colour_leve_info(uint16_t *tab_registers,uint16_t addr)  
{
  uint16_t i;
	uint16_t base;
	if((addr>=1529)&&(addr<=1536))
	{
		for(i=0;i<4;i++)
		{
			base = 1529+i*2;
			if(tab_registers[base]>tab_registers[base+1])	
			{
			  tab_registers[base+1] = tab_registers[base];
			}
		}
	  get_user_set_part(&user_set_buf);

		rt_memcpy(&user_set_buf.colour_leve,&tab_registers[1529],8*2);
		save_user_set_to_eep(&user_set_buf);
	}
}
// 设置脏蛋等级
static void dispose_set_dirty_leve_info(uint16_t *tab_registers,uint16_t addr)  
{
  uint16_t i;
	uint16_t base;
	if((addr>=1537)&&(addr<=1542))
	{
		for(i=0;i<3;i++)
		{
			base = 1537+i*2;
			if(tab_registers[base]>tab_registers[base+1])	
			{
			  tab_registers[base+1] = tab_registers[base];
			}
		}
	  get_user_set_part(&user_set_buf);

		rt_memcpy(&user_set_buf.drty_leve,&tab_registers[1537],6*2);
		save_user_set_to_eep(&user_set_buf);
	}
}
// 设置裂纹等级
static void dispose_set_crack_leve_info(uint16_t *tab_registers,uint16_t addr)  
{
  uint16_t i;
	uint16_t base;
	if((addr>=1543)&&(addr<=1550))
	{
		for(i=0;i<4;i++)
		{
			base = 1543+i*2;
			if(tab_registers[base]>tab_registers[base+1])	
			{
			  tab_registers[base+1] = tab_registers[base];
			}
		}
	  get_user_set_part(&user_set_buf);

		rt_memcpy(&user_set_buf.crack_leve,&tab_registers[1543],8*2);
		save_user_set_to_eep(&user_set_buf);
	}
}
// 抛弃通道设置
void dispose_throw_ch_set()
{
	 uint16_t *tab_reg;
	 rt_base_t level;
	 throw_ch_product_type *throw_part;
	 
	 throw_part = malloc(sizeof(throw_ch_product_type));
	
	 if(throw_part == RT_NULL)
	 {
		  return;
	 }
	 rt_memset(throw_part,0,sizeof(throw_ch_product_type));
	 level = rt_hw_interrupt_disable();
	 tab_reg = mb_mapping->tab_registers;
     rt_memcpy(&throw_part->cha[0].weight_leve,&tab_reg[5600-1],10*2);  //重量
	 rt_memcpy(&throw_part->cha[1].weight_leve,&tab_reg[5640-1],10*2);  //重量
	 rt_memcpy(&throw_part->cha[2].weight_leve,&tab_reg[5680-1],10*2);  //重量
	 
	 rt_memcpy(&throw_part->cha[0].colour_leve,&tab_reg[5612-1],4*2);  //颜色
	 rt_memcpy(&throw_part->cha[1].colour_leve,&tab_reg[5652-1],4*2);  //
	 rt_memcpy(&throw_part->cha[2].colour_leve,&tab_reg[5692-1],4*2);  //
	 
	 rt_memcpy(&throw_part->cha[0].crack_leve,&tab_reg[5618-1],4*2);  //裂纹
	 rt_memcpy(&throw_part->cha[1].crack_leve,&tab_reg[5658-1],4*2);  //
	 rt_memcpy(&throw_part->cha[2].crack_leve,&tab_reg[5698-1],4*2);  //
	
	 rt_memcpy(&throw_part->cha[0].dirty_leve,&tab_reg[5624-1],3*2);  //脏蛋
	 rt_memcpy(&throw_part->cha[1].dirty_leve,&tab_reg[5664-1],3*2);  //
	 rt_memcpy(&throw_part->cha[2].dirty_leve,&tab_reg[5704-1],3*2);  //
	 
	 throw_part->cha[0].bood_leve = tab_reg[5629-1];
	 throw_part->cha[1].bood_leve = tab_reg[5669-1];
	 throw_part->cha[2].bood_leve = tab_reg[5709-1];
	 
	 throw_part->cha[0].print_id = tab_reg[5630-1];
	 throw_part->cha[1].print_id = tab_reg[5670-1];
	 throw_part->cha[2].print_id = tab_reg[5710-1];
	 
	 throw_part->cha[0].abnormal_stop_count = tab_reg[5631-1];
	 throw_part->cha[1].abnormal_stop_count = tab_reg[5671-1];
	 throw_part->cha[2].abnormal_stop_count = tab_reg[5711-1];
	 
	 throw_part->cha[0].abnormal_stop_time = 1;// tab_reg[1404];
	 throw_part->cha[1].abnormal_stop_time = 1;//tab_reg[1410];
	 throw_part->cha[2].abnormal_stop_time = 1;//tab_reg[1478];
	 
	 throw_part->force_throwa_stop_count =    tab_reg[5713-1];
	 throw_part->force_throwa_stop_time  =    1;//tab_reg[1412];
	 
	 
	 rt_memcpy(&throw_part->chb[0].weight_leve,&tab_reg[5730-1],10*2);  //重量
	 rt_memcpy(&throw_part->chb[1].weight_leve,&tab_reg[5770-1],10*2);  //重量
	 rt_memcpy(&throw_part->chb[2].weight_leve,&tab_reg[5810-1],10*2);  //重量
	 
	 
	 rt_memcpy(&throw_part->chb[0].colour_leve,&tab_reg[5742-1],4*2);  //颜色
	 rt_memcpy(&throw_part->chb[1].colour_leve,&tab_reg[5782-1],4*2);  //
	 rt_memcpy(&throw_part->chb[2].colour_leve,&tab_reg[5822-1],4*2);  //
	 
	 
	
	 
	 
	 rt_memcpy(&throw_part->chb[0].crack_leve,&tab_reg[5748-1],4*2);  //裂纹
	 rt_memcpy(&throw_part->chb[1].crack_leve,&tab_reg[5788-1],4*2);  //
	 rt_memcpy(&throw_part->chb[2].crack_leve,&tab_reg[5828-1],4*2);  //
	 
	  rt_memcpy(&throw_part->chb[0].dirty_leve,&tab_reg[5754-1],3*2);  //脏蛋
	 rt_memcpy(&throw_part->chb[1].dirty_leve,&tab_reg[5794-1],3*2);  //
	 rt_memcpy(&throw_part->chb[2].dirty_leve,&tab_reg[5834-1],3*2);  //
	 
	 throw_part->chb[0].bood_leve = tab_reg[5759-1];
	 throw_part->chb[1].bood_leve = tab_reg[5799-1];
	 throw_part->chb[2].bood_leve = tab_reg[5839-1];
	 
	 throw_part->chb[0].print_id = tab_reg[5760-1];
	 throw_part->chb[1].print_id = tab_reg[5800-1];
	 throw_part->chb[2].print_id = tab_reg[5840-1];
	 
	 throw_part->chb[0].abnormal_stop_count = tab_reg[5761-1];
	 throw_part->chb[1].abnormal_stop_count = tab_reg[5801-1];
	 throw_part->chb[2].abnormal_stop_count = tab_reg[5841-1];

	 throw_part->chb[0].abnormal_stop_time = 1;// tab_reg[1404];
	 throw_part->chb[1].abnormal_stop_time = 1;//tab_reg[1410];
	 throw_part->chb[2].abnormal_stop_time = 1;//tab_reg[1478];
	 
	
	 
	 throw_part->force_throwb_stop_count =    tab_reg[5843-1];
	 throw_part->force_throwb_stop_time  =    1;//tab_reg[1412];
	 
	 rt_hw_interrupt_enable(level);
	 save_product_throw_ch_part(throw_part);
	 rt_free(throw_part);
}

// 设置已经使用的包装线
void lock_packge_line(uint16_t *tab_registers)
{
	uint8_t i;
	
	uint8_t index;
	uint16_t line=0;
	uint16_t *addra;
	uint16_t *addrb;
	user_set_part_type user_set;
    pre_production_info pre_production;
	index = tab_registers[3052];
	
	get_user_set_part(&user_set);
	
	for(i=0;i<TASK_PRUDUCTION_COUNT;i++)
	{
	   rt_memset(&pre_production,0,sizeof(pre_production_info));
	   get_pre_production_info(i,&pre_production);
	   if(pre_production.empty>0)	// 如果某个预生产产品不为空，即有要生产的产品，则看看这个生产的产品设置使用了哪些包装线
		 {
		   DEBUG_CARE("use pacgekv=04x\n",pre_production.packge_line);
			 if((index-1)!=i)
			 {
				line = line| pre_production.packge_line ;	// pre_production.packge_line 这个里面保存的有该预生产产品需要哪些线来生产
			 }
		 }
	}
	addra = &user_set.device_use_info.chA.packge_ch1;
	addrb = &user_set.device_use_info.chB.packge_ch1;
	for(i=0;i<12;i++)	// 12个通道，没有用的则显示空白勾选框，用了的则显示打钩的蓝色勾选框
	{
		if(line &0x01)
		{
		  tab_registers[1779+i] = 0;
		}
		else 
		{
		    tab_registers[1779+i] = 1;
			
			if(i%2==0)
			{
				tab_registers[1779+i]= addra[i/2];
			}
			else 
			{
			    tab_registers[1779+i]= addrb[i/2];
			}
			//rt_kprintf("line=%d val=%d\n",i,addr[i]);
		}
		line = line>>1;
	}
}


static uint16_t edit_per_pruduct_id=0;
// 预生产信息编辑
static void dispose_edit_per_pruduction(uint16_t *tab_registers,uint16_t addr ) 
{
	rt_base_t level;
	uint16_t index,i,disp_addr;
	uint16_t line=0;
	product_packge_info  packge_info;
	pre_production_info pre_production;
	product_attribute_info product_attribute;
	
	
	if(addr==3052)                                   //进入编辑界面
	{
		mb_clear_reg(tab_registers,1649,1682);  //设置的等级
		lock_packge_line(tab_registers);
	  index = tab_registers[3052];
		
		rt_memset(&pre_production,0,sizeof(pre_production_info));
	  get_pre_production_info(index-1,&pre_production);
	//	tab_registers[1610]= pre_production.weight_leve;
	//	tab_registers[1611]= pre_production.colour_leve;
	//	tab_registers[1612]= pre_production.crack_leve;
	//	tab_registers[1613]= pre_production.dirty_leve;
	//	tab_registers[1614]= pre_production.print_id;
		
		 edit_per_pruduct_id = pre_production.product_id;
		 for(i=0;i<10;i++)
		 {
			  tab_registers[1649+i] = pre_production.weight_leve[i];   //重量
		 }
		 for(i=0;i<6;i++)
		 {
			  tab_registers[1659+i] = pre_production.colour_leve[i];   //颜色
		 }
		 for(i=0;i<6;i++)
		 {
			  tab_registers[1665+i] = pre_production.crack_leve[i];   //裂纹
		 }
		 for(i=0;i<5;i++)
		 {
			  tab_registers[1671+i] = pre_production.dirty_leve[i];   //脏蛋
		 }
		 for(i=0;i<5;i++)
		 {
			  tab_registers[1676+i] = pre_production.auto_clacure_weight[i];  //自动分配盘
		 }
		 tab_registers[1682] = pre_production.bood_leve;
		 tab_registers[1681] = pre_production.box_count;
		 tab_registers[1627] = pre_production.print_id;
		 
		rt_memcpy(&tab_registers[1599],pre_production.packge_name,sizeof(pre_production.packge_name));
	  rt_memcpy(&tab_registers[1589],pre_production.product_name,sizeof(pre_production.product_name));
	  
		for( i=0;i<12;i++)
		{ 
		  if(pre_production.packge_line&(1<<i))
				tab_registers[1759+i]=1;
			else 
				tab_registers[1759+i]=0;
		}
		
		tab_registers[1623]=0;
		tab_registers[1624]=0;
		tab_registers[1625]=0;
		if(pre_production.task_complete_warn==1)
			tab_registers[1623]=1;
		else if(pre_production.task_complete_warn==2)
			tab_registers[1624]=1;
		else if(pre_production.task_complete_warn==3)
			tab_registers[1625]=1;
		tab_registers[1621]= pre_production.task_box_count&0xffff;
		tab_registers[1622]= pre_production.task_box_count>>16;
		
		tab_registers[1646]= pre_production.task_egg_count>>16;
		tab_registers[1645]= pre_production.task_egg_count&0x00ffff;
		pruduct_packge_id = pre_production.packge_id;
		
		DEBUG_CARE("PACKGE ID=%d\n",pruduct_packge_id);
	}
	else if((addr==3051)&&(tab_registers[addr]==1))         //删除预生产产品          
	{
		index = tab_registers[3053]-1;

		rt_memset(&pre_production,0,sizeof(pre_production_info));
		get_pre_production_info(index,&pre_production);
      save_product_hostroy(index);
	  delete_pre_production_info(index);   
		clear_already_pruduct_egg_dat(index);
	}
	else if((addr==2199)&&(tab_registers[1626]==1))  //选择 1626 =1 
	{
		
		edit_per_pruduct_id = (tab_registers[1800]-1)*13 + (tab_registers[2199]-1);
		get_pruduct_attribute_info(edit_per_pruduct_id, &product_attribute);
	
	  for(i=0;i<10;i++)
		{
			tab_registers[1649+i] = product_attribute.weight_leve[i];   //重量
		}
		for(i=0;i<6;i++)
		{
			tab_registers[1659+i] = product_attribute.colour_leve[i];   //颜色
		}
		for(i=0;i<6;i++)
		{
			tab_registers[1665+i] = product_attribute.crack_leve[i];   //裂纹
		}
		for(i=0;i<5;i++)
		{
			tab_registers[1671+i] = product_attribute.dirty_leve[i];   //脏蛋
		}
		tab_registers[1614]= product_attribute.print_id;
		tab_registers[1682]= product_attribute.bood_leve;
		pruduct_packge_id = product_attribute.packge_id;
	  rt_memcpy(&tab_registers[1599],product_attribute.packge_name,sizeof(pre_production.packge_name));
	  rt_memcpy(&tab_registers[1589],product_attribute.product_name,sizeof(pre_production.product_name));
	//	rt_kprintf("product_name=%s packge=%s packge id=%d\n",product_attribute.product_name,product_attribute.packge_name,pruduct_packge_id);

	}
	else if((addr>=1623)&&(addr<=1625))   //任务完成时提示
	{
		tab_registers[1623]=0;
		tab_registers[1624]=0;
		tab_registers[1625]=0;
		tab_registers[addr]=1;
	}
	else if((addr==3050)&&(tab_registers[addr]==1))   //确认保存编辑
	{
		
	    index = tab_registers[3052]-1;
		rt_memset(&pre_production,0,sizeof(pre_production_info));
	    pre_production.product_id = edit_per_pruduct_id;
		
		 for(i=0;i<10;i++)
		 {
			  pre_production.weight_leve[i]=tab_registers[1649+i];   //重量
		 }
		 for(i=0;i<6;i++)
		 {
			  pre_production.colour_leve[i]=tab_registers[1659+i];   //颜色
		 }
		 for(i=0;i<6;i++)
		 {
			  pre_production.crack_leve[i]=tab_registers[1665+i];   //裂纹
		 }
		 for(i=0;i<5;i++)
		 {
			  pre_production.dirty_leve[i]=tab_registers[1671+i];   //脏蛋
		 }
		 for(i=0;i<5;i++)
		 {
			  pre_production.auto_clacure_weight[i]=tab_registers[1676+i];  //自动分配盘
		 }
		pre_production.box_count =   tab_registers[1681];
		pre_production.print_id = tab_registers[1627];
		pre_production.bood_leve = tab_registers[1682];
		line =0;					// 12条包装线，12个位来保存该包装线是否使用
											// 这个是编辑某个预生产产品的时候，保存该预生产产品设置的需要哪些线来生产
		line += tab_registers[1770]&&1;
		line = line<<1;
		line += tab_registers[1769]&&1;
		line = line<<1;
		line += tab_registers[1768]&&1;
		line = line<<1;
		line += tab_registers[1767]&&1;
		line = line<<1;
		line += tab_registers[1766]&&1;
		line = line<<1;
		line += tab_registers[1765]&&1;
		line = line<<1;
		line += tab_registers[1764]&&1;
		line = line<<1;
		line += tab_registers[1763]&&1;
		line = line<<1;
		line += tab_registers[1762]&&1;
		line = line<<1;
		line += tab_registers[1761]&&1;
		line = line<<1;
		line += tab_registers[1760]&&1;
		line = line<<1;
		line += tab_registers[1759]&&1;
		 
		pre_production.packge_line = line;
		
		if(tab_registers[1623])
			pre_production.task_complete_warn=1;
		else if(tab_registers[1624])
			pre_production.task_complete_warn=2;
		else if(tab_registers[1625])
			pre_production.task_complete_warn=3;
		else 
			pre_production.task_complete_warn=1;
		

		for(i=0;i<6;i++)
		{
			if(tab_registers[1615+(5-i)]==1) //预生产界面颜色条
			{
			
			//  tab_registers[3072+i]= pre_production.weight_leve;
			//	tab_registers[3081+i]= pre_production.colour_leve;
			//	tab_registers[3090+i]= pre_production.crack_leve;
			//	tab_registers[3099+i]= pre_production.dirty_leve;
			//	tab_registers[3108+i]= 1;                               //血斑等级
			}
		}
		
		pre_production.task_box_count =  (tab_registers[1621] + (tab_registers[1622]<<16));
		pre_production.packge_id = pruduct_packge_id;
    
		get_pruduct_packge_info(pre_production.packge_id,&packge_info);
        pre_production.empty=1;
		pre_production.task_egg_count =  pre_production.task_box_count * (packge_info.line*packge_info.row);
		pre_production.tray_row = packge_info.row;
		pre_production.tray_line = packge_info.line;
		rt_memcpy(pre_production.product_name,&tab_registers[1589],sizeof(pre_production.product_name));
		rt_memcpy(pre_production.packge_name,&tab_registers[1599],sizeof(pre_production.packge_name));
		
		
		pre_production_info *odl_pre_production;
		
		odl_pre_production =rt_malloc(sizeof(pre_production_info));
		if(odl_pre_production !=RT_NULL)
		{
			uint16_t comp_len;
			comp_len = (uint8_t *)&pre_production.auto_clacure_weight[0]- (uint8_t *)&pre_production.empty;
			get_pre_production_info(index,odl_pre_production);
			if(rt_memcmp((uint8_t *)&pre_production,(uint8_t *)odl_pre_production,comp_len)!=0)
			{
				if(odl_pre_production->empty !=0)
				{
				}
			}
		  rt_free(odl_pre_production);
		}
		save_pre_production_info(index,&pre_production);
		//DEBUG_CARE("SAVE ID=%d  name=%s packge id=%d \n",index,pre_production.product_name,pre_production.packge_id);
	}
	else if(addr==1621)
	{
	  pre_production.task_box_count =  (tab_registers[1621] + (tab_registers[1622]<<16));
		pre_production.packge_id = pruduct_packge_id;
    
		get_pruduct_packge_info(pre_production.packge_id,&packge_info);

		pre_production.task_egg_count =  pre_production.task_box_count * (packge_info.line*packge_info.row);
	  tab_registers[1646]= pre_production.task_egg_count>>16;
		tab_registers[1645]= pre_production.task_egg_count & 0x00ffff;
	}
	
	if((addr>=1649)&&(addr<=1681))
	{
	   if((addr>=1649)&&(addr<=1656))
		 {
		   tab_registers[1657]= 0;
		   tab_registers[1658]= 0;
		 }
		 else if((addr>=1657)&&(addr<=1658))
		 {
		   if (tab_registers[1657]>tab_registers[1658])
			 {
				tab_registers[1658]= tab_registers[1657];
			 }
			 mb_clear_reg(tab_registers,1649,1656);
		 }
		 else if((addr>=1659)&&(addr<=1662))
		 {
		   tab_registers[1663]= 0;
		   tab_registers[1664]= 0;
		 }
		 else if((addr>=1663)&&(addr<=1664))
		 {
		   if (tab_registers[1663]>tab_registers[1664])
			 {
				tab_registers[1664]= tab_registers[1663];
			 }
			 mb_clear_reg(tab_registers,1659,1662);
		 }
		 
		 else if((addr>=1665)&&(addr<=1668))
		 {
		   tab_registers[1669]= 0;
		   tab_registers[1670]= 0;
		 }
		 else if((addr>=1669)&&(addr<=1670))
		 {
		   if (tab_registers[1669]>tab_registers[1670])
			 {
				tab_registers[1670]= tab_registers[1669];
			 }
			 mb_clear_reg(tab_registers,1665,1668);
		 }
		 else if((addr>=1671)&&(addr<=1673))
		 {
		   tab_registers[1674]= 0;
		   tab_registers[1675]= 0;
		 }
		 else if((addr>=1674)&&(addr<=1675))
		 {
		   if (tab_registers[1674]>tab_registers[1675])
			 {
				tab_registers[1675]= tab_registers[1674];
			 }
			 mb_clear_reg(tab_registers,1671,1673);
		 }
		 else if((addr>=1676)&&(addr<=1681))
		 {
		   if (tab_registers[1676]==0)
			 {
				 mb_clear_reg(tab_registers,1677,1681);
			 }
			 else 
			 {
				 if(tab_registers[1677]<50)
				 {
					 tab_registers[1677] = 50;
				 }
			   if ((tab_registers[1677]+5)>tab_registers[1678])
				 {
				   tab_registers[1678] =  tab_registers[1677]+5;
				 }
			 }
			 if (tab_registers[1679]>tab_registers[1680])
			 {
				tab_registers[1680]= tab_registers[1679];
			 }
			 
			 if (tab_registers[1676]>0)  //启用自动配盘
			 {
					uint32_t box_egg_count=0;
					uint32_t max_weight;
					uint32_t min_weight; 
					get_pruduct_packge_info(pruduct_packge_id,&packge_info);
					box_egg_count = packge_info.row *packge_info.line;
					if((box_egg_count>4)&&(box_egg_count<60)&&(tab_registers[1681]>0))
					{
						box_egg_count = box_egg_count*tab_registers[1681];
						min_weight = tab_registers[1677]*1000;
						max_weight = tab_registers[1678]*1000;
						min_weight = min_weight/box_egg_count;
						max_weight = max_weight/box_egg_count;
						
						if(min_weight%10>5)
						{
						  min_weight = min_weight/10 +1;
						}
						else 
						{
						  min_weight = min_weight/10;
						}
						if(max_weight%10>5)
						{
						  max_weight = max_weight/10 +1;
						}
						else 
						{
						  max_weight = max_weight/10;
						}
						//if(min_weight>3) min_weight = min_weight -2;  //2=上下补偿波动
						//max_weight = max_weight+2;
						tab_registers[1679] = min_weight;
						tab_registers[1680] = max_weight;
					}
			 }
		 }
	}
}
// 新建或者修改产品
static void dispose_crate_prodict_attribute_info(uint16_t *tab_registers,uint16_t addr )   //新增或者修改确认键触发
{
	uint16_t idex;
  product_attribute_info attribute_info;
	
	if((addr==1726)&&(tab_registers[1726]==0)) return;  //取消操作


	if(addr==1802)                        //新建按钮
	{
		uint16_t index;
		product_attribute_info info;
		
		if(tab_registers[1800]>0)
			index = (tab_registers[1800]-1)*13;
		else 
			index =0;
		if(tab_registers[1802]>0)           //编辑新增键
		{
			index += (tab_registers[1802]-1);
			get_pruduct_attribute_info(index,&info);
		}
		else 
		{
			rt_memset(&info,0,sizeof(product_attribute_info));
			set_packge_id(10000);
		}
		rt_kprintf("edit attribute id=%d name=%s\n",index,info.product_name);
		rt_memcpy(&tab_registers[1699],&info.product_name[0],sizeof(info.product_name));
		rt_memcpy(&tab_registers[1709],&info.packge_name[0],sizeof(info.packge_name));
		rt_memcpy(&tab_registers[1728],&info.weight_leve[0],sizeof(info.weight_leve));
		rt_memcpy(&tab_registers[1738],&info.colour_leve[0],sizeof(info.colour_leve));
		rt_memcpy(&tab_registers[1744],&info.crack_leve[0],sizeof(info.crack_leve));
		rt_memcpy(&tab_registers[1750],&info.dirty_leve[0],sizeof(info.dirty_leve));
		tab_registers[1723]= info.print_id;
    tab_registers[1755]= info.bood_leve;
	}
	
	else if(addr==1726)                                 //产品管理/产品编辑 保存界面
	{
			rt_memcpy(&attribute_info.product_name[0],&tab_registers[1699],sizeof(attribute_info.product_name));
		  rt_memcpy(&attribute_info.packge_name[0],&tab_registers[1709],sizeof(attribute_info.packge_name));
		  rt_memcpy(&attribute_info.weight_leve[0],&tab_registers[1728],sizeof(attribute_info.weight_leve));
		  rt_memcpy(&attribute_info.colour_leve[0],&tab_registers[1738],sizeof(attribute_info.colour_leve));
		  rt_memcpy(&attribute_info.crack_leve[0],&tab_registers[1744],sizeof(attribute_info.crack_leve));
		  rt_memcpy(&attribute_info.dirty_leve[0],&tab_registers[1750],sizeof(attribute_info.dirty_leve));
		  attribute_info.print_id = tab_registers[1723];
		  attribute_info.bood_leve = tab_registers[1755];
		  attribute_info.packge_id =  get_packge_id();
		  rt_kprintf("packge id=%d\n",attribute_info.packge_id);
			if(tab_registers[1802]==0)
			{
				crate_product_attribute_info(&attribute_info);
			}
			else
			{
				
				if(tab_registers[1800]>0)
				{
					idex = (tab_registers[1800]-1)*13+tab_registers[1802];
				}
				else 
				{
					idex = tab_registers[1802];
				}
				edit_product_attribute_info(idex,&attribute_info);
			}
	}
	else if((addr==1727)&&(tab_registers[1727]==1))   
	{
	  if(tab_registers[1800]>0)
		{
					idex = (tab_registers[1800]-1)*13+tab_registers[1803];
		}
		else 
		{
					idex = tab_registers[1803];
		}
		delete_product_attribute_info(idex);
	}
}
extern void reinit_position();
extern void init_packge_line_start_position();
extern void set_show_force_egg_info_ch_time(uint8_t time);
// 编辑通道的落点脉冲偏移值
void edit_decive_offest(uint16_t *tab_registers,uint16_t addr )  
{
	
	if((addr>=1309)&&(addr<=1346))
	{
		device_offest_position device_position;
		
		rt_memcpy(&device_position.chA,&tab_registers[1309],sizeof(offest_position));
		rt_memcpy(&device_position.chB,&tab_registers[1329],sizeof(offest_position));
		
		//if(device_position.chA.spt_cup_count<MIN_CUP_COUNT) device_position.spt_cup_count = MIN_CUP_COUNT;
	 // if(device_position.spt_cup_count>MAX_CUP_COUNT) device_position.spt_cup_count = MAX_CUP_COUNT;
		
		save_device_offest_to_eep(&device_position);
		show_decive_offest();
	}
	if(addr==1307)
	{
		rt_hw_cpu_reset();
		//init_packge_line_start_position();
		//reinit_position();
	}
	if(addr==1308)
	{
		set_envent_id(1);
		set_show_force_egg_info_ch_time(20);
	}
	if(addr==1305)
	{
		set_envent_id(2);
		set_show_force_egg_info_ch_time(20);
	}
	if(addr==1306)
	{
		set_envent_id(3);
		set_show_force_egg_info_ch_time(20);
	}
}

void dispose_change_page(uint16_t *tab_registers,uint16_t addr )
{

   if(tab_registers[addr]==56)
	 { 
     show_decive_offest();
	 }
	 else if(tab_registers[addr]==250)  
	 {
	   displsy_mode_en_setting(tab_registers,addr);
	 }	 
}

 // 返回0 数据不属于此范围
// 分级线调试界面调试
uint8_t dispose_debug_packeg_lind(uint16_t *tab_registers,uint16_t addr )
{
      uint8_t message_buf[MESSAGE_DAT_LEN]; 
	    rt_memset(message_buf,0,sizeof(message_buf));
			if((addr>=1217)&&(addr<=1220))
			{
				mb_clear_reg(tab_registers,1217,1220);
				tab_registers[addr]=1;
				if(tab_registers[1217]==0)             //自动发托盘
				{
					tab_registers[1235]=0;
				}
				if(tab_registers[1218]==0)              //蛋托传输
				{
					mb_clear_reg(tab_registers,1222,1223);
				}
				if(tab_registers[1219]==0)              //接受抓
				{
					mb_clear_reg(tab_registers,1236,1237);   //
				}
				if(tab_registers[1220]==0)                 //蛋夹
				{
					mb_clear_reg(tab_registers,1238,1239);
				}
			}
			else if((addr>=1235)&&(addr<=1235))
			{
				tab_registers[1235]= tab_registers[1217] ? tab_registers[1235] :0;

			}
			else if((addr>=1222)&&(addr<=1223))
			{
				mb_clear_reg(tab_registers,1222,1223);
				if(tab_registers[1218])
				{
					tab_registers[addr] =1;
				}
			}
			else if((addr>=1236)&&(addr<=1237))
			{
				mb_clear_reg(tab_registers,1236,1237);
				if(tab_registers[1219])
				{
					tab_registers[addr] =1;
				}
			}
			else if((addr>=1238)&&(addr<=1239))
			{
				mb_clear_reg(tab_registers,1238,1239);
				if(tab_registers[1220])
				{
					tab_registers[addr] =1;
				}
			}
			else if((addr>=1209)&&(addr<=1214))
			{
				message_buf[0]=5;      //调试命令
				message_buf[1]=tab_registers[SCREEN_PACKGE_CH_ADDR]%2;      //蛋带通道通道				  0-1
				message_buf[2]=addr-1209;      //蛋杯通道   0-5
				message_buf[3]=tab_registers[addr];      //开关状态      开
				message_buf[4]=1;      //开启时间
				send_message_to_packge_device(tab_registers[SCREEN_PACKGE_CH_ADDR],message_buf);     
			}
			else if((addr>=610)&&(addr<=622))
			{
				uint16_t packge_ch=0;
				
				packge_ch = tab_registers[609];
				if(addr==612)
				{
					message_buf[0]=2;      //
					message_buf[1]=packge_ch%2;      //蛋带通道通道				  0-1
					message_buf[2]= tab_registers[addr];      //启动/停止
					message_buf[3]=1;      //1 正转  2 反转
					send_message_to_packge_device(packge_ch,message_buf);   
				}
				else if((addr==610)&&(addr<=611))
				{
				  message_buf[0]=2;      //
					message_buf[1]=packge_ch%2;      //蛋带通道通道				  0-1
					message_buf[2]= tab_registers[addr];      //启动/停止
					message_buf[3]=addr-609;      //1 正转  2 反转
					send_message_to_packge_device(packge_ch,message_buf);    
				}
				else if((addr>=613)&&(addr<=614))
				{
					message_buf[0]=3;      //传输线控制
					message_buf[1]=packge_ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=tab_registers[addr];     //启动/停止
					message_buf[3]=addr-612;      //1上升 2 下降
					send_message_to_packge_device(packge_ch,message_buf);     
				}
				if((addr>=615)&&(addr<=616))
				{
				  message_buf[0]=4;      //
					message_buf[1]=packge_ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=addr - 614;      //2关闭  1打开    
					send_message_to_packge_device(packge_ch,message_buf);     
				}
				else if((addr>=617)&&(addr<=622))   //蛋杯开关
				{
					message_buf[0]=5;      //调试命令
					message_buf[1]=packge_ch%2;      //蛋带通道通道				  0-1
					message_buf[2]=addr-617;      //蛋杯通道   0-5
					message_buf[3]=tab_registers[addr];      //开关状态      开
					message_buf[4]=1;      //开启时间
					send_message_to_packge_device(packge_ch,message_buf);     
				}
			}
			else if((addr>=1240)&&(addr<=1244)) 
			{
			  uint8_t i;
				
				if(tab_registers[1245] !=2)
				{
					tab_registers[1245] =1;
					if(tab_registers[addr]>PACKGE_BOX_LEN)
					{
						tab_registers[addr]=PACKGE_BOX_LEN;
					}
					for(i=0;i<4;i++)
					{
						if(tab_registers[1241+i]<tab_registers[1240+i])
						{
							tab_registers[1241+i] = tab_registers[1240+i];
						}
					}
				}
				else 
				{
				
				  if(tab_registers[addr]>(PACKGE_BOX_LEN*2))
					{
						tab_registers[addr]=(PACKGE_BOX_LEN*2);
					}
					for(i=0;i<4;i++)
					{
						if(tab_registers[1241+i]<tab_registers[1240+i])
						{
							tab_registers[1241+i] = tab_registers[1240+i];
						}
					}
				}
			}
			else 
			{
				return 0;
			}
			return 1;
}

// 设置包装落点脉冲参数并发给包装通道设备
void dispose_set_product_packge(uint16_t *tab_registers,uint16_t addr )
{

  uint8_t message_buf[MESSAGE_DAT_LEN]; 
	product_packge_info dat;
	
	copy_screen_packge_set_dat(&dat,tab_registers);
	rt_memset(message_buf,0,sizeof(message_buf));
	message_buf[0]=10;
	rt_memcpy(&message_buf[1],(uint8_t*)&dat,(MESSAGE_DAT_LEN-1));
	send_message_to_packge_device(1,message_buf);     
}
extern void ceqr();
extern void set_export_recode();
extern void re_spt_egg_to_packge_line(uint8_t line);
uint8_t packge_paixu_flag=0;
extern void paixu_product_packge();
void dispose_simple_function(uint16_t *tab_registers,uint16_t addr )
{
	uint16_t count,index;
	rtc_info_t time;
	uint32_t buf;
	
	
	if(addr==2407)
	{
		if(packge_paixu_flag==0)
		{
		  packge_paixu_flag=1;
			paixu_product_packge();
			packge_paixu_flag=0;
			rt_kprintf("packge paixu\r\n");
		}
	}
	if(addr==1245)
	{
		if(tab_registers[addr]>2)
		{
			tab_registers[addr]=2;
		}
		else if(tab_registers[addr]==0)
		{
			tab_registers[addr]=1;
		}
	}
	else if(addr==3990)
	{
		set_export_recode();
	}
	else if(addr==1284)
	{
		rt_hw_cpu_reset();
	}
	else if(addr==3564)
	{
		ceqr();
	}
	else if(addr==1799)
	{
		count = get_pruduct_attribute_count();
		if(tab_registers[1799]==0)  //上一页
		{
		  if(tab_registers[1800]>1)
			{
				tab_registers[1800]--;
			}
		}
		else 
		{
		  if(tab_registers[1800]<tab_registers[1801])
			{
				tab_registers[1800]++;
			}
		}
	}
	else if(addr==2399)
	{
		count = get_pruduct_packge_count();
		if(tab_registers[2399]==0)  //上一页
		{
		  if(tab_registers[2400]>1)
			{
				tab_registers[2400]--;
			}
		}
		else 
		{
		  if(tab_registers[2400]<tab_registers[2401])
			{
				tab_registers[2400]++;
			}
		}
	}
	else if(addr==3399)     //供应商管理翻页
	{
		count = get_supplier_count();
		if(tab_registers[3399]==0)  //上一页
		{
		  if(tab_registers[3400]>1)
			{
				tab_registers[3400]--;
			}
		}
		else 
		{
		  if(tab_registers[3400]<tab_registers[3401])
			{
				tab_registers[3400]++;
			}
		}
	}
	else if((addr==2201)&&(tab_registers[3049]==1))
	{
		 supplier_management_info *info;
		 index = (tab_registers[3400]-1)*13;
		 index += (tab_registers[2201]-1);
		 info = (supplier_management_info *) &tab_registers[3039];
		 get_supplier_info(index,info);
		 
		 production_type * dat;
		 
		 dat =rt_malloc(sizeof(production_type));
		 if(dat !=RT_NULL)
		 {
			 get_production_mange_set(dat);
			 uint8_t len;
			 len = strlen((char*)&tab_registers[3039]);
			 if(len>sizeof(dat->egg_source))
				 len= sizeof(dat->egg_source);
			 rt_memset(dat->egg_source,0,sizeof(dat->egg_source));
			 rt_memcpy(dat->egg_source,&tab_registers[3039],len);
			 get_save_ayaylisis_egg_index((char *)dat->egg_source);
			 save_production_mange_set(dat);
			 rt_free(dat);
		 }
	 }
	 else if(addr==4405)
	 {
	   index =  ReadMBHotRegU16(4404);
		 if(index>SEPARATE_CHA_CUP_NUMBER)
		 {
			 index = SEPARATE_CHA_CUP_NUMBER;
			 WriteMBHotRegU16(4404,index);
		 }
		 set_cup_move_to_position(index);
	 }
	 else if(addr==2744)
	 {
	   set_clear_egg_flag();
	 }
	 else if(addr==2745)
	 {
	   //re_spt_egg_to_packge_line(0);
	 }
     else if(addr==2746)
	 {
	   //re_spt_egg_to_packge_line(1);
	 }
	 else if(addr==4709)
	 {
		 if(ReadMBHotRegU32(3561)<1000)
		 {
		    get_local_time(&time);
			  buf = (time.year+1900)*10000;
		    buf = buf+ (time.month*100);
			  buf = buf + time.date;
			  WriteMBHotRegU32(3561,buf);  
		 }
		 set_disp_sypply_id("所有供应商");
	 }
	 else if(addr==1482)
	 {
		 if(tab_registers[1482]==1)
		 {
			  dispose_throw_ch_set();
		 }
	 }
	 else if(addr==1483)
	 {
		 if(tab_registers[1483]==1)
		 {
			  dispose_throw_ch_set();
		 }
	 }
	 else if((addr==5637)||(addr==5638))
	 {
	   show_throw_ch_set();
	 }
	 else if(addr==4198)
	 {
	   clear_electromagnet_err_count();
	 }
	 else if((addr==1404)||(addr == 1410)||(addr == 1478)||(addr == 1412))
	 {
			if(tab_registers[addr]>10) tab_registers[addr]=10;
	 }
	 else if((addr>=1205)&&(addr<=1208))
	 {
	   if(tab_registers[1205]>0)
		 {
		   if(tab_registers[1205]<=tab_registers[1206])
			 {
			   tab_registers[1206] = tab_registers[1205]-1;
			 }
			 
			 if(tab_registers[1205]<=tab_registers[1207])
			 {
			   tab_registers[1207] = tab_registers[1205]-1;
			 }
			 if(tab_registers[1205]<=tab_registers[1208])
			 {
			   tab_registers[1208] = tab_registers[1205]-1;
			 }
		 }
		 else 
		 {
			 
		   tab_registers[1205] =0;
			 tab_registers[1206] =0;
			 tab_registers[1207] =0;
			 tab_registers[1208] =0;
		 }
	 
	 }
}
// 显示混杂设备的信息
void disp_mix_device_part(uint16_t *addr)
{
   mix_device_part mix_device;
	
   get_mix_device_part(&mix_device);
	
	 addr[4195] = mix_device.tick_egg_moto_err_count;
   addr[4400] = mix_device.egg_cup_err_count ;

}
// 设置混杂设备的信息
void dispose_set_mix_device_part(uint16_t *tab_registers,uint16_t addr )
{
	 uint16_t id;
	 uint16_t c_page;
   mix_device_part mix_device;
	 
  
	
   if(addr==4195)
	 {
		 get_mix_device_part(&mix_device);
	   mix_device.tick_egg_moto_err_count = tab_registers[4195];
		 save_mix_device_part(&mix_device);
		 disp_mix_device_part(tab_registers);
	 }
   else if(addr==4197)
	 {
		 id = ReadMBHotRegU16(4196);
		 c_page = get_current_screen_page();
		 if((c_page == 62) ||(c_page == 63)) 
		 {
		   clear_one_electromagnet_err_count(0,id-1);
		 }
		 else if((c_page == 65) ||(c_page == 66)) 
		 {
		   clear_one_electromagnet_err_count(1,id-1);
		 }
		 
	 }
	 else if(addr==4400)
	 {
	   get_mix_device_part(&mix_device);
	   mix_device.egg_cup_err_count = tab_registers[4400];
		 save_mix_device_part(&mix_device);
		 disp_mix_device_part(tab_registers);
	 }
	 else if(addr==4401)
	 {
		 clear_all_cup_err_count();
	 }
	 else if(addr==4407)
	 {
		 id = ReadMBHotRegU16(4406);
		 c_page = get_current_screen_page();
		 if((c_page == 62) ||(c_page == 63)) 
		 {
			 clear_ont_cup_err_count(0,id-1);
		 }
		 if((c_page == 65) ||(c_page == 66)) 
		 {
			 clear_ont_cup_err_count(1,id-1);
		 }
	 }
   
}
// 各种重连次数设置
void dispose_debug_expansion_link(uint16_t *tab_registers,uint16_t addr )
{

	
		get_user_set_part(&user_set_buf);
		user_set_buf.commun_part.tcp_alarm_count=tab_registers[1279];
		user_set_buf.commun_part.reconnect_timeou_number=tab_registers[1280];
		user_set_buf.commun_part.servo_moto_connect_number = tab_registers[1281];
		
		if(user_set_buf.commun_part.tcp_alarm_count>10)
		{
			user_set_buf.commun_part.tcp_alarm_count = 10;
		}
		save_user_set_to_eep(&user_set_buf);
	
}
void dispose_debug_mast_control_device(uint16_t *tab_registers,uint16_t addr )
{

	if((addr>=809)&&(addr<=812))
	{
 
	 mb_clear_reg(tab_registers,809,812);
	 tab_registers[addr]=1;
	}
	else if((addr>=813)&&(addr<=814))
	{
   mb_clear_reg(tab_registers,813,814);
	 tab_registers[addr]=1;
	}
	else if((addr>=817)&&(addr<=818))
	{
   mb_clear_reg(tab_registers,817,818);
	 tab_registers[addr]=1;
	}
}
void dispose_select_weight(uint16_t *tab_registers,uint16_t addr )
{
  if(tab_registers[1413] == 3)
	{
	  tab_registers[1719] = addr -1515;
	}
	if(tab_registers[1413]==4)
	{
    tab_registers[1610] = tab_registers[addr];
	}
}


//  dat ：1 恢复原点
// 包装伺服恢复到原点
// 包装伺服单步调试
void dispose_debug_packge_box(uint16_t *tab_registers,uint16_t addr )
{
	uint8_t step;
	uint8_t row;
	uint8_t sys_tem_state;
	uint8_t message_buf[MESSAGE_DAT_LEN]; 
	uint16_t step_pluse;
	float egg_box_len=0;
	float buf;
	
	sys_tem_state = get_system_state();
	if(sys_tem_state !=SYSTEM_IS_IDLE)
	{
		return ;
	}
	rt_memset(message_buf,0,sizeof(message_buf));
  if(addr==1234)
	{
		tab_registers[1232]=0;
		tab_registers[1233] = tab_registers[1199]+ 1; 
		message_buf[0]=DEBUG_PACKGE_BOX_CMD;      //命令
		message_buf[1]=tab_registers[1188]%2;      //通道		0 左 1 右
		message_buf[2]= 1;    // dat
		send_message_to_packge_device(tab_registers[1188],message_buf); 
	}
	else if(addr==1231)
	{

		tab_registers[1233] = tab_registers[1199]+1; 
		
		egg_box_len = tab_registers[1204] -  tab_registers[1203];
		if(egg_box_len<=0)
		{
			egg_box_len =1;
		}
		if(tab_registers[1199] >1)
		{
			
		  egg_box_len = (float)(egg_box_len/(tab_registers[1199]-1));
		}
    
		step = tab_registers[1232];
		row = tab_registers[1199];
		
		
		if(step==0)
		{
		  step_pluse =  tab_registers[1240];
			buf = (float)PACKGE_SERVO_CYCLE_SETP/PACKGE_BOX_LEN;   //52.583
		  buf = buf*step_pluse;
		  step_pluse = (uint16_t)(buf);
		}
		else if(step<tab_registers[1199])
		{
			step_pluse =  tab_registers[1240+ step] - tab_registers[1239+ step];
			DEBUG_INFO("step=%d reg0=%d reg1=%d add=%d\n",step,tab_registers[1240+ step],tab_registers[1241+ step],step_pluse);
		  
			buf = (float)PACKGE_SERVO_CYCLE_SETP/PACKGE_BOX_LEN;   //52.583
		  buf = buf*step_pluse;
		  step_pluse = (uint16_t)(buf);
		}
		else 
		{
			if( tab_registers[1245] !=2)
			{
				step_pluse = PACKGE_BOX_LEN - tab_registers[1240+(row-1)];
			}
			else 
			{
			  step_pluse = (PACKGE_BOX_LEN*2) - tab_registers[1240+(row-1)];
			}
			buf = (float)PACKGE_SERVO_CYCLE_SETP/PACKGE_BOX_LEN;   //52.583
		  buf = buf*step_pluse;
		  step_pluse = (uint16_t)(buf);
		}
		if( tab_registers[1245] !=2)
		{
			if(step_pluse>PACKGE_SERVO_CYCLE_SETP)
			{
				DEBUG_ERR("setp part err\n");
				return;
			}
		}
		else 
		{
		  if(step_pluse>(PACKGE_SERVO_CYCLE_SETP*2))
			{
				DEBUG_ERR("setp part err\n");
				return;
			}
		}
		tab_registers[1232]++;
		if(tab_registers[1232]>=tab_registers[1233])
		{
		   tab_registers[1232]=0;
		}
		
		rt_kprintf("servo step pluse=%d\n",step_pluse);
		
		message_buf[0]=DEBUG_PACKGE_BOX_CMD;      //命令
		message_buf[1]=tab_registers[1188]%2;      //通道		0 左 1 右
		message_buf[2]= 2;    // dat
		message_buf[3]= (step_pluse>>8);
		message_buf[4]= (step_pluse&0x00ff);
		send_message_to_packge_device(tab_registers[1188],message_buf); 
	}
}
// 设置每个小时生产多少个蛋(生产速度)
void dispose_set_product_speed(uint16_t *tab_registers,uint16_t addr )
{
	uint32_t count=0;
  mb_clear_reg(tab_registers,3190,3197);
	tab_registers[addr] = 1;
	tab_registers[3189] = addr - 3190;
	
	if(tab_registers[3189]<8)
	{
	  for(int i=0;i<tab_registers[3189];i++)
		{
		  tab_registers[3190+i]=1;
		}
	}
	
	count = tab_registers[3189] *1800;
	count += 23400;
	//count = (tab_registers[3189]+1) *4500;
    //count =2000;
	WriteMBHotRegU32(2823,count*2);
	
	 production_type * dat;
	 dat =rt_malloc(sizeof(production_type));
	 if(dat !=RT_NULL)
	 {
			get_production_mange_set(dat);
			dat->speed = ReadMBHotRegU32(2823)/2;
			save_production_mange_set(dat);
			rt_free(dat);
	 }
}
void dispose_screen_dat(uint16_t *tab_registers,uint16_t addr,uint16_t cmd)
{
	  uint8_t function;
		
	  if(addr==0)
			return;
	  rt_kprintf("write addr=%d val=%d\n",addr,tab_registers[addr]);
	
		function = get_mb_write_reg_function(addr);
		switch(function)
		{
			case SET_TIME:
				set_rtc(tab_registers,addr);
				break;
			case SET_TRANSPORT_PART:
				dispose_transport_line_setting(tab_registers,addr);
				break;
			case SET_USE_MODE:
				dispose_use_mode_setting(tab_registers,addr);
				break;
			case SET_MODE_EN:
				dispose_mode_en_setting(tab_registers,addr);
				break;
			case SET_VOICE:
				dispose_voice_setting(tab_registers,addr);
				break;
			case DEBUG_TRANSPORT_LINE:
				dispose_debug_transport_lind(tab_registers,addr);
				break;
			case DEBUG_THROW_LINE:
				dispose_debug_throw_line(tab_registers,addr);
				break;
			case DEBUG_PACKGE_LINE:
				dispose_debug_packeg_lind(tab_registers,addr);
				break;
			case CHANGE_PAGE:
				dispose_change_page(tab_registers,addr);
				break;
			case DEBUG_EXPANSION_LINK:
				dispose_debug_expansion_link(tab_registers,addr);
				break;
			case EDIT_PRODUCT_PACK_INFO:
				dispose_crate_prodict_packge_info(tab_registers,addr);
				break;
			case EDIT_SUPPLIER_INFO:
				dispose_crate_supplier_info(tab_registers,addr);
				break;
			case EDIT_PRODUCT_ATTRIBUTE_INFO:
				dispose_crate_prodict_attribute_info(tab_registers,addr);
				break;
			case EDIT_PER_PRODUCTION_INFO:
				dispose_edit_per_pruduction(tab_registers,addr);
				break;
			case SET_EGG_LEVE_INFO:
				dispose_set_egg_leve_info(tab_registers,addr);
				break;
			case SET_COLOUR_LEVE_INFO:
				dispose_set_colour_leve_info(tab_registers,addr);
				break;

			case SET_DIRTY_LEVE_INFO:
				dispose_set_dirty_leve_info(tab_registers,addr);
				break;
			
			case SET_CRACK_LEVE_INFO:
				dispose_set_crack_leve_info(tab_registers,addr);
				break;
			
			case SIMPLE_FUN:
				dispose_simple_function(tab_registers,addr);
				break;
			case SET_PRUDUCT_PACKGE:
				dispose_set_product_packge(tab_registers,addr);
				break;
			case DEBUG_PACKGE_BOX:
				dispose_debug_packge_box(tab_registers,addr);
				break;
			case DEBUG_MAST_CONTROL:
				dispose_debug_mast_control_device(tab_registers,addr);
				break;
			case EDIT_PRODUCT_EGG_SPEED:
				{
					if(tab_registers[addr]>36000) tab_registers[addr]=36000;
				}
				break;
			case SELECT_WEIGHT_INFO:
				dispose_select_weight(tab_registers,addr);
				break;
			case EDIT_PRODUCT_SPEED:
				dispose_set_product_speed(tab_registers,addr);
				break;
			case SHOW_PRODUCT_HOSTROY:
				disp_product_host_log(tab_registers,addr);
				break;
			case DISP_ANALISYS_QUILITY:
				disp_analisys_quility(tab_registers,addr);
				break;
			case EDIT_DEVICE_POSITION:
				edit_decive_offest(tab_registers,addr);
				break;
			case SET_MIX_DEVICE_PART:
				dispose_set_mix_device_part(tab_registers,addr);
				break;
			default:
				simple_set(tab_registers,addr);
				break;
		}
}

