// Ethercat_Master.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include <string.h>
#include "sys.h"
#include "usart.h"
#include "DM9162.h"
#include "sram.h"
#include "rtc.h"
#include "bsp_timer.h"
#include "sram.h"
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatdc.h"
#include "ethercatconfig.h"

#include "ethercatcoe.h"
#include "stm32f4xx.h"
#include "DM9000.h"



#define SYNC0TIME 1000
/*
DM9161 主网卡  DM9000付网卡
0    主网卡 发送 付网卡 接收   正常运行
1    主网卡 发送 主网卡 接收		付网卡坏或者付网卡网线没插
2    付网卡 发送 付网卡 接收   主网卡坏或者主网卡网线没插
3    主网卡 发送 主网卡 接收   付网卡 发送 付网卡接收  中间模块或者网线损坏
ff   主网卡 付网卡 均损坏
*/

char IOmap[512];
uint32_t vid=0;
uint16 cur_status;
int32 cur_pos = 0;
uint8 cur_mode;
uint8 all_slave_op = 0;
__IO uint32_t pdoTimeFlag = 0;
PDO_Output *outputs1;
PDO_Input *inputs1;

uint8_t socket_mode;
uint8_t DM9000_Status;
uint8_t DM9162_Status;
unsigned char restart_waittime=0;

static void TIM3_Config(uint16_t period,uint16_t prescaler);

int sahdasdj;
uint8  sadkakd[128];
uint16 times_cnt=0;
uint16 time_cnt=1;

