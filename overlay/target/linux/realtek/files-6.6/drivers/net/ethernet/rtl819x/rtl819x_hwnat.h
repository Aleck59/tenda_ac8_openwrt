/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The conntrack hardware NAT of the DIR-842 port hooks the ndo_flow_offload
 * interface of the OpenWrt 4.14 kernels, which Linux 6.6 does not have.
 * Software routing only: the lifecycle hooks are no-ops.
 */
#ifndef _RTL819X_HWNAT_H
#define _RTL819X_HWNAT_H

#include <linux/netdevice.h>

static inline void rtl819x_hwnat_start(struct net_device *dev) { }
static inline void rtl819x_hwnat_stop(void) { }

#endif /* _RTL819X_HWNAT_H */
