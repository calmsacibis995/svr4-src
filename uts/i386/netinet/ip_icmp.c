/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:ip_icmp.c	1.3.1.3"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */


/*
 * System V STREAMS TCP - Release 3.0 
 *
 * Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI) 
 * All Rights Reserved. 
 *
 * The copyright above and this notice must be preserved in all copies of this
 * source code.  The copyright above does not evidence any actual or intended
 * publication of this source code. 
 *
 * This is unpublished proprietary trade secret source code of Lachman
 * Associates.  This source code may not be copied, disclosed, distributed,
 * demonstrated or licensed except as expressly authorized by Lachman
 * Associates. 
 *
 * System V STREAMS TCP was jointly developed by Lachman Associates and
 * Convergent Technologies. 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <net/strioc.h>
#include <sys/socketvar.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#ifdef SYSV
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/cmn_err.h>
#endif SYSV
#include <netinet/nihdr.h>
#ifdef SYSV
#include <sys/tiuser.h>
#include <sys/tihdr.h>
#else
#include <nettli/tiuser.h>
#include <nettli/tihdr.h>
#endif SYSV
#include <netinet/ip_str.h>
#include <netinet/ip_var.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/tcpip.h>
#include <netinet/udp_var.h>

/*extern void     bcopy();*/
extern struct ip_provider *prov_withaddr();

mblk_t         *reallocb();
extern mblk_t  *dupmsg();

#define CHECKSIZE(bp,size) if (((bp) = reallocb((bp), (size),0)) == NULL) {\
			return;\
			}

/*
 * ICMP routines: error generation, receive packet processing, and routines
 * to turnaround packets back to the originator, and host table maintenance
 * routines.
 */


extern struct ip_provider provider[];

int             nodev(), icmpopen(), icmpclose(), icmpuwput();
int             icmplrput(), icmplrsrv();

static struct module_info icmpm_info[MUXDRVR_INFO_SZ] = {
	ICMPM_ID, "icmp", 0, 8192, 8192, 1024,
	ICMPM_ID, "icmp", 0, 8192, 8192, 1024,
	ICMPM_ID, "icmp", 0, 8192, 8192, 1024,
	ICMPM_ID, "icmp", 0, 8192, 8192, 1024,
	ICMPM_ID, "icmp", 0, 8192, 8192, 1024
};

static struct qinit icmpurinit =
{NULL, NULL, icmpopen, icmpclose, NULL, &icmpm_info[IQP_RQ], NULL};

static struct qinit icmpuwinit =
{icmpuwput, NULL, icmpopen, icmpclose, NULL, &icmpm_info[IQP_WQ], NULL};

static struct qinit icmplrinit =
{icmplrput, icmplrsrv, icmpopen, icmpclose, NULL, &icmpm_info[IQP_MUXRQ], NULL};

static struct qinit icmplwinit =
{NULL, NULL, icmpopen, icmpclose, NULL, &icmpm_info[IQP_MUXWQ], NULL};

struct streamtab icmpinfo = {&icmpurinit, &icmpuwinit, &icmplrinit,
	&icmplwinit
};


queue_t        *icmp_qbot;	/* This had better be IP */
int             icmp_index;	/* mux id of lower stream */

/*
 * These are the basic stream module routines for icmp
 */

struct inpcb    icmb;
extern struct inpcb *in_pcballoc();

unsigned char   icmp_dev[64];	/* bit mask of minor devs */

static int      icmpinited;

struct icmpstat icmpstat;

/*
 * The transport level protocols in the Internet implementation are very odd
 * beasts.  In particular, they have no real minor number, just a pointer to
 * the inpcb struct.
 */

/* ARGSUSED */
icmpopen(q, dev, flag, sflag)
	queue_t        *q;
{
	mblk_t         *bp;
	struct stroptions *sop;
	struct inpcb   *inp;
	register short  i, j;
	int             mindev;

	STRLOG(ICMPM_ID, 1, 8, SL_TRACE, "icmpopen: opening dev %x", dev);

	if (!icmpinited)
		icmpinit();
	if (sflag == CLONEOPEN) {
		for (i = 0; i < 64; i++) {
			if (icmp_dev[i] != 0xFF) {
				break;
			}
		}
		if (i == 64) {
			setuerror(ENXIO);
			return (OPENFAIL);
		}
		for (j = 0; j < 8; j++) {
			if ((icmp_dev[i] & (1 << j)) == 0) {
				break;
			}
		}
		mindev = (i * 8) + j;
	} else {
		setuerror(EINVAL);
		return (OPENFAIL);
	}
	if ((q->q_ptr = (char *) in_pcballoc(&icmb)) == NULL) {
		setuerror(ENOMEM);
		return (OPENFAIL);
	}
	/*
	 * Set up the correct stream head flow control parameters
	 */
	while ((bp = allocb(sizeof(struct stroptions), BPRI_HI)) == NULL)
#if (ATT > 30) || (INTEL > 30)
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI, 3)) {
#else
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI)) {
#endif
			STRLOG(ICMPM_ID, 1, 2, SL_TRACE,
			       "icmpopen failed: no memory for stropts");
			in_pcbdetach((struct inpcb *) q->q_ptr);
			return (OPENFAIL);
		}
	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = icmpm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = icmpm_info[IQP_HDRQ].mi_lowat;
	putnext(q, bp);
	inp = (struct inpcb *) q->q_ptr;
	WR(q)->q_ptr = q->q_ptr;
	inp->inp_minor = mindev;
