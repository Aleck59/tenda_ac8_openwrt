# SPDX-License-Identifier: GPL-2.0
#
# Realtek RTL8197F (RTL819x) switch-core CPU-port Ethernet driver, from the
# D-Link DIR-842 R1 OpenWrt port (github.com/ADCDS/openwrt-dlink-dir842-r1,
# Linux 4.14), ported to Linux 6.6 without its hardware NAT offload.
#

obj-$(CONFIG_NET_RTL819X) += rtl819x.o

rtl819x-y := rtl819x-eth.o rtl819x_swnic.o rtl865x_asichal.o
