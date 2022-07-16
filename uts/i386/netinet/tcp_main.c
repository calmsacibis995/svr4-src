/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:tcp_main.c	1.3"

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

/*
 * This is the main stream interface module for the DoD Transmission Control
 * Protocol (TCP).  Here, we deal with the stream setup and tear-down.  The
 * TPI state machine processing is in tcp_state.c and the specific I/O packet
 * handling happens in tcp_input.c and tcp_output.c 
 */


#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/errno.h>
#include <sys/signal.h>
#ifdef SYSV
#include <sys/cred.h>
#include <sys/proc.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif
#endif /* SYSV */
#include <sys/user.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/conf.h>
#include <sys/debug.h>
#ifdef SYSV
#include <sys/tihdr.h>
#else
#include <nettli/tihdr.h>
#endif SYSV
#include <sys/timod.h>
#include <netinet/nihdr.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/strioc.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/in_systm.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/ip_var.h>
#include <netinet/ip_str.h>
#include <netinet/tcp.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcpip.h>
#include <sys/kmem.h>

int             nodev(), tcpopen(), tcpclose(), tcp_deqdata(), tcpuwput(), tcpuwsrv();
int             tcplrput(), tcplrsrv(), tcplwsrv();

static struct module_info tcpm_info[MUXDRVR_INFO_SZ] = {
	TCPM_ID, "tcp", 0, 4096, 4096, 1024,	/* IQP_RQ */
	TCPM_ID, "tcp", 0, 4096, 4096, 1024,	/* IQP_WQ */
	TCPM_ID, "tcp", 0, 4096, 4096, 1024,	/* IQP_HDRQ */
	TCPM_ID, "tcp", 0, 4096, 4096, 1024,	/* IQP_MUXRQ */
	TCPM_ID, "tcp", 0, 4096, 4096, 1024	/* IQP_MUXWQ */
};
static struct qinit tcpurinit =
{NULL, tcp_deqdata, tcpopen, tcpclose, NULL, &tcpm_info[IQP_RQ], NULL};

static struct qinit tcpuwinit =
{tcpuwput, tcpuwsrv, tcpopen, tcpclose, NULL, &tcpm_info[IQP_WQ], NULL};

static struct qinit tcplrinit =
{tcplrput, tcplrsrv, tcpopen, tcpclose, NULL, &tcpm_info[IQP_MUXRQ], NULL};

static struct qinit tcplwinit =
{NULL, tcplwsrv, tcpopen, tcpclose, NULL, &tcpm_info[IQP_MUXWQ], NULL};

struct streamtab tcpinfo = {&tcpurinit, &tcpuwinit, &tcplrinit, &tcplwinit};

queue_t        *tcp_qbot;
int             tcp_index;
extern int      tcpfastid, tcpslowid;

extern int	tcpdprintf;
static int      tcpinited;

tcp_seq         tcp_iss;
struct inpcb    tcb;

mblk_t	       *tcp_dihdr;

/* configurable parameters */
extern unsigned char   tcp_dev[];	/* bit mask of minor devs */
extern int		ntcp;


#define PLINGER	PZERO+1
#undef min

/*
 * The transport level protocols in the Internet implementation are very odd
 * beasts.  In particular, they have no real minor number, just a pointer to
 * the inpcb struct. 
 */

