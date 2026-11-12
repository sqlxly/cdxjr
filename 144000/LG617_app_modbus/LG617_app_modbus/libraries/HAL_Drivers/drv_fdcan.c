/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-07-06     thread-liu   first version
 */

#include "board.h"

#if defined(BSP_USING_FDCAN1) || defined(BSP_USING_FDCAN2)

#include "drv_fdcan.h"

//#define DRV_DEBUG
#define LOG_TAG             "drv.fdcan"
#include <drv_log.h>

#define setbitv(x,y)  x|=(1<<y)   //位读写
#define clrbitv(x,y)  x&=~(1<<y) 
#define getbitv(x,y)  ((x) >> (y)&1)

struct stm32_fdcan
{
    struct rt_device dev;
    FDCAN_HandleTypeDef fdcan;
    FDCAN_FilterTypeDef filter;
    FDCAN_TxHeaderTypeDef tx_config;
    FDCAN_RxHeaderTypeDef rx_config;
    volatile rt_uint8_t fifo0;
    volatile rt_uint8_t fifo1;
};
static struct stm32_fdcan rt_fdcan = {0};
static struct rt_semaphore rx_sem;     /* can recv msg semaphore */
static struct rt_can_msg fdcanRxMsg[8];
static uint8_t recvMark = 0;

extern void updateSensorStatus(int index);
void setModbusHoldingReg(uint16_t addr, uint16_t v);

static rt_err_t rt_fdcan_init(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);
    struct stm32_fdcan *device = (struct stm32_fdcan *)dev;

    device->fdcan.Instance                  = FDCAN1;
    device->fdcan.Init.FrameFormat          = FDCAN_FRAME_CLASSIC;
    device->fdcan.Init.Mode                 = FDCAN_MODE_NORMAL;
    device->fdcan.Init.AutoRetransmission   = DISABLE;
    device->fdcan.Init.TransmitPause        = DISABLE;
    device->fdcan.Init.ProtocolException    = DISABLE;
    device->fdcan.Init.NominalPrescaler     = 0x01; /* tq = NominalPrescaler x (1/fdcan_ker_ck) */
    device->fdcan.Init.NominalSyncJumpWidth = 0x08;
    device->fdcan.Init.NominalTimeSeg1      = 0x1F; /* NominalTimeSeg1 = Propagation_segment + Phase_segment_1 */
    device->fdcan.Init.NominalTimeSeg2      = 0x08;
		device->fdcan.Init.DataPrescaler        = 0x01;
		device->fdcan.Init.DataSyncJumpWidth    = 0x08;
		device->fdcan.Init.DataTimeSeg1         = 0x1F; /* DataTimeSeg1 = Propagation_segment + Phase_segment_1 */
		device->fdcan.Init.DataTimeSeg2         = 0x08;
    device->fdcan.Init.MessageRAMOffset     = 0;
    device->fdcan.Init.StdFiltersNbr        = 0;
    device->fdcan.Init.ExtFiltersNbr        = 2;
    device->fdcan.Init.RxFifo0ElmtsNbr      = 32;
    device->fdcan.Init.RxFifo0ElmtSize      = FDCAN_DATA_BYTES_8;
    device->fdcan.Init.RxFifo1ElmtsNbr      = 0;
    device->fdcan.Init.RxFifo1ElmtSize      = FDCAN_DATA_BYTES_8;
    device->fdcan.Init.RxBuffersNbr         = 0;
		device->fdcan.Init.RxBufferSize				  = FDCAN_DATA_BYTES_8;
    device->fdcan.Init.TxEventsNbr          = 0;
    device->fdcan.Init.TxBuffersNbr         = 0;
    device->fdcan.Init.TxFifoQueueElmtsNbr  = 32;
    device->fdcan.Init.TxFifoQueueMode      = FDCAN_TX_FIFO_OPERATION;
    device->fdcan.Init.TxElmtSize           = FDCAN_DATA_BYTES_8;

    if (HAL_FDCAN_Init(&device->fdcan) != HAL_OK)
    {
        return RT_ERROR;
    }

    device->filter.IdType             = FDCAN_EXTENDED_ID;
    device->filter.FilterIndex        = 0;
    device->filter.FilterType         = FDCAN_FILTER_MASK;
    device->filter.FilterConfig       = FDCAN_FILTER_TO_RXFIFO0;
    device->filter.FilterID1          = 0x061602c0;
    device->filter.FilterID2          = 0x1ffffff0;
    if (HAL_FDCAN_ConfigFilter(&device->fdcan, &device->filter)!=HAL_OK)
    {
        return RT_ERROR;
    }

    device->filter.FilterIndex        = 1;
    device->filter.FilterID1          = 0x061608c0;
    if (HAL_FDCAN_ConfigFilter(&device->fdcan, &device->filter)!=HAL_OK)
    {
        return RT_ERROR;
    }
		
		HAL_FDCAN_ConfigGlobalFilter(&device->fdcan, FDCAN_REJECT, FDCAN_REJECT, FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE);
		HAL_FDCAN_Start(&device->fdcan);
    HAL_FDCAN_ActivateNotification(&device->fdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0); /* open rx fifo0 new message it */
		
    device->fifo0 = RESET;
    device->fifo1 = RESET;

		rt_sem_init(&rx_sem, "fdcan_sem", 0, RT_IPC_FLAG_FIFO);
		
    return RT_EOK;
}

