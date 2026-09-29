// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL8197F cr6c/cs6c firmware parser.
 *
 * The 16-byte header of the Realtek boot loader holds a big-endian length
 * at offset 12: the loader and kernel plus the two checksum bytes.  The
 * SquashFS starts right after them, at 0x10 + length.  The erase blocks
 * after the SquashFS become "rootfs_data" for the JFFS2 overlay; the images
 * end with a padjffs2 (deadc0de) marker at that boundary.
 *
 * Taken from the OpenWrt 24.10 RTL8197F port (tobyw121/Openwrt_RTL) and the
 * Tenda AC8 rootfs_data change, for Linux 4.14.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/byteorder/generic.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>

#include "mtdsplit.h"

#define RTL8197F_NR_PARTS 3
/* Fewer free erase blocks than this are no use to JFFS2. */
#define RTL8197F_MIN_DATA_BLOCKS 4

struct rtl8197f_img_header {
	u8 signature[4];
	__be32 start_addr;
	__be32 burn_addr;
	__be32 length;
} __packed;

static bool rtl8197f_valid_signature(const u8 signature[4])
{
	return !memcmp(signature, "cr6c", 4) ||
	       !memcmp(signature, "cs6c", 4) ||
	       !memcmp(signature, "csys", 4);
}

static int mtdsplit_parse_rtl8197f(struct mtd_info *master,
				   const struct mtd_partition **pparts,
				   struct mtd_part_parser_data *data)
{
	struct rtl8197f_img_header hdr;
	struct mtd_partition *parts;
	size_t retlen;
	u64 rootfs_offset, data_offset, data_end;
	size_t squashfs_len;
	u32 image_len;
	int nr_parts = 2;
	int err;

	err = mtd_read(master, 0, sizeof(hdr), &retlen, (void *)&hdr);
	if (err)
		return err;
	if (retlen != sizeof(hdr))
		return -EIO;
	if (!rtl8197f_valid_signature(hdr.signature))
		return -EINVAL;

	image_len = be32_to_cpu(hdr.length);
	rootfs_offset = sizeof(hdr) + (u64)image_len;
	if (image_len < 2 || rootfs_offset >= master->size)
		return -EINVAL;

	err = mtd_check_rootfs_magic(master, rootfs_offset, NULL);
	if (err)
		return err;

	parts = kcalloc(RTL8197F_NR_PARTS, sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	pr_info("rtl8197f-fw: %s split kernel=0x%llx rootfs=0x%llx\n",
		master->name, rootfs_offset, rootfs_offset);

	if ((master->flags & MTD_WRITEABLE) &&
	    !mtd_get_squashfs_len(master, rootfs_offset, &squashfs_len)) {
		data_offset = mtd_roundup_to_eb(rootfs_offset + squashfs_len,
						master);
		data_end = mtd_rounddown_to_eb(master->size, master);
		if (data_end > data_offset &&
		    data_end - data_offset >=
		    (u64)RTL8197F_MIN_DATA_BLOCKS * master->erasesize) {
			parts[2].name = ROOTFS_SPLIT_NAME;
			parts[2].offset = data_offset;
			parts[2].size = data_end - data_offset;
			nr_parts = 3;
			pr_info("rtl8197f-fw: %s rootfs_data=0x%llx+0x%llx\n",
				master->name, data_offset, data_end - data_offset);
		}
	}

	*pparts = parts;
	return nr_parts;
}

static struct mtd_part_parser mtdsplit_rtl8197f_parser = {
	.owner = THIS_MODULE,
	.name = "rtl8197f-fw",
	.parse_fn = mtdsplit_parse_rtl8197f,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

static int __init mtdsplit_rtl8197f_init(void)
{
	register_mtd_parser(&mtdsplit_rtl8197f_parser);

	return 0;
}

subsys_initcall(mtdsplit_rtl8197f_init);
