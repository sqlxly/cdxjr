
#include <rtthread.h>
#ifndef __BSP_OLED_H
#define __BSP_OLED_H




  
//º¯ÊýÉùÃ÷
void fun_oled_display(void);
void OLED_Configuration(void);
void Main_Page_Display_OS(void);
void Function_Setup_Sampling_Period_OS(void);
void Function_Setup_Launch_OS(void);
void Function_Alarm_Threshold_OS(void);
void Function_Check_All_Chx_AD_OS(void);
void Launch_Setup_Enable_Status_OS(void);
void Launch_Setup_Disable_Status_OS(void);
void Setup_Sampling_20S_OS(void);
void Setup_Sampling_40S_OS(void);
void Setup_Sampling_60S_OS(void);
void Check_AD_Value_Of_A_Ch_OS( uint16_t usAdValue);
void Setup_Alarm_Threshold_OS(uint8_t ucAlarmThreshold);
void Function_Return_Back_To_Default_OS(void);
void Return_Back_To_Default_Submit_OS(void);
void Return_Back_To_Default_Cancel_OS(void);


#endif
