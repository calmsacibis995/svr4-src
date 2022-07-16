/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:udp_main.c	1.3.1.1"

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
 * This is the main stream interface module for the DoD User Datagram
 * Protocol (UDP).  Here, we deal with the stream setup and tear-down.  The
 * TPI state machine processing is in udp_state.c. 
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
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/conf.h>
#include <sys/debug.h>
#ifdef SYSV
#include <sys/cred.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/tiuser.h>
#include <sys/tihdr.h>
#else
#include <nettli/tiuser.h>
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

struct inpcb    udb;
extern struct inpcb *in_pcballoc();
extern unsigned char   udp_dev[];	/* bit mask of minor devs */
extern int	nudp;

int             nodev(), udpopen(), udpclose(), udpuwput(), udplrput();

static struct module_info udpm_info[MUXDRVR_INFO_SZ] = {
	UDPM_ID, "udp", 0, 16384, 16384, 1024,	/* IQP_RQ	*/
	UDPM_ID, "udp", 1, 16384, 16384, 1024,	/* IQP_WQ	*/
	UDPM_ID, "udp", 0, 16384, 16384, 1024,	/* IQP_HDRQ	*/
	UDPM_ID, "udp", 0, 16384, 16384, 1024,	/* IQP_MUXRQ	*/
	UDPM_ID, "udp", 0, 16384, 16384, 1024	/* IQP_MUXWQ	*/
};
static struct qinit udpurinit =
{NULL, NULL, udpopen, udpclose, NULL, &udpm_info[IQP_RQ], NULL};

static struct qinit udpuwinit =
{udpuwput, NULL, udpopen, udpclose, NULL, &udpm_info[IQP_WQ], NULL};

static struct qinit udplrinit =
{udplrput, NULL, udpopen, udpclose, NULL, &udpm_info[IQP_MUXRQ], NULL};

static struct qinit udplwinit =
{NULL, NULL, udpopen, udpclose, NULL, &udpm_info[IQP_MUXWQ], NULL};

struct streamtab udpinfo = {&udpurinit, &udpuwinit, &udplrinit, &udplwinit};

queue_t        *udp_qbot;
int             udp_index;

static int      udpinited;

/* configurable parameters */
extern u_char	udp_dev[];
extern int     nudp;
	

/*
 * The transport level protocols in the Internet implementation are very odd
 * beasts.  In particular, they have no real minor number, just a pointer to
 * the inpcb struct. 
 */

/* ARGSUSED */
udpopen(q, dev, flag, sflag)
	queue_t        *q;
{
	mblk_t         *bp;
	struct stroptions *sop;
	struct inpcb   *inp;
	register short  i, j, n;
	int             mindev;

	STRLOG(UDPM_ID, 1, 9, SL_TRACE, "udpopen: opening dev %x", dev);

	if (!udpinited)
		udpinit();
	if (sflag == CLONEOPEN) {
		n = (nudp + 7) / 8;
		for (i = 0; i < n; i++) {
			if (udp_dev[i] != 0xFF) {
				break;
			}
		}
		if (i == n) {
			setuerror(ENXIO);
			return (OPENFAIL);
		}
		for (j = 0; j < 8; j++) {
			if ((udp_dev[i] & (1 << j)) == 0) {
				break;
			}
		}
		mindev = (i * 8) + j;
	} else {
		if (q->q_ptr)
	        	return 0;
		setuerror(EINVAL);
		return (OPENFAIL);
	}
	if ((q->q_ptr = (char *) in_pcballoc(&udb)) == NULL) {
		setuerror(ENOSR);
		return (OPENFAIL);
	}
	/*
	 * Set up the correct stream head flow control parameters 
	 */
	while (!(bp = allocb(sizeof(struct stroptions), BPRI_HI)))
#if (ATT > 30) || (INTEL > 30)
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI, 3)) {
#else
		if (strwaitbuf(sizeof(struct stroptions), BPRI_HI)) {
#endif
			STRLOG(UDPM_ID, 1, 9, SL_TRACE,
			       "udpopen failed: no memory for stropts");
			in_pcbdetach((struct inpcb *)q->q_ptr);
			return (OPENFAIL);
		}
	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = udpm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = udpm_info[IQP_HDRQ].mi_lowat;
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
	STRLOG(UDPM_ID, 1, 9, SL_TRACE, "Udpopen succeeded");
	udp_dev[i] |= 1 << j;
	return (mindev);
}

