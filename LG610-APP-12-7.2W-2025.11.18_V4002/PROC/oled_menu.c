//#include "oled_gui.h"
#include "drv_key.h"
#include "common.h"
#include "font.h"
#include <stdio.h>
#include <board.h>
#include <netif/ethernetif.h>
#include <netdev.h>
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <u8g2_port.h>

#define OLED_I2C_PIN_SCL                    49  // PD1
#define OLED_I2C_PIN_SDA                    48  // PD0
typedef enum
{
    ANTI = 0,		//Invert Display
    NORMAL,			//normal Display
    UNDERLINE,      //underline
}display_mode;

static const unsigned char logo[] = {
0x00,0xF0,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x07,0x00,0x00,0xF0,0x03,0x00,0xF8,0xFF,0xFF,0xFF,0x01,0x80,0x1F,0x00,0x00,0xFE,0x0F,0x00,0x00,0xF0,0x03,0x00,0xF8,0xFF,0xFF,0xFF,0x01,0x80,0x1F,0x00,0x80,0xFF,0x1F,0x00,0x3F,0xF0,0x03,0x3F,0xF8,0x01,0x00,0x00,0x00,0x80,0x1F,0x00,
0x80,0xFF,0x3F,0x00,0x3F,0xF0,0x03,0x3F,0xF8,0x01,0x00,0x00,0x00,0x80,0x1F,0x00,0xC0,0xFF,0x7F,0x00,0x3F,0xF0,0x03,0x3F,0xF8,0x01,0x00,0x00,0x00,0x80,0x1F,0x00,0xC0,0xFF,0x7F,0x00,0x3F,0xF0,0x03,0x3F,0xF8,0xFF,0xFF,0x7F,0x00,0x80,0x1F,0x00,0xE0,0xFF,0x7F,0x00,0x3F,0xF0,0x03,0x3F,0xF8,0xFF,0xFF,0x7F,0x00,0x80,0x1F,0x00,
0xE0,0xFF,0x7F,0x00,0x3F,0xF0,0x03,0x7F,0xF8,0xFF,0xFF,0x7F,0x00,0xC0,0x3F,0x00,0xC0,0xFF,0x7F,0x80,0x3F,0xF0,0x03,0x7F,0xF8,0x01,0x00,0x7E,0x00,0xE0,0x7F,0x00,0xC0,0xFF,0x7F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0x01,0x00,0x7E,0x00,0xF0,0xFF,0x00,0xC0,0xFF,0x7F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0x01,0x00,0x7E,0x00,0xF8,0xFF,0x01,
0x80,0xFF,0x3F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0x01,0x00,0x7E,0x00,0xFC,0xFF,0x03,0x80,0xFF,0x1F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0xFF,0xFF,0x7F,0x00,0xFE,0xF9,0x07,0x00,0xFF,0x1F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0xFF,0xFF,0x7F,0x00,0xFF,0xF0,0x0F,0x00,0xFE,0x0F,0x80,0x1F,0xF0,0x03,0x7E,0xF8,0x01,0x00,0x00,0x80,0x7F,0xE0,0x1F,
0x00,0xF8,0x03,0x80,0x1F,0xF0,0x03,0xFE,0xF8,0x01,0x00,0x00,0x80,0x3F,0xC0,0x3F,0x28,0x00,0x40,0xC1,0x1F,0xF0,0x03,0xFC,0xF8,0x01,0x00,0x00,0xC0,0x1F,0x80,0x7F,0xFE,0x01,0xF8,0xCF,0x0F,0xF8,0x03,0xFC,0xF8,0xFF,0xFF,0xFF,0xE1,0x0F,0x00,0xFF,0xFF,0x03,0xF8,0xDF,0x0F,0xF8,0x01,0xFC,0xF8,0xFF,0xFF,0xFF,0xF1,0x07,0x00,0xFE,
0xFF,0x07,0xFE,0xDF,0x0F,0xF8,0x01,0xFC,0xF8,0xFF,0xFF,0xFF,0xF9,0x03,0x00,0xFC,0xFE,0x0F,0xFF,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x0F,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFC,0x1F,0xFF,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0xF8,0x1F,0xFF,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,0x9F,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x9F,0x7F,0xC0,0x80,0xC3,0x87,0x0F,0x03,0x3E,0x80,0x0F,0x0E,0x0C,0x98,0xE0,0xC0,0x1F,0x7F,0xC0,0x00,0x01,0x01,0x02,0x03,0x02,0x80,0x00,0x04,0x0C,0x98,0xC0,
0x80,0x0F,0x1F,0xC0,0x00,0x01,0x01,0x02,0x03,0x1E,0x80,0x00,0x04,0x1E,0xB8,0xC0,0x00,0x0F,0x0F,0xC0,0x00,0x01,0x01,0x02,0x03,0x02,0x80,0x18,0x04,0x12,0xE8,0xC0,0x00,0x0E,0x0E,0xC0,0x00,0x01,0x01,0x02,0x03,0x02,0x80,0x18,0x04,0x3F,0xC8,0xC0,0x00,0x04,0x06,0xC0,0x87,0x03,0x01,0x02,0x1F,0x3E,0x80,0x1F,0x0E,0x23,0xC8,0xC0
};
const unsigned char bmp_wired_13[26]=
{0x00,0x00,0xFE,0x0F,0xAA,0x0A,0xAA,0x0A,0xAA,0x0A,0x02,0x08,0x02,0x08,0x02,0x08,0x0E,0x0E,0x18,0x03,0xF0,0x01,0x00,0x00,0x00,0x00};
const unsigned char bmp_wired_error_13[26]=
{0x00,0x00,0xFE,0x0F,0xAA,0x0A,0xAA,0x0A,0xAA,0x0A,0x02,0x08,0xA2,0x08,0x42,0x08,0xAE,0x0E,0x18,0x03,0xF0,0x01,0x00,0x00,0x00,0x00};