/* ARGSUSED */
tcpopen(q, dev, flag, sflag)
	queue_t        *q;
{
	mblk_t         *bp;
	struct stroptions *sop;
	short           error;
	struct inpcb   *inp;
	register short  i, j, n;
	int             min;

	STRLOG(TCPM_ID, 1, 9, SL_TRACE, "tcpopen: wq %x dev %x", WR(q), dev);

	if (!tcpinited && (tcpinit(), !tcpinited))
		return (OPENFAIL);
	if (sflag == CLONEOPEN) {
		n = (ntcp + 7) / 8;
		for (i = 0; i < n; i++) {
			if (tcp_dev[i] != 0xFF) {
				break;
			}
		}
		if (i == n) {
			setuerror(ENXIO);
			return (OPENFAIL);
		}
		for (j = 0; j < 8; j++) {
			if ((tcp_dev[i] & (1 << j)) == 0) {
				break;
			}
		}
		min = (i * 8) + j;
	} else {
		if (q->q_ptr)
			return 0;
		setuerror(EINVAL);
		return (OPENFAIL);
	}

	if (error = tcp_attach(q)) {
		setuerror((unsigned short) error);
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
			STRLOG(TCPM_ID, 1, 2, SL_TRACE,
			       "tcpopen failed: no memory for stropts");
			tcp_freespc(qtotcpcb(q));
			return (OPENFAIL);
		}
	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = tcpm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = tcpm_info[IQP_HDRQ].mi_lowat;
	putnext(q, bp);
	inp = (struct inpcb *) q->q_ptr;
	inp->inp_minor = min;
#ifdef SYSV
	if (suser(u.u_cred) != 0) {
#else
	if (suser() != 0) {
#endif SYSV
		inp->inp_state |= SS_PRIV;
	} else {
		setuerror(0);	/* suser sets u_error, so clear */
	}
	if ((inp->inp_protoopt & SO_LINGER) && inp->inp_linger == 0)
		inp->inp_linger = TCP_LINGERTIME;
	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "tcpopen succeeded wq %x tcb %x",
	       WR(q), inp->inp_ppcb);
	tcp_dev[i] |= 1 << j;
	return (min);
}

tcpclose(q)
	queue_t        *q;
{
	struct tcpcb   *tp;
	struct inpcb   *inp;
	short		saveminor;
	short           i, j;
	extern void     lingertimer();
	int		ss;

	ss = splstr();
	ASSERT(q != NULL);
	inp = qtoinp(q);
	STRLOG(TCPM_ID, 1, 5, SL_TRACE, "tcpclose: wq %x pcb %x",
	       WR(q), inp);
	ASSERT(inp != NULL);
	ASSERT(inp == qtoinp(WR(q)));
	tp = (struct tcpcb *) inp->inp_ppcb;
	ASSERT(tp->t_inpcb == inp);
	saveminor = tp->t_inpcb->inp_minor;
	if (inp->inp_protoopt & SO_ACCEPTCONN) {
		struct tcpcb   *ctp;

		inp->inp_protoopt &= ~SO_ACCEPTCONN;
		while (ctp = tp->t_q0) {
			tpqremque(ctp, 0);
			(void) tcp_disconnect(ctp);
		}
		while (ctp = tp->t_q) {
			tpqremque(ctp, 1);
			(void) tcp_disconnect(ctp);
		}
	}
	inp->inp_state |= SS_NOFDREF | SS_CANTRCVMORE;
	inp->inp_tstate = TS_UNBND;
	if (tp->t_state > TCPS_LISTEN)
		tp = tcp_disconnect(tp);
	else
		tp = tcp_close(tp, 0);
	if (tp && tp->t_qsize) {
		if (inp->inp_protoopt & SO_LINGER && inp->inp_linger) {
			lingerstart(tp);

			/*
			 * In case sleep returns prematurely, which it can,
			 * check that we're still doing the linger boogie.
			 */

			while (tp->t_linger) {
				if (sleep((caddr_t) tp, PLINGER | PCATCH)) {
					/*
					 * Caught signal, so later.
					 */
					tp->t_linger = 0;
					tp->t_timer[TCPT_LINGER] = 0;
				 	tcpstat.tcps_lingerabort++;
					tp->t_qsize = 0;
					break;
				}
			}
		} else {
			tp->t_qsize = 0;
		}
	}
	if (q->q_ptr == (caddr_t) inp) {
		inp->inp_q = NULL;
		q->q_ptr = NULL;
	}
	i = saveminor;
	j = i % 8;
	i = i / 8;
	tcp_dev[i] &= ~(1 << j);
	splx(ss);
}

/*
 * tcpuwput is the upper write put routine.  It takes messages from user
 * level for processing.  Protocol requests can fed into the state machine in
 * tcp_state. 
 */

