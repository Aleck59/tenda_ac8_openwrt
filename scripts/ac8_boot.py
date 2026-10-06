#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Extract the shareable boot area of a Tenda AC8 v1 flash dump.

Writes two files used by the CI to build the full programmer image:

  bootloader.bin         0x00000-0x1bfff, the Realtek bootloader v3.4.13,
                         copied verbatim (it holds no per-device data)
  factory-reference.bin  0x1c000-0x1ffff, a factory NVRAM block with only
                         the board name and the Wi-Fi calibration (HW_*) of
                         the dumped unit. MAC addresses, serial numbers, the
                         WPS PIN and test stamps are dropped on purpose.

A unit flashed with the reference block gets usable Wi-Fi power tables;
OpenWrt generates the MAC addresses on first boot. To keep a router's own
factory data use mkflash.py --dump or write the image region by region.
"""

import argparse
import os
import sys

from ac8_nvram import NVRAM_OFFSET, NVRAM_SIZE, build_nvram, parse_nvram

BOOT_SIZE = NVRAM_OFFSET            # 0x1c000
FACTORY_SIZE = 0x20000 - NVRAM_OFFSET
NVRAM_AREA = 0x3000                 # FLSH block + zero padding, as on stock


def keep(key: str) -> bool:
    return key == "BOARD_NAME" or key.startswith("HW_")


def factory_reference(nv: dict) -> bytes:
    items = {k: nv[k] for k in sorted(nv) if keep(k)}
    if not any(k.startswith("HW_WLAN0_") for k in items) or \
       not any(k.startswith("HW_WLAN1_") for k in items):
        raise ValueError("the dump has no calibration for both radios")
    return build_nvram(items, NVRAM_AREA) + b"\xff" * (FACTORY_SIZE - NVRAM_AREA)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("dump", help="programmer dump (2 or 8 MiB)")
    ap.add_argument("-o", "--outdir", default="boot")
    args = ap.parse_args()

    with open(args.dump, "rb") as f:
        dump = f.read()
    try:
        if len(dump) < 0x20000:
            raise ValueError("dump is too small")
        boot = dump[:BOOT_SIZE]
        if boot[:4] in (b"\xff" * 4, b"\x00" * 4):
            raise ValueError("no bootloader at 0x0")
        nv = parse_nvram(dump[NVRAM_OFFSET:NVRAM_OFFSET + NVRAM_SIZE])
        factory = factory_reference(nv)
    except ValueError as e:
        print("error: %s" % e, file=sys.stderr)
        return 1

    os.makedirs(args.outdir, exist_ok=True)
    for name, data in (("bootloader.bin", boot), ("factory-reference.bin", factory)):
        with open(os.path.join(args.outdir, name), "wb") as f:
            f.write(data)
        print("%-22s %6d bytes" % (name, len(data)))
    dropped = sorted(k for k in nv if not keep(k))
    print("dropped from NVRAM: %s" % ", ".join(dropped))
    return 0


if __name__ == "__main__":
    sys.exit(main())
