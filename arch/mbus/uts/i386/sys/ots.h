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

#ident	"@(#)mbus:uts/i386/sys/ots.h	1.3"

/*
** ABSTRACT:	Main include file for System V/386 OTS driver
**
** MODIFICATIONS:
*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/dir.h>
#include <sys/user.h>
#include <sys/sysmacros.h>
#include <sys/cmn_err.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strstat.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h> 
#include <sys/mps.h>
#include <sys/cmn_err.h>
#ifndef V_3
#include <sys/cred.h>
#endif
#include "sys/otsuser.h"

#define TRUE		1
#define FALSE		0

#define SPL splstr
typedef char bool;
typedef unsigned char		uchar;

#define OTS_NUMBER	5
#define OTS_NAME	"ots"
#define OTSDG_NAME	"otsdg"
#define OTS_STRMIN	0
#define OTS_STRMAX	4096
#define OTS_UP_HIWAT	4096
#define OTS_UP_LOWAT	2048
#define OTS_DOWN_HIWAT	256
#define OTS_DOWN_LOWAT	0
#define OTS_QUEUE_SIZE	20

#define TIDU_SIZE		4096	/* Transport Interface Data Unit ??? */

#define AM_MAX_PEND		0xFF	/* maximum values for config parms */
#define AM_QUEUE_LEN		204
#define AM_ADDR_SIZE		4
#define AM_OPTS_SIZE		4
#define AM_TSDU_SIZE		0x1000000
#define AM_ETSDU_SIZE		0x1000
#define AM_CDATA_SIZE		0x200
#define AM_DDATA_SIZE		0x200
#define AM_DATAGRAM_SIZE	0x1000
#define AM_BUFFER_SIZE		0x1000

#define C_IDLE		0x01		/* stream state flags */
#define C_OPEN		0x02
#define C_DRAIN		0x04
#define C_CLOSE		0x08
#define C_ERROR		0x10

#define A_SOL		1		/* types of TKI sends */
#define A_UNSOL		2
#define A_RSVP		3
#define A_REPLY		4

#define HIGH_PRI	0x100		/* error_ack should be high priority */

#define TS_INVALID	(uchar)127	/* = nr in timod.c */

#define MB2ERR		20	/* One greater than the last TLI error code */

#define SBLK4096	0x1000		/* largest STREAMS buffer */

struct otscfg				/* SV-ots configuration info */
{
	opts vc_defaults;
	opts dg_defaults;
	ulong tsdu_size;
	ulong max_fragment;
	ushort etsdu_size;
	ushort datagram_size;
	ushort first_port;
	ushort opts_size;
	ushort addr_size;
	ushort cdata_size;
	ushort ddata_size;
	ushort queue_len;
	ushort buffer_size;
	ushort n_connects;
	ushort n_ports;
	ushort n_endpoints;
	ushort n_vcs;
	ushort n_dgs;
	ushort xmt_retries;
	ushort rd_hiwat;
	ushort rd_lowat;
	ushort wr_hiwat;
	ushort wr_lowat;
	ushort sh_hiwat;
	ushort sh_lowat;
	uchar max_pend;
};

typedef struct _endpoint endpoint;
typedef struct _connect connect;
typedef struct _ots_addr ots_addr;

struct _ots_addr
{
	short length;
	char *data;
};

struct _endpoint
{
	queue_t	*rd_q;		/* stream read queue */
	uchar	tli_state;	/* TLI state */
	uchar	str_state;	/* stream state */
	uchar	nbr_datarq;	/* number of outstanding data sends */
	uchar	exclude;	/* endpoint opened exclusively */
	ushort	max_pend;	/* maximum outstanding connect indications */
	ushort	nbr_pend;	/* current outstanding connect indicatinos */
	int	tsdu_sbytes;	/* receive TSDU byte count */
	int	etsdu_sbytes;	/* receive ETSDU byte count */
	int	tsdu_rbytes;	/* receive TSDU byte count */
	int	etsdu_rbytes;	/* receive ETSDU byte count */
	ots_addr addr;		/* bound address */
	opts	options;	/* services supported on this endpoint */
				/* OTS dependent fields below */
	connect	*control;	/* control socket information */
	connect	*data;		/* data socket information */
	endpoint *next;		/* control socket's endpoint list ptr */
	mb2socid_t rsoc;	/* remote control socket */
	mblk_t	*ddata;		/* queued disconnect data buffer */
	endpoint *dest_ep;	/* local target datagram endpoint */
	int	timeout;	/* id returned by timeout() */
	mps_msgbuf_t *breq;		/* queued datagram sol request */
	ushort	dgr_retry;	/* time until next retry at datagram receive */
	uchar	discon_ind;	/* type of disconnect indication queued */
	uchar	reason;		/* reason for disconnect */
	uchar	rctid;		/* remote transaction id */
	uchar	xctid;		/* local transaction id */
	uchar	version;	/* OTS version supported on endpoint */
};