#ifdef SYSV
	if (suser(u.u_cred) != 0) {
#else
	if (suser() != 0) {
#endif SYSV
		inp->inp_state |= SS_PRIV;
	} else {
		setuerror(0);
	}
	inp->inp_q = q;
	inp->inp_tstate = TS_UNBND;
	STRLOG(ICMPM_ID, 1, 5, SL_TRACE, "Icmpopen succeeded wq %x pcb %x",
	       WR(q), inp);
	icmp_dev[i] |= 1 << j;
	return (mindev);
}

icmpclose(q)
	queue_t        *q;
{
	struct inpcb   *inp;
	short           i, j;

	STRLOG(ICMPM_ID, 1, 5, SL_TRACE, "icmpclose: closing pcb @ %x",
	       q->q_ptr);
	inp = (struct inpcb *) q->q_ptr;
	if (inp == NULL) {
		STRLOG(ICMPM_ID, 1, 2, SL_TRACE, "icmpclose: no pcb!\n");
		setuerror(EINVAL);
		return;
	}
	i = inp->inp_minor;
	j = i % 8;
	i = i / 8;
	icmp_dev[i] &= ~(1 << j);
	in_pcbdetach(inp);
	q->q_ptr = NULL;
}


icmpuwput(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	STRLOG(ICMPM_ID, 3, 9, SL_TRACE, "icmpuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		icmpioctl(q, bp);
		break;

	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:

		STRLOG(ICMPM_ID, 3, 9, SL_TRACE, "passing data through icmp");
		icmp_state(q, bp);
		break;

	case M_CTL:
		freemsg(bp);
		break;

	case M_FLUSH:
		if (*bp->b_rptr & FLUSHW) {
			flushq(q, FLUSHALL);
			*bp->b_rptr &= ~FLUSHW;
		}
		if (*bp->b_rptr & FLUSHR)
			qreply(q, bp);
		else
			freemsg(bp);
		break;

	default:
		freemsg(bp);
		break;
	}
}


icmpioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp;

	iocbp = (struct iocblk *) bp->b_rptr;
	iocbp->ioc_error = 0;
	iocbp->ioc_rval = 0;

	switch (iocbp->ioc_cmd) {
	case I_PLINK:
	case I_LINK:
		STRLOG(ICMPM_ID, 0, 9, SL_TRACE,
		       "icmpioctl: linking new provider");
		iocbp->ioc_count = 0;
		if (icmp_qbot) {
			iocbp->ioc_error = ENOSR;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(ICMPM_ID, 0, 3, SL_TRACE,
			       "I_LINK failed: icmp in use");
			qreply(q, bp);
			return;
		} else {
			struct linkblk *lp;
			struct N_bind_req *bindr;
			mblk_t         *nbp;

			lp = (struct linkblk *) bp->b_cont->b_rptr;
			icmp_qbot = lp->l_qbot;
			icmp_index = lp->l_index;
			/* make sure buffer is large enough to hold response */
			if ((nbp = allocb(sizeof(union N_primitives), BPRI_HI))
			    == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(ICMPM_ID, 0, 2, SL_TRACE,
				     "I_LINK failed: Can't alloc bind buf");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_bind_req);
			bindr = (struct N_bind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_BIND_REQ;
			bindr->N_sap = IPPROTO_ICMP;
			if (icmp_qbot) {
				putnext(icmp_qbot, nbp);
			} else {
				freemsg(nbp);
			}
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(ICMPM_ID, 0, 5, SL_TRACE, "I_LINK succeeded");
			qreply(q, bp);
			((struct inpcb *) q->q_ptr)->inp_state |=
				SS_CANTRCVMORE;
			return;
		}

	case I_PUNLINK:
	case I_UNLINK:
		{
			struct linkblk *lp;
			mblk_t         *nbp;
			struct N_unbind_req *bindr;

			lp = (struct linkblk *) bp->b_cont->b_rptr;
			iocbp->ioc_count = 0;
			if (icmp_index != lp->l_index) {
				iocbp->ioc_error = ENXIO;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(ICMPM_ID, 0, 3, SL_TRACE,
				       "I_UNLINK: wrong index = %x",
				       lp->l_index);
				qreply(q, bp);
				return;
			}
			/* Do the network level unbind */

			/* make sure buffer is large enough to hold response */
			if ((nbp = allocb(sizeof(union N_primitives),
					  BPRI_HI)) == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(ICMPM_ID, 0, 2, SL_TRACE,
				       "I_UNLINK: no buf for unbind");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_unbind_req);
			bindr = (struct N_unbind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_UNBIND_REQ;
			putnext(lp->l_qbot, nbp);
			icmp_qbot = NULL;
			icmp_index = 0;
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(ICMPM_ID, 0, 5, SL_TRACE, "I_UNLINK succeeded");
			qreply(q, bp);
			return;
		}
	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, icmpm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
		if (icmp_qbot == NULL) {
			iocbp->ioc_error = ENXIO;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(ICMPM_ID, 4, 2, SL_TRACE,
			       "icmpioctl: not linked");
			iocbp->ioc_count = 0;
			qreply(q, bp);
			return;
		}
		if (msgblen(bp) < sizeof(struct iocblk_in)) {
			if (bpsize(bp) < sizeof(struct iocblk_in)) {
				mblk_t         *nbp;

				nbp = allocb(sizeof(struct iocblk_in), BPRI_MED);
				if (!nbp) {
					iocbp->ioc_error = ENOSR;
					bp->b_datap->db_type = M_IOCNAK;
					STRLOG(ICMPM_ID, 4, 3, SL_TRACE,
					 "icmpioctl: can't enlarge iocblk");
					qreply(q, bp);
					return;
				}
				bcopy(bp->b_rptr, nbp->b_rptr, sizeof(struct iocblk));
				nbp->b_cont = bp->b_cont;
				nbp->b_datap->db_type = bp->b_datap->db_type;
				freeb(bp);
				bp = nbp;
				iocbp = (struct iocblk *) bp->b_rptr;
			}
			bp->b_wptr = bp->b_rptr + sizeof(struct iocblk_in);
		}
		((struct iocblk_in *) iocbp)->ioc_transport_client = RD(q);
		putnext(icmp_qbot, bp);
		return;
	}
}


/*
 * this is the subfunction of the upper put routine which handles data and
 * protocol packets for us.
 */

icmp_state(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	register union T_primitives *t_prim;
	register struct inpcb *inp = qtoinp(q);
	int             error = 0;
	mblk_t         *head;
	struct sockaddr_in *sin;
	struct in_addr  laddr;

	/*
	 * check for pending error, or a broken state machine
	 */

	STRLOG(ICMPM_ID, 3, 9, SL_TRACE, "got to icmp_state");
	if (inp->inp_error != 0) {
		T_errorack(q, bp, TSYSERR, inp->inp_error);
		return;
	}
	/* just send pure data, if we're ready */
	if (bp->b_datap->db_type == M_DATA) {
		if ((inp->inp_state & SS_ISCONNECTED) != 0) {
			icmp_output(inp, bp);
		} else {
			CHECKSIZE(bp, sizeof(struct T_error_ack));
			bp->b_datap->db_type = M_PCPROTO;
			t_prim = (union T_primitives *) bp->b_rptr;
			bp->b_wptr = bp->b_rptr + sizeof(struct T_error_ack);
			t_prim->type = T_ERROR_ACK;
			t_prim->error_ack.ERROR_prim = T_DATA_REQ;
			t_prim->error_ack.TLI_error = TOUTSTATE;
			qreply(q, bp);
		}
		return;
	}
	/* if it's not data, it's proto or pcproto */

	t_prim = (union T_primitives *) bp->b_rptr;
	STRLOG(ICMPM_ID, 3, 7, SL_TRACE, "Proto msg, type is %d", t_prim->type);

	switch (t_prim->type) {
	case T_INFO_REQ:
		/* our state doesn't matter here */
		CHECKSIZE(bp, sizeof(struct T_info_ack));
		bp->b_rptr = bp->b_datap->db_base;
		bp->b_wptr = bp->b_rptr + sizeof(struct T_info_ack);
		t_prim = (union T_primitives *) bp->b_rptr;
		t_prim->type = T_INFO_ACK;
		t_prim->info_ack.TSDU_size = q->q_maxpsz;
		t_prim->info_ack.ETSDU_size = 1;
		t_prim->info_ack.CDATA_size = -2;	/* ==> not supported */
		t_prim->info_ack.DDATA_size = -2;
		t_prim->info_ack.ADDR_size = sizeof(struct sockaddr_in);
		t_prim->info_ack.OPT_size = -1;
		t_prim->info_ack.TIDU_size = 16 * 1024;
		t_prim->info_ack.SERV_type = T_CLTS;
		t_prim->info_ack.CURRENT_state = inp->inp_tstate;
		t_prim->info_ack.PROVIDER_flag = 0;
		bp->b_datap->db_type = M_PCPROTO;	/* make sure */
		qreply(q, bp);
		break;

	case T_BIND_REQ:
		if (inp->inp_tstate != TS_UNBND) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		if (t_prim->bind_req.CONIND_number > 0) {
			error = EOPNOTSUPP;
			break;
		}
		if (t_prim->bind_req.ADDR_length == 0) {
			error = in_pcbbind(inp, (mblk_t *) NULL);
		} else {
			bp->b_rptr += t_prim->bind_req.ADDR_offset;
			sin = (struct sockaddr_in *) bp->b_rptr;
			if (sin->sin_port == 0)
				sin->sin_port = 1;
			error = in_pcbbind(inp, bp);
			bp->b_rptr -= t_prim->bind_req.ADDR_offset;
		}
		if ( error == EINVAL) {
			T_errorack(q, bp, TBADADDR, 0);
			break;
		}
		else if (error)
			break;
		inp->inp_tstate = TS_IDLE;
		if ((bp = reallocb(bp, sizeof(struct T_bind_ack)
				   + inp->inp_addrlen, 1))
		    == NULL) {
			return;
		}
		t_prim = (union T_primitives *) bp->b_rptr;
		t_prim->bind_ack.PRIM_type = T_BIND_ACK;
		t_prim->bind_ack.ADDR_length = inp->inp_addrlen;
		t_prim->bind_ack.ADDR_offset = sizeof(struct T_bind_ack);
		sin = (struct sockaddr_in *)
			(bp->b_rptr + sizeof(struct T_bind_ack));
		bp->b_wptr = (unsigned char *)
			(((caddr_t) sin) + inp->inp_addrlen);
		bzero((caddr_t) sin, inp->inp_addrlen);
		sin->sin_family = inp->inp_family;
		sin->sin_addr = inp->inp_laddr;
		sin->sin_port = inp->inp_lport;
		bp->b_datap->db_type = M_PCPROTO;
		qreply(q, bp);
		break;

	case T_UNBIND_REQ:
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		in_pcbdisconnect(inp);
		inp->inp_laddr.s_addr = INADDR_ANY;
		inp->inp_lport = 0;
		inp->inp_tstate = TS_UNBND;
		T_okack(q, bp);
		break;

		/*
		 * Initiate connection to peer. For icmp this is simply faked
		 * by asigning a pseudo-connection, and sending up a
		 * conection confirmation.
		 */
	case T_CONN_REQ:
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		bp->b_rptr += t_prim->conn_req.DEST_offset;
		sin = (struct sockaddr_in *) bp->b_rptr;
		if (sin->sin_port == 0)
			sin->sin_port = 1;
		error = in_pcbconnect(inp, bp);
		bp->b_rptr -= t_prim->conn_req.DEST_offset;
		if (error == EINVAL) {
			error = 0;
			T_errorack(q, bp, TBADADDR, 0);
			break;
		}
		else if (error)
			break;
		T_okack(q, bp);
		T_conn_con(inp);
		break;

	case T_DISCON_REQ:
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		in_pcbdisconnect(inp);
		inp->inp_state &= ~SS_ISCONNECTED;	/* XXX */
		T_okack(q, bp);
		break;

	case T_OPTMGMT_REQ:
		icmp_ctloutput(q, bp);
		break;

	case T_DATA_REQ:
		if ((inp->inp_state & SS_ISCONNECTED) == 0) {
			freemsg(bp);	/* TLI doesn't want errors here */
			break;
		}
		head = bp;
		bp = bp->b_cont;
		head->b_cont = NULL;
		freeb(head);
		if (bp == NULL) {
			break;
		}
		icmp_output(inp, bp);
		break;

	case T_UNITDATA_REQ:
		laddr = inp->inp_laddr;
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		if (bp->b_cont == NULL) {
			freeb(bp);
			break;
		}
		bp->b_rptr += t_prim->unitdata_req.DEST_offset;
		sin = (struct sockaddr_in *) bp->b_rptr;
		if (sin->sin_port == 0)
			sin->sin_port = 1;
		if (error = in_pcbconnect(inp, bp))
			break;
		head = bp;
		bp = bp->b_cont;
		head->b_cont = NULL;
		freeb(head);
		icmp_output(inp, bp);
		in_pcbdisconnect(inp);
		inp->inp_laddr = laddr;
		break;

	case T_CONN_RES:
	case T_ORDREL_REQ:
	case T_EXDATA_REQ:
	default:
		T_errorack(q, bp, TNOTSUPPORT, 0);
		return;
	}
	if (error)
		T_errorack(q, bp, TSYSERR, error);
}

icmplrput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	union N_primitives *op;

	switch (bp->b_datap->db_type) {
	case M_PROTO:
	case M_PCPROTO:
		op = (union N_primitives *) bp->b_rptr;
		switch (op->prim_type) {
		case N_BIND_ACK:
			STRLOG(ICMPM_ID, 1, 5, SL_TRACE, "got bind ack");
			freemsg(bp);
			break;

		case N_UNITDATA_IND:
			putq(q, bp);
			break;

		case N_ERROR_ACK:
			STRLOG(ICMPM_ID, 3, 3, SL_TRACE,
			       "ERROR ACK: prim = %d, net error = %d, UNIX error = %d",
			       op->error_ack.ERROR_prim,
			       op->error_ack.N_error,
			       op->error_ack.UNIX_error);
			freemsg(bp);
			break;

		case N_OK_ACK:
			STRLOG(ICMPM_ID, 3, 9, SL_TRACE,
			       "Got OK ack, prim = %x",
			       op->ok_ack.CORRECT_prim);
			freemsg(bp);
			break;

		case N_UDERROR_IND:
			STRLOG(ICMPM_ID, 2, 1, SL_TRACE,
			       "IP level error, type = %x",
			       op->error_ind.ERROR_type);
			icmp_uderr(bp);
			freemsg(bp);
			break;

		default:
			STRLOG(ICMPM_ID, 3, 3, SL_TRACE,
			       "stray icmp PROTO type %d", op->prim_type);
			freemsg(bp);
			break;
		}
		break;

	case M_IOCACK:
	case M_IOCNAK:{
			struct iocblk_in *iocbp = (struct iocblk_in *) bp->b_rptr;

			putnext(iocbp->ioc_transport_client, bp);
			break;
		}

	case M_FLUSH:
		/*
		 * Flush read queue free msg (can't route upstream)
		 */
		STRLOG(ICMPM_ID, 1, 6, SL_TRACE, "Got flush message type = %x",
		       *bp->b_rptr);
		if (*bp->b_rptr & FLUSHR)
			flushq(q, FLUSHALL);
		if (*bp->b_rptr & FLUSHW) {
			*bp->b_rptr &= ~FLUSHR;
			flushq(WR(q), FLUSHALL);
			qreply(q, bp);
		} else
			freemsg(bp);
		return;

	case M_CTL:
		icmp_ctlinput(bp);
		freemsg(bp);
		break;

	default:
		freemsg(bp);
		break;

	}
}

