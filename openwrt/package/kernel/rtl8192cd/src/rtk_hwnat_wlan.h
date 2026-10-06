#ifndef _RTK_HWNAT_WLAN_H_
#define _RTK_HWNAT_WLAN_H_

#ifdef __KERNEL__
#include <linux/netdevice.h>
#endif

#include "8192cd_cfg.h"
#include "8192cd.h"
#if !defined(CONFIG_RTK_SOC_RTL8198D) && defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
#include <rtk_rg_wlan_internal.h>
#endif

#if !defined(CONFIG_ARCH_LUNA_SLAVE)
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION)
void rtk_hwnat_netif_rx(struct sk_buff *pskb);
#elif defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)
#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
// New fwdEngine API based on new NIC skb recycle mechanism
extern int rtk_fc_fastfwd_netif_rx(struct sk_buff *skb);
#define rtk_hwnat_netif_rx(x) rtk_fc_fastfwd_netif_rx(x)
#else
void rtk_hwnat_netif_rx(struct sk_buff *pskb);
#endif // CONFIG_RTL_ETH_RECYCLED_SKB
#endif // defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION)
#endif // !defined(CONFIG_ARCH_LUNA_SLAVE)

#ifdef CONFIG_HWNAT_NETIF_RX_QUEUE_SUPPORT
void process_hwnat_netif_rx_queue(unsigned long task_priv);
#endif

#endif //end of #ifndef _RTK_HWNAT_WLAN_H_
