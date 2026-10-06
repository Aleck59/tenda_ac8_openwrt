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

/*8814C DPK ver:0x5 20210126*/


void _dpk_lut_sram_write_8814c(void *dm_void, u8 path)
{
	return;
}

void _dpk_bp_macbb_8814c(struct dm_struct *dm, u32 *mac_bp,
			 u32 *bb_bp, u32 *bp_mac_reg, u32 *bp_bb_reg)
{
	u32 i;

	for (i = 0; i < DPK_MAC_NUM_8814C; i++) {
		mac_bp[i] = odm_read_4byte(dm, bp_mac_reg[i]);
		//RF_DBG(dm, DBG_RF_DPK, "[DPK] bp mac 0x%x = 0x%x\n", bp_mac_reg[i], mac_bp[i]);
	}
	for (i = 0; i < DPK_BB_NUM_8814C; i++) {
		bb_bp[i] = odm_read_4byte(dm, bp_bb_reg[i]);		
		//RF_DBG(dm, DBG_RF_DPK, "[DPK] bp bb 0x%x = 0x%x\n", bp_bb_reg[i], bb_bp[i]);
	}
}

void _dpk_bp_rf_8814c(struct dm_struct *dm, u32 rf_bp[][SS_8814C],
		      u32 *bp_reg)
{
	u8 i, path;

	for (path = 0; path < SS_8814C; path++) {
		for (i = 0; i < DPK_RF_NUM_8814C; i++) {
			rf_bp[i][path] = odm_get_rf_reg(dm, path, bp_reg[i], MASK20BITS);
			//RF_DBG(dm, DBG_RF_DPK, "[DPK] bp 0x%x = 0x%x\n", bp_reg[i], rf_bp[i][path]);
		}
	}
}


void _dpk_reload_macbb_8814c(struct dm_struct *dm, u32 *mac_bp,
			     u32 *bb_bp, u32 *bp_mac_reg, u32 *bp_bb_reg)
{
	u32 i;

	/*toggle IGI*/
	odm_write_4byte(dm, 0x1d70, 0x50505050);

	for (i = 0; i < DPK_MAC_NUM_8814C; i++) {
		odm_write_4byte(dm, bp_mac_reg[i], mac_bp[i]);		
		//RF_DBG(dm, DBG_RF_DPK, "[DPK] restore mac 0x%x = 0x%x\n", bp_mac_reg[i], mac_bp[i]);
	}

	for (i = 0; i < DPK_BB_NUM_8814C; i++) {
		odm_write_4byte(dm, bp_bb_reg[i], bb_bp[i]);		
		//RF_DBG(dm, DBG_RF_DPK, "[DPK] restore bb 0x%x = 0x%x\n", bp_bb_reg[i], bb_bp[i]);
	}
}
void _dpk_reload_rf_8814c(struct dm_struct *dm, u32 *reg,
			   u32 data[][SS_8814C])
{
	u32 i = 0x0;
	u8 path = 0x0;
	u32 tmp = 0x0;

	for (path = 0; path < SS_8814C; path++)
		for (i = 0; i < DPK_RF_NUM_8814C; i++) {
			//RF_DBG(dm, DBG_RF_DPK, "[DPK] reg 0x%x = 0x%x\n", reg[i], data[i][0]);
			odm_set_rf_reg(dm, path, reg[i], 0xfffff, data[i][path]);
	}
	odm_set_rf_reg(dm, RF_PATH_A, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_B, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_C, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_D, 0xde, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_A, 0xdf, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_B, 0xdf, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_C, 0xdf, MASK20BITS, 0x0);
	odm_set_rf_reg(dm, RF_PATH_D, 0xdf, MASK20BITS, 0x0);
		
	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xde, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, 0xde = %x !!!\n", path, tmp);	
	}
			
	for(path = 0; path < SS_8814C; path++) {
		tmp = odm_get_rf_reg(dm, path, 0xdf, MASK20BITS);
		if(tmp != 0)
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, 0xdf = %x !!!\n", path, tmp);	
	}

}

void _dpk_clear_lut_table_8814c(struct dm_struct *dm)
{
	u8 i;
	u32 reg_1bdc;
	RF_DBG(dm, DBG_RF_DPK, "[DPK] clear lut table\n");

	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C |  RF_PATH_A << 1);	
	odm_write_1byte(dm, 0x1b54, 0x0);
	for (i = 0; i < 0x80; i++) {
		odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00B88800 | i << 24);		
		odm_write_4byte(dm, 0x1b5c, 0x0);
		/*RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bdc = 0x%x\n", reg_1bdc);*/
	}

	
	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C |  RF_PATH_B << 1);	
	odm_write_1byte(dm, 0x1b54, 0x0);
	for (i = 0; i < 0x80; i++) {
		odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00B88800 | i << 24);		
		odm_write_4byte(dm, 0x1b5c, 0x0);
		/*RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bdc = 0x%x\n", reg_1bdc);*/
	}

	
	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C |  RF_PATH_C << 1);	
	odm_write_1byte(dm, 0x1b54, 0x0);
	for (i = 0; i < 0x80; i++) {
		odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00B88800 | i << 24);		
		odm_write_4byte(dm, 0x1b5c, 0x0);
		/*RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bdc = 0x%x\n", reg_1bdc);*/
	}

	
	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C |  RF_PATH_D << 1);	
	odm_write_1byte(dm, 0x1b54, 0x0);
	for (i = 0; i < 0x80; i++) {
		odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00B88800 | i << 24);		
		odm_write_4byte(dm, 0x1b5c, 0x0);
		/*RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bdc = 0x%x\n", reg_1bdc);*/
	}
	odm_write_4byte(dm, 0x1b58, 0x0);	
	odm_write_4byte(dm, 0x1b5c, 0x0);
}

