---
name: ac8-release
description: Cut a Tenda AC8 firmware release (version tag, CI build, GitHub Release assets). Use when asked to publish, tag or release a new firmware version.
---

# Releasing

1. Make sure `main` builds in CI (Actions -> "Build firmware") and docs are
   current (`docs/install.md`, `README.md` status section).
2. Pick the version: `vMAJOR.MINOR.PATCH`; a hyphen suffix (`v1.2.0-rc1`)
   becomes a pre-release.
3. Tag and push:

   ```sh
   git tag -a v1.2.0 -m "Tenda AC8 OpenWrt v1.2.0"
   git push origin v1.2.0
   ```

4. The workflow builds with `CONFIG_VERSION_CODE="ac8-<tag>"` (and
   `VERSION_CODE_FILENAMES`), so images are named
   `openwrt-ac8-<tag>-rtl819x-rtl8197f-tenda_ac8-v1-*.bin`, and creates the release from
   `docs/release-notes.md` (`@VERSION@`, `@REPO@` substituted) plus
   `sha256sums`, the package manifest and `*.buildinfo`.
5. Check the release page: sysupgrade + initramfs-nfjrom images, sha256sums.

Never move or delete a published tag; fix forward with a new patch version.
