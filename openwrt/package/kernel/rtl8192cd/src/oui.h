#ifndef _OUI_H_
#define _OUI_H_

#define OUI_TO_UINT32(m1, m2, m3) \
	(((unsigned int)(m1) << 16) + ((unsigned int)(m2) << 8) + (unsigned int)(m3))

/* Indicate different STA vendors for IOT issue. */
typedef enum _HT_IOT_PEER {
	HT_IOT_PEER_UNKNOWN			= 0,
	HT_IOT_PEER_REALTEK			= 1,
	HT_IOT_PEER_REALTEK_92SE		= 2,
	HT_IOT_PEER_BROADCOM			= 3,
	HT_IOT_PEER_RALINK			= 4,
	HT_IOT_PEER_ATHEROS			= 5,
	HT_IOT_PEER_CISCO			= 6,
	HT_IOT_PEER_MERU			= 7,
	HT_IOT_PEER_MARVELL			= 8,
	HT_IOT_PEER_REALTEK_SOFTAP		= 9,  // peer is RealTek SOFT_AP, by Bohn, 2009.12.17
	HT_IOT_PEER_SELF_SOFTAP			= 10, // Self is SoftAP
	HT_IOT_PEER_AIRGO			= 11,
	HT_IOT_PEER_INTEL			= 12,
	HT_IOT_PEER_RTK_APCLIENT		= 13,
	HT_IOT_PEER_REALTEK_81XX		= 14,
	HT_IOT_PEER_REALTEK_WOW			= 15,
	HT_IOT_PEER_HTC				= 16,
	HT_IOT_PEER_REALTEK_8812		= 17,
	HT_IOT_PEER_CMW				= 18,
	HT_IOT_PEER_SPIRENT			= 19,
	HT_IOT_PEER_VERIWAVE			= 20,
	HT_IOT_PEER_APPLE			= 21,
	HT_IOT_PEER_REALTEK_8814		= 22,
	HT_IOT_PEER_NETGEAR			= 23,
	HT_IOT_PEER_OCTOSCOPE			= 24,
	HT_IOT_PEER_PSP				= 25,
	HT_IOT_PEER_MAX				/* MUST keep this enum for size assertion */
} HT_IOT_PEER_E, *PHTIOT_PEER_E;

/* Use STA's MAC OUI to search IOT */
HT_IOT_PEER_E mac_oui_2_iot(unsigned int oui);

/* Use OUI in vendor specific IE to search IOT */
HT_IOT_PEER_E vsie_oui_2_iot(unsigned int oui);

#endif // _OUI_H_