void _dpk_pas_gl_read_8814c(struct dm_struct *dm, u8 path)
{
	u8 k, j;
	u32 reg_1bfc, reg_1b4c, tmp;

	RF_DBG(dm, DBG_RF_DPK, "\n");
	RF_DBG(dm, DBG_RF_DPK, "[DPK]_dpk_PAS_GL_read_8814c\n");	
	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C | 0x8);
	odm_write_1byte(dm, 0x1b49, 0x00);
	odm_write_4byte(dm, 0x1bd4, 0x00060001);		

	RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1b00 = 0x%x, 0x1bd4 = 0x%x\n",
	       odm_get_bb_reg(dm, 0x1b00, MASKDWORD),
	       odm_get_bb_reg(dm, 0x1bd4, MASKDWORD));
	for (k = 0; k < 8; k++) {
		//1st
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x00080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
		}
	
	for (k = 0; k < 8; k++) {
		//2nd		
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x08080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
		//3rd
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x10080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
		//4th
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x18080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
		//5th
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x20080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
		//6th
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x28080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
		//7th
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x30080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}	
	for (k = 0; k < 8; k++) {
	//8th
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, (0x38080000 | k << 24));
		reg_1bfc = odm_read_4byte(dm, 0x1bfc);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 1bfc = 0x%x\n", reg_1bfc);
	}
	RF_DBG(dm, DBG_RF_DPK, "\n");	
	odm_write_4byte(dm, 0x1b4c, 0x00000000);
}

void _dpk_set_macbbafe_8814c(
	struct dm_struct *dm)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;	

	RF_DBG(dm, DBG_RF_DPK, "[IQK]===>%s\n", __func__);
	/*MACBB register setting*/
	odm_write_1byte(dm, REG_TXPAUSE, 0xff);
//01_8814C_AFE_on_BB_Setting_IQK_reg
	odm_set_bb_reg(dm, 0x1b00, 0x0000000f, 0x8);
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

void _dpk_enable_disable_8814c(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C | path << 1);
	if (dpk_info->is_dpk_enable) {
		odm_set_bb_reg(dm, 0x1b54, BIT(0), 0x1);
		odm_set_bb_reg(dm, 0x1b20, BIT(25), 0x0);	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPK enable\n", path);
	} else {
		odm_set_bb_reg(dm, 0x1b54, BIT(0), 0x0);			
		odm_set_bb_reg(dm, 0x1b20, BIT(25), 0x1);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPK bypass\n", path);
	}
}

boolean _dpk_check_cal_8814c( struct dm_struct *dm, u8 path, u8 cmd)
{
	boolean notready = true, fail = true;
	u32 delay_count = 0x0;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] ===>%s\n", __func__);
	while (notready) {
		if (odm_read_1byte(dm, 0x2d9c) == 0x55) {
			if (cmd == IQK_CMD_GL) /*GL*/
				fail = false;
			else 
				fail = (boolean)odm_get_bb_reg(dm, 0x1b08, BIT(26));
			notready = false;
		} else {
			ODM_delay_us(1);
			delay_count++;
		}

		if (delay_count >= 5000) {
			fail = true;
			RF_DBG(dm, DBG_RF_DPK, "[DPK]DPK timeout!!!\n");
			break;
		}
	}	
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b10, 0x000000ff, 0x0);
	halrf_delay_10us(1);
#if 0
	if(!fail)
		odm_set_bb_reg(dm, 0x1b20, BIT(25), 0x0);	
	else
		odm_set_bb_reg(dm, 0x1b20, BIT(25), 0x1);
#endif

	RF_DBG(dm, DBG_RF_DPK, "[DPK]delay count = %d!!!\n", delay_count);	
	RF_DBG(dm, DBG_RF_DPK, "[DPK]0x1b08 = 0x%x!!!\n", odm_get_bb_reg(dm, 0x1b08, BIT(26)));

	return fail;
}


boolean
_dpk_deb_log_8814c(struct dm_struct *dm)
{
	u8 path = 0x0;
	u8 i;

#if 1
	for (path = 0x3; path < SS_8814C; path++) {
		odm_write_4byte(dm, 0x1b00, IQK_CMD_8814C | path << 1);
		for (i = 0; i <  0x100/4; i++)
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d 1b%x = 0x%x\n", path, i*4, odm_read_4byte(dm, (0x1b00 + i*4)));
		for (i = 0; i <  0xfe; i++)
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d RF%x = 0x%x\n", path, i, odm_get_rf_reg(dm, (enum rf_path)path, i, 0xfffff));
	}
#endif
	return true;
}

