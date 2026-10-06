# SPDX-License-Identifier: GPL-2.0-only
#
# RTL819x image recipes.

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

DTS_DIR := ../dts

# The Realtek bootloader copies the boot payload to the RAM address in the
# cvimg header (flash boot) or loads a TFTP "nfjrom" file to 0x80a00000
# (RAM boot) and jumps to its first byte. A two-stage lzma-loader is used:
# stage 1 runs there and copies stage 2 (decompressor + compressed kernel) high
# into RAM, so that even a large initramfs kernel can be decompressed to the
# kernel load base without overwriting the running loader.
LOADER_ENTRY := 0x80a00000
LOADER_STAGE2 := 0x82000000
RAMSIZE := 0x04000000

# The generic lzma-loader decompresses to KERNEL_ENTRY and jumps there, which
# assumes the kernel entry symbol sits at the load base. On MIPS kernel_entry
# is inside .text, so read both from the built ELF, decompress to the load
# base and jump to the real entry. $(1): ELF name suffix ("-initramfs").
define Build/loader-kernel
	rm -rf $@.src
	mkdir -p $@.src
	$(CP) ../../generic/image/lzma-loader/src/* $@.src/
	cp "$@" "$@.src/vmlinux.lzma"
	$(SED) 's/unsigned char \*\*buffer/const unsigned char **buffer/' \
		-e 's/unsigned char \*buffer;/const unsigned char *buffer;/' \
		$@.src/decompress.c
	$(SED) 's|(unsigned char\*)KERNEL_ENTRY|(unsigned char*)KERNEL_LOAD|' \
		$@.src/decompress.c
	# Progress markers on the console (the generic loader is silent).
	cp ./loader-dbg.h $@.src/loader-dbg.h
	$(SED) 's|#include "LzmaDecode.h"|#include "LzmaDecode.h"\n#include "loader-dbg.h"|' \
		$@.src/decompress.c
	$(SED) 's|\tdata = lzma_start;|\tdbg_puts("loader: decompressing kernel\\n");\n\tdata = lzma_start;|' \
		$@.src/decompress.c
	$(SED) 's|\t\tblast_dcache(dcache_size, dcache_lsize);|\t\tdbg_puts("loader: done, jumping to kernel\\n");\n\t\tblast_dcache(dcache_size, dcache_lsize);|' \
		$@.src/decompress.c
	set -e; \
	kload=`python3 ./kernel-addrs.py --load $(KDIR)/vmlinux$(1).elf`; \
	kentry=`python3 ./kernel-addrs.py --entry $(KDIR)/vmlinux$(1).elf`; \
	echo "kernel load=$$kload entry=$$kentry loader=$(LOADER_ENTRY)/$(LOADER_STAGE2)"; \
	echo "CFLAGS += -DKERNEL_LOAD=$$kload" >> $@.src/Makefile; \
	$(MAKE) -C $@.src \
		CC="$(TARGET_CC) -EL" \
		LD="$(TARGET_CROSS)ld -EL" \
		OBJDUMP="$(TARGET_CROSS)objdump -EL" \
		O_FORMAT=elf32-tradlittlemips \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		IMAGE_COPY=1 \
		LOADER_ENTRY=$(LOADER_ENTRY) \
		LOADADDR=$(LOADER_STAGE2) \
		KERNEL_ENTRY=$$kentry \
		RAMSIZE=$(RAMSIZE) \
		all
	mv "$@.src/lzma.bin" "$@"
	rm -rf $@.src
endef

# $(1): "<RAM address the bootloader copies the payload to> <flash offset>"
define Build/cvimg
	python3 ./cvimg.py --signature cs6c \
		--start-addr $(word 1,$(1)) --burn-addr $(word 2,$(1)) "$@" "$@.new"
	mv "$@.new" "$@"
endef

define Device/Default
  PROFILES := Default
  KERNEL_DEPENDS = $$(wildcard $$(DTS_DIR)/$$(DEVICE_DTS).dts)
  BLOCKSIZE := 64k
endef

define Device/tenda_ac8-v1
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := AC8
  DEVICE_VARIANT := v1 (8 MiB flash)
  DEVICE_DTS := rtl8197f-tenda-ac8
  IMAGE_SIZE := 8064k
  KERNEL := kernel-bin | append-dtb | lzma | loader-kernel | \
	cvimg $(LOADER_ENTRY) 0x20000
  KERNEL_INITRAMFS := kernel-bin | append-dtb | lzma | loader-kernel -initramfs
  KERNEL_INITRAMFS_SUFFIX := -nfjrom.bin
  DEVICE_PACKAGES := kmod-rtl8192cd kmod-gpio-button-hotplug
  # board name of the earlier experimental builds of this repository
  SUPPORTED_DEVICES += tenda,ac8-v1-8m
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := append-kernel | pad-to $$$$(BLOCKSIZE) | \
	append-rootfs | pad-rootfs | check-size | append-metadata
endef
TARGET_DEVICES += tenda_ac8-v1

$(eval $(call BuildImage))
