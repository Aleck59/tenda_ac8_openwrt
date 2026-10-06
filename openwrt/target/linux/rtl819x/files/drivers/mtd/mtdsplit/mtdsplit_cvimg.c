// SPDX-License-Identifier: GPL-2.0-only
/*
 * Firmware partition parser for Realtek "cvimg" boot images.
 *
 * The Realtek RTL819x bootloader boots the image at the start of the
 * firmware area: a 16 byte header (signature "cs6c"/"cr6c", then big-endian
 * load address, flash address and payload length) followed by the payload
 * (here: the OpenWrt lzma-loader with the kernel) and a 16-bit checksum.
 * The squashfs root filesystem follows on the next erase block boundary.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,12,0)
#include <asm/unaligned.h>
#else
#include <linux/unaligned.h>
#endif

#include "mtdsplit.h"

#define CVIMG_NR_PARTS		2
#define CVIMG_HDR_LEN		16

struct cvimg_header {
	u8 signature[4];
	u8 start_addr[4];
	u8 burn_addr[4];
	u8 len[4];
};

static int mtdsplit_parse_cvimg(struct mtd_info *master,
				const struct mtd_partition **pparts,
				struct mtd_part_parser_data *data)
{
	struct mtd_partition *parts;
	struct cvimg_header hdr;
	size_t retlen, rootfs_offset;
	u64 kernel_end;
	u32 len;
	int err;

	err = mtd_read(master, 0, sizeof(hdr), &retlen, (void *)&hdr);
	if (err)
		return err;
	if (retlen != sizeof(hdr))
		return -EIO;

	if (memcmp(hdr.signature, "cs6c", 4) && memcmp(hdr.signature, "cr6c", 4))
		return -EINVAL;

	len = get_unaligned_be32(hdr.len);
	kernel_end = (u64)CVIMG_HDR_LEN + len;
	if (!len || kernel_end >= master->size)
		return -EINVAL;

	err = mtd_find_rootfs_from(master,
				   round_up(kernel_end, master->erasesize),
				   master->size, &rootfs_offset, NULL);
	if (err)
		return err;

	parts = kcalloc(CVIMG_NR_PARTS, sizeof(*parts), GFP_KERNEL);
	if (!parts)
		return -ENOMEM;

	parts[0].name = KERNEL_PART_NAME;
	parts[0].offset = 0;
	parts[0].size = rootfs_offset;

	parts[1].name = ROOTFS_PART_NAME;
	parts[1].offset = rootfs_offset;
	parts[1].size = master->size - rootfs_offset;

	*pparts = parts;
	return CVIMG_NR_PARTS;
}

static const struct of_device_id mtdsplit_cvimg_of_match_table[] = {
	{ .compatible = "realtek,cvimg" },
	{},
};
MODULE_DEVICE_TABLE(of, mtdsplit_cvimg_of_match_table);

static struct mtd_part_parser mtdsplit_cvimg_parser = {
	.owner = THIS_MODULE,
	.name = "cvimg-fw",
	.of_match_table = mtdsplit_cvimg_of_match_table,
	.parse_fn = mtdsplit_parse_cvimg,
	.type = MTD_PARSER_TYPE_FIRMWARE,
};

module_mtd_part_parser(mtdsplit_cvimg_parser);
