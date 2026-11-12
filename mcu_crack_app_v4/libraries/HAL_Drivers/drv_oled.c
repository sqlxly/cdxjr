#include <board.h>
#include "drv_oled.h"
#include "string.h"

static struct drv_lcd_device _lcd;

/*
*\屏延时函数
*\time:要延迟的us数
*/
static void lcd_delay_time(rt_uint32_t time)
{
	//系统时钟168MHz
	while(time--)
	{
		__NOP();
	}
}
/*
*\lcd引脚初始化
*/
static void lcd_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();    

    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    //OLED->CS
    GPIO_InitStructure.Pin = OLED_CS1_PIN;
    HAL_GPIO_Init(OLED_CS1_PORT,&GPIO_InitStructure);
    //FONT->CS
    GPIO_InitStructure.Pin = OLED_CS2_PIN;
    HAL_GPIO_Init(OLED_CS2_PORT,&GPIO_InitStructure);
    //OLED->DC
    GPIO_InitStructure.Pin = OLED_DC_PIN;
    HAL_GPIO_Init(OLED_DC_PORT,&GPIO_InitStructure);
    //SPI->SCK
    GPIO_InitStructure.Pin = OLED_SCK_PIN;
    HAL_GPIO_Init(OLED_SCK_PORT,&GPIO_InitStructure);
    //SPI->MOSI
    GPIO_InitStructure.Pin = OLED_MOSI_PIN;
    HAL_GPIO_Init(OLED_MOSI_PORT,&GPIO_InitStructure);    
    //SPI->MISO
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pin = OLED_MISO_PIN;
    HAL_GPIO_Init(OLED_MISO_PORT,&GPIO_InitStructure);   
}
/*
*\模拟SPI收发数据
*/
static rt_uint8_t soft_spi_rw(rt_uint8_t data)
{
    rt_uint8_t byte = 0;

    for(rt_uint8_t i=0;i<8;i++)
    {
        oled_sck_write(0);
        if(data & 0x80)
            oled_sdi_write(1);
        else
            oled_sdi_write(0);
        data <<= 1;
        lcd_delay_time(1);
        
        oled_sck_write(1);
        byte <<= 1;
        if(oled_sdo_read() != 0)
            byte |= 0x01;  
        lcd_delay_time(1);
    }

    return byte;
}
/*
*\向LCD发送指令
*/
static void lcd_write_command(rt_uint8_t cmd)
{
	oled_dc_write(0);
    oled_cs_write(0);
    soft_spi_rw(cmd);
    oled_cs_write(1);
}
/*
*\向屏发送数据
*/
static void lcd_write_data(rt_uint8_t data)
{
	oled_dc_write(1);
    oled_cs_write(0);
    soft_spi_rw(data);
    oled_cs_write(1);
}
/*
*\向字库发送指令
*\cmd:指令
*/
static void font_write_cmd(rt_uint8_t cmd)
{
    soft_spi_rw(cmd);
}
/*
*\读取字库数据
*\data:数据缓存
*\len:要读取的长度
*\return:读到数据长度
*/
static rt_uint8_t font_read_data(void)
{
    rt_uint8_t buf_dat_r = 0,buf_dat_t = 0;	
    
	buf_dat_t = 0xFF;
	buf_dat_r = soft_spi_rw(buf_dat_t);		
    
	return buf_dat_r;
}
/*
*\获取字库数据
*/
static void get_font_data(long addr,unsigned char *data,int len)
{
    font_cs_write(0);

    font_write_cmd(0x03);

    font_write_cmd((rt_uint8_t)(addr >> 16));

    font_write_cmd((rt_uint8_t)(addr >> 8));

    font_write_cmd((rt_uint8_t)addr);

    for(rt_uint16_t i=0;i<len;i++)
    {
        data[i] = font_read_data();
    }

    font_cs_write(1);
}

/*
*\设置坐标
*\写LCD行列地址:X 为起始的列地址，Y 为起始的行地址
*/
static void lcd_set_cursor(int column,int page)
{
	column += 2;
  lcd_write_command(((column&0xf0)>>4)|0x10);  //设置列地址的高4位
	lcd_write_command(column&0x0f);              //设置列地址的低4位
	lcd_write_command(0xb0+page);              	 //设置页地址
}
/*
*\清屏
*/
static void lcd_clear_screen(void)
{
	int i = 0,j = 0;
    
	for(i=0;i<LCD_Y_SIZE/8;i++)
	{
        lcd_set_cursor(0,i);
        for(j=0;j<LCD_X_SIZE;j++)
        {
            lcd_write_data(0);
        }
    }
}