icmplrsrv(q)
	queue_t        *q;
{
	mblk_t         *bp;

	while (bp = getq(q)) {
		icmp_input(q, bp);
	}
}

icmpinit()
{
	icmb.inp_next = icmb.inp_prev = &icmb;
	icmpinited = 1;
}

/*
 * From here down is the real goo of the icmp protocol.
 */

/*
 * Generate an error packet of type error in response to bad packet ip.
 */
/* VARARGS4 */
icmp_error(oip, type, code, q, dest)
	struct ip      *oip;
	int             type, code;
	queue_t        *q;
	struct in_addr  dest;
{
	register unsigned oiplen = oip->ip_hl << 2;
	register struct icmp *icp;
	mblk_t         *bp;
	struct ip      *nip;
	unsigned        icmplen;
	struct ip_provider *prov;

	STRLOG(ICMPM_ID, 0, 1, SL_TRACE,
	       "icmp_error(%x, %d, %d)\n", oip, type, code);
	if (q != NULL) {
		prov = (struct ip_provider *) q->q_ptr;
	} else {
		prov = prov_withaddr(oip->ip_dst);
	}
	if (prov == NULL) {
		return;
	}
	if (type != ICMP_REDIRECT)
		icmpstat.icps_error++;
	/*
	 * Don't send error if not the first fragment of message. Don't EVER
	 * error if the old packet protocol was ICMP. (Could do ECHO, etc,
	 * but not error indications.)
	 */
	if (oip->ip_off & ~(IP_MF | IP_DF))
		return;
	if (oip->ip_p == IPPROTO_ICMP && type != ICMP_REDIRECT &&
		!ICMP_INFOTYPE(((struct icmp *)((caddr_t)oip + oiplen))->icmp_type)) {
		icmpstat.icps_oldicmp++;
		return;
	}
	/*
	 * First, formulate icmp message
	 */
	icmplen = oiplen + MIN(8, oip->ip_len);
	bp = allocb((int) (icmplen + ICMP_MINLEN + oiplen), BPRI_HI);
	if (bp == NULL)
		return;
	bp->b_rptr += oiplen;
	bp->b_wptr += icmplen + ICMP_MINLEN + oiplen;
	icp = (struct icmp *) bp->b_rptr;
	if ((u_int) type > ICMP_MAXTYPE)
#ifdef SYSV
		cmn_err(CE_PANIC, "icmp_error: type %d > ICMP_MAXTYPE %d",
			type, ICMP_MAXTYPE);
#else
		panic ("icmp_error");
#endif SYSV
	icmpstat.icps_outhist[type]++;
	icp->icmp_type = type;
	if (type == ICMP_REDIRECT)
		icp->icmp_gwaddr = dest;
	else
		icp->icmp_void = 0;
	if (type == ICMP_PARAMPROB) {
		icp->icmp_pptr = code;
		code = 0;
	}
	icp->icmp_code = code;
	bcopy((caddr_t) oip, (caddr_t) & icp->icmp_ip, icmplen);
	nip = &icp->icmp_ip;
	nip->ip_len += oiplen;

