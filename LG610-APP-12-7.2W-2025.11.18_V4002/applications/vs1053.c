
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "mdebug.h"
#include "vs1053.h"
#include "drv_vs1053.h"

#define CS_PIN GET_PIN(A, 10)
#define DCS_PIN GET_PIN(C, 0)
#define DREQ_PIN GET_PIN(C, 6)
#define REST_PIN GET_PIN(C, 7)

#define VS_DQ          rt_pin_read(DREQ_PIN)     //输入

#define VS_XDCSH     rt_pin_write(DCS_PIN,1)   //输出 
#define VS_XDCSL     rt_pin_write(DCS_PIN,0)   //输出 

#define VS_RESTH     rt_pin_write(REST_PIN,1)   //输出 
#define VS_RESTL     rt_pin_write(REST_PIN,0)   //输出 

#define VS_XCSH  rt_pin_write(CS_PIN,1)   //输出 
#define VS_XCSL  rt_pin_write(CS_PIN,0)   //输出 



#define FILE_MAX_TYPE_NUM		6	//最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM		13	//最多FILE_MAX_SUBT_NUM个小类
ALIGN(RT_ALIGN_SIZE)

 //文件类型列表
uint8_t*const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
{"BIN"},			//BIN文件
{"LRC"},			//LRC文件
{"NES"},			//NES文件
{"TXT","C","H"},	//文本文件
{"MP1","MP2","MP3","MP4","M4A","3GP","3G2","OGG","AAC","WMA","WAV","MID","FLAC"},//音乐文件
{"BMP","JPG","JPEG","GIF"},//图片文件 
};

//VS10XX默认设置参数
_vs10xx_obj vsset=
{
	210,	//音量:220
	6,		//低音上线 60Hz
	15,		//低音提升 15dB	
	10,		//高音下限 10Khz	
	15,		//高音提升 10.5dB
	0,		//空间效果	
	1,		//板载喇叭默认打开.
};

//将小写字母转为大写字母,如果是数字,则保持不变.
uint8_t char_upper(uint8_t c)
{
	if(c<'A')return c;//数字,保持不变.
	if(c>='a')return c-0x20;//变为大写.
	else return c;//大写,保持不变
}	

//报告文件的类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
uint8_t f_typetell(uint8_t *fname)
{
	uint8_t tbuf[5];
	uint8_t *attr='\0';//后缀名
	uint8_t i=0,j;
	while(i<250)
	{
		i++;
		if(*fname=='\0')break;//偏移到了最后了.
		fname++;
	}
	if(i==250)return 0XFF;//错误的字符串.
 	for(i=0;i<5;i++)//得到后缀名
	{
		fname--;
		if(*fname=='.')
		{
			fname++;
			attr=fname;
			break;
		}
  	}
	strcpy((char *)tbuf,(const char*)attr);//copy
 	for(i=0;i<4;i++)tbuf[i]=char_upper(tbuf[i]);//全部变为大写 
	for(i=0;i<FILE_MAX_TYPE_NUM;i++)	//大类对比
	{
		for(j=0;j<FILE_MAX_SUBT_NUM;j++)//子类对比
		{
			if(*FILE_TYPE_TBL[i][j]==0)break;//此组已经没有可对比的成员了.
			if(strcmp((const char *)FILE_TYPE_TBL[i][j],(const char *)tbuf)==0)//找到了
			{
				return (i<<4)|j;
			}
		}
	}
	return 0XFF;//没找到		 			   
}	 

rt_err_t vs1053_init()
{
	 rt_err_t result;
	
	 rt_pin_mode(CS_PIN,   PIN_MODE_OUTPUT);
	 rt_pin_mode(DCS_PIN,  PIN_MODE_OUTPUT);
	 rt_pin_mode(REST_PIN, PIN_MODE_OUTPUT);
	 rt_pin_mode(DREQ_PIN, PIN_MODE_INPUT_PULLUP);
   result = rt_vs1053_dev_init(VS1053_CS_PORT,GPIO_PIN_15); //CS  脚不能用
	 return result;
}