void _dpk_lut_sram_read_8814c(struct dm_struct *dm, u8 path)
{
	u8 i;
	u32 reg_1bfc, tmp;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, ===>%s\n", path,__func__);

	// fix path 0
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b08, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x1b58, 0x00000008, 0x0);
	odm_set_bb_reg(dm, 0x1b54, 0x00000001, 0x0);
	odm_set_bb_reg(dm, 0x1bd4, MASKDWORD, 0x00260001);
	odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00388800);
	RF_DBG(dm, DBG_RF_DPK, "\n");

	for (i = 0; i< 0x40; i++){
		odm_set_bb_reg(dm, 0x1b58, 0xff000000, i);
		ODM_delay_us(2);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bfc = 0x%x\n", odm_get_bb_reg(dm, 0x1bfc, MASKDWORD));
	}
	odm_set_bb_reg(dm, 0x1b58, MASKDWORD, 0x00000000);
	
	RF_DBG(dm, DBG_RF_DPK, "\n");
	return;
}


void _dpk_log_pas_8814c(struct dm_struct *dm, u8 path)
{
	u8 i;
	u32 reg_1bfc, tmp;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, ===>%s\n", path,__func__);

	// fix path 0
	odm_set_bb_reg(dm, 0x1b00, 0x0000000f, IQK_CMD_8814C);
	odm_set_bb_reg(dm, 0x1b48, 0x0000ff00, 0x00);
	odm_set_bb_reg(dm, 0x1bd4, MASKDWORD, 0x00060001);
	odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, 0x00080000);
	RF_DBG(dm, DBG_RF_DPK, "\n");

	for (i = 0; i< 0x40; i++){
		odm_set_bb_reg(dm, 0x1b4c, 0xff000000, i);
		ODM_delay_us(2);
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x1bfc = 0x%x\n", odm_get_bb_reg(dm, 0x1bfc, MASKDWORD));
	}
	odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, 0x00000000);
	
	RF_DBG(dm, DBG_RF_DPK, "\n");
}

boolean _dpk_one_shot_8814c(struct dm_struct *dm, u8 path, u8 cmd_id)
{
	boolean fail = false;
	u32 IQK_CMD = 0x0;
	
	IQK_CMD = 0x1000 | (cmd_id << 8) | (1 << (path + 4)) | ((path << 1) | 0x8);
	
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, a rf0 = 0x%x\n", path, odm_get_rf_reg(dm, path, 0x00, 0xfffff));
	RF_DBG(dm, DBG_RF_DPK, "[DPK] one-shot IQK_CMD = 0x%x\n", IQK_CMD + 1);
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD);
	odm_set_bb_reg(dm, 0x1b00, MASKDWORD, IQK_CMD + 1);
	ODM_delay_us(10);
	fail = _dpk_check_cal_8814c(dm, path, cmd_id);		
	return fail;
}


u8 _dpk_thermal_read_8814c(
	void *dm_void,
	u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u8 tmp = 0x0;

	odm_set_rf_reg(dm, path, 0x42, BIT(17), 0x1);
	odm_set_rf_reg(dm, path, 0x42, BIT(17), 0x0);
	odm_set_rf_reg(dm, path, 0x42, BIT(17), 0x1);
	ODM_delay_us(100);

	tmp = (u8)odm_get_rf_reg(dm, path, 0x42, 0x0fc00);
	//RF_DBG(dm, DBG_RF_DPK, "[DPK] thermal dbg S%d = %d\n", path, tmp);
	return tmp;
}

void _dpk_restore_func_8814c(struct dm_struct *dm) {

	u8 path = 0;

	//11_8814C_restore_AFE_BB_RF_KIP_Settings_20210105
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


void _dpk_on_func_8814c(struct dm_struct *dm, u8 path, boolean kfail) {

	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, ===>%s\n", path,__func__);
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
	odm_set_bb_reg(dm, 0x1b58, 0x00000080, 0x1);
	odm_set_bb_reg(dm, 0x1b54, MASKDWORD, 0x80000001);
	if (kfail == false) {
		odm_set_bb_reg(dm, 0x1b20, 0x03000000, 0x1);		
		odm_set_bb_reg(dm, 0x1b54, 0x00000001, 0x1);
		RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, DPD ON\n", path);
	} else {
		odm_set_bb_reg(dm, 0x1b20, 0x03000000, 0x3);		
		odm_set_bb_reg(dm, 0x1b54, 0x00000001, 0x0);		
		RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, DPD OFF\n", path);
	}
	odm_set_bb_reg(dm, 0x1b6c, 0x03ff0000, 0x0a1);

	if (*dm->band_width == CHANNEL_WIDTH_20)
		dpk_info->thr_pwsf[path] = 0x62; // 0db
	else if(*dm->band_width == CHANNEL_WIDTH_40)	
		dpk_info->thr_pwsf[path] = 0x62 - 0x4; // 1db 
	else if(*dm->band_width == CHANNEL_WIDTH_80)
		dpk_info->thr_pwsf[path] = 0x62 - 0x6; //1 // 1.5db 
	else
		dpk_info->thr_pwsf[path] = 0x62; // 0db

	odm_set_bb_reg(dm, 0x1b64, 0xff000000, dpk_info->thr_pwsf[path]);
	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, 0x1b64[31:24] = 0x%x\n", path, dpk_info->thr_pwsf[path]);

	//turn off rf debug mode
	odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x3); //rx mode
	odm_set_rf_reg(dm, path, 0xde, 0x10000, 0x0);
	odm_set_rf_reg(dm, path, 0xdf, 0x10000, 0x0);

	return;
}

