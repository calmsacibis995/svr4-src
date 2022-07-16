/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/enet.h	1.3.2.1"

/*
 *  Modifications:
 *	I000	8/28/87		DF	Intel
 *		Changed the req_blk structure.
 *	I001	9/22/87		DF	Intel
 *		Added compile switch MB1 for multibus 1 driver.
 *		Changed to run at spl6.
 *	I002	9/22/87		DF	Intel
 *		Add "flags" field to the "endpoint" structure.
 *		The 0th bit of the "flags" is used to indicate whether
 *		the write queue is disabled.
 *	I003	5/11/88		NL	Intel
 *		Added "rb" field to the "req_blk" structure to store
 *		the original address of the Request Block sent to mipsend()
 *		for the PCL2NIA version of the driver. This is done because
 *		the phystokv() routine does not return the orignal value passed
 *		to kvtophys(). Apparently, a lot of the driver depends on the
 *		original address of the Request Block.
 *	I004	6/29/88		RJS/SLH		Intel
 *		DATAGRAM_SIZE now defined in space.c.  "dr_count" redefined as
 *		char array due to allignment problems.
 *	I005	08/28/88	RJS		Intel
 *		Made "rb" field referred to in I003 above generic for all
 *		versions of the driver.
 *	I006	07/06/89	rjf		Intel
 *		Added EDL support.
 *	I007	10/25/89	rjf		Intel
 *		Changed to run at splstr.
 */

#ifndef ENETH
#define ENETH

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/sysmacros.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strstat.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h> 
#include <sys/cmn_err.h> 
#include <sys/systm.h> 
#include <sys/cred.h> 
#include <sys/ddi.h> 
#include "enetuser.h"

#define SPL splstr		/* I001 */	/* I007 */
#define memcpy(d, s, c) bcopy(s, d, c)

#define DEB_NONE 0
#define DEB_ERROR 1
#define DEB_CALL 2
#define DEB_FULL 3
#define DEB_STOP 4

#define enet_NUMBER 5
#define enet_NAME "enet"
#define enet_STRMIN 0
#define enet_STRMAX 4096
#define enet_HIWAT 256
#define enet_LOWAT 0

#define ST_ALFA 0
#define ST_RMSG 1
#define ST_REXP 2
#define ST_RUNI 3
#define ST_SMSG 4
#define ST_SPCK 5
#define ST_SEXP 6
#define ST_EPCK 7
#define ST_SUNI 8
#define ST_UPCK 9
#define ST_BRCV 10
#define ST_ERCV 11
#define ST_URCV 12
#define ST_BSNT 13
#define ST_ESNT 14
#define ST_USNT 15
#define ST_CURO 16
#define ST_TOTO 17
#define ST_CURC 18
#define ST_TOTC 19
#define ST_NORES 20
#define ST_NOERES 21
#define ST_UNKCLO 22
#define ST_RBFA 23
#define ST_PDFA 24
#define enet_SCNT 25

/*
 * enetmod specific statistics counts are as follows:
 *
 *	enet_stat[ST_ALFA]	Count of STREAMS buffer allocation failures
 *	enet_stat[ST_RMSG]	Count of received messages
 *	enet_stat[ST_SMSG]	Count of sent messages
 *	enet_stat[ST_SPCK]	Count of packets sent
 *	enet_stat[ST_BRCV]	Count of data bytes passed to users
 *	enet_stat[ST_BSNT]	Count of data bytes written for users
 *	enet_stat[ST_CURO]	Number of currently open endpoints
 *	enet_stat[ST_TOTO]	Total number of opens done
 *	enet_stat[ST_CURC]	Number of currently connected endpoints
 *	enet_stat[ST_TOTC]	Total number of connections made
 *	enet_stat[ST_NORES]	Count of send failures due to full iNA961 queue
 *	enet_stat[ST_UNKCLO]	Count of bad close operations due to bad CDB
 *	enet_stat[ST_RBFA]	Number of failures due to lack of req_blks
 *	enet_stat[ST_PDFA]	Number of failures due to lack of pend_lists
 */

#define C_IDLE 1
#define C_OPEN 2
#define C_DISCON 4
#define C_ERROR 16

