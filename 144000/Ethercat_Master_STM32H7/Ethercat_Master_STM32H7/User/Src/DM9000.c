/*
*********************************************************************************************************
*
*	模块名称 : DM9000AEP 底层驱动模块(For STM32F4XX， uip)
*	文件名称 : dm9k_uip.c
*	版    本 : V1.1
*	说    明 : 这是硬件底层驱动程序的主文件。每个c文件可以 #include "bsp.h" 来包含所有的外设驱动模块。
*			   bsp = Borad surport packet 板级支持包
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-03-01  armfly   正式发布
*		V1.1    2013-06-20  armfly   规范注释，添加必要说明
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp_timer.h"
#include "DM9000.h"
#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"
#include "string.h"
#include "main.h"

SRAM_HandleTypeDef SRAM_Handler;    //SRAM句柄(用于控制LCD)
//#define DM9000A_FLOW_CONTROL
//#define DM9000A_UPTO_100M
//#define Fifo_Point_Check
//#define Point_Error_Reset
#define Fix_Note_Address
#define FifoPointCheck

/* DM9000A 接收函数设置宏 */
#define Rx_Int_enable
#define Max_Int_Count			1
#define Max_Ethernet_Lenth		1536
#define Broadcast_Jump
#define Max_Broadcast_Lenth		500

/* DM9000A 传送函数设置宏 */
#define Max_Send_Pack			2

#define NET_BASE_ADDR		0x68400000
#define NET_REG_ADDR		(*((volatile uint16_t *) NET_BASE_ADDR))
#define NET_REG_DATA		(*((volatile uint16_t *) (NET_BASE_ADDR + 0x00080000)))

#define ETH_ADDR_LEN			6

/* 定义网卡的 MAC 地址 */
static unsigned char DEF_MAC_ADDR[ETH_ADDR_LEN] = {0x01, 0x02, 0x01, 0x02, 0x01, 0x02};
uint8_t SendPackOk = 0;
uint8_t s_FSMC_Init_Ok = 0;	/* 用于指示FSMC是否初始化 */

uint16_t  receiveLen_DM9000;
uint8_t   receiveBuffer_DM9000[1528];
uint16_t  receiveLen;
uint8_t   receiveBuffer[1528];


//#define printk(...)
//#define printk printf

static void DM9K_CtrlLinesConfig(void);
static void DM9K_FSMCConfig(void);
void DM9000_Initnic(void);

/*
*********************************************************************************************************
*	函 数 名: DM9000_Init
*	功能说明: uIP 接口函数,初始化网卡.  uIP 接口函数.
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DM9000_Init(void)
{

	DM9K_CtrlLinesConfig();
	DM9K_FSMCConfig();
	 s_FSMC_Init_Ok = 1;

	DM9000_Initnic();			/* 配置DM9000 */
}

//SRAM底层驱动，时钟使能，引脚分配
//此函数会被HAL_SRAM_Init()调用
//hsram:SRAM句柄
void DM9K_CtrlLinesConfig(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
//	__HAL_RCC_SYSCFG_CLK_ENABLE();				//使能SYSCFG时钟
    __HAL_RCC_FMC_CLK_ENABLE();                 //使能FMC时钟
    __HAL_RCC_GPIOC_CLK_ENABLE();               //使能GPIOC时钟
    __HAL_RCC_GPIOD_CLK_ENABLE();               //使能GPIOD时钟
    __HAL_RCC_GPIOE_CLK_ENABLE();               //使能GPIOE时钟
    __HAL_RCC_GPIOF_CLK_ENABLE();               //使能GPIOF时钟
    __HAL_RCC_GPIOG_CLK_ENABLE();               //使能GPIOG时钟
	
	/* 设置 PD.00(D2), PD.01(D3), PD.04(NOE), PD.05(NWE), PD.08(D13), PD.09(D14),
	 PD.10(D15), PD.14(D0), PD.15(D1) 设置 PD.13(A18 (RS))   为复用推挽输出 */
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;  
    GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //推挽复用
    GPIO_Initure.Pull=GPIO_PULLUP;              //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;         //高速
    GPIO_Initure.Alternate=GPIO_AF12_FMC;       //复用为FMC    
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);         //初始化

	/* 设置 PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) 为复用推挽输出 */