const unsigned char bmp_lte_4_13[26]=
{0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x0C,0x80,0x0D,0x80,0x0D,0xB0,0x0D,0xB0,0x0D,0xB6,0x0D,0xB6,0x0D,0xB6,0x0D,0x00,0x00,0x00,0x00};
const unsigned char bmp_lte_3_13[26]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x80,0x01,0xB0,0x01,0xB0,0x01,0xB6,0x01,0xB6,0x01,0xB6,0x01,0x00,0x00,0x00,0x00};
const unsigned char bmp_lte_2_13[26]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x30,0x00,0x36,0x00,0x36,0x00,0x36,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_lte_1_13[26]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x00,0x06,0x00,0x06,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_lte_0_13[26]=
{0x00,0x00,0x00,0x00,0x24,0x0C,0x18,0x0C,0x98,0x0D,0xA4,0x0D,0x80,0x0D,0xB0,0x0D,0xB0,0x0D,0xB6,0x0D,0xB6,0x0D,0xB6,0x0D,0x00,0x00};
const unsigned char bmp_mqtt_13[26]=
{0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x04,0xFE,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0xFE,0x0F,0x04,0x00,0x08,0x00,0x10,0x00,0x00,0x00};
const unsigned char bmp_mqtt_error_13[26]=
{0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x04,0xFE,0x0F,0xA0,0x00,0x40,0x00,0xA0,0x00,0xFE,0x0F,0x04,0x00,0x08,0x00,0x10,0x00,0x00,0x00};
const unsigned char bmp_reg_error_13[26]=
{0x00,0x00,0x3F,0x0C,0x46,0x0C,0x86,0x0C,0x86,0x0C,0x86,0x0C,0x46,0x0C,0x3E,0x0C,0x26,0x00,0x46,0x0C,0x86,0x0C,0x0F,0x03,0x00,0x00};
const unsigned char bmp_reg_normal_13[26]=
{0x00,0x00,0x3F,0x00,0x46,0x00,0x86,0x00,0x86,0x00,0x86,0x00,0x46,0x00,0x3E,0x00,0x26,0x00,0x46,0x00,0x86,0x00,0x0F,0x03,0x00,0x00};
const unsigned char bmp_clear_13[26]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_ac2000_7_13[26]={0x00,0x00,0x00,0x41,0x22,0x14,0x08,0x14,0x22,0x41,0x00,0x00,0x00};
const unsigned char bmp_clear_7_13[26]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_clear_32x16[64]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_cursor[32]=
{
0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x05,0x00,0x09,0xFE,0x11,0x02,0x20,0x02,0xC0,0x02,0xC0,0x02,0x20,0xFE,0x11,0x00,0x09,0x00,0x05,0x00,0x03,0x00,0x01,0x00,0x00
};
const unsigned char bmp_cursor_8[8]=
{
0x10,0x30,0x5F,0x81,0x81,0x5F,0x30,0x10
};
const unsigned char bmp_cursor_clear[32]=
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_cursor_clear_8[8]={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_box_24x16[48]=
{0xFF,0xFF,0xFF,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0x01,0x00,0x80,0xFF,0xFF,0xFF};
const unsigned char bmp_box_clear_24x16[48]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_box_clear_18x16[48]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const unsigned char bmp_box_18x16[48]=
{0xFF,0xFF,0x03,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0x01,0x00,0x02,0xFF,0xFF,0x03};
const unsigned char bmp_underline_18x16[48]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x03};
const unsigned char bmp_underline_clear_18x16[48]=
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
typedef enum{
	eMainMenu=0,				//home page
	eSelectMenu=1,			//Function Selection Menu
	eDisplaySet=2,     //display mode
	eNetworkSet=3,			//Setting Networking Mode
	eIpAndGateSet=4,		//IP Gateway Settings
	eSystemInfo=5,			//system information
	eFactoryReset,			//Query about restoring factory Settings
	eDeviceId,					//Equipment IEMI
	eVersionInfo,				//version information
 	eUnkown						
}EMainStatus;

EMainStatus  gMainState = eMainMenu;			//Global primary state variable
	
static rt_int8_t glOK = RT_FALSE;					//Long press the OK key to return to the upper-level menu
static rt_int8_t gOK = RT_FALSE;					//OK to determine key
static rt_int8_t gLeft = RT_FALSE;				//left button
static rt_int8_t gRight = RT_FALSE;				//right button

rt_uint16_t blacklight_cnt = 0;						//Page standby mode delay
rt_int8_t isRefreshDisplay = RT_TRUE;			//refresh display
rt_int8_t gStateData = 0;							
rt_int8_t gStateDataEx = 0;								

rt_int8_t  updateDisplay = RT_TRUE; 			//Whether to refresh the display
rt_int8_t  isClearDisplay = RT_FALSE; 		//CLS switch
rt_int8_t  isInputNumStatus = RT_FALSE; 	//Whether it is in the digital input state
rt_uint8_t focu_pos_num = 0;   					  //The number or symbol displayed at the cursor position
rt_int8_t	 cursorMark = 0;								//Cursor flicker status
unsigned char network_status = 0;
void staticMainMenu(void);
void main_menu_refresh_display(rt_int8_t refresh);
void refreshNetworkSet(TNetworkInfo *info, rt_int8_t pos, rt_uint8_t flag);
void display_one_terminal_status(rt_uint8_t page, rt_uint8_t index);
void displaySelectNumber(rt_uint8_t gStateData, rt_uint8_t flag, TNetworkInfo *info);
void oled_setNetworkStatus(unsigned char status);
/* mqtt_client_proc.c */
extern int is_online;
/* drv_eth.c*/
extern rt_int8_t rj45_status;
/* at_device_air720.c*/
extern int rssi;