u32 _dpk_func_pas_iqval_8814c(
	struct dm_struct *dm,
	u8 iq_idx)
{
	u32 i_val = 0, q_val = 0;

	// fix path 0
	odm_set_bb_reg(dm, 0x1b00, 0x0000000f, IQK_CMD_8814C);
	odm_set_bb_reg(dm, 0x1b48, 0x00004000, 0x0);	
	odm_set_bb_reg(dm, 0x1bd4, MASKDWORD, 0x00060001);	
	odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, 0x00080000 | iq_idx << 24);
	ODM_delay_us(10);	
	i_val = odm_get_bb_reg(dm, 0x1bfc, MASKHWORD);
	q_val = odm_get_bb_reg(dm, 0x1bfc, MASKLWORD);

	if (i_val >> 15 != 0)
		i_val = 0x10000 - i_val;
	if (q_val >> 15 != 0)
		q_val = 0x10000 - q_val;
	
	odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, 0x00000000);
#if 1
	RF_DBG(dm, DBG_RF_DPK, "[DPK][%x] (2) i=%d, q=%d, i^2+q^2=%d\n",
	       iq_idx, i_val, q_val, i_val*i_val + q_val*q_val);
#endif
	return i_val*i_val + q_val*q_val;
}

s32 _dpk_pas_gain_db_8814c(struct dm_struct *dm, u8 path, u8 iq_idx)
{
	u8 result = 0;
	u32 i_val = 0, q_val = 0, loss = 0, gain = 0;
	s32 loss_db = 0;
	s32 gain_db = 0;
	s32 gain_db2 = 0;
	u8 i = 0;

	gain = _dpk_func_pas_iqval_8814c(dm, iq_idx) >> 10; //offset 90db
	gain = gain & 0x003fffff;
	gain_db2 = halrf_psd_log2base(gain);
	gain_db =  (s32)(3 *(halrf_psd_log2base(gain)/10 - 280));

#if 1
	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x i^2+q^2= %d\n", path, gain);
	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x log(gain)= %d\n", path, gain_db2);	
	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x Gain_db= %d\n", path, gain_db);
#endif
#if 0	
	RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x Test Case 1 offset = 13\n");
	for(i = 0; i < 17; i++) {
		gain = (0x1 << (i+13)) >> 13;
		gain = gain & 0x0007ffff;
		gain_db2 = halrf_psd_log2base(gain);
		gain_db =  (s32)(3 *(halrf_psd_log2base(gain)/10 - 280));
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x i^2+q^2= %d\n", path, gain);
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x log(gain)= %d\n", path, gain_db2); 
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x Gain_db= %d\n", path, gain_db);
	}
	RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]\n");
	RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x Test Case 2 offset = 10\n");
	for(i = 0; i < 20; i++) {
		gain = (0x1 << (i+10)) >> 10;		
		gain = gain & 0x003fffff;
		gain_db2 = halrf_psd_log2base(gain);
		gain_db =  (s32)(3 *(halrf_psd_log2base(gain)/10 - 280));
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x i^2+q^2= %d\n", path, gain);
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x log(gain)= %d\n", path, gain_db2); 
		RF_DBG(dm, DBG_RF_DPK, "[DPK][GL]S%x Gain_db= %d\n", path, gain_db);
	}
#endif
	return gain_db;

}

boolean _dpk_pas_tune_rxtagc_8814c(struct dm_struct *dm, u8 path)
{
	u8 tmp_rxbb = 0x0;
	s32 gain_db;
	u32 rf0;
	boolean fail = false;

	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, ===>%s\n", path,__func__);
	
	gain_db = _dpk_pas_gain_db_8814c(dm, path, 0x1);		
	tmp_rxbb = (u8)odm_get_rf_reg(dm,path,0x00, 0x003e0);
	rf0 = odm_get_rf_reg(dm, path, 0x00, 0xfffff);
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, old rf0 = 0x%x\n", path, odm_get_rf_reg(dm, path, 0x00, 0xfffff));		

	if(tmp_rxbb < 0x00 + 3)
		tmp_rxbb = 3;
	if(tmp_rxbb > 0x1f - 3)
		tmp_rxbb = 0x1c;
	
	if (gain_db < -80-300) {// <-80
		tmp_rxbb = tmp_rxbb + 3;
		fail = true;
	} else if (gain_db < -20-300) {// -20 ~-80dB
		tmp_rxbb = tmp_rxbb + 0;
		fail = false;
	} else {// > -20dB
		tmp_rxbb = tmp_rxbb - 3;
		fail = true;
	}
	odm_set_rf_reg(dm, path, 0x00, 0x003e0, tmp_rxbb);
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, new rf0 = 0x%x\n", path, odm_get_rf_reg(dm, path, 0x00, 0xfffff));		

	return fail;

}