#define PRESENT 1
#define BOOTED 2
#define RESET 4 
#define UNKFWV 8
#define LOADING 16

#define TRUE 1
#define FALSE 0

#define MIPERR 20	/* One greater than the last TLI error code */
#define HIGH_PRI 0x100	/* Signifies that the error_ack should be high pri. */

#define TS_INVALID 127	/* = nr in timod.c */

#define FILENAME961 "/etc/ina961.13"

#define RSN_NORMAL 0

/* definition of N_ENET and N_ENDPOINTS moved from here to io/enet/space.c */
/* where it can be overridden by config.h */
#define CLOSE_BUF_LEN 32
#define CONN_BUF_LEN 64
#define ETSDU_SIZE 16
#define TSDU_SIZE 0xffff
#define TIDU_SIZE 0xffff
#define RQD_Q_SIZE 16
#define LRQD_Q_SIZE	4		/* Log2(RQD_Q_SIZE) */

#define EADDR_SIZE 6
#define IDELAY 300
#define I_DEV_COUNT		1	/* # of devices			*/
#define I_IDS_COUNT		1	/* # of Inter Device Segs	*/
#define L_DEV_ID		0x1	/* Local Device ID		*/
#define I_IDS_LENGTH		(64/4)	/* IDS == 1st 64K		*/
#define PORT_960	  (char)0x10	/* 960 command			*/
#define PORT_RETURN_RB	  (char)0x11	/* Return user RB		*/
#define I_STATUS		0xff
#define I_IDS_ID		0x00
#define I_TIMEOUT		0xFF	/* No Timeout */
#define MULTIBUS_INTERRUPT	0x03	/* Edge-level */

#define M_Q_FULL_NO_LONGER	0x80
#define M_Q_EMPTY_NO_LONGER	0x01
#define M_NO_CHANGE		0x00
#define M_GIVE_HALT		0x40
#define M_TAKE_HALT		0x40
#define M_GIVE_FACTOR		0x80
#define M_TAKE_FACTOR		0x80
#define PTR_MASK		0x7f

#define M_SEND	0x70
#define M_ACK_NO_COPY	0x80
#define M_ACK_COPY	0x82

#define LOWWORD(x) ((ushort)(x&0x00ff))
#define HIGHWORD(x) ((ushort)((x>>16)&0x0ff))
#define USHORT(x)	(*((ushort*)(x)))
#define OFFSETOF(p)	USHORT(p)
#define SELECTOROF(p)	USHORT(p+2)
#define UNSIGNED(c)	((unsigned)((c)&0xff))

#define USER_OPT 0x8000

#define OPTMGMT_NEXT_STATE(ep) (ep->tli_state==TS_WRES_CIND?TS_WRES_CIND:ti_statetbl[TE_OPTMGMT_ACK][ep->tli_state])


/*
 * Wake-up port codes
 */
#define I_RESET		0x01		/* Reset the board */
#define I_START		0x02		/* Signal to start a command */
#define I_CLEAR		0x04		/* Clear a limited level interrupt */

/*
 * Response codes
 */
#define OK_RESP			0x01
#define OK_EOM_RESP		0x03
#define OK_DECIDE_REQ_RESP	0x05
#define OK_CLOSED_RESP		0x07
#define OK_WITHDRAW_RESP	0x09	/* This is a guess, it's undocumented */
#define OK_REJECT_CONN_RESP	0x0b

#define INVALID_REQ		0x02
#define NO_RESOURCES		0x04
#define UNKNOWN_REFERENCE	0x06
#define BUFFER_TOO_SHORT	0x08
#define BUFFER_TOO_LONG		0x0a
#define ILLEGAL_REQ		0x0c
#define REM_ABORT		0x0e
#define LOC_TIMEOUT		0x10
#define UNKNOWN_CONN_CLASS	0x12
#define DUP_REQ			0x14
#define CONN_REJECT		0x16
#define NEGOT_FAILED		0x18
#define ILLEGAL_ADDRESSS	0x1a
#define NETWORK_ERROR		0x1c
#define PROTOCOL_ERR		0x1e
#define ILLEGAL_RB_LENGTH	0x20

/*
 * Transport Control opcodes
 */
