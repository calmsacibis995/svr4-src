%{
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
%}
%{
/*	UNIX System Laboratories, Inc.                     	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%%
[_A-Za-z][_A-Za-z0-9]*	do_id(yytext);
%%
struct idmap {
	char *old;
	char *new;
	int flags;
};
#undef INTERACTIVE
struct idmap mb2map[] = {
	{ "AMPcancel",				"mps_AMPcancel", 0 },
	{ "AMPreceive",				"mps_AMPreceive", 0 },
	{ "AMPreceive_frag",		"mps_AMPreceive_frag", 0 },
	{ "AMPsend",				"mps_AMPsend", 0 },
	{ "AMPsend_data",			"mps_AMPsend_data", 0 },
	{ "AMPsend_reply",			"mps_AMPsend_reply", 0 },
	{ "AMPsend_rsvp",			"mps_AMPsend_rsvp", 0 },
	{ "BLKPRIO",				"MPS_BLKPRIO", 0 },
	{ "BOARD_SPECIFIC_REC",			"ICS_BOARD_SPECIFIC_REC", 0 },
	{ "CLKPRIO",				"MPS_CLKPRIO", 0 },
	{ "EOT_TYPE",					"ICS_EOT_TYPE", 0 },
	{ "FP_PORT",					"MPS_FP_PORT", 0 },
	{ "HI",						"ICS_HI", 0 },
	{ "HIB",					"MPS_HIB", 0 },
	{ "HOST_ID_TYPE",			"ICS_HOST_ID_TYPE", 0 },
	{ "ICBistDataIn",				"ICS_BistDataIn", 0 },
	{ "ICBistDataOut",				"ICS_BistDataOut", 0 },
	{ "ICBistMasterStatus",			"ICS_BistMasterStatus", 0 },
	{ "ICBistSlaveStatus",			"ICS_BistSlaveStatus", 0 },
	{ "ICBistSupportLevel",			"ICS_BistSupportLevel", 0 },
	{ "ICBistTestID",				"ICS_BistTestID", 0 },
	{ "ICClassID",					"ICS_ClassID", 0 },
	{ "ICGeneralControl",			"ICS_GeneralControl", 0 },
	{ "ICGeneralStatus",			"ICS_GeneralStatus", 0 },
	{ "ICHardwareTestRev",			"ICS_HardwareTestRev", 0 },
	{ "ICNMIEnable",				"ICS_NMIEnable", 0 },
	{ "ICProductCode",				"ICS_ProductCode", 0 },
	{ "ICProgramTableIndex",		"ICS_ProgramTableIndex", 0 },
	{ "ICVendorIDH",				"ICS_VendorIDH", 0 },
	{ "ICVendorIDL",				"ICS_VendorIDL", 0 },
	{ "IC_DEBUG_MASK",				"ICS_DEBUG_MASK", 0 },
	{ "IC_DEBUG_NMI",				"ICS_DEBUG_NMI", 0 },
	{ "IC_ENABLE_NMI",				"ICS_ENABLE_NMI", 0 },
	{ "IC_PARITY_MASK",				"ICS_PARITY_MASK", 0 },
	{ "IC_PARITY_OFFSET",			"ICS_PARITY_OFFSET", 0 },
	{ "IC_SWNMI_MASK",				"ICS_SWNMI_MASK", 0 },
	{ "IC_SW_NMI",					"ICS_SW_NMI", 0 },
	{ "LO",						"ICS_LO", 0 },
	{ "LOB",					"MPS_LOB", 0 },
	{ "MAXMSGSZ",				"MPS_MAXMSGSZ", 0 },
	{ "MAX_REG",					"ICS_MAX_REG", 0 },
	{ "MAX_SLOT",				"ICS_MAX_SLOT", 0 },
	{ "MB2_TPDT",					"MPS_MB2_TPDT", 0 },
	{ "MG_BGDC",					"MPS_MG_BGDC", 0 },
	{ "MG_BGLI",					"MPS_MG_BGLI", 0 },
	{ "MG_BGLL",					"MPS_MG_BGLL", 0 },
	{ "MG_BGOVHD",					"MPS_MG_BGOVHD", 0 },
	{ "MG_BGRANT",					"MPS_MG_BGRANT", 0 },
	{ "MG_BJLI",					"MPS_MG_BJLI", 0 },
	{ "MG_BJOVHD",					"MPS_MG_BJOVHD", 0 },
	{ "MG_BRDCST",					"MPS_MG_BRDCST", 0 },
	{ "MG_BRDP",					"MPS_MG_BRDP", 0 },
	{ "MG_BREJ",					"MPS_MG_BREJ", 0 },
	{ "MG_BREQ",				"MPS_MG_BREQ", 0 },
	{ "MG_BRLI",					"MPS_MG_BRLI", 0 },
	{ "MG_BRML",					"MPS_MG_BRML", 0 },
	{ "MG_BROVHD",					"MPS_MG_BROVHD", 0 },
	{ "MG_BRPI",					"MPS_MG_BRPI", 0 },
	{ "MG_BRSP",					"MPS_MG_BRSP", 0 },
	{ "MG_BRTC",					"MPS_MG_BRTC", 0 },
	{ "MG_BRTI",					"MPS_MG_BRTI", 0 },
	{ "MG_BRUD",					"MPS_MG_BRUD", 0 },
	{ "MG_DA",						"MPS_MG_DA", 0 },
	{ "MG_DFBIND",				"MPS_MG_DFBIND", 0 },
	{ "MG_DONE",				"MPS_MG_DONE", 0 },
	{ "MG_DUTY1C",					"MPS_MG_DUTY1C", 0 },
	{ "MG_DUTY2C",					"MPS_MG_DUTY2C", 0 },
	{ "MG_ESOL",				"MPS_MG_ESOL", 0 },
	{ "MG_MT",						"MPS_MG_MT", 0 },
	{ "MG_NFDP",					"MPS_MG_NFDP", 0 },
	{ "MG_NFFL",					"MPS_MG_NFFL", 0 },
	{ "MG_NFOVHD",					"MPS_MG_NFOVHD", 0 },
	{ "MG_NFPI",					"MPS_MG_NFPI", 0 },
	{ "MG_NFSP",					"MPS_MG_NFSP", 0 },
	{ "MG_NFTC",					"MPS_MG_NFTC", 0 },
	{ "MG_NFTI",					"MPS_MG_NFTI", 0 },
	{ "MG_REQ",						"MPS_MG_REQ", 0 },
	{ "MG_RES",						"MPS_MG_RES", 0 },
	{ "MG_RI",						"MPS_MG_RI", 0 },
	{ "MG_RQMF",					"MPS_MG_RQMF", 0 },
	{ "MG_RQNF",					"MPS_MG_RQNF", 0 },
	{ "MG_RQSF",					"MPS_MG_RQSF", 0 },
	{ "MG_RQXF",					"MPS_MG_RQXF", 0 },
	{ "MG_RRMSK",					"MPS_MG_RRMSK", 0 },
	{ "MG_RRTMSK",					"MPS_MG_RRTMSK", 0 },
	{ "MG_RSCN",					"MPS_MG_RSCN", 0 },
	{ "MG_RSET",					"MPS_MG_RSET", 0 },
	{ "MG_RSNE",					"MPS_MG_RSNE", 0 },
	{ "MG_SA",						"MPS_MG_SA", 0 },
	{ "MG_TERR",				"MPS_MG_TERR", 0 },
	{ "MG_UMDP",					"MPS_MG_UMDP", 0 },
	{ "MG_UMOVHD",					"MPS_MG_UMOVHD", 0 },
	{ "MG_UMPI",					"MPS_MG_UMPI", 0 },
	{ "MG_UMSP",					"MPS_MG_UMSP", 0 },
	{ "MG_UMTC",					"MPS_MG_UMTC", 0 },
	{ "MG_UMTI",					"MPS_MG_UMTI", 0 },
	{ "MG_UMUD",					"MPS_MG_UMUD", 0 },
	{ "MG_UNSOL",				"MPS_MG_UNSOL", 0 },
	{ "MY_SLOT_ID",				"ICS_MY_SLOT_ID", 0 },
	{ "NRMPRIO",				"MPS_NRMPRIO", 0 },
	{ "PARITY_CONTROL_TYPE",		"ICS_PARITY_CONTROL_TYPE", 0 },
	{ "PC16_CONFIG_OFFSET",			"ICS_PC16_CONFIG_OFFSET", 0 },
	{ "PC16_STATUS_OFFSET",			"ICS_PC16_STATUS_OFFSET", 0 },
	{ "PSB_CONTROL_TYPE",			"ICS_PSB_CONTROL_TYPE", 0 },
	{ "PSB_SLOT_ID_REG",			"ICS_PSB_SLOT_ID_REG", 0 },
	{ "READ_ICS",				"ICS_READ_ICS", 0 },
	{ "SLOT_ID_OFFSET",				"ICS_SLOT_ID_OFFSET", 0 },
	{ "SRLPRIO",				"MPS_SRLPRIO", 0 },
	{ "WRITE_ICS",				"ICS_WRITE_ICS", 0 },
	{ "agent_cmp",				"ics_agent_cmp", 0 },
	{ "buf_count",				"mps_buf_count", 0 },
	{ "close_chan",				"mps_close_chan", 0 },
	{ "cpunum",					"ics_cpunum()", 0 },
	{ "datbuf_t",					"mps_datbuf_t", 0 },
	{ "db_join",					"dma_buf_join", 0 },
	{ "dbuf_breakup",				"dma_buf_breakup", 0 },
	{ "fetch_db",				"mps_fetch_db", 0 },
	{ "fetch_mb",				"mps_fetch_mb", 0 },
	{ "find_port",				"mps_find_port", 0 },
	{ "find_transaction",		"mps_find_transaction", 0 },
	{ "free_dbuf",				"mps_free_dbuf", 0 },
	{ "free_mbuf",				"mps_free_mbuf", 0 },
	{ "free_msgbuf",			"mps_free_msgbuf", 0 },
	{ "free_tid",				"mps_free_tid", 0 },
	{ "get_dbuf",				"mps_get_dbuf", 0 },
	{ "get_mbuf",				"mps_get_mbuf", 0 },
	{ "get_msgbuf",				"mps_get_msgbuf", 0 },
	{ "get_soldata",			"mps_get_soldata", 0 },
	{ "get_tid",				"mps_get_tid", 0 },
	{ "get_unsoldata",			"mps_get_unsoldata", 0 },
	{ "handle_err",				"mps_handle_err", 0 },
	{ "init_dmacb",				"impc_init_dmacb", 0 },
	{ "lhid",						"mps_lhid()", 0 },
	{ "mk_bgrant",				"mps_mk_bgrant", 0 },
	{ "mk_brdcst",				"mps_mk_brdcst", 0 },
	{ "mk_breject",				"mps_mk_breject", 0 },
	{ "mk_mb2socid",				"mps_mk_mb2socid", 0 },
	{ "mk_mb2soctohid",				"mps_mk_mb2soctohid", 0 },
	{ "mk_mb2soctopid",				"mps_mk_mb2soctopid", 0 },
	{ "mk_snf",					"mps_mk_snf", 0 },
	{ "mk_sol",					"mps_mk_sol", 0 },
	{ "mk_solrply",				"mps_mk_solrply", 0 },
	{ "mk_unsol",				"mps_mk_unsol", 0 },
	{ "mk_unsolrply",			"mps_mk_unsolrply", 0 },
	{ "mpc_copyright",			"impc_copyright", 0 },
	{ "mpc_send",				"impc_send", 0 },
	{ "mpc_start",				"impc_start", 0 },
	{ "msg_comp",				"mps_msg_comp", 0 },
	{ "msg_dispatch",			"mps_msg_dispatch", 0 },
	{ "msg_getbrlen",				"mps_msg_getbrlen", 0 },
	{ "msg_getdstmid",				"mps_msg_getdstmid", 0 },
	{ "msg_getdstpid",				"mps_msg_getdstpid", 0 },
	{ "msg_getfraglen",				"mps_msg_getfraglen", 0 },
	{ "msg_getlsnid",				"mps_msg_getlsnid", 0 },
	{ "msg_getmsgtyp",				"mps_msg_getmsgtyp", 0 },
	{ "msg_getprotid",				"mps_msg_getprotid", 0 },
	{ "msg_getreqid",				"mps_msg_getreqid", 0 },
	{ "msg_getrid",					"mps_msg_getrid", 0 },
	{ "msg_getsrcmid",				"mps_msg_getsrcmid", 0 },
	{ "msg_getsrcpid",				"mps_msg_getsrcpid", 0 },
	{ "msg_gettransctl",			"mps_msg_gettransctl", 0 },
	{ "msg_gettrnsid",				"mps_msg_gettrnsid", 0 },
	{ "msg_getudp",					"mps_msg_getudp", 0 },
	{ "msg_iscancel",				"mps_msg_iscancel", 0 },
	{ "msg_iscompletion",			"mps_msg_iscompletion", 0 },
	{ "msg_iseot",					"mps_msg_iseot", 0 },
	{ "msg_iserror",				"mps_msg_iserror", 0 },
	{ "msg_isreq",					"mps_msg_isreq", 0 },
	{ "msg_proc",				"mps_msg_proc", 0 },
	{ "msg_que",				"mps_msg_que", 0 },
	{ "msg_setbglen",				"mps_msg_setbglen", 0 },
	{ "msg_setbrlen",				"mps_msg_setbrlen", 0 },
	{ "msg_setcancel",				"mps_msg_setcancel", 0 },
	{ "msg_setduty",				"mps_msg_setduty", 0 },
	{ "msg_setrid",					"mps_msg_setrid", 0 },
	{ "msg_setsrcpid",				"mps_msg_setsrcpid", 0 },
	{ "msg_showmsg",			"mps_msg_showmsg", 0 },
	{ "msgbuf_t",					"mps_msgbuf_t", 0 },
	{ "myslotid",					"ics_myslotid()", 0 },
	{ "open_chan",				"mps_open_chan", 0 },
	{ "requestid",					"mps_requestid()", 0 },
	{ "set_rid",					"mps_set_rid", 0 },
	{ "setup_dmabufs",			"impc_setup_dmabufs", 0 },
	{ "sic_cb",					"impc_sic_cb", 0 },
	{ "slot_t",						"ics_slot_t", 0 },
	{ "soc_cb",					"impc_soc_cb", 0 },
	{ "sol_deque",				"impc_sol_deque", 0 },
	{ "sol_que",				"impc_sol_que", 0 },
	{ "transportcopyright",		"mps_copyright", 0 },
	{ 0,						0 },
};

int mb2cnt = sizeof(mb2map)/sizeof(mb2map[0]);

#ifdef INTERACTIVE
int flags = 0;
#endif /* INTERACTIVE */

do_id(id)
char *id;
{	register struct idmap *p;
	register s;

	for (p = mb2map; p->old; p++) {
		if (id[0] > p->old[0])
			continue;		/* No match yet */
		s = strcmp(id, p->old);
		if (s == 0) {
#ifdef INTERACTIVE
			if (p->flags > flags) {
				fprintf(stderr, "Replace %s with %s? ", p->old, p->new);
				freopen("/dev/tty", "r+", stderr);
				flags = (getc(stderr) == 'y');
				if (!flags)
					break;	/* User says no match */
			}
#endif /* INTERACTIVE */
			printf("%s", p->new);
			return;			/* Found a match */
		}
		if (s < 0)
			break;			/* Too late for a match */
	}
	printf("%s", id);
}
