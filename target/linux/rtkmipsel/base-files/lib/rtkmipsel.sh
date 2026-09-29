#!/bin/sh
#
# Board detection from the "machine" line of /proc/cpuinfo, which the kernel
# takes from MIPS_MACHINE() (arch/mips/rtl8197f/mach-tenda-ac8.c), and from
# the size of the "firmware" partition: the 8 and 16 MiB flash variants take
# different images.
#
# /tmp/sysinfo/board_name is what sysupgrade compares with the
# supported_devices of the image metadata (SUPPORTED_DEVICES in
# image/Makefile): change both together.  The names are the ones of the
# OpenWrt 24.10 builds of this port, so their sysupgrade takes these images.
#

rtkmipsel_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*"Tenda AC8"*)
		case "$(awk -F'[: ]+' '$4 == "\"firmware\"" { print $2 }' /proc/mtd)" in
		007c0000) name="tenda,ac8-v1-8m" ;;
		00fc0000) name="tenda,ac8-v1-16m" ;;
		*) name="tenda,ac8-v1" ;;
		esac
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$name" > /tmp/sysinfo/board_name
	echo "$machine" > /tmp/sysinfo/model
}