#define OP_OPEN		(char)0x00
#define OP_SCR		(char)0x01
#define OP_ACRT		(char)0x02 
#define OP_ACRU		(char)0x03
#define OP_ACR		(char)0x04
#define OP_SD		(char)0x05 
#define OP_EOM_SD	(char)0x06 
#define OP_RD		(char)0x07
#define OP_WRB		(char)0x08
#define OP_EX_SD	(char)0x09 
#define OP_EX_RD	(char)0x0a
#define OP_WEB		(char)0x0b
#define OP_CLOSE	(char)0x0c
#define OP_ACLOSE	(char)0x0d
#define OP_STAT		(char)0x0e
#define OP_DEFSTAT	(char)0x0f
#define OP_SDGM		(char)0x11 
#define OP_RDGM		(char)0x12 
#define OP_WDGM		(char)0x13

/*
 * Network Management Facility opcodes
 */
#define OP_NMF_RO	(char)0x00
#define OP_NMF_RACO	(char)0x01
#define OP_NMF_SO	(char)0x02
#define OP_NMF_RMEM	(char)0x03
#define OP_NMF_SMEM	(char)0x04
#define OP_NMF_DUMP	(char)0x05
#define OP_NMF_ECHO	(char)0x06
#define OP_NMF_FLOAD	(char)0x07
#define OP_NMF_SUPPLYB	(char)0x08
#define OP_NMF_TAKEB	(char)0x09

#ifdef EDL
/*
 * EDL Facility opcodes
 */
#define OP_EDL_RAWTRAN	0x7E
#define OP_EDL_RAWRECV	0x7F
#define OP_EDL_CONN	0x82
#define OP_EDL_DISC	0x83
#define OP_EDL_TRAN	0x84
#define OP_EDL_POST	0x85
#define OP_EDL_MCADD	0x87
#define OP_EDL_CONF	0x88
#define OP_EDL_IASET	0x89
#define OP_EDL_MCREM	0x8A
#define OP_EDL_ENRING	0x8B
#define OP_EDL_EXRING	0x8C
#endif

#ifdef NS
/*
 * NS opcodes
 */
#define OP_NS_ADD_NAME	0x00
#define OP_NS_DEL_NAME	0x01
#define OP_NS_GET_VAL	0x02
#define OP_NS_CHNG_VAL	0x03
#define OP_NS_DEL_PROP	0x04
#define OP_NS_GET_NAME	0x05
#define OP_NS_GET_SPOK	0x06
#define OP_NS_LIST_TAB	0x07
#endif

/*
 *  The various subsystems
 */
#ifdef EDL
#define SUB_EDL		0x20		/* iSBC XXX */
#endif
#define SUB_VC		0x40
#define SUB_DG		0x41
#ifdef NS
#define SUB_NS		0x50
#endif
#define SUB_NMF		0x80

typedef char bool;

struct _pend_list {
	struct _pend_list *next;
	ushort	reference;
	int	seqno;
	struct orb *o_rb;
};
typedef struct _pend_list pend_list;

struct _endpoint {
	queue_t	*rd_q;
	pend_list *pend_connects;
	int	tli_state;
	int	str_state;
	int	nbr_pend;
	opts	options;
	int	max_pend;
	int	bnum;
	ushort	l_reference;	/* The listening CDB reference (if one) */
	ushort	c_reference;	/* The connected CDB reference (if one) */
	ushort	tsap;
	int	req_seqno;
	int	complete_seqno;
	int	ex_req_seqno;
	int	ex_complete_seqno;
	struct _endpoint *listen_ep;
	void	(*open_cont)();	/* Called when interrupted after open request */
	mblk_t	*flow;
	mblk_t	*ex_flow;
	int	nbr_datarq;	/* number of outstanding data sends */
};
typedef struct _endpoint endpoint;


struct enetinf {			/* I000 */
	ulong	base;
	ulong	port;
	ushort  slot;
	char	board_type[11];
	unsigned char	fwv;
	unsigned char	inav;		/* ina961 release version */
	unsigned char	numvc;		/* number of vc's configured */
	unsigned char	eaddr[6];
};


/* Read queue structure */

