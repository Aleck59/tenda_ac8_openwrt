#ifndef _PE_IPC_H_
#define _PE_IPC_H_

/*
*	Host send to target command
*/
typedef enum {
        WFO_IPC_H2T_INIT = 0,
        WFO_IPC_H2T_START_PROCESS = 1,
        WFO_IPC_H2T_STOP_PROCESS = 2,
        WFO_IPC_H2T_PKT_RECYCLE = 3,
        WFO_IPC_H2T_STA_ADD = 4,
        WFO_IPC_H2T_STA_DEL = 5,
        WFO_IPC_H2T_VAR_SET = 6,
        WFO_IPC_H2T_MAX = 7,
} ca_wfo_ipc_h2t_t;

/*
*	Target send to host command
*/
typedef enum {
		WFO_IPC_T2H_PE_READY = 0,
        WFO_IPC_T2H_PKT_RECV = 1,
        WFO_IPC_T2H_INFO_REPORT = 2,
        WFO_IPC_T2H_FUNC = 3,
        WFO_IPC_T2H_DEBUG = 4,
        WFO_IPC_T2H_MAX = 5,
} ca_wfo_ipc_t2h_t;

typedef enum {
	CMD_INIT_TX,
	CMD_START_PROCESS,
	CMD_STOP_PROCESS,
	CMD_ADD_STA,
	CMD_DEL_STA,
	CMD_SET_VAR,
}IPC_TX_CMD_TYPE;

struct ipc_tx_ctl_s{
	IPC_TX_CMD_TYPE name;
	void (* send_ipc)(unsigned char *,unsigned int);
};

int register_wfo_ipc(void);
void rtl_ipc_tx_handler(IPC_TX_CMD_TYPE action,unsigned char *ipc_content,unsigned int buf_len);

#endif