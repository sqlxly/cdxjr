

#include "oledfont.h"
#include "rtthread.h"
#include <rtdevice.h>
#include "mdebug.h"
#include "recode_dat.h"

#define SOFTWARE  "1.1"      //版本 1.0

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据
#define MAX_COLUMN 128


#define KEY_PAGE_BACK 0
#define KEY_PAGE_UP 1
#define KEY_PAGE_DOWN 2
#define KEY_OK 3

struct disp_dat_type{
	
	uint8_t page_pos;
	uint8_t edit_flag;
	uint8_t index;
	uint32_t tick;
};

static uint8_t board_addr=0;
static struct disp_dat_type disp;

rt_err_t IIC1_READ(rt_uint8_t *data,rt_uint8_t len,rt_uint8_t add);
rt_err_t IIC1_WRITE(rt_uint8_t *data,rt_uint8_t len, rt_uint8_t add);

/**********************************************
// IIC Write Command
**********************************************/
#define OLED_ADDR 0X78>>1
void Write_IIC_Command(unsigned char IIC_Command)
{

	 uint8_t buf[2]={0x00,0x00};
	 buf[1]= IIC_Command;
	 IIC1_WRITE(buf,2,OLED_ADDR);
}
/**********************************************
// IIC Write Data
**********************************************/
void Write_IIC_Data(unsigned char IIC_Data)
{
   uint8_t buf[2]={0x40,0x00};
	 buf[1]= IIC_Data;
   IIC1_WRITE(buf,2,OLED_ADDR);
}


void OLED_WR_Byte(uint8_t dat, uint8_t cmd)
{
	
	
	if(cmd)	Write_IIC_Data(dat);
	else Write_IIC_Command(dat);
 

}


//坐标设置

void OLED_Set_Pos(uint8_t x, uint8_t y) 
{ 	
		uint8_t buf[4];
		
	  buf[0] = 0x00;
		buf[1] = 0xb0+y;
	  buf[2] = ((x&0xf0)>>4)|0x10;
	  buf[3] = (x&0x0f);
	  IIC1_WRITE(buf,4,OLED_ADDR);
	
		//OLED_WR_Byte(0xb0+y,OLED_CMD);
		//OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
		//OLED_WR_Byte((x&0x0f),OLED_CMD); 
}   	

void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no)
{      			    
		uint8_t t,adder=0;
		uint8_t *buf;
		buf = (uint8_t *)rt_malloc(20);
		if(buf == RT_NULL) return;
	  rt_memset(buf,0,20);
	  buf[0]= 0x40;
		OLED_Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
			 buf[t+1] = Hzk[2*no][t];
    }	
		IIC1_WRITE(buf,17,OLED_ADDR);
		OLED_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
		{	
				buf[t+1] = Hzk[2*no+1][t];
    }	
	  IIC1_WRITE(buf,17,OLED_ADDR);
		rt_free(buf);
}
void OLED_ShowCHinese_select(uint8_t x, uint8_t y, uint8_t no)
{      			    
		uint8_t t,adder=0;
		uint8_t *buf;
		buf = (uint8_t *)rt_malloc(20);
		if(buf == RT_NULL) return;
	  rt_memset(buf,0,20);
	  buf[0]= 0x40;
		OLED_Set_Pos(x,y);	
    for(t=0;t<16;t++)
		{
			 buf[t+1] = ~Hzk[2*no][t];
    }	
		IIC1_WRITE(buf,17,OLED_ADDR);
		OLED_Set_Pos(x,y+1);	
    for(t=0;t<16;t++)
		{	
				buf[t+1] = ~Hzk[2*no+1][t];
    }	
	  IIC1_WRITE(buf,17,OLED_ADDR);
		rt_free(buf);
}


/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{ 	
 unsigned int j=0;
 unsigned char x,y;
  
  if(y1%8==0) y=y1/8;      
  else y=y1/8+1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
	    {      
	    	OLED_WR_Byte(BMP[j++],OLED_DATA);	    	
	    }
	}
} 