struct	vcp {
	short		type;
	short		post;
	struct	vcrb	*vp;
};

#if defined(MB1) || defined(AT386)
/*
struct  mip_rqe 
{
  char		r_request;
  char		r_sreq_id;		/* source request ID	
  char		r_ddev_id;		/* destination dev ID	
  char		r_dport_id;		/* destination port ID	
  char		r_sdev_id;		/* source device ID	
  char		*r_dataptr;		/* Ptr to ref'd thing	
  ushort	r_datalen;		/* length(data)		
  char		r_ids_id;		/* IDS ID		
  char		r_odev_id;		/* owner device ID	
  char		r_reserved[3];
};
*/
typedef char mip_rqe[16];
#define RQE_REQUEST(x) (x[0])
#define RQE_SREQ_ID(x) (x[1])
#define RQE_DDEV_ID(x) (x[2])
#define RQE_DPORT_ID(x) (x[3])
#define RQE_SDEV_ID(x) (x[4])
#define RQE_DATAPTR(x) (*(char **)&(x)[5])
#define RQE_DATALEN(x) (*(ushort *)&(x)[9])
#define RQE_IDS_ID(x) (x[11])
#define RQE_ODEV_ID(x) (x[12])

struct  mip_rqd {
  char			d_empty;
  char			d_full;
  char			d_rq_size;
  char			d_rqe_length;
  char			d_give_index;
  char			d_give_state;
  char			d_take_index;
  char			d_take_state;
  mip_rqe		d_rqe[RQD_Q_SIZE];	/* Actual queue entries */
};

struct enetboard {
	char	state;
	ulong	base;
	ulong	port;
	ushort	sel;
	unsigned char	fwv;
	unsigned char	eaddr[6];
	unsigned char	tstr;
/*	struct mip_rqd	qxmt;
	struct mip_rqd	qrcv;*/
	struct mip_rqd	*qxmt;
	struct mip_rqd	*qrcv;
};

struct enetcfg {
	ulong	port;
	ulong	cmdaddr;
	ulong	fwv[10];
};


/*  
 * iNA961 Presence Command
 */

struct iNA961pres {
	unchar	p_command;
	unchar	p_response;
	unchar	p_test_result;
	unchar	p_version;
	unchar	p_eaddr[EADDR_SIZE];
};
#define PRESENCE_CMD 0x01

/*  
 * iNA961 Start (and Load) Command
 */
struct iNA961start {
/*	unchar	s_command;	/* Left out due to alignment problems */
/*	unchar	s_response;	/* Left out due to alignment problems */
	ulong	s_host_load_area;
	ulong	s_comm_load_area;
	ushort 	s_length;
	ushort	s_start_offset;
	unchar	s_dev_count;
	unchar	s_ids_count;
	unchar	s_dest_dev_id;
	unchar	s_filler;
	unchar	s_ids_base;
	unchar	s_ids_length;
	unchar	s_src_dev_id;
	unchar	s_status;
	ulong	s_trans_rqd;
	ulong	s_recv_rqd;
	unchar	s_int_type;
	unchar	s_timeout;
	ushort	s_int_addr;
};
#define START_CMD 0x02

/*
 * iNA961 Load Command
 */
struct iNA961load {
/*	unchar	l_command;	/* Left out due to alignment problems */
/*	unchar	l_response;	/* Left out due to alignment problems */
	ulong	l_host_load_area;
	ulong	l_comm_load_area;
	ushort	l_length;
};
#define LOAD_CMD 0x05
#define DUMP_CMD 0x06
#else
struct enetboard {
	char	state;
	ulong	base;
	ulong	port;
	ushort  slot_id;
	ushort	sel;
	unsigned char	fwv;
	unsigned char	eaddr[6];
	unsigned char	tstr;
};

struct enetcfg {
	ushort	slot_id;
	ulong	lcis_rb_port;
	ulong	lcis_xmit_port;
	ulong	lcis_rcv_port;
};

#endif