#define M_NO_REASON		0x0	/* ISO Disconnect Reason Codes */
#define M_UNK_ADDR		0x3
#define M_DISCONNECT		0x80
#define M_NO_RESOURCES		0x81
#define M_PROTOCOL_ERROR	0x85
#define M_REFUSED		0x88

#define M_FLOW_CONTROL		0x1F

#define CT_FREE			1		/* connect entry types */
#define CT_CONTROL		2
#define CT_DATA			3
#define CT_LOCAL		4

#define S_PENDING		0x1		/* connect entry state flags */
#define S_ABORT			0x2
#define S_SOLICITED_SEND	0x4
#define S_QUEUE_FULL		0x8
#define S_ALLOCPORT		0x10

#define D_FREE		0x8000

#define M_CT_FIND	(ushort)0	/* M_find_c() flags */
#define M_CT_ALLOC	(ushort)1

struct _connect
{
	short		channel;	/* index returned by mps_open_chan() */
	char		type;		/* connection type (CONTROL or DATA) */
	char		index;		/* entry's connection table index */
	ushort		port;		/* MB2 port id */
	ushort		state;		/* connection state */
	endpoint	*ep;		/* back ptr to associated endpoint */
	connect		*next;		/* links entries by type */
	connect		*pending;	/* pending list */
	union
	{
		struct _control_socket
		{
			ushort	ref_cnt;	/* # referencing endpoints */
		} cntrl;
		struct _data_socket
		{
			mb2socid_t rem_cntrl;	/* remote control socket */
			mb2socid_t rem_data;	/* remote data socket */
			ulong	bufsize;	/* remote buffer size */
			mblk_t	*queue;		/* unsol queue */
			char	*qhead;		/* unsol queue head */
			char	*qtail;		/* unsol queue tail */
			mps_msgbuf_t *breq;		/* queued sol request */
			opts	options;	/* remote options */
			ushort	xcnt;		/* transmit count */ 
			ushort	rcnt;		/* receive count */
			ushort	sbr_retry;	/* time until next breq retry */
			ushort	ubr_retry;	/* time until next unsol retry*/
			uchar	xdtid;		/* data xmt transaction id */
			uchar	rdtid;		/* data rcv transaction id */
			uchar	rctid;		/* O_CONNECT transaction id */
			uchar	version;	/* OTS version supported */
		} data;
		struct _local_socket
		{
			connect	*dest_cntrl;	/* connected control entry */
			connect	*dest_data;	/* connected data entry */
			connect	*cntrl;		/* associated control entry */
			mb2socid_t rem_cntrl;	/* remote control socket */
			mblk_t	*udata;		/* connect user data */
			mblk_t	*odata;		/* connect options */
		} local;
	} skt;
};

union qm			/* queued messages serialized by iTLIrsrv */
{
	struct ID
	{
		long PRIM_type;		/* message type */
		endpoint *ep;		/* endpoint */
		ushort seqno;		/* disconnect VC seqno */
		ushort error;		/* disconect reason */
		ushort more;		/* data indication more flag */
	} id;
	struct T_data_ind tdata;	/* incoming data indication */
	struct T_discon_ind ddata;	/* incoming disconnect indication */
	struct T_ordrel_ind odata;	/* incoming orderly release indication */
};

/*
 * control socket solicited buffer request timeout period computation macros
 */
#define CSBR_FIRST	0x5	/* .01 secs before 1st buffer request retry */
#define CSBR_MAX	0x160	/* maximum .01 secs between retries */

#define CSBR_INIT(x)	((x)->dgr_retry = CSBR_FIRST)
#define CSBR_RETRY(x)	(((x)->dgr_retry < CSBR_MAX) ? \
			 ((x)->dgr_retry *= 2) : CSBR_MAX)
/*
 * data socket solicited buffer request timeout period computation macros
 */
#define DSBR_FIRST	0x5	/* .01 secs before 1st buffer request retry */
#define DSBR_MAX	0x160	/* maximum .01 secs between retries */

#define DSBR_INIT(x)	((x)->skt.data.sbr_retry = DSBR_FIRST)
#define DSBR_RETRY(x)	(((x)->skt.data.sbr_retry < DSBR_MAX) ? \
			 ((x)->skt.data.sbr_retry *= 2) : DSBR_MAX)
/*
 *  data socket unsolicited buffer request timeout period computation macros
 */
#define DUBR_FIRST	0x5	/* .01 secs before 1st buffer request retry */
#define DUBR_MAX	0x160	/* maximum .01 secs between retries */

