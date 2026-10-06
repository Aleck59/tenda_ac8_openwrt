/*
 *  802.11k related routines
 *
 *  Copyright (c) 2017 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8192CD_11K_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <asm/byteorder.h>
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#endif

#include "../8192cd_cfg.h"

#if !defined(__KERNEL__) && !defined(__ECOS)
#include "../sys-support.h"
#endif

#include "../8192cd.h"
#ifdef __KERNEL__
#include "../ieee802_mib.h"
#elif defined(__ECOS)
#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#endif
#include "../8192cd_util.h"
#include "../8192cd_headers.h"
#include "../8192cd_debug.h"

#ifdef DOT11K
const unsigned char wfa_oui[] = {0x00, 0x50, 0xf2};

static unsigned char * construct_measure_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen, int index)
{
    unsigned char temp[3];
    temp[0] = priv->rm.measure_token[index];
    if(priv->rm.measure_result[index] == MEASUREMENT_REFUSED)
    {
        temp[1] = BIT2;
    }
    else if(priv->rm.measure_result[index] == MEASUREMENT_INCAPABLE)
    {
        temp[1] = BIT1;
    }
    else
    {
        temp[1] = 0;
    }
    temp[2] = priv->rm.measure_type[index];
    pbuf = set_ie(pbuf, _MEASUREMENT_REPORT_IE_, 3, temp, frlen);
    return pbuf;

}

unsigned char * construct_rm_enable_cap_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen)
{
    unsigned char temp[5];
    temp[0] = temp[1] = temp[2] = temp[3] = temp[4] = 0;
    if(priv->pmib->dot11StationConfigEntry.dot11RMLinkMeasurementActivated)
    {
        temp[0] |= BIT0;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMNeighborReportActivated)
    {
        temp[0] |= BIT1;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconPassiveMeasurementActivated)
    {
        temp[0] |= BIT4;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconActiveMeasurementActivated)
    {
        temp[0] |= BIT5;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMBeaconTableMeasurementActivated)
    {
        temp[0] |= BIT6;
    }

    if(priv->pmib->dot11StationConfigEntry.dot11RMAPChannelReportActivated)
    {
        temp[2] |= BIT0;
    }

    pbuf = set_ie(pbuf, _RM_ENABLE_CAP_IE_, 5, temp, frlen);
    return pbuf;
}

static int __issue_measurement_report(struct rtl8192cd_priv *priv, struct stat_info* pstat, int *measure_index)
{
    unsigned char   *pbuf;
    unsigned int frlen;
    unsigned char full = 0;
    int ret;
    DECLARE_TXINSN(txinsn);

    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;
    txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
    #ifndef TX_LOWESTRATE
    txinsn.lowest_tx_rate = txinsn.tx_rate;
    #endif
    txinsn.fixed_rate = 1;
#ifdef CONFIG_IEEE80211W		
	if(pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif		
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
    if (pbuf == NULL)
        goto issue_measure_report_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);
    if (txinsn.phdr == NULL)
        goto issue_measure_report_fail;

    memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

    pbuf[0] = _RADIO_MEASUREMENT_CATEGORY_ID_;
    pbuf[1] = _RADIO_MEASUREMENT_REPORT_ACTION_ID_;
    pbuf[2] = priv->rm.dialog_token;

    frlen = 3;
    pbuf += frlen;
    while(*measure_index < priv->rm.measure_count)
    {
        if(priv->rm.measure_result[*measure_index] == MEASUREMENT_REFUSED||
                priv->rm.measure_result[*measure_index] == MEASUREMENT_INCAPABLE)
        {

            if(frlen + 5 > MAX_REPORT_FRAME_SIZE)
            {
                break;
            }
            pbuf = construct_measure_report_ie(priv, pbuf, &frlen, *measure_index);
        }
        else if(priv->rm.measure_result[*measure_index] == MEASUREMENT_SUCCEED)
        {
            switch(priv->rm.measure_type[*measure_index])
            {
                case MEASUREMENT_TYPE_BEACON:
                    pbuf = construct_beacon_report_ie(priv, pbuf, &frlen, priv->rm.measure_token[*measure_index], &full);
                    break;
                default:
                    break;
            }

            if(full)
            {
                break;
            }
        }

        (*measure_index)++;
    }

    txinsn.fr_len = frlen;

    SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif	
    memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->cmn_info.mac_addr, MACADDRLEN);
    memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
    memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
    ret = check_dz_mgmt(priv, pstat, &txinsn);
    if (ret < 0)
        goto issue_measure_report_fail;
    else if (ret==1)
        return 0;
    else
#endif
    if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return 0;

issue_measure_report_fail:

    if (txinsn.phdr)
        release_wlanhdr_to_poll(priv, txinsn.phdr);
    if (txinsn.pframe)
        release_mgtbuf_to_poll(priv, txinsn.pframe);
    return -1;
}

static void issue_measurement_report(struct rtl8192cd_priv *priv, struct stat_info* pstat)
{
    int measure_index = 0;
    while(measure_index < priv->rm.measure_count)
    {
        __issue_measurement_report(priv, pstat, &measure_index);
    }

}

static enum MEASUREMENT_RESULT rm_do_measure(struct rtl8192cd_priv *priv, int idx)
{
	enum MEASUREMENT_RESULT ret = MEASUREMENT_UNKNOWN;

	switch (priv->rm.measure_type[idx]) {
		case MEASUREMENT_TYPE_BEACON:
			ret = rm_do_beacon_measure(priv);
			break;
		default:
			break;
	}

	return ret;
}

static void rm_do_next_measure(struct rtl8192cd_priv *priv)
{
	unsigned long flag;
	int i;

	SAVE_INT_AND_CLI(flag);
	SMP_LOCK(flag);

	for (i = 0; i < priv->rm.measure_count; i++) {
		if (priv->rm.measure_result[i] == MEASUREMENT_PROCESSING) {
			priv->rm.measure_result[i] = rm_do_measure(priv, i);

			if (priv->rm.measure_result[i] == MEASUREMENT_PROCESSING) {
				/*some measurement is processing*/
				break;
			}
		}
	}

	if (i >= priv->rm.measure_count) {
		/* all measurement is finished*/
		issue_measurement_report(priv, priv->rm.req_pstat);
		priv->rm.req_pstat = NULL;
	}

	SMP_UNLOCK(flag);
	RESTORE_INT(flag);
}

