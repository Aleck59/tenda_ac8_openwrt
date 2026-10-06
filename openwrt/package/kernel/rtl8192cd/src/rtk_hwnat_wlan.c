/*
 *  HWNAT handle routines
 *
 *  Copyright (c) 2018 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_HWNAT_C_

#include "rtk_hwnat_wlan.h"

#ifdef CONFIG_HWNAT_NETIF_RX_QUEUE_SUPPORT
void process_hwnat_netif_rx_queue(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct sk_buff *pskb;

	while (skb_queue_len(&priv->pshare->hwnat_netif_rx_queue) > 0) {
		pskb = skb_dequeue(&priv->pshare->hwnat_netif_rx_queue);
		if (pskb == NULL) {
			break;
		}
#if (defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION)||defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)) && !defined(CONFIG_ARCH_LUNA_SLAVE)
		rtk_hwnat_netif_rx(pskb);
#else
		netif_rx(pskb);
#endif
	}
}
#endif

#if !defined(CONFIG_ARCH_LUNA_SLAVE)
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) || (defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_RTL_ETH_RECYCLED_SKB))
void rtk_hwnat_netif_rx(struct sk_buff *pskb)
{
    extern int fwdEngine_wifi_rx(struct sk_buff *skb);
    enum {
        RE8670_RX_STOP=0,
        RE8670_RX_CONTINUE,
        RE8670_RX_STOP_SKBNOFREE,
        RE8670_RX_END
    };
    int ret;

#if defined(CONFIG_RTL_TRIBAND_SUPPORT)
    if (pskb->dev->netdev_ops->ndo_start_xmit == rtl8192cd_start_xmit) {
        netif_rx(pskb);
        return;
    }
#endif

    pskb->data-=14;
    pskb->len+=14;
    //printk("[%s]\n",pskb->dev->name);
    ret=fwdEngine_wifi_rx(pskb);
    
    if(ret==RE8670_RX_CONTINUE)
    {
        pskb->data+=14;
        pskb->len-=14;
        //printk("WLAN0 rx, fwdEngine is handled, trap to netif_rx\n");
        netif_rx(pskb); 
    }
    else if(ret==RE8670_RX_STOP)
    {
        kfree_skb(pskb);
    }
}

#endif // defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) || (defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_RTL_ETH_RECYCLED_SKB))
#endif // !defined(CONFIG_ARCH_LUNA_SLAVE)
