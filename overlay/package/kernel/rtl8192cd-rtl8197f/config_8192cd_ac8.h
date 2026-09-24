/*
 * rtl8192cd build configuration for the Tenda AC8 v1 radio pair:
 * integrated RTL8197FH 2.4 GHz + RTL8812F(R) 5 GHz on PCIe slot 1.
 *
 * The port ships a config_8192cd.h for the Tenda MW5 (RTL8822B on
 * PCIe), which makes the C code reference RTL8822B tables that the
 * 8812F build does not compile.  This header replaces it for the ac8
 * variant and mirrors the MW5 header except for the external radio.
 * The RFE type of the 8812F on the AC8 is not known yet (0 assumed).
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
#define CONFIG_USE_PCIE_SLOT_1 1
#define CONFIG_SLOT_1_8812FE 1
#define CONFIG_SLOT_1_RFE_TYPE_0 1
#define CONFIG_OPENWRT_SDK 1
#define _LITTLE_ENDIAN_ 1
#define NOT_RTK_BSP 1
#define CONFIG_MW5_NO_LED 1
#endif