static void rm_check_result(struct rtl8192cd_priv *priv)
{
    unsigned int rand_time;
    unsigned char need_process = 0;
    int i;
    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
        {
            need_process = 1;
            break;
        }
    }

    if(need_process)
    {
        if(priv->rm.measure_interval[i])   /*need random delay*/
        {
            SET_PSEUDO_RANDOM_NUMBER(rand_time);
            rand_time %= priv->rm.measure_interval[i];
            rtk_mod_timer(&priv->rm.delay_measure_timer,jiffies + RTL_MILISECONDS_TO_JIFFIES(rand_time));
        }
        else
        {
            rm_do_next_measure(priv);
        }
    }
    else
    {
        /* all measurement is finished*/
        issue_measurement_report(priv, priv->rm.req_pstat);
        priv->rm.req_pstat = NULL;
    }

}



void rm_done(struct rtl8192cd_priv *priv, unsigned char measure_type, unsigned char check_result)
{
    int i;
    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(measure_type == priv->rm.measure_type[i])
        {
            priv->rm.measure_result[i] = MEASUREMENT_SUCCEED;
            break;
        }
    }

    if(check_result)
        rm_check_result(priv);
}

void rm_terminate(struct rtl8192cd_priv *priv)
{
    int i;
    if (rtk_timer_pending(&priv->rm.delay_measure_timer))
        rtk_del_timer_sync(&priv->rm.delay_measure_timer);

    for(i = 0; i < priv->rm.measure_count; i++)
    {
        if(priv->rm.measure_result[i] == MEASUREMENT_PROCESSING)
        {
            switch(priv->rm.measure_type[i])
            {
                case MEASUREMENT_TYPE_BEACON:
                    priv->rm.measure_result[i] = rm_terminate_beacon_measure(priv);
                    break;
                default:
                    priv->rm.measure_result[i] = MEASUREMENT_UNKNOWN;
                    break;
            }
        }
    }
}

