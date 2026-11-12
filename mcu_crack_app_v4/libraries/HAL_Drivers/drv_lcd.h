#ifndef __DRV_LCD_H
#define __DRV_LCD_H
#include <rtthread.h>
#include "rtdevice.h"
#include <drv_common.h>



#define LCD_X_SIZE          (128)
#define LCD_Y_SIZE          (64)


#define OLED_SCK_PORT       GPIOA
#define OLED_SCK_PIN        GPIO_PIN_5
#define OLED_MISO_PORT      GPIOA
#define OLED_MISO_PIN       GPIO_PIN_6
#define OLED_MOSI_PORT      GPIOD
#define OLED_MOSI_PIN       GPIO_PIN_7
#define OLED_CS1_PORT       GPIOD
#define OLED_CS1_PIN        GPIO_PIN_6
#define OLED_CS2_PORT       GPIOD
#define OLED_CS2_PIN        GPIO_PIN_3
#define OLED_DC_PORT        GPIOA
#define OLED_DC_PIN         GPIO_PIN_4


#define font_cs_write(a)   if(a)\
                            HAL_GPIO_WritePin(OLED_CS2_PORT,OLED_CS2_PIN,GPIO_PIN_SET);\
                            else\
                            HAL_GPIO_WritePin(OLED_CS2_PORT,OLED_CS2_PIN,GPIO_PIN_RESET)
                            
#define oled_cs_write(a)    if(a)\
                            HAL_GPIO_WritePin(OLED_CS1_PORT,OLED_CS1_PIN,GPIO_PIN_SET);\
                            else\
                            HAL_GPIO_WritePin(OLED_CS1_PORT,OLED_CS1_PIN,GPIO_PIN_RESET)

#define oled_sck_write(a)   if(a)\
                            HAL_GPIO_WritePin(OLED_SCK_PORT,OLED_SCK_PIN,GPIO_PIN_SET);\
                            else\
                            HAL_GPIO_WritePin(OLED_SCK_PORT,OLED_SCK_PIN,GPIO_PIN_RESET)

#define oled_sdi_write(a)   if(a)\
                            HAL_GPIO_WritePin(OLED_MOSI_PORT,OLED_MOSI_PIN,GPIO_PIN_SET);\
                            else\
                            HAL_GPIO_WritePin(OLED_MOSI_PORT,OLED_MOSI_PIN,GPIO_PIN_RESET)

#define oled_dc_write(a)    if(a)\
                            HAL_GPIO_WritePin(OLED_DC_PORT,OLED_DC_PIN,GPIO_PIN_SET);\
                            else\
                            HAL_GPIO_WritePin(OLED_DC_PORT,OLED_DC_PIN,GPIO_PIN_RESET) 

#define oled_sdo_read(a)    HAL_GPIO_ReadPin(OLED_MISO_PORT, OLED_MISO_PIN) 



struct drv_lcd_device
{
    struct rt_device parent;

    struct rt_device_graphic_info lcd_info;
};

#define LCD_DEVICE(dev) (struct drv_lcd_device *)(dev)









#endif
