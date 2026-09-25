#!/usr/bin/env bash
# Write .config for the selected Tenda AC8 profiles and check that
# "make defconfig" kept them (it silently drops unknown symbols).
#
# Usage: [PACKAGE_FEED=1] scripts/configure.sh <openwrt-dir> "<devices>" ["<extra packages>"]
#   devices: any of tenda_ac8-v1 tenda_ac8-v1-8m tenda_ac8-v1-16m
#   PACKAGE_FEED=1: also build every kernel module package for the feed of
#   this build (scripts/make-feed.sh), ~17 more minutes.
set -euo pipefail

repo_root=$(cd "$(dirname "$0")/.." && pwd)
tree=${1:?openwrt dir}
devices=${2:-tenda_ac8-v1 tenda_ac8-v1-8m tenda_ac8-v1-16m}
extra=${3:-}

cd "$tree"
cp "$repo_root/configs/tenda_ac8.config" .config
for dev in $devices; do
	case "$dev" in
	tenda_ac8-v1|tenda_ac8-v1-8m|tenda_ac8-v1-16m) ;;
	*) echo "error: unknown device '$dev'" >&2; exit 1 ;;
	esac
	echo "CONFIG_TARGET_DEVICE_realtek_rtl8197f_DEVICE_$dev=y" >> .config
done
for pkg in $extra; do
	echo "CONFIG_PACKAGE_$pkg=y" >> .config
done

if [ "${PACKAGE_FEED:-}" = 1 ]; then
	echo "CONFIG_ALL_KMODS=y" >> .config
fi
make defconfig >/dev/null

# With PACKAGE_FEED, CONFIG_ALL_KMODS: the kernel version hash of OpenWrt
# depends on the selected module packages, so the package feed can only
# offer modules built together with the images.  Drop the drivers
# for hardware the AC8 does not have (configs/kmods-exclude.txt), and every
# Wi-Fi driver of the mac80211 and mt76 packages: they build in one go with
# cfg80211, which the AC8 driver needs, and one that does not compile for
# this target would take cfg80211 down with it.
if [ "${PACKAGE_FEED:-}" = 1 ]; then
	exclude=$(grep -vE '^[[:space:]]*(#|$)' "$repo_root/configs/kmods-exclude.txt" | paste -sd'|')
	wifi=$(awk '/^Source-Makefile: / { src = $2 }
		/^Package: / && (src == "package/kernel/mac80211/Makefile" || src == "package/kernel/mt76/Makefile") { print $2 }' \
		tmp/.packageinfo | grep -vx kmod-cfg80211 | paste -sd'|')
	exclude="$exclude|$wifi"
	grep -oE '^CONFIG_PACKAGE_kmod-[^=]+=m$' .config | sed 's/^CONFIG_PACKAGE_//; s/=m$//' |
		{ grep -Ex "$exclude" || true; } | sed 's/.*/# CONFIG_PACKAGE_& is not set/' >> .config
	make defconfig >/dev/null
	echo "kernel module packages: $(grep -c '^CONFIG_PACKAGE_kmod-.*=[ym]$' .config)"
	# Excluded modules that other packages still select.
	kept=$(grep -oE '^CONFIG_PACKAGE_kmod-[^=]+=[ym]$' .config | sed 's/^CONFIG_PACKAGE_//; s/=.$//' |
		{ grep -Ex "$exclude" || true; } | paste -sd' ')
	[ -z "$kept" ] || echo "selected by other packages: $kept"
fi

fail=0
for dev in $devices; do
	if ! grep -q "^CONFIG_TARGET_DEVICE_realtek_rtl8197f_DEVICE_$dev=y$" .config; then
		echo "error: device $dev was dropped by defconfig" >&2
		fail=1
	fi
done
for pkg in $extra; do
	if ! grep -Eq "^CONFIG_PACKAGE_$pkg=[ym]$" .config; then
		echo "error: package $pkg is unknown or has unmet dependencies" >&2
		fail=1
	fi
done
[ "$fail" = 0 ] || exit 1

grep -E '^CONFIG_TARGET_(BOARD|SUBTARGET|DEVICE_realtek)' .config
