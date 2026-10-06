/**
  *  @file 8192cd_h2d_tx.c of H2D Tx process from HostCPU to Data-CPU  (Use 8814A to simulation)
  *  @brief Handle Tx-processes in HostCPU
  *
  *  Packet Offload Engine will help RTK WiFi Chip to decrease host platform CPU utilization.
  *  This functon will handle Tx-processes in Host-CPU
  *
  *  Copyright (c) 2015 Realtek Semiconductor Corp.
  *
  *  @author Peter Yu
  *  @date 2015/10/21
 **/

#define _8192CD_CORE_TX_H2D_C_

#ifdef __KERNEL__
#include <linux/if_arp.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/tcp.h>
#endif

#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "./sys-support.h"
#endif

#ifdef RTL8192CD_VARIABLE_USED_DMEM
#include "./8192cd_dmem.h"
#endif

#if defined(CONFIG_RTL_WAPI_SUPPORT)
#include "wapiCrypto.h"
#endif
#if defined (CONFIG_RTL_VLAN_8021Q) || defined (CONFIG_PUMA_VLAN_8021Q)
#include <linux/if_vlan.h>
#endif

#ifdef CONFIG_RTL867X_VLAN_MAPPING
#include "../../re_vlan.h"
#endif

#ifdef PERF_DUMP
#include "romeperf.h"
#endif

#ifdef TAROKO_0
//no HalComTXDesc.h
#else
#include "HalHeader/HalComTXDesc.h"
#endif
#include "WlanHAL/HalPrecomp.h"

__IRAM_IN_865X
int check_full_ofld(struct rtl8192cd_priv *priv, 
                            struct stat_info *pstat, 
                            struct sk_buff *pskb, 
                            struct tx_insn* txcfg)
{
    WPA_STA_INFO *pWpaInfo;
	
    if (OPMODE & WIFI_MP_STATE)
        return 0;
    
    if (!pstat)
        return 0;

    if (pskb->cb[_SKB_CB_FLAGS] & (_SKB_CB_FLAGS_URGENT | _SKB_CB_FLAGS_DHCP))
        return 0;

    if (SWCRYPTO)
        return 0;

    pWpaInfo = pstat->wpa_sta_info;
    if (pWpaInfo) {
        if (pWpaInfo->UnicastCipher == DOT11_ENC_TKIP)
            return 0;
    }

    if(pstat->aggre_mthd & AGGRE_MTHD_MPDU)
        txcfg->frag_thrshld = 2346;
    else
        txcfg->frag_thrshld = FRAGTHRSLD - _CRCLNG_;

    if(pskb->len > txcfg->frag_thrshld)
        return 0;

    return 1;
}
                                        
/** @brief --transmit the packet to Data-CPU
  *
  * @param param_out None
  * @param param_in  priv   : which interface.
  *                  pstat  : which station
  *                  skb    : packets content
  *                  txcfg  : tx config for singin
 **/
