#if defined(THERMAL_CONTROL_DAEMON)
#ifdef THERMAL_CONTROL_DEFAULT_VALUE
struct ther_setting_s {
	unsigned int  id;
	unsigned char ext_pa;

	unsigned char ther_low;
	unsigned char ther_hi;
	unsigned char ther_max;

	unsigned char ther_by_funcoff;
	unsigned char ther_by_txduty;
	unsigned char ther_by_path;
	unsigned char ther_by_power;
};

struct ther_setting_s ther_default[] = {
    /*id,             epa,  low,   hi,    max,   funcoff,  txduty,   path,     power*/
    { VERSION_8814B,  0,    40,    43,    45,    0,        1,        1,        1 },
    { VERSION_8198F,  0,    39,    44,    48,    1,        0,        0,        0 },
    { VERSION_8812F,  1,    44,    47,    53,    1,        0,        0,        0 },
    { VERSION_8812F,  0,    48,    51,    57,    1,        0,        0,        0 },
    { VERSION_8192F,  0,    51,    56,    62,    1,        0,        0,        0 },
};

void apply_ther_setting(struct rtl8192cd_priv * priv, struct ther_setting_s *val)
{
	priv->pshare->rf_ft_var.ther_hi = val->ther_hi;
	priv->pshare->rf_ft_var.ther_low = val->ther_low;
	priv->pshare->rf_ft_var.ther_max = val->ther_max;

	priv->pshare->rf_ft_var.ther_by_funcoff = val->ther_by_funcoff;
	priv->pshare->rf_ft_var.ther_by_txduty = val->ther_by_txduty;
	priv->pshare->rf_ft_var.ther_by_path = val->ther_by_path;
	priv->pshare->rf_ft_var.ther_by_power = val->ther_by_power;
}

void init_ther_setting(struct rtl8192cd_priv * priv)
{
	int i, idx = -1, num = sizeof(ther_default)/sizeof(struct ther_setting_s);
	int use_ext_pa = 0;

#ifdef HIGH_POWER_EXT_PA
	use_ext_pa = priv->pshare->rf_ft_var.use_ext_pa;
#endif

	for (i=0; i<num; i++) {
		if (GET_CHIP_VER(priv) == ther_default[i].id && use_ext_pa == ther_default[i].ext_pa) {
			idx = i;
			break;
		}
	}

	if (idx >=0 && idx < num) {
		apply_ther_setting(priv, &ther_default[idx]);
	}
	else {
		printk("==> %s:%d, unknown\n", __func__, __LINE__);
	}
}
#endif /* THERMAL_CONTROL_DEFAULT_VALUE */
//-----------------------------------------------------------------------------------------

void therctl_update_ther(struct rtl8192cd_priv * priv, int enable)
{
	extern void update_del_ther(struct rtl8192cd_priv *priv, int enable);
	//therctl_print("==> %s, dev[%s], enable[%d]\n", __func__, priv->dev->name, enable);

	update_del_ther(priv, enable);
}

void therctl_limit_tp(struct rtl8192cd_priv * priv, int level)
{
	extern void decide_limit_tp_new(struct rtl8192cd_priv *priv);
	extern void cancel_limit_tp(struct rtl8192cd_priv *priv);
	therctl_print("==> %s, dev[%s], level[%d]\n", __func__, priv->dev->name, level);

	priv->pshare->rf_ft_var.txduty_level = level;

	if (level) {
		if (priv->pshare->rf_ft_var.txduty == 0)
			priv->pshare->rf_ft_var.txduty = 1;
		therctl_print("==> %s, dev[%s], level[%d] -> decide_limit_tp_new\n", __func__, priv->dev->name, level);
		decide_limit_tp_new(priv);
	}
	else {
		priv->pshare->rf_ft_var.txduty = 0;
		therctl_print("==> %s, dev[%s], level[%d] -> cancel_limit_tp\n", __func__, priv->dev->name, level);
		cancel_limit_tp(priv);
	}
}

void therctl_bandwidth(struct rtl8192cd_priv * priv, int bw)
{
	therctl_print("==> %s, dev[%s], bw[%d]\n", __func__, priv->dev->name, bw);
}