	/*
	 * Now, copy old ip header in front of icmp message.
	 */
	bp->b_rptr -= oiplen;
	nip = (struct ip *) bp->b_rptr;
	bcopy((caddr_t) oip, (caddr_t) nip, oiplen);
	nip->ip_len = bp->b_wptr - bp->b_rptr;
	nip->ip_p = IPPROTO_ICMP;
	icmp_reflect(bp, prov);
}

static struct in_addr icmpsrc;
static struct in_addr icmpdst;
static struct in_addr icmpgw;
static struct sockaddr_in icmpsin;

/*
 * Process a received ICMP message.
 */
/* ARGSUSED */
icmp_input(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	register struct icmp *icp;
	register struct ip *ip;
	int             icmplen, hlen;
	register int    i;
	struct ip_provider *prov;
	int             code;
	mblk_t         *Obp;
	struct T_unitdata_ind *hdr;
	extern u_char   ip_protox[];
	extern struct in_addr in_makeaddr();
	register struct inpcb *inp;
	int		addrlen = 0;
	struct in_addr	src, dst;
	ushort		srcport, dstport;

	/*
	 * Locate icmp structure in buffer, and check that not corrupted and
	 * of at least minimum length.
	 */
	prov = *((struct ip_provider **)
		 (bp->b_rptr + sizeof(struct N_unitdata_ind)));
	Obp = bp;
	bp = bp->b_cont;
	freeb(Obp);
	ip = (struct ip *) bp->b_rptr;
	icmplen = ip->ip_len;
	hlen = ip->ip_hl << 2;
	STRLOG(ICMPM_ID, 2, 5, SL_TRACE,
	       "icmp_input from %x, len %d\n", ip->ip_src, icmplen);
	if (icmplen < ICMP_MINLEN) {
		icmpstat.icps_tooshort++;
		goto free;
	}
	i = hlen + MIN(icmplen, ICMP_ADVLENMIN);
	if (((bp->b_wptr - bp->b_rptr) < i) &&
	    pullupmsg(bp, i) == 0) {
		icmpstat.icps_tooshort++;
		goto free;
	}
	ip = (struct ip *) bp->b_rptr;
	bp->b_rptr += hlen;
	icp = (struct icmp *) bp->b_rptr;
	if (in_cksum(bp, icmplen)) {
		icmpstat.icps_checksum++;
		goto free;
	}
	bp->b_rptr -= hlen;

	/*
	 * Message type specific processing.
	 */
	STRLOG(ICMPM_ID, 2, 5, SL_TRACE,
	       "icmp_input, type %d code %d proto %d",
	       icp->icmp_type, icp->icmp_code, icp->icmp_ip.ip_p);
	if (icp->icmp_type > ICMP_MAXTYPE)
		goto raw;
	icmpstat.icps_inhist[icp->icmp_type]++;
	code = icp->icmp_code;
	switch (icp->icmp_type) {

	case ICMP_UNREACH:
		if (code > 5)
			goto badcode;
		code += PRC_UNREACH_NET;
		goto deliver;

	case ICMP_TIMXCEED:
		if (code > 1)
			goto badcode;
		code += PRC_TIMXCEED_INTRANS;
		goto deliver;

	case ICMP_PARAMPROB:
		if (code)
			goto badcode;
		code = PRC_PARAMPROB;
		goto deliver;

	case ICMP_SOURCEQUENCH:
		if (code)
			goto badcode;
		code = PRC_QUENCH;
deliver:
		/*
		 * Problem with datagram; advise higher level routines.
		 */
		icp->icmp_ip.ip_len = ntohs((u_short) icp->icmp_ip.ip_len);
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			goto free;
		}
		STRLOG(ICMPM_ID, 0, 1, SL_TRACE,
		       "deliver to protocol %d\n", icp->icmp_ip.ip_p);
		icmpsrc = icp->icmp_ip.ip_dst;
		icmpdst = icp->icmp_ip.ip_src;
		if ( icp->icmp_ip.ip_p == IPPROTO_TCP ) {
			struct tcpiphdr	*tcp = (struct tcpiphdr *)&(icp->icmp_ip);
			srcport = tcp->ti_dport;
			dstport = tcp->ti_sport;
		} else if ( icp->icmp_ip.ip_p == IPPROTO_UDP ) {
			struct udpiphdr	*udp = (struct udpiphdr *)&(icp->icmp_ip);
			srcport = udp->ui_dport;
			dstport = udp->ui_sport;
		} else 
			srcport = dstport = 0;
		genctlmsg(code, icmpsrc, srcport, icmpdst, dstport,
					(int) icp->icmp_ip.ip_p);
		break;

badcode:
		icmpstat.icps_badcode++;
		break;

	case ICMP_ECHO:
		icp->icmp_type = ICMP_ECHOREPLY;
		goto reflect;

	case ICMP_TSTAMP:
		if (icmplen < ICMP_TSLEN) {
			icmpstat.icps_badlen++;
			break;
		}
		icp->icmp_type = ICMP_TSTAMPREPLY;
		icp->icmp_rtime = iptime();
		icp->icmp_ttime = icp->icmp_rtime;	/* bogus, do later! */
		goto reflect;

	case ICMP_IREQ:
#define	satosin(sa)	((struct sockaddr_in *)(sa))
		if (in_netof(ip->ip_src) == 0)
			ip->ip_src = in_makeaddr(in_netof(*PROV_INADDR(prov)),
						 in_lnaof(ip->ip_src));
		icp->icmp_type = ICMP_IREQREPLY;
		goto reflect;

	case ICMP_MASKREQ:
		if (icmplen < ICMP_MASKLEN)
			break;
		icp->icmp_type = ICMP_MASKREPLY;
		icp->icmp_mask = htonl(prov->ia.ia_netmask);
		if (ip->ip_src.s_addr == 0) {
			if (prov->if_flags & IFF_BROADCAST)
				ip->ip_src = 
					satosin(&prov->if_broadaddr)->sin_addr;
			else if (prov->if_flags & IFF_POINTOPOINT)
				ip->ip_src = 
					satosin(&prov->if_dstaddr)->sin_addr;
		}
reflect:
		ip->ip_len += hlen;	/* since ip_input deducts this */
		icmpstat.icps_reflect++;
		icmpstat.icps_outhist[icp->icmp_type]++;
		icmp_reflect(bp, prov);
		return;

	case ICMP_REDIRECT:
		if (icmplen < ICMP_ADVLENMIN || icmplen < ICMP_ADVLEN(icp)) {
			icmpstat.icps_badlen++;
			break;
		}
		/*
		 * Short circuit routing redirects to force immediate change
		 * in the kernel's routing tables.  The message is also
		 * handed to anyone listening on a raw socket (e.g. the
		 * routing daemon for use in updating its tables).
		 */
		icmpgw = ip->ip_src;
		icmpdst = icp->icmp_gwaddr;
		STRLOG(ICMPM_ID, 0, 1, SL_TRACE,
		       "redirect dst %x to %x\n", icp->icmp_ip.ip_dst,
		       icp->icmp_gwaddr);
		if (code == ICMP_REDIRECT_NET || code == ICMP_REDIRECT_TOSNET) {
			icmpsrc =
				in_makeaddr(in_netof(icp->icmp_ip.ip_dst), INADDR_ANY);
			rtredirect(icmpsrc, icmpdst, RTF_GATEWAY, icmpgw);
			icmpsrc = icp->icmp_ip.ip_dst;
			genctlmsg(PRC_REDIRECT_NET, icmpsrc, 0, icmpdst, 0, -1);
		} else {
			icmpsrc = icp->icmp_ip.ip_dst;
			rtredirect(icmpsrc, icmpdst, RTF_GATEWAY | RTF_HOST,
				   icmpgw);
			genctlmsg(PRC_REDIRECT_HOST, icmpsrc, 0, icmpdst, 0, -1);
		}
		break;

		/*
		 * No kernel processing for the following; just fall through
		 * to send to raw listener.
		 */
	case ICMP_ECHOREPLY:
	case ICMP_TSTAMPREPLY:
	case ICMP_IREQREPLY:
	case ICMP_MASKREPLY:
	default:
		break;
	}