__IRAM_IN_865X
int rtl8192cd_xmit_offload(struct stat_info *pstat, struct sk_buff *pskb, struct tx_insn* txcfg)
{
    struct net_device *dev = pskb->dev;
    struct rtl8192cd_priv *priv = GET_DEV_PRIV(dev);
    struct rtl8192cd_hw *phw = GET_HW(priv);
    int ret = SUCCESS;
    unsigned int q_num, pri, req_txbd;
    
    /*  Full offload
     *   - pstat should not be NULL
     *   - mcast pkt go through here with 'isMC2UC' set
     */
     
    pri = pskb->cb[_SKB_CB_PRIORITY];
    q_num = pskb->cb[_SKB_CB_QNUM];

#ifdef TX_AMSDU_REFINE
    req_txbd = 7;
#else
    req_txbd = 4;
#endif

    if (getAvailableTXBD(priv, q_num) < NUM_TX_DESC/4) {
        rtl8192cd_tx_queueDsr(priv, q_num);
        if (getAvailableTXBD(priv, q_num) < req_txbd) {
		priv->ext_stats.tx_drops++;
#ifdef TRX_VI_DATA_LOG
		if (q_num == VI_QUEUE)
			priv->ext_stats.tx_vi_drop_pkts++;
#endif
#ifdef __LINUX_2_6_33__
		printk_ratelimited(KERN_WARNING "%s: no available TXBD\n", __FUNCTION__);
#else
		DEBUG_WARN("%s: no available TXBD\n", __FUNCTION__);
#endif
                goto free_and_stop;
        }
    }

    //set txcfg
    txcfg->pstat = pstat;
    txcfg->fr_type = _SKB_FRAME_TYPE_;
    txcfg->pframe = pskb;
    txcfg->fr_len = pskb->len;
    txcfg->hdr_len = 0;
    txcfg->q_num = q_num;
    if (txcfg->h2d_type != TXPKTINFO_TYPE_AMSDU)
        txcfg->h2d_type = TXPKTINFO_TYPE_8023;

    /* BA construction */
    if (!pstat->ADDBA_ready[pri]) {
        if ((pstat->ADDBA_req_num[pri] < 5) && !pstat->ADDBA_sent[pri]) {
            pstat->ADDBA_req_num[pri]++;
            SMP_UNLOCK_XMIT(flags);
            //issue_ADDBAreq(priv, pstat, pri);
            prepare_ADDBAreq(priv, pstat, pri);
            SMP_LOCK_XMIT(flags);
            pstat->ADDBA_sent[pri]++;
        }
            if (pstat->ADDBA_sent[pri])
                pstat->ADDBA_accu[pri]++;
    }

    if (txcfg->aggre_en == 0) {
        if ((pstat->aggre_mthd & AGGRE_MTHD_MPDU) && 
                pstat->ADDBA_ready[pri])
            txcfg->aggre_en = FG_AGGRE_MPDU;
    }

    txcfg->privacy = get_sta_encrypt_algthm(priv, pstat);

    // for SW LED
    priv->pshare->LED_tx_cnt++;

    tx_sum_up(priv, txcfg->pstat, txcfg);
    if (q_num == VO_QUEUE)
        phw->VO_pkt_count++;
    else if (q_num == VI_QUEUE)
        phw->VI_pkt_count++;
    else if (q_num == BK_QUEUE)
        phw->BK_pkt_count++;
    else if (q_num == BE_QUEUE)
        phw->BE_pkt_count++;

    rtl88XX_signin_offload_htxdesc(priv, txcfg);

    goto stop_proc;

free_and_stop:        /* Free current packet and stop TX process */
   
    if (pskb)
        rtl_kfree_skb(priv, pskb, _SKB_TX_);

stop_proc:
    return ret;
}

/** @brief -- MappingChannelDMA
  * 
 **/
static int MappingChannelDMA(int TxQNum)
{
	switch (TxQNum) {
	case BK_QUEUE:
		return HCI_TXDMA_CHAN_03;
	case BE_QUEUE:
		return HCI_TXDMA_CHAN_02;
	case VI_QUEUE:
		return HCI_TXDMA_CHAN_01;
	case VO_QUEUE:
		return HCI_TXDMA_CHAN_00;
	case MGNT_QUEUE:
		return 14;
	case HIGH_QUEUE_V2:
	case HIGH_QUEUE1_V2:
	case HIGH_QUEUE2_V2:
	case HIGH_QUEUE3_V2:
	case HIGH_QUEUE4_V2:
	case HIGH_QUEUE5_V2:
	case HIGH_QUEUE6_V2:
	case HIGH_QUEUE7_V2:
	case HIGH_QUEUE8_V1:
	case HIGH_QUEUE9_V1:
	case HIGH_QUEUE10_V1:
	case HIGH_QUEUE11_V1:
	case HIGH_QUEUE12_V1:
	case HIGH_QUEUE13_V1:
	case HIGH_QUEUE14_V1:
	case HIGH_QUEUE15_V1:
	case HIGH_QUEUE16_V1:
	case HIGH_QUEUE17_V1:
	case HIGH_QUEUE18_V1:
	case HIGH_QUEUE19_V1:
		return 15;
	case CMD_QUEUE_V2:
		return 16;
	case BEACON_QUEUE:
		return 19;
	default: 
        printk("%s: Unknown ChannelDMA Mapping (%d)\n", __func__, TxQNum);
		return HCI_TXDMA_CHAN_00;
	}
}


/** @brief -- MappingQSel
  * 
 **/
static int MappingQSel(HAL_PADAPTER Adapter, int TxQNum, int Tid)
{
	switch (TxQNum) {
	case HIGH_QUEUE:
	case HIGH_QUEUE1:
	case HIGH_QUEUE2:
	case HIGH_QUEUE3:
	case HIGH_QUEUE4:
	case HIGH_QUEUE5:
	case HIGH_QUEUE6:
	case HIGH_QUEUE7:
		return TXDESC_QSEL_HIGH;
	case MGNT_QUEUE: 
		return TXDESC_QSEL_MGT;
	case CMD_QUEUE_V2: 
		return 20;
	case BEACON_QUEUE: 
		return TXDESC_QSEL_BCN;
	default: 
#if CFG_HAL_RTL_MANUAL_EDCA
    	if (HAL_VAR_MANUAL_EDCA) {
	    	switch (TxQNum) {
	    	case VO_QUEUE:
	    		return TXDESC_QSEL_TID6;
	    	case VI_QUEUE:
	    		return TXDESC_QSEL_TID4;
	    	case BE_QUEUE:
	    		return TXDESC_QSEL_TID0;
	    	default:
	    		return TXDESC_QSEL_TID1;
	    	}
    	} else
    		return Tid;
#else
        return Tid;
#endif
	}
}