/* Internal address structure used to talk to the board */
/*
struct tabuf {
	unsigned char local_net_addr_len;	- always 0
	unsigned char local_tsap_len;		- always 2
	unsigned short local_tsap;
	unsigned char rem_net_addr_len;
	union {
		struct {
			unsigned char length;
			unsigned char afi;		- always 0x49
			unsigned short subnet_no;
			unsigned char subnet_addr[7];	- host addr, 0
			unsigned char rem_nsap_id;	- optional
		} subnet_addr;
		struct {
			unsigned char length;
			unsigned char afi;		- always 0x49
			unsigned char areaid[5];
			unsigned char subnet_no;
			unsigned char subnet_addr[7];	- host addr, 0
			unsigned char rem_nsap_id;	- optional
		} area_addr;
		struct {
			unsigned char length;		- always 12
			unsigned long subnet_no;	- always 1
			unsigned char subnet_addr[6];	- host addr
			unsigned short rem_nsap_id;	- always 1
		} compat_addr;
};
*/
/***typedef unsigned char tabuf[24];***/
/***#define MAX_NAME_LEN 24***/
/*** rjf - increased for larger tsaps */
typedef unsigned char tabuf[38];
#define MAX_NAME_LEN 38

#define ADDR_LENGTH(tab) (tab[0]+1 + tab[tab[0]+1]+1)
#define REM_ADDR(tab) (&(tab)[4])

#define T_LNA_LEN(tab) ((tab)[0])
#define T_LTSAP_LEN(tab) ((tab)[1])
#define T_LTSAP(tab) (*(unsigned short *)&(tab)[2])
#define T_RNA_LEN(tab) ((tab)[4])

#define TS_RNA_LENGTH(tab) ((tab)[5])
#define TS_RNA_AFI(tab) ((tab)[6])
#define TS_RNA_SUBNETNO(tab) (*(unsigned short *)&(tab)[7])
#define TS_RNA_SUBNETADDR(tab) ((unsigned char *)&(tab)[9])
#define TS_RNA_NSAP_ID(tab) ((tab)[16])

#define TA_RNA_LENGTH(tab) ((tab)[5])
#define TA_RNA_AFI(tab) ((tab)[6])
#define TA_RNA_AREAID(tab) ((unsigned char *)&(tab)[7])
#define TA_RNA_SUBNETNO(tab) ((tab)[12])
#define TA_RNA_SUBNETADDR(tab) ((unsigned char *)&(tab)[13])
#define TA_RNA_NSAP_ID(tab) ((tab)[20])

#define TC_RNA_LENGTH(tab) ((tab)[5])
#define TC_RNA_SUBNETNO(tab) (*(unsigned long *)&(tab)[6])
#define TC_RNA_SUBNETADDR(tab) ((unsigned char *)&(tab)[10])
#define TC_RNA_NSAP(tab) (*(unsigned short *)&(tab)[16])

#define T_RTSAP_LEN(tab) ((tab)[(tab)[4]+5])
#define T_RTSAP(tab) (*(unsigned short *)&(tab)[(tab)[4]+6])

struct _crbh
{
  ushort	c_resvd1[2];
  unchar	c_len;
  unchar	c_user[2];	/* always 0 */
  unchar	c_resp_port;	/* always 0xff */
  unchar	c_port_id;
  unchar	c_dev_id;
  unchar	c_resvd2[2];
  unchar	c_subsys;
  unchar	c_opcode;
  ushort	c_resp;
};
typedef struct _crbh crbh;

/* Open Request Block */
struct orb {
	crbh		or_crbh;
	ushort		or_reference;
	/* Internal info (ignored by INA) */
	mblk_t		*or_message;
	endpoint	*or_ep;
};
#define ORB_SIZE (sizeof(struct orb)-sizeof(mblk_t *)-sizeof(endpoint *))

/* Connection Request Request Block */
struct crrb {
	crbh	cr_crbh;
	unchar	cr_iso_rc;
	unchar	cr_resvd[4];
	unchar	cr_ack_d_est[2];	/* Really a ushort sans alignment */
	unchar	cr_tabufp[4];		/* Really a tabuf pointer */
	unchar	cr_pc[2];		/* Really a ushort */
	unchar	cr_ato[2];		/* Really a ushort */
	unchar	cr_reference[2];	/* Really a ushort */
	unchar	cr_qos;			/* iNA960 PRM calls this gos (?) */
	unchar	cr_neg_ops[2];		/* Really a ushort */
	unchar	cr_u_buf[4];		/* Really a char pointer */
	unchar	cr_u_len;
	/* Internal info (ignored by INA) */
	endpoint *cr_ep;
	mblk_t	*cr_message;
	mblk_t	*cr_databuf;
	tabuf	cr_tabuf;
	pend_list *cr_pending;
};
#define CRRB_SIZE (sizeof(struct crrb)-sizeof(endpoint *)-(2*sizeof(mblk_t *))-sizeof(tabuf)-sizeof(pend_list *))