raw:

	if (icmb.inp_next == &icmb)
		goto free;

	icmpsin.sin_family = AF_INET;
	icmpsin.sin_addr = ip->ip_src;
	Obp = allocb(sizeof(struct T_unitdata_ind) +
		     sizeof(struct sockaddr_in), BPRI_HI);
	if (Obp == 0) {
		goto free;
	}
	Obp->b_datap->db_type = M_PROTO;
	hdr = (struct T_unitdata_ind *) Obp->b_rptr;
	Obp->b_wptr += sizeof(struct T_unitdata_ind) +
		sizeof(struct sockaddr_in);
	hdr->PRIM_type = T_UNITDATA_IND;
	hdr->SRC_length = sizeof(struct sockaddr_in);
	hdr->SRC_offset = sizeof(struct T_unitdata_ind);
	hdr->OPT_length = 0;
	hdr->OPT_offset = 0;
	bcopy((caddr_t) & icmpsin, (caddr_t) Obp->b_rptr +
	      sizeof(struct T_unitdata_ind), sizeof(struct sockaddr_in));
	Obp->b_cont = bp;

	addrlen = sizeof(struct sockaddr_in);
	src = ip->ip_src;
	dst = ip->ip_dst;
	for (inp = icmb.inp_next; inp != &icmb; inp = inp->inp_next) {
		if (inp->inp_laddr.s_addr != INADDR_ANY &&
		    dst.s_addr != inp->inp_laddr.s_addr)
			continue;
		if ((inp->inp_state & SS_ISCONNECTED) &&
		    inp->inp_faddr.s_addr != src.s_addr)
			continue;
		if (inp->inp_q && (inp->inp_state & SS_CANTRCVMORE) == 0
		    && canput(inp->inp_q->q_next)) {
			if (inp->inp_addrlen != addrlen || inp->inp_family != icmpsin.sin_family) {
				if (!(bp = copyb(Obp)))
					continue;
				bp->b_cont = Obp->b_cont;
				freeb(Obp);
				Obp = bp;
				Obp->b_wptr += (inp->inp_addrlen - addrlen);
				addrlen = inp->inp_addrlen;
				((struct T_unitdata_ind *) Obp->b_rptr)->SRC_length = addrlen;
				icmpsin.sin_family = inp->inp_family;
				((struct sockaddr_in *)(bp->b_rptr + sizeof(struct T_unitdata_ind)))->sin_family = icmpsin.sin_family;
			}
			if (!(bp = dupmsg(Obp)))
				break;
			putnext(inp->inp_q, bp);
		}
	}
	bp = Obp;
