#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Self-checks for the image and flash tools on synthetic data."""

import os
import struct
import subprocess
import sys
import tempfile
import unittest

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "scripts"))
sys.path.insert(0, os.path.join(ROOT, "openwrt/target/linux/rtl819x/image"))

import ac8_nvram  # noqa: E402
import cvimg  # noqa: E402
import mkflash  # noqa: E402


def make_nvram(items):
    body = b"".join(("%s=%s" % kv).encode() + b"\0" for kv in items) + b"\0"
    total = 20 + len(body)
    blob = b"FLSH" + struct.pack("<IIII", total, 0, 0, 0) + body
    return blob + b"\0" * (0x1000 - len(blob))


def make_dump(size=2 * 1024 * 1024):
    nv = make_nvram([("et0macaddr", "04:95:E6:29:74:B0"), ("BOARD_NAME", "AC8_V2.0"),
                     ("HW_WLAN0_11N_THER", "1e"), ("wps_device_pin", "12345670")])
    dump = bytearray(b"\xff" * size)
    dump[0:4] = b"\x0b\xf0\x00\x02"          # anything but erased flash
    dump[0x1C000:0x1D000] = nv
    return bytes(dump)


class Tools(unittest.TestCase):
    def test_cvimg_checksum(self):
        img = cvimg.build(b"\x01\x02\x03", b"cs6c", 0x80A00000, 0x20000)
        sig, start, burn, length = struct.unpack(">4sIII", img[:16])
        self.assertEqual((sig, start, burn, length), (b"cs6c", 0x80A00000, 0x20000, 6))
        self.assertEqual(cvimg.checksum16(img[16:]), 0)

    def test_nvram(self):
        nv = ac8_nvram.parse_nvram(make_dump()[0x1C000:0x1D000])
        self.assertEqual(nv["et0macaddr"], "04:95:E6:29:74:B0")
        self.assertEqual(nv["HW_WLAN0_11N_THER"], "1e")

    def test_mkflash(self):
        fw = cvimg.build(os.urandom(4096), b"cs6c", 0x80A00000, 0x20000)
        fw += b"\x00" * (0x10000 - len(fw)) + b"hsqs" + os.urandom(1000)
        meta = b'{"metadata":1}'
        trailer = b"FWx0" + b"\0" * 4 + b"\x01\0\0\0" + struct.pack(">I", len(meta) + 16)
        with tempfile.TemporaryDirectory() as d:
            paths = [os.path.join(d, n) for n in ("dump.bin", "fw.bin", "out.bin")]
            with open(paths[0], "wb") as f:
                f.write(make_dump())
            with open(paths[1], "wb") as f:
                f.write(fw + meta + trailer)
            subprocess.run([sys.executable, os.path.join(ROOT, "scripts/mkflash.py"),
                            "--dump", paths[0], "--firmware", paths[1], "-o", paths[2]],
                           check=True, stdout=subprocess.DEVNULL)
            with open(paths[2], "rb") as f:
                out = f.read()
        self.assertEqual(len(out), mkflash.FLASH_SIZE)
        self.assertEqual(out[:0x20000], make_dump()[:0x20000])
        self.assertEqual(out[0x20000:0x20000 + len(fw)], fw)
        self.assertEqual(set(out[0x20000 + len(fw):]), {0xFF})

    def test_mkflash_rejects_bad_image(self):
        bad = bytearray(cvimg.build(b"\x10" * 64, b"cs6c", 0x80A00000, 0x20000))
        bad[20] ^= 0xFF
        with self.assertRaises(ValueError):
            mkflash.check_cvimg(bytes(bad))



class BootArea(unittest.TestCase):
    """The committed boot files must stay shareable and well-formed."""

    def setUp(self):
        with open(os.path.join(ROOT, "boot/bootloader.bin"), "rb") as f:
            self.boot = f.read()
        with open(os.path.join(ROOT, "boot/factory-reference.bin"), "rb") as f:
            self.factory = f.read()

    def test_sizes(self):
        self.assertEqual(len(self.boot), 0x1C000)
        self.assertEqual(len(self.factory), 0x4000)

    def test_no_device_data(self):
        nv = ac8_nvram.parse_nvram(self.factory[:0x1000])
        self.assertTrue(all(k == "BOARD_NAME" or k.startswith("HW_") for k in nv), nv.keys())
        self.assertTrue(any(k.startswith("HW_WLAN0_") for k in nv))
        self.assertTrue(any(k.startswith("HW_WLAN1_") for k in nv))
        for blob in (self.boot, self.factory):
            self.assertNotIn(b"macaddr=", blob)
            self.assertNotIn(b"hwaddr=", blob)
            self.assertNotIn(b"wps_device_pin", blob)

    def test_full_image(self):
        fw = cvimg.build(b"\x01" * 512, b"cs6c", 0x80A00000, 0x20000)
        with tempfile.TemporaryDirectory() as d:
            src, out = os.path.join(d, "fw.bin"), os.path.join(d, "full.bin")
            with open(src, "wb") as f:
                f.write(fw)
            subprocess.run([sys.executable, os.path.join(ROOT, "scripts/mkflash.py"),
                            "--bootloader", os.path.join(ROOT, "boot/bootloader.bin"),
                            "--factory", os.path.join(ROOT, "boot/factory-reference.bin"),
                            "--firmware", src, "-o", out],
                           check=True, stdout=subprocess.DEVNULL)
            with open(out, "rb") as f:
                img = f.read()
        self.assertEqual(img[:0x1C000], self.boot)
        self.assertEqual(img[0x1C000:0x20000], self.factory)
        self.assertEqual(img[0x20000:0x20000 + len(fw)], fw)


if __name__ == "__main__":
    unittest.main()