u8g2_t u8g2;
int hmi_interface_init(void)
{
    // Initialization//U8G2_R0,U8G2_R2
	TNetworkInfo *netInfo;
	netInfo = getTNetworInfo();
	if(1==netInfo->displayMode){
		u8g2_Setup_ssd1306_i2c_128x32_univision_f( &u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_rtthread);
	}
	else if(2==netInfo->displayMode)
	{
		u8g2_Setup_ssd1306_i2c_128x32_univision_f( &u8g2, U8G2_R2, u8x8_byte_sw_i2c, u8x8_gpio_and_delay_rtthread);
	}
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_CLOCK, OLED_I2C_PIN_SCL);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_DATA, OLED_I2C_PIN_SDA);
	
	u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
	u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312b);
	u8g2_SetFontPosTop(&u8g2);
	u8g2_ClearBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 0, 0, 128, 64, (const uint8_t *)logo);//display logo
	u8g2_SendBuffer(&u8g2);
	rt_thread_mdelay(2000);
	return 0;
}
//INIT_DEVICE_EXPORT(hmi_interface_init);
void processKeyMsg(rt_ubase_t key)
{
	switch(key) 
	{
		case TAG_KEY_ACTION: 		gOK = RT_TRUE; break;
		case LEFT_KEY_ACTION:		gLeft = RT_TRUE; break;
		case RIGHT_KEY_ACTION: 	gRight = RT_TRUE; break;
		case QUIT_KEY_ACTION: 	glOK = RT_TRUE; break;
		default: break;
	}
}

void erasureKeyPlus(rt_ubase_t key)
{
	switch(key) 
	{
		case TAG_KEY_ACTION: 		gOK = RT_FALSE; break;
		case LEFT_KEY_ACTION:		gLeft = RT_FALSE; break;
		case RIGHT_KEY_ACTION: 	gRight = RT_FALSE; break;
		default: break;
	}
}

//Update cursor position // Left and right keys
void updateMenuSel_x(rt_uint8_t len)
{
	if(gLeft){//Left keys
		gStateData = (gStateData+len-1)%len;
	}else if(gRight){//right keys
		gStateData = (gStateData+1)%len;
	}
}

void staticMainMenu(void)
{
	main_menu_refresh_display(RT_TRUE);
}

void staticSelectMenu(void)
{
	u8g2_SetFontMode(&u8g2, 2);
	u8g2_DrawUTF8(&u8g2, 17, 2, (const char*)"显示模式");
	u8g2_DrawUTF8(&u8g2, 17, 18,(const char*)"网络设置");
	u8g2_DrawUTF8(&u8g2, 64+17, 2, (const char*)"系统信息");
	u8g2_DrawUTF8(&u8g2, 64+17, 18,(const char*)"退出    ");
	u8g2_SendBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
	u8g2_SendBuffer(&u8g2);
}

void staticDisplayModelSet(void)
{
	TNetworkInfo *netInfo;
	netInfo = getTNetworInfo();
	
	u8g2_DrawUTF8(&u8g2, 17+24, 2, "正向");
	u8g2_DrawUTF8(&u8g2, 64+17, 2,"逆向");
	u8g2_DrawUTF8(&u8g2, 17+24+24, 18,"退出    ");
	u8g2_SendBuffer(&u8g2);
	if(1==netInfo->displayMode)
	{
		u8g2_DrawXBMP(&u8g2, 24, 0, 16, 16, (const uint8_t *)bmp_cursor);
	}
	else
	{
		gStateData=1;
		u8g2_DrawXBMP(&u8g2, 24+40,0, 16,16 ,(const uint8_t *)bmp_cursor);
	}
	u8g2_SendBuffer(&u8g2);
}

void staticNetworkSet(void)
{
	TNetworkInfo *info;
	
	info = getTNetworInfo();
	if(gStateData < 2) 
	{
		u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
	}
	else if((gStateData >= 2)&&(gStateData < 4) ) 
	{
		u8g2_DrawXBMP(&u8g2, 65+12, (gStateData%2)*16, 16, 16, (const uint8_t *)bmp_cursor);
	}
	u8g2_DrawUTF8(&u8g2, 17, 2, "模式: ");
	if(info->netMode == 1)
	{
		u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "4G");
	}
	else
	{
		u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "有线");
	}
	u8g2_DrawUTF8(&u8g2, 17+64+12, 2, "IP信息");
	
	u8g2_DrawUTF8(&u8g2, 17, 2+16, "DHCP:");
	if(info->ipMode == 1){
		u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "手动");
	}else{
		u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "自动");
	}
	u8g2_DrawUTF8(&u8g2, 17+64+12, 2+16, "退出 ");
	u8g2_SendBuffer(&u8g2);
	refreshNetworkSet(info, gStateData, ANTI);
}

