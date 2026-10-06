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
/*8814C IQK ver:0x3 20210112*/
#include "mp_precomp.h"
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#if RT_PLATFORM == PLATFORM_MACOSX
#include "phydm_precomp.h"
#else
#include "../phydm_precomp.h"
#endif
#else
#include "../../phydm_precomp.h"
#endif

#if (RTL8814C_SUPPORT == 1)

/*---------------------------Define Local Constant---------------------------*/
#if !(DM_ODM_SUPPORT_TYPE & ODM_AP)
void do_iqk_8814c(
	void *dm_void,
	u8 delta_thermal_index,
	u8 thermal_value,
	u8 threshold)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	dm->rf_calibrate_info.thermal_value_iqk = thermal_value;
	halrf_segment_iqk_trigger(dm, true, false);
}
#else
/*Originally config->do_iqk is hooked phy_iq_calibrate_8814C, but do_iqk_8814C and phy_iq_calibrate_8814C have different arguments*/
void do_iqk_8814c(
	void *dm_void,
	u8 delta_thermal_index,
	u8 thermal_value,
	u8 threshold)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	boolean is_recovery = (boolean)delta_thermal_index;

	halrf_segment_iqk_trigger(dm, true, false);
}
#endif

void _iqk_dump_reg_8814c(struct dm_struct *dm, u8 path)
{
	u32 i = 0;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	for(i = 0; i < 0x100/4; i++)
	RF_DBG(dm, DBG_RF_IQK, "[IQK][Dump]0x1b08 = 0x%x!!!\n", odm_get_bb_reg(dm, 0x1b00 | i << 2 , BIT(26)));
}
boolean _iqk_check_cal_8814c(
	struct dm_struct *dm,
	u8 path,
	u8 cmd)
{
	boolean notready = true, fail = true;
	u32 delay_count = 0x0;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	while (notready) {
		if (odm_read_1byte(dm, 0x2d9c) == 0x55) {
			if (cmd == 0x0) /*LOK*/
				fail = false;
			else
				fail = (boolean)odm_get_bb_reg(dm, 0x1b08, BIT(26));
			notready = false;
		} else {
			ODM_delay_us(1);
			delay_count++;
		}

		if (delay_count >= 30000) {
			fail = true;
			RF_DBG(dm, DBG_RF_IQK, "[IQK]IQK timeout!!!\n");
			break;
		}
	}	
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x0);
	halrf_delay_10us(1);
	if(!fail)
		odm_set_bb_reg(dm, 0x1b20, BIT(26), 0x1);	
	else
		odm_set_bb_reg(dm, 0x1b20, BIT(26), 0x0);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]delay count = %d!!!\n", delay_count);	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]0x1b08 = 0x%x!!!\n", odm_get_bb_reg(dm, 0x1b08, BIT(26)));
	//return fail;	
	return false;
}

void _iqk_information_8814c(
	struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	iqk_info->iqk_band = (u8)*dm->band_type;
	iqk_info->iqk_ch = (u8)*dm->channel;
	iqk_info->iqk_bw = (u8)*dm->band_width;

	RF_DBG(dm, DBG_RF_DPK, "[IQK] Band/ CH/ BW = %s / %d / %s\n",
	       iqk_info->iqk_band == 0 ? "2G" : "5G",
	       iqk_info->iqk_ch,
	       iqk_info->iqk_bw == 0 ? "20M" : (iqk_info->iqk_bw == 1 ? "40M" : "80M"));
}

void _iqk_backup_mac_bb_8814c(
	struct dm_struct *dm,
	u32 *mac_backup,
	u32 *bb_backup,
	u32 *backup_mac_reg,
	u32 *backup_bb_reg)
{
	u32 i;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);

	for (i = 0; i < MAC_REG_NUM_8814C; i++){
		mac_backup[i] = odm_read_4byte(dm, backup_mac_reg[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]Backup mac addr = %x, value =% x\n", backup_mac_reg[i], mac_backup[i]);
	}
	for (i = 0; i < BB_REG_NUM_8814C; i++){
		bb_backup[i] = odm_read_4byte(dm, backup_bb_reg[i]);		
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]Backup bbaddr = %x, value =% x\n", backup_bb_reg[i], bb_backup[i]);
	}
	return;
}

void _iqk_backup_rf_8814c(
	struct dm_struct *dm,
	u32 rf_backup[][SS_8814C],
	u32 *backup_rf_reg)
{
	u32 i = 0;
	u8 path = 0;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	for(path = 0; path < SS_8814C; path++)
		for (i = 0; i < RF_REG_NUM_8814C; i++) {
			rf_backup[i][path] = odm_get_rf_reg(dm, path, backup_rf_reg[i], MASK20BITS);			
			//RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, bp 0x%x = 0x%x\n", path, backup_rf_reg[i], rf_backup[i][path]);
	}
}

