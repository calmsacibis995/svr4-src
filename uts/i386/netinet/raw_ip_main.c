/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:raw_ip_main.c	1.3.1.2"

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
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/debug.h>
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
#include <netinet/nihdr.h>
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
#include <netinet/ip_str.h>

/*extern void     bcopy();*/
extern struct ip_provider *prov_withaddr();

mblk_t         *reallocb();
extern mblk_t  *dupmsg();

#define CHECKSIZE(bp,size) if (((bp) = reallocb((bp), (size),0)) == NULL) {\
			return;\
			}


int             nodev(), ripopen(), ripclose(), ripuwput(), ripuwsrv();
int             riplrput(), riplrsrv();

static struct module_info ripm_info[MUXDRVR_INFO_SZ] = {
	RIPM_ID, "rip", 0, 8192, 8192, 1024,
	RIPM_ID, "rip", 0, 8192, 8192, 1024,
	RIPM_ID, "rip", 0, 8192, 8192, 1024,
	RIPM_ID, "rip", 0, 8192, 8192, 1024,
	RIPM_ID, "rip", 0, 8192, 8192, 1024
};

static struct qinit ripurinit =
{NULL, NULL, ripopen, ripclose, NULL, &ripm_info[IQP_RQ], NULL};

static struct qinit ripuwinit =
{ripuwput, ripuwsrv, ripopen, ripclose, NULL, &ripm_info[IQP_WQ], NULL};

static struct qinit riplrinit =
{riplrput, riplrsrv, ripopen, ripclose, NULL, &ripm_info[IQP_MUXRQ], NULL};

static struct qinit riplwinit =
{NULL, NULL, ripopen, ripclose, NULL, &ripm_info[IQP_MUXWQ], NULL};

struct streamtab ripinfo = {&ripurinit, &ripuwinit, &riplrinit,
			     &riplwinit
};


queue_t        *rip_qbot;	/* This had better be IP */
int             rip_index;	/* mux id of lower stream */
struct inpcb	rawcb;

/*
 * These are the basic stream module routines for rip 
 */

int		nrip = 64;
unsigned char   rip_dev[64];	/* bit mask of minor devs */

static int      ripinited;