free:
	freemsg(bp);
}

/*
 * Reflect the ip packet back to the source
 */
icmp_reflect(bp, prov)
	mblk_t         *bp;
	struct ip_provider *prov;
{
	register struct ip *ip = (struct ip *) bp->b_rptr;
	struct in_addr  t;
	mblk_t         *opts = 0, *ip_srcroute();
	int             optlen = (ip->ip_hl << 2) - sizeof(struct ip);
	register struct ip_provider *prov1;
	extern struct ip_provider *lastprov;

	t = ip->ip_dst;
	ip->ip_dst = ip->ip_src;
	/*
	 * If the incoming packet was addressed directly to us, use dst as
	 * the src for the reply.  Otherwise (broadcast or anonymous), use
	 * the address which corresponds to the incoming interface.
	 */
	for (prov1 = provider; prov1 <= lastprov; prov1++) {
		if (t.s_addr == PROV_INADDR(prov1)->s_addr)
			break;
		if ((prov1->if_flags & IFF_BROADCAST) &&
		    t.s_addr == satosin(&prov1->if_broadaddr)->sin_addr.s_addr)
			break;
	}
	if (prov1 > lastprov) {
		prov1 = prov;
	}
	ip->ip_src = *PROV_INADDR(prov1);
	ip->ip_ttl = MAXTTL;

	if (optlen > 0) {
		int	pad = 0, hlen = 0;

		/*
		 * Retrieve any source routing from the incoming packet and
		 * and merge in the other non-routing options.  Adjust the
		 * IP length.  In case we'll be putting in any extra options,
		 * we had better make sure there's room in the mblk for
		 * them.
		 */

		opts = ip_srcroute(optlen);

		if (!opts)
			opts = allocb(optlen, BPRI_HI);

		if (opts) {

			ip_stripoptions(bp, opts);

			/*
			 * Source routes are always padded.  So, if there
			 * is a source route and we attach this stuff as well,
			 * bad things will happen, because now we will have 
			 * extra padding.  So, if the option length is 
			 * not a multiple of 4, pad it out with NOPs.
			 */

			hlen = (opts->b_wptr - opts->b_rptr) % 4;
			if (hlen) {
				pad = 4 - hlen;
				while (pad > 0) {
					*opts->b_wptr++ = IPOPT_NOP;
					pad--;
				}
			}
		} else
			ip_stripoptions(bp, (mblk_t *)0);
		ip->ip_len -= optlen;
	}
	icmp_send(bp, opts);
}

/*
 * send data originating in user land
 */

icmp_output(inp, bp0)
	register struct inpcb *inp;
	mblk_t         *bp0;
{
	register mblk_t *bp;
	register struct ip *ip;
	register int    len = 0;

	/*
	 * Calculate data length and get a message for ICMP and IP headers.
	 */
	len = msgdsize(bp0);
	bp = allocb(sizeof(struct ip) + ICMP_MINLEN, BPRI_MED);
	if (bp == 0) {
		freemsg(bp0);
		return;
	}
	/*
	 * Fill in mbuf with extended ICMP header and addresses and length
	 * put into network format.
	 */
	bp->b_wptr += sizeof(struct ip);
	bp->b_cont = bp0;
	bp->b_datap->db_type = M_DATA;
	if (pullupmsg(bp, sizeof(struct ip) + ICMP_MINLEN) == 0) {
		freemsg(bp);
		return;
	}
	ip = (struct ip *) bp->b_rptr;
	ip->ip_p = IPPROTO_ICMP;
	ip->ip_src = inp->inp_laddr;
	ip->ip_dst = inp->inp_faddr;
	ip->ip_len = sizeof(struct ip) + len;
	ip->ip_hl = sizeof(struct ip) >> 2;
	ip->ip_ttl = 32;
	icmp_send(bp, inp->inp_options);
}


/*
 * Send an icmp packet back to the ip level, after supplying a checksum.
 */
