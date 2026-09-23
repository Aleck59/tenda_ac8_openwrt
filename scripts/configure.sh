#!/usr/bin/env bash
# Write .config for the selected Tenda AC8 profiles and check that
# "make defconfig" kept them (it silently drops unknown symbols).
#
# Usage: scripts/configure.sh <openwrt-dir> "<devices>" ["<extra packages>"]
#   devices: any of tenda_ac8-v1 tenda_ac8-v1-8m tenda_ac8-v1-16m
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

make defconfig >/dev/null

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