//向VS10XX写命令
//address:命令地址
//data:命令数据
void VS_WR_Cmd(uint8_t address,uint16_t data)
{  
	uint8_t wbuf[10];
	uint8_t rbuf[10];
	while(VS_DQ==0);//等待空闲		  
	VS_XDCSH; 
	VS_XCSL;
	wbuf[0]= VS_WRITE_COMMAND;
	wbuf[1]= address;
	wbuf[2]= data>>8;
	wbuf[3]= data &0x00ff;
  vs1053_transfer(wbuf,rbuf,4);
	VS_XCSH;
} 

      
//address：寄存器地址
//返回值：读到的值

uint16_t VS_RD_Reg(uint8_t address)
{ 
	
	uint8_t wbuf[10];
	uint8_t rbuf[10];
	
	uint16_t temp=0;    	
  uint8_t dat_hi;
	uint8_t dat_lo;
  while(VS_DQ==0);//非等待空闲状态 		  
	VS_XDCSH;       
  VS_XCSL;
	
	rbuf[2]=0;
	rbuf[3]=0;
	wbuf[0]= VS_READ_COMMAND;
	wbuf[1]= address;
	wbuf[2]=0;
	wbuf[3]=0;
  vs1053_transfer(wbuf,rbuf,4);
	temp = rbuf[2]<<8;
	temp += rbuf[3];


	
	VS_XCSH;
  return temp; 
}



//设定VS10XX播放的音量和高低音
//volx:音量大小(0~254)
void VS_Set_Vol(uint8_t volx)
{
    uint16_t volt=0; 			//暂存音量值
    volt=254-volx;			//取反一下,得到最大值,表示最大的表示 
	  volt<<=8;
    volt+=254-volx;			//得到音量设置后大小
    VS_WR_Cmd(SPI_VOL,volt);//设音量 
}



//设定高低音控制
//bfreq:低频上限频率	2~15(单位:10Hz)
//bass:低频增益			0~15(单位:1dB)
//tfreq:高频下限频率 	1~15(单位:Khz)
//treble:高频增益  	 	0~15(单位:1.5dB,小于9的时候为负数)
void VS_Set_Bass(uint8_t bfreq,uint8_t bass,uint8_t tfreq,uint8_t treble)
{
    uint16_t bass_set=0; //暂存音调寄存器值
    signed char temp=0;   	 
	if(treble==0)temp=0;	   		//变换
	else if(treble>8)temp=treble-8;
 	else temp=treble-9;  
	bass_set=temp&0X0F;				//高音设定
	bass_set<<=4;
	bass_set+=tfreq&0xf;			//高音下限频率
	bass_set<<=4;
	bass_set+=bass&0xf;				//低音设定
	bass_set<<=4;
	bass_set+=bfreq&0xf;			//低音上限    
	VS_WR_Cmd(SPI_BASS,bass_set);	//BASS 
}


//设定音效
//eft:0,关闭;1,最小;2,中等;3,最大.
void VS_Set_Effect(uint8_t eft)
{
	uint16_t temp;	 
	temp=VS_RD_Reg(SPI_MODE);	//读取SPI_MODE的内容
	if(eft&0X01)temp|=1<<4;		//设定LO
	else temp&=~(1<<5);			//取消LO
	if(eft&0X02)temp|=1<<7;		//设定HO
	else temp&=~(1<<7);			//取消HO						   
	VS_WR_Cmd(SPI_MODE,temp);	//设定模式    
}	 
//重设解码时间                          
void VS_Reset_DecodeTime(void)
{
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);
	VS_WR_Cmd(SPI_DECODE_TIME,0x0000);//操作两次
}
//vs10xx装载patch.
//patch：patch首地址
//len：patch长度
void VS_Load_Patch(uint16_t *patch,uint16_t len) 
{
	uint16_t i; 
	uint16_t addr, n, val; 	  			   
	for(i=0;i<len;) 
	{ 
		addr = patch[i++]; 
		n    = patch[i++]; 
		if(n & 0x8000U) //RLE run, replicate n samples 
		{ 
			n  &= 0x7FFF; 
			val = patch[i++]; 
			while(n--)VS_WR_Cmd(addr, val);  
		}else //copy run, copy n sample 
		{ 
			while(n--) 
			{ 
				val = patch[i++]; 
				VS_WR_Cmd(addr, val); 
			} 
		} 
	} 	
} 