#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
static void rm_delay_measure_timer_callback(unsigned long task_priv)
#else
static __always_inline void rm_delay_measure_timer_callback(unsigned long task_priv)
#endif
{
	unsigned long flag;
	int i;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	SAVE_INT_AND_CLI(flag);
	SMP_LOCK(flag);

	for (i = 0; i < priv->rm.measure_count; i++) {
		if (priv->rm.measure_result[i] == MEASUREMENT_PROCESSING) {
			priv->rm.measure_result[i] = rm_do_measure(priv, i);

			if (priv->rm.measure_result[i] == MEASUREMENT_PROCESSING) {
				/*some measurement is processing*/
				break;
			}
		}
	}

	if (i >= priv->rm.measure_count) {
		/* all measurement is finished*/
		issue_measurement_report(priv, priv->rm.req_pstat);
		priv->rm.req_pstat = NULL;
	}

	SMP_UNLOCK(flag);
	RESTORE_INT(flag);
}
DEFINE_TIMER_CALLBACK(rm_delay_measure_timer_callback);

void rm_init_delay_measure_timer(struct rtl8192cd_priv *priv)
{
#if defined(CONFIG_PCI_HCI)
	if (GET_HCI_TYPE(priv) == RTL_HCI_PCIE) {
		rtk_timer_setup(&(priv->rm.delay_measure_timer), rm_delay_measure_timer_callback, (unsigned long)priv, 0);
	} else
#endif
#if defined(CONFIG_USB_HCI) || defined(CONFIG_SDIO_HCI)
	{
		rtk_timer_setup(&(priv->rm.delay_measure_timer), timer_event_timer_fn, (unsigned long)(&(priv->rm.delay_measure_timer_event)), 0);
		INIT_TIMER_EVENT_ENTRY(&priv->rm.delay_measure_timer_event, rm_delay_measure_timer_callback, (unsigned long)priv);
	}
#endif
	;
}

void OnRadioMeasurementRequest(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe,
                               int frame_len)
{
    int len;
    unsigned char element_id;
    unsigned char element_len;
    unsigned char incapable_all = 0;
    enum MEASUREMENT_RESULT result = MEASUREMENT_PROCESSING;

    /*if previous measurement is on-going*/
    if(priv->rm.req_pstat)
    {
        if(priv->rm.req_pstat != pstat || priv->rm.dialog_token != pframe[2])
        {
            rm_terminate(priv);
            issue_measurement_report(priv, priv->rm.req_pstat);
            priv->rm.req_pstat = NULL;
        }
        else
            return;
    }


    /*parsing radio measurment request frame*/
    priv->rm.req_pstat = pstat;
    priv->rm.dialog_token = pframe[2]; /*dialog token*/
    if(pframe[3] != 0 || pframe[4] != 0)  /* repetition != 0 */
    {
        incapable_all = 1 ;
    }
    len = 5;

    priv->rm.measure_count = 0;
    while(len + 2 <= frame_len)
    {
        element_id = pframe[len];
        element_len = pframe[len + 1];

        /*parsing every radio measurment request element*/
        if(element_id == _MEASUREMENT_REQUEST_IE_)
        {
            result = MEASUREMENT_PROCESSING;
            priv->rm.measure_token[priv->rm.measure_count] = pframe[len + 2];
            priv->rm.measure_type[priv->rm.measure_count] = pframe[len + 4];

            if(pframe[len + 3] || incapable_all) /*check measurement mode*/
            {
                result = MEASUREMENT_INCAPABLE;
            }
            else
            {
                switch(priv->rm.measure_type[priv->rm.measure_count])
                {
                    case MEASUREMENT_TYPE_BEACON:
                        result = rm_parse_beacon_request(priv, pframe, len + 5, len + 2 + element_len, &priv->rm.measure_interval[priv->rm.measure_count]);
                        break;
                    default:
                        result = MEASUREMENT_INCAPABLE;
                        break;

                }
            }

            priv->rm.measure_result[priv->rm.measure_count] = result;
            priv->rm.measure_count++;
            if(priv->rm.measure_count >= MAX_MEASUREMENT_REQUEST)
                break;
        }
        len += 2 + element_len;
    }

    rm_check_result(priv);
    return;
}



void OnRadioMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat,
                              unsigned char *pframe, int frame_len)
{
    int len;
    unsigned char element_id;
    unsigned char element_len;

    /*the report from iphone 6 always has dialog token 0, thus do not check dialog token when the value is 0*/
    if((pframe[2] != 0 && pstat->rm.dialog_token != pframe[2]))// || pstat->rm.measure_result != MEASUREMENT_PROCESSING)
    {
        return;
    }

    len = 3;
    while(len + 5 <= frame_len)
    {
        element_id = pframe[len];
        element_len = pframe[len + 1];
        /*parsing every radio measurment report element*/
        if(element_id == _MEASUREMENT_REPORT_IE_)
        {
            if(pframe[len + 4] == MEASUREMENT_TYPE_BEACON)
            {
                len = rm_parse_beacon_report(priv, pstat, pframe, len, frame_len);
                continue;
            }
        }
        len += 2 + element_len;
    }
	
	if (pstat->rm.measure_result == MEASUREMENT_RECEIVING)
		pstat->rm.measure_result = MEASUREMENT_RECEIVED;
		
#ifdef RTK_MULTI_AP
	if(pstat->rm.beacon_report_num)
		rtk_multi_ap_beaconMetricsResponseNotify(pstat);
#endif

}

#ifdef CLIENT_MODE
unsigned char * construct_WFA_TPC_report_ie(struct rtl8192cd_priv *priv, unsigned char	*pbuf, unsigned int *frlen)
{
    unsigned char temp[7];    
    memcpy(temp, wfa_oui, 3); /*oui*/    
    temp[3] = 0x08; /*oui type */    
    temp[4] = 0x00; /*oui subtype */    
    temp[5] = priv->pmib->dot11hTPCEntry.tpc_tx_power; /*transmit power*/    
    temp[6] = 0;    /*link margin, should be 0*/
    pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, 7, temp, frlen);
    return pbuf;
}
#endif

void sm_cca_expire(struct stat_info *pstat)
{
	if ((pstat->cca_link_time_bkup) && (pstat->link_time - pstat->cca_link_time_bkup >= 5)) {
		if (pstat->sm.cca_result == MEASUREMENT_RECEIVED) {
			pstat->sm.cca_result = MEASUREMENT_SUCCEED;
		}
		else if (pstat->sm.cca_result == MEASUREMENT_PROCESSING) {
			// DEBUG_INFO("Change cca_result from PROCESSING to UNKNOWN !!!\n");
			pstat->sm.cca_result = MEASUREMENT_UNKNOWN;
		}

		pstat->cca_link_time_bkup = 0;
	}
}

static unsigned char * construct_cca_request_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen,
        unsigned char token, struct dot11k_cca_measurement_req * cca_req)
{
	unsigned char temp[30];
	unsigned int len;
	unsigned long long tsf = 0;

	printk("[%s] Start!\n", __FUNCTION__);

	temp[0] = token;
	temp[1] = 0; /* measurement request mode */
	temp[2] = MEASUREMENT_TYPE_CCA; /* measurement type */

	temp[3] = cca_req->channel; /* channel number */
#if defined(CONFIG_WLAN_HAL_8814BE)
	if (GET_CHIP_VER(priv) == VERSION_8814B) {
		tsf = RTL_R32(REG_TSFTR_LOW_8814B+4);
		tsf = (tsf<<32) + (RTL_R32(REG_TSFTR_LOW_8814B));
	}
	else
#endif
	{
		tsf = RTL_R32(TSFTR+4);
		tsf = (tsf<<32) + (RTL_R32(TSFTR));
	}
	cca_req->start_time = tsf;
	*(unsigned long long *)(temp + 4) = cpu_to_le64(cca_req->start_time); /* start time */
	*(unsigned short *)(temp + 12) = cpu_to_le16(cca_req->measure_duration); /* measurement duration */
	len = 14;

	pbuf = set_ie(pbuf, _MEASUREMENT_REQUEST_IE_, len, temp, frlen);

	return pbuf;
}

static int issue_cca_measurement_request(struct rtl8192cd_priv *priv, struct stat_info *pstat, 
	struct dot11k_cca_measurement_req* cca_req)
{
	unsigned char *pbuf;
	unsigned int frlen;
	int ret;

	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;    
#endif
	txinsn.fixed_rate = 1;
#ifdef CONFIG_IEEE80211W		
	if (pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif		

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_cca_request_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_cca_request_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _SPECTRUM_MANAGEMENT_CATEGORY_ID_;
	pbuf[1] = _MEASUREMENT_REQUEST_ACTION_ID_;

	if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
		pstat->dialog_token++;

	pstat->sm.cca_dialog_token = pstat->dialog_token;
	pbuf[2] = pstat->dialog_token;
	frlen = 3;

	pbuf = construct_cca_request_ie(priv, pbuf + frlen, &frlen, 1, cca_req);
	txinsn.fr_len = frlen;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->cmn_info.mac_addr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
    
	if (ret < 0)
		goto issue_cca_request_fail;
	else if (ret == 1)
		return 0;
	else
#endif
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return 0;

issue_cca_request_fail:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);

