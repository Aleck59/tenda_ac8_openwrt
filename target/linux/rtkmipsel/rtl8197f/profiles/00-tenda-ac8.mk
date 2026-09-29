#
# Tenda AC8 v1: RTL8197FH-VG, RTL8812FR (5 GHz, PCIe), RTL8367RB-VB switch.
#
# The rtknet Ethernet driver and the RTL8367RB support are built into the
# kernel.  kmod-rtl8192cd is the vendor Wi-Fi driver for both radios; it is
# configured through wireless extensions (iwpriv from wireless-tools) by
# /lib/netifd/wireless/rtl8192cd.sh, WPA2-PSK is done by the driver itself.
#
define Profile/tenda_ac8
  NAME:=Tenda AC8 v1
  PRIORITY:=1
  PACKAGES:=kmod-rtl8192cd wireless-tools luci
endef

define Profile/tenda_ac8/Description
	Tenda AC8 v1 (RTL8197FH-VG + RTL8812FR + RTL8367RB-VB), 8 or 16 MiB flash
endef

$(eval $(call Profile,tenda_ac8))
