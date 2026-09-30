#
# Tenda AC8 v1 images.
#
# The Realtek boot loader of the AC8 expects a "cs6c" image at flash
# 0x20000: a 16-byte header (signature, start address, burn address,
# length), the payload, and two bytes that make the 16-bit sum of the
# payload zero.  It copies the payload to the start address and jumps there.
# OpenWrt uses the same layout, with the SquashFS right after the payload:
#
#   0x20000: cs6c header | LZMA loader + kernel | checksum | SquashFS | JFFS2
#
# The mtdsplit parser rtl8197f-fw splits the "firmware" partition into
# kernel, rootfs (the SquashFS right after the payload) and rootfs_data (the
# erase blocks after the SquashFS, where padjffs2 puts the deadc0de marker).
#
# Flash layout (mtdparts on the command line of the loader):
#   0x000000  boot          128k  boot loader, Tenda board data at 0x1c000
#                                 (MAC addresses, Wi-Fi calibration), ro
#   0x020000  firmware            cs6c image + rootfs_data
#   end-128k  tenda_config  128k  Tenda configuration, ro
#
# Two flash-size variants of the same kernel differ only in the command line
# of the LZMA loader.  The kernel does not override it (no CMDLINE_BOOL).  The
# loader passes it as its own argv (argc 2, argv[1]): a0/a1 of the Tenda boot
# loader are no argument vector.
#
#   *-squashfs-flash.bin       the firmware partition from 0x20000, for the
#                              boot loader (TFTP), a programmer or
#                              scripts/make-fullflash.py
#   *-squashfs-sysupgrade.bin  the same with metadata, for sysupgrade/LuCI
#
# This is free software, licensed under the GNU General Public License v2.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

# The boot loader copies the payload to the start address of the header and
# jumps there; the kernel is unpacked to KERNEL_LOADADDR.  The kernel stays
# below 0x80a00000, see Image/Prepare.
FLASH_TEXT_START := 0x80a00000
KERNEL_LOADADDR := 0x80000000
FW_BURN_ADDR := 0x20000

# Firmware partitions: flash size - 128k boot - 128k Tenda configuration.
AC8_FW_SIZE_8m := 8126464
AC8_FW_SIZE_16m := 16515072

# mem=: prom_detect_memsize() of the target reads the RAM size from the bond
# option register, and its table has no entry for the RTL8197FH-VG.
AC8_CMDLINE := console=ttyS0,115200 mem=64M rootfstype=squashfs
AC8_MTDPARTS_8m := mtdparts=m25p80:128k(boot)ro,7936k(firmware),128k(tenda_config)ro
AC8_MTDPARTS_16m := mtdparts=m25p80:128k(boot)ro,16128k(firmware),128k(tenda_config)ro

# /tmp/sysinfo/board_name of each variant (base-files/lib/rtkmipsel.sh), the
# names of the OpenWrt 24.10 builds of this port.
AC8_SUPPORTED_8m := tenda,ac8-v1-8m
AC8_SUPPORTED_16m := tenda,ac8-v1-16m

MKRTKIMG := python3 $(TOPDIR)/target/linux/rtkmipsel/image/mkrtkimg.py
LOADER_MAKE := $(NO_TRACE_MAKE) -C lzma-loader KDIR=$(KDIR) LINUX_DIR=$(LINUX_DIR)

define Build/Clean
	$(LOADER_MAKE) clean
endef

# $(1): flash size variant (8m, 16m)
define Image/BuildLoader
	$(LOADER_MAKE) LOADER=loader-tenda-ac8-$(1).bin \
		KERNEL_CMDLINE="$(AC8_CMDLINE) $(AC8_MTDPARTS_$(1))" \
		LZMA_TEXT_START=$(FLASH_TEXT_START) \
		LOADADDR=$(KERNEL_LOADADDR) \
		LOADER_DATA="$(KDIR)/vmlinux.bin.lzma" \
		BOARD="tenda-ac8" \
		compile loader.bin
endef

# The unpacked kernel (up to _end, .bss included) must end below the
# loader, which runs at FLASH_TEXT_START.
define Image/Prepare
	end=$$$$($(TARGET_CROSS)nm $(LINUX_DIR)/vmlinux | awk '$$$$3 == "_end" { print substr($$$$1, length($$$$1) - 7) }'); \
	[ -n "$$$$end" ] && [ $$$$(( 0x$$$$end )) -lt $$$$(( $(FLASH_TEXT_START) )) ] || { \
		echo "kernel end 0x$$$$end overlaps the loader at $(FLASH_TEXT_START)" >&2; exit 1; }
	$(STAGING_DIR_HOST)/bin/lzma e $(KDIR)/vmlinux -lc1 -lp2 -pb2 \
		$(KDIR)/vmlinux.bin.lzma
	$(call Image/BuildLoader,8m)
	$(call Image/BuildLoader,16m)
endef

# $(1): flash size variant
#   1. loader + kernel -> cs6c image (header, payload, checksum bytes);
#   2. + SquashFS, then padjffs2 puts the deadc0de marker at the next 64k
#      boundary (the image starts at 0x20000, so the boundary is one of the
#      flash too): the JFFS2 overlay starts there on the first boot;
#   3. size check against the firmware partition;
#   4. sysupgrade.bin = flash.bin + metadata.
define Image/Build/tenda-ac8
	$(MKRTKIMG) build --sig cs6c --start $(FLASH_TEXT_START) --burn $(FW_BURN_ADDR) \
		--body $(KDIR)/loader-tenda-ac8-$(1).bin \
		--out $(KDIR)/tenda-ac8-$(1)-kernel.bin
	$(MKRTKIMG) concat --out $(KDIR)/tenda-ac8-$(1)-flash.bin \
		$(KDIR)/tenda-ac8-$(1)-kernel.bin $(KDIR)/root.squashfs
	$(STAGING_DIR_HOST)/bin/padjffs2 $(KDIR)/tenda-ac8-$(1)-flash.bin 64
	$(MKRTKIMG) checksize --file $(KDIR)/tenda-ac8-$(1)-flash.bin \
		--max $$$$(( $(AC8_FW_SIZE_$(1)) - 4 * 65536 ))
	$(CP) $(KDIR)/tenda-ac8-$(1)-flash.bin \
		$(BIN_DIR)/$(IMG_PREFIX)-tenda_ac8-v1-$(1)-squashfs-flash.bin
	$(CP) $(KDIR)/tenda-ac8-$(1)-flash.bin \
		$(BIN_DIR)/$(IMG_PREFIX)-tenda_ac8-v1-$(1)-squashfs-sysupgrade.bin
	echo $(call metadata_json,$(AC8_SUPPORTED_$(1))) | \
		fwtool -I - $(BIN_DIR)/$(IMG_PREFIX)-tenda_ac8-v1-$(1)-squashfs-sysupgrade.bin
endef

define Image/Build/squashfs
	$(call Image/Build/tenda-ac8,8m)
	$(call Image/Build/tenda-ac8,16m)
endef

define Image/Build
	$(call Image/Build/$(1),$(1))
endef

$(eval $(call BuildImage))