	return -1;
}

int sm_cca_measurement_request(struct rtl8192cd_priv *priv, unsigned char *macaddr, struct dot11k_cca_measurement_req* cca_req)
{
	int measure_time;
	int ret = -1;
	struct stat_info *pstat;
	int i, total_channel_num = 0;

#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	if (priv->pmib->dot11StationConfigEntry.dot11SMMeasurementActivated)
	{
		pstat = get_stainfo(priv, macaddr);
		if (pstat)
		{
			if (pstat->expire_to == 0)
				goto cca_req_fail;

			/* check if measurement is on-going */
			if (pstat->sm.cca_result == MEASUREMENT_PROCESSING) {
				panic_printk("[%s] cca_result is already PROCESSING\n", __FUNCTION__);	
				goto cca_req_fail;
			}

			memcpy(&pstat->sm.cca_req, cca_req, sizeof(struct dot11k_cca_measurement_req));
			ret = issue_cca_measurement_request(priv, pstat, &pstat->sm.cca_req);
			if (ret == 0)   /*issue cca measurement request succeed*/
			{
				pstat->sm.cca_result = MEASUREMENT_PROCESSING;
			}

			pstat->cca_link_time_bkup = pstat->link_time + 1;
		}
	}

cca_req_fail:
#ifdef SDIO_AP_OFFLOAD
	ap_offload_activate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	return ret;
}

int sm_get_cca_report(struct rtl8192cd_priv *priv, unsigned char *macaddr, unsigned char *result_buf)
{
	int len = -1;
	struct stat_info *pstat;

#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	if (priv->pmib->dot11StationConfigEntry.dot11SMMeasurementActivated)
	{
		pstat = get_stainfo(priv, macaddr);
		if (pstat)
		{
			*result_buf = pstat->sm.cca_result;
			len = 1;

			if (pstat->sm.cca_result == MEASUREMENT_SUCCEED)
			{
				memcpy(result_buf + 1, &pstat->sm.cca_report, sizeof(struct dot11k_cca_measurement_report));
				len += sizeof(struct dot11k_cca_measurement_report);
				pstat->sm.cca_result = MEASUREMENT_UNKNOWN;
			}
			else
				panic_printk("[%s] cca_result is NOT success (%d)\n", __FUNCTION__, pstat->sm.cca_result);
		}
	}

#ifdef SDIO_AP_OFFLOAD
	ap_offload_activate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	return len;
}

void OnCCAMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe)
{
	int len;

	/*the report from iphone 6 always has dialog token 0, thus do not check dialog token when the value is 0*/
	if ((pframe[2] != 0 && pstat->sm.cca_dialog_token != pframe[2]))// || pstat->sm.cca_result != MEASUREMENT_PROCESSING)
		return;

	pstat->sm.cca_result = MEASUREMENT_RECEIVED;
	len = 3;

	if (pframe[len] == _MEASUREMENT_REPORT_IE_)
	{
		if (pframe[len + 4] == MEASUREMENT_TYPE_CCA)
		{
			if (pframe[len + 3] == 0)  /*succeed*/
			{
				pstat->sm.cca_report.channel = pframe[len + 5];
				pstat->sm.cca_report.start_time = le64_to_cpu(*(unsigned int *)&pframe[len + 6]);
				pstat->sm.cca_report.measure_duration = le16_to_cpu(*(unsigned short *)&pframe[len + 14]);
				pstat->sm.cca_report.cca_busy_fraction = pframe[len + 16];

				pstat->sm.cca_result = MEASUREMENT_SUCCEED;
			}
			else if (pframe[len + 3] & BIT1)   /*incapable*/
			{
				printk("Receive cca_report: MEASUREMENT_INCAPABLE\n");
				pstat->sm.cca_result = MEASUREMENT_INCAPABLE;
			}
			else if (pframe[len + 3] & BIT2)  /*refused*/
			{
				printk("Receive cca_report: MEASUREMENT_REFUSED\n");
				pstat->sm.cca_result = MEASUREMENT_REFUSED;
			}
		}
	}
}

