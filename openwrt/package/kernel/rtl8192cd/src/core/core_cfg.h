#ifndef	_CORE_CFG_H_
#define _CORE_CFG_H_

#if defined(CONFIG_RTL_ULINKER_BRSC)
#include "linux/ulinker_brsc.h"
#endif

#ifdef CONFIG_ARCH_RTL8198F
#define USE_RTL8186_SDK
// CN2 also use the exact name "CONFIG_RTL_MULTI_LAN_DEV"
#define CONFIG_RTL_8198F
#define CONFIG_ENABLE_CCI400
#undef CONFIG_RTL_MULTI_LAN_DEV
#endif

#ifdef __OSK__
	#if !defined(CONFIG_RTL_819X) && !defined(CONFIG_RTL8672)
		#define CONFIG_RTL8672
	#endif
	#define USE_RTL8186_SDK
	#define CONFIG_RTL8196C
	#define CONFIG_RTL_8196C
#endif

//#define _LITTLE_ENDIAN_
//#define _BIG_ENDIAN_

//this is for WLAN HAL driver coexist with not HAL driver for code size reduce
#ifdef CONFIG_RTL_WLAN_HAL_NOT_EXIST
#define CONFIG_WLAN_NOT_HAL_EXIST 1
#else
#define CONFIG_WLAN_NOT_HAL_EXIST 0//96e_92e, 8881a_92e, 8881a_only, 96d_92er, is only HAL driver
#endif

#ifdef __ECOS
#define CONFIG_RTL_WLAN_CONF_TXT_NOT_EXIST
#endif

#ifdef CONFIG_RTL_WLAN_CONF_TXT_NOT_EXIST
#define CONFIG_WLAN_CONF_TXT_EXIST 0
#else
#define CONFIG_WLAN_CONF_TXT_EXIST 1
#endif

#ifdef __MIPSEB__

#ifndef _BIG_ENDIAN_
	#define _BIG_ENDIAN_
#endif

#ifdef _LITTLE_ENDIAN_
#undef _LITTLE_ENDIAN_
#endif
//### add by sen_liu 2011.4.14 CONFIG_NET_PCI defined in V2.4 and CONFIG_PCI
// define now to replace it. However,some modules still use CONFIG_NET_PCI
#ifdef CONFIG_PCI
#define CONFIG_NET_PCI
#endif

//### end
#endif	//__MIPSEB__

#if defined(__MIPSEL__) || defined(__ARMEL__)
#ifndef _LITTLE_ENDIAN_
	#define _LITTLE_ENDIAN_
#endif

#ifdef _BIG_ENDIAN_
#undef _BIG_ENDIAN_
#endif

#endif //__MIPSEL__ or __ARMEL__

//-------------------------------------------------------------
// Linux kernel header
//-------------------------------------------------------------
#ifdef __KERNEL__
#include "../osdep_service_linux.h"

#ifdef __LINUX_2_6_26__
// use seq_file to display big file to avoid buffer overrun when using create_proc_entry
#ifndef CONFIG_RTL_PROC_NEW
#define CONFIG_RTL_PROC_NEW
#endif
#endif // __LINUX_2_6_26__

// Include kernel configuration header, i.e. CONFIG_XXX
#ifndef __LINUX_2_6_20__
#include <linux/config.h>
#elif defined(__LINUX_3_4__)
#include <linux/kconfig.h>
#elif !(defined(__LINUX_2_6_33__))
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#ifdef __LINUX_2_6_20__
#include <linux/jiffies.h>
#include <asm/param.h>
#endif
#endif // __KERNEL__

//-------------------------------------------------------------
// Type definition
//-------------------------------------------------------------
#include "typedef.h"

#ifdef __ECOS
	#include <pkgconf/system.h>
	#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
	#include <pkgconf/devs_eth_rltk_819x_wlan.h>
	#include <sys/param.h>
