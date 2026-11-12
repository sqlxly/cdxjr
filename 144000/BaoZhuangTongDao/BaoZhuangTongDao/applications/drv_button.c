/************************************************************
  * @brief   按键驱动
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    button.c
  ***********************************************************/
#include "drv_button.h"
#include <string.h>
/*******************************************************************
 *                          变量声明                               
 *******************************************************************/

static struct button* Head_Button = NULL;


/*******************************************************************
 *                         函数声明     
 *******************************************************************/
static void Add_Button(Button_t* btn);


/************************************************************
  * @brief   按键创建
	* @param   name : 按键名称
	* @param   btn : 按键结构体
  * @param   read_btn_level : 按键电平读取函数,实现返回uint8_t类型的电平信号
  * @param   btn_trigger_level : ??????
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
void Button_Create(Button_t *btn, uint8_t(*read_btn_level)(void), uint8_t btn_trigger_level)
{
  if( btn == NULL)
  {
    rt_kprintf("struct button is null!");
  }
  
  memset(btn, 0, sizeof(struct button));  //清除结构体
 
  btn->Button_State = NONE_TRIGGER;           			//按键状态
  btn->Button_Trigger_Event = NONE_TRIGGER;   			//按键触发事件
  btn->Read_Button_Level = read_btn_level;    			//按键读电平函数
  btn->Button_Trigger_Level = btn_trigger_level;  	//按键触发电平
  btn->Button_Last_Level = btn->Read_Button_Level(); //按键当前电平
  btn->Debounce_Time = 0;
  
  Add_Button(btn);          //添加单链表
}

/************************************************************
  * @brief   按键触发事件与回调函数映射
	* @param   btn : 按键结构体
	* @param   btn_event : 按键触发事件
  * @param   btn_callback : 按键触发之后的回调函数
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
void Button_Attach(Button_t *btn,Button_Event btn_event,Button_CallBack btn_callback)
{
  if( btn == NULL)
  {
    rt_kprintf("struct button is null!");    
  }
  
  btn->CallBack_Function[btn_event] = btn_callback; //按键事件触发的回调函数,用于处理按键事件
}

/************************************************************
  * @brief   删除一个已经创建的按键
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
void Button_Delete(Button_t *btn)
{
  struct button** curr;

  for(curr = &Head_Button; *curr;) 
  {
    struct button* entry = *curr;
    if (entry == btn) 
    {
      *curr = entry->Next;
    } 
    else
    {
      curr = &entry->Next;
    }
  }
}

/************************************************************
  * @brief   获取按键触发事件
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
void Get_Button_EventInfo(Button_t *btn)
{
  //按键事件触发的回调函数,用于处理按键事件
  for(uint8_t i = 0 ; i < number_of_event-1 ; i++)
  {
    if(btn->CallBack_Function[i] != 0)
    {
      rt_kprintf("Button_Event:%d",i);
    }      
  } 
}

uint8_t Get_Button_Event(Button_t *btn)
{
  return (uint8_t)(btn->Button_Trigger_Event);
}

/************************************************************
  * @brief   回去按键触发事件
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  ***********************************************************/
uint8_t Get_Button_State(Button_t *btn)
{
  return (uint8_t)(btn->Button_State);
}

/************************************************************
  * @brief   按键周期处理函数
  * @param   btn:处理的按键
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    必须以一定周期调用此函数,建议周期20~50ms
  ***********************************************************/
void Button_Cycle_Process(Button_t *btn)
{
  uint8_t current_level = (uint8_t)btn->Read_Button_Level();//获取当前按键电平

  if(current_level != btn->Button_Last_Level) 							//按键电平发生变化
  {	
			if(++(btn->Debounce_Time) >= BUTTON_DEBOUNCE_TIME)  	//消抖
			{	
				 btn->Button_Last_Level = current_level; 						//更新当前按键电平
				 btn->Debounce_Time = 0;                 						//确定了是按下
				 if(btn->Button_State == NONE_TRIGGER){ 						//如果按键是没被按下的,改变按键状态为按下
						btn->Button_State = BUTTON_DOWM;
						btn->triggerMark = 0;				 
				 }else if(btn->Button_State == BUTTON_DOWM){				//释放按键
						btn->Button_State = BUTTON_UP;
						btn->triggerMark = 0;	
				 }
			}
  }

  switch(btn->Button_State)
  {
    case BUTTON_DOWM :            														//按下状态
      if(btn->Button_Last_Level == btn->Button_Trigger_Level){//按键按下
        if(++(btn->Long_Time) >= BUTTON_LONG_TIME){  					//释放按键前更新触发事件为长按	
            btn->Button_Trigger_Event = BUTTON_LONG;
						if(1 == btn->triggerMark){
								TRIGGER_CB(BUTTON_LONG);    									//长按
								btn->triggerMark = 2;
						}
            btn->Long_Time = BUTTON_LONG_TIME;								//更新时间溢出
        }else{
						btn->Button_Trigger_Event = BUTTON_DOWM;
						if(0 == btn->triggerMark){
								TRIGGER_CB(BUTTON_DOWM);
								btn->triggerMark = 1;
						}
				}
      }
      break;
		case BUTTON_UP :        																	//按键抬起
      if(btn->Button_Trigger_Event == BUTTON_DOWM){  					//触发单击		
          btn->Long_Time = 0;		
          TRIGGER_CB(BUTTON_UP);    													//释放
          btn->Button_State = NONE_TRIGGER;
					btn->Button_Trigger_Event = NONE_TRIGGER;
      }else if(btn->Button_Trigger_Event == BUTTON_LONG){			//长按释放
					btn->Long_Time = 0;
					//TRIGGER_CB(BUTTON_UP);
					btn->Button_State = NONE_TRIGGER;
					btn->Button_Trigger_Event = NONE_TRIGGER;
      }
			break;
    default :
      break;
  }
}

/************************************************************
  * @brief   遍历的方式扫描按键,不会丢失每个按键
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    此函数要周期调用,建议20-50ms调用一次
  ***********************************************************/
void Button_Process(void)
{
  struct button* pass_btn;

  for(pass_btn = Head_Button; pass_btn != NULL; pass_btn = pass_btn->Next)
  {
      Button_Cycle_Process(pass_btn);
  }
}

/************************************************************
  * @brief   使用单链表将按键连接起来
	* @param   NULL
  * @return  NULL
  * @author  jiejie
  * @github  https://github.com/jiejieTop
  * @date    2018-xx-xx
  * @version v1.0
  * @note    NULL
  ***********************************************************/
static void Add_Button(Button_t* btn)
{
  btn->Next = Head_Button;
  Head_Button = btn;
}

