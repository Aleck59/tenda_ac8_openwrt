#!/usr/bin/env bash
# Point the opkg feeds of the images (/etc/opkg/distfeeds.conf) at
# - the feed of this build (kernel modules and AC8 packages, published by
#   .github/workflows/packages.yml, see scripts/make-feed.sh), when it has
#   one;
# - the official OpenWrt release whose packages have the same library ABI
#   as the pinned tree (configs/packages.env) for everything else.
# The OpenWrt defaults would name targets/realtek/rtl8197f, which the
# official servers do not build.
#
# Usage: scripts/distfeeds.sh <openwrt-dir> [<feed-url>]
set -euo pipefail

repo_root=$(cd "$(dirname "$0")/.." && pwd)
# shellcheck source=../configs/packages.env
. "$repo_root/configs/packages.env"
tree=${1:?openwrt dir}
url=${2:-}

arch=$(sed -n 's/^CONFIG_TARGET_ARCH_PACKAGES="\(.*\)"$/\1/p' "$tree/.config")
[ -n "$arch" ] || { echo "error: no package architecture in $tree/.config" >&2; exit 1; }

mkdir -p "$tree/files/etc/opkg"
{
	[ -z "$url" ] || echo "src/gz tenda_ac8 $url"
	for feed in base luci packages routing telephony; do
		echo "src/gz openwrt_$feed https://downloads.openwrt.org/releases/$PACKAGES_RELEASE/packages/$arch/$feed"
	done
} > "$tree/files/etc/opkg/distfeeds.conf"
cat "$tree/files/etc/opkg/distfeeds.conf"