void _iqk_restore_mac_bb_8814c(
	struct dm_struct *dm,
	u32 *mac_backup,
	u32 *bb_backup,
	u32 *backup_mac_reg,
	u32 *backup_bb_reg)
{
	u32 i;	

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);

	/*toggle IGI*/
	odm_write_4byte(dm, 0x1d70, 0x50505050);

	for (i = 0; i < MAC_REG_NUM_8814C; i++){
		odm_write_4byte(dm, backup_mac_reg[i], mac_backup[i]);
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]restore mac = %x, value = %x\n",backup_mac_reg[i],mac_backup[i]);
	}
	for (i = 0; i < BB_REG_NUM_8814C; i++){
		odm_set_bb_reg(dm, backup_bb_reg[i], MASKDWORD, bb_backup[i]);		
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]restore bb = %x, value = %x\n",backup_bb_reg[i],bb_backup[i]);
	}	
	/*rx go throughput IQK*/
	odm_set_bb_reg(dm, 0x180c, BIT(31), 0x1);
	odm_set_bb_reg(dm, 0x410c, BIT(31), 0x1);
	odm_set_bb_reg(dm, 0x520c, BIT(31), 0x1);
	odm_set_bb_reg(dm, 0x530c, BIT(31), 0x1);
}

void _iqk_restore_rf_8814c(
	struct dm_struct *dm,
	u32 *rf_reg,
	u32 data[][SS_8814C])
{
	u32 i = 0;
	u8 path = 0;
	u32 tmp =0;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	for(path = 0; path < SS_8814C; path++)
		for (i = 0; i < RF_REG_NUM_8814C; i++) {
			odm_set_rf_reg(dm, path, rf_reg[i], MASK20BITS, data[i][path]);
			RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, restore 0x%x = 0x%x\n", path, rf_reg[i], data[i][path]);
	}
#if 1
	odm_set_rf_reg(dm, RF_PATH_A, 0xef, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_B, 0xef, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_C, 0xef, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_D, 0xef, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_A, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_B, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_C, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_D, 0xde, MASK20BITS, 0x0);

	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xef, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, 0xef = %x !!!\n", path, tmp);	
	}
	
	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xde, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, 0xde = %x !!!\n", path, tmp);	
	}
#endif
}

void _iqk_set_macbbafe_8814c(
	struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;	

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	/*MACBB register setting*/
	odm_write_1byte(dm, REG_TXPAUSE, 0xff);
//01_8814C_AFE_on_BB_Setting_IQK_reg
	odm_set_bb_reg(dm, 0x1e24, 0x00020000, 0x1);
	odm_set_bb_reg(dm, 0x1cd0, 0x10000000, 0x1);
	odm_set_bb_reg(dm, 0x1cd0, 0x20000000, 0x1);
	odm_set_bb_reg(dm, 0x1cd0, 0x40000000, 0x1);
	odm_set_bb_reg(dm, 0x1cd0, 0x80000000, 0x0);
	odm_set_bb_reg(dm, 0x1b08, MASKDWORD, 0x00000080);
	odm_set_bb_reg(dm, 0x1d58, 0x00000ff8, 0x1ff);
	odm_set_bb_reg(dm, 0x1834, 0x00008000, 0x1);
	odm_set_bb_reg(dm, 0x4134, 0x00008000, 0x1);
	odm_set_bb_reg(dm, 0x5234, 0x00008000, 0x1);
	odm_set_bb_reg(dm, 0x5334, 0x00008000, 0x1);
#if 1
//need check with RDC
	/*dont_rst_sync_path_when_txon*/
	odm_set_bb_reg(dm, 0x1d60, 0x80000000, 0x1);
	//Prevent anapar state unknown at sine PSD
	/*OFDM/CCK off*/
	/*odm_set_bb_reg(dm, 0x1c3c, BIT(0), 0x0);*/
	/*odm_set_bb_reg(dm, 0x1c3c, BIT(1), 0x0);*/
	odm_set_bb_reg(dm, 0x1a14, 0x00000300, 0x3);
	/*prevent CCK CCA*/
	//odm_set_bb_reg(dm, 0x1a00, 0x00000003, 0x2);
	/*rx path on*/
	odm_set_bb_reg(dm, 0x0824, 0x000f0000, 0xf);
	/*tx_scale_0dB*/
	odm_set_bb_reg(dm, 0x186c, BIT(7), 0x1);
	odm_set_bb_reg(dm, 0x416c, BIT(7), 0x1);
	odm_set_bb_reg(dm, 0x526c, BIT(7), 0x1);
	odm_set_bb_reg(dm, 0x536c, BIT(7), 0x1);
#endif
	odm_set_bb_reg(dm, 0x180c, 0x08000000, 0x1);
	odm_set_bb_reg(dm, 0x410c, 0x08000000, 0x1);
	odm_set_bb_reg(dm, 0x520c, 0x08000000, 0x1);
	odm_set_bb_reg(dm, 0x530c, 0x08000000, 0x1);
	odm_set_bb_reg(dm, 0x186c, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x416c, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x526c, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x536c, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x180c, 0x00000003, 0x0);
	odm_set_bb_reg(dm, 0x410c, 0x00000003, 0x0);
	odm_set_bb_reg(dm, 0x520c, 0x00000003, 0x0);
	odm_set_bb_reg(dm, 0x530c, 0x00000003, 0x0);
	odm_set_bb_reg(dm, 0x1a00, 0x00000003, 0x2);
	odm_set_bb_reg(dm, 0x1c38, MASKDWORD, 0xffffffff);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x701f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x702f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x703f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x704f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x705f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x706f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x707f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x708f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x709f0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70af0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70bf0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70cf0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70df0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70ef0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x701f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x702f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x703f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x704f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x705f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x706f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x707f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x708f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x709f0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70af0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70bf0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70cf0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70df0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70ef0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x701f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x702f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x703f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x704f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x705f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x706f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x707f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x708f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x709f0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70af0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70bf0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70cf0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70df0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70ef0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x700f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x701f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x702f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x703f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x704f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x705f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x706f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x707f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x708f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x709f0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70af0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70bf0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70cf0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70df0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70ef0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70ff0001);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70ff0001);
	return;
}