//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size)
{      	
	unsigned char c=0,i=0;	
	uint8_t buf[20];
	
	  buf[0]=0x40;
		c=chr-' ';//得到偏移后的值			
		if(x>MAX_COLUMN-1){x=0;y=y+2;}
		if(Char_Size ==16)
			{
			OLED_Set_Pos(x,y);	
			for(i=0;i<8;i++)
			{
				buf[i+1] = F8X16[c*16+i];
			}
			IIC1_WRITE(buf,9,OLED_ADDR);
			OLED_Set_Pos(x,y+1);
			for(i=0;i<8;i++)
			{
				buf[i+1] = F8X16[c*16+i+8];
			}
			IIC1_WRITE(buf,9,OLED_ADDR);
			}
			else {	
				OLED_Set_Pos(x,y);
				for(i=0;i<6;i++)
				{
						buf[i+1] = F6x8[c][i];
				}
				IIC1_WRITE(buf,7,OLED_ADDR);
			}
}


//显示一个字符号串
void OLED_ShowString(uint8_t x, uint8_t y, char *chr, uint8_t Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{
			OLED_ShowChar(x,y,chr[j],Char_Size);
			x+=8;
			if(x>120){x=0;y+=2;}
			j++;
	}
}

void OLED_DrawBlankPixel(uint8_t x, uint8_t y, uint8_t Pixel_Size)
{
		uint8_t t,adder=0;
		uint8_t *buf=NULL;
		buf = (uint8_t *) rt_malloc(Pixel_Size+5);
		if(buf == RT_NULL) 
			return;
		
		OLED_Set_Pos(x,y);	
		rt_memset(buf,0,Pixel_Size+1);
		buf[0] = 0x40;	
	  IIC1_WRITE(buf,Pixel_Size+1,OLED_ADDR);
		OLED_Set_Pos(x,y+1);	
		IIC1_WRITE(buf,Pixel_Size+1,OLED_ADDR);
		rt_free(buf);

}

void OLED_DrawWhitePixel(uint8_t x, uint8_t y, uint8_t Pixel_Size)
{
		uint8_t t,adder=0;
		uint8_t *buf=NULL;
		buf = (uint8_t *) rt_malloc(Pixel_Size+5);
		if(buf == RT_NULL) 
			return;
		
		OLED_Set_Pos(x,y);	
		rt_memset(buf,0xff,Pixel_Size+1);
	  IIC1_WRITE(buf,Pixel_Size+1,OLED_ADDR);
		OLED_Set_Pos(x,y+1);	
		IIC1_WRITE(buf,Pixel_Size+1,OLED_ADDR);
		rt_free(buf);

}






//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	uint8_t i,n;		    
	uint8_t *buf;
	buf =(uint8_t *)rt_malloc(138);
	if(buf == RT_NULL) return;
	for(i=0;i<8;i++)  
	{  
		buf[0] = 0;
		buf[1] = 0xb0+i;
		buf[2] = 0;
		buf[3] = 0x10;
		IIC1_WRITE(buf,4,OLED_ADDR);
		rt_memset(buf,0,128+1);
		buf[0]= 0x40;
		IIC1_WRITE(buf,128+1,OLED_ADDR);
	} 
	rt_free(buf);
}
void OLED_On(void)  
{  
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)OLED_WR_Byte(1,OLED_DATA); 
	} 
}

void Main_Page_Display()
{	
	OLED_ShowCHinese(0,1,8);					  //分
	OLED_DrawBlankPixel(16, 1, 2);      
	OLED_ShowCHinese(18,1,9);					//级
	OLED_DrawBlankPixel(34, 1, 2);      
	OLED_ShowCHinese(36,1,11);					//包
	OLED_DrawBlankPixel(52, 1, 2);      
	OLED_ShowCHinese(54,1,12);					//装
	OLED_DrawBlankPixel(70, 1, 2);      
	OLED_ShowCHinese(72,1,13);					//
	OLED_DrawBlankPixel(88, 1, 2);      
	OLED_ShowCHinese(90,1,14);					//
	OLED_DrawBlankPixel(106, 1, 2);      
	OLED_ShowCHinese(108,1,15);					//
	OLED_DrawBlankPixel(124, 1, 4);      
}

