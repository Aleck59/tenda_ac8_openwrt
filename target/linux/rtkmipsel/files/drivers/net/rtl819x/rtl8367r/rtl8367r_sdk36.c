/*
 * Functions the rtknet driver of Realtek SDK v3.6.0 (rtl_nic.c,
 * rtl865x_proc_debug.c) calls for an external RTL8367 switch, and which the
 * older rtl8367r/ API of the Tenda AC10U fork does not have.  SDK v3.6.0 has
 * them only for its rtl83xx/ API.  Implemented with the rtl8367b API where
 * the driver needs them (port counters, PHY autonegotiation restart,
 * mirroring); the debug-only ones say they are not supported.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>

#include "rtk_types.h"
#include "rtk_error.h"
#include "rtk_api.h"
#include "rtk_api_ext.h"
#include "rtl8367b_asicdrv.h"

/* SMI accesses of the statistics path must not interleave. */
static DEFINE_SPINLOCK(rtl8367r_stat_lock);

/* Port counters for the net_device statistics of rtknet. */
int rtl_stat_port_getserial(rtk_uint32 port, rtk_stat_port_cntr_t *pPort_cntrs)
{
	unsigned long flags;
	rtk_api_ret_t ret;

	spin_lock_irqsave(&rtl8367r_stat_lock, flags);
	ret = rtk_stat_port_getAll(port, pPort_cntrs);
	spin_unlock_irqrestore(&rtl8367r_stat_lock, flags);

	return ret;
}

/* Restart autonegotiation of a PHY port (BMCR bit 9). */
int rtl_8367r_restartPHYNway(rtk_uint32 port)
{
	rtk_uint32 data;
	rtk_api_ret_t ret;

	if (port > RTK_PHY_ID_MAX)
		return RT_ERR_PORT_ID;

	ret = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data);
	if (ret != RT_ERR_OK)
		return ret;

	return rtk_port_phyReg_set(port, PHY_CONTROL_REG, data | 0x0200);
}

/* The 8367R path has no ACL to the CPU (CONFIG_RTL_83XX_ACL_SUPPORT is off). */
int rtl865x_disableRtl83xxToCpuAcl(void)
{
	return 0;
}

int rtl_mirror_portBased_set(rtk_uint32 mirroring_port, rtk_uint32 Mirrored_rx_portmask,
			     rtk_uint32 Mirrored_tx_portmask)
{
	rtk_portmask_t rx, tx;

	rx.bits[0] = Mirrored_rx_portmask;
	tx.bits[0] = Mirrored_tx_portmask;

	return rtk_mirror_portBased_set(mirroring_port, &rx, &tx);
}

int rtl_mirror_portBased_get(rtk_uint32 *mirroring_port, rtk_uint32 *Mirrored_rx_portmask,
			     rtk_uint32 *Mirrored_tx_portmask)
{
	rtk_portmask_t rx, tx;
	rtk_port_t port;
	rtk_api_ret_t ret;

	ret = rtk_mirror_portBased_get(&port, &rx, &tx);
	if (ret != RT_ERR_OK)
		return ret;

	*mirroring_port = port;
	*Mirrored_rx_portmask = rx.bits[0];
	*Mirrored_tx_portmask = tx.bits[0];

	return RT_ERR_OK;
}

int rtl_mirror_portIso_set(rtk_uint32 isolation)
{
	return rtk_mirror_portIso_set(isolation ? ENABLED : DISABLED);
}

int rtl_mirror_portIso_get(rtk_uint32 *isolation)
{
	rtk_enable_t enable;
	rtk_api_ret_t ret;

	ret = rtk_mirror_portIso_get(&enable);
	if (ret != RT_ERR_OK)
		return ret;

	*isolation = (enable == ENABLED);

	return RT_ERR_OK;
}

int rtl_port_isolation_leak_set(bool enable)
{
	return 0;
}

int rtl_8367_chip_type_read(struct seq_file *s, void *v)
{
	rtk_uint32 id = 0, ver = 0;

	rtl8367b_getAsicReg(0x1300, &id);
	rtl8367b_getAsicReg(0x1301, &ver);
	seq_printf(s, "chip id 0x%04x version 0x%04x\n", id, ver);

	return 0;
}

int rtl_83xx_share_meter_read_proc(struct seq_file *s, void *v)
{
	seq_puts(s, "not supported with the rtl8367r API\n");

	return 0;
}

int rtl_83xx_storm_ctrl_read_proc(struct seq_file *s, void *v)
{
	seq_puts(s, "not supported with the rtl8367r API\n");

	return 0;
}

int rtl_83xx_storm_ctrl_write_proc(struct file *file, const char *buffer,
				   unsigned long count, void *data)
{
	return count;
}

void rtl_get_83xx_snr(void)
{
	printk("SNR: not supported with the rtl8367r API\n");
}
