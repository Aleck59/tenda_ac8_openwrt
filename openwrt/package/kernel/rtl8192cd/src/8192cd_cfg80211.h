/*
 * Copyright (c) 2011 Atheros Communications Inc.
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef RTK_NL80211
/**
 * enum ieee80211_band - supported frequency bands
 *
 * The bands are assigned this way because the supported
 * bitrates differ in these bands.
 *
 * @IEEE80211_BAND_2GHZ: 2.4GHz ISM band
 * @IEEE80211_BAND_5GHZ: around 5GHz band (4.9-5.7)
 * @IEEE80211_BAND_60GHZ: around 60 GHz band (58.32 - 64.80 GHz)
 * @IEEE80211_NUM_BANDS: number of defined bands
 */
enum ieee80211_band {
	IEEE80211_BAND_2GHZ = NL80211_BAND_2GHZ,
	IEEE80211_BAND_5GHZ = NL80211_BAND_5GHZ,
	IEEE80211_BAND_60GHZ = NL80211_BAND_60GHZ,

	/* keep last */
	IEEE80211_NUM_BANDS
};

#include "./wifi.h"/*cfg p2p cfg p2p*/

#define DSSET_IE_LEN	1
#define HTCAP_IE_LEN	26
#define HTINFO_IE_LEN	22
#define MAX_PROBED_SSIDS 32

#define VIF_NUM				RTL8192CD_NUM_VWLAN //eric-vap
#define IF_NUM				(VIF_NUM+2)  //#vap + root + vxd
#define VIF_NAME_SIZE		10
/*cfg p2p cfg p2p*/
#define MAX_IE_LEN 			768

#define MAX_ASSOC_REQ_LEN 	512
#define MAX_ASSOC_RSP_LEN 	512


#define RATETAB_ENT(_rate, _rateid, _flags) {   \
	.bitrate    = (_rate),                  \
	.flags      = (_flags),                 \
	.hw_value   = (_rateid),                \
}


static struct ieee80211_rate realtek_rates[] = {
	RATETAB_ENT(10, 0x1, 0),
	RATETAB_ENT(20, 0x2, 0),
	RATETAB_ENT(55, 0x4, 0),
	RATETAB_ENT(110, 0x8, 0),
	RATETAB_ENT(60, 0x10, 0),
	RATETAB_ENT(90, 0x20, 0),
	RATETAB_ENT(120, 0x40, 0),
	RATETAB_ENT(180, 0x80, 0),
	RATETAB_ENT(240, 0x100, 0),
	RATETAB_ENT(360, 0x200, 0),
	RATETAB_ENT(480, 0x400, 0),
	RATETAB_ENT(540, 0x800, 0),
};

struct realtek_80211_hdr{
	u16 frame_control;
	u16 duration_id;
	u8 addr1[6];
	u8 addr2[6];
	u8 addr3[6];
	u16 seq_ctrl;
	/* followed by 'u8 addr4[6];' if ToDS and FromDS is set in data frame
	 */
};

#define IEEE80211_HDRLEN (sizeof(struct realtek_80211_hdr))

#define realtek_g_rates     (realtek_rates + 0)
#define realtek_g_rates_size    12

#define realtek_a_rates     (realtek_rates + 4)
#define realtek_a_rates_size    8

#define realtek_g_htcap (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | \
			IEEE80211_HT_CAP_SGI_20		 | \
			IEEE80211_HT_CAP_SGI_40)

#define realtek_a_htcap (IEEE80211_HT_CAP_SUP_WIDTH_20_40 | \
			IEEE80211_HT_CAP_SGI_20		 | \
			IEEE80211_HT_CAP_SGI_40)

#define realtek_a_vhtcap (IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_80MHZ)


/* WMI_CONNECT_CMDID  */
enum network_type {
	INFRA_NETWORK = 0x01,
	ADHOC_NETWORK = 0x02,
	ADHOC_CREATOR = 0x04,
	AP_NETWORK = 0x10,
};

enum scan_abort_case {
	SCAN_ABORT_DEL_IFACE = 0, 
	SCAN_ABORT_START_AP,
};