/** @brief -- fill pktinfo.
  *
  * @param param_out None
  * @param param_in  priv:  which Wi-Fi interface.
  *                  pstat: which station.
  * @return 0: OK, 1: others
 **/
__IRAM_IN_865X int rtl88XX_fill_txPktInfo(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, unsigned char pktType, unsigned char *phdr)
{
    struct h2d_txpkt_info *pTxPktInfo;

    if (phdr)
        pTxPktInfo = (struct h2d_txpkt_info *)(phdr - sizeof(struct h2d_txpkt_info));
    else
        pTxPktInfo = (struct h2d_txpkt_info *)(txcfg->phdr - sizeof(struct h2d_txpkt_info));
    
    memset((char *)pTxPktInfo, 0, sizeof(struct h2d_txpkt_info));

	pTxPktInfo->ver = 0;
	pTxPktInfo->dummy_len = 0;

    pTxPktInfo->pkt_type = pktType;
    pTxPktInfo->ifid = priv->if_id;
    /* use macid0 if h2d */
    if (txcfg->h2d_type == TXPKTINFO_TYPE_H2D)
        pTxPktInfo->macid = 0;
    else
        pTxPktInfo->macid = (txcfg->pstat ? REMAP_AID(txcfg->pstat) : 0);
    if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST)
        pTxPktInfo->last_msdu = 1;
    /* FIXME: fix correct macid for each interface */
    pTxPktInfo->tid = txcfg->h2d_tid;
#ifdef _BIG_ENDIAN_
{
    int *pswap = (int *)pTxPktInfo;
    pswap[0] = cpu_to_le32(pswap[0]);
    pswap[1] = cpu_to_le32(pswap[1]);
    pswap[2] = cpu_to_le32(pswap[2]);
    pswap[3] = cpu_to_le32(pswap[3]);
}	
#endif	

    return 0;
}

/** @brief -- fill txDMAinfo.
  *
  * @param param_out None
  * @param param_in  priv:  which Wi-Fi interface.
  *                  pstat: which station.
  *                  txcfg : tx-config
  * @return 0: OK, 1: others
 **/
static __IRAM_IN_865X int rtl88XX_fill_txDMAInfo(struct rtl8192cd_priv *priv, struct tx_insn *txcfg, PH2D_TXDMA_INFO_DATA pTxdma_Info)
{
    struct stat_info *pstat = txcfg->pstat;
    
    if (txcfg->h2d_type == TXPKTINFO_TYPE_H2D) {
        pTxdma_Info->dw0_ie_end = 1;
        pTxdma_Info->dw0_agg_en = 0;
        pTxdma_Info->dw0_pkt_offset = 0;
        pTxdma_Info->dw0_offset = offsetof(H2D_TXDMA_INFO_DATA, txie);
    }
    else {
        pTxdma_Info->dw0_ie_end = 1;
        pTxdma_Info->dw0_agg_en = !!txcfg->aggre_en;  //Aggregate this MPDU if possible.
        pTxdma_Info->dw0_pkt_offset = 0;
        pTxdma_Info->dw0_offset = 16;
			
        if (pstat) {
#ifdef A4_STA
            if (pstat->state & WIFI_A4_STA)
                pTxdma_Info->dw1_mc2u = 0;
            else
#endif
                pTxdma_Info->dw1_mc2u = txcfg->isMC2UC;
            pTxdma_Info->dw1_txbw = pstat->tx_bw;
            pTxdma_Info->dw2_mimops = pstat->MIMO_ps;
        }
        if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST)
            pTxdma_Info->dw2_last_mpdu = 1;
        
        pTxdma_Info->dw1_enc = (txcfg->privacy ? 1 : 0);
    }

    pTxdma_Info->dw1_full_ofld = 1;
    
    pTxdma_Info->dw2_h2d_type = txcfg->h2d_type;
    pTxdma_Info->dw2_if_id = priv->if_id;
    pTxdma_Info->dw2_h2d_tid = txcfg->h2d_tid;

    if (txcfg->h2d_type == TXPKTINFO_TYPE_8023 || txcfg->h2d_type == TXPKTINFO_TYPE_AMSDU) {
        pTxdma_Info->dw0_txpktsize = txcfg->fr_len + txcfg->hdr_len;
    } else {
        pTxdma_Info->dw0_txpktsize = txcfg->fr_len + txcfg->hdr_len + txcfg->llc;
        if (txcfg->privacy) {
            if (txcfg->use_sw_enc)
                pTxdma_Info->dw0_txpktsize += txcfg->iv + txcfg->icv + txcfg->mic;
            else
                pTxdma_Info->dw0_txpktsize += txcfg->iv;
        }
    }
    pTxdma_Info->dw1_qsel = MappingQSel(priv, txcfg->q_num, txcfg->h2d_tid);
    pTxdma_Info->dw1_macid = (txcfg->pstat ? REMAP_AID(txcfg->pstat) : 0);

    if (txcfg->h2d_type == TXPKTINFO_TYPE_AMSDU)
        pTxdma_Info->dw2_dma_pri = 1;

    pTxdma_Info->dw3_dma_channel = MappingChannelDMA(txcfg->q_num);
    pTxdma_Info->dw3_offload_size = pTxdma_Info->dw0_offset + pTxdma_Info->dw0_txpktsize;

    return 0;
}