/* ARGSUSED */
ripopen(q, dev, flag, sflag)
	queue_t        *q;
{
	mblk_t         *bp;
	struct stroptions *sop;
	struct inpcb   *inp;
	register short  i, j, n;
	int             mindev;
	int		error;

	STRLOG(RIPM_ID, 1, 9, SL_TRACE, "ripopen: opening dev %x", dev);

#ifdef SYSV
	if (!suser(u.u_cred)) {
#else
	if (!suser()) {
#endif SYSV
		setuerror(EACCES);
		return (OPENFAIL);
	}
	if (!ripinited)
		ripinit();
	if (sflag == CLONEOPEN) {
		n = (nrip + 7) / 8;
		for (i = 0; i < n; i++) {
			if (rip_dev[i] != 0xFF) {
				break;
			}
		}
		if (i == n) {
			setuerror(ENXIO);
			return (OPENFAIL);
		}
		for (j = 0; j < 8; j++) {
			if ((rip_dev[i] & (1 << j)) == 0) {
				break;
			}
		}
		mindev = (i * 8) + j;
	} else {
		setuerror(EINVAL);
		return (OPENFAIL);
	}
	if (error = rip_attach(q)) {
		setuerror((unsigned short) error);
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
			STRLOG(RIPM_ID, 1, 9, SL_TRACE,
			       "ripopen failed: no memory for stropts");
			rip_detach(qtoinp(q));
			return (OPENFAIL);
		}
	bp->b_datap->db_type = M_SETOPTS;
	bp->b_wptr += sizeof(struct stroptions);
	sop = (struct stroptions *) bp->b_rptr;
	sop->so_flags = SO_HIWAT | SO_LOWAT;
	sop->so_hiwat = ripm_info[IQP_HDRQ].mi_hiwat;
	sop->so_lowat = ripm_info[IQP_HDRQ].mi_lowat;
	putnext(q, bp);
	STRLOG(RIPM_ID, 1, 9, SL_TRACE, "ripopen succeeded");
	inp = (struct inpcb *)q->q_ptr;
	inp->inp_minor = mindev;
	inp->inp_q = q;
	inp->inp_tstate = TS_UNBND;
	rip_dev[i] |= 1 << j;
	return (mindev);
}

ripclose(q)
	queue_t        *q;
{
	struct inpcb   *inp;
	short           i, j;

	STRLOG(RIPM_ID, 1, 5, SL_TRACE, "ripclose: closing pcb @ %x",
	       q->q_ptr);
	inp = (struct inpcb *) q->q_ptr;
	if (inp == NULL) {
		STRLOG(RIPM_ID, 1, 2, SL_TRACE, "ripclose: no pcb!\n");
		setuerror(EINVAL);
		return;
	}
	i = inp->inp_minor;
	j = i % 8;
	i = i / 8;
	rip_dev[i] &= ~(1 << j);
	rip_detach(inp);
	q->q_ptr = NULL;
}


ripuwput(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	STRLOG(RIPM_ID, 3, 9, SL_TRACE, "ripuwput: received strbufs from above");

	switch (bp->b_datap->db_type) {

	case M_IOCTL:
		ripioctl(q, bp);
		break;

	case M_DATA:
	case M_PROTO:
	case M_PCPROTO:

		STRLOG(RIPM_ID, 3, 9, SL_TRACE, "passing data through rip");
		rip_state(q, bp);
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

ripuwsrv(q)
	queue_t      *q;
{
	int           error = 0;
	mblk_t	     *bp;
	union T_primitives  *t_prim;
	struct inpcb	*inp = qtoinp(q);

        while (bp = getq(q)) {
		t_prim = (union T_primitives *)bp->b_rptr;
		if (t_prim->type == T_UNITDATA_REQ) {
			struct sockaddr_in *sin;

			sin = (struct sockaddr_in *)(bp->b_rptr + 
					      t_prim->unitdata_req.DEST_offset);
			error = rip_connaddr(inp, sin);
                	if (!error) {
				error = rip_output(q, bp->b_cont);
				rip_disconnect(inp);
			} else
				freemsg(bp->b_cont);
		} else {
			error = rip_output(q, bp->b_cont);
		}
		if (error > 0 && t_prim->type == T_UNITDATA_REQ) {
			struct T_uderror_ind *ind;
			mblk_t *nbp;

			nbp = allocb(sizeof(struct T_uderror_ind) +
				     t_prim->unitdata_req.DEST_length +
				     t_prim->unitdata_req.OPT_length, BPRI_HI);
			if (!nbp) {
				freeb(bp);
				continue;
			}
			ind = (struct T_uderror_ind *)nbp->b_rptr;
			ind->PRIM_type = T_UDERROR_IND;
			ind->ERROR_type = error;
			ind->DEST_length = t_prim->unitdata_req.DEST_length;
			ind->DEST_offset = sizeof(struct T_uderror_ind);
			bcopy(bp->b_rptr + t_prim->unitdata_req.DEST_offset, 
			      nbp->b_rptr + sizeof(struct T_uderror_ind),
			      t_prim->unitdata_req.DEST_length);
			ind->OPT_length = t_prim->unitdata_req.OPT_length;
			ind->OPT_offset = sizeof(struct T_uderror_ind) +
					  ind->DEST_length;
			bcopy(bp->b_rptr + t_prim->unitdata_req.OPT_offset, 
			      nbp->b_rptr + ind->OPT_offset,
			      t_prim->unitdata_req.OPT_length);
			nbp->b_wptr += ind->OPT_offset + ind->OPT_length;
			nbp->b_datap->db_type = M_PROTO;
			freeb(bp);
                	qreply(q, nbp);
			continue;
		} else if (error < 0) {
			putbq(q, bp);
			return;
		}
		freeb(bp);
        }
}


ripioctl(q, bp)
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
		STRLOG(RIPM_ID, 0, 9, SL_TRACE,
		       "ripioctl: linking new provider");
		iocbp->ioc_count = 0;
		if (rip_qbot) {
			iocbp->ioc_error = ENOSR;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(RIPM_ID, 0, 3, SL_TRACE,
			       "I_LINK failed: rip in use");
			qreply(q, bp);
			return;
		} else {
			struct linkblk *lp;
			struct N_bind_req *bindr;
			mblk_t         *nbp;

			lp = (struct linkblk *) bp->b_cont->b_rptr;
			rip_qbot = lp->l_qbot;
			rip_index = lp->l_index;
			if ((nbp = allocb(sizeof(struct N_bind_req), BPRI_HI))
			    == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(RIPM_ID, 0, 2, SL_TRACE,
				     "I_LINK failed: Can't alloc bind buf");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_bind_req);
			bindr = (struct N_bind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_BIND_REQ;
			bindr->N_sap = IPPROTO_RAW;
			if (rip_qbot) {
				putnext(rip_qbot, nbp);
			} else {
				freemsg(nbp);
			}
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(RIPM_ID, 0, 5, SL_TRACE, "I_LINK succeeded");
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
			if (rip_index != lp->l_index) {
				iocbp->ioc_error = ENXIO;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(RIPM_ID, 0, 3, SL_TRACE,
				       "I_UNLINK: wrong index = %x",
				       lp->l_index);
				qreply(q, bp);
				return;
			}
			/* Do the network level unbind */

			if ((nbp = allocb(sizeof(union N_primitives),
					  BPRI_HI)) == NULL) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
				STRLOG(RIPM_ID, 0, 2, SL_TRACE,
				       "I_UNLINK: no buf for unbind");
				qreply(q, bp);
				return;
			}
			nbp->b_datap->db_type = M_PROTO;
			nbp->b_wptr += sizeof(struct N_unbind_req);
			bindr = (struct N_unbind_req *) nbp->b_rptr;
			bindr->PRIM_type = N_UNBIND_REQ;
			putnext(lp->l_qbot, nbp);
			rip_qbot = NULL;
			rip_index = 0;
			bp->b_datap->db_type = M_IOCACK;
			STRLOG(RIPM_ID, 0, 5, SL_TRACE, "I_UNLINK succeeded");
			qreply(q, bp);
			return;
		}
	case INITQPARMS:
		if (iocbp->ioc_error = initqparms(bp, ripm_info, MUXDRVR_INFO_SZ))
			bp->b_datap->db_type = M_IOCNAK;
		else
			bp->b_datap->db_type = M_IOCACK;
		iocbp->ioc_count = 0;
		qreply(q, bp);
		return;

	default:
		if (rip_qbot == NULL) {
			iocbp->ioc_error = ENXIO;
			bp->b_datap->db_type = M_IOCNAK;
			STRLOG(RIPM_ID, 4, 2, SL_TRACE,
			       "ripioctl: not linked");
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
					STRLOG(RIPM_ID, 4, 3, SL_TRACE,
					 "ripioctl: can't enlarge iocblk");
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
		putnext(rip_qbot, bp);
		return;
	}
}


/*
 * this is the subfunction of the upper put routine which handles data and
 * protocol packets for us. 
 */
static	struct	sockaddr_in ripsin;

rip_state(q, bp)
	queue_t        *q;
	register mblk_t *bp;
{
	register union T_primitives *t_prim;
	register struct inpcb *inp = qtoinp(q);
	int             error = 0;
	struct sockaddr_in *sin;

	/*
	 * check for pending error, or a broken state machine 
	 */

	STRLOG(RIPM_ID, 3, 9, SL_TRACE, "got to rip_state");
	if (inp->inp_error != 0) {
		T_errorack(q, bp, TSYSERR, inp->inp_error);
		return;
	}
	if (bp->b_datap->db_type == M_DATA) {
		CHECKSIZE(bp, sizeof(struct T_error_ack));
		bp->b_datap->db_type = M_PCPROTO;
		t_prim = (union T_primitives *) bp->b_rptr;
		bp->b_wptr = bp->b_rptr + sizeof(struct T_error_ack);
		t_prim->type = T_ERROR_ACK;
		t_prim->error_ack.ERROR_prim = T_DATA_REQ;
		t_prim->error_ack.TLI_error = TOUTSTATE;
		qreply(q, bp);
		return;
	}
	/* if it's not data, it's proto or pcproto */

	t_prim = (union T_primitives *) bp->b_rptr;
	STRLOG(RIPM_ID, 3, 7, SL_TRACE, "Proto msg, type is %d", t_prim->type);

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
			ripsin.sin_family = AF_INET;
			ripsin.sin_addr.s_addr = INADDR_ANY;
			error = rip_bind(inp, &ripsin);
		} else {
			if (!in_chkaddrlen(t_prim->bind_req.ADDR_length)) {
				T_errorack(q, bp, TBADADDR, 0);
				break;
			}
			inp->inp_addrlen = t_prim->bind_req.ADDR_length;
			sin = (struct sockaddr_in *) 
				(bp->b_rptr + t_prim->bind_req.ADDR_offset);
			error = rip_bind(inp, sin);
		}
		if (error)
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
		bp->b_datap->db_type = M_PCPROTO;
		qreply(q, bp);
		break;

	case T_UNBIND_REQ:
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		inp->inp_laddr.s_addr = INADDR_ANY;
		inp->inp_tstate = TS_UNBND;
		T_okack(q, bp);
		break;

	case T_CONN_REQ:
		if (inp->inp_tstate != TS_IDLE) {
			T_errorack(q, bp, TOUTSTATE, 0);
			break;
		}
		if (!(in_chkaddrlen(t_prim->conn_req.DEST_length))) {
			T_errorack(q, bp, TBADADDR, 0);
			return;
		}
		inp->inp_addrlen = t_prim->conn_req.DEST_length;
		sin = (struct sockaddr_in *) 
				(bp->b_rptr + t_prim->conn_req.DEST_offset);
		error = rip_connaddr(inp, sin);
		if (error) {
			T_errorack(q, bp, TBADADDR, 0);
			return;
		}
		T_okack(q, bp);
		T_conn_con(inp);
		break;

	case T_DISCON_REQ:
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		rip_disconnect(inp);
		inp->inp_state &= ~SS_ISCONNECTED;	/* XXX */
		T_okack(q, bp);
		break;

	case T_OPTMGMT_REQ:
		rip_ctloutput(q, bp);
		break;

	case T_DATA_REQ:
		if ((inp->inp_state & SS_ISCONNECTED) == 0) {
			freemsg(bp);	/* TLI doesn't want errors here */
			break;
		}
		if (bp->b_cont == NULL) {
                        freeb(bp);
                        break;
                }
		putq(q, bp);
		break;

	case T_UNITDATA_REQ:
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		if (bp->b_cont == NULL) {
			freeb(bp);
			break;
		}
		if (!in_chkaddrlen(t_prim->unitdata_req.DEST_length)) {
			T_errorack(q, bp, TBADADDR, 0);
			return;
		}
		inp->inp_addrlen = t_prim->unitdata_req.DEST_length;
		putq(q, bp);
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

riplrput(q, bp)
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
			STRLOG(RIPM_ID, 1, 5, SL_TRACE, "got bind ack");
			freemsg(bp);
			break;

		case N_UNITDATA_IND:
			putq(q, bp);
			break;

		case N_ERROR_ACK:
			STRLOG(RIPM_ID, 3, 3, SL_TRACE,
			       "ERROR ACK: prim = %d, net error = %d, ctix error = %d",
			       op->error_ack.ERROR_prim,
			       op->error_ack.N_error,
			       op->error_ack.UNIX_error);
			freemsg(bp);
			break;

		case N_OK_ACK:
			STRLOG(RIPM_ID, 3, 9, SL_TRACE,
			       "Got OK ack, prim = %x",
			       op->ok_ack.CORRECT_prim);
			freemsg(bp);
			break;

		case N_UDERROR_IND:
			STRLOG(RIPM_ID, 3, 9, SL_TRACE,
			       "IP level error, type = %x",
			       op->error_ind.ERROR_type);
			rip_uderr(bp);
			freemsg(bp);
			break;

		default:
			STRLOG(RIPM_ID, 3, 3, SL_TRACE,
			       "stray rip PROTO type %d", op->prim_type);
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
		STRLOG(RIPM_ID, 1, 6, SL_TRACE, "Got flush message type = %x",
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
		rip_ctlinput(bp);
		freemsg(bp);
		break;

	default:
		freemsg(bp);
		break;

	}
}

riplrsrv(q)
	queue_t        *q;
{
	mblk_t         *bp;

	while (bp = getq(q)) {
		rip_input(q, bp);
	}
}

ripinit()
{
	rawcb.inp_next = rawcb.inp_prev = &rawcb;
	ripinited = 1;
}

/*
 * rip_snduderr -- send T_UDERROR_IND to user.
 */
rip_snduderr(inp)
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
	sin->sin_port = inp->inp_fport;
	putnext(inp->inp_q, mp);
}