void _iqk_reload_macbbafe_8814c(
	struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
//11_8814C_restore_AFE_BB_RF_KIP_Settings_20210105
	//odm_set_rf_reg(dm, RF_PATH_A, 0xde, 0x10000, 0x0);
	//odm_set_rf_reg(dm, RF_PATH_B, 0xde, 0x10000, 0x0);
	//odm_set_rf_reg(dm, RF_PATH_C, 0xde, 0x10000, 0x0);
	//odm_set_rf_reg(dm, RF_PATH_D, 0xde, 0x10000, 0x0);
	odm_set_bb_reg(dm, 0x1b08, MASKDWORD, 0x00000000);
	odm_set_bb_reg(dm, 0x1c38, MASKDWORD, 0xffa1005e);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x700b8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70144041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70244041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70344041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70444041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x705b8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70644041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x707b8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x708b8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x709b8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70ab8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70bb8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70cb8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70db8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70eb8041);
	odm_set_bb_reg(dm, 0x1830, MASKDWORD, 0x70fb8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x700b8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70144041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70244041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70344041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70444041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x705b8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70644041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x707b8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x708b8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x709b8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70ab8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70bb8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70cb8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70db8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70eb8041);
	odm_set_bb_reg(dm, 0x4130, MASKDWORD, 0x70fb8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x700b8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70144041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70244041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70344041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70444041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x705b8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70644041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x707b8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x708b8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x709b8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70ab8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70bb8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70cb8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70db8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70eb8041);
	odm_set_bb_reg(dm, 0x5230, MASKDWORD, 0x70fb8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x700b8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70144041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70244041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70344041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70444041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x705b8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70644041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x707b8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x708b8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x709b8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70ab8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70bb8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70cb8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70db8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70eb8041);
	odm_set_bb_reg(dm, 0x5330, MASKDWORD, 0x70fb8041);
	odm_set_bb_reg(dm, 0x1d58, 0x00000ff8, 0x000);
	odm_set_bb_reg(dm, 0x1834, 0x00008000, 0x0);
	odm_set_bb_reg(dm, 0x4134, 0x00008000, 0x0);
	odm_set_bb_reg(dm, 0x5234, 0x00008000, 0x0);
	odm_set_bb_reg(dm, 0x5334, 0x00008000, 0x0);
	odm_set_bb_reg(dm, 0x180c, 0x08000000, 0x0);
	odm_set_bb_reg(dm, 0x410c, 0x08000000, 0x0);
	odm_set_bb_reg(dm, 0x520c, 0x08000000, 0x0);
	odm_set_bb_reg(dm, 0x530c, 0x08000000, 0x0);
	odm_set_bb_reg(dm, 0x186c, 0x00000080, 0x0);
	odm_set_bb_reg(dm, 0x416c, 0x00000080, 0x0);
	odm_set_bb_reg(dm, 0x526c, 0x00000080, 0x0);
	odm_set_bb_reg(dm, 0x536c, 0x00000080, 0x0);
	odm_set_bb_reg(dm, 0x180c, 0x00000003, 0x3);
	odm_set_bb_reg(dm, 0x410c, 0x00000003, 0x3);
	odm_set_bb_reg(dm, 0x520c, 0x00000003, 0x3);
	odm_set_bb_reg(dm, 0x530c, 0x00000003, 0x3);
	odm_set_bb_reg(dm, 0x1a00, 0x00000003, 0x0);
	return;
}

boolean
_lok_check_8814c(struct dm_struct *dm, u8 path)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;
	boolean fail = true;
	u32 temp;
	u8 idac_i, idac_q;
	u8 i;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);	
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	temp = odm_get_rf_reg(dm, path, 0x58, 0xfffff);
	RF_DBG(dm, DBG_RF_IQK, "[IQK] result_0x58 = 0x%x\n", temp);
	idac_i = (u8)((temp & 0xfc000) >> 14);
	idac_q = (u8)((temp & 0x03f00) >> 8);
	RF_DBG(dm, DBG_RF_IQK, "[IQK] idac_i = 0x%x, idac_q = 0x%x\n", idac_i, idac_q);

	if(idac_i > 0x3a)
		return true;
	else if (idac_i > 0x5)
		fail = false;
	else
		return true;

	if(idac_q > 0x3a)
		return true;
	else if (idac_q > 0x5)
		fail = false;
	else
		return true;
			
	return fail;
}