boolean _dpk_func_8814c(struct dm_struct *dm, u8 path, u8 backoff) {
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u32 rf0 = 0x0, rxbb_ori = 0x0, i = 0x0;
	boolean fail = true, kfail = true;
	u8 new_txbb = 0x0, txbb = 0x0, tmp = 0, new_rxbb = 0x0, rxbb = 0x0;

//04_DPK	
	RF_DBG(dm, DBG_RF_DPK, "[DPK]S%x, ===>%s\n", path,__func__);
	txbb = (u8) odm_get_rf_reg(dm, path, 0x00, 0x0001f);
	rxbb = (u8) odm_get_rf_reg(dm, path, 0x00, 0x003e0);
	tmp = (u8)odm_get_bb_reg(dm, 0x1b60, 0x001f0000);
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, old txbb = 0x%x, rxbb = 0x%x\n", path, txbb, rxbb);		
		
	if (*dm->band_type == ODM_BAND_2_4G) {
		new_txbb = txbb - backoff;		
		new_rxbb = rxbb + (backoff / 2);
	} else {
		new_txbb = txbb - backoff;		
		new_rxbb = rxbb + (backoff / 2);
	}
	if(new_txbb < tmp)
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x,new_TxAGC < 0x1b60[20:16] happen  !!!\n", path);		


	for(i = 0; i < 2; i++) {
		odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x5);
		odm_set_rf_reg(dm, path, 0x00, 0x0001f, new_txbb);
		odm_set_rf_reg(dm, path, 0x00, 0x003e0, new_rxbb);
		ODM_delay_us(1);
		odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
		odm_set_bb_reg(dm, 0x1b64, 0x0000ff00, new_txbb);
		odm_set_bb_reg(dm, 0x1b68, 0x01ff0000, 0x1ff);
		odm_set_bb_reg(dm, 0x1b48, 0x0000ff00, 0x00);
		odm_set_bb_reg(dm, 0x1b20, 0x03000000, 0x0);
		odm_set_bb_reg(dm, 0x1b58, 0x000000ff, 0x8b);
		odm_set_bb_reg(dm, 0x1b40, 0xffff0000, 0x0200);

		kfail = _dpk_one_shot_8814c(dm, path, IQK_CMD_DPD);
		//fail = _dpk_pas_tune_rxtagc_8814c(dm, path);		
		fail = false;
		if (!fail)
			break;
	}
	dpk_info->thermal_dpk[path] = _dpk_thermal_read_8814c(dm, path);

	for(i = 0; i < AVG_THERMAL_NUM_DPK; i++) {
		dpk_info->thermal_dpk_avg[path][i] = dpk_info->thermal_dpk[path];		
		//RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, dpk_info->thermal_dpk_avg[%d][%x] = %d\n", path, path, i, dpk_info->thermal_dpk_avg[path][i]);		
	}

	odm_set_bb_reg(dm, 0x1b48, 0x0000ff00, 0x00);
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, new rf0 = 0x%x\n", path, odm_get_rf_reg(dm, path, 0x00, 0xfffff));		
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, new txbb = 0x%x, new rxbb = 0x%x\n", path, new_txbb, new_rxbb);		
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, new 0x1b64[15:8] = 0x%x\n", path, odm_get_bb_reg(dm, 0x1b64, 0x0000ff00));		
	return kfail;
}


u8 _dpk_gainloss_func_8814c(struct dm_struct *dm, u8 path) {
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u32 rf0;
	boolean fail = true, kfail = true;
	u8 backoff = 0x0;
	u32 iq_val = 0x0, txbb = 0;
	u8 i = 0x0, k = 0;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, ===>%s\n", path,__func__);

	//03_Gainloss_func
	for(k = 0; k < 2; k++) {
		for(i = 0; i < 2; i++) {
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
			odm_set_bb_reg(dm, 0x1b48, 0x0000ff00, 0x00);
			odm_set_bb_reg(dm, 0x1b6c, MASKDWORD, 0x00800080);
			odm_set_bb_reg(dm, 0x1b20, 0x03000000, 0x0);
			kfail = _dpk_one_shot_8814c(dm, path, IQK_CMD_GL);
			ODM_delay_us(10);
			fail = _dpk_pas_tune_rxtagc_8814c(dm, path);
			if (!fail)
				break; //break i
		}
		odm_set_bb_reg(dm, 0x1b00, 0x0000000f, 0x8);
		odm_set_bb_reg(dm, 0x1bd4, MASKDWORD, 0x00060001);
		odm_set_bb_reg(dm, 0x1b4c, MASKDWORD, 0x00000000);
		odm_set_bb_reg(dm, 0x1b48, 0x00004000, 0x1);
		backoff = (u8) odm_get_bb_reg(dm, 0x1bfc, 0x000000f0);
		odm_set_bb_reg(dm, 0x1b48, 0x00004000, 0x0);
		if (backoff == 0) {			
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, backoff = 0\n", path);	
			txbb = odm_get_rf_reg(dm, path, 0x0, 0x0001f);
			odm_set_rf_reg(dm, path, 0x0, 0x0001f, txbb + 2);
		} else
			break; // break k
	}
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, backoff = 0x%x\n", path, backoff);	
	RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, rf0 = 0x%x\n", path, odm_get_rf_reg(dm, path, 0x0, MASK20BITS));
	return backoff;
}