/*
 * rip_uderr - process N_UDERROR_IND from IP
 * If the error is not ENOSR and there are endpoints "connected"
 * to this address, send error.
 */
rip_uderr(bp)
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
	in_pcbnotify(&rawcb, 0, &sin, uderr->ERROR_type, rip_snduderr, 0);
}

rip_ctlinput(bp)
	mblk_t         *bp;
{
	struct ip_ctlmsg *ctl;
	extern u_char   inetctlerrmap[];
	int             in_rtchange();
	struct sockaddr_in sin;

	ctl = (struct ip_ctlmsg *) bp->b_rptr;
	if ((unsigned) ctl->command > PRC_NCMDS)
		return;
	if (ctl->src_addr.s_addr == INADDR_ANY)
		return;
	sin.sin_family = AF_INET;
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
		in_pcbnotify(&rawcb, 0, &sin, 0, in_rtchange, 0);
		break;

	default:
		if (inetctlerrmap[ctl->command] == 0)
			return;	/* XXX */
		/*
		 * this is done based on the proto (since we have it)
		 */
		rip_notify(&rawcb, &sin, ctl->proto,
			 (int) inetctlerrmap[ctl->command]);
	}
}


rip_notify(head, dst, proto, errno)
	struct inpcb   *head;
	register struct sockaddr_in *dst;
	int		proto, errno;
{
	register struct inpcb *inp, *oinp;
	mblk_t *mp, *nmp;
	struct T_uderror_ind *uderr;
	int addrlen;
	short family;

	STRLOG(IPM_ID, 3, 4, SL_TRACE,
	     "rip_notify: sending error %d to pcbs from %x", errno, head);

	if (!(mp = allocb(sizeof(struct T_uderror_ind)+sizeof(*dst), BPRI_HI)))
		return;
	mp->b_wptr += sizeof(struct T_uderror_ind) + sizeof(*dst);
	mp->b_datap->db_type = M_PROTO;
	uderr = (struct T_uderror_ind *) mp->b_rptr;
	uderr->PRIM_type = T_UDERROR_IND;
	uderr->DEST_length = sizeof(*dst);
	uderr->DEST_offset = sizeof(struct T_uderror_ind);
	uderr->OPT_length = 0;
	uderr->OPT_offset = 0;
	uderr->ERROR_type = errno;
	bcopy(dst, mp->b_rptr+sizeof(struct T_uderror_ind),
		sizeof(*dst));

	addrlen = sizeof(*dst);
	family = dst->sin_family;
	for (inp = head->inp_next; inp != head; inp = inp->inp_next) {
		if (inp->inp_proto != proto)
			continue;
		if (errno)
			inp->inp_error = errno;
		/*
		 * send T_UDERROR_IND up
		 */
		if (!inp->inp_q)
			continue;
		if (inp->inp_addrlen != addrlen || inp->inp_family != family) {
			if (!(nmp = copyb(mp)))
				continue;
			freeb(mp);
			mp = nmp;
			mp->b_wptr += (inp->inp_addrlen - addrlen);
			addrlen = inp->inp_addrlen;
			((struct T_uderror_ind *)mp->b_rptr)->DEST_length = addrlen;
			family = inp->inp_family;
			((struct sockaddr_in *) (mp->b_rptr + sizeof(struct T_uderror_ind)))->sin_family = family;
		}
		if (!(nmp = dupb(mp)))
			break;
		putnext(inp->inp_q, nmp);
	}
	freeb(mp);
}