void therctl_txduty(struct rtl8192cd_priv * priv, int level)
{
	therctl_print("==> %s, dev[%s], level[%d]\n", __func__, priv->dev->name, level);

#if CFG_HAL_PWR_REDUCE
	if (GET_CHIP_VER(priv) == VERSION_8814B) {
		if (level == 0) {
			priv->pshare->rf_ft_var.fwduty = 0;
		}
		else {
			priv->pshare->rf_ft_var.fwduty = 1;
			priv->pshare->rf_ft_var.fwduty_level = level - 1;
		}

		if (priv->pshare->rf_ft_var.fwduty == 1) {
			therctl_print("apply FW Tx Duty Cycle level %d\n", priv->pshare->rf_ft_var.fwduty_level);
			TxDutyCycleLevel88XX(priv, priv->pshare->rf_ft_var.fwduty_level);
		}
		else {
			therctl_print("disable FW Tx Duty Cycle\n");
			GET_HAL_INTERFACE(priv)->SetPWRReduceHandler(priv, 0, 0, 0, 84, 4);
		}
	}
#endif
}

void therctl_power(struct rtl8192cd_priv * priv, int low_power)
{
	extern void set_power_desc(struct rtl8192cd_priv *priv);
	extern unsigned char degrade_power(struct rtl8192cd_priv *priv);
	extern unsigned char cancel_degrade_power(struct rtl8192cd_priv *priv);
	therctl_print("==> %s, dev[%s], low_power[%d]\n", __func__, priv->dev->name, low_power);
	priv->pshare->rf_ft_var.low_power = low_power;
	priv->pshare->rf_ft_var.power_desc = 1;

#if defined(CONFIG_WLAN_HAL_8814BE)
	if (GET_CHIP_VER(priv) == VERSION_8814B) {
		if ((priv->pshare->rf_ft_var.low_power == 2) && priv->pshare->rf_ft_var.power_desc) {
			therctl_print("degrade Tx Power level %d\n");
			priv->pshare->rf_ft_var.current_power_desc = priv->pshare->rf_ft_var.power_desc;
			set_power_desc(priv); /* clear shortcut cache */
		}
		else if (priv->pshare->rf_ft_var.low_power == 0) {
			therctl_print("restore Tx Power\n");
			priv->pshare->rf_ft_var.current_power_desc = 0;
			set_power_desc(priv); /* clear shortcut cache */
		}
		return;
	}
#endif

	/* 1. Tx Power*/
	if(priv->pshare->rf_ft_var.low_power == 1){
		if(degrade_power(priv)) /* degrade Tx power by descriptor per STA */
			set_power_desc(priv);
	}else if(priv->pshare->rf_ft_var.low_power == 2){ /*  set Tx power from MIB power_desc*/
		if(priv->pshare->rf_ft_var.current_power_desc != (priv->pshare->rf_ft_var.power_desc) ){
			priv->pshare->rf_ft_var.current_power_desc = priv->pshare->rf_ft_var.power_desc;
			set_power_desc(priv);
			therctl_print("Use %ddB\n", priv->pshare->rf_ft_var.current_power_desc==3?-11:priv->pshare->rf_ft_var.current_power_desc==2?-7:priv->pshare->rf_ft_var.current_power_desc==1?-3:priv->pshare->rf_ft_var.current_power_desc==0?0:100);
		}
	}else if(priv->pshare->rf_ft_var.low_power == 0){
		if(cancel_degrade_power(priv)) /* cancel degrading Tx power by descriptor per STA */
			set_power_desc(priv);
	}
}

void therctl_setpath(struct rtl8192cd_priv * priv, int path)
{
	extern void set_path(struct rtl8192cd_priv *priv);
	therctl_print("==> %s, dev[%s], path[%d]\n", __func__, priv->dev->name, path);
	priv->pshare->rf_ft_var.path = path;

	set_path(priv);
}

void therctl_funcoff(struct rtl8192cd_priv * priv, int enable)
{
	extern void ther_ctrl_set_func_off(struct rtl8192cd_priv *priv);
	extern void ther_ctrl_release_func_off(struct rtl8192cd_priv *priv);
	therctl_print("==> %s, dev[%s], enable[%d]\n", __func__, priv->dev->name, enable);

	if (enable) {
		ther_ctrl_set_func_off(priv);
	}
	else {
		ther_ctrl_release_func_off(priv);
	}
}
//-----------------------------------------------------------------------------------------

