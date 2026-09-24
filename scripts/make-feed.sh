#!/usr/bin/env bash
# Collect the package feed of a build: the kernel modules and the other
# target packages (built against exactly this kernel), the LuCI theme and
# the patched wifi-scripts, in one flat directory with a Packages index
# signed by the build key the images trust.  Everything else comes from
# the official OpenWrt feeds, see scripts/distfeeds.sh.
#
# Usage: scripts/make-feed.sh <openwrt-dir> <output-dir>
set -euo pipefail

tree=$(cd "${1:?openwrt dir}" && pwd)
out=${2:?output dir}

cfg() { sed -n "s/^CONFIG_$1=\"\(.*\)\"$/\1/p" "$tree/.config"; }
board=$(cfg TARGET_BOARD)
subtarget=$(cfg TARGET_SUBTARGET)
arch=$(cfg TARGET_ARCH_PACKAGES)

mkdir -p "$out"
out=$(cd "$out" && pwd)
cp "$tree/bin/targets/$board/$subtarget/packages/"*.ipk "$out/"
cp "$tree/bin/packages/$arch/footstrap/"*.ipk "$out/"
cp "$tree/bin/packages/$arch/base/"wifi-scripts_*.ipk "$out/"

# The same steps as "make package/index".
export MKHASH="$tree/staging_dir/host/bin/mkhash"
cd "$out"
"$tree/scripts/ipkg-make-index.sh" . 2>/dev/null > Packages.manifest
grep -vE '^(Maintainer|LicenseFiles|Source|SourceName|Require|SourceDateEpoch)' \
	Packages.manifest > Packages
# usign SHA-512 bug, see package/Makefile
case "$(((64 + $(stat -L -c%s Packages)) % 128))" in
110|111) printf '\n\n' >> Packages ;;
esac
gzip -9nc Packages > Packages.gz
if grep -q '^CONFIG_SIGNED_PACKAGES=y$' "$tree/.config"; then
	"$tree/staging_dir/host/bin/usign" -S -m Packages -s "$tree/key-build"
fi
rm Packages.manifest

echo "$(ls -1 *.ipk | wc -l) packages in $out"
