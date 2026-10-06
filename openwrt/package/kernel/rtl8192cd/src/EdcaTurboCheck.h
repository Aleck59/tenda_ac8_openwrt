#ifndef __EDCATURBOCHECK_H__
#define __EDCATURBOCHECK_H__

struct rtl8192cd_priv;
struct sta_info;

void EdcaParaInit(
	struct rtl8192cd_priv *priv
	);

#ifdef WIFI_WMM
VOID
IotEdcaSwitch(
	struct rtl8192cd_priv *priv,
	unsigned char		enable
	);
#endif

BOOLEAN
ChooseIotMainSTA(
	struct rtl8192cd_priv *priv,
	struct sta_info*		pstat
	);

VOID 
IotEngine(
	struct rtl8192cd_priv *priv
	);

void reset_swq_max_xmit(struct rtl8192cd_priv *priv);

void manual_swq_max_xmit(struct rtl8192cd_priv *priv);

#endif // __EDCATURBOCHECK_H__