/* Virtual Circuit Request Block */
struct vcrb {
	crbh	vc_crbh;
	unchar	vc_iso_rc;
	unchar	vc_resvd[15];
	ushort	vc_reference;
	unchar	vc_qos;
	unchar	vc_count[2];
	unchar	vc_nblks;
	unchar	vc_bufp[4];	/* Really "unchar *vc_bufp" sans alignment */
	ushort	vc_buflen; 
	/* Internal info (ignored by INA) */
	endpoint	*vc_ep;
	mblk_t	*vc_databuf;
	mblk_t	*vc_message;
	int	seqno;
};
#define VCRB_SIZE (sizeof(struct vcrb)-sizeof(endpoint *)-(2*sizeof(mblk_t *))-sizeof(int))

/* Datagram Request Block */
struct drb {
	crbh	dr_crbh;
	unchar	dr_resvd[4];
	unchar	dr_tabufp[4];	/* Really a tabuf pointer */
	unchar	dr_qos;
	unchar	dr_count[2];	/* I004 */
	unchar	dr_nblks;
	unchar	dr_bufp[4];	/* Really "unchar *dr_bufp" sans alignment */
	ushort	dr_buflen;
	/* Internal info (ignored by INA) */
	endpoint	*dr_ep;
	mblk_t	*dr_databuf;
	mblk_t	*dr_message;
	tabuf	dr_tabuf;
};
#define DRB_SIZE (sizeof(struct drb)-sizeof(endpoint *)-(2*sizeof(mblk_t *))-sizeof(tabuf))

#ifdef EDL
/* EDL Connection Request Request Block */
struct connrb {
	crbh	cr_rbh;			/* 16 bytes */
	unchar	cr_lsap;
	unchar	cr_reserved;
	unchar	cr_port;
	/* Internal use */
	queue_t	*cr_q;
	endpoint	*cr_ep;
};
#define CONNRB_SIZE (sizeof(struct connrb)-sizeof(queue_t *)-sizeof(endpoint *))

/* EDL Disconnect Request Request Block */
struct discrb {
	crbh	dr_rbh;			/* 16 bytes */
	unchar	dr_lsap;
	unchar	dr_reserved;
	/* Internal use */
	queue_t	*dr_q;
	endpoint	*dr_ep;
};
#define DISCRB_SIZE (sizeof(struct discrb)-sizeof(queue_t *)-sizeof(endpoint *))

/* EDL RAW transmit data block */
struct tranrb {
	crbh	tr_rbh;			/* 16 bytes */
	ushort	tr_reserved;
	ushort	tr_ether_type;
	unchar	tr_src_addr[6];		/* optional */
	ushort	tr_byte_cnt;
	unchar	tr_buffer_ptr[4];	/* really a DWORD */
	unchar	tr_dst_addr_ptr[4];	/* really a DWORD */
	/* Internal use */
	queue_t	*tr_q;
	mblk_t	*tr_mp;
	tabuf	tr_tabuf;
};
#define ETRANSIZE	(sizeof (struct tranrb) - (8 + sizeof (tabuf)))
#define TRANRB_SIZE (sizeof(struct tranrb)-sizeof(queue_t *)-sizeof(mblk_t *)-sizeof(tabuf))

/* EDL RAW receive post command request block format */
struct postrb {
	struct _crbh	rbh;
	unsigned char	lsap;
	unsigned char	reserved;
	unsigned short	num_blks;
	unsigned short	filled_length;
	unsigned short	buffer_length;
	unsigned short	max_copy_len;
	unsigned short	max_frames;
	unsigned short	actual_frames;
	char		buffer_ptr[4];		/* really a DWORD */
	/* Internal use */
	endpoint	*ep;
	mblk_t		*mp;
};
#define EPOSTSIZE	(sizeof (struct postrb) - 8)
#define POSTRB_SIZE (sizeof(struct postrb)-sizeof(endpoint *)-sizeof(mblk_t *))