static rt_err_t rt_fdcan_open(rt_device_t dev, rt_uint16_t oflag)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_fdcan_close(rt_device_t dev)
{
    RT_ASSERT(dev != RT_NULL);

    return RT_EOK;
}

static rt_err_t rt_fdcan_control(rt_device_t dev, int cmd, void *args)
{
    RT_ASSERT(dev != RT_NULL);
    struct stm32_fdcan *device = (struct stm32_fdcan *)dev;

    switch (cmd)
    {
        case FDCAN_MODE_NORMAL:
            device->fdcan.Init.Mode  = FDCAN_MODE_NORMAL;
            break;
        case FDCAN_MODE_INTERNAL_LOOPBACK:
            device->fdcan.Init.Mode  = FDCAN_MODE_INTERNAL_LOOPBACK;
            break;
        default:
            break;
    }

    HAL_FDCAN_Init(&device->fdcan);

    return RT_EOK;
}

void reset_fdcan_read_status(void)
{
		rt_sem_control(&rx_sem, RT_IPC_CMD_RESET, 0);
		recvMark = 0;
		rt_memset(fdcanRxMsg, 0, sizeof(struct rt_can_msg)*8);
}

static rt_size_t rt_fdcan_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);
		rt_size_t rSize = 0;
    struct stm32_fdcan *device = (struct stm32_fdcan *)dev;
		struct rt_can_msg *pmsg;

		rt_sem_take(&rx_sem, 30);
	
    if (rt_fdcan.fifo0 == SET)
    {
        rt_fdcan.fifo0 = RESET;
				
				rSize = recvMark;
			
				rt_memcpy(buffer, fdcanRxMsg, sizeof(struct rt_can_msg)*8);
			
        return rSize;
    }
    if (rt_fdcan.fifo1 == SET)
    {
        rt_fdcan.fifo0 = RESET;
			
				if (HAL_FDCAN_GetRxMessage(&device->fdcan, FDCAN_RX_FIFO1, &device->rx_config, (uint8_t *)pmsg->data) != HAL_OK)
				{
						LOG_E("get msg error from fdcan fifo1!");
						return 0;
				}
				
        return rSize;
    }

    return 0;
}

static rt_size_t rt_fdcan_write(rt_device_t dev, rt_off_t pos, const void *buffer, rt_size_t size)
{
    RT_ASSERT(dev != RT_NULL);
    struct stm32_fdcan *device = (struct stm32_fdcan *)dev;
		struct rt_can_msg *pmsg = (struct rt_can_msg *) buffer;

    device->tx_config.Identifier          = pmsg->id;
    device->tx_config.IdType              = FDCAN_EXTENDED_ID;
    device->tx_config.TxFrameType         = FDCAN_DATA_FRAME;
    device->tx_config.DataLength          = FDCAN_DLC_BYTES_8;
    device->tx_config.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    device->tx_config.BitRateSwitch       = FDCAN_BRS_OFF;
    device->tx_config.FDFormat            = FDCAN_CLASSIC_CAN;
    device->tx_config.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
    device->tx_config.MessageMarker       = 0x0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&device->fdcan, &device->tx_config, (uint8_t *)pmsg->data) != HAL_OK)
    {
        return RT_ERROR;
    }

    return RT_EOK;
}

void FDCAN1_IT0_IRQHandler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    HAL_FDCAN_IRQHandler(&rt_fdcan.fdcan);

    /* leave interrupt */
    rt_interrupt_leave();
}

