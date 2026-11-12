#ifndef _SPI_DMA_MASTER_H_
#define _SPI_DMA_MASTER_H_

#include "stm32h7xx.h"

#define	SPI_BUFFER_SIZE		(4 * 1024)				/*  */

/* 重定义下SPI SCK时钟，方便移植 */
#define SPI_BAUDRATEPRESCALER_100M      SPI_BAUDRATEPRESCALER_2			/* 100M */
#define SPI_BAUDRATEPRESCALER_50M       SPI_BAUDRATEPRESCALER_4			/* 50M */
#define SPI_BAUDRATEPRESCALER_12_5M     SPI_BAUDRATEPRESCALER_8			/* 12.5M */
#define SPI_BAUDRATEPRESCALER_6_25M     SPI_BAUDRATEPRESCALER_16		/* 6.25M */
#define SPI_BAUDRATEPRESCALER_3_125M    SPI_BAUDRATEPRESCALER_32		/* 3.125M */
#define SPI_BAUDRATEPRESCALER_1_5625M   SPI_BAUDRATEPRESCALER_64		/* 1.5625M */
#define SPI_BAUDRATEPRESCALER_781_25K   SPI_BAUDRATEPRESCALER_128		/* 781.25K */
#define SPI_BAUDRATEPRESCALER_390_625K  SPI_BAUDRATEPRESCALER_256		/* 390.625K */

void bsp_InitSPIBus(void);
void bsp_spiTransfer(void);
void bsp_InitSPIParam(uint32_t _BaudRatePrescaler, uint32_t _CLKPhase, uint32_t _CLKPolarity);

extern uint8_t g_spiTxBuf[SPI_BUFFER_SIZE];
extern uint8_t g_spiRxBuf[SPI_BUFFER_SIZE];
extern uint32_t g_spiLen;

extern uint8_t g_spi_busy;

enum {
	TRANSFER_WAIT,
	TRANSFER_ACTIVE,
	TRANSFER_COMPLETE,
	TRANSFER_ERROR
};

extern __IO uint32_t wTransferState;

#endif


/* Master 0, Slave 1, "SSC-Device"
 * Vendor ID:       0x00123456
 * Product code:    0x26483052
 * Revision number: 0x00030111
 */

//ec_pdo_entry_info_t slave_1_pdo_entries[] = {
//    {0x7010, 0x01, 16}, /* oData 1 */
//    {0x7010, 0x02, 16}, /* oData 2 */
//    {0x7010, 0x03, 16}, /* oData 3 */
//    {0x7010, 0x04, 16}, /* oData 4 */
//    {0x7010, 0x05, 16}, /* oData 5 */
//    {0x7010, 0x06, 16}, /* oData 6 */
//    {0x7010, 0x07, 16}, /* oData 7 */
//    {0x7010, 0x08, 16}, /* oData 8 */
//    {0x6000, 0x01, 16}, /* iData 1 */
//    {0x6000, 0x02, 16}, /* iData 2 */
//    {0x6000, 0x03, 16}, /* iData 3 */
//    {0x6000, 0x04, 16}, /* iData 4 */
//    {0x6000, 0x05, 16}, /* iData 5 */
//    {0x6000, 0x06, 16}, /* iData 6 */
//    {0x6000, 0x07, 16}, /* iData 7 */
//    {0x6000, 0x08, 16}, /* iData 8 */
//    {0x6020, 0x01, 1 }, /* Underrange */
//    {0x6020, 0x02, 1 }, /* Overrange */
//    {0x6020, 0x03, 2 }, /* Limit 1 */
//    {0x6020, 0x05, 2 }, /* Limit 2 */
//    {0x0000, 0x00, 8 }, /* Gap */
//    {0x1802, 0x07, 1 }, /* TxPDOState */
//    {0x1802, 0x09, 1 }, /* TxPDO Toggle */
//    {0x6020, 0x11, 16}, /* Analog input */
//};

//ec_pdo_info_t slave_1_pdos[] = {
//    {0x1601, 8, slave_1_pdo_entries + 0 }, /* DO RxPDO-Map */
//    {0x1a00, 8, slave_1_pdo_entries + 8 }, /* DI TxPDO-Map */
//    {0x1a02, 8, slave_1_pdo_entries + 16}, /* AI TxPDO-Map */
//};

//ec_sync_info_t slave_1_syncs[] = {
//    {0, EC_DIR_OUTPUT, 0, NULL, 						EC_WD_DISABLE},
//    {1, EC_DIR_INPUT,  0, NULL, 						EC_WD_DISABLE},
//    {2, EC_DIR_OUTPUT, 1, slave_1_pdos + 0, EC_WD_ENABLE},
//    {3, EC_DIR_INPUT,  2, slave_1_pdos + 1, EC_WD_DISABLE},
//    {0xff}
//};
