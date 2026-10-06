---
name: ac8-hardware
description: Tenda AC8 v1 hardware reference (flash map, factory NVRAM and Wi-Fi calibration keys, GPIOs, switch ports, bootloader image format) and where each fact is implemented. Use before changing the DTS, board.d scripts, calibration loader, image recipe or install docs.
---

# Hardware facts and their implementation

Source of truth: `docs/hardware.md` (derived from the stock bootloader v3.4.13
and eCos V02.03.01.78). Keep code and doc in sync.

| Fact | Implemented in |
|---|---|
| Flash map (bootloader 0x0, factory NVRAM 0x1c000, firmware 0x20000) | `dts/rtl8197f-tenda-ac8.dts` partitions, `scripts/mkflash.py` |
| Shareable boot area for the programmer image | `boot/` (from `scripts/ac8_boot.py`), `scripts/build.sh` |
| No MACs in the factory block | random MAC fallback in `02_network` and the driver loader |
| Boot image: `cs6c` header, BE fields, 16-bit BE sum == 0, load addr | `image/cvimg.py`, `image/Makefile` (`LOADER_ENTRY`), `files/drivers/mtd/mtdsplit/mtdsplit_cvimg.c`, `base-files/lib/upgrade/platform.sh` |
| TFTP `nfjrom` RAM boot at 0x80a00000 | `KERNEL_INITRAMFS` + two-stage loader (`IMAGE_COPY=1`, stage 2 at 0x82000000) |
| LED GPIO 35 (E3, active low), button GPIO 36 (E4, active low) | DTS `gpio_efgh` lines 3 and 4 |
| RTL8367 SMI: MDC GPIO 56 (H0), MDIO GPIO 55 (G7) | DTS switch node, `realtek,smi-pins` on eth0 (eth driver HW-NAT path) |
| Ports: lan1=3, lan2=1, lan3=0, wan=4, CPU=6 RGMII rx 600 ps / tx 0 | DTS `ethernet-ports`, `base-files/etc/board.d/02_network` |
| P0GMIICR |= bit 1 for chip id 0x81970000 | `rtl8197f_eth.c` `rtl_apply_a()` |
| NVRAM: "FLSH", le32 total length incl. 20-byte header, key=value\0 | `rtl8192cd/src/8192cd_cfg80211.c` `realtek_load_flash_calibration()`, `scripts/ac8_nvram.py` |
| HW_WLAN0_* = 5 GHz RTL8812F, HW_WLAN1_* = 2.4 GHz RTL8197F | same loader (chosen by chip version) |
| MACs: et0macaddr LAN=WAN, wl0_hwaddr 2.4 GHz, wl1_hwaddr 5 GHz | `02_network` (`mtd_get_mac_ascii factory`), driver loader |
| Default Wi-Fi key = `wps_device_pin` | `base-files/etc/board.d/03_wireless` |
| RFE type 0 for both radios | driver module params `rfe_soc`, `rfe_pcie` |
| wlan0 = RTL8812F (PCI, RTKWiFi0), wlan1 = RTL8197F (embedded, RTKWiFi1) | `8192cd_osdep.c` wlan_device table and init order, `patches/openwrt/0001-*` |

## Re-deriving facts from the stock firmware

The stock image is a cvimg (`cs6c`, start 0x80700000) with an LZMA eCos
payload. Decompress it, load at 0x80000000 and disassemble MIPS32 LE with
capstone; strings such as `HW_WLAN0_` or `et0macaddr` lead to the code that
consumes them. The bootloader is gzip-compressed at offset 0x8d60 of the dump
and runs at 0x80000000.