uint8_t VS_Send_MusicData(uint8_t* wbuf,uint16_t len)
{
	uint8_t n;

	if(VS_DQ!=0)  //送数据给VS10XX
	{			   	 
		VS_XDCSL; 
    //VS_XCSL;		
		//	VS_SPI_ReadWriteByte(buf[n]);	 			
		vs1053_transfer(wbuf,RT_NULL,len);
		VS_XDCSH;  
    //VS_XCSH;		
	}
	else
	{
		return 1;
	}
	return 0;//成功发送了
}

//设置音量,音效等.
void VS_Set_All(void) 				
{			 
	VS_Set_Vol(vsset.mvol);			//设置音量
	VS_Set_Bass(vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);  
	VS_Set_Effect(vsset.effect);	//设置空间效果
	//VS_SPK_Set(vsset.speakersw);	//控制板载喇叭状态 
}
////////////////////////////////////////////////////////////////////////////////	 	  
//软复位VS10XX
void VS_Soft_Reset(void)
{	 
	uint8_t retry=0;  				   
	while(VS_DQ==0); 					//等待软件复位结束	   
	retry=0;
	rt_kprintf("VS_RD_Reg(SPI_MODE)=%04x\n",VS_RD_Reg(SPI_MODE));
	rt_kprintf("VS_RD_Reg(SPI_MODE)=%04x\n",VS_RD_Reg(1));
	rt_kprintf("VS_RD_Reg(SPI_MODE)=%04x\n",VS_RD_Reg(2));
	rt_kprintf("VS_RD_Reg(SPI_MODE)=%04x\n",VS_RD_Reg(3));
	while(VS_RD_Reg(SPI_MODE)!=0x0800)	// 软件复位,新模式  
	{
		VS_WR_Cmd(SPI_MODE,0x0804);		// 软件复位,新模式	    
		rt_thread_mdelay(2);//等待至少1.35ms 
		if(retry++>100)break; 	    
	}	 		 
	while(VS_DQ==0);//等待软件复位结束	 
	retry=0;
	while(VS_RD_Reg(SPI_CLOCKF)!=0XF800)//设置VS10XX的时钟,4.5倍频 ,1.5xADD 
	{
		VS_WR_Cmd(SPI_CLOCKF,0XF800);	//设置VS10XX的时钟,4.5倍频 ,1.5xADD
		if(retry++>100)break; 	    
	}	 
	rt_thread_mdelay(20);
} 
//硬复位MP3
//返回1:复位失败;0:复位成功	   
uint8_t VS_HD_Reset(void)
{
	uint8_t retry=0;
	VS_RESTL;
	rt_thread_mdelay(20);
	VS_XDCSH;//取消数据传输
	VS_XCSH;
	VS_RESTH;	   
	while(VS_DQ==0&&retry<200)//等待DREQ为高
	{
		retry++;
		rt_thread_mdelay(5);
	};
	rt_thread_mdelay(20);	
	if(retry>=200)return 1;
	else return 0;	    		 
}


