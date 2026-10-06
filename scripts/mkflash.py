#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Build a full 8 MiB SPI flash image for a Tenda AC8 v1.

The boot area (0x00000-0x1ffff: Realtek bootloader, factory NVRAM with MAC
addresses and Wi-Fi calibration) comes either from the router's own
programmer dump (--dump, recommended: keeps that unit's MACs and
calibration) or from separate files (--bootloader/--factory, used by the CI
with boot/bootloader.bin and boot/factory-reference.bin). The OpenWrt image
goes to 0x20000 and the rest is erased (0xff).

--firmware takes the *-squashfs-sysupgrade.bin or a full 8 MiB image (for
example the *-full-8m.bin of a release, to put your own dump's boot area in).
"""

import argparse
import struct
import sys

from ac8_nvram import NVRAM_OFFSET, NVRAM_SIZE, parse_nvram

FLASH_SIZE = 8 * 1024 * 1024
FW_OFFSET = 0x20000
FW_MAX = FLASH_SIZE - FW_OFFSET
FACTORY_SIZE = FW_OFFSET - NVRAM_OFFSET
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


def load_firmware(path: str) -> bytes:
    with open(path, "rb") as f:
        data = f.read()
    if len(data) == FLASH_SIZE and data[FW_OFFSET:FW_OFFSET + 4] == b"cs6c":
        data = data[FW_OFFSET:].rstrip(b"\xff")   # full image: keep the firmware part
    return strip_fwtool(data)


def boot_area(args) -> bytes:
    if args.dump:
        with open(args.dump, "rb") as f:
            dump = f.read()
        if len(dump) not in (2 * 1024 * 1024, 4 * 1024 * 1024, FLASH_SIZE):
            raise ValueError("unexpected dump size %d (want 2, 4 or 8 MiB)" % len(dump))
        head = dump[:FW_OFFSET]
    else:
        with open(args.bootloader, "rb") as f:
            boot = f.read()
        with open(args.factory, "rb") as f:
            factory = f.read()
        if len(boot) > NVRAM_OFFSET:
            raise ValueError("bootloader is larger than 0x%x" % NVRAM_OFFSET)
        if len(factory) > FACTORY_SIZE:
            raise ValueError("factory area is larger than 0x%x" % FACTORY_SIZE)
        head = boot.ljust(NVRAM_OFFSET, b"\xff") + factory.ljust(FACTORY_SIZE, b"\xff")
    if head[:4] in (b"\xff\xff\xff\xff", b"\x00\x00\x00\x00"):
        raise ValueError("no bootloader at 0x0")
    return head


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    src = ap.add_mutually_exclusive_group(required=True)
    src.add_argument("--dump", help="programmer dump of this router (recommended)")
    src.add_argument("--bootloader", help="bootloader image for 0x0 (boot/bootloader.bin)")
    ap.add_argument("--factory", help="factory area for 0x1c000 (with --bootloader)")
    ap.add_argument("--firmware", required=True,
                    help="OpenWrt *-squashfs-sysupgrade.bin or a full 8 MiB image")
    ap.add_argument("-o", "--output", required=True, help="output 8 MiB image")
    args = ap.parse_args()
    if args.bootloader and not args.factory:
        ap.error("--bootloader needs --factory")

    try:
        head = boot_area(args)
        nv = parse_nvram(head[NVRAM_OFFSET:NVRAM_OFFSET + NVRAM_SIZE])
        fw = load_firmware(args.firmware)
        check_cvimg(fw)
        if len(fw) > FW_MAX:
            raise ValueError("firmware is %d bytes, only %d fit" % (len(fw), FW_MAX))
    except (OSError, ValueError) as e:
        print("error: %s" % e, file=sys.stderr)
        return 1

    print("board          %s, LAN MAC %s" % (nv.get("BOARD_NAME", "?"),
                                            nv.get("et0macaddr", "none (OpenWrt generates one)")))
    image = head + fw + b"\xff" * (FW_MAX - len(fw))
    assert len(image) == FLASH_SIZE
    with open(args.output, "wb") as f:
        f.write(image)
    print("written        %s (%d bytes, firmware %d bytes at 0x%x)" %
          (args.output, len(image), len(fw), FW_OFFSET))
    return 0


if __name__ == "__main__":
    sys.exit(main())