boolean
_lok_one_shot_8814c(
	struct dm_struct *dm, u8 path)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	u8 delay_count = 0;
	boolean LOK_notready = false;
	u32 temp = 0;
	u32 IQK_CMD = 0x0;
	u8 idac_i, idac_q;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]======S%d LOK======\n", path);
	//IQK_CMD = (1 << (4 + path)) | (path << 1) | 0x8;	
	IQK_CMD = (1 << (4 + path)) | 0x8;
	halrf_delay_10us(1);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]LOK_Trigger = 0x%x\n", IQK_CMD + 1);
	
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD);
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD + 1);
	halrf_delay_10us(1);
	/*LOK: CMD ID = 0	{0xf8000018, 0xf8000028}*/
	/*LOK: CMD ID = 0	{0xf8000019, 0xf8000029}*/

	LOK_notready = _iqk_check_cal_8814c(dm, path, 0x0);

	iqk_info->rf_reg58 = odm_get_rf_reg(dm, path, 0x58, 0xfffff);

	if (!LOK_notready) {
		RF_DBG(dm, DBG_RF_IQK, "[IQK]0x58 = 0x%x\n",
		       odm_get_rf_reg(dm, path, RF_0x58, 0xfffff));
	} else
		RF_DBG(dm, DBG_RF_IQK, "[IQK]==>S%d LOK Fail!!!\n", path);
	iqk_info->lok_fail[path] = LOK_notready;
	return LOK_notready;
}

boolean
_iqk_one_shot_8814c(
	struct dm_struct *dm,
	u8 path,
	u8 idx)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;
	boolean notready = true, fail = true, is_NB_IQK = false;
	u32 IQK_CMD = 0x0;

	if (idx == TXIQK)
		RF_DBG(dm, DBG_RF_IQK, "[IQK]============ S%d WBTXIQK ============\n", path);
	else
		RF_DBG(dm, DBG_RF_IQK, "[IQK]============ S%d WBRXIQK STEP2============\n", path);

	if ((*dm->band_width == CHANNEL_WIDTH_5) ||(*dm->band_width == CHANNEL_WIDTH_10))
		is_NB_IQK = true;

	//set path
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);

	if (idx == TXIQK) {
		if (is_NB_IQK)
			IQK_CMD = (0x1 << 8) | (1 << (path + 4)) | 0x8;
		else {
			IQK_CMD = ((*dm->band_width + 4) << 8) | (1 << (path + 4)) | 0x8;
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]TXK_Trigger = 0x%x\n", IQK_CMD + 1);
		halrf_delay_10us(1);
		/*{0xf8000118, 0xf800012a} ==> NB TXK   (CMD = 1)*/
		/*{0xf8000418, 0xf800042a} ==> 20 WBTXK (CMD = 3)*/
		/*{0xf8000518, 0xf800052a} ==> 40 WBTXK (CMD = 4)*/
		/*{0xf8000618, 0xf800062a} ==> 80 WBTXK (CMD = 5)*/
	} else if (idx == RXIQK2) {
		if (is_NB_IQK)			
			IQK_CMD = (0x3 << 8) | (1 << (path + 4)) | 0x8;
		else {
			IQK_CMD = ((*dm->band_width + 0xa) << 8) | (1 << (path + 4)) | 0x8;
		}
		RF_DBG(dm, DBG_RF_IQK, "[IQK]RXK2_Trigger = 0x%x\n", IQK_CMD + 1);
		halrf_delay_10us(1);
		/*{0xf8000318, 0xf800031a} ==> NB RXK2   (CMD = 3)*/
		/*{0xf8000918, 0xf8000a1a} ==> 20 WBRXK2 (CMD = a)*/
		/*{0xf8000a18, 0xf8000b1a} ==> 40 WBRXK2 (CMD = b)*/
		/*{0xf8000b18, 0xf8000c1a} ==> 80 WBRXK2 (CMD = c)*/
	}
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD);
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD + 1);
	fail = _iqk_check_cal_8814c(dm, path, 0x1);
	if (idx == TXIQK) {		
		iqk_info->iqk_fail_report[0][path][TXIQK] = fail;
		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		odm_set_bb_reg(dm, 0x1bf0, 0x0000000f, fail << path);
	}
	if (idx == RXIQK2) {		
		iqk_info->iqk_fail_report[0][path][RXIQK] = fail;
		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		odm_set_bb_reg(dm, 0x1be8, 0x000fffff, odm_get_rf_reg(dm,path, RF_0x0, MASK20BITS));
		odm_set_bb_reg(dm, 0x1be8, 0x0ff00000, iqk_info->tmp1bcc);
		odm_set_bb_reg(dm, 0x1bf0, 0x000f0000, fail << (path + 16));
	}
	return fail;
}