void inter_alarm(uint8_t dat)
{
 
  uint8_t wbuf[10];
	uint8_t rbuf[10];
	
	dat |= 0x20;
	VS_HD_Reset();	 
	VS_WR_Cmd(0x0b,0X2020);	  //设置音量	 
 	VS_WR_Cmd(SPI_MODE,0x0820);//进入VS10XX的测试模式     
	while(VS_DQ==0);     //等待DREQ为高
	//printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
 	//向VS10XX发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//其中n = 0x24, 设定VS10XX所产生的正弦波的频率值，具体计算方法见VS10XX的datasheet

	VS_XDCSL;//选中数据传输
	
	wbuf[0]= 0x53;
	wbuf[1]= 0xef;
	wbuf[2]= 0x6e;
	wbuf[3]= dat;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	VS_XDCSH;
}
void inter_alarm_stop(uint8_t dat)
{
	uint8_t wbuf[10];
	uint8_t rbuf[10];	
	
	VS_XDCSL;//选中数据传输
	wbuf[0]= 0x45;
	wbuf[1]= 0x78;
	wbuf[2]= 0x69;
	wbuf[3]= 0x74;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	rt_thread_mdelay(1000);
	VS_XDCSH; 
}
void VS_Sine_Test(void)
{	

  uint8_t wbuf[10];
	uint8_t rbuf[10];
	
	VS_HD_Reset();	 
	VS_WR_Cmd(0x0b,0X2020);	  //设置音量	 
 	VS_WR_Cmd(SPI_MODE,0x0820);//进入VS10XX的测试模式     
	while(VS_DQ==0);     //等待DREQ为高
	//printf("mode sin:%x\n",VS_RD_Reg(SPI_MODE));
 	//向VS10XX发送正弦测试命令：0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
 	//其中n = 0x24, 设定VS10XX所产生的正弦波的频率值，具体计算方法见VS10XX的datasheet

	VS_XDCSL;//选中数据传输
	
	wbuf[0]= 0x53;
	wbuf[1]= 0xef;
	wbuf[2]= 0x6e;
	wbuf[3]= 0x24;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	rt_thread_mdelay(1000);
	VS_XDCSH; 
	
	//退出正弦测试
	VS_XDCSL;//选中数据传输
	wbuf[0]= 0x45;
	wbuf[1]= 0x78;
	wbuf[2]= 0x69;
	wbuf[3]= 0x74;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	rt_thread_mdelay(1000);
	VS_XDCSH; 	
	return;
    //再次进入正弦测试并设置n值为0x44，即将正弦波的频率设置为另外的值

	VS_XDCSL;//选中数据传输
	wbuf[0]= 0x53;
	wbuf[1]= 0xef;
	wbuf[2]= 0x6e;
	wbuf[3]= 0x14;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	rt_thread_mdelay(1000);
	VS_XDCSH; 	
	
    //退出正弦测试
		
	VS_XDCSL;//选中数据传输
	wbuf[0]= 0x45;
	wbuf[1]= 0x78;
	wbuf[2]= 0x69;
	wbuf[3]= 0x74;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
	rt_thread_mdelay(1000);
	VS_XDCSH; 
		
}


uint16_t VS_Ram_Test(void)
{ 
	uint8_t wbuf[10];
	uint8_t rbuf[10];
	uint16_t result;
	VS_HD_Reset();     
 	VS_WR_Cmd(SPI_MODE,0x0820);// 进入VS10XX的测试模式
	while (VS_DQ==0); // 等待DREQ为高			   

	VS_XDCSL;	       		    // xDCS = 1，选择VS10XX的数据接口
	while(VS_DQ==0);//等待空闲		  
	VS_XDCSH; 
	VS_XCSL;
	wbuf[0]= 0x4d;
	wbuf[1]= 0xea;
	wbuf[2]= 0x6d;
	wbuf[3]= 0x54;
	wbuf[4]= 0x00;
	wbuf[5]= 0x00;
	wbuf[6]= 0x00;
	wbuf[7]= 0x00;
  vs1053_transfer(wbuf,rbuf,8);
//	vs1053_write_byte(0x4d);
//	vs1053_write_byte(0xea);
//	vs1053_write_byte(0x6d);
//	vs1053_write_byte(0x54);
//	vs1053_read_byte(0x00);
//	vs1053_read_byte(0x00);
//	vs1053_read_byte(0x00);
//	vs1053_read_byte(0x00);
	rt_thread_mdelay(150);  
	VS_XDCSH;
	result = VS_RD_Reg(SPI_HDAT0);      // VS1003如果得到的值为0x807F，则表明完好;VS1053为0X83FF.;      
	rt_kprintf("test ram=%04x\n",result);   // VS1003如果得到的值为0x807F，则表明完好;VS1053为0X83FF.;      
	return result;
} 