void _dpk_rfsetting_8814c(struct dm_struct *dm, u8 path)
{
	struct dm_iqk_info	*iqk_info = &dm->IQK_info;
	u32 rf_reg00 = 0x0l, rf0;
	u8 tx_agc_init_value = 0x1c; /* DPK TXAGC value*/
	u8 rxbb = 0x0, mixgain = 0x0;

	RF_DBG(dm, DBG_RF_DPK, "[DPK]===>%s\n", __func__);

	if (*dm->band_type == ODM_BAND_2_4G) {
		odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x5);
		ODM_delay_us(1);
		odm_set_rf_reg(dm, path, 0x00, 0x0001f, 0x18);
		ODM_delay_us(1);
		odm_set_rf_reg(dm, path, 0x5c, 0x03000, 0x1);
		odm_set_rf_reg(dm, path, 0x5c, 0xf0000, 0xe);
		odm_set_rf_reg(dm, path, 0x80, 0x00400, 0x1);
		odm_set_rf_reg(dm, path, 0x00, 0x003e0, 0x05);
		ODM_delay_us(1);
		odm_set_rf_reg(dm, path, 0xdf, 0x10000, 0x1);
		odm_set_rf_reg(dm, path, 0x88, 0x00300, 0x2);
		odm_set_rf_reg(dm, path, 0x88, 0x00c00, 0x0);
#if 0
		RF_DBG(dm, DBG_RF_DPK, "[DPK] ODM_BAND_2_4G\n");
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x5c = %x\n", odm_get_rf_reg(dm, path, 0x5c, 0x03000));	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x5c = %x\n", odm_get_rf_reg(dm, path, 0x5c, 0xf0000));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x80 = %x\n", odm_get_rf_reg(dm, path, 0x80, 0x00400));	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x00 = %x\n", odm_get_rf_reg(dm, path, 0x00, 0x003e0));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0xdf = %x\n", odm_get_rf_reg(dm, path, 0xdf, 0x10000));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x88 = %x\n", odm_get_rf_reg(dm, path, 0x88, 0x00300));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x88 = %x\n", odm_get_rf_reg(dm, path, 0x88, 0x00c00));

#endif
	} else {
		odm_set_rf_reg(dm, path, 0x00, 0xf0000, 0x5);
		ODM_delay_us(1);
		odm_set_rf_reg(dm, path, 0x00, 0x0001f, 0x1c);		
		ODM_delay_us(1);
		odm_set_rf_reg(dm, path, 0x63, 0x30000, 0x0);
		odm_set_rf_reg(dm, path, 0x63, 0x0f000, 0xf);
		odm_set_rf_reg(dm, path, 0x8c, 0x00010, 0x0);
		odm_set_rf_reg(dm, path, 0x8c, 0x001e0, 0x8);
		odm_set_rf_reg(dm, path, 0x00, 0x003e0, 0x07);		
		ODM_delay_us(1);
#if 0
		RF_DBG(dm, DBG_RF_DPK, "[DPK] ODM_BAND_5G\n");
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x00 = %x\n",odm_get_rf_reg(dm, path, 0x00, RFREGOFFSETMASK));	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x63 = %x\n",odm_get_rf_reg(dm, path, 0x63, 0x30000));	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x63 = %x\n",odm_get_rf_reg(dm, path, 0x63, 0x0f000));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x8c = %x\n",odm_get_rf_reg(dm, path, 0x8c, 0x00010));	
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x8c = %x\n",odm_get_rf_reg(dm, path, 0x8c, 0x001e0));
		RF_DBG(dm, DBG_RF_DPK, "[DPK] 0x00 = %x\n",odm_get_rf_reg(dm, path, 0x00, 0x003e0));

#endif
 	}
	ODM_delay_us(10);
}