//tune lna
void _iqk_rx_gain_search1_8814c(struct dm_struct *dm, u8 path)
{
	struct dm_iqk_info *iqk = &dm->IQK_info;
	boolean fail = true, isbnd = false;
	u32 IQK_CMD = 0x0, rf_reg0 = 0x0, new_rf_reg0 = 0x0;
	u32 lna = 0x0, rxbb = 0x0;
	u8 i = 0, idx = 0x0;
	u8 IQMUX[5] = {0x9, 0x12, 0x1b, 0x24, 0x24};

	RF_DBG(dm, DBG_RF_IQK, "[IQK]============ S%d WBRXIQK GS1============\n", path);

	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b18, 0x00000002, 0x1);
	odm_set_bb_reg(dm, 0x1b24, MASKDWORD, 0x00070408); //lna =1
	//odm_set_bb_reg(dm, 0x1b24, MASKDWORD, 0x00070808); //lna =2
	for(i = 0; i < 4; i++) {
		odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
		odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
		odm_set_bb_reg(dm, 0x1b2c, 0x0fff0000, 0x018);
		IQK_CMD = (0xf << 8) | (1 << (path + 4)) | 0x8;
		odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD);
		odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD + 1);
		
		halrf_delay_10us(1);
		fail = _iqk_check_cal_8814c(dm, path, 0x1);

		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		rf_reg0 = odm_get_rf_reg(dm, path, 0x0, MASK20BITS);		
		RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, IQK_CMD = 0x%x, old rf0 = 0x%x\n", i, IQK_CMD, rf_reg0);			
		lna  = (rf_reg0 & 0x01c00) >> 10; //rf0[12:10]
		rxbb = (rf_reg0 & 0x003e0) >> 5;  // rf0[9:5]
		RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, lna = 0x%x, rxbb = 0x%x\n", i, lna, rxbb);
	
		if (rxbb > 0x9) {
			lna++;
			isbnd = true;
		} else if (rxbb > 0x1) {
			isbnd = false;
		} else {
			//lna--;
			isbnd = true;
		}
#if 0
		if(!((lna < 8) && (lna > 1)))
			lna = 1;
#else
		if(lna < 1)
			lna = 1;
		if(lna > 7)
			lna = 7;
#endif
		if ((lna == 1) && (rxbb == 0x1)){
			idx++;	
			isbnd = true;
		}
	
		odm_set_rf_reg(dm, path, 0x0, 0x01c00,lna);
		ODM_delay_us(1);
		new_rf_reg0 = odm_get_rf_reg(dm, path, 0x0, MASK20BITS);
		
		if(isbnd){
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
			odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, IQMUX[idx]);
			odm_set_bb_reg(dm, 0x1b24, MASKDWORD, new_rf_reg0);
			odm_set_bb_reg(dm, 0x1b24, 0x00001c00, lna);
			odm_set_bb_reg(dm, 0x1b24, 0x000003e0, rxbb);
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, new 0x1b00 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b00, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, new 0x1b24 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, new 0x1bcc = 0x%x\n", i, odm_get_bb_reg(dm, 0x1bcc, MASKDWORD));			
		} else {
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);			
			odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, IQMUX[idx]);
			odm_set_bb_reg(dm, 0x1b24, MASKDWORD, new_rf_reg0);
			odm_set_bb_reg(dm, 0x1b24, 0x00001c00, lna);
			odm_set_bb_reg(dm, 0x1b24, 0x000003e0, rxbb);
			odm_set_bb_reg(dm, 0x1b18, 0x00000002, 0x0);			
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, 0x1b00 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b00, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, 0x1b24 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b24, MASKDWORD));			
			RF_DBG(dm, DBG_RF_IQK, "[IQK]GS %d, 0x1bcc = 0x%x\n", i, odm_get_bb_reg(dm, 0x1bcc, MASKDWORD));
			break;
		}
	}
	return;
}