void sm_rpi_expire(struct stat_info *pstat)
{
	if ((pstat->rpi_link_time_bkup) && (pstat->link_time - pstat->rpi_link_time_bkup >= 5)) {
		if (pstat->sm.rpi_result == MEASUREMENT_RECEIVED) {
			pstat->sm.rpi_result = MEASUREMENT_SUCCEED;
		}
		else if (pstat->sm.rpi_result == MEASUREMENT_PROCESSING) {
			// DEBUG_INFO("Change rpi_result from PROCESSING to UNKNOWN !!!\n");
			pstat->sm.rpi_result = MEASUREMENT_UNKNOWN;
		}

		pstat->rpi_link_time_bkup = 0;
	}
}

static unsigned char * construct_rpi_request_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen,
        unsigned char token, struct dot11k_rpi_measurement_req * rpi_req)
{
	unsigned char temp[30];
	unsigned int len;
	unsigned long long tsf = 0;

	temp[0] = token;
	temp[1] = 0; /* measurement request mode */
	temp[2] = MEASUREMENT_TYPE_RPI; /* measurement type */

	temp[3] = rpi_req->channel; /* channel number */
#if defined(CONFIG_WLAN_HAL_8814BE)
	if (GET_CHIP_VER(priv) == VERSION_8814B) {
		tsf = RTL_R32(REG_TSFTR_LOW_8814B+4);
		tsf = (tsf<<32) + (RTL_R32(REG_TSFTR_LOW_8814B));
	}
	else
#endif
	{
		tsf = RTL_R32(TSFTR+4);
		tsf = (tsf<<32) + (RTL_R32(TSFTR));
	}
	rpi_req->start_time = tsf;
	*(unsigned long long *)(temp + 4) = cpu_to_le64(rpi_req->start_time); /* start time */
	*(unsigned short *)(temp + 12) = cpu_to_le16(rpi_req->measure_duration); /* measurement duration */
	len = 14;

	pbuf = set_ie(pbuf, _MEASUREMENT_REQUEST_IE_, len, temp, frlen);

	return pbuf;
}

static int issue_rpi_measurement_request(struct rtl8192cd_priv *priv, struct stat_info *pstat, 
	struct dot11k_rpi_measurement_req* rpi_req)
{
	unsigned char *pbuf;
	unsigned int frlen;
	int ret;

	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;    
#endif
	txinsn.fixed_rate = 1;
#ifdef CONFIG_IEEE80211W		
	if (pstat)
		txinsn.isPMF = pstat->isPMF;
	else
		txinsn.isPMF = 0;	
#endif		

	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_rpi_request_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_rpi_request_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _SPECTRUM_MANAGEMENT_CATEGORY_ID_;
	pbuf[1] = _MEASUREMENT_REQUEST_ACTION_ID_;

	if (!(++pstat->dialog_token))	// dialog token set to a non-zero value
		pstat->dialog_token++;

	pstat->sm.rpi_dialog_token = pstat->dialog_token;
	pbuf[2] = pstat->dialog_token;
	frlen = 3;

	pbuf = construct_rpi_request_ie(priv, pbuf + frlen, &frlen, 1, rpi_req);
	txinsn.fr_len = frlen;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char*)(txinsn.phdr+1) |= BIT(6); // enable privacy 
#endif
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), pstat->cmn_info.mac_addr, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

#if defined(WIFI_WMM)
	ret = check_dz_mgmt(priv, pstat, &txinsn);
    
	if (ret < 0)
		goto issue_rpi_request_fail;
	else if (ret == 1)
		return 0;
	else
#endif
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		return 0;

issue_rpi_request_fail:
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);

	return -1;
}