void FDCAN1_IT1_IRQHandler(void)
{
   /* enter interrupt */
   rt_interrupt_enter();

   HAL_FDCAN_IRQHandler(&rt_fdcan.fdcan);

   /* leave interrupt */
   rt_interrupt_leave();
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
		FDCAN_RxHeaderTypeDef rx_config;
		uint8_t rx_data[8], cmd, addr;
		uint8_t sensor_pos, param_pos;
		uint16_t calib_cnt = 0;
		uint32_t rx_id = 0, iValue = 0;
	
    if (hfdcan->Instance == FDCAN1)
    {
        if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
        {
            rt_fdcan.fifo0 = SET;

						if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_config, (uint8_t *)rx_data) != HAL_OK)
						{
								LOG_E("get msg error from fdcan fifo0!");
						}

						rx_id = rx_config.Identifier;
						cmd = ((rx_id >> 8) & 0xff);
						switch(cmd)
						{
							case 0x02:		//接收到禽蛋的裂纹信息
								addr = (rx_id & 0x0f);
								if((addr >= 1)&&(addr <= 6)){
										fdcanRxMsg[addr - 1].ide = rx_config.IdType;
										fdcanRxMsg[addr - 1].id = rx_config.Identifier;
										rt_memcpy(fdcanRxMsg[addr - 1].data, rx_data, 8);
										setbitv(recvMark, (addr - 1));
								}
								if(recvMark == 0x3f){
									rt_sem_release(&rx_sem);
								}
								break;
							case 0x08:		//接收到裂纹传感器校准值 直接跟新到modbus寄存器中就可以了 不需要应用程序读取和处理
								addr = (rx_id & 0x0f);
								if((addr >= 1)&&(addr <= 6)){
									sensor_pos = rx_data[0];
									param_pos = rx_data[1];
									if(200 == sensor_pos){   //显示保存成功
										setModbusHoldingReg(1601, 2);
										break;
									}
									iValue = rx_data[2];
									iValue <<= 8;
									iValue |= rx_data[3];
									iValue <<= 8;
									iValue |= rx_data[4];
									iValue <<= 8;
									iValue |= rx_data[5];
									//跟新到modbus的对应寄存器位置
									switch(param_pos)
									{
										case 0:
											setModbusHoldingReg(1250+sensor_pos, iValue);
											break;
										case 1:
											setModbusHoldingReg(1300+sensor_pos, iValue);
											break;
										case 2:
											calib_cnt = ((rx_data[6]&0x00ff)<<8) + (rx_data[7]&0x00ff);
											iValue = (iValue > 655350) ? 655350 : iValue;
											setModbusHoldingReg(1350+sensor_pos, iValue/10);
											setModbusHoldingReg(1400+sensor_pos, calib_cnt);
											updateSensorStatus(sensor_pos);
											break;
										default: break;
									}
								}
								break;
							default: break;
						}

						HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
        }
    }
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo1ITs)
{
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != RESET)
    {
        rt_fdcan.fifo1 = SET;
				
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0);
    }
}

int fdcan_init(void)
{
    rt_fdcan.dev.type      = RT_Device_Class_CAN;
    rt_fdcan.dev.init      = rt_fdcan_init;
    rt_fdcan.dev.open      = rt_fdcan_open;
    rt_fdcan.dev.close     = rt_fdcan_close;
    rt_fdcan.dev.read      = rt_fdcan_read;
    rt_fdcan.dev.write     = rt_fdcan_write;
    rt_fdcan.dev.control   = rt_fdcan_control;
    rt_fdcan.dev.user_data = RT_NULL;

    rt_device_register(&rt_fdcan.dev, "fdcan1", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_REMOVABLE | RT_DEVICE_FLAG_STANDALONE);

    LOG_I("fdcan1 init success!");

    return RT_EOK;
}

INIT_DEVICE_EXPORT(fdcan_init);

#if 0
#include <finsh.h>
#include <stdlib.h>

int fdcan_sample(int argc, char **argv)
{
    rt_err_t result = RT_EOK;
    rt_uint8_t i, rx_buf[8], tx_buf[8];
    struct rt_device *dev = RT_NULL;

    if (argc != 9)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("fdcan_sample 1 2 3 4 5 6 7 8\n");
        return -1;
    }

    for (i = 0; i < 8; i++)
    {
        tx_buf[i] = atoi(argv[i+1]);
    }

    dev = rt_device_find("fdcan1");
    if (dev == RT_NULL)
    {
        rt_kprintf("can't find fdcan1 device!\n");
        return RT_ERROR;
    }
    rt_device_open(dev, RT_DEVICE_OFLAG_RDWR);

    rt_device_write(dev, 0, tx_buf, 8);
    rt_thread_delay(1);
    rt_device_read(dev, 0, rx_buf, 8);

    rt_kprintf("fdcan1 loopback test over, rbuf = ");
    for (i = 0; i < 8; i++)
    {
        rt_kprintf(" %x ", rx_buf[i]);
    }
    rt_kprintf("\n");

    return result;
}

MSH_CMD_EXPORT(fdcan_sample, fdcan loopback mode test);

#endif

#endif