// tune 1bcc
void _iqk_rx_gain_search2_8814c(struct dm_struct *dm, u8 path)
{
	struct dm_iqk_info *iqk = &dm->IQK_info;
	boolean isbnd = true, fail = false;
	u32 IQK_CMD = 0x0, rf_reg0 = 0x0, new_rf_reg0 = 0x0;
	u32 lna = 0x0, rxbb = 0x0;
	u8 i = 0, idx = 0x0;	
	u8 IQMUX[5] = {0x9, 0x12, 0x1b, 0x24, 0x24};

	RF_DBG(dm, DBG_RF_IQK, "[IQK]============ S%d WBRXIQK GS2============\n", path);

	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b18, 0x00000002, 0x1);
	odm_set_bb_reg(dm, 0x1b24, MASKDWORD, 0x00070408);
	odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
	odm_set_bb_reg(dm, 0x1b2c, 0x0fff0000, 0x018);
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]ini 0x1b24 = 0x%x\n", odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
	//RF_DBG(dm, DBG_RF_IQK, "[IQK]ini 0x1bcc = 0x%x\n", odm_get_bb_reg(dm, 0x1bcc, MASKDWORD));			

	idx = 0x0;

	for(i = 0; i< 2; i++) {
		IQK_CMD = (0xf << 8) | (1 << (path + 4)) | 0x8;
		odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD);
		odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD + 1);	
		halrf_delay_10us(1);
		fail = _iqk_check_cal_8814c(dm, path, 0x1);

		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		rf_reg0 = odm_get_rf_reg(dm, path, 0x0, MASK20BITS);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %x, IQK_CMD = %x, rf0 = 0x%x\n", i, IQK_CMD, rf_reg0);			
		lna  = (rf_reg0 & 0x01c00) >> 10; //rf0[12:10]
		rxbb = (rf_reg0 & 0x003e0) >> 5;  // rf0[9 :5]

		if (rxbb > 0x9) {
			lna++;
			isbnd = true;
		}else if (rxbb > 0x1) {
			isbnd = false;
		}
		else {
			idx++;
			isbnd = true;
		}
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, rxbb = 0x%x\n", i, rxbb);			
		//RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, lna  = 0x%x\n", i, lna);

		if(isbnd){			
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
			odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, IQMUX[idx]);			
			odm_set_bb_reg(dm, 0x1b24, 0x00001c00, lna);
			odm_set_bb_reg(dm, 0x1b24, 0x000003e0, rxbb);
			iqk->tmp1bcc = (u8) odm_get_bb_reg(dm, 0x1bcc,0x0000003f);			
			RF_DBG(dm, DBG_RF_IQK, "[IQK]isbound %d, new rf0 = 0x%x\n", i, rf_reg0);
			RF_DBG(dm, DBG_RF_IQK, "[IQK]isbound %d, new 0x1b00 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b00, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]isbound %d, new 0x1b24 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]isbound %d, new 0x1bcc = 0x%x\n", i, odm_get_bb_reg(dm, 0x1bcc, MASKDWORD));			
		} else {
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);			
			odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, IQMUX[idx]);			
			iqk->tmp1bcc = (u8) odm_get_bb_reg(dm, 0x1bcc,0x0000003f);
			rf_reg0 = odm_get_rf_reg(dm, path, 0x0, MASK20BITS);
			odm_set_bb_reg(dm, 0x1b24, MASKDWORD, rf_reg0);
			odm_set_bb_reg(dm, 0x1b24, 0x00001c00, lna);
			odm_set_bb_reg(dm, 0x1b24, 0x000003e0, rxbb);
			odm_set_bb_reg(dm, 0x1b18, 0x00000002, 0x0);			
			RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, rf0 = 0x%x\n", i, rf_reg0);
			RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, 0x1b00 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b00, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, 0x1b24 = 0x%x\n", i, odm_get_bb_reg(dm, 0x1b24, MASKDWORD));
			RF_DBG(dm, DBG_RF_IQK, "[IQK]gs_done %d, 0x1bcc = 0x%x\n", i, odm_get_bb_reg(dm, 0x1bcc, MASKDWORD));			
			break;
		}
	}
	return;
}

void _iqk_2g_rxk_iqk_8814c(struct dm_struct *dm, u8 path)
{
	boolean kfail = false;
	u32 tmp = 0x0;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x7);	
	//ODM_delay_us(1);
	odm_set_rf_reg(dm, path, 0x9e, 0x00020, 0x0);
	odm_set_rf_reg(dm, path, 0x9e, 0x00400, 0x0);
	odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x1);
	odm_set_rf_reg(dm, path, 0x56, 0x03fff, 0x2067);
	_iqk_rx_gain_search2_8814c(dm, path);
	//odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
	//odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	//odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
	kfail = _iqk_one_shot_8814c(dm,path,RXIQK2);
	odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x3); //rx mode
	odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x0);
#if 0
	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xde, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, 0xde = %x !!!\n", path, tmp);	
	}
#endif
	return;

}

void _iqk_5g_rxk_iqk_8814c(struct dm_struct *dm, u8 path)
{
	boolean kfail = false;
	u32 tmp = 0x0;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	//odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x7);	
	//ODM_delay_us(1);
	odm_set_rf_reg(dm, path, 0x9e, MASK20BITS, 0xd8009);
	odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x1);
	odm_set_rf_reg(dm, path, 0x56, 0x00fff, 0x248);
	_iqk_rx_gain_search2_8814c(dm, path);
	//odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
	//odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	//odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
	kfail = _iqk_one_shot_8814c(dm,path,RXIQK2);
	odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x3); //rx mode
	odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x0);
#if 0
	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xde, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_IQK, "[IQK] S%x, 0xde = %x !!!\n", path, tmp);	
	}
#endif
	return;

}

