#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "e6b2.h"
#include "app_screen.h"
#include "udp_weight.h"
#include "mail_box.h"
#include "sync_egginfo.h"
#include "modbus_dirty.h"
#include "mdebug.h"
#include "app_screen.h"
#include "device_alarm.h"

static ai_dat_type new_dirty_datA,new_dirty_datB;
static weight_and_crack_info new_weight_datA,new_weight_datB;
static uint8_t cha_crack_info[6];
static uint8_t chb_crack_info[6];

static  rt_tick_t recive_dirty_info_pluse=0;
static  rt_tick_t recive_weight_info_pluse=0;

static  uint32_t weight_delay_pluse=500;
static  uint32_t dirty_delay_pluse=500;
static  uint16_t ai_crack_delay_pluse;


void init_recive_crack_delay_time(uint32_t pluse)
{
  ai_crack_delay_pluse = pluse%1000;
  DEBUG_WARN("crack delay time=%d\n",ai_crack_delay_pluse);
}

void init_recive_dirty_delay_time(uint32_t pluse)
{
  dirty_delay_pluse = pluse%1000;		// 走一圈编码器产生1000个脉冲，那这个是要等到走多少圈？
  DEBUG_WARN("dirty delay time=%d\n",dirty_delay_pluse);
}

// 将称重位置的脉冲乘以10传进来
void init_recive_weight_delay_time(uint32_t pluse)
{
  weight_delay_pluse = pluse%1000;
  DEBUG_WARN("weight delay time=%d\n",weight_delay_pluse);
}

uint32_t get_crack_info(uint8_t ch,uint8_t *dat)
{

	   rt_base_t level;
	
	
	   level = rt_hw_interrupt_disable();
	
	   if(ch==0)
		 {
			 rt_memcpy(dat,cha_crack_info,6);
			 rt_memset(cha_crack_info,0,6);
		 }
		 else 
		 {
		   rt_memcpy(dat,chb_crack_info,6);
			 rt_memset(chb_crack_info,0,6);
		 }
     rt_hw_interrupt_enable(level);
     return  0;
}