udpclose(q)
	queue_t        *q;
{
	struct inpcb   *inp;
	short           i, j;

	STRLOG(UDPM_ID, 1, 9, SL_TRACE, "udpclose: closing pcb @ %x",
	       q->q_ptr);
	inp = (struct inpcb *) q->q_ptr;
	if (inp == NULL) {
		STRLOG(UDPM_ID, 1, 9, SL_TRACE, "udpclose: no pcb!\n");
		setuerror(EINVAL);
		return;
	}
	i = inp->inp_minor;
	j = i % 8;
	i = i / 8;
	udp_dev[i] &= ~(1 << j);
	in_pcbdetach(inp);
	q->q_ptr = NULL;
}


/*
 * udpuwput is the upper write put routine.  It takes messages from user
 * level for processing.  Protocol requests can fed into the state machine in
 * udp_state. 
 */

udpuwput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{

	STRLOG(UDPM_ID, 3, 9, SL_TRACE, "udpuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		udpioctl(q, bp);
		break;

	case M_IOCDATA:
		udpiocdata(q, bp);
		break;

	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:
		udp_state(q, bp);
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

	case M_CTL:		/* No control messages understood */
	default:
		freemsg(bp);
		break;
	}
}

/*
 * udpiocdata handles M_IOCDATA messages for transparent ioctls.
 */
udpiocdata(q, bp)
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

