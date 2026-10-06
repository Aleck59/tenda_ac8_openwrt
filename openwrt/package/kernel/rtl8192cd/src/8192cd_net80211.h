/*
 *  Header file for Net80211-compatible handling routines
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 

#ifndef _8192CD_NET80211_H_
#define _8192CD_NET80211_H_
 

#define MAX_CONFIG_FILE_SIZE (20*1024) // for 8192, added to 20k
#define MAX_PARAM_BUF_SIZE (1024) // for 8192, added to 20k

#ifdef WIFI_HAPD
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>

int rtl_net80211_setparam(struct net_device *dev, int param, int value);
int rtl_net80211_setappiebuf(struct net_device *dev, struct ieee80211req_getset_appiebuf *ie);
int rtl_net80211_getwpaie(struct net_device *dev, struct ieee80211req_wpaie *ie);
int rtl_hapd_config(struct net_device *dev, struct rtk_hapd_config *config);
int rtl_net80211_setmlme(struct net_device *dev, struct ieee80211req_mlme *mlme);
int rtl_net80211_setkey(struct net_device *dev, struct ieee80211req_key *wk);
int rtl_net80211_delkey(struct net_device *dev, struct ieee80211req_del_key *wk);
void rsn_init(struct rtl8192cd_priv *priv);

#ifdef WIFI_WPAS
int rtl_net80211_setoptie(struct net_device *dev, unsigned char *ie, unsigned int length);
int rtl_wpas_custom(struct net_device *dev, struct rtk_wpas_config *config);
#endif // WIFI_WPAS

#if	defined(WDS) && !defined(HAPD_DRV_PSK_WPS)
int rtl_net80211_wdsaddmac(struct net_device *dev, struct rtk_wds_config *wds);
int rtl_net80211_wdsdelmac(struct net_device *dev, struct rtk_wds_config *wds);
#endif // defined(WDS) && !defined(HAPD_DRV_PSK_WPS)

#elif defined(RTK_NL80211)
/*
 * Macros and structures defined in net80211/xxx.h only exist in the linux-4.4.x
 * kernel of Realtek SDK. To be compatabile with the latest version of kernel,
 * define those macros and structures here when building driver with cfg80211.
 */

#define	IEEE80211_ADDR_LEN		6
#define	IEEE80211_NWID_LEN		32
#define	IEEE80211_KEYBUF_SIZE	16
#define	IEEE80211_MICBUF_SIZE	(8 + 8)

/*
 * NB: these values are ordered carefully; there are lots of
 * of implications in any reordering.  In particular beware
 * that 4 is not used to avoid conflicting with IEEE80211_F_PRIVACY.
 */
#define IEEE80211_CIPHER_WEP            0
#define IEEE80211_CIPHER_TKIP           1
#define IEEE80211_CIPHER_AES_OCB        2
#define IEEE80211_CIPHER_AES_CCM        3
#define IEEE80211_CIPHER_AES_CMAC       4
#define IEEE80211_CIPHER_CKIP           5
#define IEEE80211_CIPHER_NONE           6       /* pseudo value */
#define IEEE80211_CIPHER_MAX            (IEEE80211_CIPHER_NONE+1)
#define IEEE80211_KEYIX_NONE    ((u_int16_t) - 1)

/*
 * WPA/RSN get/set key request.  Specify the key/cipher
 * type and whether the key is to be used for sending and/or
 * receiving.  The key index should be set only when working
 * with global keys (use IEEE80211_KEYIX_NONE for ``no index'').
 * Otherwise a unicast/pairwise key is specified by the bssid
 * (on a station) or mac address (on an ap).  They key length
 * must include any MIC key data; otherwise it should be no
 more than IEEE80211_KEYBUF_SIZE.
 */
#pragma pack(1)
struct ieee80211req_key {
        u_int8_t ik_type;               /* key/cipher type */
        u_int8_t ik_pad;
        u_int16_t ik_keyix;             /* key index */
        u_int8_t ik_keylen;             /* key length in bytes */
        u_int8_t ik_flags;
/* NB: IEEE80211_KEY_XMIT and IEEE80211_KEY_RECV defined elsewhere */
#define IEEE80211_KEY_DEFAULT   0x80    /* default xmit key */
        u_int8_t ik_macaddr[IEEE80211_ADDR_LEN];
        u_int64_t ik_keyrsc;            /* key receive sequence counter */ //RTK_WPAS WIFI_WPAS
        //u_int64_t ik_keytsc;          /* key transmit sequence counter */
        u_int8_t ik_keydata[IEEE80211_KEYBUF_SIZE+IEEE80211_MICBUF_SIZE];
};
#pragma pack()

/*
 * Delete a key either by index or address.  Set the index
 * to IEEE80211_KEYIX_NONE when deleting a unicast key.
 */
struct ieee80211req_del_key {
        u_int8_t idk_keyix;             /* key index */
        u_int8_t idk_macaddr[IEEE80211_ADDR_LEN];
};

/*
 * MLME state manipulation request.  IEEE80211_MLME_ASSOC
 * only makes sense when operating as a station.  The other
 * requests can be used when operating as a station or an
 * ap (to effect a station).
 */
struct ieee80211req_mlme {
        u_int8_t im_op;                 /* operation to perform */
#define IEEE80211_MLME_ASSOC            1       /* associate station */
#define IEEE80211_MLME_DISASSOC         2       /* disassociate station */
#define IEEE80211_MLME_DEAUTH           3       /* deauthenticate station */
#define IEEE80211_MLME_AUTHORIZE        4       /* authorize station */
#define IEEE80211_MLME_UNAUTHORIZE      5       /* unauthorize station */
#define IEEE80211_MLME_CLEAR_STATS      6       /* clear station statistic */
        u_int8_t im_ssid_len;           /* length of optional ssid */
        u_int16_t im_reason;            /* 802.11 reason code */
        u_int8_t im_macaddr[IEEE80211_ADDR_LEN];
        u_int8_t im_ssid[IEEE80211_NWID_LEN];
};

int rtl_net80211_setmlme(struct net_device *dev, struct ieee80211req_mlme *mlme);
int rtl_net80211_setkey(struct net_device *dev, struct ieee80211req_key *wk);
int rtl_net80211_delkey(struct net_device *dev, struct ieee80211req_del_key *wk);
void rsn_init(struct rtl8192cd_priv *priv);
int rtl_wpas_join(struct rtl8192cd_priv *priv, int bss_num);

#endif // WIFI_HAPD, RTK_NL80211
#endif  // _8192CD_NET80211_H_