void staticIpAndGateSetPage(rt_uint8_t gStateData)
{
	struct netdev *netdev = RT_NULL;
	TNetworkInfo *info, tInfo;
	uint32_t add_t = 0;
	uint8_t *ipByte = 0;
	char buf[32];
	
	if(getTNetworInfo_net_mode() == 1){ //4G
		tInfo.netMode = 1;
		netdev = netdev_get_by_name("air720");
	}else{															
		tInfo.netMode = 0;
		netdev = netdev_get_by_name("e0");
	}
	add_t = netdev->ip_addr.addr;
	ipByte = (uint8_t *)&add_t;
	tInfo.ip[0] = *ipByte;
	tInfo.ip[1] = *(ipByte+1);
	tInfo.ip[2] = *(ipByte+2);
	tInfo.ip[3] = *(ipByte+3);	
	
	add_t = netdev->netmask.addr;
	ipByte = (uint8_t *)&add_t;
	tInfo.nm[0] = *ipByte;
	tInfo.nm[1] = *(ipByte+1);
	tInfo.nm[2] = *(ipByte+2);
	tInfo.nm[3] = *(ipByte+3);

	add_t = netdev->gw.addr;		
	ipByte = (uint8_t *)&add_t;
	tInfo.gw[0] = *ipByte;
	tInfo.gw[1] = *(ipByte+1);
	tInfo.gw[2] = *(ipByte+2);
	tInfo.gw[3] = *(ipByte+3);
	
	tInfo.ipMode = getTNetworInfo_ip_mode();
	tInfo.slave_type = getTNetworInfo_ac2000_type();
	tInfo.displayMode = getTNetworInfo_display_mode();
	setNetworkDynamicInfo(tInfo);
		
	info = getTNetworInfo();
	if(gStateData < 8){
		u8g2_DrawUTF8(&u8g2, 10, 2, "IP:");
		sprintf(buf, "%3d.%3d.%3d.%3d", info->ip[0], info->ip[1], info->ip[2], info->ip[3]);
		u8g2_DrawUTF8(&u8g2, 38, 2, buf);
		
		u8g2_DrawUTF8(&u8g2, 10, 18, (const char*)"子网:");
		sprintf(buf, "%3d.%3d.%3d.%3d", info->nm[0], info->nm[1], info->nm[2], info->nm[3]);
		u8g2_DrawUTF8(&u8g2, 38, 18, buf);
		
		displaySelectNumber(gStateData, ANTI, info);
	}else{
		u8g2_DrawUTF8(&u8g2, 10, 2, (const char*)"网关:");
		sprintf(buf, "%3d.%3d.%3d.%3d", info->gw[0], info->gw[1], info->gw[2], info->gw[3]);
		u8g2_DrawUTF8(&u8g2, 38, 2, buf);

		u8g2_DrawUTF8(&u8g2, 50, 18, (const char*)" 退出 ");
		
		displaySelectNumber(gStateData, ANTI, info);
	}
	u8g2_SendBuffer(&u8g2);
}

void staticSystemInfo(void)
{
	u8g2_SetFontMode(&u8g2, 2);
	if(gStateData < 2) 
	{
		u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
	}
	else if((gStateData >= 2)&&(gStateData < 4) ) 
	{
		u8g2_DrawXBMP(&u8g2, 65, (gStateData%2)*16, 16, 16, (const uint8_t *)bmp_cursor);
	}
	u8g2_DrawUTF8(&u8g2, 17, 2, "恢复设置");
	u8g2_DrawUTF8(&u8g2, 17, 18,"设备SN");
	u8g2_DrawUTF8(&u8g2, 64+17, 2, "程序版本");
	u8g2_DrawUTF8(&u8g2, 64+17, 18,"退出    ");
	u8g2_SendBuffer(&u8g2);
}

void staticFactoryReset(void)
{
	u8g2_DrawUTF8(&u8g2, 20, 2, " 恢复默认设置？ ");
	u8g2_DrawXBMP(&u8g2, 80-16, 18, 16, 16, (const uint8_t *)bmp_cursor);
	gStateData = 1;
	u8g2_DrawUTF8(&u8g2, 30, 18, "是 ");
	u8g2_DrawUTF8(&u8g2, 80, 18, "否 ");
	u8g2_SendBuffer(&u8g2);
}
extern void create_u8g2_qrcode_bmp(uint8_t index_x, uint8_t index_y);
void staticDeviceId(void)
{
	char buf[20];
	u8g2_DrawUTF8(&u8g2, 0, 10, "SN:");
	for(int i=0;i<8;i++)
	{
	rt_memcpy(buf, &stm32_uniqueId[i*4], 4);
	buf[4] = '\0';
		if(i*30<(128-30))
	u8g2_DrawUTF8(&u8g2, 14+i*30, 2, buf);
		else
		u8g2_DrawUTF8(&u8g2, 14+(i-4)*30, 20, buf);
	}
	u8g2_SendBuffer(&u8g2);
	u8g2_DrawXBMP(&u8g2, 2, 2, 29, 29, (const uint8_t *)qrcodeData);
//	create_u8g2_qrcode_bmp(2,2);
//	u8g2_SendBuffer(&u8g2);
}

void staticVersionInfo(void)
{
	u8g2_DrawUTF8(&u8g2, 0, 2, "版本:");
	u8g2_DrawUTF8(&u8g2, 64, 2, VERSION);
	u8g2_DrawUTF8(&u8g2, 0, 18, "日期:");
	u8g2_DrawUTF8(&u8g2, 32, 18, DATETIME);
	u8g2_SendBuffer(&u8g2);
}

extern unsigned char ac2000_link_status;const unsigned char back_clear[192]={0};
void _roll_display_alarm_info(unsigned char status)
{
	if(status)
	{
		u8g2_DrawXBMP(&u8g2, 32, 0, 7, 13,(const uint8_t *)bmp_ac2000_7_13);
//		u8g2_DrawXBMP(&u8g2, 40+12*2, 16, 32, 16, (const uint8_t *)bmp_clear_32x16);
//		u8g2_DrawUTF8(&u8g2, 40, 18, "连接断开");
	}
	else
	{
		u8g2_DrawXBMP(&u8g2, 32, 0, 7, 13,(const uint8_t *)bmp_clear_7_13);
//		u8g2_DrawXBMP(&u8g2, 40+12*2, 16, 32, 16, (const uint8_t *)bmp_clear_32x16);
//		u8g2_DrawUTF8(&u8g2, 40, 18, "连接正常");
	}
	u8g2_DrawXBMP(&u8g2, 0, 16, 128, 12,(const uint8_t *)back_clear);
	oled_setNetworkStatus(network_status);
	u8g2_SendBuffer(&u8g2);
}