icmp_send(bp, opts)
	register mblk_t *bp, *opts;
{
	register struct ip *ip = (struct ip *) bp->b_rptr;
	register int    hlen;
	register struct icmp *icp;
	register mblk_t *newbp = NULL;
	register struct ip_unitdata_req *req;

	hlen = ip->ip_hl << 2;
	bp->b_rptr += hlen;
	icp = (struct icmp *) bp->b_rptr;
	icp->icmp_cksum = 0;
	icp->icmp_cksum = in_cksum(bp, ip->ip_len - hlen);
	bp->b_rptr -= hlen;
	STRLOG(ICMPM_ID, 2, 1, SL_TRACE,
	       "icmp_send dst %x src %x\n", ip->ip_dst, ip->ip_src);
	if (icmp_qbot == NULL || !canput(icmp_qbot->q_next)
	    || (newbp = allocb(sizeof(struct ip_unitdata_req), BPRI_HI))
	    == NULL) {
		freemsg(bp);
		if (opts)
			freemsg(opts);
		if (newbp)
			freemsg(newbp);
		return;
	}
	req = (struct ip_unitdata_req *) newbp->b_rptr;
	newbp->b_wptr += sizeof(struct ip_unitdata_req);
	newbp->b_datap->db_type = M_PROTO;
	req->dl_primitive = N_UNITDATA_REQ;
	req->dl_dest_addr_length = 0;
	req->options = opts;
	req->flags = 0;
	req->route.ro_rt = NULL;
	newbp->b_cont = bp;
	putnext(icmp_qbot, newbp);
}

n_time
iptime()
{
	u_long          t;

#ifdef BSD
	struct timeval  atv;

	microtime(&atv);
	t = (atv.tv_sec % (24 * 60 * 60)) * 1000 + atv.tv_usec / 1000;
#else
	t = (hrestime.tv_sec % (24 * 60 * 60)) * (1000);
#endif
	return (htonl(t));
}

/*
 * Give ip a control message for other protocols to use.
 */

genctlmsg(code, srcaddr, srcport, dstaddr, dstport, proto)
	int             code, proto;
	struct in_addr  srcaddr, dstaddr;
	u_short		srcport, dstport;
{
	register mblk_t *bp;
	register struct ip_ctlmsg *ipctl;

	if ((!canput(icmp_qbot)) ||
	    (bp = allocb(sizeof(struct ip_ctlmsg), BPRI_HI)) == NULL) {
		return;
	}
	bp->b_datap->db_type = M_CTL;
	ipctl = (struct ip_ctlmsg *) bp->b_rptr;
	bp->b_wptr += sizeof(struct ip_ctlmsg);
	ipctl->command = code;
	ipctl->src_addr = srcaddr;
	ipctl->dst_addr = dstaddr;
	ipctl->src_port = srcport;
	ipctl->dst_port = dstport;
	ipctl->proto = proto;
	if (icmp_qbot) {
		putnext(icmp_qbot, bp);
	} else {
		freemsg(bp);
	}
}

icmp_ctloutput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	int             in_pcboptmgmt(), ip_options();
	static struct opproc funclist[] = {
		SOL_SOCKET, in_pcboptmgmt,
		IPPROTO_IP, ip_options,
		0, 0
	};

	dooptions(q, bp, funclist);
}

/*
 * icmp_snduderr -- send T_UDERROR_IND to user.
 */
icmp_snduderr(inp)
struct inpcb *inp;
{
	mblk_t *mp;
	struct T_uderror_ind *uderr;
	struct sockaddr_in *sin;

	if (!inp->inp_q)
		return;
	if (!(mp = allocb(sizeof(struct T_uderror_ind)+inp->inp_addrlen, BPRI_HI)))
		return;
	mp->b_wptr += sizeof(struct T_uderror_ind) + inp->inp_addrlen;
	mp->b_datap->db_type = M_PROTO;
	uderr = (struct T_uderror_ind *) mp->b_rptr;
	uderr->PRIM_type = T_UDERROR_IND;
	uderr->DEST_length = inp->inp_addrlen;
	uderr->DEST_offset = sizeof(struct T_uderror_ind);
	uderr->OPT_length = 0;
	uderr->OPT_offset = 0;
	uderr->ERROR_type = inp->inp_error;
	sin = (struct sockaddr_in *) (mp->b_rptr+sizeof(struct T_uderror_ind));
	bzero(sin, inp->inp_addrlen);
	sin->sin_family = inp->inp_family;
	sin->sin_addr = inp->inp_faddr;
	putnext(inp->inp_q, mp);
}

icmp_ctlinput(bp)
	mblk_t         *bp;
{
	struct ip_ctlmsg *ctl;
	extern u_char   inetctlerrmap[];
	int             in_rtchange();
	struct sockaddr_in sin;
	int icmp_snduderr();

	ctl = (struct ip_ctlmsg *) bp->b_rptr;
	if ((unsigned) ctl->command > PRC_NCMDS)
		return;
	if (ctl->src_addr.s_addr == INADDR_ANY)
		return;
	sin.sin_addr.s_addr = ctl->src_addr.s_addr;
	sin.sin_port = 0;

	switch (ctl->command) {

	case PRC_QUENCH:
		break;

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		in_pcbnotify(&icmb, 0, &sin, 0, in_rtchange, 0);
		break;

	default:
		if (inetctlerrmap[ctl->command] == 0)
			return;	/* XXX */
		in_pcbnotify(&icmb, 0, &sin,
			 (int) inetctlerrmap[ctl->command], icmp_snduderr, 0);
	}
}

/*
 * icmp_uderr - process N_UDERROR_IND from IP
 * If the error is not ENOSR and there are endpoints "connected"
 * to this address, send error.
 */
icmp_uderr(bp)
mblk_t *bp;
{
	struct N_uderror_ind *uderr;
	struct sockaddr_in sin;

	uderr = (struct N_uderror_ind *) bp->b_rptr;
	if (uderr->ERROR_type == ENOSR) {
		return;
	}
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = *(struct in_addr *)(bp->b_rptr + uderr->RA_offset);
	in_pcbnotify(&icmb, 0, &sin, uderr->ERROR_type, icmp_snduderr, 0);
}
