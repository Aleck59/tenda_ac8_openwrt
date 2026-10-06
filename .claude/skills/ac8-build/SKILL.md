---
name: ac8-build
description: Build the Tenda AC8 OpenWrt firmware or iterate on part of it (kernel, rtl8192cd Wi-Fi driver, DTS, base-files, image recipe). Use when asked to build, rebuild, test a change, check image size or debug a build failure.
---

# Building and iterating

## Full build

```sh
./scripts/build.sh                 # prepare + download + make -> out/
```

- Tree: `build/openwrt` (override with `OPENWRT_DIR`), feeds in `build/feeds`.
- `SKIP_PREPARE=1` reuses the tree; `CONFIG_EXTRA=file` appends .config lines.
- First build ~1-2 h, ~20 GB. On failure the script re-runs `make -j1 V=s`.
- Run as an unprivileged user (OpenWrt refuses to build as root unless
  `FORCE_UNSAFE_CONFIGURE=1`).

## After editing the overlay

`prepare.sh` copies `openwrt/` into the tree. After editing files in this
repository either re-run `./scripts/prepare.sh` or copy the changed files
into `build/openwrt` by hand, then:

| Changed | Rebuild |
|---|---|
| DTS, kernel patches, `files/`, `config-6.18` | `make target/linux/{clean,compile}` then `make` |
| `package/kernel/rtl8192cd` | `make package/kernel/rtl8192cd/{clean,compile} V=s` |
| `base-files/` | `make package/base-files/{clean,compile}` then `make` |
| `image/` | `make target/linux/install` |

## Checks before pushing

```sh
for f in scripts/*.sh; do sh -n "$f"; done; shellcheck -S warning scripts/*.sh
python3 -m py_compile scripts/*.py openwrt/target/linux/rtl819x/image/*.py
```

Compile the DTS standalone (needs kernel `include/`):

```sh
cd openwrt/target/linux/rtl819x/dts
cpp -nostdinc -undef -x assembler-with-cpp -I. -I<linux>/include \
  rtl8197f-tenda-ac8.dts | dtc -I dts -O dtb -o /tmp/ac8.dtb -
```

## Verifying images

- `out/*-squashfs-sysupgrade.bin` must start with `cs6c`; `scripts/mkflash.py`
  validates header, burn address 0x20000 and the 16-bit checksum.
- The size check of the image recipe fails the build above 8064 KiB.
- `out/*.manifest` lists the installed packages.

## Driver build notes

- GCC 14 permerrors are downgraded for the vendor code
  (`-Wno-error=incompatible-pointer-types -Wno-error=int-conversion`);
  `implicit-function-declaration` stays an error, fix those.
- modpost "undefined" means a vendor helper is compiled out or a kernel
  symbol is not exported (see `rtl8197f_compat.c` for the DMA cache hooks).
