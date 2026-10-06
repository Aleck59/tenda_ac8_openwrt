/**
  *  @file Hal88XXHTxDesc.h  : Data structure of HTXBD/HTxDMAInfo/TxPktInfo  (Use 8814A to simulation)
  *  @brief Handle HTXBD/TXdESC/TxPktInfo of Tx-processes in HostCPU
  *
  *  Packet Offload Engine will help RTK WiFi Chip to decrease host platform CPU utilization.
  *  This functon will handle Tx-processes in Host-CPU
  *
  *  Copyright (c) 2015 Realtek Semiconductor Corp.
  *
  *  @author Peter Yu
  *  @date 2015/11/4
 **/

#ifndef _HAL88XXHTXDESC_H
#define _HAL88XXHTXDESC_H

typedef enum _HCI_TX_DMA_QUEUE_OFLD_PEOFLD_
{
//only for software ph2d_tx_dma->tx_queue use
    HCI_TXQUEUE_MGT_PEOFLD = 0,		
    HCI_TXQUEUE_BCN_PEOFLD,		
    HCI_TXQUEUE_CMD_PEOFLD,		
    HCI_TXQUEUE_HI0_PEOFLD,  	
    HCI_TXQUEUE_HI1_PEOFLD,
    HCI_TXQUEUE_HI2_PEOFLD,
    HCI_TXQUEUE_HI3_PEOFLD,
    HCI_TXQUEUE_HI4_PEOFLD,
    HCI_TXQUEUE_HI5_PEOFLD,
    HCI_TXQUEUE_HI6_PEOFLD,
    HCI_TXQUEUE_HI7_PEOFLD,
    HCI_TXQUEUE_HI8_PEOFLD,
    HCI_TXQUEUE_HI9_PEOFLD,
    HCI_TXQUEUE_HI10_PEOFLD,
    HCI_TXQUEUE_HI11_PEOFLD,
    HCI_TXQUEUE_HI12_PEOFLD,
    HCI_TXQUEUE_HI13_PEOFLD,
    HCI_TXQUEUE_HI14_PEOFLD,
    HCI_TXQUEUE_HI15_PEOFLD,
    HCI_TXQUEUE_HI16_PEOFLD,
    HCI_TXQUEUE_HI17_PEOFLD,
    HCI_TXQUEUE_HI18_PEOFLD,
    HCI_TXQUEUE_HI19_PEOFLD,

    /* keep last */
    HCI_TXQUEUE_MAX_NUM_PEOFLD
} HCI_TX_DMA_QUEUE_OFLD_PEOFLD, *PHCI_TX_DMA_QUEUE_OFLD_PEOFLD;


#define H2D_PEOFLD_TX_DMA_QUEUE_MAX_NUM    HCI_TXQUEUE_MAX_NUM_PEOFLD

typedef struct _H2D_PEOFLD_TX_DMA_MANAGER_
{
    HCI_TX_DMA_QUEUE_STRUCT_88XX tx_queue[H2D_PEOFLD_TX_DMA_QUEUE_MAX_NUM];
} H2D_PEOFLD_TX_DMA_MANAGER, *PH2D_PEOFLD_TX_DMA_MANAGER;

enum rt_status
PrepareH2D_TXBD88XX(
    IN      HAL_PADAPTER    Adapter
);

#endif //#ifndef _HAL88XXHTXDESC_H