void _iqk_2g_txk_iqk_8814c(struct dm_struct *dm, u8 path)
{
	boolean kfail = false;
	u8 i = 0;
	u32 tmp = 0;


	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);

	for(i = 0 ;i < 2; i++) {
		odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x1);
		odm_set_rf_reg(dm, path, 0x56, 0x03fff, 0x2667);
		odm_set_rf_reg(dm, path, 0x57, 0x08000, 0x1);
		odm_set_rf_reg(dm, path, 0x57, 0x00e00, 0x2 + i);
		odm_set_rf_reg(dm, path, 0xef, 0x00010, 0x1);
		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		odm_set_bb_reg(dm, 0x1b28, 0x80000000, 0x0);
		odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
		odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
		odm_set_bb_reg(dm, 0x1b2c, 0x00000fff, 0x038);
		kfail = _lok_one_shot_8814c(dm, path);
		kfail = _lok_check_8814c(dm, path);
		if(!kfail)
			break;
	}
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
	kfail = _iqk_one_shot_8814c(dm, path, TXIQK);
	odm_set_rf_reg(dm, path, 0xef, 0x00010, 0x0);
#if 0
	tmp = odm_get_rf_reg(dm, path, 0xef, MASK20BITS);
	if(tmp != 0)
		RF_DBG(dm, DBG_RF_IQK, "[IQK] Error S%x, 0xef = %x !!!\n", path, tmp);	
#endif
	return;

}


void _iqk_5g_txk_iqk_8814c(struct dm_struct *dm, u8 path)
{
	boolean kfail = false;
	u8 i = 0;
	u32 tmp = 0;
	
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	
	for(i = 0 ;i < 2; i++) {
		odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x1);
		odm_set_rf_reg(dm, path, 0x56, 0x00fff, 0xee8);
		odm_set_rf_reg(dm, path, 0x57, 0x08000, 0x1);
		odm_set_rf_reg(dm, path, 0x57, 0x70000, 0x2 + i);
		odm_set_rf_reg(dm, path, 0xef, 0x00010, 0x1);
		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		odm_set_bb_reg(dm, 0x1b28, 0x80000000, 0x1);
		odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x00);
		odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
		odm_set_bb_reg(dm, 0x1b2c, 0x00000fff, 0x038);		
		kfail = _lok_one_shot_8814c(dm, path);
		kfail = _lok_check_8814c(dm, path);
		if(!kfail)
			break;
	}
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1bcc, 0x0000003f, 0x09);
	kfail = _iqk_one_shot_8814c(dm, path, TXIQK);
	odm_set_rf_reg(dm, path, 0xef, 0x00010, 0x0);
#if 0	
	tmp = odm_get_rf_reg(dm, path, 0xef, MASK20BITS);
	if(tmp != 0)
		RF_DBG(dm, DBG_RF_IQK, "[IQK] Error S%x, 0xef = %x !!!\n", path, tmp);	
#endif
	return;

}

void _iqk_iqk_by_path_8814c(
	void *dm_void,
	boolean segment_iqk)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	boolean KFAIL = true, is_NB_IQK = false;
	u32 counter = 0x0;
	u8 path = 0x0;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	
	for(path = 0x0; path < SS_8814C; path++) {
		odm_set_rf_reg(dm, path, 0x8a, 0x000ff, 0xf7);		
		RF_DBG(dm, DBG_RF_IQK, "[IQK]0x8a = 0xf7\n");
		if (*dm->band_type == ODM_BAND_2_4G) {			
			_iqk_2g_txk_iqk_8814c(dm, path);
			_iqk_2g_rxk_iqk_8814c(dm, path);
		} else {		
			_iqk_5g_txk_iqk_8814c(dm, path);			
			_iqk_5g_rxk_iqk_8814c(dm, path);
		}
	}
	//return to S0
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, RF_PATH_A);
}

void _iqk_start_iqk_8814c(
	struct dm_struct *dm,
	boolean segment_iqk)
{
	RF_DBG(dm, DBG_RF_IQK, "[IQK]===>%s\n", __func__);
	_iqk_iqk_by_path_8814c(dm, segment_iqk);
	return;		
}

void _iq_calibrate_8814c_init(
	struct dm_struct *dm)
{
	static boolean firstrun = true;

	if (firstrun) {		
		firstrun = false;
		odm_read_and_config_mp_8814c_cal_init(dm);
	}
#if 0
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 i, j, k, m;
	static boolean firstrun = true;

	if (firstrun) {
		firstrun = false;
		RF_DBG(dm, DBG_RF_IQK, "[IQK]=====>PHY_IQCalibrate_8814c_Init\n");

		for (i = 0; i < SS_8814C; i++) {
			for (j = 0; j < 2; j++) {
				iqk_info->lok_fail[i] = true;
				iqk_info->iqk_fail[j][i] = true;
				iqk_info->iqc_matrix[j][i] = 0x20000000;
			}
		}

		for (i = 0; i < 2; i++) {
			iqk_info->iqk_channel[i] = 0x0;
			for (j = 0; j < SS_8814C; j++) {
				iqk_info->lok_idac[i][j] = 0x0;
				iqk_info->rxiqk_agc[i][j] = 0x0;
				iqk_info->bypass_iqk[i][j] = 0x0;

				for (k = 0; k < 2; k++) {
					iqk_info->iqk_fail_report[i][j][k] = true;
					for (m = 0; m <= 16; m++) {
						iqk_info->iqk_cfir_real[i][j][k][m] = 0x0;
						iqk_info->iqk_cfir_imag[i][j][k][m] = 0x0;
					}
				}

				for (k = 0; k < 3; k++)
					iqk_info->retry_count[i][j][k] = 0x0;
			}
		}
	}
#endif
	return;
}