boolean _do_dpk_8814c(void *dm_void)
{
	u32 mac_bp[DPK_MAC_NUM_8814C];
	u32 bb_bp[DPK_BB_NUM_8814C];
	u32 rf_bp[DPK_RF_NUM_8814C][SS_8814C];
	u32 bp_mac_reg[DPK_MAC_NUM_8814C] = {0x520};
	u32 bp_bb_reg[DPK_BB_NUM_8814C] = {0x1c38, 0x1d58, 0x1d60, 0x180c, 0x410c,
				       0x520c, 0x530c, 0x1d70, 0x1a00, 0x824,
				       0x1a14, 0x1830, 0x4130, 0x5230, 0x5330,
				       0x1860, 0x4160, 0x5260, 0x5360, 0x183c,
				       0x1840, 0x1844, 0x413c, 0x4140, 0x4144,
				       0x523c, 0x5240, 0x5244, 0x533c, 0x5340,
				       0x5344};
	u32 bp_rf_reg[DPK_RF_NUM_8814C] = {0x0, 0x88, 0x8c, 0x8e};
	u8 path = 0x0;
	u8 backoff = 0x0;
	boolean kfail = false;

	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	RF_DBG(dm, DBG_RF_DPK, "[DPK]===>%s\n", __func__);

	_dpk_bp_macbb_8814c(dm, mac_bp, bb_bp, bp_mac_reg, bp_bb_reg);
	_dpk_bp_rf_8814c(dm, rf_bp, bp_rf_reg);

	_dpk_set_macbbafe_8814c(dm);
	for (path = 0; path < SS_8814C; path++) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPK process start\n", path);
		_dpk_rfsetting_8814c(dm, path);
		backoff = _dpk_gainloss_func_8814c(dm, path);
		if (DPK_PAS_DBG_8814C)
			_dpk_log_pas_8814c(dm, path);
		
		kfail = _dpk_func_8814c(dm, path, backoff);
		if (DPK_PAS_DBG_8814C)
			_dpk_log_pas_8814c(dm, path);
		
		if (DPK_PAS_DBG_8814C)
			_dpk_lut_sram_read_8814c(dm, path);
		
		_dpk_on_func_8814c(dm, path, kfail);
		
		RF_DBG(dm, DBG_RF_DPK, "[DPK] S%d DPK process end, thermal = %d,\n", path, dpk_info->thermal_dpk[path]);
	}
	//return to S0
	odm_set_bb_reg(dm, 0x1b00, 0x00000006, RF_PATH_A);

	_dpk_restore_func_8814c(dm);

	_dpk_reload_rf_8814c(dm, bp_rf_reg, rf_bp);
	_dpk_reload_macbb_8814c(dm, mac_bp, bb_bp, bp_mac_reg,
				bp_bb_reg);
	
	return false;

}

void dpk_reload_8814c(void *dm_void)
{
	return;
}

void do_dpk_8814c(void *dm_void)
{

	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	boolean KFail;
	struct _hal_rf_ *rf = &dm->rf_table;	
	static boolean firstrun = true;

	RF_DBG(dm, DBG_RF_DPK, "[DPK]===>%s\n", __func__);
	
	if (!(rf->rf_supportability & HAL_RF_DPK))
		return;

	rf->is_dpk_in_progress = true;
	if (dm->ext_pa || dm->ext_pa_5g) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Skip DPK due to ext_PA exist!!\n");
		return;
	} else if (!dpk_info->is_dpk_pwr_on) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK] Skip DPK due to DPD PWR off !!\n");
		return;
	} else {
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK]DPK Start (Ver: %s), Cv: %d, Package: %d, rfe : %d\n",
		       DPK_VERSION_8814C, dm->cut_version, dm->package_type, dm->rfe_type);
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK]dm->ext_pa : %d, ext_pa_5g : %d\n", dm->ext_pa, dm->ext_pa_5g);		
	}
	

	if (firstrun) {		
		firstrun = false;
		//odm_read_and_config_mp_8814c_cal_init(dm);
	}
	dpk_info->thermal_dpk_avg_index = 0;
	KFail = _do_dpk_8814c(dm);
	dpk_info->dpk_path_ok = true;
	rf->is_dpk_in_progress = false;

	return;

}


void dpk_track_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	u8 i = 0, k = 0;
	u8 delta_dpk[4] = {0, 0, 0, 0};
	u8 offset[4] = {0, 0, 0, 0};
	u32 thermal_avg[4] = {0, 0, 0, 0};
	u8 thermal_value[4] = {0, 0, 0, 0};
	u8 path;
	u8 is_increase[4] = {0, 0, 0, 0};
	u8 reg1b64[4] = {0, 0, 0, 0};
	
	//DbgPrint("33333111111111111111111111111111111111\n");
	if (dm->ext_pa) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK]Skip DPK : ext_PA!!\n");
		return;
	} else if (!dpk_info->is_dpk_pwr_on) {
		RF_DBG(dm, DBG_RF_DPK, "[DPK]Skip DPK : DPD PWR off\n");
		return;
	} else {
		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] ***DPK Start (Ver: %s), Cv: %d, Package: %d**\n",
		       DPK_VERSION_8814C, dm->cut_version, dm->package_type);
	}

	/*Average times */
	if (dpk_info->thermal_dpk_avg_index >= AVG_THERMAL_NUM_DPK)
		dpk_info->thermal_dpk_avg_index = 0;
	
	for(path = 0; path < SS_8814C; path++) {
		thermal_value[path] = _dpk_thermal_read_8814c(dm, path);
		dpk_info->thermal_dpk_avg[path][dpk_info->thermal_dpk_avg_index] =
			thermal_value[path];
		
		for (i = 0; i < dpk_info->thermal_dpk_avg_index; i++) {
			if (dpk_info->thermal_dpk_avg[path][i]) {
				thermal_avg[path] += dpk_info->thermal_dpk_avg[path][i];
			}
#if 0
			RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] i=%d, S%x avg_ther = %d (%d), cur_ther= %d, dpk_ther =%d\n",
		       i, path, thermal_avg[path], dpk_info->thermal_dpk_avg_index, thermal_value[path], dpk_info->thermal_dpk[path]);
