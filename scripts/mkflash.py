#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Build a full 8 MiB SPI flash image for a Tenda AC8 v1.

The first 128 KiB of the router's own programmer dump (Realtek bootloader,
factory NVRAM with MAC addresses and Wi-Fi calibration) are kept as they are,
the OpenWrt sysupgrade image is placed at 0x20000 and the rest is erased
(0xff). The result is written to the chip with a programmer (CH341A etc.).

The dump may come from the original 2 MiB chip or from the 8 MiB chip.
"""

import argparse
import struct
import sys

from ac8_nvram import NVRAM_OFFSET, NVRAM_SIZE, parse_nvram

FLASH_SIZE = 8 * 1024 * 1024
FW_OFFSET = 0x20000
FW_MAX = FLASH_SIZE - FW_OFFSET
FWTOOL_MAGIC = b"FWx0"


def strip_fwtool(data: bytes) -> bytes:
    """Drop the sysupgrade metadata/signature trailers (OpenWrt fwtool)."""
    while len(data) > 16 and data[-16:-12] == FWTOOL_MAGIC:
        size = struct.unpack(">I", data[-4:])[0]
        if size < 16 or size > len(data):
            break
        data = data[:-size]
    return data


def check_cvimg(fw: bytes) -> None:
    sig, start, burn, length = struct.unpack(">4sIII", fw[:16])
    if sig != b"cs6c":
        raise ValueError("firmware does not start with a Realtek cs6c header")
    if burn != FW_OFFSET:
        raise ValueError("cs6c header burn address is 0x%x, expected 0x%x" % (burn, FW_OFFSET))
    payload = fw[16:16 + length]
    if len(payload) != length or length % 2:
        raise ValueError("truncated cs6c payload")
    csum = 0
    for (word,) in struct.iter_unpack(">H", payload):
        csum = (csum + word) & 0xFFFF
    if csum:
        raise ValueError("cs6c checksum mismatch")
    print("boot image     cs6c, load 0x%08x, %d bytes, checksum ok" % (start, length))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--dump", required=True, help="programmer dump of this router")
    ap.add_argument("--firmware", required=True, help="OpenWrt *-squashfs-sysupgrade.bin")
    ap.add_argument("-o", "--output", required=True, help="output 8 MiB image")
    args = ap.parse_args()

    with open(args.dump, "rb") as f:
        dump = f.read()
    with open(args.firmware, "rb") as f:
        fw = strip_fwtool(f.read())

    try:
        if len(dump) not in (2 * 1024 * 1024, 4 * 1024 * 1024, FLASH_SIZE):
            raise ValueError("unexpected dump size %d (want 2, 4 or 8 MiB)" % len(dump))
        head = dump[:FW_OFFSET]
        if head[:4] in (b"\xff\xff\xff\xff", b"\x00\x00\x00\x00"):
            raise ValueError("the dump has no bootloader at 0x0")
        nv = parse_nvram(head[NVRAM_OFFSET:NVRAM_OFFSET + NVRAM_SIZE])
        check_cvimg(fw)
        if len(fw) > FW_MAX:
            raise ValueError("firmware is %d bytes, only %d fit" % (len(fw), FW_MAX))
    except ValueError as e:
        print("error: %s" % e, file=sys.stderr)
        return 1

    print("board          %s, LAN MAC %s" % (nv.get("BOARD_NAME", "?"), nv.get("et0macaddr", "?")))
    image = head + fw + b"\xff" * (FW_MAX - len(fw))
    assert len(image) == FLASH_SIZE
    with open(args.output, "wb") as f:
        f.write(image)
    print("written        %s (%d bytes, firmware %d bytes at 0x%x)" %
          (args.output, len(image), len(fw), FW_OFFSET))
    return 0


if __name__ == "__main__":
    sys.exit(main())