tcpuwput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{

	STRLOG(TCPM_ID, 3, 8, SL_TRACE, "tcpuwput wq %x pcb %x", q, q->q_ptr);

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		tcpioctl(q, bp);
		break;

	case M_IOCDATA:
		tcpiocdata(q, bp);
		break;

	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
		tcp_state(q, bp);
		break;

	case M_FLUSH:
		/*  
		* When flushing the write queue we must update the transmit
		* queue size stored in the PCB.  
		*/

		if (*bp->b_rptr & FLUSHW) {
			int s;

			ASSERT(qtoinp(q));
			ASSERT(intotcpcb(qtoinp(q)));
			s = splstr();
			(intotcpcb(qtoinp(q)))->t_qsize = 0;
			flushq(q, FLUSHALL);
			splx(s);
			*bp->b_rptr &= ~FLUSHW;
		}
		if (*bp->b_rptr & FLUSHR)
			qreply(q, bp);
		else
			freemsg(bp);
		break;

	case M_CTL:		/* No control messages understood */
	default:
		freemsg(bp);
		break;
	}
}

tcpuwsrv(q)
	queue_t        *q;
{
	register struct tcpcb *tp = qtotcpcb(q);

	if (tp == NULL) {
		STRLOG(TCPM_ID, 3, 1, SL_TRACE,
		       "tcpuwsrv: null tp; q %x inp %x", q, q->q_ptr);
		return;
	}
	tcp_io(tp, TF_NEEDOUT, NULL);
}

/*
 * tcpiocdata handles M_IOCDATA messages for transparent ioctls.
 */
tcpiocdata(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct inpcb   *inp;

	inp = qtoinp(q);
	if (inp)
		switch (inp->inp_iocstate) {
		case INP_IOCS_DONAME:
			inet_doname(q, bp);
			break;
			
		default:
			break;
		}
}

