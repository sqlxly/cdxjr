#ifndef __USER_BUTTON_H
#define __USER_BUTTON_H

#include <rtthread.h>
#include <rtdevice.h>

#define BUTTON_DEBOUNCE_TIME 	  4   //消抖时间(n-1)*10ms
#define BUTTON_LONG_TIME 	      250	//持续n秒((n-1)*10ms),长按 2秒

#define KEY_ON									1
#define KEY_OFF									0

#define TRIGGER_CB(event)   \
        if(btn->CallBack_Function[event]) \
          btn->CallBack_Function[event]((Button_t*)btn)

typedef void (*Button_CallBack)(void*); 

typedef enum {
  BUTTON_DOWM 		= 0,
  BUTTON_UP				= 1,
  BUTTON_LONG 		= 2,
  number_of_event = 3,
  NONE_TRIGGER		= 4
}Button_Event;

typedef struct button
{
	uint8_t (*Read_Button_Level)(void); 				/* 读取按键电平函数,需要用户实现 */
  	
  uint8_t Button_State              :   4;	  /* 按键当前状态(按下还是弹起) */
  uint8_t Button_Trigger_Level      :   2;    /* 按键触发电平 */
  uint8_t Button_Last_Level         :   2;    /* 按键当前电平 */
  
  uint8_t Button_Trigger_Event;     /* 按键触发事件,单击,双击,长按等 */
  
  Button_CallBack CallBack_Function[number_of_event];
  
	uint8_t Debounce_Time;						/* 消抖时间 */
  
	uint8_t Long_Time;		  					/* 按下持续时间 */
  
	uint8_t triggerMark;
	
  struct button *Next;
  
} Button_t;

void Button_Create(Button_t *btn, uint8_t(*read_btn_level)(void), uint8_t btn_trigger_level);

void Button_Attach(Button_t *btn,Button_Event btn_event,Button_CallBack btn_callback);   

void Button_Cycle_Process(Button_t *btn);   

void Button_Process(void);

void Button_Delete(Button_t *btn);   

void Get_Button_EventInfo(Button_t *btn);

uint8_t Get_Button_Event(Button_t *btn);

uint8_t Get_Button_State(Button_t *btn);

void channel_button_init(void);

#endif