void setNetworkStatus(unsigned char status)
{
	network_status = status;
}

void oled_setNetworkStatus(unsigned char status)
{
	switch(status)
	{
		case Checking:
			u8g2_DrawUTF8(&u8g2, 0, 18, "    网络检测中...   ");
			break;
		case ModuleAbnormal:
			u8g2_DrawUTF8(&u8g2, 0, 18, "  网络模块通信异常  ");
			break;
		case SIMAbnormal:
			u8g2_DrawUTF8(&u8g2, 0, 18, "      SIM卡异常     ");
			break;
		case GSMNetworkRejected:
			u8g2_DrawUTF8(&u8g2, 0, 18, "  GSM网络注册失败  ");
			break;
		case GPRSNetworkRejected:
			u8g2_DrawUTF8(&u8g2, 0, 18, "  GPRS网络注册失败  ");
			break;
		case SingalStrength:
			u8g2_DrawUTF8(&u8g2, 0, 18, "    信号强度检测失败   ");
			break;
		case GPRSConnect:
			u8g2_DrawUTF8(&u8g2, 0, 18, "     不能连接GPRS    ");
			break;
		case LocalAddr:
			u8g2_DrawUTF8(&u8g2, 0, 18, "   获取本地地址失败   ");
			break;
		case NetworkInit:
			u8g2_DrawUTF8(&u8g2, 0, 18, "  网络初始化失败   ");
			break;
		case ServerConnetFailure:
			u8g2_DrawUTF8(&u8g2, 0, 18, "   服务器通讯失败  ");
			break;
		case DeviceNotRegistered:
			u8g2_DrawUTF8(&u8g2, 0, 18, "     设备未注册    ");
			break;
		case WiredNetworkException:
			u8g2_DrawUTF8(&u8g2, 0, 18, "    有线网络异常   ");
			break;
		case Normal:
			u8g2_DrawUTF8(&u8g2, 0, 18, "      网络正常     ");
			break;
	}

}
	
void display4GModeRssi(rt_int8_t rssi)
{
	if((rssi < 2)||(rssi > 31)){
		u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_lte_0_13);
	}else{
		if((rssi>=2)&&(rssi<9))
			u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_lte_1_13);
		else if((rssi>=9)&&(rssi<16))
			u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_lte_2_13);
		else if((rssi>=16)&&(rssi<23))
			u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_lte_3_13);
		else if((rssi>=23)&&(rssi<=31))
			u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_lte_4_13);
	}
	u8g2_SendBuffer(&u8g2);
}

void main_menu_refresh_display(rt_int8_t refresh)
{
	static rt_int8_t showAlarmCnt = 0;
	rt_int8_t slave_type=0;
	
	if(refresh){
		showAlarmCnt = 8;
	}
	showAlarmCnt++;
	if(showAlarmCnt > 8){
		showAlarmCnt = 0;

		/* Display signal strength SIM card status Cable network connection status */
		if(getTNetworInfo_net_mode() == 1){
			display4GModeRssi(rssi);
		}else{
			if(rj45_status == 1){
				u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_wired_13);//connect
			}else{
				u8g2_DrawXBMP(&u8g2, 0, 0, 13, 13, (const uint8_t *)bmp_wired_error_13);//disconnect
			}
		}

		/* Displays the MQTT status */
		if(is_online == 1){
			u8g2_DrawXBMP(&u8g2, 16, 0, 13, 13, (const uint8_t *)bmp_mqtt_13);
		}else{
			u8g2_DrawXBMP(&u8g2, 16, 0, 13, 13, (const uint8_t *)bmp_mqtt_error_13);
		}
		slave_type = getTNetworInfo_ac2000_type();
		if(1 == slave_type)
		{
			u8g2_DrawUTF8(&u8g2, 40, 2,"AC2000-3G");
		}
		else if(2 == slave_type)
		{
			u8g2_DrawUTF8(&u8g2, 40, 2,"AC2000-Platinum");
		}
		else if(3 == slave_type)
		{
			u8g2_DrawUTF8(&u8g2, 40, 2,"AC2000-Plat-Pro");
		}
		else if(4 == slave_type)
		{
			u8g2_DrawUTF8(&u8g2, 40, 2,"AC2000-Plus");
		}else u8g2_DrawUTF8(&u8g2, 40, 2,"AC2000");

		u8g2_SendBuffer(&u8g2);
		/* Add the number of scrolling displays of alarm information online also shown in this function */
		_roll_display_alarm_info(ac2000_link_status);
	}
}

void keyMainMenu(void)
{
	main_menu_refresh_display(RT_FALSE);
	
	if(gOK){
		gStateData = 0;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSelectMenu;
	}
}