void _phy_iq_calibrate_8814c(
	struct dm_struct *dm,
	boolean reset,
	boolean segment_iqk)
{
	u32 mac_backup[MAC_REG_NUM_8814C];
	u32 bb_backup[BB_REG_NUM_8814C];
	u32 rf_backup[RF_REG_NUM_8814C][SS_8814C];
	u32 backup_mac_reg[MAC_REG_NUM_8814C] = {0x520};
	u32 backup_bb_reg[BB_REG_NUM_8814C] = {0x1c38, 0x1d58, 0x1d60, 0x180c, 0x410c,
				       0x520c, 0x530c, 0x1d70, 0x1a00, 0x824,
				       0x1a14, 0x1830, 0x4130, 0x5230, 0x5330,
				       0x1860, 0x4160, 0x5260, 0x5360, 0x183c,
				       0x1840, 0x1844, 0x413c, 0x4140, 0x4144,
				       0x523c, 0x5240, 0x5244, 0x533c, 0x5340,
				       0x5344};
	u32 backup_rf_reg[RF_REG_NUM_8814C] = {0x0, 0x8a};
	boolean is_mp = false;
	u8 i = 0;

	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (*dm->mp_mode)
		is_mp = true;
	else
		is_mp = false;
	iqk_info->rf_reg18 = odm_get_rf_reg(dm, RF_PATH_A, RF_0x18, MASK20BITS);

	RF_DBG(dm, DBG_RF_IQK, "[IQK]==========IQK start!!!!!==========\n");
	RF_DBG(dm, DBG_RF_IQK, "[IQK]band_type = %s, band_width = %d, ExtPA2G = %d, ext_pa_5g = %d\n", (*dm->band_type == ODM_BAND_5G) ? "5G" : "2G", *dm->band_width, dm->ext_pa, dm->ext_pa_5g);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]Interface = %d, Cv = %x\n", dm->support_interface, dm->cut_version);
	RF_DBG(dm, DBG_RF_IQK, "[IQK]Test V4 \n");
	iqk_info->iqk_times++;
	iqk_info->kcount = 0;
	iqk_info->iqk_step = 0;
	iqk_info->rxiqk_step = 0;

	_iqk_information_8814c(dm);
	_iqk_backup_mac_bb_8814c(dm, mac_backup, bb_backup, backup_mac_reg, backup_bb_reg);
	_iqk_backup_rf_8814c(dm, rf_backup, backup_rf_reg);

	//for(i =0; i <SS_8814C; i++)
	//	_iqk_dump_reg_8814c(dm, i);
	_iqk_set_macbbafe_8814c(dm);
	_iqk_start_iqk_8814c(dm, segment_iqk);	
	_iqk_reload_macbbafe_8814c(dm);
	//for(i =0; i <SS_8814C; i++)
	//	_iqk_dump_reg_8814c(dm, i);

	_iqk_restore_rf_8814c(dm, backup_rf_reg, rf_backup);
	_iqk_restore_mac_bb_8814c(dm, mac_backup, bb_backup, backup_mac_reg, backup_bb_reg);

	iqk_info->kcount = 0;
	RF_DBG(dm, DBG_RF_IQK, "[IQK]==========IQK end!!!!!==========\n");
}

void phy_iq_calibrate_8814c(
	void *dm_void,
	boolean clear,
	boolean segment_iqk)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!(rf->rf_supportability & HAL_RF_IQK))
		return;

	dm->rf_calibrate_info.is_iqk_in_progress = true;

	_iq_calibrate_8814c_init(dm);
	_phy_iq_calibrate_8814c(dm, clear, segment_iqk);

	dm->rf_calibrate_info.is_iqk_in_progress = false;

}

void iqk_reload_iqk_8814c(void *dm_void, boolean reset)
{
	return;
}

void phy_get_iqk_cfir_8814c(void *dm_void, u8 idx, u8 path, boolean debug)
{
	return;
}

void phy_iqk_dbg_cfir_backup_8814c(void *dm_void)
{
	return;
}

void phy_iqk_dbg_cfir_backup_update_8814c(void *dm_void)
{
	return;
}

void phy_iqk_dbg_cfir_reload_8814c(void *dm_void)
{
	return;
}

void phy_iqk_dbg_cfir_write_8814c(void *dm_void, u8 type, u32 path, u32 idx,
			      u32 i, u32 data)
{
	return;
}

void phy_iqk_dbg_cfir_backup_show_8814c(void *dm_void)
{
	return;
}

#endif
