// SPDX-License-Identifier: GPL-2.0-only
/*
 * OpenWrt RTL8197F platform glue for the vendor driver.
 *
 * The vendor code keeps DMA buffers coherent with explicit MIPS cache
 * maintenance (_dma_cache_wback_inv() and friends). Those hooks are not
 * exported to modules any more, so route them through the streaming DMA API,
 * which performs the same cache operations on non-coherent MIPS.
 */
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <asm/addrspace.h>

#include "./8192cd_cfg.h"

#if defined(CONFIG_RTL_8197F_WRT) && defined(CONFIG_DMA_NONCOHERENT)
static struct device rtl8192cd_dma_dummy;
static struct device *rtl8192cd_dma_dev = &rtl8192cd_dma_dummy;
static struct platform_device *rtl8192cd_dma_pdev;

void rtl8192cd_dma_init(void)
{
	struct device_node *np;

	if (rtl8192cd_dma_pdev)
		return;

	np = of_find_compatible_node(NULL, NULL, "realtek,rtl8197f-wifi");
	if (np) {
		rtl8192cd_dma_pdev = of_find_device_by_node(np);
		of_node_put(np);
	}
	if (rtl8192cd_dma_pdev)
		rtl8192cd_dma_dev = &rtl8192cd_dma_pdev->dev;
}

void rtl8192cd_dma_exit(void)
{
	if (rtl8192cd_dma_pdev) {
		rtl8192cd_dma_dev = &rtl8192cd_dma_dummy;
		put_device(&rtl8192cd_dma_pdev->dev);
		rtl8192cd_dma_pdev = NULL;
	}
}

void rtl8192cd_dma_cache_op(unsigned long start, unsigned long size, int dir)
{
	/* uncached (KSEG1) mappings need no maintenance */
	if (!size || KSEGX(start) == KSEG1)
		return;

	dma_sync_single_for_device(rtl8192cd_dma_dev, CPHYSADDR(start), size,
				   (enum dma_data_direction)dir);
}
#else
void rtl8192cd_dma_init(void)
{
}

void rtl8192cd_dma_exit(void)
{
}
#endif