RCC_ClocksTypeDef ClockInfo;
/** Read PDO assign structure */
/****************************************************
主函数
*****************************************************/
uint32_t timeoftest;
int main(void)
{	
		uint32_t i,j;
	  uint16_t slc,cnt;
	  uint8 u8val;
	  uint16 u16val;
	
	  bsp_Init(); /*IO口初始化 */
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);/*设置系统中断优先级分组2 */
      SystemCoreClockUpdate(); /*系统时钟更新 */
	  uart_init(115200);   	/*调试串口波特率设置*/
	  RCC_GetClocksFreq(&ClockInfo);/*获取系统时钟值*/
      TIM2_Int_Init(0x7fffffff-1,84-1);	/*us定时，定时器时钟84M，分频系数84， 向上计数，延时定时器*/ 
	  TIM3_Config(999,899);/*DC同步时钟定时器，定时时间为SYNC0TIME，单位us*/
	  printf("\r\nWelcome to the ECAT World \n\r");
	re_check_net:     /*重新检测网口标识	*/		
	  vid = dm9k_ReadID();/*读取dm9000 ID=0x0A469000 若不等于此值，并口的操作错误*/	
	  DM9000_Init(); /* DM9000网口初始化	 */
	  DM9162_Init(); /* DM9162网口初始化  */

      socket_mode=0;
		do
			{
//双网冗余运行模式
			if((ETH_ReadPHYRegister(DM9162_PHY_ADDRESS, PHY_BSR) & 0x0004)&&(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_PHY))
				{
				 socket_mode=2;
				}
//单网运行模式
			else if((ETH_ReadPHYRegister(DM9162_PHY_ADDRESS, PHY_BSR) & 0x0004)&&(!(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_PHY)))
			 {
				 socket_mode=1;
			 }
//其他情况 未检测到网卡 错误灯闪烁
			else
			 {
				 socket_mode=0;
			 }				
				printf("Please plug in the Ethernet cable first \n\r");
				Delay_ms(100);
			  bsp_LedToggle(2);    /* 错误灯闪		*/
			}while(socket_mode==0); /* 检测到错误不允许运行 */
			
		 if(restart_waittime==1)
		  {
				restart_waittime=0;
        goto re_check_net;
		  }	
		
			bsp_LedOff(2);  /* 清除LED错误灯 */
			
		  MAC_Init(); /*  DM9162网卡参数初始化 */
		  ec_init();  /*  配置单网运行or双网冗余运行 */
			
      printf("\r\nSocket Mode: %d\n\r", socket_mode);	
			Delay_ms(1000);
      bsp_LedOn(1);   /*  Run 灯点亮	 */
			
	  if(ec_config_init(FALSE)>0)		/*  查找从站，初始化从站等	 */
	    {
	      printf("%d slaves found and configured.\r\n",ec_slavecount);         
				if((ec_slavecount >= 1))
				{
					for(slc = 1; slc <= ec_slavecount; slc++)
						{
							printf("Found %s at position %d\n", ec_slave[slc].name, slc);
						}
				}			 
	       ec_config_map(&IOmap);	/* 分配从站PDO,与IOmap变量关联 */					
				 printf("Slaves mapped, state to SAFE_OP.\n");
	        /* 等待所有从站状态切换到 SAFE_OP */
         ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE);

				 printf("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
			   printf("Slave 1 State=0x%04x\r\n",ec_slave[1].state);
				 do
				   {
							ec_statecheck(0, EC_STATE_SAFE_OP, 50000);
			   		 ec_statecheck(1, EC_STATE_SAFE_OP, 50000);
				    }
				    while ((ec_slave[0].state != EC_STATE_SAFE_OP) || (ec_slave[1].state != EC_STATE_SAFE_OP));
						printf("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
						printf("Slave 1 State=0x%04x\r\n",ec_slave[1].state);
					/* 打印分配的信息 */
          for( cnt = 1 ; cnt <= ec_slavecount ; cnt++)
            {
               printf("\nSlave:%d\n Name:%s\n Output size: %dbits\n Input size: %dbits\n State: %d\n Delay: %d[ns]\n Has DC: %d\n",
                       cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
                       ec_slave[cnt].state, ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);
               printf(" Configured address: %x\n", ec_slave[cnt].configadr);
               printf(" Outputs address: %x\n", ec_slave[cnt].outputs);
               printf(" Inputs address: %x\n", ec_slave[cnt].inputs);

               for(j = 0 ; j < ec_slave[cnt].FMMUunused ; j++)
               {
                  printf(" FMMU%1d Ls:%x Ll:%4d Lsb:%d Leb:%d Ps:%x Psb:%d Ty:%x Act:%x\n", j,
                          (int)ec_slave[cnt].FMMU[j].LogStart, ec_slave[cnt].FMMU[j].LogLength, ec_slave[cnt].FMMU[j].LogStartbit,
                          ec_slave[cnt].FMMU[j].LogEndbit, ec_slave[cnt].FMMU[j].PhysStart, ec_slave[cnt].FMMU[j].PhysStartBit,
                          ec_slave[cnt].FMMU[j].FMMUtype, ec_slave[cnt].FMMU[j].FMMUactive);
               }
               printf(" FMMUfunc 0:%d 1:%d 2:%d 3:%d\n",
                        ec_slave[cnt].FMMU0func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU2func, ec_slave[cnt].FMMU3func);

            }
						printf("Request operational state for all slaves\n");
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            /* send one valid process data to make outputs in slaves happy*/
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            /* 请求所有从站运行到OP 状态 */
            ec_writestate(0);

            /* 等待所有从站切换到 OP 状态 */
            ec_statecheck(0, EC_STATE_OPERATIONAL,  EC_TIMEOUTSTATE);								
					  do
				    {
				   		ec_send_processdata();
				   		ec_receive_processdata(EC_TIMEOUTRET);
							ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
				   		ec_statecheck(1, EC_STATE_OPERATIONAL, 50000);
				    }
				    while ((ec_slave[0].state != EC_STATE_OPERATIONAL)|| (ec_slave[1].state != EC_STATE_OPERATIONAL));
						printf("Slave 0 State=0x%04x\r\n",ec_slave[0].state);
						printf("Slave 1 State=0x%04x\r\n",ec_slave[1].state);
						
						/* 如果所有从站状态为OP 则对从站的 输入输出进行重映射*/
            if (ec_slave[0].state == EC_STATE_OPERATIONAL)
            {	
               printf("Operational state reached for all slaves.\n");
							 all_slave_op = 1;												
							 outputs1 = (PDO_Output *)ec_slave[1].outputs;
               inputs1  = (PDO_Input *)ec_slave[1].inputs;
            }
            else /* 反之 打印从站状态*/
            {
               printf("Not all slaves reached operational state.\n");
               ec_readstate();
               for(i = 1; i<=ec_slavecount ; i++)
               {
                  if(ec_slave[i].state != EC_STATE_OPERATIONAL)
                  {
                     printf("Slave %d State=0x%04x StatusCode=0x%04x\n",
                             i, ec_slave[i].state, ec_slave[i].ALstatuscode);
                  }
               }
            }
	     }
		 else
			{
				printf("No slaves found!\r\n");
			}			
	while(1)
	{
		//PDO的通讯在TIM3中断函数中
   if(pdoTimeFlag == 1)	/* PDO通讯定时标志位 */
	  {
			DM9162_Status=ETH_ReadPHYRegister(DM9162_PHY_ADDRESS, PHY_BSR) & 0x0004;/* 主网口link状态标志 */
			DM9000_Status=(dm9k_ReadReg(DM9000_REG_NSR) & DM9000_PHY);/* 冗余口link状态标志 */
			if(( !DM9162_Status) && (!DM9000_Status))/* 主网口和冗余口都掉线时跳转到re_check_net 等待网口重新连接 */
			{
			 printf("Ethernets are all error,NOW must reset! goto  lable re_check_net\n");
			 restart_waittime=1;
			 goto	re_check_net;
			}				 
			pdoTimeFlag=0;
//		  ecat_loop();
	  }
	}
}
/**
  * @brief  通用定时器3中断初始化
  * @param  period : 自动重装值。
  * @param  prescaler : 时钟预分频数            
  * @retval 无                             
  * @note   定时器溢出时间计算方法:Tout=((period+1)*(prescaler+1))/Ft us.
  *          Ft=定时器工作频率,为SystemCoreClock/2=90,单位:Mhz
  */
static void TIM3_Config(uint16_t period,uint16_t prescaler)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  ///使能TIM3时钟
	
	TIM_TimeBaseInitStructure.TIM_Prescaler=84 - 1;//定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period=SYNC0TIME - 1 ;//自动重装载值
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	//NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //定时器3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x0; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x03; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM3,ENABLE); //使能定时器3
	
	
}

/**
  * @brief  定时器3中断服务函数
  * @param  无
  * @retval 无
  */
void TIM3_IRQHandler(void)
{
	
	
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET) //溢出中断
	{
//			pdoTimeFlag = 1;
		if(all_slave_op==1)  /* 所有从站已经处于OP状态 可以进行PDO的操作 */
		{
			pdoTimeFlag = 1;   /* PDO通讯定时标志位 */
			//跑马灯逻辑
			times_cnt++;
			if(times_cnt>10)
			{
			times_cnt=0;
			if(time_cnt<128)
				 time_cnt=time_cnt*2;
			else 
				 time_cnt=1;
			 }
		   ecat_loop(); /* 跑马灯产生的逻辑赋值到RxPDO */
		 } 
		else
		{
		  ec_send_processdata();	
		}

	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);  //清除中断标志位
}