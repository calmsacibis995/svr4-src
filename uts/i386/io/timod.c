/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:timod.c	1.3.1.3"
/*
 * Transport Interface Library cooperating module - issue 2
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/debug.h"
#include "sys/signal.h"
#include "sys/tss.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"
#include "sys/kmem.h"
#include "sys/file.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"

/*
 * T_info_ack changed to support XTI.
 * Need to remain compatible with transport
 * providers written before SVR4.
 */
#define	OLD_INFO_ACK_SZ	(sizeof(struct T_info_ack)-sizeof(long))

extern struct tim_tim tim_tim[];
extern int tim_cnt;
extern nulldev();

STATIC int tim_setname();
STATIC mblk_t *tim_filladdr();
STATIC void tim_bcopy();

#define TIMOD_ID	3

/* stream data structure definitions */

int timodopen(), timodclose(), timodput(), timodrsrv(), timodwsrv();
static struct module_info timod_info = {TIMOD_ID, "timod", 0, INFPSZ, 4096, 1024};
static struct qinit timodrinit = { timodput, timodrsrv, timodopen, timodclose, nulldev, &timod_info, NULL};
static struct qinit timodwinit = { timodput, timodwsrv, timodopen, timodclose, nulldev, &timod_info, NULL};
struct streamtab timinfo = { &timodrinit, &timodwinit, NULL, NULL };

int timdevflag = 0;

/*
 * state transition table for TI interface 
 */
#define nr	127		/* not reachable */

