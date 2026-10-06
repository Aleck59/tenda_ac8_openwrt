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

#ifndef __HALRF_DPK_8814C_H__
#define __HALRF_DPK_8814C_H__

#if (RTL8814C_SUPPORT == 1)
/*--------------------------Define Parameters-------------------------------*/
#define DPK_VERSION_8814C 0x0
#define DPK_RF_PATH_NUM_8814C 4
#define DPK_GROUP_NUM_8814C 9
#define DPK_MAC_REG_NUM_8814C 2
#define DPK_BB_REG_NUM_8814C 11
#define DPK_RF_REG_NUM_8814C 1
#define DPK_PAS_DBG_8814C 1
#define DPK_SRAM_IQ_DBG_8814C 0
#define DPK_SRAM_read_DBG_8814C 0
#define DPK_SRAM_write_DBG_8814C 1
#define DPK_DO_PATH_A 1
#define DPK_DO_PATH_B 1
#define DPK_DO_PATH_C 1
#define DPK_DO_PATH_D 1
#define DPK_THRESHOLD_8814C 2
#define IQK_CMD_8814C 0x8
#define IQK_CMD_GL 0x0
#define IQK_CMD_DPD 0x1
#define GSK2 0x2
#define GSK3 0x3
#define GSK4 0x4

#define DPK_RF_NUM_8814C 4
#define DPK_MAC_NUM_8814C 1
#define DPK_BB_NUM_8814C 31



/*---------------------------End Define Parameters----------------------------*/

void dpk_sram_read_8814c(void *dm_void);

void dpk_enable_disable_8814c(void *dm_void);

void dpk_reload_8814c(void *dm_void);

void do_dpk_8814c(void *dm_void);

void dpk_track_8814c(void *dm_void);

void dpk_set_dpkbychannel_8814c(void *dm_void, boolean dpk_by_ch);

boolean dpk_get_dpkbychannel_8814c(void *dm_void);

void dpk_set_is_dpk_enable_8814c(void *dm_void, boolean is_dpk_enable);

boolean dpk_get_is_dpk_enable_8814c(void *dm_void);

void dpk_thermal_read_8814c(void *dm_void);






#endif /* RTL8814C_SUPPORT */

#endif /*#ifndef __HALRF_DPK_8814C_H__*/