#if 0/*cfg p2p cfg p2p*/
enum mgmt_type {
	MGMT_BEACON = 0,
	MGMT_PROBERSP = 1,
	MGMT_ASSOCRSP = 2,
	MGMT_ASSOCREQ = 3,
	MGMT_PROBEREQ = 4,	
};
#endif

static const u32 cipher_suites[] = {
	WLAN_CIPHER_SUITE_WEP40,
	WLAN_CIPHER_SUITE_WEP104,
	WLAN_CIPHER_SUITE_TKIP,
	WLAN_CIPHER_SUITE_CCMP,
#ifdef CONFIG_IEEE80211W 
	WLAN_CIPHER_SUITE_AES_CMAC,
	WLAN_CIPHER_SUITE_GCMP,
	WLAN_CIPHER_SUITE_GCMP_256,
	WLAN_CIPHER_SUITE_CCMP_256,
	WLAN_CIPHER_SUITE_BIP_GMAC_128,
	WLAN_CIPHER_SUITE_BIP_GMAC_256,
	WLAN_CIPHER_SUITE_BIP_CMAC_256,
#endif
};

static const u32 akm_suites[] = {
	WLAN_AKM_SUITE_8021X,
	WLAN_AKM_SUITE_PSK,
	WLAN_AKM_SUITE_FT_8021X,
	WLAN_AKM_SUITE_FT_PSK,
	WLAN_AKM_SUITE_8021X_SHA256,
	WLAN_AKM_SUITE_PSK_SHA256,
	WLAN_AKM_SUITE_SAE,
};

#define CHAN2G(_channel, _freq, _flags) {   \
	.band           = IEEE80211_BAND_2GHZ,  \
	.hw_value       = (_channel),           \
	.center_freq    = (_freq),              \
	.flags          = (_flags),             \
	.max_antenna_gain   = 0,                \
	.max_power      = 30,                   \
}

#define CHAN5G(_channel, _flags) {              \
    .band           = IEEE80211_BAND_5GHZ,      \
    .hw_value       = (_channel),               \
    .center_freq    = 5000 + (5 * (_channel)),  \
    .flags          = (_flags),                 \
    .max_antenna_gain   = 0,                    \
    .max_power      = 30,                       \
	.dfs_cac_ms		= 60000,					\
} 
   