tcpioctl(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp;
	struct sockaddr_in *sin;
	struct inpcb   *inp;

	iocbp = (struct iocblk *) bp->b_rptr;

	switch (iocbp->ioc_cmd) {
	case I_PLINK:
	case I_LINK:
		STRLOG(TCPM_ID, 4, 9, SL_TRACE,
		       "tcpioctl: linking new provider");
		iocbp->ioc_count = 0;
		if (tcp_qbot != NULL) {
			iocbp->ioc_error = EBUSY;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(TCPM_ID, 4, 3, SL_TRACE,
			       "I_LINK failed: tcp already linked");
			qreply(q, bp);
			return;
		} else {
			struct linkblk *lp;
			struct N_bind_req *bindr;
			mblk_t         *nbp;

			lp = (struct linkblk *) bp->b_cont->b_rptr;
			tcp_qbot = lp->l_qbot;
			tcp_index = lp->l_index;
			/* make sure buffer is large enough to hold response */
			if ((nbp = allocb(sizeof(union N_primitives), BPRI_HI))
			    == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(TCPM_ID, 0, 2, SL_TRACE,
				     "I_LINK failed: Can't alloc bind buf");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_bind_req);
			bindr = (struct N_bind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_BIND_REQ;
			bindr->N_sap = IPPROTO_TCP;
			putnext(tcp_qbot, nbp);
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_error = 0;
			STRLOG(TCPM_ID, 0, 5, SL_TRACE, "I_LINK succeeded");
			qreply(q, bp);
			return;
		}
	case I_PUNLINK:
	case I_UNLINK:
		{
			struct linkblk *lp;
			mblk_t         *nbp;
			struct N_unbind_req *bindr;

			iocbp->ioc_count = 0;
			lp = (struct linkblk *) bp->b_cont->b_rptr;

			if (tcp_qbot == NULL) {
				iocbp->ioc_error = ENXIO;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(TCPM_ID, 0, 3, SL_TRACE,
				    "I_UNLINK: spurious unlink, index = %x",
				       lp->l_index);
				qreply(q, bp);
				return;
			}
			/* Do the IP unbind */

			/* make sure buffer is large enough to hold response */
			if ((nbp = allocb(sizeof(union N_primitives),
					  BPRI_HI)) == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(TCPM_ID, 0, 2, SL_TRACE,
				       "I_UNLINK: no buf for unbind");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_unbind_req);
			bindr = (struct N_unbind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_UNBIND_REQ;
			putnext(lp->l_qbot, nbp);
			tcp_qbot = NULL;
			tcp_index = 0;
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(TCPM_ID, 0, 5, SL_TRACE, "I_UNLINK succeeded");
			qreply(q, bp);
			return;
		}

	case SIOCGETNAME:	/* obsolete - replaced by TI_GETMYNAME */
		iocbp->ioc_count = 0;
		inp = qtoinp(q);
		if ((bp->b_cont = allocb(inp->inp_addrlen, BPRI_LO))
		    == NULL) {
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_error = ENOSR;
			qreply(q, bp);
			return;
		}
		in_setsockaddr(inp, bp->b_cont);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = inp->inp_addrlen;
		qreply(q, bp);
		return;

	case SIOCGETPEER:	/* obsolete - replaced by TI_GETPEERNAME */
		iocbp->ioc_count = 0;
		inp = qtoinp(q);
		if ((inp->inp_state & SS_ISCONNECTED) == 0) {
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_error = ENOTCONN;
			qreply(q, bp);
			return;
		}
		if ((bp->b_cont = allocb(inp->inp_addrlen, BPRI_LO))
		    == NULL) {
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_error = ENOSR;
			qreply(q, bp);
			return;
		}
		in_setpeeraddr(inp, bp->b_cont);
		bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = inp->inp_addrlen;
		qreply(q, bp);
		return;

	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, tcpm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	case TI_GETMYNAME:
		/* 
		 * inet_doname sets and clears inp_iocstate, so we 
		 * don't have to.
		 */
		inet_doname(q, bp);
		return;

	case TI_GETPEERNAME:
		inp = qtoinp(q);
		if ((inp->inp_state & SS_ISCONNECTED) == 0) {
			bp->b_datap->db_type = M_IOCNAK;
			iocbp->ioc_error = ENOTCONN;
			qreply(q, bp);
			return;
		}
		inet_doname(q, bp);
		return;
		
	default:
		if (tcp_qbot == NULL) {
			iocbp->ioc_error = ENXIO;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(TCPM_ID, 4, 3, SL_TRACE,
			       "tcpioctl: not linked");
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
					STRLOG(TCPM_ID, 4, 3, SL_TRACE,
					  "tcpioctl: can't enlarge iocblk");
					qreply(q, bp);
					return;
				}
				bcopy((caddr_t)bp->b_rptr, (caddr_t)nbp->b_rptr, sizeof(struct iocblk));
				nbp->b_cont = bp->b_cont;
				nbp->b_datap->db_type = bp->b_datap->db_type;
				freeb(bp);
				bp = nbp;
				iocbp = (struct iocblk *) bp->b_rptr;
			}
			bp->b_wptr = bp->b_rptr + sizeof(struct iocblk_in);
		}
		((struct iocblk_in *) iocbp)->ioc_transport_client = RD(q);
		putnext(tcp_qbot, bp);
		return;
	}
}

tcpinit()
{
	struct T_data_ind *di;

	STRLOG(TCPM_ID, 0, 9, SL_TRACE, "tcpinit starting");


	/* allocate header for T_DATA_IND messages */
	if (!(tcp_dihdr = allocb(sizeof(struct T_data_ind), BPRI_HI))) {
		setuerror(ENOSR);
		return;
	}
	tcp_dihdr->b_datap->db_type = M_PROTO;
	tcp_dihdr->b_wptr += sizeof(struct T_data_ind);
	di = (struct T_data_ind *) tcp_dihdr->b_rptr;
	di->PRIM_type = T_DATA_IND;
	di->MORE_flag = 0;

	tcp_iss = 1;		/* wrong */
	tcb.inp_next = tcb.inp_prev = &tcb;

	tcp_slowtimo();
	tcp_fasttimo();

	ipinit();
	ipregister();
	tcpinited = 1;

	STRLOG(TCPM_ID, 0, 5, SL_TRACE, "tcpinit succeeded");
}