#ifndef RTLPKG_DEVS_ETH_RLTK_819X_RX_ZERO_COPY
    #define CONFIG_RTL8190_PRIV_SKB
#endif
#endif
#ifdef __LINUX_2_6_29__ // linux 2.6.29
#define NETDEV_NO_PRIV
#endif

#define SWQ_TIMER_NUM	(NUM_STAT*4)
#define SWQ_HWTIMER_TOLERANCE	500 // us
#define SWQ_HWTIMER_MINIMUN	1000 // us
#define SWQ_HWTIMER_MAXIMUN	500000 // us
#define SWQ_HWTIMER_DELAY	34 // us
#define SWQ_TIMEOUT_THD  300 // ms
#define SWQ_TIMEOUT_THD_LOWER  3 // count

#define RC_ENTRY_NUM		128
#define RC_TIMER_NUM		64

#define GKEY_ID_SECOND 		2
#define AMSDU_TIMER_NUM		64
#define TUPLE_WINDOW		128
#define NUM_TXPKT_QUEUE		64
#define NUM_APSD_TXPKT_QUEUE	32
#define GBWC_TO_MILISECS	250
#define TX_SC_ENTRY_NUM		4
#define RX_SC_ENTRY_NUM		4
#define MAX_FRAG_COUNT		16

// for counting association number
#define INCREASE		1
#define DECREASE		0

#define NUM_RX_DESC_IF(priv)  (priv->pshare->wlandev_idx == (0^WLANIDX)?NUM_RX_DESC:NUM_RX_DESC_2G)
#define CURRENT_NUM_TX_DESC	priv->pshare->current_num_tx_desc


#ifdef OSK_LOW_TX_DESC
	#define BE_TXDESC			(NUM_TX_DESC)
	#define NONBE_TXDESC		(BE_TXDESC>>2)
	#define NUM_TX_DESC_HQ		(NUM_TX_DESC>>3)
#else
	#define BE_TXDESC			(NUM_TX_DESC)
	#define NONBE_TXDESC		(NUM_TX_DESC)
	#define NUM_TX_DESC_HQ		(NUM_TX_DESC>>3)
#endif

//This is for LUNA SDK - Apollo to config 8812 in slave CPU and shift mem 33M
#if defined(CONFIG_ARCH_LUNA_SLAVE) && !defined(CONFIG_WLAN_HAL)
#define CONFIG_LUNA_SLAVE_PHYMEM_OFFSET CONFIG_RTL8686_DSP_MEM_BASE
#else
#define CONFIG_LUNA_SLAVE_PHYMEM_OFFSET 0x0
#endif

//-------------------------------------------------------------
// Refine for multi-station
//-------------------------------------------------------------
#if defined(CONFIG_RTL_8812_SUPPORT) || defined(CONFIG_WLAN_HAL) 
#define MULTI_STA_REFINE
#endif
#ifdef MULTI_STA_REFINE
#define PKTAGINGTIME 1200
#define MAXPAUSEDSTA 24
#define MAXPAUSEDQUEUE 7
#define LOWAGGRESTA	8
#define STA_TX_LOWRATE ((txcfg->pstat->ht_cap_buf.support_mcs[2])? _MCS8_RATE_:_MCS4_RATE_)
#endif


//-------------------------------------------------------------
// to prevent broadcast storm attacks
//-------------------------------------------------------------
#define PREVENT_BROADCAST_STORM	1

//-------------------------------------------------------------
// to prevent ARP spoofing attacks
//-------------------------------------------------------------
#ifdef PREVENT_BROADCAST_STORM
/*
 *	NOTE: The driver will skip the other broadcast packets if the system free memory is less than FREE_MEM_LOWER_BOUND 
 *		   and the broadcast packet amount is larger than BROADCAST_STORM_THRESHOLD in one second period.
 */

#define BROADCAST_STORM_THRESHOLD		50 //16
#define FREE_MEM_LOWER_BOUND			800 //uint: KBytes
#endif

#endif