uint32_t get_weight_info(uint8_t ch,weight_and_crack_info *dat)
{

	   rt_base_t level;
	
	
	   level = rt_hw_interrupt_disable();
	
	   if(ch==0)
		 {
			 rt_memcpy(dat,&new_weight_datA,sizeof(weight_and_crack_info));
			 rt_memset(&new_weight_datA,0,sizeof(weight_and_crack_info));
		 }
		 else 
		 {
		   rt_memcpy(dat,&new_weight_datB,sizeof(weight_and_crack_info));
			 rt_memset(&new_weight_datB,0,sizeof(weight_and_crack_info));
		 }
     rt_hw_interrupt_enable(level);
     return  0;
}
// 会获取脏蛋信息
uint32_t get_dirty_info(uint8_t ch,ai_dat_type *dat)
{

	 rt_base_t level;
	
	 level = rt_hw_interrupt_disable();
	
	 if(ch==0)
	 {
			rt_memcpy(dat,&new_dirty_datA,sizeof(ai_dat_type));
			rt_memset(&new_dirty_datA,2,sizeof(ai_dat_type));
			new_dirty_datA.body[0].is_egg=1;
			new_dirty_datA.body[1].is_egg=1;
			new_dirty_datA.body[2].is_egg=1;
			new_dirty_datA.body[3].is_egg=1;
			new_dirty_datA.body[4].is_egg=1;
			new_dirty_datA.body[5].is_egg=1;
	 }
	 else 
	 {
			rt_memcpy(dat,&new_dirty_datB,sizeof(ai_dat_type));
			rt_memset(&new_dirty_datB,2,sizeof(ai_dat_type));
			new_dirty_datB.body[0].is_egg=1;
			new_dirty_datB.body[1].is_egg=1;
			new_dirty_datB.body[2].is_egg=1;
			new_dirty_datB.body[3].is_egg=1;
			new_dirty_datB.body[4].is_egg=1;
			new_dirty_datB.body[5].is_egg=1;
	 }
   rt_hw_interrupt_enable(level);
   return  0;
}
uint32_t dirty_sync_ok_count=0;
static uint32_t up_dirty_recive_info_index=0;
extern uint8_t dirty_sync_err_flag;
extern uint8_t dirty_sync_err_count;
// 一直接收脏蛋信息
void recive_dirty_mesg()
{
   uint8_t j,i;
   rt_base_t level;
   send_mesg_info buf;
   uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
   uint32_t cpluse,recive_pluse;
   uint32_t index=0;
	 int add_pluse;
	

	if(rt_mq_recv(&masseg_ai, &mesg_buf, AI_MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK)
	{
		  // 收到了数据，先获取分级线的编码器脉冲值,则记录下了收到蛋信息的时候的编码器值
			// 这个值到分级线的距离是多少呢
      recive_pluse = get_spt_encode_pluse();
			WriteMBHotRegU16(829,recive_pluse%10000);	// 将获取到的分级线编码器的脉冲值同步到信息2同步值上
	    recive_dirty_info_pluse = recive_pluse;		// 记录下当前收到的脏蛋数据的脉冲值，有什么用呢?
		 // rt_kprintf("recive ai date,recive p=%d delay pluse=%d\n",recive_pluse,dirty_delay_pluse);
		while(1)
		{
		  cpluse = get_spt_encode_pluse();					// 重新获取分级线的编码器脉冲值
		  add_pluse =  cpluse - recive_pluse;				// 然后和接收数据时候的脉冲值求差得到一个值
		  if((add_pluse>dirty_delay_pluse)&&(add_pluse>0))	// 这个时间大于设置的脏蛋设备脉冲位置(这里写的是延迟)
				{																									// 延迟的这段时间，输送线又走了多少个格子呢？
				//rt_kprintf("recive ai date,recive p=%d c pluse=%d\n",add_pluse,cpluse);
				
			    index = cpluse/1000;	// 分级线的编码器脉冲最大是多少
																// 走一个蛋杯，编码器变化多少
																// 如果走一个蛋杯大概是1000个脉冲，那么这里的意思是不是得到了这个脏蛋仔哪一个蛋杯上(相较于第一个原点蛋杯)
				
																// 编码器走一圈的脉冲值是1000
				// 获取到此时编码器的值除以1000得到此时的输送线的第几个格子(输送线格子id)
			   if(up_dirty_recive_info_index>30)
				 {
				  if((index-up_dirty_recive_info_index) !=1)
				  {
					  if(dirty_sync_ok_count<500)
					  {
						set_device_alarm(DIRTY_INFO_SYNC_ERR);
						dirty_sync_err_flag=1;
					  }
					  dirty_sync_ok_count =0;
				  }
				  else
				  {
					dirty_sync_ok_count++;
				  }
				}
				up_dirty_recive_info_index = index;	// 记录上一次接收蛋的id
				
            //rt_kprintf("mesg_buf=\n");
				for( i=0;i<38;i++)
				{
					//rt_kprintf("%04x ",mesg_buf[i]);
				}
				//rt_kprintf("\n");
				
				level = rt_hw_interrupt_disable();
				// 延迟了一段时间后，才将脏蛋信息存储到相应位置
				// 脏蛋信息一次发过来是2条线的信息，每条线包含6个蛋的信息
				new_dirty_datA.id = mesg_buf[0];
				for( i=0;i<6;i++)
				{
					new_dirty_datA.body[i].is_egg= mesg_buf[(i*3)+2]>>8;
					new_dirty_datA.body[i].dirty = mesg_buf[(i*3)+2]&0x00ff;
					new_dirty_datA.body[i].rgb[0]= mesg_buf[(i*3)+3]>>8;
					new_dirty_datA.body[i].rgb[1]= mesg_buf[(i*3)+3]&0x00ff;
					new_dirty_datA.body[i].rgb[2]= mesg_buf[(i*3)+4]>>8;
					new_dirty_datA.body[i].level = mesg_buf[(i*3)+4]&0x00ff;
				}
				new_dirty_datB.id = mesg_buf[0];
				for( i=0;i<6;i++)
				{
					new_dirty_datB.body[i].is_egg= mesg_buf[(i*3)+20]>>8;
					new_dirty_datB.body[i].dirty = mesg_buf[(i*3)+20]&0x00ff;
					new_dirty_datB.body[i].rgb[0]= mesg_buf[(i*3)+21]>>8;
					new_dirty_datB.body[i].rgb[1]= mesg_buf[(i*3)+21]&0x00ff;
					new_dirty_datB.body[i].rgb[2]= mesg_buf[(i*3)+22]>>8;
					new_dirty_datB.body[i].level = mesg_buf[(i*3)+22]&0x00ff;
				}
			
				rt_hw_interrupt_enable(level);
				
			
				   //rt_kprintf("d1=%d d2=%d d3=%d d4=%d d5=%d d6=%d",i,);
					// rt_kprintf("i=%d is eggA=%d chad=%d chac=%d  is eggB=%d chbd=%d chbc=%d\n",i,new_dirty_datA.body[i].is_egg,new_dirty_datA.body[i].dirty,new_dirty_datA.body[i].level,
					//                                                                              new_dirty_datB.body[i].is_egg,new_dirty_datB.body[i].dirty,new_dirty_datB.body[i].level);
				
				
				return ;
		  }
			rt_thread_mdelay(10);
		}
	}
}

static uint32_t up_recive_mesg_id=0;
static uint32_t up_weight_recive_info_index=0;
uint32_t err_index=0;
uint32_t up_index=0;
uint32_t up_mesg_id=0;
uint32_t cmesg_id=0;
// 没有使用这个函数
void get_weight_err_info(uint32_t *err_index1,uint32_t *up_index1,uint32_t *up_mesg_id1,uint32_t *cmesg_id1)
{

 *err_index1 = err_index;
 *up_index1 = up_index;
 *up_mesg_id1 = up_mesg_id;
 *cmesg_id1 =cmesg_id;
}

void show_weightB_pluse()
{
  uint32_t recive_pluse;
	
	recive_pluse = get_spt_encode_pluse();
	WriteMBHotRegU16(829,recive_pluse%1000);
}
#define WEIGHT_DEVICE_POSITION 0  //0:1
void recive_wieght_mesg()
{
	 uint8_t j;
	 rt_base_t level;
	 send_mesg_info buf;
	 send_mesg_info deviceB_buf;
     uint16_t mesg_buf[MESSAGE_DAT_LEN] = {0};
     uint32_t cpluse,recive_pluse;
     uint32_t index=0;
   
	 int add_pluse;
	 // A通道有重量数据
	 if(rt_mq_recv(&masseg_weight, &mesg_buf, MESSAGE_DAT_LEN, RT_WAITING_NO) == RT_EOK)
	 {
		recive_pluse = get_spt_encode_pluse();
		WriteMBHotRegU16(828,recive_pluse%1000);	// 同步一下信息1同步值
		recive_weight_info_pluse = recive_pluse;
		rt_memcpy(&buf,mesg_buf,sizeof(send_mesg_info));
       // rt_thread_mdelay(20);
		
		while(1)
		{
		  cpluse = get_spt_encode_pluse();
		  add_pluse =  cpluse - recive_pluse;
			
		  if(add_pluse>weight_delay_pluse)
																			// 将称重位置的脉冲乘以10传进来
																			// weight_delay_pluse = pluse%1000;
																			// 是不是可以这样理解，就是说在初始化的时候，称重设备的脉冲并不一定在1000的整数的脉冲上，即称重的位置相对于
																			// 分级线的脉冲值不在整圈上，所以要等到延迟到不在一整圈上的部分跑完了才取数据
																			// 或者说输送线的称重数据是按照输送线的编码器一格一格地发的
																			// 但是输送线的一格一格不一定对应分级线的一格一格？
		  {
				get_deviceB_weight_info(&deviceB_buf);		// 为什么A有数据后才去读取B通道的重量数据呢？？？
			  index = cpluse/1000;
				// 就是说这个分级线的编码器脉冲，会在收到第一个蛋杯信号清零
				// 然后这个脉冲开始走
				// 可能走1个蛋杯的脉冲为1000左右
				// 这样算到的index就是以1号蛋杯开始算的第几号蛋杯
				// 然后意思是说，每一圈走到大于30个蛋杯的时候开始同步一下
				// 同步的方式是如果这10ms走的蛋杯个数和上一次蛋杯的个数之差应该是1即1个蛋杯
				// 也就是大概10ms左右走1个蛋杯？
			  if(up_weight_recive_info_index>30)
				{
					
				  //if(up_dirty_recive_info_index==index)  //数据提前了
				  //{
				     
				  //}
				  if((index-up_weight_recive_info_index) !=1)	// 和称重数据进行数据同步，这个是什么原理，又是怎么同步的
				  {
					  err_index = cpluse;
					  up_index = up_weight_recive_info_index;
					  cmesg_id = buf.mesg_id;
					  up_mesg_id = up_recive_mesg_id;
					  set_device_alarm(WEIGHT_SYNC_ERR);
				  }
			
				}
				up_recive_mesg_id = buf.mesg_id;
				up_weight_recive_info_index = index;
				
				level = rt_hw_interrupt_disable();

				// 保存重量裂纹信息
				// 哪里在用这个新重量数据呢？
				for(j=0;j<6;j++)
				{
					if(WEIGHT_DEVICE_POSITION==1)
					{
						new_weight_datA.weight[j] = buf.weight[j];
						new_weight_datA.crack[j] = buf.level[j];
					
						new_weight_datB.weight[j] = deviceB_buf.weight[j];
						new_weight_datB.crack[j]  = deviceB_buf.level[j];
					}
					else 
					{
					  new_weight_datA.weight[j] = buf.weight[5-j];
						new_weight_datA.crack[j] = buf.level[5-j];
					
						new_weight_datB.weight[j] = deviceB_buf.weight[5-j];
						new_weight_datB.crack[j]  = deviceB_buf.level[5-j];
					}
				}
				// B的id和脉冲值呢？用A的？
				new_weight_datA.mesg_id = buf.mesg_id;
				new_weight_datA.pluse = get_spt_encode_pluse();		// 
				rt_hw_interrupt_enable(level);
		    //DEBUG_INFO("recive weight info index=%d pluse=%d\n",new_weight_dat.pluse/1000,cpluse%10000);
				//DEBUG_INFO("recive weight  index=%d pluse index=%d pluse=%d\n",recive_weight_dat_count,cpluse/1000,cpluse%1000);
				//DEBUG_INFO("weight=%d crack =%d\n",new_weight_dat.weight[0],new_weight_dat.crack[0]);
				
				return ;
		  }
		  rt_thread_mdelay(10);
		}
	 }
}

uint8_t dirty_err_count=0;
uint8_t crack_err_count=0;


static uint8_t cha_crack_buf[6];
static uint8_t chb_crack_buf[6];
static uint16_t recive_crack_flag=0;
static uint16_t crack_id=0xff88;
void set_cha_crack_info(uint16_t *dat) //
{
	
	uint32_t recive_pluse;
  
	uint8_t i;
	user_set_part_type *user_set;

	get_user_set_point(&user_set); 
	if(crack_id !=dat[0])	// 视觉裂纹的第一个字和上一次的字不一样，说明有新视觉裂纹数据
	{
		recive_crack_flag=0xa5a6;	// 收到视觉裂纹标志置为0xa5a6
		crack_id = dat[0];				// 记录本次的裂纹id编号
		if(dat[7]>0)							// 第7个字有数据并且开启了裂纹检测功能则报警，为什么第7个字有数据就要报警，是他们约定的吗
		{
			if(user_set->device_use_info.chA.crack_check)
			{
				set_device_alarm(CRACK_DEVICE_ERROR);
			}
		}
		for(i=0;i<6;i++)
		{
			cha_crack_buf[i]=dat[i+1];	// 第1到6字保存的是裂纹数据
		}
	
		recive_pluse = get_spt_encode_pluse();	// 获取分级线的编码脉冲值
		WriteMBHotRegU16(830,recive_pluse%1000);	// 将获取到的分级线编码器脉冲同步到分级线调试的信息3同步值中
	}

}

void set_chb_crack_info(uint16_t *dat) //
{
	uint8_t i;
	uint32_t recive_pluse;
	user_set_part_type *user_set;

	get_user_set_point(&user_set); 
	
	
	if(dat[7]>0)
	{
		if(user_set->device_use_info.chB.crack_check)
		{
				set_device_alarm(CRACK_DEVICE_ERROR);
		}
	}
	for(i=0;i<6;i++)
	{
	  chb_crack_buf[i]=dat[i+1];
	}
	
	recive_pluse = get_spt_encode_pluse();
	WriteMBHotRegU16(833,recive_pluse%1000);

}

// 一直接收视觉裂纹蛋信息，并处理
static uint32_t up_crack_index=0;
void recive_crack_mesg()
{
	 uint8_t j;
	 rt_base_t level;

   uint32_t cpluse,recive_pluse;
   uint32_t index=0;
   
	 int add_pluse;
	
	if(recive_crack_flag==0xa5a6)		// ai视觉裂纹有数据的时候会将这个置为0xa5a6
	{
	  recive_crack_flag=0;
		recive_pluse = get_spt_encode_pluse();
		while(1)
		{
		  cpluse = get_spt_encode_pluse();
		  add_pluse =  cpluse - recive_pluse;
		  if(add_pluse>ai_crack_delay_pluse)
		  {
				
			  index = cpluse/1000;
			  if(up_crack_index>30)
				{
				  if((index-up_crack_index) !=1)
				  {
					  set_device_alarm(BLOOD_INFO_SYNC_ERR);
						//rt_kprintf("device err\r\n");
				  }
				}
				//rt_kprintf("\r\nindex=%d upindex=%d %02x %02x %02x %02x %02x %02x\r\n",index,up_crack_index,cha_crack_buf[0],cha_crack_buf[1],cha_crack_buf[2],cha_crack_buf[3],cha_crack_buf[4],cha_crack_buf[5]);
				up_crack_index = index;
				level = rt_hw_interrupt_disable();
				rt_memcpy(cha_crack_info,cha_crack_buf,6);
        rt_memcpy(chb_crack_info,chb_crack_buf,6);
				rt_hw_interrupt_enable(level);
				return ;
		  }
		  rt_thread_mdelay(10);
	 }
	}
}

// 脏蛋信息线程
static void sync_egg_dirty_info_thread(void *param)
{
  rt_memset(&new_dirty_datA,0,sizeof(ai_dat_type));
	rt_memset(&new_dirty_datB,0,sizeof(ai_dat_type));
	while(1)
	{
     recive_dirty_mesg();		// 脏蛋线程里面，接收消息队列里面的数据，有数据的话更新到新脏蛋数据里面
	   rt_thread_mdelay(10);
	}
}



static void sync_egg_weight_info_thread(void *param)
{

	
	rt_memset(&new_weight_datA,0,sizeof(weight_and_crack_info));
	rt_memset(&new_weight_datB,0,sizeof(weight_and_crack_info));
	while(1)
	{
       recive_wieght_mesg();// 称重线程里面，接收消息队列里面的数据，有数据的话更新到新称重数据里面
	     rt_thread_mdelay(10);
	}
}
static void sync_egg_crack_info_thread(void *param)
{

	rt_memset(&cha_crack_info,0,6);
	rt_memset(&chb_crack_info,0,6);
	while(1)
	{
     recive_crack_mesg();
	   rt_thread_mdelay(10);
	}
}
static int sync_egg_init(void)
{   
    rt_thread_t tid = RT_NULL;
    rt_thread_t tid1 = RT_NULL;
		rt_thread_t tid2 = RT_NULL;
	
    tid = rt_thread_create("rweight", sync_egg_weight_info_thread, RT_NULL, 2048, 20, 10);
    if(tid == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(tid);
		
	  tid1 = rt_thread_create("rdity", sync_egg_dirty_info_thread, RT_NULL, 2048, 21, 10);
    if(tid1 == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(tid1);
	
		#if 0	// qianxuping 山西裂纹数据不是光裂纹，是和重量一起出来的
		tid2 = rt_thread_create("acrack", sync_egg_crack_info_thread, RT_NULL, 2048, 21, 10);
    if(tid2 == RT_NULL)
    {
        return RT_ERROR;
    }
    rt_thread_startup(tid2);
		#endif
		
    return RT_EOK;
}
INIT_APP_EXPORT(sync_egg_init);