/*
 * tcplrput is the lower read put routine.  It takes packets and examines
 * them.  Control packets are dealt with right away and data packets are
 * queued for tcp_input to deal with.  The message formats understood by the
 * M_PROTO messages here are those used by the link level interface (see
 * dlpi.h). 
 */

tcplrput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	union N_primitives *op;
	mblk_t         *head;

	switch (bp->b_datap->db_type) {

	case M_PROTO:
	case M_PCPROTO:
		op = (union N_primitives *) bp->b_rptr;
		switch (op->prim_type) {
		case N_INFO_ACK:
			STRLOG(TCPM_ID, 4, 5, SL_TRACE, "Got Info ACK?");
			freemsg(bp);
			break;

		case N_BIND_ACK:
			STRLOG(TCPM_ID, 0, 5, SL_TRACE, "got bind ack");
			freemsg(bp);
			break;

		case N_ERROR_ACK:
			STRLOG(TCPM_ID, 3, 3, SL_TRACE,
			       "ERROR ACK: prim = %d, net error = %d, unix error = %d",
			       op->error_ack.ERROR_prim,
			       op->error_ack.N_error,
			       op->error_ack.UNIX_error);
			freemsg(bp);
			break;

		case N_OK_ACK:
			STRLOG(TCPM_ID, 3, 8, SL_TRACE,
			       "Got OK ack, prim = %x",
			       op->ok_ack.CORRECT_prim);
			freemsg(bp);
			break;

		case N_UNITDATA_IND:
			if (tcpdprintf)
#ifdef SYSV
				cmn_err(CE_NOTE, "tcplrput: got N_UNITDATA_IND\n");
#else
				printf( "tcplrput: got N_UNITDATA_IND\n");
#endif
			head = bp;
			bp = bp->b_cont;
			freeb(head);
			putq(q, bp);
			break;

		case N_UDERROR_IND:
			STRLOG(TCPM_ID, 2, 1, SL_TRACE,
			       "IP level error, type = %x",
			       op->error_ind.ERROR_type);
			tcp_uderr(bp);
			freemsg(bp);
			break;

		default:
			STRLOG(TCPM_ID, 3, 9, SL_ERROR,
			   "tcplrput: unrecognized prim %d", op->prim_type);
			freemsg(bp);
			break;
		}
		break;

	case M_IOCACK:
	case M_IOCNAK:{
			struct iocblk_in *iocbp = (struct iocblk_in *) bp->b_rptr;

			if (tcpdprintf)
#ifdef SYSV
				cmn_err(CE_NOTE, "tcplrput: got M_IOCACK/NAK\n");
#else
				printf( "tcplrput: got M_IOCACK/NAK\n");
#endif
			putnext(iocbp->ioc_transport_client, bp);
			break;
		}

	case M_CTL:
		if (tcpdprintf)
#ifdef SYSV
			cmn_err(CE_NOTE, "tcplrput: got M_CTL\n");
#else
			printf( "tcplrput: got M_CTL\n");
#endif
		tcp_ctlinput(bp);
		freemsg(bp);
		break;

	case M_FLUSH:
		/*
		 * Flush read queue free msg (can't route upstream) 
		 */
		STRLOG(TCPM_ID, 4, 5, SL_TRACE, "Got flush message type = %x",
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

	default:
		STRLOG(TCPM_ID, 3, 9, SL_ERROR,
		"tcplrput: unexpected block type %d", bp->b_datap->db_type);
		freemsg(bp);
		break;
	}
}

/*
 * tcplrsrv feeds mblks to tcp_input. 
 */

tcplrsrv(q)
	queue_t        *q;
{
	mblk_t         *bp;

	ASSERT(q == RD(tcp_qbot));
	while (bp = getq(q)) {
		tcp_linput(q, bp);
	}
}

/*
 * tcplwsrv will only be called to back enable the queues after flow control
 * blockage from below.
 */

/*ARGSUSED*/
tcplwsrv(q)
	queue_t        *q;
{
#ifdef lint
	q = q;
#endif
}