char ti_statetbl[TE_NOEVENTS][TS_NOSTATES] = {
				/* STATES */
/* 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16 */

 { 1, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  2, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  4, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr,  0,  3, nr,  3,  3, nr, nr,  7, nr, nr, nr,  6,  7,  9, 10, 11},
 {nr, nr,  0, nr, nr,  6, nr, nr, nr, nr, nr, nr,  3, nr,  3,  3,  3},
 {nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, nr, nr, nr,  3, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr,  3, nr, nr, nr, nr,  3, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr,  7, nr, nr, nr, nr,  7, nr, nr, nr},
 {nr, nr, nr,  5, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr,  8, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, 12, 13, nr, 14, 15, 16, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, 11, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, nr, 11, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr, 10, nr,  3, nr, nr, nr, nr, nr},
 {nr, nr, nr,  7, nr, nr, nr,  7, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr,  9, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, 10, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr,  9, 10, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr, nr, nr, 11,  3, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr,  3, nr, nr,  3,  3,  3, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr, nr, nr, nr, nr,  7, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  9, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
 {nr, nr, nr,  3, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr, nr},
};

/*
 * check to make sure that q is valid and its q_ptr points to a valid
 * tp
 */

void
timodqenable(qp)
queue_t *qp;
{
 	register struct tim_tim *tp;

        tp = (struct tim_tim *) qp->q_ptr;
	if ((unsigned int) tp < (unsigned int) &tim_tim[0])
		return;
	if ((unsigned int) tp >= (unsigned int) &tim_tim[tim_cnt])
		return;
	if (tp->tim_rdq != qp && WR(tp->tim_rdq) != qp)
		return;
	if (tp->tim_flags & USED)
		qenable(qp);
	return;
}

/*
 * timodopen -	open routine gets called when the module gets pushed
 *		onto the stream.
 */
/*ARGSUSED*/
timodopen(q, devp, flag, sflag, crp)
	register queue_t *q;
	dev_t *devp;
	int flag;
	int sflag;
	cred_t *crp;
{
	register struct tim_tim *tp;
	register int s;
	void timtime();

	ASSERT(q != NULL);

	if (q->q_ptr)
		return (0);

	for (tp = tim_tim; tp < &tim_tim[tim_cnt]; tp++)
		if (!(tp->tim_flags & USED))
			break;

	if (tp >= &tim_tim[tim_cnt])
		return (ENOSPC);

	tp->tim_flags = USED;
	tp->tim_rdq = q;
	tp->tim_iocsave = NULL;
	tp->tim_consave = NULL;

	/*
	 * Try to pre-allocate the buffers to be used to hold
	 * the local address and the peer's address.  If we
	 * can't do it, sleep, waking up every TIMWAIT ticks,
	 * and try it again.  If we were opened NDELAY, fail
	 * with EAGAIN.
	 */
	while ((tp->tim_myname = kmem_alloc(PRADDRSZ, KM_NOSLEEP)) == NULL) {
		if (flag & (FNDELAY|FNONBLOCK)) {
			tp->tim_flags = 0;
			return (EAGAIN);
		}
		s = splstr();
		tp->tim_flags |= OPENWAIT;
		tp->tim_mylen = timeout(timtime, (caddr_t)tp, TIMWAIT);
		if (sleep((caddr_t)tp, STOPRI|PCATCH)) {
			tp->tim_flags = 0;
			untimeout(tp->tim_mylen);
			splx(s);
			return (EINTR);
		} else {
			splx(s);
		}
	}
	while ((tp->tim_peername = kmem_alloc(PRADDRSZ, KM_NOSLEEP)) == NULL) {
		if (flag & (FNDELAY|FNONBLOCK)) {
			tp->tim_flags = 0;
			kmem_free(tp->tim_myname, PRADDRSZ);
			return (EAGAIN);
		}
		s = splstr();
		tp->tim_flags |= OPENWAIT;
		tp->tim_peerlen = timeout(timtime, (caddr_t)tp, TIMWAIT);
		if (sleep((caddr_t)tp, STOPRI|PCATCH)) {
			tp->tim_flags = 0;
			untimeout(tp->tim_peerlen);
			splx(s);
			kmem_free(tp->tim_myname, PRADDRSZ);
			return (EINTR);
		} else {
			splx(s);
		}
	}

	tp->tim_mylen = 0;
	tp->tim_peerlen = 0;
	tp->tim_mymaxlen = PRADDRSZ;
	tp->tim_peermaxlen = PRADDRSZ;
	q->q_ptr = (caddr_t)tp;
	WR(q)->q_ptr = (caddr_t)tp;
	return (0);
}

void
timtime(tp)
	struct tim_tim *tp;
{
	if (!(tp->tim_flags & USED))
		return; 
	if (!(tp->tim_flags & OPENWAIT))
		return; 
	tp->tim_flags &= ~OPENWAIT;
	wakeup((caddr_t)tp);
	return; 
}

/*
 * timodclose - This routine gets called when the module gets popped
 * off of the stream.
 */
/*ARGSUSED*/
timodclose(q, flag, crp)
	register queue_t *q;
	int flag;
	cred_t *crp;
{
	register struct tim_tim *tp;
	register mblk_t *mp;
	register mblk_t *nmp;

	ASSERT(q != NULL);

	tp = (struct tim_tim *)q->q_ptr;

	ASSERT(tp != NULL);
	 
	freemsg(tp->tim_iocsave);
	mp = tp->tim_consave;
	while (mp) {
		nmp = mp->b_next;
		freemsg(mp);
		mp = nmp;
	}
	if (tp->tim_mymaxlen != 0)
		kmem_free(tp->tim_myname, tp->tim_mymaxlen);
	if (tp->tim_peermaxlen != 0)
		kmem_free(tp->tim_peername, tp->tim_peermaxlen);
	tp->tim_flags = 0;
	return (0);
}

/*
 * timodput -	Module read/write put procedure.  This is called from
 *		the module, driver, or stream head upstream/downstream.
 *		Handles M_FLUSH messages.  All others are queued to
 *		be handled by the service procedures.
 */
timodput(q, mp)
register queue_t *q;
register mblk_t *mp;
{
	queue_t *rdq;
	queue_t *wrq;

	ASSERT(q != NULL);
	if (q->q_flag & QREADR) {
		rdq = q;
		wrq = WR(q);
	} else {
		wrq = q;
		rdq = RD(q);
	}
	switch (mp->b_datap->db_type) {
	default:
		putq(q, mp);
		break;

	case M_FLUSH:
		/* 
		 * TODO:
		 * do something intelligent if the state gets screwed up 
		 */
		if (*mp->b_rptr & FLUSHBAND) {
			if (*mp->b_rptr & FLUSHW)
				flushband(wrq, FLUSHDATA, *(mp->b_rptr + 1));
			if (*mp->b_rptr & FLUSHR)
				flushband(rdq, FLUSHDATA, *(mp->b_rptr + 1));
		} else {
			if (*mp->b_rptr & FLUSHW)
				flushq(wrq, FLUSHDATA);
			if (*mp->b_rptr & FLUSHR)
				flushq(rdq, FLUSHDATA);
		}
		if (!SAMESTR(q)) {
			switch (*mp->b_rptr & FLUSHRW) {
			case FLUSHR:
				*mp->b_rptr = (*mp->b_rptr & ~FLUSHR) | FLUSHW;
				break;

			case FLUSHW:
				*mp->b_rptr = (*mp->b_rptr & ~FLUSHW) | FLUSHR;
				break;
			}
		}
		putnext(q, mp);
		break;
	}
	return (0);
}

/*
 * timodrsrv -	Module read queue service procedure.  This is called when
 *		messages are placed on an empty queue, when high priority
 *		messages are placed on the queue, and when flow control
 *		restrictions subside.  This code used to be included in a
 *		put procedure, but it was moved to a service procedure
 *		because several points were added where memory allocation
 *		could fail, and there is no reasonable recovery mechanism
 *		from the put procedure.
 */
/*ARGSUSED*/
timodrsrv(q)
register queue_t *q;
{
	register mblk_t *mp;
	register union T_primitives *pptr;
	register struct tim_tim *tp;
	register struct iocblk *iocbp;
	register mblk_t *nbp;
	mblk_t *tmp;
	int size;

	ASSERT(q != NULL);

	tp = (struct tim_tim *)q->q_ptr;
	if (!tp || !(tp->tim_flags & USED))
	    return (0);

rgetnext:
	if ((mp = getq(q)) == NULL)
	    return (0);

	if ((mp->b_datap->db_type < QPCTL) && !bcanput(q->q_next, mp->b_band)) {
	    putbq(q, mp);
	    return (0);
	}

	switch(mp->b_datap->db_type) {
	default:
	    putnext(q, mp);
	    goto rgetnext;

	case M_PROTO:
	case M_PCPROTO:
	    /* assert checks if there is enough data to determine type */

	    ASSERT((mp->b_wptr - mp->b_rptr) >= sizeof(long));

	    pptr = (union T_primitives *)mp->b_rptr;
	    switch (pptr->type) {
	    default:
		putnext(q, mp);
		goto rgetnext;

	    case T_ERROR_ACK:
error_ack:
		ASSERT((mp->b_wptr - mp->b_rptr) == sizeof(struct T_error_ack));

		if (tp->tim_flags & WAITIOCACK) {

		    ASSERT(tp->tim_iocsave != NULL);

		    if (pptr->error_ack.ERROR_prim !=
		      *(long *)tp->tim_iocsave->b_cont->b_rptr) {
			putnext(q, mp);
			goto rgetnext;
		    }

		    switch (pptr->error_ack.ERROR_prim) {
		    case T_INFO_REQ:
		    case T_OPTMGMT_REQ:
		    case T_BIND_REQ:
		    case T_UNBIND_REQ:
			/* get saved ioctl msg and set values */
			iocbp = (struct iocblk *)tp->tim_iocsave->b_rptr;
			iocbp->ioc_error = 0;
			iocbp->ioc_rval = pptr->error_ack.TLI_error;
			if (iocbp->ioc_rval == TSYSERR)
			    iocbp->ioc_rval |= pptr->error_ack.UNIX_error << 8;
			tp->tim_iocsave->b_datap->db_type = M_IOCACK;
			putnext(q, tp->tim_iocsave);
			tp->tim_iocsave = NULL;
			tp->tim_flags &= ~WAITIOCACK;
			freemsg(mp);
			goto rgetnext;
		    }
		} 
		putnext(q, mp);
		goto rgetnext;

	    case T_OK_ACK:
		if (tp->tim_flags & WAITIOCACK) {

		    ASSERT(tp->tim_iocsave != NULL);

		    if (pptr->ok_ack.CORRECT_prim !=
		      *(long *)tp->tim_iocsave->b_cont->b_rptr) {
			putnext(q, mp);
			goto rgetnext;
		    }
		    if (pptr->ok_ack.CORRECT_prim == T_UNBIND_REQ)
			tp->tim_mylen = 0;
		    goto out;
		}
		putnext(q, mp);
		goto rgetnext;

	    case T_BIND_ACK:
		if (tp->tim_flags & WAITIOCACK) {
		    struct T_bind_ack *ackp = (struct T_bind_ack *)mp->b_rptr;
		    caddr_t p;

		    ASSERT(tp->tim_iocsave != NULL);

		    if (*(long *)tp->tim_iocsave->b_cont->b_rptr !=
		      T_BIND_REQ) {
			putnext(q, mp);
			goto rgetnext;
		    }
		    if (tp->tim_mymaxlen != 0) {
			if (ackp->ADDR_length > tp->tim_mymaxlen) {
			    p = kmem_alloc(ackp->ADDR_length, KM_NOSLEEP);
			    if (p == NULL) {
				putbq(q, mp);
				(void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
				return (0);
			    }
			    kmem_free(tp->tim_myname, tp->tim_mymaxlen);
			    tp->tim_myname = p;
			    tp->tim_mymaxlen = ackp->ADDR_length;
		 	}
			tp->tim_mylen = ackp->ADDR_length;
			p = (caddr_t)mp->b_rptr + ackp->ADDR_offset;
			bcopy(p, tp->tim_myname, tp->tim_mylen);
		    }
		    goto out;
		}
		putnext(q, mp);
		goto rgetnext;

	    case T_OPTMGMT_ACK:
		if (tp->tim_flags & WAITIOCACK) {

		    ASSERT(tp->tim_iocsave != NULL);

		    if (*(long *)tp->tim_iocsave->b_cont->b_rptr !=
		      T_OPTMGMT_REQ) {
			putnext(q, mp);
			goto rgetnext;
		    }
		    goto out;
		}
		putnext(q, mp);
		goto rgetnext;

	    case T_INFO_ACK:
		if (tp->tim_flags & WAITIOCACK) {

		    ASSERT(tp->tim_iocsave != NULL);
		    size = mp->b_wptr - mp->b_rptr;
		    ASSERT((size == sizeof(struct T_info_ack)) ||
			(size == OLD_INFO_ACK_SZ));
		    if (*(long *)tp->tim_iocsave->b_cont->b_rptr!=T_INFO_REQ) {
			putnext(q, mp);
			return (0);
		    }
		    q->q_maxpsz = pptr->info_ack.TIDU_size;
		    OTHERQ(q)->q_maxpsz = pptr->info_ack.TIDU_size;
		    if ((pptr->info_ack.SERV_type == T_COTS) ||
		      (pptr->info_ack.SERV_type == T_COTS_ORD)) {
			tp->tim_flags = (tp->tim_flags & ~CLTS) | COTS;
		    } else if (pptr->info_ack.SERV_type == T_CLTS) {
			tp->tim_flags = (tp->tim_flags & ~COTS) | CLTS;
		    }

		    /*
		     * make sure the message sent back is the size of
		     * a T_info_ack.
		     */
		    if (size == OLD_INFO_ACK_SZ) {
			if (mp->b_datap->db_lim - mp->b_wptr < sizeof(long)) {
			    tmp = allocb(sizeof(struct T_info_ack), BPRI_HI);
			    if (tmp == NULL) {
	 			ASSERT((mp->b_datap->db_lim -
				    mp->b_datap->db_base) <
				    sizeof(struct T_error_ack));
				mp->b_rptr = mp->b_datap->db_base;
				mp->b_wptr = mp->b_rptr +
				    sizeof(struct T_error_ack);
				pptr = (union T_primitives *)mp->b_rptr;
				pptr->error_ack.ERROR_prim = T_INFO_ACK;
				pptr->error_ack.TLI_error = TSYSERR;
				pptr->error_ack.UNIX_error = EAGAIN;
				pptr->error_ack.PRIM_type = T_ERROR_ACK;
				mp->b_datap->db_type = M_PCPROTO;
				goto error_ack;
			    } else {
				bcopy((char *)mp->b_rptr, (char *)tmp->b_rptr,
				    size);
				tmp->b_wptr += size;
				pptr = (union T_primitives *)tmp->b_rptr;
				freemsg(mp);
				mp = tmp;
			    }
			}
			mp->b_wptr += sizeof(long);
			pptr->info_ack.PROVIDER_flag = 0;
		    }
		    goto out;
		}
		putnext(q, mp);
		goto rgetnext;

out:
		iocbp = (struct iocblk *)tp->tim_iocsave->b_rptr;
		ASSERT(tp->tim_iocsave->b_datap != NULL);
		tp->tim_iocsave->b_datap->db_type = M_IOCACK;
		mp->b_datap->db_type = M_DATA;
		freemsg(tp->tim_iocsave->b_cont);
		tp->tim_iocsave->b_cont = mp;
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		iocbp->ioc_count = mp->b_wptr - mp->b_rptr;
		putnext(q, tp->tim_iocsave);
		tp->tim_iocsave = NULL;
		tp->tim_flags &= ~WAITIOCACK;
		goto rgetnext;

	    case T_CONN_IND:
		if (tp->tim_peermaxlen != 0) {
		    nbp = dupmsg(mp);
		    if (nbp) {
			nbp->b_next = tp->tim_consave;
			tp->tim_consave = nbp;
		    } else {
			putbq(q, mp);
			if (!bufcall((uint) sizeof(mblk_t), BPRI_MED, timodqenable,
				(caddr_t)q))
					(void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
			return (0);
		    }
		}
		putnext(q, mp);
		goto rgetnext;

	    case T_CONN_CON:
		tp->tim_flags &= ~CONNWAIT;
		putnext(q, mp);
		goto rgetnext;

	    case T_DISCON_IND:
	      {
		struct T_discon_ind *disp;
		struct T_conn_ind *conp;
		mblk_t *pbp = NULL;

		disp = (struct T_discon_ind *)mp->b_rptr;
		tp->tim_flags &= ~(CONNWAIT|LOCORDREL|REMORDREL);
		tp->tim_peerlen = 0;
		for (nbp = tp->tim_consave; nbp; nbp = nbp->b_next) {
		    conp = (struct T_conn_ind *)nbp->b_rptr;
		    if (conp->SEQ_number == disp->SEQ_number)
			break;
		    pbp = nbp;
		}
		if (nbp) {
		    if (pbp)
			pbp->b_next = nbp->b_next;
		    else
			tp->tim_consave = nbp->b_next;
		    freemsg(nbp);
		}
		putnext(q, mp);
		goto rgetnext;
	      }

	    case T_ORDREL_IND:
		if (tp->tim_flags & LOCORDREL) {
		    tp->tim_flags &= ~(LOCORDREL|REMORDREL);
		    tp->tim_peerlen = 0;
		} else {
		    tp->tim_flags |= REMORDREL;
		}
		putnext(q, mp);
		goto rgetnext;
	    }

	case M_IOCACK:
	    iocbp = (struct iocblk *)mp->b_rptr;
	    if (iocbp->ioc_cmd == TI_GETMYNAME) {

		/*
		 * Transport provider supports this ioctl,
		 * so I don't have to.
		 */
		if (tp->tim_mymaxlen != 0) {
		    kmem_free(tp->tim_myname, tp->tim_mymaxlen);
		    tp->tim_mymaxlen = 0;
		    freemsg(tp->tim_iocsave);
		    tp->tim_iocsave = NULL;
		}
	    } else if (iocbp->ioc_cmd == TI_GETPEERNAME) {
		register mblk_t *bp;

		/*
		 * Transport provider supports this ioctl,
		 * so I don't have to.
		 */
		if (tp->tim_peermaxlen != 0) {
		    kmem_free(tp->tim_peername, tp->tim_peermaxlen);
		    tp->tim_peermaxlen = 0;
		    freemsg(tp->tim_iocsave);
		    tp->tim_iocsave = NULL;
		    bp = tp->tim_consave;
		    while (bp) {
			nbp = bp->b_next;
			freemsg(bp);
			bp = nbp;
		    }
		    tp->tim_consave = NULL;
		}
	    }
	    putnext(q, mp);
	    goto rgetnext;

	case M_IOCNAK:
	    iocbp = (struct iocblk *)mp->b_rptr;
	    if (((iocbp->ioc_cmd == TI_GETMYNAME) || (iocbp->ioc_cmd == TI_GETPEERNAME)) &&
	       ((iocbp->ioc_error == EINVAL) || (iocbp->ioc_error == 0))) {
			freemsg(mp);
			if (tp->tim_iocsave) {
			    mp = tp->tim_iocsave;
			    tp->tim_iocsave = NULL;
			    tp->tim_flags |= NAMEPROC;
			    if (ti_doname(WR(q), mp, tp->tim_myname, (uint) tp->tim_mylen,
				tp->tim_peername, (uint) tp->tim_peerlen) != DONAME_CONT) {
				    tp->tim_flags &= ~NAMEPROC;
				}
			    goto rgetnext;
		        }
	    }
	    putnext(q, mp);
	    goto rgetnext;
	}
}

/*
 * timodwsrv -	Module write queue service procedure.
 *		This is called when messages are placed on an empty queue,
 *		when high priority messages are placed on the queue, and
 *		when flow control restrictions subside.  This code used to
 *		be included in a put procedure, but it was moved to a
 *		service procedure because several points were added where
 *		memory allocation could fail, and there is no reasonable
 *		recovery mechanism from the put procedure.
 */
timodwsrv(q)
register queue_t *q;
{
	register mblk_t *mp;
	register union T_primitives *pptr;
	register struct tim_tim *tp;
	register mblk_t *tmp;
 	struct iocblk *iocbp;

	ASSERT(q != NULL);
	tp = (struct tim_tim *)q->q_ptr;
	if (!tp || !(tp->tim_flags & USED))
	    return (0);

wgetnext:
	if ((mp = getq(q)) == NULL)
	    return (0);
	if ((mp->b_datap->db_type < QPCTL) && !bcanput(q->q_next, mp->b_band)) {
	    putbq(q, mp);
	    return (0);
	}

	switch(mp->b_datap->db_type) {
	default:
	    putnext(q, mp);
	    goto wgetnext;

	case M_DATA:
	    if (tp->tim_flags & CLTS) {
		if ((tmp = tim_filladdr(q, mp)) == NULL) {
			putbq(q, mp);
			return (0);
		} else {
			mp = tmp;
		}
	    }
	    putnext(q, mp);
	    goto wgetnext;
	
	case M_IOCTL:
	    iocbp = (struct iocblk *)mp->b_rptr;

	    ASSERT((mp->b_wptr - mp->b_rptr) == sizeof(struct iocblk));

	    if (tp->tim_flags & WAITIOCACK) {
		mp->b_datap->db_type = M_IOCNAK;
		iocbp->ioc_error = EPROTO;
		qreply(q, mp);
		goto wgetnext;
	    }

	    switch (iocbp->ioc_cmd) {
	    default:
		putnext(q, mp);
		goto wgetnext;

	    case TI_BIND:
	    case TI_UNBIND:
	    case TI_GETINFO:
	    case TI_OPTMGMT:
		if (iocbp->ioc_count == TRANSPARENT) {
		    mp->b_datap->db_type = M_IOCNAK;
		    iocbp->ioc_error = EINVAL;
		    qreply(q, mp);
		    goto wgetnext;
		}
		if (mp->b_cont == NULL) {
		    mp->b_datap->db_type = M_IOCNAK;
		    iocbp->ioc_error = EINVAL;
		    qreply(q, mp);
		    goto wgetnext;
		}
		if (!pullupmsg(mp->b_cont, -1)) {
		    mp->b_datap->db_type = M_IOCNAK;
		    iocbp->ioc_error = EAGAIN;
		    qreply(q, mp);
		    goto wgetnext;
		} 
		if ((tmp = copymsg(mp->b_cont)) == NULL) {
		    int i = 0;

		    putbq(q, mp);
		    for (tmp = mp; tmp; tmp = tmp->b_next)
			i += (int)(tmp->b_wptr - tmp->b_rptr);
		    if (!bufcall(i, BPRI_MED, timodqenable, (caddr_t)q))
			 (void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
		    return (0);
		}
		tp->tim_iocsave = mp;
		tp->tim_flags |= WAITIOCACK;
		if (iocbp->ioc_cmd == TI_GETINFO)
		    tmp->b_datap->db_type = M_PCPROTO;
		else
		    tmp->b_datap->db_type = M_PROTO;
		putnext(q, tmp);
		goto wgetnext;

	    case TI_GETMYNAME:
		if (tp->tim_mymaxlen == 0) {
		    putnext(q, mp);
		    goto wgetnext;
		}
		goto getname;

	    case TI_GETPEERNAME:
		if (tp->tim_peermaxlen == 0) {
		    putnext(q, mp);
		    goto wgetnext;
		}
getname:
		if ((tmp = copymsg(mp)) == NULL) {
		    int i = 0;

		    putbq(q, mp);
		    for (tmp = mp; tmp; tmp = tmp->b_next)
			i += (int)(tmp->b_wptr - tmp->b_rptr);
		    if (!bufcall((uint) i, BPRI_MED, timodqenable, (caddr_t)q))
			 (void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
		    return (0);
		}
		tp->tim_iocsave = mp;
		putnext(q, tmp);
		goto wgetnext;

	    case TI_SETMYNAME:

		/*
		 * Kludge ioctl for root only.  If TIMOD is pushed
		 * on a stream that is already "bound", we want
		 * to be able to support the TI_GETMYNAME ioctl if the
		 * transport provider doesn't support it.
		 */
		if (iocbp->ioc_uid != 0)
		    iocbp->ioc_error = EPERM;

		/*
		 * If tim_mymaxlen is zero, the transport provider supports
		 * the TI_GETMYNAME ioctl, so setting the name here won't
		 * be of any use.
		 */
		if (tp->tim_mymaxlen == 0)
		    iocbp->ioc_error = EBUSY;

		goto setname;

	    case TI_SETPEERNAME:

		/*
		 * Kludge ioctl for root only.  If TIMOD is pushed
		 * on a stream that is already "connected", we want
		 * to be able to support the TI_GETPEERNAME ioctl if the
		 * transport provider doesn't support it.
		 */
		if (iocbp->ioc_uid != 0)
		    iocbp->ioc_error = EPERM;

		/*
		 * If tim_peermaxlen is zero, the transport provider supports
		 * the TI_GETPEERNAME ioctl, so setting the name here won't
		 * be of any use.
		 */
		if (tp->tim_peermaxlen == 0)
		    iocbp->ioc_error = EBUSY;

setname:
		if (iocbp->ioc_error == 0) {
		    if (!tim_setname(q, mp))
			return (0);
		} else {
		    mp->b_datap->db_type = M_IOCNAK;
		    freemsg(mp->b_cont);
		    mp->b_cont = NULL;
		    qreply(q, mp);
		}
		goto wgetnext;
	    }

	case M_IOCDATA:
	    if (tp->tim_flags & NAMEPROC) {
		if (ti_doname(q, mp, tp->tim_myname, (uint) tp->tim_mylen,
		  tp->tim_peername, (uint) tp->tim_peerlen) != DONAME_CONT) {
		    tp->tim_flags &= ~NAMEPROC;
		}
		goto wgetnext;
	    }
	    putnext(q, mp);
	    goto wgetnext;

	case M_PROTO:
	case M_PCPROTO:
	    /* assert checks if there is enough data to determine type */
	    ASSERT((mp->b_wptr - mp->b_rptr) >= sizeof(long));

	    pptr = (union T_primitives *)mp->b_rptr;
	    switch (pptr->type) {
	    default:
		putnext(q, mp);
		goto wgetnext;

	    case T_UNITDATA_REQ:
		if (tp->tim_flags & CLTS) {
			if ((tmp = tim_filladdr(q, mp)) == NULL) {
				putbq(q, mp);
				return (0);
			} else {
				mp = tmp;
			}
		}
		putnext(q, mp);
		goto wgetnext;

	    case T_CONN_REQ:
	      {
		struct T_conn_req *reqp = (struct T_conn_req *)mp->b_rptr;
		caddr_t p;

		if (tp->tim_peermaxlen != 0) {
		    if (reqp->DEST_length > tp->tim_peermaxlen) {
			p = kmem_alloc(reqp->DEST_length, KM_NOSLEEP);
			if (p == NULL) {
			    putbq(q, mp);
			    (void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
			    return (0);
			}
			kmem_free(tp->tim_peername, tp->tim_peermaxlen);
			tp->tim_peername = p;
			tp->tim_peermaxlen = reqp->DEST_length;
		    }
		    tp->tim_peerlen = reqp->DEST_length;
		    p = (caddr_t)mp->b_rptr + reqp->DEST_offset;
		    bcopy(p, tp->tim_peername, tp->tim_peerlen);
		    if (tp->tim_flags & COTS)
			tp->tim_flags |= CONNWAIT;
		}
		putnext(q, mp);
		goto wgetnext;
	      }

	    case T_CONN_RES:
	      {
		struct T_conn_res *resp;
		struct T_conn_ind *indp;
		mblk_t *pmp = NULL;
		queue_t *aqp;		/* accepting queue ptr */
		struct tim_tim *ntp;
		int i;
		caddr_t p;

		resp = (struct T_conn_res *)mp->b_rptr;
		for (tmp = tp->tim_consave; tmp; tmp = tmp->b_next) {
		    indp = (struct T_conn_ind *)tmp->b_rptr;
		    if (indp->SEQ_number == resp->SEQ_number)
			break;
		    pmp = tmp;
		}
		if (!tmp) 
		    goto cresout;
		if (pmp)
		    pmp->b_next = tmp->b_next;
		else
		    tp->tim_consave = tmp->b_next;

		/*
		 * Find the correct timod data structure
		 * to copy the peer address to.
		 */
		aqp = resp->QUEUE_ptr;
		while (aqp->q_next && aqp->q_next->q_next)
		    aqp = aqp->q_next;

		/*
		 * aqp->q_next will only be NULL if
		 * someone sent us a T_CONN_RES for a
		 * stream that had no module on it.
		 * Assumption is that the top-most
		 * module on the stream is TIMOD.
		 */
		if (!aqp->q_next)
		    goto cresout;
		ntp = tim_tim;
		for (i = 0; i < tim_cnt; i++) {
		    if ((ntp->tim_flags & USED) && (ntp->tim_rdq == aqp))
			break;
		    ntp++;
		}
		if (i >= tim_cnt)
		    goto cresout;
		if (ntp->tim_peermaxlen != 0) {
		    if (indp->SRC_length > ntp->tim_peermaxlen) {
			p = kmem_alloc(indp->SRC_length, KM_NOSLEEP);
			if (p == NULL) {
			    putbq(q, mp);
			    tmp->b_next = tp->tim_consave;
			    tp->tim_consave = tmp;
			    (void) timeout(timodqenable, (caddr_t)q, TIMWAIT);
			    return (0);
			}
			kmem_free(ntp->tim_peername, ntp->tim_peermaxlen);
			ntp->tim_peername = p;
			ntp->tim_peermaxlen = indp->SRC_length;
		    }
		    ntp->tim_peerlen = indp->SRC_length;
		    p = (caddr_t)tmp->b_rptr + indp->SRC_offset;
		    bcopy(p, ntp->tim_peername, ntp->tim_peerlen);
		}
cresout:
		freemsg(tmp);
		putnext(q, mp);
		goto wgetnext;
	      }

	    case T_DISCON_REQ:
	      {
		struct T_discon_req *disp;
		struct T_conn_ind *conp;
		mblk_t *pmp = NULL;

		disp = (struct T_discon_req *)mp->b_rptr;
		tp->tim_flags &= ~(CONNWAIT|LOCORDREL|REMORDREL);
		tp->tim_peerlen = 0;

		/*
		 * If we are already connected, there won't
		 * be any messages on tim_consave.
		 */
		for (tmp = tp->tim_consave; tmp; tmp = tmp->b_next) {
		    conp = (struct T_conn_ind *)tmp->b_rptr;
		    if (conp->SEQ_number == disp->SEQ_number)
			break;
		    pmp = tmp;
		}
		if (tmp) {
		    if (pmp)
			pmp->b_next = tmp->b_next;
		    else
			tp->tim_consave = tmp->b_next;
		    freemsg(tmp);
		}
		putnext(q, mp);
		goto wgetnext;
	      }

	    case T_ORDREL_REQ:
		if (tp->tim_flags & REMORDREL) {
		    tp->tim_flags &= ~(LOCORDREL|REMORDREL);
		    tp->tim_peerlen = 0;
		} else {
		    tp->tim_flags |= LOCORDREL;
		}
		putnext(q, mp);
		goto wgetnext;
	    }
	
	}
}

/*
 * Process the TI_GETNAME ioctl.  If no name exists, return len = 0
 * in netbuf structures.  The state transitions are determined by what
 * is hung of cq_private (cp_private) in the copyresp (copyreq) structure.
 * The high-level steps in the ioctl processing are as follows:
 *
 * 1) we recieve an transparent M_IOCTL with the arg in the second message
 *	block of the message.
 * 2) we send up an M_COPYIN request for the netbuf structure pointed to
 *	by arg.  The block containing arg is hung off cq_private.
 * 3) we receive an M_IOCDATA response with cp->cp_private->b_cont == NULL.
 *	This means that the netbuf structure is found in the message block
 *	mp->b_cont.
 * 4) we send up an M_COPYOUT request with the netbuf message hung off
 *	cq_private->b_cont.  The address we are copying to is netbuf.buf.
 *	we set netbuf.len to 0 to indicate that we should copy the netbuf
 *	structure the next time.  The message mp->b_cont contains the
 *	address info.
 * 5) we receive an M_IOCDATA with cp_private->b_cont != NULL and
 *	netbuf.len == 0.  Restore netbuf.len to either llen ot rlen.
 * 6) we send up an M_COPYOUT request with a copy of the netbuf message
 *	hung off mp->b_cont.  In the netbuf structure in the message hung
 *	off cq_private->b_cont, we set netbuf.len to 0 and netbuf.maxlen
 *	to 0.  This means that the next step is to ACK the ioctl.
 * 7) we receive an M_IOCDATA message with cp_private->b_cont != NULL and
 *	netbuf.len == 0 and netbuf.maxlen == 0.  Free up cp->private and
 *	send an M_IOCACK upstream, and we are done.
 *
 */
int
ti_doname(q, mp, lname, llen, rname, rlen)
	queue_t *q;		/* queue message arrived at */
	mblk_t *mp;		/* M_IOCTL or M_IOCDATA message only */
	caddr_t lname;		/* local name */
	uint llen;		/* length of local name (0 if not set) */
	caddr_t rname;		/* remote name */
	uint rlen;		/* length of remote name (0 if not set) */
{
	struct iocblk *iocp;
	struct copyreq *cqp;
	struct copyresp *csp;
	struct netbuf *np;
	int ret;
	mblk_t *bp;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		if ((iocp->ioc_cmd != TI_GETMYNAME) &&
		    (iocp->ioc_cmd != TI_GETPEERNAME)) {
			cmn_err(CE_WARN, "ti_doname: bad M_IOCTL command\n");
			iocp->ioc_error = EINVAL;
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			ret = DONAME_FAIL;
			break;
		}
		if ((iocp->ioc_count != TRANSPARENT) ||
		    (mp->b_cont == NULL) || ((mp->b_cont->b_wptr -
		    mp->b_cont->b_rptr) != sizeof(caddr_t))) {
			iocp->ioc_error = EINVAL;
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			ret = DONAME_FAIL;
			break;
		}
		cqp = (struct copyreq *)mp->b_rptr;
		cqp->cq_private = mp->b_cont;;
		cqp->cq_addr = (caddr_t)*(long *)mp->b_cont->b_rptr;
		mp->b_cont = NULL;
		cqp->cq_size = sizeof(struct netbuf);
		cqp->cq_flag = 0;
		mp->b_datap->db_type = M_COPYIN;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		qreply(q, mp);
		ret = DONAME_CONT;
		break;

	case M_IOCDATA:
		csp = (struct copyresp *)mp->b_rptr;
		iocp = (struct iocblk *)mp->b_rptr;
		cqp = (struct copyreq *)mp->b_rptr;
		if ((csp->cp_cmd != TI_GETMYNAME) &&
		    (csp->cp_cmd != TI_GETPEERNAME)) {
			cmn_err(CE_WARN, "ti_doname: bad M_IOCDATA command\n");
			iocp->ioc_error = EINVAL;
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			ret = DONAME_FAIL;
			break;
		}
		if (csp->cp_rval) {	/* error */
			freemsg(csp->cp_private);
			freemsg(mp);
			ret = DONAME_FAIL;
			break;
		}
		ASSERT(csp->cp_private != NULL);
		if (csp->cp_private->b_cont == NULL) {	/* got netbuf */
			ASSERT(mp->b_cont);
			np = (struct netbuf *)mp->b_cont->b_rptr;
			if (csp->cp_cmd == TI_GETMYNAME) {
				if (llen == 0) {
					np->len = 0;	/* copy just netbuf */
				} else if (llen > np->maxlen) {
					iocp->ioc_error = ENAMETOOLONG;
					freemsg(csp->cp_private);
					freemsg(mp->b_cont);
					mp->b_cont = NULL;
					mp->b_datap->db_type = M_IOCNAK;
					qreply(q, mp);
					ret = DONAME_FAIL;
					break;
				} else {
					np->len = llen;	/* copy buffer */
				}
			} else {	/* REMOTENAME */
				if (rlen == 0) {
					np->len = 0;	/* copy just netbuf */
				} else if (rlen > np->maxlen) {
					iocp->ioc_error = ENAMETOOLONG;
					freemsg(mp->b_cont);
					mp->b_cont = NULL;
					mp->b_datap->db_type = M_IOCNAK;
					qreply(q, mp);
					ret = DONAME_FAIL;
					break;
				} else {
					np->len = rlen;	/* copy buffer */
				}
			}
			csp->cp_private->b_cont = mp->b_cont;
			mp->b_cont = NULL;
		}
		np = (struct netbuf *)csp->cp_private->b_cont->b_rptr;
		if (np->len == 0) {
			if (np->maxlen == 0) {

				/*
				 * ack the ioctl
				 */
				freemsg(csp->cp_private);
				iocp->ioc_count = 0;
				iocp->ioc_rval = 0;
				iocp->ioc_error = 0;
				mp->b_datap->db_type = M_IOCACK;
				freemsg(mp->b_cont);
				mp->b_cont = NULL;
				qreply(q, mp);
				ret = DONAME_DONE;
				break;
			}

			/*
			 * copy netbuf to user
			 */
			if (csp->cp_cmd == TI_GETMYNAME)
				np->len = llen;
			else 	/* TI_GETPEERNAME */
				np->len = rlen;
			if ((bp = allocb(sizeof(struct netbuf), BPRI_MED))
			    == NULL) {
				iocp->ioc_error = EAGAIN;
				freemsg(csp->cp_private);
				freemsg(mp->b_cont);
				bp->b_cont = NULL;
				mp->b_datap->db_type = M_IOCNAK;
				qreply(q, mp);
				ret = DONAME_FAIL;
				break;
			}
			bp->b_wptr += sizeof(struct netbuf);
			bcopy((caddr_t) np, (caddr_t) bp->b_rptr,
			    sizeof(struct netbuf));
			cqp->cq_addr =
			    (caddr_t)*(long *)csp->cp_private->b_rptr;
			cqp->cq_size = sizeof(struct netbuf);
			cqp->cq_flag = 0;
			mp->b_datap->db_type = M_COPYOUT;
			mp->b_cont = bp;
			np->len = 0;
			np->maxlen = 0; /* ack next time around */
			qreply(q, mp);
			ret = DONAME_CONT;
			break;
		}

		/*
		 * copy the address to the user
		 */
		if ((bp = allocb(np->len, BPRI_MED)) == NULL) {
			iocp->ioc_error = EAGAIN;
			freemsg(csp->cp_private);
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			ret = DONAME_FAIL;
			break;
		}
		bp->b_wptr += np->len;
		if (csp->cp_cmd == TI_GETMYNAME)
			bcopy((caddr_t) lname, (caddr_t) bp->b_rptr, llen);
		else 	/* TI_GETPEERNAME */
			bcopy((caddr_t) rname, (caddr_t) bp->b_rptr, rlen);
		cqp->cq_addr = (caddr_t)np->buf;
		cqp->cq_size = np->len;
		cqp->cq_flag = 0;
		mp->b_datap->db_type = M_COPYOUT;
		mp->b_cont = bp;
		np->len = 0;	/* copy the netbuf next time around */
		qreply(q, mp);
		ret = DONAME_CONT;
		break;

	default:
		cmn_err(CE_WARN,
		    "ti_doname: freeing bad message type = %d\n",
		    mp->b_datap->db_type);
		freemsg(mp);
		ret = DONAME_FAIL;
		break;
	}
	return (ret);
}

STATIC int
tim_setname(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register struct iocblk *iocp;
	register struct copyreq *cqp;
	register struct copyresp *csp;
	struct tim_tim *tp;
	struct netbuf *netp;
	unsigned int len;
	caddr_t p;

	tp = (struct tim_tim *)q->q_ptr;
	iocp = (struct iocblk *)mp->b_rptr;
	cqp = (struct copyreq *)mp->b_rptr;
	csp = (struct copyresp *)mp->b_rptr;

	switch (mp->b_datap->db_type) {
	case M_IOCTL:
		if ((iocp->ioc_cmd != TI_SETMYNAME) &&
		    (iocp->ioc_cmd != TI_SETPEERNAME)) {
			cmn_err(CE_PANIC, "ti_setname: bad M_IOCTL command\n");
		}
		if ((iocp->ioc_count != TRANSPARENT) ||
		    (mp->b_cont == NULL) || ((mp->b_cont->b_wptr -
		    mp->b_cont->b_rptr) != sizeof(caddr_t))) {
			iocp->ioc_error = EINVAL;
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			break;
		}
		cqp->cq_addr = (caddr_t)*(long *)mp->b_cont->b_rptr;
		freemsg(mp->b_cont);
		mp->b_cont = NULL;
		cqp->cq_size = sizeof(struct netbuf);
		cqp->cq_flag = 0;
		cqp->cq_private = NULL;
		mp->b_datap->db_type = M_COPYIN;
		mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
		qreply(q, mp);
		break;

	case M_IOCDATA:
		if (csp->cp_rval) {
			freemsg(mp);
			break;
		}
		if (csp->cp_private == NULL) {	/* got netbuf */
			netp = (struct netbuf *)mp->b_cont->b_rptr;
			csp->cp_private = mp->b_cont;
			mp->b_cont = NULL;
			cqp->cq_addr = netp->buf;
			cqp->cq_size = netp->len;
			cqp->cq_flag = 0;
			mp->b_datap->db_type = M_COPYIN;
			mp->b_wptr = mp->b_rptr + sizeof(struct copyreq);
			qreply(q, mp);
			break;
		} else {			/* got addr */
			len = msgdsize(mp->b_cont);
			if (csp->cp_cmd == TI_SETMYNAME) {
				if (len > tp->tim_mymaxlen) {
					p = kmem_alloc(len, KM_NOSLEEP);
					if (p == NULL) {
						putbq(q, mp);
						(void) timeout(timodqenable,
						    (caddr_t)q, TIMWAIT);
						return (0);
					}
					kmem_free(tp->tim_myname,
					    tp->tim_mymaxlen);
					tp->tim_myname = p;
					tp->tim_mymaxlen = len;
				}
				tp->tim_mylen = len;
				tim_bcopy(mp->b_cont, tp->tim_myname, len);
			} else if (csp->cp_cmd == TI_SETPEERNAME) {
				if (len > tp->tim_peermaxlen) {
					p = kmem_alloc(len, KM_NOSLEEP);
					if (p == NULL) {
						putbq(q, mp);
						(void) timeout(timodqenable,
						    (caddr_t)q, TIMWAIT);
						return (0);
					}
					kmem_free(tp->tim_peername,
					    tp->tim_peermaxlen);
					tp->tim_peername = p;
					tp->tim_peermaxlen = len;
				}
				tp->tim_peerlen = len;
				tim_bcopy(mp->b_cont, tp->tim_peername, len);
			} else {
				cmn_err(CE_PANIC,
				    "ti_setname: bad M_IOCDATA command\n");
			}
			freemsg(csp->cp_private);
			iocp->ioc_count = 0;
			iocp->ioc_rval = 0;
			iocp->ioc_error = 0;
			mp->b_datap->db_type = M_IOCACK;
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
			qreply(q, mp);
		}
		break;

	default:
		cmn_err(CE_PANIC, "ti_setname: bad message type = %d\n",
		    mp->b_datap->db_type);
	}
	return (1);
}

/*
 * Copy data from a message to a buffer taking into account
 * the possibility of the data being split between multiple
 * message blocks.
 */
STATIC void
tim_bcopy(frommp, to, len)
	mblk_t *frommp;
	register caddr_t to;
	register int len;
{
	register mblk_t *mp;
	register int size;

	mp = frommp;
	while (mp && len > 0) {
		size = MIN((mp->b_wptr - mp->b_rptr), len);
		bcopy((caddr_t)mp->b_rptr, to, size);
		len -= size;
		to += size;
		mp = mp->b_cont;
	}
}

/*
 * Fill in the address of a connectionless data packet if a connect
 * had been done on this endpoint.
 */
STATIC mblk_t *
tim_filladdr(q, mp)
	register queue_t *q;
	register mblk_t *mp;
{
	register mblk_t *bp;
	register struct tim_tim *tp;
	struct T_unitdata_req *up;
	struct T_unitdata_req *nup;

	tp = (struct tim_tim *)q->q_ptr;
	if (mp->b_datap->db_type == M_DATA) {
		bp = allocb(sizeof(struct T_unitdata_req) + tp->tim_peerlen,
		    BPRI_MED);
		if (bp == NULL)
			return (bp);
		bp->b_datap->db_type = M_PROTO;
		up = (struct T_unitdata_req *)bp->b_rptr;
		up->PRIM_type = T_UNITDATA_REQ;
		up->DEST_length = tp->tim_peerlen;
		bp->b_wptr += sizeof(struct T_unitdata_req);
		up->DEST_offset = sizeof(struct T_unitdata_req);
		up->OPT_length = 0;
		up->OPT_offset = 0;
		bcopy((caddr_t) tp->tim_peername, (caddr_t) bp->b_wptr,
		   tp->tim_peerlen);
		bp->b_wptr += tp->tim_peerlen;
		bp->b_cont = mp;
		return (bp);
	} else {
		ASSERT(mp->b_datap->db_type == M_PROTO);
		up = (struct T_unitdata_req *)mp->b_rptr;
		ASSERT(up->PRIM_type == T_UNITDATA_REQ);
		if (up->DEST_length != 0)
			return (mp);
		bp = allocb((mp->b_wptr - mp->b_rptr) + tp->tim_peerlen,
		    BPRI_MED);
		if (bp == NULL)
			return(NULL);
		bp->b_datap->db_type = M_PROTO;
		nup = (struct T_unitdata_req *)bp->b_rptr;
		nup->PRIM_type = T_UNITDATA_REQ;
		nup->DEST_length = tp->tim_peerlen;
		bp->b_wptr += sizeof(struct T_unitdata_req);
		nup->DEST_offset = sizeof(struct T_unitdata_req);
		bcopy((caddr_t) tp->tim_peername, (caddr_t) bp->b_wptr, tp->tim_peerlen);
		bp->b_wptr += tp->tim_peerlen;
		if (up->OPT_length == 0) {
			nup->OPT_length = 0;
			nup->OPT_offset = 0;
		} else {
			nup->OPT_length = up->OPT_length;
			nup->OPT_offset = sizeof(struct T_unitdata_req) +
			    tp->tim_peerlen;
			bcopy((caddr_t) (mp->b_wptr + up->OPT_offset), 
			     (caddr_t) bp->b_wptr, up->OPT_length);
			bp->b_wptr += up->OPT_length;
		}
		bp->b_cont = mp->b_cont;
		mp->b_cont = NULL;
		freeb(mp);
		return(bp);
	}
}