void keySelectMenu(void)
{
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 0,gStateData*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
		else if((gStateData >= 2)&&(gStateData < 4) )
		{
			u8g2_DrawXBMP(&u8g2, 65,(gStateData%2)*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
	}
	updateMenuSel_x(4);
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
		else if((gStateData >= 2)&&(gStateData < 4) ) 
		{
			u8g2_DrawXBMP(&u8g2, 65, (gStateData%2)*16, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
	}
	if(gOK){
		switch(gStateData){
			case 0: gMainState = eDisplaySet; break;
			case 1: gMainState = eNetworkSet;  break;
			case 2: gMainState = eSystemInfo;  break;
			case 3: gMainState = eMainMenu;    break;
			default: break;
		}
		gStateData = 0;
		gStateDataEx = 0;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
	}
}

void keyDisplaySet(void)
{
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 24+gStateData*40,0, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
		else if((gStateData == 2)&&(gStateData < 4) )
		{
			u8g2_DrawXBMP(&u8g2, 24+24, 16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
	}
	updateMenuSel_x(3);
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 24+gStateData*40, 0, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
		else if((gStateData >= 2)&&(gStateData < 4) ) 
		{
			u8g2_DrawXBMP(&u8g2, 24+24, 16, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
	}
	if(gOK){
		if(gStateData<2){
			setDisplayMode(gStateData+1);
		}
		gMainState = eSelectMenu;
		gStateData = 0;
		gStateDataEx = 0;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
	}
}

void refreshNetworkSet(TNetworkInfo *info, rt_int8_t pos, rt_uint8_t flag)
{
	switch(pos){
		case 0:
			u8g2_DrawXBMP(&u8g2, 17+24+6, 0, 32, 16, (const uint8_t *)bmp_clear_32x16);
			if(info->netMode == 1){
				u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "4G");
			}else{
				u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "有线");
			}
			break;
		case 1:
			u8g2_DrawXBMP(&u8g2, 17+32, 16, 32, 16, (const uint8_t *)bmp_clear_32x16);
			if(info->ipMode == 1){
				u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "手动");
			}else{
				u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "自动");
			}
			break;
		case 2:

			break;
		case 3:

			break;
		default: break;
	}
	u8g2_SendBuffer(&u8g2);
}

