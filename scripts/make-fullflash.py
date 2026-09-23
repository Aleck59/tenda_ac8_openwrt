#!/usr/bin/env python3
"""Build a complete SPI-NOR image for a Tenda AC8 v1 from a stock backup.

The result is meant to be written with an external programmer (CH341A +
flashrom) to the original 4 MiB chip or to a new 8/16 MiB chip:

    0x000000..0x020000  boot loader + board data, copied from the backup
    0x020000..          OpenWrt *-squashfs-flash.bin (cs6c image)
    last 128 KiB        copied from the backup when the chip size is
                        unchanged (Tenda config), otherwise left erased

Example:
    make-fullflash.py --backup ac8-stock.bin \\
        --firmware openwrt-realtek-rtl8197f-tenda_ac8-v1-8m-squashfs-flash.bin \\
        --size 8M --output ac8-openwrt-8m-full.bin
"""

import argparse
import hashlib
import struct
import sys

BOOT_SIZE = 0x20000
CONFIG_SIZE = 0x20000
SIZES = {"4M": 4 << 20, "8M": 8 << 20, "16M": 16 << 20}
SIGNATURES = (b"cs6c", b"cr6c")


def checksum16_ok(payload: bytes) -> bool:
    total = 0
    for i in range(0, len(payload) & ~1, 2):
        total = (total + ((payload[i] << 8) | payload[i + 1])) & 0xFFFF
    if len(payload) & 1:
        total = (total + (payload[-1] << 8)) & 0xFFFF
    return total == 0


def die(msg: str) -> None:
    print(f"error: {msg}", file=sys.stderr)
    sys.exit(1)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--backup", required=True, help="full dump of the stock flash chip")
    ap.add_argument("--firmware", required=True, help="OpenWrt *-squashfs-flash.bin")
    ap.add_argument("--size", required=True, choices=sorted(SIZES), help="size of the target chip")
    ap.add_argument("--output", required=True)
    ap.add_argument("--force", action="store_true",
                    help="continue although the backup does not look like the expected layout")
    args = ap.parse_args()

    backup = open(args.backup, "rb").read()
    firmware = open(args.firmware, "rb").read()
    size = SIZES[args.size]

    if len(backup) not in SIZES.values():
        die(f"backup is {len(backup)} bytes, expected a full 4/8/16 MiB dump")
    stock_sig = backup[BOOT_SIZE:BOOT_SIZE + 4]
    if stock_sig not in SIGNATURES:
        msg = (f"backup has {stock_sig!r} at 0x{BOOT_SIZE:x}, not a Realtek cs6c/cr6c "
               "firmware header: the flash layout differs from the one this port assumes")
        if not args.force:
            die(msg + " (use --force only if you know the layout)")
        print("warning: " + msg, file=sys.stderr)
    if backup[:BOOT_SIZE] == b"\xff" * BOOT_SIZE:
        die("boot area of the backup is empty")

    sig, start, burn, length = struct.unpack(">4sIII", firmware[:16])
    if sig not in SIGNATURES:
        die(f"firmware has no Realtek header (found {sig!r})")
    if burn != BOOT_SIZE:
        die(f"firmware burn address is 0x{burn:x}, expected 0x{BOOT_SIZE:x}")
    if 16 + length > len(firmware) or not checksum16_ok(firmware[16:16 + length]):
        die("firmware header checksum is wrong")
    if firmware[16 + length:16 + length + 4] != b"hsqs":
        die("no SquashFS right after the firmware payload")

    limit = size - BOOT_SIZE - CONFIG_SIZE
    if len(firmware) > limit:
        die(f"firmware is {len(firmware)} bytes, only {limit} fit on a {args.size} chip")

    image = bytearray(b"\xff" * size)
    image[:BOOT_SIZE] = backup[:BOOT_SIZE]
    image[BOOT_SIZE:BOOT_SIZE + len(firmware)] = firmware
    if len(backup) == size:
        image[size - CONFIG_SIZE:] = backup[size - CONFIG_SIZE:]

    with open(args.output, "wb") as f:
        f.write(image)

    print(f"stock header : {stock_sig.decode(errors='replace')} at 0x{BOOT_SIZE:x}")
    print(f"firmware     : {sig.decode()} start=0x{start:08x} burn=0x{burn:x} "
          f"payload={length} bytes, total {len(firmware)} bytes")
    print(f"free space   : {limit - len(firmware)} bytes")
    print(f"output       : {args.output} ({args.size}), "
          f"sha256 {hashlib.sha256(image).hexdigest()}")


if __name__ == "__main__":
    main()
