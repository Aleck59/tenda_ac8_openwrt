# SPDX-License-Identifier: GPL-2.0-only
#
# Realtek vendor Wi-Fi driver (rtl8192cd) for the RTL8197F integrated 2.4 GHz
# radio and the RTL8812F 5 GHz PCIe radio, built in cfg80211 (OpenWrt SDK)
# mode so that hostapd/wpa_supplicant drive it through nl80211.
#
# Source: rtl8192cd from Cudy's GP3000 GPL release (linux-5.10.x tree), the
# Linux 6.18 port follows Putpocket/iptime-a2004mu-openwrt.

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8192cd
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING
PKG_BUILD_PARALLEL:=1
PKG_FLAGS:=nonshared

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192cd
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8197F (2.4 GHz) + RTL8812F (5 GHz) vendor driver
  DEPENDS:=@TARGET_rtl819x_rtl8197f +kmod-cfg80211 +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8192cd.ko
  AUTOLOAD:=$(call AutoLoad,50,rtl8192cd)
endef

define KernelPackage/rtl8192cd/description
  Realtek rtl8192cd driver for the RTL8197F on-SoC 2.4 GHz radio and the
  RTL8812F PCIe 5 GHz radio. Calibration and MAC addresses are read from
  the factory NVRAM block through NVMEM.
endef

define KernelPackage/rtl8192cd/config
  if PACKAGE_kmod-rtl8192cd

  config RTL8192CD_SOC_RFE_TYPE
	int "RTL8197F (2.4 GHz) RFE type"
	default 0
	help
	  Front-end layout of the SoC radio. 0 = internal PA/LNA.
	  Can be overridden at runtime with the rfe_soc module parameter.

  config RTL8192CD_PCIE_RFE_TYPE
	int "RTL8812F (5 GHz) RFE type"
	default 0
	help
	  Front-end layout of the PCIe radio. 0 = internal PA/LNA.
	  Can be overridden at runtime with the rfe_pcie module parameter.

  endif
endef

STAMP_CONFIGURED_DEPENDS := \
	$(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

NOSTDINC_FLAGS := \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-include backport/backport.h

# Kconfig-style switches of the vendor tree. Each one is passed to make (object
# selection in src/Makefile) and to the compiler (the C code tests the macros).
RTL8192CD_CONFIGS := \
	CONFIG_RTL8192CD=m \
	CONFIG_RTL_8197F=y \
	CONFIG_RTL_8197F_GW=y \
	CONFIG_RTL_8197F_WRT=y \
	CONFIG_RTL_819X=y \
	CONFIG_SOC_RTL8197F=y \
	CONFIG_SOC_WIFI=y \
	CONFIG_PCI_HCI=y \
	CONFIG_WLAN_HAL=y \
	CONFIG_WLAN_HAL_88XX=y \
	CONFIG_WLAN_HAL_8197F=y \
	CONFIG_WLAN_HAL_8812FE=y \
	CONFIG_WLAN_MACHAL_API=y \
	CONFIG_WLAN_MACHAL_API_V0=y \
	CONFIG_RTL_ODM_WLAN_DRIVER=y \
	CONFIG_USE_PCIE_SLOT_0=y \
	CONFIG_SLOT_0_8812FE=y \
	CONFIG_RTL_5G_SLOT_0=y \
	CONFIG_BAND_5G_ON_WLAN0=y \
	CONFIG_OPENWRT_SDK=y \
	CONFIG_RTL_11W_SUPPORT=y \
	CONFIG_RTL_8197F_WRT_PCI=y

RTL8192CD_CFLAGS := \
	-DNOT_RTK_BSP -D_LITTLE_ENDIAN_ -DDM_ODM_SUPPORT_TYPE=0x01 \
	-DCONFIG_RTL8192CD_MODULE -DCONFIG_RTL_DISABLE_WLAN_MIPS16 \
	-DCONFIG_CFG80211_MODULE -DCONFIG_BACKPORTED_CFG80211_MODULE \
	-DRTL8192CD_SOC_RFE_TYPE=$(if $(CONFIG_RTL8192CD_SOC_RFE_TYPE),$(CONFIG_RTL8192CD_SOC_RFE_TYPE),0) \
	-DRTL8192CD_PCIE_RFE_TYPE=$(if $(CONFIG_RTL8192CD_PCIE_RFE_TYPE),$(CONFIG_RTL8192CD_PCIE_RFE_TYPE),0) \
	$(foreach c,$(RTL8192CD_CONFIGS),-D$(word 1,$(subst =, ,$(c)))) \
	-Wno-error -Wno-error=incompatible-pointer-types -Wno-error=int-conversion

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		RTL8192CD_BP_BACKPORT_DIR="$(STAGING_DIR)/usr/include/mac80211-backport" \
		RTL8192CD_BP_KERNEL_DIR="$(STAGING_DIR)/usr/include/mac80211" \
		RTL8192CD_PLATFORM_CFLAGS="$(RTL8192CD_CFLAGS)" \
		$(RTL8192CD_CONFIGS) \
		modules
endef

$(eval $(call KernelPackage,rtl8192cd))