void disp_save_scuess()
{	
	OLED_ShowCHinese(0,1,25);					   
	OLED_DrawBlankPixel(16, 1, 2);      
	OLED_ShowCHinese(18,1,28);					 
	OLED_DrawBlankPixel(34, 1, 2);      
	OLED_ShowCHinese(36,1,29);					 
	OLED_DrawBlankPixel(52, 1, 2);      
	OLED_ShowCHinese(54,1,30);	
  OLED_DrawBlankPixel(80, 1, 48);      	
}
void disp_save_fall()
{	
	OLED_ShowCHinese(0,1,25);					   
	OLED_DrawBlankPixel(16, 1, 2);      
	OLED_ShowCHinese(18,1,28);					 
	OLED_DrawBlankPixel(34, 1, 2);      
	OLED_ShowCHinese(36,1,31);					 
	OLED_DrawBlankPixel(52, 1, 2);      
	OLED_ShowCHinese(54,1,32);		
  OLED_DrawBlankPixel(80, 1, 48);      		
}

static void disp_software()   //SOFTWARE
{
	
		OLED_ShowChar(0, 1, '2', 16);
		OLED_ShowChar(8, 1, '.', 16);
		OLED_DrawBlankPixel(16, 1, 2);      
		OLED_ShowCHinese(18,1,4);				
		OLED_DrawBlankPixel(34, 1, 2);   
		OLED_ShowCHinese(36,1,5);				
		OLED_DrawBlankPixel(52, 1, 2); 
		OLED_ShowCHinese(54,1,6);				
		OLED_DrawBlankPixel(70, 1, 2);  
		OLED_ShowCHinese(72,1,7);		
		OLED_DrawBlankPixel(88, 1, 2);  
		OLED_ShowChar(90, 1, ':', 16);	//
	  OLED_DrawBlankPixel(98, 1, 1);  
		OLED_ShowChar(99, 1, SOFTWARE[0], 16);
	  OLED_DrawBlankPixel(107, 1, 1);  
		OLED_ShowChar(108, 1, SOFTWARE[1], 16);
	  OLED_DrawBlankPixel(116, 1, 1);  
		OLED_ShowChar(117, 1, SOFTWARE[2], 16);

}
void disp_exit()
{
	OLED_ShowChar(0, 1, '3', 16);
	OLED_ShowChar(8, 1, '.', 16);
	OLED_DrawBlankPixel(16, 1, 1);      
	OLED_ShowCHinese(17,1,22);				
	OLED_DrawBlankPixel(33, 1, 1); 
  OLED_ShowCHinese(34,1,23);				
}
void disp_cancel()
{
	OLED_DrawBlankPixel(0, 1, 4);      
	OLED_ShowCHinese(4,1,26);				
	OLED_DrawBlankPixel(20, 1, 2); 
  OLED_ShowCHinese(22,1,27);				
}
void disp_save_addr(uint8_t index)
{
	if(index ==1)
	{
		OLED_DrawBlankPixel(0, 1, 4);      
		OLED_ShowCHinese_select(4,1,24);	
    OLED_DrawWhitePixel(20,1,1);
    OLED_DrawWhitePixel(21,1,1);			
		OLED_ShowCHinese_select(22,1,25);		
		
		OLED_DrawBlankPixel(90, 1, 4);      
		OLED_ShowCHinese(94,1,26);
		OLED_ShowCHinese(112,1,27);		
	}
	else 
	{
		OLED_DrawBlankPixel(0, 1, 4);      
		OLED_ShowCHinese(4,1,24);
    OLED_DrawBlankPixel(20, 1, 2); 		
		OLED_ShowCHinese(22,1,25);		
		
		OLED_DrawBlankPixel(90, 1, 4);      
		OLED_ShowCHinese_select(94,1,26);			
    OLED_DrawWhitePixel(110,1,1);	
    OLED_DrawWhitePixel(111,1,1);			
		OLED_ShowCHinese_select(112,1,27);		
	}	
}

