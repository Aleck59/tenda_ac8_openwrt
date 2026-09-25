/*
 * rtl8192cd build configuration for the Tenda AC8 v1 radio pair:
 * integrated RTL8197FH 2.4 GHz + RTL8812F(R) 5 GHz on PCIe.
 *
 * The port ships a config_8192cd.h for the Tenda MW5 (RTL8822B on
 * PCIe), which makes the C code reference RTL8822B tables that the
 * 8812F build does not compile.  This header replaces it for the ac8
 * variant.
 *
 * - The RTL8197F has one PCIe port: the external radio sits in slot 0.
 *   CONFIG_USE_PCIE_SLOT_0 with CONFIG_RTL_8197F is also the vendor's
 *   dual-band configuration (CONCURRENT_MODE: per-radio index, RX rings
 *   and buffer pools).
 * - RFE type 0 for the 8812F, as the stock firmware sets it.
 * - CONFIG_RTL8197F_WMAC_PLATFORM: the integrated radio is probed from
 *   its DT node (realtek,rtl8197f-wmac) and gets a device for the DMA
 *   API, see 8192cd_osdep.c.
 */
#ifndef __AC8_CONFIG_8192CD_H__
#define __AC8_CONFIG_8192CD_H__
#define CONFIG_RTL8192CD 1
#define CONFIG_WIRELESS_LAN_MODULE 1
#define CONFIG_WIFI_HCI 1
#define CONFIG_PCI_HCI 1
#define CONFIG_WLAN_HAL 1
#define CONFIG_WLAN_HAL_88XX 1
#define CONFIG_WLAN_HAL_8197F 1
#define CONFIG_WLAN_HAL_8812FE 1
#define CONFIG_NET_PCI 1
#define CONFIG_RTL_ODM_WLAN_DRIVER 1
#define CONFIG_WLAN_MACHAL_API 1
#define CONFIG_WLAN_MACHAL_API_V0 1
#define CONFIG_RTL_VAP_SUPPORT 1
#define CONFIG_RTL_CLIENT_MODE_SUPPORT 1
#define CONFIG_RTL_REPEATER_MODE_SUPPORT 1
#define CONFIG_RTL_80211D_SUPPORT 1
#define CONFIG_RTL_11W_SUPPORT 1
#define CONFIG_TXPWR_LMT 1
#define CONFIG_RTL_8197F 1
#define CONFIG_RTL_8197F_VG 1
#define CONFIG_RTL_DISABLE_WLAN_MIPS16 1
#define CONFIG_BAND_2G_ON_WLAN0 1
#define CONFIG_USE_PCIE_SLOT_0 1
#define CONFIG_SLOT_0_8812FE 1
#define CONFIG_SLOT_0_RFE_TYPE_0 1
#define CONFIG_RTL8197F_WMAC_PLATFORM 1
#define CONFIG_OPENWRT_SDK 1
#define _LITTLE_ENDIAN_ 1
#define NOT_RTK_BSP 1
#define CONFIG_MW5_NO_LED 1
#endif
