#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Wrap a payload into a Realtek RTL819x "cvimg" boot image.

Layout (all fields big-endian):

    0x00  signature   4 bytes, "cs6c" (bootable system image)
    0x04  start_addr  RAM address the bootloader copies the payload to and
                      jumps to
    0x08  burn_addr   flash offset the image lives at
    0x0c  length      payload length including the trailing checksum
    0x10  payload ... + 16-bit checksum

The bootloader requires the 16-bit sum of all big-endian halfwords of the
payload (checksum included) to be zero. "cr6c" images additionally carry a
rootfs check and are not produced here.
"""

import argparse
import struct
import sys


def checksum16(data: bytes) -> int:
    total = 0
    for (word,) in struct.iter_unpack(">H", data):
        total = (total + word) & 0xFFFF
    return total


def build(payload: bytes, signature: bytes, start: int, burn: int) -> bytes:
    if len(payload) % 2:
        payload += b"\x00"
    csum = (-checksum16(payload)) & 0xFFFF
    payload += struct.pack(">H", csum)
    assert checksum16(payload) == 0
    return struct.pack(">4sIII", signature, start, burn, len(payload)) + payload


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--signature", default="cs6c")
    ap.add_argument("--start-addr", type=lambda s: int(s, 0), required=True)
    ap.add_argument("--burn-addr", type=lambda s: int(s, 0), required=True)
    ap.add_argument("input")
    ap.add_argument("output")
    args = ap.parse_args()

    sig = args.signature.encode()
    if len(sig) != 4:
        ap.error("signature must be 4 characters")

    with open(args.input, "rb") as f:
        payload = f.read()
    image = build(payload, sig, args.start_addr, args.burn_addr)
    with open(args.output, "wb") as f:
        f.write(image)
    return 0


if __name__ == "__main__":
    sys.exit(main())
