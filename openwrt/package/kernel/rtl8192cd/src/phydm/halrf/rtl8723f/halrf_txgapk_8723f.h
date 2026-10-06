/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

#ifndef __HALRF_TXGAPK_8723F_H__
#define __HALRF_TXGAPK_8723F_H__

#if (RTL8723F_SUPPORT == 1)
/*--------------------------Define Parameters-------------------------------*/
#define D_CLR_8723F 0
#define DIFFK_EN_8723F 1
#define TAK_EN_8723F 2
#define GAPK_RF_PATH_NUM_8723F 2
#define GAPK_BB_REG_NUM_8723F 6
#define GAPK_RF_REG_NUM_8723F 8
#define GAPK_REG_DBG_8723F 0
#define GAPK_PSD_PWR_DBG_8723F 1
#define GAPK_TRACK_TABLE_8723F 1
#define GAPK_POWER_TABLE_8723F 0 /*disable power table gapk*/

/*---------------------------End Define Parameters----------------------------*/

//void halrf_txgapk_reload_tx_gain_8723f(void *dm_void);

void halrf_txgapk_8723f(struct dm_struct *dm);

#endif /* RTL8723F_SUPPORT */
#endif /*#ifndef __HALRF_TXGAPK_8723F_H__*/
