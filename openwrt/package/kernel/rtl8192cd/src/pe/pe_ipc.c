#include <linux/kernel.h>

#include <8192cd.h>
#include <8192cd_cfg.h>

#include <ca_types.h>
#include <ca_ipc_pe.h>

#include <pe_fw.h>
#include <pe_ipc.h>

#include <core/8192cd_core_rx.h>

/*
*	RX CMD path
*/
extern ca_status_t ca_ipc_msg_handle_register(ca_ipc_session_id_t session_id,
	const ca_ipc_msg_handle_t * msg_handle_array,ca_uint32_t msg_handle_count);

/*
*	TX CMD path
*/
extern ca_status_t ca_ipc_msg_async_send(ca_ipc_pkt_t* p_ipc_pkt);
extern ca_status_t ca_ipc_msg_sync_send(ca_ipc_pkt_t* p_ipc_pkt,void * result_data,ca_uint16_t * result_size);

int tx_init_cmd(void);
int tx_start_process_cmd(void);
int tx_stop_process_cmd(void);
int tx_pkt_recycle_cmd(unsigned char *buf,unsigned int buf_len);
int tx_sta_add_cmd(unsigned char *buf,unsigned int buf_len);
int tx_sta_del_cmd(unsigned char *buf,unsigned int buf_len);
int tx_var_set_cmd(void);

#if defined(CONFIG_PE_ENABLE)

static struct ipc_tx_ctl_s ipc_tx_ctl[]={
	{CMD_INIT_TX,tx_init_cmd},
	{CMD_START_PROCESS,tx_start_process_cmd},
	{CMD_STOP_PROCESS,tx_stop_process_cmd},
	{CMD_ADD_STA,tx_sta_add_cmd},
	{CMD_DEL_STA,tx_sta_del_cmd},
	{CMD_SET_VAR,tx_var_set_cmd},
};


void rtl_ipc_rx_handler(unsigned char *ipc_skb_ptr)
{
	//struct rtl8192cd_priv *priv = wlan_device[0].priv;	//hardcode for temporary
	struct rtl8192cd_priv *priv = ipc_8814b_priv;	//hardcode for temporary
	struct sk_buff	*pskb = NULL;
		
	pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 1);
	if(pskb == NULL){
		printk("%s : error !! \n",__func__);
		return;
	}
	
	
	pskb->data = ipc_skb_ptr;

	rtl88XX_ipc_rx_isr(priv, pskb);
}

void rtl_ipc_tx_handler(IPC_TX_CMD_TYPE action,unsigned char *ipc_content,unsigned int buf_len)
{
	ipc_tx_ctl[action].send_ipc(ipc_content,buf_len);
}

int tx_init_cmd(void)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_INIT;
    send_date.msg_size = 0;
    send_date.msg_data = 0;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_start_process_cmd(void)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_START_PROCESS;
    send_date.msg_size = 0;
    send_date.msg_data = 0;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_stop_process_cmd(void)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STOP_PROCESS;
    send_date.msg_size = 0;
    send_date.msg_data = 0;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}


int tx_pkt_recycle_cmd(unsigned char *buf, unsigned int buf_len)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_PKT_RECYCLE;
    send_date.msg_size = buf_len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_sta_add_cmd(unsigned char *buf,unsigned int buf_len)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STA_ADD;
    send_date.msg_size = buf_len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_sta_del_cmd(unsigned char *buf,unsigned int buf_len)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STA_DEL;
    send_date.msg_size = buf_len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_var_set_cmd(void)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_VAR_SET;
    send_date.msg_size = 0;
    send_date.msg_data = 0;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}


int rx_pe_ready_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	char byestr[]={"Bye from ARM , nice to meet you"};
	int size_str = sizeof(byestr);
	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	memcpy(msg_data,byestr,size_str);
	*msg_size = size_str;
	return 0;
}

int rx_pkt_recv_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	unsigned int skb[1];
	int phys_mem_shift = 0;
//	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

//	panic_printk("skb = %x, skb->data = %x, skb->len = %d\r\n", ((int*)msg_data)[0], ((int*)msg_data)[1], ((int*)msg_data)[2]);

	// < skb virt addr || skb->data phys addr || skb->len > 
	
//	panic_printk("mem virt addr = %x\r\n", get_pe_fw_virt_addr());
//	panic_printk("mem phys addr = %x\r\n", get_pe_fw_phys_addr());
	phys_mem_shift = ((int*)msg_data)[1] - (get_pe_fw_phys_addr()& 0x0FFFFFFF);
//	panic_printk("phys shift = %x\r\n", phys_mem_shift);
	
//	panic_printk("content of skb->data = %s\r\n", (char*)(get_pe_fw_virt_addr() + phys_mem_shift));

	//MEMORY COPY
	unsigned char *data  = (char*)(get_pe_fw_virt_addr() + phys_mem_shift);
	
	rtl_ipc_rx_handler(data);
	//RECYCLE
	skb[0] = ((int*)msg_data)[0];
	tx_pkt_recycle_cmd(skb, sizeof(skb));

	return 0;
}

int rx_info_report_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	char byestr[]={"Bye from ARM , nice to meet you"};
	int size_str = sizeof(byestr);
	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	memcpy(msg_data,byestr,size_str);
	*msg_size = size_str;
	return 0;
}

int rx_func_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	char byestr[]={"Bye from ARM , nice to meet you"};
	int size_str = sizeof(byestr);
	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	memcpy(msg_data,byestr,size_str);
	*msg_size = size_str;
	return 0;
}

int rx_debug_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	char byestr[]={"Bye from ARM , nice to meet you"};
	int size_str = sizeof(byestr);
	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	memcpy(msg_data,byestr,size_str);
	*msg_size = size_str;
	return 0;
}


ca_ipc_msg_handle_t invoke_procs_wfo[]=
{
	{ .msg_no=0, .proc=rx_pe_ready_cmd },
	{ .msg_no=1, .proc=rx_pkt_recv_cmd },
	{ .msg_no=2, .proc=rx_info_report_cmd },
	{ .msg_no=3, .proc=rx_func_cmd },
	{ .msg_no=4, .proc=rx_debug_cmd },
};

int register_wfo_ipc(void)
{
	int rc;
	//ca_uint16_t result_size = 0x10;
	panic_printk("Register IPC \r\n");
	rc= ca_ipc_msg_handle_register(CA_IPC_SESSION_WFO, invoke_procs_wfo, 5);
	if( CA_E_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}
	return 0;
}
#endif
