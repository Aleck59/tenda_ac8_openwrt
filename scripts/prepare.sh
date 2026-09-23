#!/usr/bin/env bash
# Fetch the RTL8197F OpenWrt tree pinned in base.env, add the Tenda AC8
# support from overlay/ and patches/, and install the package feeds.
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

echo "==> Fetching $BASE_REPO @ $BASE_COMMIT ($BASE_SUBDIR)"
mkdir -p "$(dirname "$out")"
tmp=$(mktemp -d "$(dirname "$out")/.openwrt-base.XXXXXX")
trap 'rm -rf "$tmp"' EXIT
git -C "$tmp" init -q
git -C "$tmp" remote add origin "$BASE_REPO"
git -C "$tmp" config gc.auto 0
# The upstream repository also carries several GB of vendor SDK dumps;
# fetch only the OpenWrt tree.
git -C "$tmp" sparse-checkout set --no-cone "/$BASE_SUBDIR/"
git -C "$tmp" fetch -q --depth 1 --filter=blob:none origin "$BASE_COMMIT"
git -C "$tmp" checkout -q FETCH_HEAD
mv "$tmp/$BASE_SUBDIR" "$out"

# Full SPI dumps of other people's routers, only used by their private
# "fullflash" images.  Never needed here.
rm -rf "$out/fullflash-templates"

echo "==> Adding Tenda AC8 support"
cp -a "$repo_root/overlay/." "$out/"
for p in "$repo_root"/patches/*.patch; do
	echo "    $(basename "$p")"
	patch -d "$out" -p1 --forward --no-backup-if-mismatch -s < "$p"
done

echo "==> Installing feeds"
cat > "$out/feeds.conf" <<'EOF'
src-git packages https://github.com/openwrt/packages.git;openwrt-24.10
src-git luci https://github.com/openwrt/luci.git;openwrt-24.10
src-git routing https://github.com/openwrt/routing.git;openwrt-24.10
src-git telephony https://github.com/openwrt/telephony.git;openwrt-24.10
EOF
cd "$out"
./scripts/feeds update -a
./scripts/feeds install -a >/dev/null

echo "==> Source tree ready in $out"