void keyNetworkSet(void)
{
	TNetworkInfo *netInfo;
	netInfo = getTNetworInfo();
	if(isInputNumStatus == RT_FALSE){
		if(gRight||gLeft){
			refreshNetworkSet(netInfo, gStateData, 1);
			if(gStateData < 2) 
			{
				u8g2_DrawXBMP(&u8g2, 0,gStateData*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
			}
			else if((gStateData >= 2)&&(gStateData < 4) )
			{
				u8g2_DrawXBMP(&u8g2, 65+12,(gStateData%2)*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
			}
		}
		updateMenuSel_x(4);
		if(gRight||gLeft){
			refreshNetworkSet(netInfo, gStateData, 1);
			if(gStateData < 2) 
			{
				u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
				u8g2_SendBuffer(&u8g2);
			}
			else if((gStateData >= 2)&&(gStateData < 4) ) 
			{
				u8g2_DrawXBMP(&u8g2, 65+12, (gStateData%2)*16, 16, 16, (const uint8_t *)bmp_cursor);
				u8g2_SendBuffer(&u8g2);
			}
		}
		if(gOK){
			switch(gStateData){
				case 0:
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
					focu_pos_num = netInfo->netMode;
					break;
				case 1:
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
					focu_pos_num = netInfo->ipMode;
					break;
				case 2:
					gStateData = 0;
					gStateDataEx = 0;
					isClearDisplay = RT_TRUE;
					updateDisplay = RT_TRUE;
					gMainState = eIpAndGateSet;
					break;
				case 3:
					gStateData = 1;
					isClearDisplay = RT_TRUE;
					updateDisplay = RT_TRUE;
					gMainState = eSelectMenu;
					break;
				default: break;
			}
		}
	}else{
		cursorMark = (cursorMark + 1) % 2;
		if(gRight||gLeft){
			focu_pos_num = (focu_pos_num+1)%2;
			cursorMark = 1;
		}
		if(gOK||glOK){
			isInputNumStatus = RT_FALSE;
			cursorMark = 1;
			if(gStateData == 0){
				setNetworkMode(focu_pos_num);
			}else if(gStateData == 1){
				setGetIpMode(focu_pos_num);
			}
			if((netInfo->netMode == 1)||(netInfo->ipMode == 0)){
				u8g2_DrawXBMP(&u8g2, 17+64+12, 0, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawUTF8(&u8g2, 17+64+12, 2, "IP信息");
			}else{
				u8g2_DrawXBMP(&u8g2, 17+64+12, 0, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawUTF8(&u8g2, 17+64+12, 2, "IP设置");
			}
		}
		if(cursorMark){
			if(gStateData == 0) {
				u8g2_DrawXBMP(&u8g2, 17+24+6, 0, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawXBMP(&u8g2, 17+24+6, 0, 24, 16, (const uint8_t *)bmp_box_24x16);
				if(focu_pos_num == 1){ 
					u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "4G");
				}else{
					u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "有线");
				}
			}
			else if(gStateData == 1){
				u8g2_DrawXBMP(&u8g2, 17+32, 16, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawXBMP(&u8g2, 17+32, 16, 24, 16, (const uint8_t *)bmp_box_24x16);
				if(focu_pos_num == 1){
					u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "手动");
				}else{
					u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "自动");
				}
			}
		}else{
			if(gStateData == 0) {
				u8g2_DrawXBMP(&u8g2, 17+24+6, 0, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawXBMP(&u8g2, 17+24+6, 0, 24, 16, (const uint8_t *)bmp_box_clear_24x16);
				if(focu_pos_num == 1){ 
					u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "4G");
				}else{
					u8g2_DrawUTF8(&u8g2, 17+24+6, 2, "有线");
				}
			}
			else if(gStateData == 1){
				u8g2_DrawXBMP(&u8g2, 17+32, 16, 32, 16, (const uint8_t *)bmp_clear_32x16);
				u8g2_DrawXBMP(&u8g2, 17+32, 16, 24, 16, (const uint8_t *)bmp_box_clear_24x16);
				if(focu_pos_num == 1){
					u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "手动");
				}else{
					u8g2_DrawUTF8(&u8g2, 17+32, 2+16, "自动");
				}
			}
		}
		u8g2_SendBuffer(&u8g2);
	}
}

void u8g2_DrawNumberBox(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, rt_uint8_t flag)
{
	if(flag){
		u8g2_DrawXBMP(u8g2, x, y, w, h, (const uint8_t *)bmp_box_clear_18x16);
	}
	else
	{
		u8g2_DrawXBMP(u8g2, x, y, w, h, (const uint8_t *)bmp_box_18x16);
	}
}

void displaySelectNumber(rt_uint8_t gStateData, rt_uint8_t flag, TNetworkInfo *info)
{
	char buf[8];
	rt_uint8_t pos = 0;
	pos = gStateData % 4;
	if(gStateData < 4){																		//IP
		sprintf(buf, "%3d", info->ip[pos]);
		if(flag == ANTI) focu_pos_num = info->ip[pos];
		u8g2_DrawNumberBox(&u8g2, 38+pos*23, 0, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 2, buf);
	}else if((gStateData >= 4)&&(gStateData < 8)){				//mask
		sprintf(buf, "%3d", info->nm[pos]);
		if(flag == ANTI) focu_pos_num = info->nm[pos];
		u8g2_DrawNumberBox(&u8g2, 38+pos*23, 16, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 18, buf);
	}else if((gStateData >= 8)&&(gStateData < 12)){				//gateway
		sprintf(buf, "%3d", info->gw[pos]);
		if(flag == ANTI) focu_pos_num = info->gw[pos];
		u8g2_DrawNumberBox(&u8g2, 38+pos*23, 0, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 2, buf);
	}else if(gStateData == 12){														//exit
		if(flag)
		u8g2_DrawXBMP(&u8g2, 40, 16, 16, 16, (const uint8_t *)bmp_cursor_clear);
		else
		u8g2_DrawXBMP(&u8g2, 40, 16, 16, 16, (const uint8_t *)bmp_cursor);
		focu_pos_num = 0;
	}
	u8g2_SendBuffer(&u8g2);
}

void u8g2_DrawNumberUnderline(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, rt_uint8_t flag)
{
	if(flag){
		u8g2_DrawXBMP(u8g2, x, y, w, h, (const uint8_t *)bmp_underline_clear_18x16);
	}
	else
	{
		u8g2_DrawXBMP(u8g2, x, y, w, h, (const uint8_t *)bmp_underline_18x16);
	}
}
void displayInputNumber(rt_uint8_t gStateData, rt_uint8_t flag, int num)
{
	char buf[8];
	rt_uint8_t pos = 0;
	
	pos = gStateData % 4;
	sprintf(buf, "%3d", num);
	if(gStateData < 4){																		//IP
		u8g2_DrawNumberUnderline(&u8g2, 38+pos*23, 0, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 2, buf);
	}else if((gStateData >= 4)&&(gStateData < 8)){				//mask
		u8g2_DrawNumberUnderline(&u8g2, 38+pos*23, 16, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 18, buf);
	}else if((gStateData >= 8)&&(gStateData < 12)){				//gateway
		u8g2_DrawNumberUnderline(&u8g2, 38+pos*23, 0, 18, 16, flag);
		u8g2_DrawUTF8(&u8g2, 38+pos*23, 2, buf);
	}
	u8g2_SendBuffer(&u8g2);
}

void keyIpAndGateSet(void)
{
	TNetworkInfo *info;
	info = getTNetworInfo();
	
	if(isInputNumStatus == RT_FALSE){	
	
		if(gLeft){
			if(gStateData == 0){  //Check the exit button
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+13-1)%13;
			}else if(gStateData == 8){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+13-1)%13;				
			}else{
				displaySelectNumber(gStateData, NORMAL, info);
				gStateData = (gStateData+13-1)%13;
				displaySelectNumber(gStateData, ANTI, info);
			}
		}else if(gRight){//right click
			if(gStateData == 7){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;			
				gStateData = (gStateData+1)%13;
			}else if(gStateData == 12){
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gStateData = (gStateData+1)%13;
			}else{
				displaySelectNumber(gStateData, NORMAL, info);
				gStateData = (gStateData+1)%13;
				displaySelectNumber(gStateData, ANTI, info);
			}
		}

		if(gOK){
			if(gStateData == 12){
				gStateData = 2;
				cursorMark = 0;
				isClearDisplay = RT_TRUE;
				updateDisplay = RT_TRUE;
				gMainState = eNetworkSet;
			}else{
				/* Editing is allowed only when you use a wired network and manually set the IP mode. */
				if((info->ipMode == 1)&&(info->netMode == 0)){
					cursorMark = 0;
					isInputNumStatus = RT_TRUE;
				}
			}
		}
	}else{
		cursorMark = (cursorMark + 1) % 2;
		if(gLeft){//left button
			focu_pos_num = (focu_pos_num+256-1)%256;
			cursorMark = 1;
		}else if(gRight){//right click
			focu_pos_num = (focu_pos_num+1)%256;
			cursorMark = 1;
		}
		if(gOK||glOK){
			if(gStateData < 4){																		//IP
				setIpAddress(gStateData%4, focu_pos_num);			
			}else if((gStateData >= 4)&&(gStateData < 8)){				//mask
				setNmAddress(gStateData%4, focu_pos_num);
			}else if((gStateData >= 8)&&(gStateData < 12)){				//gateway
				setGwAddress(gStateData%4, focu_pos_num);	
			}
			isInputNumStatus = RT_FALSE;
			cursorMark = 1;
		}
		if(cursorMark){
			displayInputNumber(gStateData, ANTI, focu_pos_num);
		}else{
			displayInputNumber(gStateData, NORMAL, focu_pos_num);
		}
	}
	
}

