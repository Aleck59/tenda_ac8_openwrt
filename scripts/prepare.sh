#!/usr/bin/env bash
# Assemble the OpenWrt 19.07 tree for the Tenda AC8 v1:
#   - OpenWrt 19.07 and the packages/luci feeds (base.env);
#   - the rtkmipsel target of the HH71VM port, without its board files;
#   - the Realtek SDK RTL8367RB driver (rtl8367r/) of the AC10U fork;
#   - the AC8 files of this repository (target/) and patches/.
#
# Usage: scripts/prepare.sh [output-dir]      (default: ./openwrt)
set -euo pipefail

repo_root=$(cd "$(dirname "$0")/.." && pwd)
# shellcheck source=../base.env
. "$repo_root/base.env"
out=${1:-$repo_root/openwrt}

if [ -e "$out" ]; then
	echo "error: $out already exists" >&2
	exit 1
fi

# fetch <dir> <url> <commit> [sparse path...]
fetch() {
	local dir=$1 url=$2 rev=$3
	shift 3
	mkdir -p "$dir"
	git -C "$dir" init -q
	git -C "$dir" remote add origin "$url"
	git -C "$dir" config gc.auto 0
	if [ $# -gt 0 ]; then
		git -C "$dir" sparse-checkout set --no-cone "$@"
		git -C "$dir" fetch -q --depth 1 --filter=blob:none origin "$rev"
	else
		git -C "$dir" fetch -q --depth 1 origin "$rev"
	fi
	git -C "$dir" checkout -q FETCH_HEAD
	echo "    $url @ $(git -C "$dir" log -1 --format='%h %cs %s')"
}

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

echo "==> Fetching sources"
fetch "$out" "$OPENWRT_REPO" "$OPENWRT_COMMIT"
fetch "$out/feeds/packages" "$PACKAGES_REPO" "$PACKAGES_COMMIT"
fetch "$out/feeds/luci" "$LUCI_REPO" "$LUCI_COMMIT"
fetch "$tmp/hh71vm" "$HH71VM_REPO" "$HH71VM_COMMIT" \
	/openwrt-feed/target/linux/rtkmipsel/ \
	/openwrt-feed/package/network/utils/iwinfo/ \
	/openwrt-feed/package/system/fstools/ \
	/openwrt-feed/patches/luci/ \
	/LICENSE /LICENSING.md
fetch "$tmp/ac10" "$AC10_REPO" "$AC10_COMMIT" \
	/target/linux/rtkmipsel/files/drivers/net/rtl819x/rtl8367r/

# The revision in the banner and LuCI: scripts/getver.sh cannot count
# commits in a shallow clone.
echo "tenda-ac8-${OPENWRT_COMMIT:0:10}" > "$out/version"

echo "==> Assembling target/linux/rtkmipsel"
target=$out/target/linux/rtkmipsel
cp -a "$tmp/hh71vm/openwrt-feed/target/linux/rtkmipsel" "$target"
# The HH71VM board: its base-files (modem, USB WAN, LEDs), profile,
# machine and prebuilt iwpriv are not used on the AC8.
rm -rf "$target/base-files" "$target/base-files.mk" "$target/rtl8197f/profiles" \
	"$target/files/arch/mips/rtl8197f/mach-hh71vm.c"
cp -a "$tmp/ac10/target/linux/rtkmipsel/files/drivers/net/rtl819x/rtl8367r" \
	"$target/files/drivers/net/rtl819x/"
# Do not depend on the executable bit of bin2c.pl.
sed -i -E 's/^(\s*)\$\(obj\)\/bin2c.pl/\1perl $(obj)\/bin2c.pl/' \
	"$target/files/drivers/net/wireless/realtek/rtl8192cd/Makefile"

# iwinfo: wireless-extensions support for rtl8192cd (LuCI status pages);
# fstools: no error for an empty overlay on the first boot.
for pkg in network/utils/iwinfo system/fstools; do
	mkdir -p "$out/package/$pkg/patches"
	cp "$tmp/hh71vm/openwrt-feed/package/$pkg/patches/"*.patch "$out/package/$pkg/patches/"
done
# LuCI: rtl8192cd encryption capabilities and band detection.
for p in 100-rtl8192cd-encryption-capabilities.patch 101-wifi-band-fallback-rtl8192cd.patch; do
	patch -d "$out/feeds/luci" -p1 --forward --no-backup-if-mismatch -s \
		< "$tmp/hh71vm/openwrt-feed/patches/luci/$p"
done

echo "==> Adding Tenda AC8 support"
cp -a "$repo_root/target/." "$out/target/"
for p in "$repo_root"/patches/*.patch; do
	echo "    $(basename "$p")"
	patch -d "$out" -p1 --forward --no-backup-if-mismatch -s < "$p"
done

echo "==> Installing feeds"
cat > "$out/feeds.conf" <<EOT
src-git packages $PACKAGES_REPO^$PACKAGES_COMMIT
src-git luci $LUCI_REPO^$LUCI_COMMIT
EOT
cd "$out"
./scripts/feeds update -i > feeds.log 2>&1 || { cat feeds.log; exit 1; }
./scripts/feeds install -a >> feeds.log 2>&1 || { cat feeds.log; exit 1; }

echo "==> Source tree ready in $out"