void disp_board_addr(uint8_t addr)
{
	  char buf[10];
		OLED_DrawBlankPixel(0, 1, 4);      
		OLED_ShowCHinese(4,1,20);				
		OLED_DrawBlankPixel(20, 1, 2);   
		OLED_ShowCHinese(22,1,21);				
		OLED_DrawBlankPixel(38, 1, 2); 
		OLED_ShowChar(40, 1, ':', 16);	//
	  sprintf(&buf[0],"%d",addr);
		OLED_DrawBlankPixel(48, 1, 1);  
		OLED_ShowChar(49, 1, buf[0], 16);
}
void disp_set_board_addr(uint8_t addr)
{
	  char buf[10];
		OLED_ShowChar(0, 1, '1', 16);
		OLED_ShowChar(8, 1, '.', 16);
		OLED_DrawBlankPixel(16, 1, 2); 
		OLED_ShowCHinese(18,1,18);				
		OLED_DrawBlankPixel(34, 1, 2);  
		OLED_ShowCHinese(36,1,19);		
		OLED_DrawBlankPixel(52, 1, 2);  
		OLED_ShowCHinese(54,1,20);	
		OLED_DrawBlankPixel(70, 1, 2);  
		OLED_ShowCHinese(72,1,21);	
	  OLED_DrawBlankPixel(88, 1, 2); 
		OLED_ShowChar(90, 1, ':', 16);	//
	  sprintf(&buf[0],"%d",addr);
		OLED_DrawBlankPixel(98, 1, 1);  
		OLED_ShowChar(99, 1, buf[0], 16);

}


// 菜单设置板卡ID
void menu_disp(uint8_t index)
{	
	if(index==1)
	{
		board_addr = read_board_addr();
		disp_set_board_addr(board_addr);
	}
	else if(index==2)
	{
		disp_software();
	}
	else if(index==3) 
	{
		disp_exit();
	}
	
}

extern void eth_set_addr(uint8_t board_addr);
void press_key()
{
	
	if(disp.page_pos==1)
	{
		
		disp.index=1;
		disp.page_pos=11;       
	}
	else if(disp.page_pos==11)
	{
		if(disp.index ==3)
		{
			disp.index=1;
			disp.page_pos=1;       
		}
		else if(disp.index ==1)
		{
			disp.index=1;
			board_addr = read_board_addr();
			disp.page_pos=30;    
      			
		}
	}
	else if(disp.page_pos==30)
	{
		disp.index=1;
		disp.page_pos =31;
	}
	else if(disp.page_pos==31)
	{
		if(disp.index==1)
		{
			
	    set_board_addr(board_addr);
			if(read_board_addr()==board_addr)
			{
			  disp_save_scuess();
				disp.index=1;
				disp.page_pos =1;
				eth_set_addr(board_addr);
			}
			else 
			{
			  disp_save_fall();
			}
		}
		else if(disp.index==2)
		{
			disp.index=1;
			disp.page_pos =1;
		}
		rt_thread_mdelay(600);
	}
}

void turn_up_key()
{
	if(disp.page_pos==11)
	{
		disp.index--;
    if(disp.index==0)  
			disp.index =3;
	}
	else if(disp.page_pos==30)
	{
		board_addr--;
		if(board_addr==0)
			board_addr =9;
	}
	else if(disp.page_pos==31)
	{
		disp.index = (disp.index==1)? 2:1;
	}
}
void turn_down_key()
{
	if(disp.page_pos==11)
	{
		disp.index++;
    if(disp.index>3)  
			disp.index =1;
	}
	else if(disp.page_pos==30)
	{
		board_addr++;
		if(board_addr>9)
			board_addr =0;
	}
	else if(disp.page_pos==31)
	{
		disp.index = (disp.index==1)? 2:1;
	}
}

static uint8_t key_val=0;
void set_key_val(uint8_t key)
{
	key_val = key;
}

void analysis_key(uint8_t key)
{
	rt_kprintf("recive key=%d\n",key);
	if(key==1)
	{
		press_key();
	}
	else if(key==2)
	{
	  turn_down_key();
	}
	else if(key==3)
	{
		turn_up_key();
		
	}
}


void oled_reflash()
{
	static uint8_t up_disp=0;
	static uint8_t up_index=0;
	
	if((up_disp != disp.page_pos)||(up_index !=disp.index))
	{
		
		up_index = disp.index;
		up_disp = disp.page_pos;
		OLED_Clear();
	}
	if(disp.page_pos==1)
	{
		Main_Page_Display();
	}
	else if(disp.page_pos==11)
	{
		menu_disp(disp.index);
	}
	else if(disp.page_pos==30)
	{
		disp_board_addr(board_addr);
	}
	else if(disp.page_pos==31)
	{

		disp_save_addr(disp.index);

	}
}
//OLED 显示界面主函数
void oled_display(void *parts)
{
	rt_tick_t ctime=0;
	disp.page_pos = 1;
	disp.edit_flag = 0;
	disp.tick = rt_tick_get();
	while(1)
	{
		if(key_val !=0)
		{
			disp.tick = rt_tick_get();
			analysis_key(key_val);
			key_val =0;
		}
		
		if((rt_tick_get() - disp.tick)>5000)
		{
			disp.page_pos = 1;
			disp.index=1;
		}
		oled_reflash();
		rt_thread_mdelay(100);
	}
}

