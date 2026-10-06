#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
#
# Prepare an OpenWrt build tree for the Tenda AC8 v1:
#   OpenWrt at the pinned commit (configs/openwrt-base.txt)
#   + this repository's overlay (openwrt/) and patches (patches/openwrt/)
#   + pinned feeds (configs/feeds.conf, fetched shallow, used as src-link)
#   + the device configuration (configs/tenda_ac8.config).
#
# Environment:
#   OPENWRT_DIR     build tree (default: build/openwrt)
#   FEEDS_SRC_DIR   feed checkouts (default: build/feeds)
#   CONFIG_EXTRA    optional file with extra .config lines
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OPENWRT_DIR="${OPENWRT_DIR:-$ROOT/build/openwrt}"
FEEDS_SRC_DIR="${FEEDS_SRC_DIR:-$ROOT/build/feeds}"

log() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }

# shallow checkout of one commit: <dir> <url> <commit>
git_checkout() {
	local dir="$1" url="$2" rev="$3"

	if [ ! -d "$dir/.git" ]; then
		mkdir -p "$dir"
		git -C "$dir" init -q
		git -C "$dir" remote add origin "$url"
	fi
	git -C "$dir" remote set-url origin "$url"
	if ! git -C "$dir" cat-file -e "$rev^{commit}" 2>/dev/null; then
		git -C "$dir" fetch -q --depth 1 origin "$rev"
	fi
	git -C "$dir" checkout -q --detach -f "$rev"
}

repository=""
commit=""
# shellcheck disable=SC1091
. "$ROOT/configs/openwrt-base.txt"
[ -n "$repository" ] && [ -n "$commit" ] || {
	echo "configs/openwrt-base.txt must set repository= and commit=" >&2
	exit 1
}

log "OpenWrt $commit"
git_checkout "$OPENWRT_DIR" "$repository" "$commit"
# untracked leftovers of an older overlay; ignored dirs (dl, build_dir,
# staging_dir, feeds, .config) are kept
git -C "$OPENWRT_DIR" clean -fdq

log "Overlay"
cp -a "$ROOT/openwrt/." "$OPENWRT_DIR/"

log "Patches"
for p in "$ROOT"/patches/openwrt/*.patch; do
	[ -e "$p" ] || continue
	echo "  $(basename "$p")"
	git -C "$OPENWRT_DIR" apply --whitespace=nowarn "$p"
done

log "Feeds"
: > "$OPENWRT_DIR/feeds.conf"
while read -r type name spec; do
	case "$type" in
	src-git)
		url="${spec%^*}"
		rev="${spec##*^}"
		[ "$url" != "$rev" ] || { echo "feed $name is not pinned to a commit" >&2; exit 1; }
		echo "  $name $rev"
		git_checkout "$FEEDS_SRC_DIR/$name" "$url" "$rev"
		echo "src-link $name $FEEDS_SRC_DIR/$name" >> "$OPENWRT_DIR/feeds.conf"
		;;
	""|\#*)
		;;
	*)
		echo "$type $name $spec" >> "$OPENWRT_DIR/feeds.conf"
		;;
	esac
done < "$ROOT/configs/feeds.conf"
feeds_log="$OPENWRT_DIR/tmp/prepare-feeds.log"
mkdir -p "$OPENWRT_DIR/tmp"
(
	cd "$OPENWRT_DIR"
	./scripts/feeds update -a
	./scripts/feeds install -a
) > "$feeds_log" 2>&1 || { tail -n 50 "$feeds_log" >&2; exit 1; }

log "Configuration"
cp "$ROOT/configs/tenda_ac8.config" "$OPENWRT_DIR/.config"
if [ -n "${CONFIG_EXTRA:-}" ]; then
	cat "$CONFIG_EXTRA" >> "$OPENWRT_DIR/.config"
fi
make -C "$OPENWRT_DIR" defconfig > "$OPENWRT_DIR/tmp/prepare-defconfig.log" 2>&1 || {
	tail -n 50 "$OPENWRT_DIR/tmp/prepare-defconfig.log" >&2
	exit 1
}

missing=0
while IFS= read -r line; do
	case "$line" in
	CONFIG_*=*)
		grep -qxF "$line" "$OPENWRT_DIR/.config" || {
			echo "  dropped by defconfig: $line" >&2
			missing=1
		}
		;;
	esac
done < "$ROOT/configs/tenda_ac8.config"
[ "$missing" = 0 ] || { echo "configuration check failed" >&2; exit 1; }

log "Ready: $OPENWRT_DIR"