void keySystemInfo(void)
{
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 0,gStateData*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
		else if((gStateData >= 2)&&(gStateData < 4) )
		{
			u8g2_DrawXBMP(&u8g2, 65,(gStateData%2)*16, 16,16 ,(const uint8_t *)bmp_cursor_clear);
		}
	}
	updateMenuSel_x(4);
	if(gLeft||gRight){
		if(gStateData < 2) 
		{
			u8g2_DrawXBMP(&u8g2, 0, gStateData*16, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
		else if((gStateData >= 2)&&(gStateData < 4) ) 
		{
			u8g2_DrawXBMP(&u8g2, 65, (gStateData%2)*16, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
	}
	if(gOK){
		switch(gStateData){
			case 0: 
				gMainState = eFactoryReset; 
			break;
			case 1: 
				gMainState = eDeviceId;  	  
			break;
			case 2: 
				gMainState = eVersionInfo;  
			break;
			case 3: 
				gMainState = eSelectMenu;
			break;
			default: break;
		}
		gStateData = 0;
		gStateDataEx = 0;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
	}
}

void keyFactoryReset(void)
{
	if(gLeft||gRight){
		gStateData = (gStateData + 1) % 2;
		if(gStateData == 0){
			u8g2_DrawXBMP(&u8g2, 30-16, 18, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_DrawXBMP(&u8g2, 80-16, 18, 16, 16, (const uint8_t *)bmp_cursor_clear);
			u8g2_SendBuffer(&u8g2);
		}else{
			u8g2_DrawXBMP(&u8g2, 30-16, 18, 16, 16, (const uint8_t *)bmp_cursor_clear);
			u8g2_DrawXBMP(&u8g2, 80-16, 18, 16, 16, (const uint8_t *)bmp_cursor);
			u8g2_SendBuffer(&u8g2);
		}
	}
	if(gOK){
		if(gStateData == 0){		
			if(RT_EOK!=factory_reset_device_config())
			{
				__set_FAULTMASK(1); 
				NVIC_SystemReset();
			}				
		}else{
			gStateData = 0;
			isInputNumStatus = RT_FALSE;
			isClearDisplay = RT_TRUE;
			updateDisplay = RT_TRUE;
			gMainState = eSystemInfo;
		}
	}
}

void keyDeviceId(void)
{
	if(gOK||gLeft||gRight){
		gStateData = 1;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSystemInfo;		
	}
}

void keyVersionInfo(void)
{
	if(gOK||gLeft||gRight){
		gStateData = 2;
		isInputNumStatus = RT_FALSE;
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;
		gMainState = eSystemInfo;		
	}
}

void keyStateMachine(rt_ubase_t key)
{
	if(isRefreshDisplay == RT_TRUE){
		isClearDisplay = RT_TRUE;
		updateDisplay = RT_TRUE;	
		isRefreshDisplay	= RT_FALSE;		
	}else{
		isClearDisplay = RT_FALSE;
		updateDisplay = RT_FALSE;
	}
	
	/* Parsing key Values */
	processKeyMsg(key);
	
	switch(gMainState){
		case eMainMenu:
			keyMainMenu();
			break;
		case eSelectMenu:
			keySelectMenu();
			break;
		case eDisplaySet:
			keyDisplaySet();
			break;
		case eNetworkSet:
			keyNetworkSet();
			break;
		case eIpAndGateSet:
			keyIpAndGateSet();
			break;
		case eSystemInfo:
			keySystemInfo();
			break;
		case eFactoryReset:
			keyFactoryReset();
			break;
		case eDeviceId:
			keyDeviceId();
			break;
		case eVersionInfo:
			keyVersionInfo();
			break;
		default: break;
	}

	/* Clears processed key values */
	erasureKeyPlus(key);
}

void staticDisplayMenu(void)
{
	if(updateDisplay == RT_FALSE) return;  	//There is no display to update

	if(isClearDisplay) u8g2_ClearBuffer(&u8g2);//Mandatory CLS

	switch(gMainState){
		case eMainMenu:
			staticMainMenu();
			break;
		case eSelectMenu:
			staticSelectMenu();
			break;
		case eDisplaySet:
			staticDisplayModelSet();
			break;
		case eNetworkSet:
			staticNetworkSet();
			break;
		case eIpAndGateSet:
			staticIpAndGateSetPage(gStateData);
			break;
		case eSystemInfo:
			staticSystemInfo();
			break;
		case eFactoryReset:
			staticFactoryReset();
			break;
		case eDeviceId:
			staticDeviceId();
			break;
		case eVersionInfo:
			staticVersionInfo();
			break;
		default: break;	
	}
}
static unsigned char link_status = 0;
static unsigned char mqtt_status = 0;
static unsigned char net_status = 0;
static unsigned char temp_net_status = 0;
void hmi_interface_process(void)
{
	rt_ubase_t keySta = RT_FALSE;
	keySta = wait_key_plus(300);
	if(keySta != RT_FALSE)
	{
		if(blacklight_cnt >= 400){
			gMainState = eMainMenu;
			isRefreshDisplay = RT_TRUE;
		}
		blacklight_cnt = 0;
	}else{
		blacklight_cnt++;
		if(blacklight_cnt >= 400){
			blacklight_cnt = 400;
			gMainState = eUnkown;
			u8g2_ClearBuffer(&u8g2);
		}
	}
	if((eMainMenu==gMainState)||(eUnkown==gMainState))
	{
		if(getTNetworInfo_net_mode() == 1){
			if((rssi < 2)||(rssi > 31))
			{
				net_status = 1;
			}
			else
			{
				net_status = 0;
			}
			if(temp_net_status!=net_status)
			{
				main_menu_refresh_display(RT_TRUE);
				temp_net_status = net_status;
			}
		}else{
			if(net_status!=rj45_status)
			{
				main_menu_refresh_display(RT_TRUE);
				net_status=rj45_status;
			}
		}
		if((link_status != ac2000_link_status)||(mqtt_status!=is_online))
		{
			main_menu_refresh_display(RT_TRUE);
			link_status = ac2000_link_status;
			mqtt_status = is_online;
		}
	}
	keyStateMachine(keySta);			//keystroke handling
	
	staticDisplayMenu();
}