int fun_oled_display(void)
{

	rt_thread_t tid1 = RT_NULL;
	
	
	OLED_WR_Byte(0xAE,OLED_CMD);																	//关闭显示
	OLED_WR_Byte(0x40,OLED_CMD);																	//---set low column address
	OLED_WR_Byte(0xB0,OLED_CMD);																	//---set high column address
	OLED_WR_Byte(0xC8,OLED_CMD);																	//-not offset
	OLED_WR_Byte(0x81,OLED_CMD);																	//设置对比度
	OLED_WR_Byte(0xff,OLED_CMD);																	
	OLED_WR_Byte(0xa1,OLED_CMD);																	//段重定向设置
	OLED_WR_Byte(0xa6,OLED_CMD);																	//
	OLED_WR_Byte(0xa8,OLED_CMD);																	//设置驱动路数
	OLED_WR_Byte(0x1f,OLED_CMD);
	OLED_WR_Byte(0xd3,OLED_CMD);
	OLED_WR_Byte(0x00,OLED_CMD);
	OLED_WR_Byte(0xd5,OLED_CMD);
	OLED_WR_Byte(0xf0,OLED_CMD);
	OLED_WR_Byte(0xd9,OLED_CMD);
	OLED_WR_Byte(0x22,OLED_CMD);
	OLED_WR_Byte(0xda,OLED_CMD);
	OLED_WR_Byte(0x02,OLED_CMD);
	OLED_WR_Byte(0xdb,OLED_CMD);
	OLED_WR_Byte(0x49,OLED_CMD);
	OLED_WR_Byte(0x8d,OLED_CMD);
	OLED_WR_Byte(0x14,OLED_CMD);
	OLED_WR_Byte(0xaf,OLED_CMD);
	OLED_Clear();

  tid1 = rt_thread_create("oled_display", oled_display, RT_NULL,1024, RT_THREAD_PRIORITY_MAX-1, 10);
  if (tid1 != RT_NULL)
  {
     rt_thread_startup(tid1);
  }
  else
  {
    rt_kprintf("thread oled display creat fall!!!!!!!!!\n");
  }
}


static struct rt_i2c_bus_device *i2c_bus = RT_NULL;    
#define EEP_I2C_BUS_NAME          "i2c1" 
void IIC1_INIT()
{
	 i2c_bus = rt_i2c_bus_device_find(EEP_I2C_BUS_NAME);     
		
    if(i2c_bus == RT_NULL)
    {
        DEBUG_ERR("IIC1 初始化失败\n %s", EEP_I2C_BUS_NAME);
    }
		else
			DEBUG_CARE("IIC1 初始化成功\n %s", EEP_I2C_BUS_NAME);
}


 rt_err_t IIC1_READ(rt_uint8_t *data,rt_uint8_t len,rt_uint8_t add)
{
    struct rt_i2c_msg msgs;


    msgs.addr = add;
    msgs.flags = RT_I2C_RD;
    msgs.buf = data;
    msgs.len = len;

    if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1)
    {

        //msgs.buf[1] += 1;
        //rt_kprintf("read ok!\n");
        return RT_EOK;
    }
    else
    {
				//rt_kprintf("read fall addr=%x\n",add<<1);
        return -RT_ERROR;
    }
}

 rt_err_t IIC1_WRITE(rt_uint8_t *data,rt_uint8_t len, rt_uint8_t add)
{

    struct rt_i2c_msg msgs;

    msgs.addr = add;
    msgs.flags = RT_I2C_WR;
    msgs.buf = data;
    msgs.len = len;
    if (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1)
    {
        return RT_EOK;
    }
    else
    {
        return -RT_ERROR;
    }
}
INIT_DEVICE_EXPORT(IIC1_INIT);
INIT_APP_EXPORT(fun_oled_display);