udpioctl(q, bp)
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
		STRLOG(UDPM_ID, 0, 9, SL_TRACE,
		       "udpioctl: linking new provider");
		iocbp->ioc_count = 0;
		if (udp_qbot != NULL) {
			iocbp->ioc_error = EBUSY;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(UDPM_ID, 0, 9, SL_TRACE,
			       "I_LINK failed: udp already linked");
			qreply(q, bp);
			return;
		} else {
			struct linkblk *lp;
			struct N_bind_req *bindr;
			mblk_t         *nbp;

			lp = (struct linkblk *) bp->b_cont->b_rptr;
			udp_qbot = lp->l_qbot;
			udp_index = lp->l_index;
			/* make sure buffer is large enough to hold response */
			if ((nbp = allocb(sizeof(union N_primitives), BPRI_HI))
			    == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(UDPM_ID, 0, 9, SL_TRACE,
				     "I_LINK failed: Can't alloc bind buf");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_bind_req);
			bindr = (struct N_bind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_BIND_REQ;
			bindr->N_sap = IPPROTO_UDP;
			putnext(udp_qbot, nbp);
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_error = 0;
			STRLOG(UDPM_ID, 0, 9, SL_TRACE, "I_LINK succeeded");
			qreply(q, bp);
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

			if (udp_qbot == NULL) {
				iocbp->ioc_error = ENXIO;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(UDPM_ID, 0, 9, SL_TRACE,
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
				STRLOG(UDPM_ID, 0, 9, SL_TRACE,
				       "I_UNLINK: no buf for unbind");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_unbind_req);
			bindr = (struct N_unbind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_UNBIND_REQ;
			putnext(lp->l_qbot, nbp);
			udp_qbot = NULL;
			udp_index = 0;
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(UDPM_ID, 0, 9, SL_TRACE, "I_UNLINK succeeded");
			qreply(q, bp);
			return;
		}

	case SIOCGETNAME:	/* obsolete - replaced by TI_GETMYNAME */
		inp = qtoinp(q);
		iocbp->ioc_count = 0;
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
		inp = qtoinp(q);
		iocbp->ioc_count = 0;
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
		if (iocbp->ioc_error = initqparms(bp, udpm_info, MUXDRVR_INFO_SZ))
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
		if (udp_qbot == NULL) {
			iocbp->ioc_error = ENXIO;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(UDPM_ID, 4, 2, SL_TRACE,
			       "udpioctl: not linked");
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
					STRLOG(UDPM_ID, 4, 3, SL_TRACE,
					  "udpioctl: can't enlarge iocblk");
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
		putnext(udp_qbot, bp);
		return;
	}
}

udpinit()
{

	STRLOG(UDPM_ID, 0, 9, SL_TRACE, "udpinit starting");
	udb.inp_next = udb.inp_prev = &udb;
	ipregister();
	udpinited = 1;
	STRLOG(UDPM_ID, 0, 9, SL_TRACE, "udpinit succeeded");
}

/*
 * udplrput is the lower read put routine.  It takes packets and examines
 * them.  Control packets are dealt with right away and data packets are
 * handed to udp_input to deal with.  The message formats understood by the
 * M_PROTO messages here are those used by the link level interface (see
 * dlpi.h). 
 */

udplrput(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	union N_primitives *op;

	switch (bp->b_datap->db_type) {

	case M_PROTO:
	case M_PCPROTO:
		op = (union N_primitives *) bp->b_rptr;
		switch (op->prim_type) {
		case N_INFO_ACK:
			STRLOG(UDPM_ID, 3, 9, SL_TRACE, "Got Info ACK?");
			freemsg(bp);
			break;

		case N_BIND_ACK:
			STRLOG(UDPM_ID, 1, 9, SL_TRACE, "got bind ack");
			freemsg(bp);
			break;

		case N_ERROR_ACK:
			STRLOG(UDPM_ID, 3, 9, SL_TRACE,
			       "ERROR ACK: prim = %d, net error = %d, unix error = %d",
			       op->error_ack.ERROR_prim,
			       op->error_ack.N_error,
			       op->error_ack.UNIX_error);
			freemsg(bp);
			break;

		case N_OK_ACK:
			STRLOG(UDPM_ID, 3, 9, SL_TRACE,
			       "Got OK ack, prim = %x",
			       op->ok_ack.CORRECT_prim);
			freemsg(bp);
			break;

		case N_UNITDATA_IND:
			STRLOG(UDPM_ID, 2, 9, SL_TRACE, "Got UNITDATA_IND");
			udp_input(q, bp);
			break;

		case N_UDERROR_IND:
			STRLOG(UDPM_ID, 3, 9, SL_TRACE,
			       "IP level error, type = %x",
			       op->error_ind.ERROR_type);
			udp_uderr(bp);
			freemsg(bp);
			break;
		}
		break;

	case M_IOCACK:
	case M_IOCNAK:{
			struct iocblk_in *iocbp = 
				(struct iocblk_in *) bp->b_rptr;

			STRLOG(UDPM_ID, 3, 9, SL_TRACE, "udplrput: got M_IOCACK/NAK");
			putnext(iocbp->ioc_transport_client, bp);
			break;
		}

	case M_CTL:
		STRLOG(UDPM_ID, 3, 9, SL_TRACE, "udplrput: got M_CTL");
		udp_ctlinput(bp);
		freemsg(bp);
		break;

	case M_FLUSH:
		/*
		 * Flush read queue free msg (can't route upstream) 
		 */
		STRLOG(UDPM_ID, 3, 9, SL_TRACE, "Got flush message type = %x",
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

	}
}

/*
 * udp_snduderr -- send T_UDERROR_IND to user.
 */
udp_snduderr(inp)
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
	bzero((caddr_t)sin, inp->inp_addrlen);
	sin->sin_family = inp->inp_family;
	sin->sin_addr = inp->inp_faddr;
	sin->sin_port = inp->inp_fport;
	putnext(inp->inp_q, mp);
}

/*
 * udp_uderr - process N_UDERROR_IND from IP
 * If the error is not ENOSR and there are endpoints "connected"
 * to this address, send error.
 */
udp_uderr(bp)
mblk_t *bp;
{
	struct N_uderror_ind *uderr;
	struct sockaddr_in sin;

	uderr = (struct N_uderror_ind *) bp->b_rptr;
	if (uderr->ERROR_type == ENOSR) {
		return;
	}
	bzero((caddr_t)&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr = *(struct in_addr *)(bp->b_rptr + uderr->RA_offset);
	in_pcbnotify(&udb, 0, &sin, uderr->ERROR_type, udp_snduderr, 0);
}
