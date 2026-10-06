#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-only
#
# Build the Tenda AC8 v1 firmware and collect the images in out/.
#
# Environment:
#   OPENWRT_DIR    build tree (default: build/openwrt)
#   OUT_DIR        output directory (default: out)
#   JOBS           parallel jobs (default: nproc + 1)
#   SKIP_PREPARE   1 = do not run scripts/prepare.sh first
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OPENWRT_DIR="${OPENWRT_DIR:-$ROOT/build/openwrt}"
OUT_DIR="${OUT_DIR:-$ROOT/out}"
JOBS="${JOBS:-$(($(nproc) + 1))}"
export OPENWRT_DIR

log() { printf '\033[1;34m==>\033[0m %s\n' "$*"; }

[ "${SKIP_PREPARE:-0}" = 1 ] || "$ROOT/scripts/prepare.sh"

log "Downloading sources"
make -C "$OPENWRT_DIR" -j"$JOBS" download

log "Building with $JOBS jobs"
if ! make -C "$OPENWRT_DIR" -j"$JOBS"; then
	log "Build failed, re-running single-threaded to show the error"
	make -C "$OPENWRT_DIR" -j1 V=s
	exit 1
fi

log "Collecting images"
bin="$OPENWRT_DIR/bin/targets/rtl819x/rtl8197f"
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"
cp "$bin"/*tenda_ac8-v1* "$OUT_DIR"/
for f in config.buildinfo feeds.buildinfo version.buildinfo; do
	[ -f "$bin/$f" ] && cp "$bin/$f" "$OUT_DIR/$f"
done
cp "$bin"/*.manifest "$OUT_DIR"/ 2>/dev/null || true

# full 8 MiB image for a flash programmer: bootloader + reference factory
# block (calibration only, no per-device data) + firmware
if [ -f "$ROOT/boot/bootloader.bin" ] && [ -f "$ROOT/boot/factory-reference.bin" ]; then
	log "Programmer image"
	for sys in "$OUT_DIR"/*-squashfs-sysupgrade.bin; do
		python3 "$ROOT/scripts/mkflash.py" \
			--bootloader "$ROOT/boot/bootloader.bin" \
			--factory "$ROOT/boot/factory-reference.bin" \
			--firmware "$sys" \
			-o "${sys%-squashfs-sysupgrade.bin}-full-8m.bin"
	done
fi

(cd "$OUT_DIR" && sha256sum -- * > sha256sums)
ls -l "$OUT_DIR"