#define ERAWLSAP	99

struct postframeh {
	ushort	record_length;
	unchar	time_stamp[4];	/* really a DWORD */
	ushort	lost_count;
	unchar	dest_addr[6];
	unchar	src_addr[6];
	ushort	len_or_type;
};
#define POSTFRAMEH_SIZE	(sizeof(struct postframeh))
#endif

#ifdef NS
struct nsrb {
	crbh	ns_crbh;
	unchar	ns_resvd[6];
	unchar	ns_nabufp[4];
	unchar	ns_unflag;
	unchar	ns_ptype[2];
	unchar	ns_pvaltype;
	unchar	ns_vabufp[4];
	unchar	ns_exbufp[4];
	ushort	ns_exbuflen;
	/* Internal use */
	endpoint *ns_ep;
	mblk_t	 *ns_mp;
};
#define NSRB_SIZE (sizeof(struct nsrb)-sizeof(endpoint *)-sizeof(mblk_t *))
#endif

struct nmfrb {
	crbh	nmf_crbh;
	ushort	nmf_reference;
	ushort	nmf_filledlen;
	unchar	nmf_respbufp[4];
	ushort	nmf_respbuflen;
	unchar	nmf_cmdbufp[4];
	ushort	nmf_cmdbuflen;
	/* Internal use */
	endpoint *nmf_ep;
	mblk_t	 *nmf_mp;
};
#define NMFRB_SIZE (sizeof(struct nmfrb)-sizeof(endpoint *)-sizeof(mblk_t *))

struct req_blk {
	union {
		struct orb  o_rb;
		struct crrb cr_rb;
		struct vcrb vc_rb;
		struct drb  d_rb;
#ifdef EDL
		struct tranrb t_rb;
		struct postrb p_rb;
#endif
#ifdef NS
		struct nsrb ns_rb;
#endif
		struct nmfrb nmf_rb;
	} u;
	int	in_use;		/* I000 */
	mblk_t	*databuf;	/* I000 */
	caddr_t	rb;		/* I003, I005 */
};

extern void enetreset();
extern opts enet_check_opts();
extern void enet_set_opts();
extern int enet_bind_req();
extern int enet_unbind_req();
extern int enet_conn_req();
extern int enet_conn_res();
extern int enet_discon_req();
extern int enet_data_req();
extern void enet_pferr();
extern mblk_t *enet_make_addr();
extern void enet_pterr();
extern void enet_send_flush();
extern void enet_ok_ack();
extern void enet_bind_ack();
extern void enet_optmgmt_ack();
extern void enet_conn_con_ack();
extern void enet_abort();
extern void enet_discon_ind();
extern void enet_conn_con();
extern void enet_conn_ind();
extern void enet_data_ind();
extern void enet_data_wait();
extern void enet_data_wait_post();
extern void enet_buf_post();
extern void enet_open_complete();
extern void enet_accept_conn_ack();
extern void iNA961_reset();
extern int iNA961_presence();
extern int iNA961_codeblock();
extern int iNA961_open();
extern int iNA961_await_conn_req();
extern int iNA961_send_connect_req();
extern int iNA961_accept_conn_req();
extern int iNA961_send_data();
extern int iNA961_receive_data();
extern void iNA961_close();
extern pend_list *getpend();
extern void relpend();
extern struct req_blk *getrb();
extern void relrb();
extern char *miprcv();
extern mblk_t *getmptr();
extern mblk_t *growmptr();
extern char *debugptr;
extern char debugtxt[];
#ifdef DEBUG
#define DEBUGC(c) if(debugptr<=debugtxt+10000)*debugptr++ = c;else *debugptr='O'
#define DEBUGP(level,args) if(enet_debug >= level) cmn_err args;
#else
#define DEBUGC(c)
#define DEBUGP(level,args)
#endif /* DEBUG */
#endif /* ENETH */
