#ifndef _DM9161_H_
#define _DM9161_H_


#define DM9161addr 1

#define  DM9161_STATUS_READ_ERROR            ((int32_t)-5)
#define  DM9161_STATUS_WRITE_ERROR           ((int32_t)-4)
#define  DM9161_STATUS_ADDRESS_ERROR         ((int32_t)-3)
#define  DM9161_STATUS_RESET_TIMEOUT         ((int32_t)-2)
#define  DM9161_STATUS_ERROR                 ((int32_t)-1)
#define  DM9161_STATUS_OK                    ((int32_t) 0)
#define  DM9161_STATUS_LINK_DOWN             ((int32_t) 1)
#define  DM9161_STATUS_100MBITS_FULLDUPLEX   ((int32_t) 2)
#define  DM9161_STATUS_100MBITS_HALFDUPLEX   ((int32_t) 3)
#define  DM9161_STATUS_10MBITS_FULLDUPLEX    ((int32_t) 4)
#define  DM9161_STATUS_10MBITS_HALFDUPLEX    ((int32_t) 5)
#define  DM9161_STATUS_AUTONEGO_NOTDONE      ((int32_t) 6)



#define DM9161_BCR      ((uint16_t)0x0000U)
#define DM9161_BSR      ((uint16_t)0x0001U)
#define DM9161_PHYI1R   ((uint16_t)0x0002U)
#define DM9161_PHYI2R   ((uint16_t)0x0003U)
#define DM9161_ANAR     ((uint16_t)0x0004U)
#define DM9161_ANLPAR   ((uint16_t)0x0005U)
#define DM9161_ANER     ((uint16_t)0x0006U)
#define DM9161_ANNPTR   ((uint16_t)0x0007U)
#define DM9161_ANNPRR   ((uint16_t)0x0008U)
#define DM9161_MMDACR   ((uint16_t)0x000DU)
#define DM9161_MMDAADR  ((uint16_t)0x000EU)
#define DM9161_ENCTR    ((uint16_t)0x0010U)
#define DM9161_MCSR     ((uint16_t)0x0011U)
#define DM9161_SMR      ((uint16_t)0x0012U)
#define DM9161_TPDCR    ((uint16_t)0x0018U)
#define DM9161_TCSR     ((uint16_t)0x0019U)
#define DM9161_SECR     ((uint16_t)0x001AU)
#define DM9161_SCSIR    ((uint16_t)0x001BU)
#define DM9161_CLR      ((uint16_t)0x001CU)
#define DM9161_ISFR     ((uint16_t)0x001DU)
#define DM9161_IMR      ((uint16_t)0x001EU)
#define DM9161_PHYSCSR  ((uint16_t)0x001FU)

#define DM9161_SMR_MODE       ((uint16_t)0x00E0U)
#define DM9161_SMR_PHY_ADDR   ((uint16_t)0x001FU)

#define DM9161_BCR_SOFT_RESET         ((uint16_t)0x8000U)
#define DM9161_BCR_LOOPBACK           ((uint16_t)0x4000U)
#define DM9161_BCR_SPEED_SELECT       ((uint16_t)0x2000U)
#define DM9161_BCR_AUTONEGO_EN        ((uint16_t)0x1000U)
#define DM9161_BCR_POWER_DOWN         ((uint16_t)0x0800U)
#define DM9161_BCR_ISOLATE            ((uint16_t)0x0400U)
#define DM9161_BCR_RESTART_AUTONEGO   ((uint16_t)0x0200U)
#define DM9161_BCR_DUPLEX_MODE        ((uint16_t)0x0100U) 

#define DM9161_BSR_100BASE_T4       ((uint16_t)0x8000U)
#define DM9161_BSR_100BASE_TX_FD    ((uint16_t)0x4000U)
#define DM9161_BSR_100BASE_TX_HD    ((uint16_t)0x2000U)
#define DM9161_BSR_10BASE_T_FD      ((uint16_t)0x1000U)
#define DM9161_BSR_10BASE_T_HD      ((uint16_t)0x0800U)
#define DM9161_BSR_100BASE_T2_FD    ((uint16_t)0x0400U)
#define DM9161_BSR_100BASE_T2_HD    ((uint16_t)0x0200U)
#define DM9161_BSR_EXTENDED_STATUS  ((uint16_t)0x0100U)
#define DM9161_BSR_AUTONEGO_CPLT    ((uint16_t)0x0020U)
#define DM9161_BSR_REMOTE_FAULT     ((uint16_t)0x0010U)
#define DM9161_BSR_AUTONEGO_ABILITY ((uint16_t)0x0008U)
#define DM9161_BSR_LINK_STATUS      ((uint16_t)0x0004U)
#define DM9161_BSR_JABBER_DETECT    ((uint16_t)0x0002U)
#define DM9161_BSR_EXTENDED_CAP     ((uint16_t)0x0001U)

#define DM9161_PHYSCSR_AUTONEGO_DONE   ((uint16_t)0x1000U)
#define DM9161_PHYSCSR_HCDSPEEDMASK    ((uint16_t)0x001CU)
#define DM9161_PHYSCSR_10BT_HD         ((uint16_t)0x0004U)
#define DM9161_PHYSCSR_10BT_FD         ((uint16_t)0x0014U)
#define DM9161_PHYSCSR_100BTX_HD       ((uint16_t)0x0008U)
#define DM9161_PHYSCSR_100BTX_FD       ((uint16_t)0x0018U) 

int DM9161_Init(void);
int DM9161_GetLinkState(void);


#endif


