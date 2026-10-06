# Tenda AC8 v1 OpenWrt port

OpenWrt overlay for the Tenda AC8 v1 (RTL8197FH + RTL8812FR + RTL8367RB-VB,
64 MiB RAM) with an 8 MiB SPI flash. Not a full OpenWrt tree:
`scripts/prepare.sh` checks out OpenWrt at `configs/openwrt-base.txt`, copies
`openwrt/` over it, applies `patches/openwrt/*.patch`, wires the pinned feeds
from `configs/feeds.conf` and applies `configs/tenda_ac8.config`.

## Rules

- User-facing docs (README.md, docs/) are in Russian; code, comments and
  commit messages in English.
- Everything is pinned (OpenWrt commit, feed commits). Bump pins deliberately,
  in a separate commit, after a full build.
- Never write to or erase flash below 0x20000 (bootloader, factory NVRAM at
  0x1c000): it holds per-device MACs and Wi-Fi calibration. Tools and images
  must keep it read-only.
- Hardware facts come from the stock bootloader/eCos; docs/hardware.md is the
  reference. Keep it in sync with the DTS, board.d and driver when changing
  any of them.
- Keep the vendor driver diff minimal and guarded by
  `CONFIG_RTL_8197F_WRT` / `CONFIG_RTL_8197F_WRT_PCI`.
- Image space is ~7.9 MiB; check `make` output for the size check before
  adding packages.

## Layout

- `openwrt/target/linux/rtl819x/` platform: kernel patches (6.18), eth/PCIe
  drivers, `dts/rtl8197f-tenda-ac8.dts`, `image/` (cvimg header, two-stage
  lzma-loader), `base-files/` (network, Wi-Fi defaults, sysupgrade).
- `openwrt/package/kernel/rtl8192cd/` Realtek Wi-Fi driver (8197F + 8812F).
- `scripts/` prepare/build, `mkflash.py` (programmer image), `ac8_nvram.py`.
- `.github/workflows/build.yml` CI: checks + build on push/PR, release on `v*`.

Skills: `ac8-build` (build and iterate), `ac8-hardware` (hardware facts and
where they live in code), `ac8-release` (cutting a release).