/*
*\初始化LCD
*/
static rt_err_t drv_lcd_init(struct rt_device *device)
{
  lcd_gpio_init();

  lcd_delay_time(0x8FFFFF);
  	
  lcd_write_command(0xAE); /*display off*/ 
	lcd_write_command(0x02); /*set lower column address*/ 
	lcd_write_command(0x10); /*set higher column address*/ 
	lcd_write_command(0x40); /*set display start line*/ 
	lcd_write_command(0xB0); /*set page address*/
	lcd_write_command(0x81); /*contract control*/ 
	lcd_write_command(0xcf); /*128*/ 
	lcd_write_command(0xA1); /*set segment remap*/ 
	lcd_write_command(0xA6); /*normal / reverse*/ 
	lcd_write_command(0xA8); /*multiplex ratio*/ 
	lcd_write_command(0x3F); /*duty = 1/64*/ 
	lcd_write_command(0xad); /*set charge pump enable*/ 
	lcd_write_command(0x8b); /* 0x8B 内供 VCC */ 
  lcd_delay_time(0x8FFFFF);
    
	lcd_write_command(0x33); /*0X30---0X33 set VPP 9V */ 
	lcd_write_command(0xC8); /*Com scan direction*/ 
	lcd_write_command(0xD3); /*set display offset*/ 
	lcd_write_command(0x00); /* 0x20 */ 
	lcd_write_command(0xD5); /*set osc division*/ 
	lcd_write_command(0x80); 
	lcd_write_command(0xD9); /*set pre-charge period*/ 
	lcd_write_command(0x1f); /*0x22*/ 
	lcd_write_command(0xDA); /*set COM pins*/ 
	lcd_write_command(0x12); 
	lcd_write_command(0xdb); /*set vcomh*/ 
	lcd_write_command(0x40);
	lcd_clear_screen();
//	lcd_write_command(0xAF); /*display ON*/	
  lcd_write_command(0xAE); /*display off*/ 

  return RT_EOK;
}

/*
*\在屏上绘制点阵图形
*\column:列
*\page:页
*\p:图形数据
*\width:图形宽度
*\heigh:图形高度
*/
void lcd_draw_pic(int column,int page,unsigned char *p,int width,int heigh,unsigned char flag)
{
    rt_uint8_t k = 0,data = 0;

    k = heigh >> 3;
    if((heigh%8) != 0)
        k++;
     
    for(int j=0;j<k;j++)
    {
        lcd_set_cursor(column,page);
        
        for(int i=0; i<width; i++ )
        {
            switch(flag)
            {
                case 0:
                    data = ~*p;
                    break;
                    
                case 1:
                    data = *p;
                    break;
                
                case 2:
                    if((j == k - 1)&&(i >= width - 8))
                        data = *p | 0xC0;
                    else
                        data = *p;
                    
                    break;
                
                default:break;
            }
            lcd_write_data(data);
            p++;
        }
        page++;
    }
}
/*
*\填充显示区域
*\column:列
*\page:页
*\width:图形宽度
*\heigh:图形高度
*\flag:显示标识，填充空白或者黑色
*/
static void lcd_fill_area(int column,int page,int width,int heigh,unsigned char flag)
{
    rt_uint8_t k = 0,data = 0;

    k = heigh >> 3;
    if((heigh%8) != 0)
        k++;

    data = (flag == 0)?0xFF:0;
    
    for(int j=0;j<k;j++)
    {
        lcd_set_cursor(column,page);
        
        for(int i=0; i<width; i++ )
        {
            lcd_write_data(data);
        }
        page++;
    }
}
/*
*\开启/关闭显示
*\status:显示状态，0：关闭，1：开启
*/
static void lcd_display_on(rt_uint8_t status)
{
    if(status != 0)
        lcd_write_command(0xAF);
    else
        lcd_write_command(0xAE);
}
/*
*\lcd操作接口
*/
struct rt_device_graphic_ops fsmc_lcd_ops =
{
    lcd_clear_screen,
    lcd_display_on,
    lcd_fill_area,
    get_font_data,
    lcd_draw_pic,
};

/*
*\获取LCD设备信息
*/
static rt_err_t drv_lcd_control(struct rt_device *device, int cmd, void *args)
{
    struct drv_lcd_device *lcd = LCD_DEVICE(device);
    struct rt_device_graphic_info *info = (struct rt_device_graphic_info *)args;
    
    switch (cmd)
    {
        case RTGRAPHIC_CTRL_GET_INFO:
        {        
            RT_ASSERT(info != RT_NULL);
            
            //this needs to be replaced by the customer
            info->pixel_format  = lcd->lcd_info.pixel_format;
            info->bits_per_pixel = lcd->lcd_info.bits_per_pixel;
            info->width = LCD_X_SIZE;
            info->height = LCD_Y_SIZE;
        }
        break;
    }

    return RT_EOK;
}
/*
*\将LCD设备添加到系统设备框架
*/
int drv_lcd_hw_init(void)
{
    rt_err_t result = RT_EOK;
    struct rt_device *device = &_lcd.parent;
    /* memset _lcd to zero */
    memset(&_lcd, 0x00, sizeof(_lcd));

    _lcd.lcd_info.bits_per_pixel = 1;
    _lcd.lcd_info.pixel_format = RTGRAPHIC_PIXEL_FORMAT_MONO;

    device->type = RT_Device_Class_Graphic;
    device->init = drv_lcd_init;
    device->control = drv_lcd_control;

    device->user_data = &fsmc_lcd_ops;
    /* register lcd device */
    rt_device_register(device, "oled", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

    return result;
}

INIT_DEVICE_EXPORT(drv_lcd_hw_init);