#define DUBR_INIT(x)	((x)->skt.data.ubr_retry = DUBR_FIRST)
#define DUBR_RETRY(x)	(((x)->skt.data.ubr_retry < DUBR_MAX) ? \
			 ((x)->skt.data.ubr_retry *= 2) : DUBR_MAX)

/*
 * short-circuited data xmt retry timeout period computation macros
 */
#define SC_FIRST	0x5	/* .01 secs before 1st buffer request retry */
#define SC_MAX		0x160	/* maximum .01 secs between retries */

#define SC_INIT(x)	((x)->sc_retry = SC_FIRST)
#define SC_RETRY(x)	(((x)->sc_retry < SC_MAX) ? \
			 ((x)->sc_retry *= 2) : SC_MAX)

#define QHEAD(X)	((X)->skt.data.qhead)
#define QTAIL(X)	((X)->skt.data.qtail)
#define Q_EMPTY(X)	(  (QHEAD(X) == QTAIL(X)) \
			 &&(((X)->state & S_QUEUE_FULL) == FALSE) \
			)

#define Q_FULL(X)	((X)->state & S_QUEUE_FULL)

#define QHEAD_INC(X) \
	QHEAD(X) += OTS_QUEUE_SIZE; \
	if (QHEAD(X) >= (char *) (X)->skt.data.queue->b_wptr) \
		QHEAD(X) = (char *) (X)->skt.data.queue->b_rptr

#define QTAIL_INC(X) \
	QTAIL(X) += OTS_QUEUE_SIZE; \
	if (QTAIL(X) >= (char *) (X)->skt.data.queue->b_wptr) \
		QTAIL(X) = (char *) (X)->skt.data.queue->b_rptr

#define OTS_BUFSIZE(x)	(((x)&0x8000) ? (((x)&0x7FFFF)*0x1000) : (x))
#define min(x,y)	((x > y) ? y : x)

extern void iTLI_bind_ack();
extern void iTLI_bind_req_complete();
extern void iTLI_conn_con();
extern void iTLI_conn_ind();
extern void iTLI_conn_req_complete();
extern void iTLI_conn_res_complete();
extern void iTLI_data_ind();
extern void iTLI_data_ind_complete();
extern void iTLI_data_req_complete();
extern void iTLI_discon_ind();
extern void iTLI_discon_ind_complete();
extern void iTLI_discon_req_complete();
extern void iTLI_exdata_ind();
extern void iTLI_exdata_req_complete();
extern void iTLI_ind_wait();
extern void iTLI_info_ack();
extern void iTLI_ok_ack();
extern void iTLI_optmgmt_ack();
extern void iTLI_ordrel_ind();
extern void iTLI_ordrel_ind_complete();
extern void iTLI_pferr();
extern void iTLI_pterr();
extern void iTLI_send_flush();
extern void iTLI_uderror_ind();
extern void iTLI_unbind_req_complete();
extern void iTLI_unitdata_ind();
extern void iTLI_unitdata_req_complete();

extern void iTLI_check_msg();
extern int iTLIwsrv();
extern int iTLIrsrv();

extern void iTLI_abort();
extern void iTLI_reset();

extern void iMB2_abort();
extern void iMB2_reset();

extern void ots_control_intr();
extern void ots_data_intr();

extern mblk_t *M_getmptr();
extern mblk_t *M_growmptr();
extern void M_add_pending();
extern void M_add_endpoint();
extern connect *M_alloc_connect();
extern ushort M_alloc_port();
extern void M_free_port();
extern void M_free_connect();
extern connect *M_find_d();
extern connect *M_find_c();
extern endpoint *M_find_listener();
extern endpoint *M_find_dgendpoint();
extern endpoint *M_find_xctid_ep();
extern endpoint *M_find_rctid_ep();
extern connect *M_find_pending();
extern connect *M_remove_pending();
extern void M_remove_endpoint();
extern void O_data_snf();
extern void O_put_unsol_queue();
extern void O_take_unsol_queue();

extern char *M_debugptr;
extern char M_debugtxt[];
extern ushort ots_where;
extern ushort sav_prt_where;

#define DEB_NONE	0
#define DEB_ERROR	1
#define DEB_CALL	2
#define DEB_FULL	3
#define DEB_STOP	4

#ifdef DEBUG
#define DEBUGC(c) if(M_debugptr<=M_debugtxt+10000)*M_debugptr++ = c;else *M_debugptr='O'
#define DEBUGP(level,args) if(ots_debug >= level)\
		 {sav_prt_where = prt_where; \
		  prt_where = ots_where ; \
		  cmn_err args; \
		  prt_where = sav_prt_where;}
#else
#define DEBUGC(c)
#define DEBUGP(level,args)
#endif