int sm_rpi_measurement_request(struct rtl8192cd_priv *priv, unsigned char *macaddr, struct dot11k_rpi_measurement_req* rpi_req)
{
	int measure_time;
	int ret = -1;
	struct stat_info *pstat;
	int i, total_channel_num = 0;

#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	if (priv->pmib->dot11StationConfigEntry.dot11SMMeasurementActivated)
	{
		pstat = get_stainfo(priv, macaddr);
		if (pstat)
		{
			if (pstat->expire_to == 0)
				goto rpi_req_fail;

			/* check if measurement is on-going */
			if (pstat->sm.rpi_result == MEASUREMENT_PROCESSING) {
				panic_printk("[%s] rpi_result is already PROCESSING\n", __FUNCTION__);	
				goto rpi_req_fail;
			}

			memcpy(&pstat->sm.rpi_req, rpi_req, sizeof(struct dot11k_rpi_measurement_req));
			ret = issue_rpi_measurement_request(priv, pstat, &pstat->sm.rpi_req);
			if (ret == 0)   /*issue rpi measurement request succeed*/
			{
				pstat->sm.rpi_result = MEASUREMENT_PROCESSING;
			}

			pstat->rpi_link_time_bkup = pstat->link_time + 1;
		}
	}

rpi_req_fail:
#ifdef SDIO_AP_OFFLOAD
	ap_offload_activate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	return ret;
}

int sm_get_rpi_report(struct rtl8192cd_priv *priv, unsigned char *macaddr, unsigned char *result_buf)
{
	int len = -1;
	struct stat_info *pstat;

#ifdef SDIO_AP_OFFLOAD
	ap_offload_deactivate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	if (priv->pmib->dot11StationConfigEntry.dot11SMMeasurementActivated)
	{
		pstat = get_stainfo(priv, macaddr);
		if (pstat)
		{
			*result_buf = pstat->sm.rpi_result;
			len = 1;

			if (pstat->sm.rpi_result == MEASUREMENT_SUCCEED)
			{
				memcpy(result_buf + 1, &pstat->sm.rpi_report, sizeof(struct dot11k_rpi_measurement_report));
				len += sizeof(struct dot11k_rpi_measurement_report);
				pstat->sm.rpi_result = MEASUREMENT_UNKNOWN;
			}
			else
				panic_printk("[%s] rpi_result is NOT success (%d)\n", __FUNCTION__, pstat->sm.rpi_result);
		}
	}

#ifdef SDIO_AP_OFFLOAD
	ap_offload_activate(priv, OFFLOAD_PROHIBIT_RADIO_MEASURE);
#endif

	return len;
}

void OnRPIMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe)
{
	int len;

	/*the report from iphone 6 always has dialog token 0, thus do not check dialog token when the value is 0*/
	if ((pframe[2] != 0 && pstat->sm.rpi_dialog_token != pframe[2]))// || pstat->sm.rpi_result != MEASUREMENT_PROCESSING)
		return;

	pstat->sm.rpi_result = MEASUREMENT_RECEIVED;
	len = 3;

	if (pframe[len] == _MEASUREMENT_REPORT_IE_)
	{
		if (pframe[len + 4] == MEASUREMENT_TYPE_RPI)
		{
			if (pframe[len + 3] == 0)  /*succeed*/
			{
				pstat->sm.rpi_report.channel = pframe[len + 5];
				pstat->sm.rpi_report.start_time = le64_to_cpu(*(unsigned int *)&pframe[len + 6]);
				pstat->sm.rpi_report.measure_duration = le16_to_cpu(*(unsigned short *)&pframe[len + 14]);
				memcpy(pstat->sm.rpi_report.rpi_density, pframe + len + 16, 8);

				pstat->sm.rpi_result = MEASUREMENT_SUCCEED;
			}
			else if (pframe[len + 3] & BIT1)   /*incapable*/
			{
				printk("Receive rpi_report: MEASUREMENT_INCAPABLE\n");
				pstat->sm.rpi_result = MEASUREMENT_INCAPABLE;
			}
			else if (pframe[len + 3] & BIT2)  /*refused*/
			{
				printk("Receive rpi_report: MEASUREMENT_REFUSED\n");
				pstat->sm.rpi_result = MEASUREMENT_REFUSED;
			}
		}
	}
}

void OnSpectrumMgmtMeasurementReport(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned char *pframe)
{
	if (pframe[7] == MEASUREMENT_TYPE_CCA)
		OnCCAMeasurementReport(priv, pstat, pframe);
	else if (pframe[7] == MEASUREMENT_TYPE_RPI)
		OnRPIMeasurementReport(priv, pstat, pframe);
	else
		panic_printk("Unknown type of Measurement Report (%d) !!\n", pframe[7]);
}

#endif
