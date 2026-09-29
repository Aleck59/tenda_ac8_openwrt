#!/usr/bin/env bash
# Write .config for the Tenda AC8 v1 and check that "make defconfig" kept
# the settings (it silently drops unknown symbols).
#
# Usage: scripts/configure.sh <openwrt-dir> ["<extra packages>"]
set -euo pipefail

repo_root=$(cd "$(dirname "$0")/.." && pwd)
tree=${1:?openwrt dir}
extra=${2:-}

cd "$tree"
cp "$repo_root/configs/tenda_ac8.config" .config
for pkg in $extra; do
	echo "CONFIG_PACKAGE_$pkg=y" >> .config
done
make defconfig >/dev/null

fail=0
while read -r line; do
	case "$line" in
	CONFIG_*=*)
		grep -qxF "$line" .config || { echo "error: '$line' was dropped by defconfig" >&2; fail=1; }
		;;
	"# CONFIG_"*" is not set")
		sym=${line#\# }; sym=${sym%% *}
		! grep -q "^$sym=" .config || { echo "error: $sym is still set" >&2; fail=1; }
		;;
	esac
done < "$repo_root/configs/tenda_ac8.config"
for pkg in $extra; do
	grep -Eq "^CONFIG_PACKAGE_$pkg=y$" .config ||
		{ echo "error: package $pkg is unknown or has unmet dependencies" >&2; fail=1; }
done
[ "$fail" = 0 ] || exit 1

grep -E '^CONFIG_TARGET_(BOARD|SUBTARGET|PROFILE)=' .config