//	GPIO_Initure.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
//	                   GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_Initure.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
	                   GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	HAL_GPIO_Init(GPIOE, &GPIO_Initure);

	/* 设置 PG10 (CS)) 为复用推挽输出 */
	GPIO_Initure.Pin = GPIO_PIN_10;
	HAL_GPIO_Init(GPIOG, &GPIO_Initure);


	/*dm9000复位硬引脚*/
	GPIO_Initure.Pin = GPIO_PIN_9; 		//PC9 推挽输出 
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;//推挽输出
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Initure.Pull  = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC,&GPIO_Initure);

	DM9000_RST;								//DM9000硬件复位
	Delay_ms(2);
	DM9000_SET; 							//DM9000硬件复位结束
	Delay_ms(300);						//一定要有这个延时，让DM9000准备就绪！
}

/*
*********************************************************************************************************
*	函 数 名: DM9K_FSMCConfig
*	功能说明: 配置FSMC并口访问时序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void DM9K_FSMCConfig(void)
{
	SRAM_HandleTypeDef hsram = {0};
	FMC_NORSRAM_TimingTypeDef SRAM_Timing = {0};

	hsram.Instance  = FMC_NORSRAM_DEVICE;
	hsram.Extended  = FMC_NORSRAM_EXTENDED_DEVICE;

	/* FMC使用的HCLK3，主频200MHz，1个FMC时钟周期就是5ns */
	SRAM_Timing.AddressSetupTime       = 8;  /* 4*5ns=20ns，地址建立时间，范围0 -15个FMC时钟周期个数 */
	SRAM_Timing.AddressHoldTime        = 2;  /* 地址保持时间，配置为模式A时，用不到此参数 范围1 -15个时钟周期个数 */
	SRAM_Timing.DataSetupTime          = 10;  /* 2*5ns=10ns，数据保持时间，范围1 -255个时钟周期个数 */
	SRAM_Timing.BusTurnAroundDuration  = 15; /* 15*5n=75ns, 两个连续数据之间的间隔, 范围0 -15个FMC时钟周期个数*/
	SRAM_Timing.CLKDivision            = 0;  /* 此配置用不到这个参数 */
	SRAM_Timing.DataLatency            = 0;  /* 此配置用不到这个参数 */
	SRAM_Timing.AccessMode             = FMC_ACCESS_MODE_A; /* 配置为模式A */

	
	hsram.Init.NSBank             = FMC_NORSRAM_BANK3;
	hsram.Init.DataAddressMux     = FMC_DATA_ADDRESS_MUX_ENABLE;
	hsram.Init.MemoryType         = FMC_MEMORY_TYPE_SRAM;
	hsram.Init.MemoryDataWidth    = FMC_NORSRAM_MEM_BUS_WIDTH_16;	/* 32位总线宽度 */
	hsram.Init.BurstAccessMode    = FMC_BURST_ACCESS_MODE_DISABLE;
	hsram.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
	hsram.Init.WaitSignalActive   = FMC_WAIT_TIMING_BEFORE_WS;
	hsram.Init.WriteOperation     = FMC_WRITE_OPERATION_ENABLE;
	hsram.Init.WaitSignal         = FMC_WAIT_SIGNAL_DISABLE;
	hsram.Init.ExtendedMode       = FMC_EXTENDED_MODE_DISABLE;
	hsram.Init.AsynchronousWait   = FMC_ASYNCHRONOUS_WAIT_DISABLE;
	hsram.Init.WriteBurst         = FMC_WRITE_BURST_DISABLE;
	hsram.Init.ContinuousClock    = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
    hsram.Init.WriteFifo		  = FMC_WRITE_FIFO_ENABLE;
	/* 初始化SRAM控制器 */
	if (HAL_SRAM_Init(&hsram, &SRAM_Timing, &SRAM_Timing) != HAL_OK)
	{
		/* 初始化错误 */
		Error_Handler();
	}
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_ReadReg
*	功能说明: 读出DM9000指定寄存器的值
*	形    参: reg 寄存器地址
*	返 回 值: 寄存器值
*********************************************************************************************************
*/
uint8_t dm9k_ReadReg(uint8_t reg)
{
	NET_REG_ADDR = reg;
	return (NET_REG_DATA);
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_WriteReg
*	功能说明: 读出DM9000指定寄存器的值
*	形    参: reg ：寄存器地址
*			 writedata : 写入的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void dm9k_WriteReg(uint8_t reg, uint8_t writedata)
{
	NET_REG_ADDR = reg;
	NET_REG_DATA = writedata;
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_hash_table
*	功能说明: 设置 DM9000A MAC 、 广播 、 多播 寄存器
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void dm9k_hash_table(void)
{
	uint8_t i;

//	/* 将MAC地址告诉uip */
//	for (i = 0; i < 6; i++)
//	{
//		uip_ethaddr.addr[i] = DEF_MAC_ADDR[i];
//	}

	/* 设置 网卡 MAC 位置，来自於 MyHardware */
	for(i = 0; i < 6; i++)
	{
		dm9k_WriteReg(DM9000_REG_PAR + i, DEF_MAC_ADDR[i]);
	}

	/* 清除 网卡多播设置 */
	for(i = 0; i < 8; i++)
	{
		dm9k_WriteReg(DM9000_REG_MAR + i, 0xFF);
	}

//	/* 设置 广播包 设置 */
//	dm9k_WriteReg(DM9000_REG_MAR + 7, 0x80);
}
/*
*********************************************************************************************************
*	函 数 名: dm9k_err_reset
*	功能说明: 发生指针错误时软件复位DM9000AE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void dm9k_err_reset(void)
{
 	dm9k_WriteReg(DM9000_REG_NCR,0x03);	   //第三步:软件复位DM9000 
	Delay_us(10); 	
	dm9k_WriteReg(DM9000_REG_NCR,0x03);	   //第五步:第二次软件复位DM9000 
	Delay_us(10); 	

		/* 基本记存器相关设置 */
	dm9k_WriteReg(DM9000_REG_IMR, DM9000_IMR_OFF); 			/* 开启内存自环模式 */
	dm9k_WriteReg(DM9000_REG_TCR2, DM9000_TCR2_SET);		/* 设置 LED 显示模式1:全双工亮，半双工灭 */
	/* 清除多余资讯 */
	dm9k_WriteReg(DM9000_REG_NSR, 0x2c);
	dm9k_WriteReg(DM9000_REG_TCR, 0x00);
	dm9k_WriteReg(DM9000_REG_ISR, 0x0f);
	dm9k_WriteReg(DM9000_REG_RCR, DM9000_RCR_SET);			/* 开启 接收工能 */
	dm9k_WriteReg(DM9000_REG_IMR, DM9000_IMR_OFF);			/* 关闭 中断模式 */
	SendPackOk = 0;
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_reset
*	功能说明: 软件复位DM9000AE
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void dm9k_reset(void)
{
	
//复位DM9000,复位步骤参考<DM9000 Application Notes V1.22>手册29页
	DM9000_RST;								//DM9000硬件复位
	Delay_ms(20);
	DM9000_SET; 							//DM9000硬件复位结束
	Delay_ms(100);								//一定要有这个延时，让DM9000准备就绪！
 	dm9k_WriteReg(DM9000_REG_GPCR,0x01);			//第一步:设置GPCR寄存器(0X1E)的bit0为1 
	dm9k_WriteReg(DM9000_REG_GPR,0);				//第二步:设置GPR寄存器(0X1F)的bit1为0，DM9000内部的PHY上电
	Delay_ms(20);                          //延时2ms以上等待PHY上电
 	dm9k_WriteReg(DM9000_REG_NCR,0x03);	   //第三步:软件复位DM9000 
  do 
	{ 
		Delay_ms(1); 
	}while(dm9k_ReadReg(DM9000_REG_NCR)&1);		//等待DM9000软复位完成
	dm9k_WriteReg(DM9000_REG_NCR,0x00);	   //第四步:软件复位完成，进入正常工作模式 
	dm9k_WriteReg(DM9000_REG_NCR,0x03);	   //第五步:第二次软件复位DM9000 
	do 
	{
		Delay_ms(1);
	}while(dm9k_ReadReg(DM9000_REG_NCR)&1);		//等待DM9000软复位完成
	dm9k_WriteReg(DM9000_REG_NCR,0x00);	   //第六步:软件复位完成，进入正常工作模式 	
		/* 基本记存器相关设置 */
	dm9k_WriteReg(DM9000_REG_IMR, DM9000_IMR_OFF); 			/* 开启内存自环模式 */
	dm9k_WriteReg(DM9000_REG_TCR2, DM9000_TCR2_SET);			/* 设置 LED 显示模式1:全双工亮，半双工灭 */
	/* 清除多余资讯 */
	dm9k_WriteReg(DM9000_REG_NSR, 0x2c);
	dm9k_WriteReg(DM9000_REG_TCR, 0x00);
	dm9k_WriteReg(DM9000_REG_ISR, 0x0f);
	dm9k_WriteReg(DM9000_REG_RCR, DM9000_RCR_SET);			/* 开启 接收工能 */
	dm9k_WriteReg(DM9000_REG_IMR, DM9000_IMR_OFF);			/* 关闭 中断模式 */
	SendPackOk = 0;
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_phy_write
*	功能说明: 软件复位DM9000AE
*	形    参: phy_reg ： PHY寄存器地址
*			  writedata ： 写入的数据
*	返 回 值: 无
*********************************************************************************************************
*/
void dm9k_phy_write(uint8_t phy_reg, uint16_t writedata)
{
	/* 设置写入 PHY 寄存器的位置 */
	dm9k_WriteReg(DM9000_REG_EPAR, phy_reg | DM9000_PHY);

	/* 设置写入 PHY 寄存器的值 */
	dm9k_WriteReg(DM9000_REG_EPDRH, ( writedata >> 8 ) & 0xff);
	dm9k_WriteReg(DM9000_REG_EPDRL, writedata & 0xff);

	dm9k_WriteReg(DM9000_REG_EPCR, 0x0a); 						/* 将资料写入 PHY 寄存器 */
	while(dm9k_ReadReg(DM9000_REG_EPCR) & 0x01);					/* 查寻是否执行结束 */
	Delay_ms(50);
//	dm9k_WriteReg(DM9000_REG_EPCR, 0x08); 						/* 清除写入命令 */
	 dm9k_WriteReg(DM9000_REG_EPCR, 0x00); 	
}
/*
*********************************************************************************************************
*	函数名: dm9k_phy_read
*	参  数: phy_reg ： 寄存器地址
*	返  回: 无
*	功  能: 读DM9000A PHY 寄存器
*********************************************************************************************************
*/
uint16_t dm9k_phy_read(uint8_t phy_reg)
{
	uint16_t readdata;

	/* 设置写入 PHY 寄存器的位置 */
	dm9k_WriteReg(DM9000_REG_EPAR, phy_reg | DM9000_PHY);

	/* 设置写入 PHY 寄存器的值 */
	dm9k_WriteReg(DM9000_REG_EPCR, 0x0C);						/* 将资料写入 PHY 寄存器 */
	
	while(dm9k_ReadReg(DM9000_REG_EPCR) & 0x01);				/* 查寻是否执行结束 */

	dm9k_WriteReg(DM9000_REG_EPCR, 0x00);						/* 清除读取命令 */	
	
	readdata = (dm9k_ReadReg(DM9000_REG_EPDRH) << 8) | (dm9k_ReadReg(DM9000_REG_EPDRL));
	
	return readdata;
}
/*
*********************************************************************************************************
*	函 数 名: DM9000_Initnic
*	功能说明: 配置DM9000AE芯片（初始化）
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DM9000_Initnic(void)
{
	
	dm9k_reset();									//复位DM9000
	
	dm9k_WriteReg(DM9000_REG_GPR, DM9000_PHY_OFF);			/* 关闭 PHY ，进行 PHY 设置*/
	dm9k_phy_write(0x00, 0x8000);					/* 重置 PHY 的寄存器 */
	dm9k_phy_write(0x04, 0x01e1);					/* 设置 自适应模式相容表 */
	dm9k_phy_write(0x00, 0x1000);					/* 设置 基本连接模式 */
//	/* 连接模式设置
//	  0x0000 : 固定10M半双工
//	  0x0100 : 固定10M全双工
//	  0x2000 : 固定100M半双工
//	  0x2100 : 固定100M全双工
//	  0x1000 : 自适应模式
//	*/
//	dm9k_phy_write(0x00, 0x2100);
	Delay_ms(2000);
	dm9k_WriteReg(DM9000_REG_GPR, DM9000_PHY_ON);				/* 结束 PHY 设置, 开启 PHY */
	Delay_ms(2000);
	dm9k_hash_table();								/* 设置 DM9000A MAC 及 多播*/
	
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_receive_packet
*	功能说明: 配置DM9000AE芯片（初始化）
*	形    参: _uip_buf : 接收结果存放的缓冲区指针
*	返 回 值: > 0 表示接收的数据长度, 0表示没有数据
*********************************************************************************************************
*/
uint16_t dm9k_receive_packet(void)
{
	uint16_t ReceiveData[1600];
	uint8_t  rx_int_count = 0;
	uint32_t  rx_checkbyte;
	uint16_t rx_status, rx_length;
	uint8_t  jump_packet;
	uint16_t i;
	uint16_t calc_len;
	uint16_t calc_MRR;
	uint16_t j;
	do
	{
		jump_packet = 0;								/* 清除跳包动作 */
		dm9k_ReadReg(DM9000_REG_MRCMDX);							/* 读取内存数据，地址不增加 */
		/* 计算内存数据位置 */
		calc_MRR = (dm9k_ReadReg(DM9000_REG_MRRH) << 8) + dm9k_ReadReg(DM9000_REG_MRRL);
		rx_checkbyte = dm9k_ReadReg(DM9000_REG_MRCMDX);			/*  */
		if(rx_checkbyte == DM9000_PKT_RDY)				/* 取 */
		{
			/* 读取封包相关资讯 及 长度 */
			NET_REG_ADDR = DM9000_REG_MRCMD;
			rx_status = NET_REG_DATA;
			rx_length = NET_REG_DATA;
			/* 若收到超过系统可承受的封包，此包跳过 */
			if(rx_length > Max_Ethernet_Lenth)
				jump_packet = 1;

#ifdef Broadcast_Jump
			/* 若收到的广播或多播包超过特定长度，此包跳过 */
			if(rx_status & 0x4000)
			{
				if(rx_length > Max_Broadcast_Lenth)
					jump_packet = 1;
			}
#endif
			/* 计算下一个包的指针位 , 若接收长度为奇数，需加一对齐偶字节。*/
			/* 若是超过 0x3fff ，则需回归绕到 0x0c00 起始位置 */
			calc_MRR += (rx_length + 4);
			if(rx_length & 0x01) calc_MRR++;
			if(calc_MRR > 0x3fff) calc_MRR -= 0x3400;

			if(jump_packet == 0x01)
			{
				/* 将指针移到下一个包的包头位置 */
				dm9k_WriteReg (DM9000_REG_MRRH, (calc_MRR >> 8) & 0xff);
				dm9k_WriteReg (DM9000_REG_MRRL, calc_MRR & 0xff );
				continue;
			}

			/* 开始将内存的资料搬到到系统中，每次移动一个 word */
			calc_len = (rx_length + 1) >> 1;
			for(i = 0 ; i < calc_len ; i++)
				ReceiveData[i] = NET_REG_DATA;
//			printf("DM9000 ReceiveData is:");
//			for(j=0;j<16;j++)
//			    printf("%x ", ReceiveData[j]);
//			printf("\n\r");	
			/* 将包长回报给 TCP/IP 上层，并减去最後 4 BYTE 的 CRC-32 检核码 */
			receiveLen_DM9000 = rx_length - 4;
            memcpy((unsigned char*)receiveBuffer_DM9000,(uint8_t *)ReceiveData,receiveLen_DM9000);
			rx_int_count++;								/* 累计收包次数 */
#ifdef FifoPointCheck
			if(calc_MRR != ((dm9k_ReadReg(DM9000_REG_MRRH) << 8) + dm9k_ReadReg(DM9000_REG_MRRL)))
			{
#ifdef Point_Error_Reset
				dm9k_reset();								/* 若是指针出错，重置 */
				return ReceiveLength;
#endif
				/*若是指针出错，将指针移到下一个包的包头位置  */
				dm9k_WriteReg(DM9000_REG_MRRH, (calc_MRR >> 8) & 0xff);
				dm9k_WriteReg(DM9000_REG_MRRL, calc_MRR & 0xff);
			}
#endif
			dm9k_WriteReg(DM9000_REG_MRRH, (calc_MRR >> 8) & 0xff);
			dm9k_WriteReg(DM9000_REG_MRRL, calc_MRR & 0xff);
			return receiveLen_DM9000;
		}
		else
		{
			if(rx_checkbyte == DM9000_PKT_NORDY)		/* 未收到包 */
			{
				dm9k_WriteReg(DM9000_REG_ISR, 0x3f);				/*  */
			}
			else
			{
				dm9k_err_reset();								/* 接收指针出错，重置 */
			}
			return (0);
		}
	}while(rx_int_count < Max_Int_Count);				/* 是否超过最多接收封包计数 */
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: dm9k_send_packet
*	功能说明: 发送一包数据
*	形    参: p_char : 发送数据缓冲区
*	返 回 值: length : 数据长度
*********************************************************************************************************
*/
void dm9k_send_packet(uint8_t *p_char, uint16_t length)
{
	uint16_t SendLength = length;
	uint16_t *SendData = (uint16_t *) p_char;
	uint16_t i;
	uint16_t calc_len;
	__IO uint16_t calc_MWR;
    uint32_t timer2;
    uint16_t j;
	/* 检查 DM9000A 是否还在传送中！若是等待直到传送结束 */
	dm9k_WriteReg(DM9000_REG_IMR,DM9000_IMR_OFF);		//关闭网卡中断 
//	if(SendPackOk == Max_Send_Pack)
//	{
//		timer2=bsp_GetTickCount();
//		while(dm9k_ReadReg(DM9000_REG_TCR) & DM9000_TCR_SET)
//		{
//			Delay_us (5);
//			if(bsp_GetTickCount()-timer2>=1000)
//				 break;
//		}
//		SendPackOk = 0;
//	}

//	SendPackOk++;										/* 设置传送计数 */
	NET_REG_ADDR = DM9000_REG_MWCMD;
	/* 开始将系统的资料搬到到内存中，每次移动一个 word */
	calc_len = (SendLength + 1) >> 1;
	for(i = 0; i < calc_len; i++)
		NET_REG_DATA = SendData[i];
	dm9k_WriteReg(DM9000_REG_TXPLH, (SendLength >> 8) & 0xff);	/* 设置传送封包的长度 */
	dm9k_WriteReg(DM9000_REG_TXPLL, SendLength & 0xff);
	dm9k_WriteReg(DM9000_REG_TCR, DM9000_TCR_SET);				/* 进行传送 */
	timer2=bsp_GetTickCount();
	while(dm9k_ReadReg(DM9000_REG_TCR)&0x01)
	   {  
			if(bsp_GetTickCount()-timer2>=100)
				 break;
		 }
	dm9k_WriteReg(DM9000_REG_NSR, 0x2c);
	dm9k_WriteReg(DM9000_REG_IMR , DM9000_IMR_OFF);
//	dm9k_err_reset();
	return;
}
/*
*********************************************************************************************************
*	函 数 名: dm9k_interrupt
*	功能说明: 中断处理函数 (webserver例程未使用中断)
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void  dm9k_interrupt(void)
{
	uint8_t  save_reg;
	uint16_t isr_status;

	save_reg = NET_REG_ADDR;							/* 暂存所使用的位置 */
	dm9k_WriteReg(DM9000_REG_IMR , DM9000_IMR_OFF);				/* 关闭 DM9000A 中断 */
	isr_status = dm9k_ReadReg(DM9000_REG_ISR);					/* 取得中断产生值 */

	if (isr_status & DM9000_RX_INTR)
	{ 					/* 检查是否为接收中断 */
		//dm9k_receive_packet();							/* 执行接收处理程序 */
	}
	dm9k_WriteReg(DM9000_REG_IMR , DM9000_IMR_SET);				/* 开启 DM9000A 中断 */
	NET_REG_ADDR = save_reg;							/* 回复所使用的位置 */

}
/**
  * @brief  PHY外部中断服务程序
  * @param  无
  * @retval 无
  */
//void EXTI15_10_IRQHandler(void)
//{
//	 EXTI_ClearITPendingBit(EXTI_Line15);
//	if(EXTI_GetITStatus(EXTI_Line15) != RESET)
//  {
//    dm9k_interrupt();
//    /* Clear interrupt pending bit */
//   
//  }
//}
/*******************************************************************************
*	函数名: dm9k_ReadID
*	参  数: 无
*	返  回: 无
*	功  能: 读取芯片ID
*/
uint32_t dm9k_ReadID(void)
{
	uint8_t vid1,vid2,pid1,pid2;

//	if (s_FSMC_Init_Ok == 0)
//	{
//		DM9K_CtrlLinesConfig();/* FSMC IO口配置 */
//		DM9K_FSMCConfig();/* FSMC 参数配置 */

//		s_FSMC_Init_Ok = 1;
//	}
	vid1 = dm9k_ReadReg(DM9000_REG_VID_L) & 0xFF;/* DM9000 的vid值低位 */
	vid2 = dm9k_ReadReg(DM9000_REG_VID_H) & 0xFF;/* DM9000 的vid值高位 */
	pid1 = dm9k_ReadReg(DM9000_REG_PID_L) & 0xFF;/* DM9000 的pid值低位 */
	pid2 = dm9k_ReadReg(DM9000_REG_PID_H) & 0xFF;/* DM9000 的pid值高位 */
	return (vid2 << 24) | (vid1 << 16) | (pid2 << 8) | pid1;
}