static struct ieee80211_channel realtek_2ghz_channels[] = {
	CHAN2G(1, 2412, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(2, 2417, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(3, 2422, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(4, 2427, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN2G(5, 2432, 0),
	CHAN2G(6, 2437, 0),
	CHAN2G(7, 2442, 0),
	CHAN2G(8, 2447, 0),
	CHAN2G(9, 2452, 0),
	CHAN2G(10, 2457, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(11, 2462, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(12, 2467, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(13, 2472, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN2G(14, 2484, IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS | IEEE80211_CHAN_NO_OFDM)
};

static struct ieee80211_channel realtek_5ghz_a_channels[] = {
	/* UNII-1 */
	CHAN5G(36, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(40, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(44, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(48, IEEE80211_CHAN_NO_HT40PLUS),
	/* UNII-2 */
	CHAN5G(52,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(56,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(60,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(64,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	/* MID */
	CHAN5G(100,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(104,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(108,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(112,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(116,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(120,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(124,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(128,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(132,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(136,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(140,
		 IEEE80211_CHAN_RADAR | IEEE80211_CHAN_NO_HT40PLUS |
		 IEEE80211_CHAN_NO_HT40MINUS),
	/* UNII-3 */
	CHAN5G(149, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(153, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(157, IEEE80211_CHAN_NO_HT40MINUS),
	CHAN5G(161, IEEE80211_CHAN_NO_HT40PLUS),
	CHAN5G(165, IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS)
};

/* IEEE 802.11b/g, channels 1..11 */
#define RTL819x_2GHZ_CH01_11 REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)

/* IEEE 802.11b/g, channels 12..13. */
#define RTL819x_2GHZ_CH12_13 REG_RULE(2467-10, 2472+10, 20, 0, 20, 0)

/* IEEE 802.11b/g, channels 1..13 */
#define RTL819x_2GHZ_CH01_13 REG_RULE(2412-10, 2472+10, 40, 0, 20, 0)

/* IEEE 802.11 channel 14 - Only JP enables
 * this and for 802.11b only */
#define RTL819x_2GHZ_CH14 REG_RULE(2484-10, 2484+10, 20, 0, 20, 0)

/* 5G chan 36 - chan 64*/
#define RTL819x_5GHZ_5150_5350	\
	REG_RULE(5150-10, 5350+10, 80, 0, 30, 0)

/* IEEE 802.11a, channel 100..144 - DFS required */
#define RTL819x_5GHZ_5500_5720	\
	REG_RULE(5500-10, 5720+10, 160, 0, 20,0)

/* 5G chan 100 - chan 165*/
#define RTL819x_5GHZ_5470_5850	\
	REG_RULE(5470-10, 5850+10, 80, 0, 30, 0)

/* 5G chan 149 - chan 165*/
#define RTL819x_5GHZ_5725_5850	\
	REG_RULE(5725-10, 5850+10, 80, 0, 30, 0)

#define RTL_2GHZ_ALL		RTL819x_2GHZ_CH01_13, \
							RTL819x_2GHZ_CH14

#define RTL_5GHZ_ALL		RTL819x_5GHZ_5150_5350, \
							RTL819x_5GHZ_5470_5850

/* This one skips what we call "mid band" */
#define RTL_5GHZ_NO_MIDBAND	RTL819x_5GHZ_5150_5350, \
							RTL819x_5GHZ_5725_5850

static const struct ieee80211_regdomain rtl_world_regdom_all = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		RTL_2GHZ_ALL,
		RTL_5GHZ_ALL,
	}
};

static const struct ieee80211_regdomain rtl_regdom_no_midband = {
	.n_reg_rules = 3,
	.alpha2 = "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
	    RTL_5GHZ_NO_MIDBAND,
	}
};

static const struct ieee80211_regdomain rtl_world_regdom_1 = {
	.n_reg_rules = 3,
	.alpha2 =  "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL_5GHZ_ALL,
	}
};

static const struct ieee80211_regdomain rtl_world_regdom_3 = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_5GHZ_5150_5350,
		RTL819x_5GHZ_5500_5720,
	}
};

static const struct ieee80211_regdomain rtl_world_regdom_6 = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		RTL_2GHZ_ALL,
		RTL819x_5GHZ_5150_5350,
		RTL819x_5GHZ_5500_5720,
	}
};

static const struct ieee80211_regdomain rtl_world_regdom_7 = {
	.n_reg_rules = 3,
	.alpha2 =  "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL819x_5GHZ_5500_5720,
	}
};

static const struct ieee80211_regdomain rtl_world_regdom_12 = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_11,
		RTL819x_2GHZ_CH12_13,
		RTL_5GHZ_ALL,
	}
};


static const struct ieee80211_regdomain rtl_world_regdom_13 = {
	.n_reg_rules = 3,
	.alpha2 =  "99",
	.reg_rules = {
		RTL819x_2GHZ_CH01_13,
		RTL_5GHZ_NO_MIDBAND,
	}
};

static const struct ieee80211_regdomain rtk_regdom = {
	.n_reg_rules = 4,
	.alpha2 =  "99",
	.reg_rules = {
		/* IEEE 802.11b/g, channels 1..13 */
		REG_RULE(2412-10, 2472+10, 40, 6, 20, 0),
		/* If any */
		/* IEEE 802.11 channel 14 - Only JP enables
		 * this and for 802.11b only
		 */
		REG_RULE(2484-10, 2484+10, 20, 6, 20, 0),
		/* IEEE 802.11a, channel 36..64 */
		REG_RULE(5150-10, 5350+10, 160, 6, 20, 0),
		/* IEEE 802.11a, channel 100..165 */
		REG_RULE(5470-10, 5850+10, 160, 6, 20, 0), }
};

static const struct ieee80211_regdomain *_rtl_regdomain_select(
						unsigned int dot11RegDomain)
{
	switch (dot11RegDomain) {
	case DOMAIN_FCC:
	case DOMAIN_NCC:
		return &rtl_world_regdom_1;
	case DOMAIN_ETSI:
		return &rtl_world_regdom_3;
	case DOMAIN_MKK:
		return &rtl_world_regdom_6;
	case DOMAIN_ISRAEL:
		return &rtl_world_regdom_7;
	case DOMAIN_RUSSIAN:
		return &rtl_world_regdom_12;
	case DOMAIN_CN:
		return &rtl_world_regdom_13;
	case DOMAIN_TEST:
		return &rtl_world_regdom_all;
	default:
		return &rtl_regdom_no_midband;
	}
}

static struct ieee80211_supported_band realtek_band_2ghz = {
	.band = NL80211_BAND_2GHZ,
	.n_channels = ARRAY_SIZE(realtek_2ghz_channels),
	.channels = realtek_2ghz_channels,
	.n_bitrates = realtek_g_rates_size,
	.bitrates = realtek_g_rates,
	.ht_cap = {
		.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 | 
				IEEE80211_HT_CAP_TX_STBC | IEEE80211_HT_CAP_RX_STBC,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = 7,
		.mcs = {
			.rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
			.rx_highest = cpu_to_le16(300),
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED
		}
	}
};

static struct ieee80211_supported_band realtek_band_5ghz = {
	.band = NL80211_BAND_5GHZ,
	.n_channels = ARRAY_SIZE(realtek_5ghz_a_channels),
	.channels = realtek_5ghz_a_channels,
	.n_bitrates = realtek_a_rates_size,
	.bitrates = realtek_a_rates,
	.ht_cap = {
		.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 |
				IEEE80211_HT_CAP_TX_STBC | IEEE80211_HT_CAP_RX_STBC,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = 7,
		.mcs = {
			.rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
			.rx_highest = cpu_to_le16(300),
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED
		}
	},
	.vht_cap = {
		.vht_supported = true,
		.cap = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991 | IEEE80211_VHT_CAP_SHORT_GI_80 | 
				IEEE80211_VHT_CAP_TXSTBC | IEEE80211_VHT_CAP_RXSTBC_1 |
#if (BEAMFORMING_SUPPORT == 1)
				IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE | IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
				(1 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT) | (1 << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT) |
#endif
				IEEE80211_VHT_CAP_HTC_VHT | (7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT), 
		.vht_mcs = {
			.rx_mcs_map = cpu_to_le16(0xfffa),
			.tx_mcs_map = cpu_to_le16(0xfffa),
			.rx_highest = cpu_to_le16(780),
			.tx_highest = cpu_to_le16(780),
		}
	}
};

struct rtk_clnt_info {
	struct wpa_ie_info	wpa_ie;
	struct rsn_ie_info	rsn_ie;
	unsigned char assoc_req[MAX_ASSOC_REQ_LEN];
	unsigned char assoc_req_len;
	unsigned char assoc_rsp[MAX_ASSOC_RSP_LEN];
	unsigned char assoc_rsp_len;
};

struct rtk_iface_info {
	unsigned char used;
	unsigned char ndev_name[32];
	struct rtl8192cd_priv *priv;
};

struct survey_info_t {
	unsigned int channel;
	unsigned int chbusytime;
	unsigned int rx_time;
	unsigned int tx_time;
	s8 noise;
};

struct rtknl {
	struct class *cl;
	struct device *dev;
	struct wiphy *wiphy;
	struct rtl8192cd_priv *priv;
	struct net_device *ndev_add;
	struct rtk_clnt_info clnt_info;
	unsigned char num_vif;
	int 		idx_vif;
	unsigned char num_vap;
	unsigned char num_vxd;
	unsigned int  vif_flag;
	unsigned char wiphy_registered;
	unsigned int  cipher;
	unsigned int  wpa;
	unsigned int  psk;
	unsigned int  sha256;
	unsigned char ndev_name[VIF_NUM][VIF_NAME_SIZE];
	unsigned char ndev_name_vxd[VIF_NAME_SIZE];
	unsigned char root_ifname[VIF_NAME_SIZE];
	unsigned char root_mac[ETH_ALEN];
	unsigned char vap_mac[VIF_NUM][ETH_ALEN];
	struct rtl8192cd_priv *priv_root;
	struct rtl8192cd_priv *priv_vxd;
	struct rtk_iface_info rtk_iface[VIF_NUM+2];
	//for survey_dump
	struct survey_info_t survey_info[ARRAY_SIZE(realtek_5ghz_a_channels)];
	/* By brian, to support per channel statistic
	unsigned int chbusytime;
	unsigned int rx_time;
	unsigned int tx_time;
	*/
	//openwrt_psd
	unsigned int psd_chnl;
	unsigned int psd_bw;
	unsigned int psd_pts;
	unsigned int psd_fft_info[1040];
	//openwrt_tx_power_use
	unsigned int pwr_rate;
	int 		 pwr_cur;
	int			 pwr_set_dbm;
	unsigned char	keep_legacy;
};

struct CFG80211_RX_MGMT_IND {
	struct list_head list;
	struct wireless_dev *wdev;
	int freq;
#ifdef RTK_MULTI_AP_R3
	u8 buf[1024];
#else
	u8 buf[512];
#endif
	size_t len;
	u32 flags;
};

/* HT Capabilities Info field within HT Capabilities element */
#define HT_CAP_INFO_SHORT_GI20MHZ		((u16) BIT(5))
#define HT_CAP_INFO_SHORT_GI40MHZ		((u16) BIT(6))
#define HT_CAP_INFO_TX_STBC					((u16) BIT(7))

/* VHT Defines */
#define VHT_CAP_SHORT_GI_80				((u32) BIT(5))

#if defined(VAP_MAC_DRV_READ_FLASH)
int read_flash_hw_mac_vap( unsigned char *mac, int vap_idx); 
#endif
unsigned char is_WRT_scan_iface(const char* if_name); //eric-vap
void realtek_cfg80211_inform_ss_result(struct rtl8192cd_priv *priv);
#ifdef RTK_129X_PLATFORM
struct rtknl *realtek_cfg80211_create(struct device *dev);
#else
struct rtknl *realtek_cfg80211_create(void); 
#endif 
int realtek_rtknl_init(struct rtknl *rtk);
int realtek_cfg80211_init(struct rtknl *rtk,struct rtl8192cd_priv *priv); 
int realtek_interface_add(struct rtl8192cd_priv *priv, struct rtknl *rtk, const char *name, 
								enum nl80211_iftype type, u8 fw_vif_idx, u8 nw_type);
u8 rtw_cfg80211_ch_switch_notify(struct rtl8192cd_priv *priv);
int event_indicate_cfg80211(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra);
void close_vxd_vap(struct rtl8192cd_priv *priv_root);
int check_5M10M_config(struct rtl8192cd_priv *priv);
void rtk_remove_dev(struct rtknl *rtk,int idx);
void realtek_change_iftype(struct rtl8192cd_priv *priv ,enum nl80211_iftype type);
#ifdef CONFIG_WLAN_HAL_8197F
int realtek_load_flash_calibration(struct rtl8192cd_priv *priv);
#endif

#define CFG_MGT_TX_TYPE_MGT 1
#define CFG_MGT_TX_TYPE_ACTION 2

#ifdef RTK_129X_PLATFORM
void rtk_free_wiphy(struct rtknl *rtk);
void rtk_unregister_wiphy(struct rtknl *rtk);
#endif
void rtk_del_priv(struct rtl8192cd_priv * priv_del, struct rtknl * rtk);
void realtek_process_cfg80211_event(unsigned long task_priv);
void realtek_enqueue_cfg80211_event(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, u32 answered);
void realtek_clear_cfg80211_event(struct rtl8192cd_priv *priv);
#ifdef SUPPORT_WPAS_SAE
int wpas_issue_sae_auth_commit( struct rtl8192cd_priv *priv);
#endif

enum external_auth_action {
	EXTERNAL_AUTH_START,
	EXTERNAL_AUTH_ABORT,
};

int rtw_sae_preprocess(
	struct rtl8192cd_priv *priv,
	 const u8 *buf, u32 len,
	 u8 tx);

#endif /* RTK_NL80211 */
