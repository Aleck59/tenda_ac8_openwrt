#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-only
"""Show the factory NVRAM of a Tenda AC8 v1 flash dump.

The block at flash offset 0x1c000 is a Broadcom-style NVRAM image: "FLSH",
le32 total length, three more le32 header words, then NUL separated
key=value strings. It holds the MAC addresses and the RF calibration of both
radios (HW_WLAN0_* = 5 GHz RTL8812F, HW_WLAN1_* = 2.4 GHz RTL8197F), which
OpenWrt reads at run time. Never erase or overwrite it.
"""

import argparse
import json
import struct
import sys

NVRAM_OFFSET = 0x1C000
NVRAM_SIZE = 0x1000
HDR_LEN = 20


def parse_nvram(blob: bytes) -> dict:
    if blob[:4] != b"FLSH":
        raise ValueError("no FLSH magic at 0x%x" % NVRAM_OFFSET)
    total = struct.unpack_from("<I", blob, 4)[0]
    if not HDR_LEN < total <= len(blob):
        raise ValueError("bad NVRAM length 0x%x" % total)
    items = {}
    for raw in blob[HDR_LEN:total].split(b"\0"):
        if not raw:
            break
        key, sep, value = raw.decode("latin-1").partition("=")
        if sep:
            items[key] = value
    return items


def build_nvram(items: dict, size: int = NVRAM_SIZE) -> bytes:
    """Serialize key=value pairs into a FLSH block (zero padded to size).

    The CRC byte of the header is left 0: OpenWrt does not check it, and the
    exact checksum variant of the stock firmware is not known.
    """
    body = b"".join(("%s=%s" % (k, v)).encode("latin-1") + b"\0" for k, v in items.items())
    body += b"\0"
    total = (HDR_LEN + len(body) + 3) & ~3
    if total > size:
        raise ValueError("NVRAM content does not fit in %d bytes" % size)
    hdr = b"FLSH" + struct.pack("<IIII", total, 0x100, 0, 0)
    return (hdr + body).ljust(size, b"\0")


def load_block(path: str) -> bytes:
    with open(path, "rb") as f:
        data = f.read()
    if data[:4] == b"FLSH":            # bare NVRAM block / factory area
        return data[:NVRAM_SIZE]
    if len(data) < NVRAM_OFFSET + NVRAM_SIZE:
        raise ValueError("%s is too small for a flash dump" % path)
    return data[NVRAM_OFFSET:NVRAM_OFFSET + NVRAM_SIZE]


def summary(nv: dict) -> list:
    def radio(prefix, band):
        lines = ["%s (%s*):" % (band, prefix)]
        for k in ("11N_XCAP", "11N_THER", "11N_THER_2", "11N_TSSI_ENABLE",
                  "RF_TYPE", "REG_DOMAIN", "LED_TYPE"):
            if prefix + k in nv:
                lines.append("  %-16s 0x%s" % (k, nv[prefix + k]))
        tables = sorted(k[len(prefix):] for k in nv
                        if k.startswith(prefix + "TX_POWER"))
        lines.append("  power tables     %d (%s ...)" %
                     (len(tables), ", ".join(tables[:3])))
        return lines

    out = [
        "board          %s" % nv.get("BOARD_NAME", "?"),
        "serial         %s" % nv.get("serial_number", "?"),
        "LAN/WAN MAC    %s  (et0macaddr)" % nv.get("et0macaddr", "?"),
        "2.4 GHz MAC    %s  (wl0_hwaddr)" % nv.get("wl0_hwaddr", "?"),
        "5 GHz MAC      %s  (wl1_hwaddr)" % nv.get("wl1_hwaddr", "?"),
        "WPS PIN        %s  (default Wi-Fi key in this firmware)" %
        nv.get("wps_device_pin", "?"),
        "country        %s" % nv.get("country_code", "?"),
    ]
    out += radio("HW_WLAN1_", "2.4 GHz RTL8197F")
    out += radio("HW_WLAN0_", "5 GHz RTL8812F")
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("dump", help="programmer dump (2 or 8 MiB) or a factory NVRAM block")
    ap.add_argument("--json", action="store_true", help="print all keys as JSON")
    ap.add_argument("--all", action="store_true", help="print all keys")
    args = ap.parse_args()

    try:
        nv = parse_nvram(load_block(args.dump))
    except (OSError, ValueError) as e:
        print("error: %s" % e, file=sys.stderr)
        return 1

    if args.json:
        json.dump(nv, sys.stdout, indent=2, sort_keys=True)
        print()
    elif args.all:
        for k in sorted(nv):
            print("%s=%s" % (k, nv[k]))
    else:
        print("\n".join(summary(nv)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