VOID
SetTxBufferDesc88XX (
    IN      HAL_PADAPTER    Adapter,
    IN      u4Byte          queueIndex,  //HCI_TX_DMA_QUEUE_88XX
    IN      PVOID           pDescData,
    IN      u1Byte          setTxbdSource
);

#ifdef TX_AMSDU_REFINE
void rtl88XX_signin_offload_htxdesc_amsdu(struct rtl8192cd_priv *priv, struct tx_insn *txcfg,
	TX_DESC_DATA_88XX *pdesc_data)
{
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	u4Byte q_num = txcfg->q_num;
	u32 halQNum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, q_num);

	PHCI_TX_DMA_MANAGER_88XX ph2d_tx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
	PHCI_TX_DMA_QUEUE_STRUCT_88XX cur_q = &(ph2d_tx_dma->tx_queue[halQNum]);
	PTX_DESC_88XX ptx_desc;
	PH2D_TXDMA_INFO_DATA pTxdma_Info = pdesc_data->ph2d_txdma_info;
        
	pswdescinfo = get_txdesc_info(priv, priv->pshare->pdesc_info, q_num);
	pdescinfo = pswdescinfo + cur_q->host_idx;
	
	ptx_desc = ((PTX_DESC_88XX)(cur_q->ptx_desc_head)) + cur_q->host_idx;

    pTxdma_Info->dw2_last_mpdu = 0;
	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST)
		pTxdma_Info->dw2_last_mpdu = 1;
	pTxdma_Info->dw0_txpktsize = txcfg->fr_len + txcfg->hdr_len;
	pTxdma_Info->dw3_offload_size = pTxdma_Info->dw0_offset + pTxdma_Info->dw0_txpktsize;
#ifdef _BIG_ENDIAN_
	{
		int i;
		u4Byte *src = (u4Byte *)pTxdma_Info;
		u4Byte *dst = (u4Byte *)ptx_desc;
		for (i = 0; i < SIZE_TXDESC_BODY/4; i++)
			*dst++ = cpu_to_le32(*src++);
	}
#else
        memcpy(ptx_desc, pTxdma_Info, SIZE_TXDESC_BODY);
#endif

	pdesc_data->pBuf = ((struct sk_buff *)txcfg->pframe)->data;
	pdesc_data->frLen = txcfg->fr_len;
	SetTxBufferDesc88XX(priv, halQNum, pdesc_data, SET_TXBD_SOURCE_NORMAL);
	
    /* for tx-recycle using */
#ifdef ENABLE_RTL_SKB_STATS
	pdescinfo->priv = priv;
#endif
	pdescinfo->type = _RESERVED_FRAME_TYPE_;
	//pdescinfo->pframe = NULL;
	//pdescinfo->len = 0;
	//pdescinfo->paddr = 0;
	pdescinfo->buf_type[0] = txcfg->fr_type;
	pdescinfo->buf_pframe[0] = txcfg->pframe;
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
	pdescinfo->buf_len[0] = txcfg->fr_len;
	pdescinfo->buf_paddr[0] = pdesc_data->buf_dma_addr;
#endif
}
#endif // TX_AMSDU_REFINE

/** @brief -- prepare HTXBD/HTXDesc/pktinfo for offload path.
  *
  * @param param_out None
  * @param param_in  priv  : which Wi-Fi interface.
  *                  pstat : which station.
  *                  txcfg : tx-config
  * @return 0: OK, 1: others
 **/