void sync_mib(struct rtl8192cd_priv * priv, void *ptr)
{
	struct ther_info_s *info = (struct ther_info_s *)ptr;

	info->control.ther_dm = priv->pshare->rf_ft_var.ther_dm;
	info->control.monitor_time = priv->pshare->rf_ft_var.monitor_time;

	info->control.dbg = priv->pshare->rf_ft_var.dbg;
	info->control.del_ther = priv->pshare->rf_ft_var.curr_ther;

#if 0//THERMAL_CONTROL_DEFAULT_VALUE
	info->control.ther_hi = priv->pshare->rf_ft_var.ther_hi;
	info->control.ther_low = priv->pshare->rf_ft_var.ther_low;
	info->control.ther_max = priv->pshare->rf_ft_var.ther_max;

	info->tp.enable = priv->pshare->rf_ft_var.ther_by_limittp;
	info->funcoff.enable = priv->pshare->rf_ft_var.ther_by_funcoff;
	info->fwduty.enable = priv->pshare->rf_ft_var.ther_by_txduty;
	info->txpath.enable = priv->pshare->rf_ft_var.ther_by_path;
	info->power.enable = priv->pshare->rf_ft_var.ther_by_power;
	info->bw.enable = priv->pshare->rf_ft_var.ther_by_bandwidth;

#ifdef CFG_HAL_PWR_REDUCE
	info->fwduty.level = priv->pshare->rf_ft_var.fwduty_level;
#endif
#endif
}

void get_therctl_val(struct rtl8192cd_priv * priv, struct ther_func_s *func)
{
	switch (func->val) {
	case THERGETVAL_ALLMIB:
		sync_mib(priv, (void *)func);
		break;

	case THERGETVAL_DELTA:
		if (GET_CHIP_VER(priv) == VERSION_8814B) {
			func->sint = priv->pshare->rf_ft_var.curr_ther;
		}
		else {
		#if 0
			func->sint = priv->pshare->rf_ft_var.del_ther;
		#else
			func->sint = priv->pshare->rf_ft_var.curr_ther;
		#endif
		}
		break;
#if 0
	case THERGETVAL_MONITOR_TIME:
		func->uint = priv->pshare->rf_ft_var.monitor_time;
		break;

	case THERGETVAL_THER_HI:
		func->uchar = priv->pshare->rf_ft_var.ther_hi;
		break;

	case THERGETVAL_THER_LOW:
		func->uchar = priv->pshare->rf_ft_var.ther_low;
		break;

	case THERGETVAL_MAN:
		func->uchar = priv->pshare->rf_ft_var.man;
		break;

	case THERGETVAL_THER_DM:
		func->uchar = priv->pshare->rf_ft_var.ther_dm;
		break;

	case THERGETVAL_DBG:
		func->uchar = priv->pshare->rf_ft_var.dbg;
		break;
#endif
	}
}

int run_therctl_func(struct rtl8192cd_priv * priv, struct ther_func_s *func)
{
	int ret = 0;
	switch (func->id) {
	case THERCTL_GETVAL:
		get_therctl_val(priv, func);
		ret = 1;
		break;

	case THERCTL_UPDATE_THER:
		therctl_update_ther(priv, func->val);
		break;

	case THERCTL_LIMIT_TP:
		therctl_limit_tp(priv, func->val);
		break;

	case THERCTL_FUNC_OFF:
		therctl_funcoff(priv, func->val);
		break;

	case THERCTL_TX_DUTY:       
		therctl_txduty(priv, func->val);
		break;

	case THERCTL_POWER:       
		therctl_power(priv, func->val);
		break;

	case THERCTL_SET_PATH:
		therctl_setpath(priv, func->val);
		break;

	case THERCTL_BANDWIDTH:       
		therctl_bandwidth(priv, func->val);
		break;
	}

	return ret;
}
#endif /* defined(THERMAL_CONTROL_DAEMON) */
