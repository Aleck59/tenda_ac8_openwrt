#
# OpenWrt 19.07 / Linux 4.14 target for the Realtek RTL8197F, Tenda AC8 v1.
#
# The target (arch code, rtknet Ethernet, rtl8192cd Wi-Fi, SPI flash) is the
# one of the Alcatel LINKHUB HH71VM port (github.com/sowarden/hh71vm-openwrt),
# itself based on the OpenWrt 18.06 fork for the Tenda AC10U
# (github.com/vladisslav2011/openwrt-AC10); scripts/prepare.sh fetches both.
#
# This is free software, licensed under the GNU General Public License v2.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=rtkmipsel
BOARDNAME:=Realtek mipsel SOC
SUBTARGETS:=rtl8197f
FEATURES:=squashfs
KERNEL_PATCHVER:=4.14

include $(INCLUDE_DIR)/target.mk

define Target/Description
	Build firmware images for Realtek RTL8197F based boards
endef

$(eval $(call BuildTarget))