__IRAM_IN_865X int rtl88XX_signin_offload_htxdesc(struct rtl8192cd_priv *priv, struct tx_insn *txcfg)
{
    struct tx_desc_info     *pswdescinfo, *pdescinfo;
    u4Byte                  q_num = txcfg->q_num;
    struct rtl8192cd_hw     *phw = GET_HW(priv);
    u32 halQNum = GET_HAL_INTERFACE(priv)->MappingTxQueueHandler(priv, q_num);

    PHCI_TX_DMA_MANAGER_88XX ph2d_tx_dma = (PHCI_TX_DMA_MANAGER_88XX)(_GET_HAL_DATA(priv)->PTxDMA88XX);
    PHCI_TX_DMA_QUEUE_STRUCT_88XX cur_q = &(ph2d_tx_dma->tx_queue[halQNum]);
    H2D_TXDMA_INFO             h2d_txdma_info;
    TX_DESC_DATA_88XX      h2d_desc_data;

    memset(&h2d_desc_data, 0, sizeof(TX_DESC_DATA_88XX));
    memset(&h2d_txdma_info, 0, sizeof(H2D_TXDMA_INFO));

    pswdescinfo = get_txdesc_info(priv, priv->pshare->pdesc_info, q_num);
    pdescinfo = pswdescinfo + cur_q->host_idx;

    if(txcfg->fr_type == _SKB_FRAME_TYPE_) {
        txcfg->h2d_tid = ((struct sk_buff *)txcfg->pframe)->cb[1];
    }

    rtl88XX_fill_txDMAInfo(priv, txcfg, (PH2D_TXDMA_INFO_DATA)(&h2d_txdma_info));

    h2d_desc_data.pHdr = NULL;
    h2d_desc_data.hdrLen = 0;

    if (txcfg->fr_type == _SKB_FRAME_TYPE_)
        h2d_desc_data.pBuf = ((struct sk_buff *)txcfg->pframe)->data;
    else
        h2d_desc_data.pBuf = (unsigned char*)txcfg->pframe;

    h2d_desc_data.frLen = txcfg->fr_len;
    h2d_desc_data.ph2d_txdma_info = (PVOID)&h2d_txdma_info;

    GET_HAL_INTERFACE(priv)->FillTxHwCtrlHandler(priv, halQNum, (void *)&h2d_desc_data);

    /* for tx-recycle using */
#ifdef ENABLE_RTL_SKB_STATS
	pdescinfo->priv = priv;
#endif
    if ((txcfg->fr_type == _SKB_FRAME_TYPE_))
        pdescinfo->type = _PRE_ALLOCLLCHDR_;
    else
        pdescinfo->type = _PRE_ALLOCHDR_;
    pdescinfo->pframe = txcfg->phdr;
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
    pdescinfo->len = h2d_desc_data.hdrLen;
    pdescinfo->paddr = h2d_desc_data.hdr_dma_addr;
#endif

    if (txcfg->fr_len != 0) {
        pdescinfo->buf_type[0]      = txcfg->fr_type;
        pdescinfo->buf_pframe[0]    = txcfg->pframe;
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
        pdescinfo->buf_len[0]       = txcfg->fr_len;
        pdescinfo->buf_paddr[0]     = h2d_desc_data.buf_dma_addr;
#endif
    }

#ifdef TX_AMSDU_REFINE
	if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
		struct sk_buff *pskb = (struct sk_buff *)txcfg->pframe;
		if (pskb->cb[_SKB_CB_FLAGS] & _SKB_CB_FLAGS_AMSDU_SEG_LIST) {
			struct sk_buff *next_skb;
			
			pskb->cb[_SKB_CB_FLAGS] &= ~_SKB_CB_FLAGS_AMSDU_SEG_LIST;
			pskb = pskb->next;
			while (pskb) {
				next_skb = pskb->next;
				txcfg->pframe = pskb;
				txcfg->aggre_en = (next_skb ? FG_AGGRE_MSDU_MIDDLE : FG_AGGRE_MSDU_LAST);
				txcfg->fr_len = pskb->len;
				tx_sum_up(priv, txcfg->pstat, txcfg);
				priv->pshare->LED_tx_cnt++;
				rtl88XX_signin_offload_htxdesc_amsdu(priv, txcfg, &h2d_desc_data);
				
				pskb = next_skb;
			}
		}
	}
#endif

    GET_HAL_INTERFACE(priv)->SyncSWTXBDHostIdxToHWHandler(priv, halQNum);

    return SUCCESS;
}