#endif
}

		/*Calculate Average ThermalValue after average enough times*/
		if(dpk_info->thermal_dpk_avg_index) {
			thermal_avg[path] = (u32)(thermal_avg[path] / dpk_info->thermal_dpk_avg_index);
		} else {
			thermal_avg[path] = (u32)thermal_value[path];
		}

		RF_DBG(dm, DBG_RF_DPK,
		       "[DPK] S%x avg_ther = %d (%d), cur_ther= %d, dpk_ther =%d\n",
		       path, thermal_avg[path], dpk_info->thermal_dpk_avg_index, thermal_value[path], dpk_info->thermal_dpk[path]);
		
		delta_dpk[path] = (u8)HALRF_ABS(thermal_avg[path], (u32)dpk_info->thermal_dpk[path]);
		is_increase[path] = ((thermal_avg[path] < dpk_info->thermal_dpk[path]) ? 0 : 1);
		offset[path] = (u8) (delta_dpk[path] / DPK_THRESHOLD_8814C); //6oC		
		RF_DBG(dm, DBG_RF_DPK,"[DPK] S%x is_increase[%x] = %x, offset[%x] =%x\n", path, path, is_increase[path], path,offset[path]);		

	}

	// write 0x1b64 
	for(path = 0; path < SS_8814C; path++) {
		if (thermal_avg[path] < (u32)((dpk_info->thermal_dpk[path] + DPK_THRESHOLD_8814C)) &&
		    thermal_avg[path] > (u32)((dpk_info->thermal_dpk[path] - DPK_THRESHOLD_8814C))){
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
			odm_set_bb_reg(dm, 0x1b64, 0xff000000, dpk_info->thr_pwsf[path]);			
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, reload 0x1b64[31:24] = 0x%x\n", path, dpk_info->thr_pwsf[path]);
		} else if(offset[path] != 0x0) {
			if (is_increase[path])
				reg1b64[path] = dpk_info->thr_pwsf[path] - (offset[path]*1);
			else
				reg1b64[path] = dpk_info->thr_pwsf[path] + (offset[path]*1);			
			odm_set_bb_reg(dm, 0x1b00, 0x00000006, path);
			odm_set_bb_reg(dm, 0x1b64, 0xff000000, reg1b64[path]);
			RF_DBG(dm, DBG_RF_DPK, "[DPK] S%x, update 0x1b64[31:24] = 0x%x\n", path, reg1b64[path]);
		}
	}
	dpk_info->thermal_dpk_avg_index++;
	//return to path A
	odm_set_bb_reg(dm, 0x1b00, 0x0000000f, IQK_CMD_8814C);
	return;	
}

void dpk_set_dpkbychannel_8814c(void *dm_void, boolean dpk_by_ch)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	dpk_info->is_dpk_by_channel = dpk_by_ch;
	RF_DBG(dm, DBG_RF_DPK, "[DPK] set_dpkbychannel_8814c\n");
	return;
}

boolean dpk_get_dpkbychannel_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] get_dpkbychannel_8814c= %x\n",
	       dpk_info->is_dpk_by_channel);

	return dpk_info->is_dpk_by_channel;
}

void dpk_set_is_dpk_enable_8814c(void *dm_void, boolean is_dpk_enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	dpk_info->is_dpk_enable = is_dpk_enable;
	RF_DBG(dm, DBG_RF_DPK, "[DPK] set_is_dpk_enable_8814c\n");
	return;
}

boolean dpk_get_is_dpk_enable_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] get_is_dpk_enable_8814c= %x\n",
	       dpk_info->is_dpk_enable);

	return dpk_info->is_dpk_enable;
}

void dpk_enable_disable_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u8 path =0x0;

	for(path = 0x0; path < 4; path++)
		_dpk_enable_disable_8814c(dm, path);
}

void dpk_thermal_read_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	u8 path = 0x0;

	for (path = 0; path < 4; path++)
		_dpk_thermal_read_8814c(dm, path);
}

void dpk_sram_read_8814c(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	RF_DBG(dm, DBG_RF_DPK, "[DPK] ========= SRAM Read Start =========\n");
	RF_DBG(dm, DBG_RF_DPK, "[DPK] ===== SRAM Read Finish =====\n");
}


#endif
