/*
 *  API-compatible handling routines
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_BAND_STEERING_C_

#include "./8192cd_band_steering.h"
#include "./8192cd.h"

#ifdef CONFIG_RTK_BAND_STEERING
static struct b_steer_block_entry *block_entry_lookup(struct rtl8192cd_priv *priv, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(priv->bsteerpriv.block_list[i]);
		if (ent->used && !memcmp(ent->mac, mac, MACADDRLEN))
			return ent;
	}

	return NULL;
}

/* ----------------------------------------- */
/* ---------- APIs for core layer ---------- */
/* ----------------------------------------- */
void _band_steering_expire(struct rtl8192cd_priv *priv)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;
	unsigned long flags = 0;

	SMP_LOCK_BS_BLOCK_LIST(flags);

	/* block entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(priv->bsteerpriv.block_list[i]);
		if (!ent->used)
			continue;

		if (ent->entry_expire) {
			if (--ent->entry_expire == 0)
				ent->used = 0;
		}
	}

	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return;
}

s32 _band_steering_block_chk(struct rtl8192cd_priv *priv, u8 *mac)
{
	s32 ret = 0;
	struct b_steer_block_entry *ent = NULL;
	unsigned long flags = 0;

	SMP_LOCK_BS_BLOCK_LIST(flags);

	ent = block_entry_lookup(priv, mac);
	if (ent)
		ret = 1;

	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return ret;
}

void _band_steering_block_entry_add(struct rtl8192cd_priv *priv, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;
	unsigned long flags = 0;

	SMP_LOCK_BS_BLOCK_LIST(flags);

	ent = block_entry_lookup(priv, mac);

	/* already exist */
	if (ent) {
		ent->entry_expire = B_STEER_BLOCK_ENTRY_EXPIRE;
		goto func_return;
	}

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!priv->bsteerpriv.block_list[i].used) {
			ent = &(priv->bsteerpriv.block_list[i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		memcpy(ent->mac, mac, MACADDRLEN);
		ent->entry_expire = B_STEER_BLOCK_ENTRY_EXPIRE;
	}

func_return:
	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return;
}

void _band_steering_block_entry_del(struct rtl8192cd_priv *priv, u8 *mac)
{
	struct b_steer_block_entry *ent = NULL;
	unsigned long flags = 0;

	SMP_LOCK_BS_BLOCK_LIST(flags);

	ent = block_entry_lookup(priv, mac);
	if (ent)
		ent->used = 0;

	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return;
}

void _band_steering_roam_block_entry_add(struct rtl8192cd_priv *priv, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;
	unsigned long flags = 0;

	SMP_LOCK_BS_BLOCK_LIST(flags);

	ent = block_entry_lookup(priv, mac);

	/* already exist */
	if (ent)
		goto func_return;

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!priv->bsteerpriv.block_list[i].used) {
			ent = &(priv->bsteerpriv.block_list[i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		memcpy(ent->mac, mac, MACADDRLEN);
		ent->entry_expire = B_STEER_ROAM_BLOCK_ENTRY_EXPIRE;
	}

func_return:
	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return;
}

void _band_steering_init(struct rtl8192cd_priv *priv)
{
	u8 i;
	unsigned long flags = 0;

	spin_lock_init(&(priv->bsteerpriv.lock));

	SMP_LOCK_BS_BLOCK_LIST(flags);

	/* block entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		priv->bsteerpriv.block_list[i].used = 0;
	}

	SMP_UNLOCK_BS_BLOCK_LIST(flags);

	return;
}

#endif /* end _8192CD_BAND_STEERING_C_ */
