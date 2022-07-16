/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:sockmod.c	1.3.1.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

/*
 * Socket Interface Library cooperating module.
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strsubr.h>
#include <sys/socketvar.h>
#include <sys/tihdr.h>
#include <sys/timod.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/tiuser.h>
#include <sys/debug.h>
#include <sys/strlog.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/cred.h>
#include <sys/errno.h>
#include <sys/kmem.h>
#include <sys/sysmacros.h>
#include <sys/ticlts.h>
#include <sys/sockmod.h>
#include <sys/cmn_err.h>

#define	SIMOD_ID	50
#define	SIMWAIT		(1*HZ)
#define	MSGBLEN(A)	(int)((A)->b_wptr - (A)->b_rptr)
#define	MBLKLEN(A)	(int)((A)->b_datap->db_lim - (A)->b_datap->db_base)
#define	RECOVER(A, B, C) \
			{ \
			putbq((A), (B)); \
			if (!bufcall((C), BPRI_MED, qenable, (caddr_t)(A))) \
				(void)timeout(qenable, (caddr_t)(A), SIMWAIT); \
			}

int sockdevflag = 0;	/* New style module */

typedef struct sockaddr * sockaddr_t;

/*
 * Pointer to the beginning of a list of
 * so_so entries that represent UNIX
 * domain sockets.
 * Has to be global so that netstat(8)
 * can find it for unix domain.
 */
struct so_so	*so_ux_list = (struct so_so *)NULL;

/*
 * Used for freeing the band 1
 * message associated with urgent
 * data handling, and for when a
 * T_DISCON_IND is received.
 */
struct free_ptr {
	frtn_t		free_rtn;
	char		*buf;
	int		buflen;
	queue_t 	*rdq;
	mblk_t		*mp;
	struct so_so	*so;
};

extern	struct so_so	so_so[];
extern 	int		so_cnt;
extern	int		nulldev();

#if defined(__STDC__)
static	int		tlitosyserr(int);
static	long		_t_setsize(long);
STATIC	int		so_options(queue_t *, mblk_t *);
STATIC	mblk_t		*_s_getmblk(mblk_t *, size_t);
STATIC	void		snd_ERRACK(queue_t *, mblk_t *, int, int);
STATIC	void		snd_OKACK(queue_t *, mblk_t *, int);
STATIC	int		so_init(struct so_so *, struct T_info_ack *);
STATIC	void		strip_zerolen(mblk_t *);
STATIC	void		save_addr(struct netbuf *, caddr_t, size_t);
STATIC	void		snd_ZERO(queue_t *);
STATIC	void		snd_FLUSHR(queue_t *);
STATIC	void		snd_ERRORW(queue_t *);
STATIC	void		snd_IOCNAK(queue_t *, mblk_t *, int);
STATIC	void		snd_HANGUP(queue_t *);
STATIC	void		ux_dellink(struct so_so *);
STATIC	void		ux_addlink(struct so_so *);
STATIC	struct so_so	*ux_findlink(caddr_t, size_t);
STATIC	void		ux_restoreaddr(struct so_so *, mblk_t *,
						caddr_t, size_t);
STATIC	void		ux_saveraddr(struct so_so *, struct bind_ux *);
STATIC	void		fill_udata_req_addr(mblk_t *, caddr_t, size_t);
STATIC	void		fill_udata_ind_addr(mblk_t *, caddr_t, size_t);
STATIC  mblk_t		*_s_makeopt(struct so_so *);
STATIC  void		_s_setopt(mblk_t *, struct so_so *);
STATIC	void		do_ERROR(queue_t *, mblk_t *);
STATIC	mblk_t		*_s_getloaned(queue_t *, mblk_t *, int);
STATIC	void		free_urg(char *);
STATIC	void		free_zero(char *);
STATIC	mblk_t		*_s_getband1(queue_t *, mblk_t *);
STATIC	int		do_urg_inline(queue_t *, mblk_t *);
STATIC	int		do_urg_outofline(queue_t *, mblk_t *);
STATIC	mblk_t		*do_esbzero(queue_t *, mblk_t *);
#else
static  int		tlitosyserr();
static  long		_t_setsize();
STATIC  int		so_options();
STATIC	mblk_t		*_s_getmblk();
STATIC	void		snd_ERRACK();
STATIC	void		snd_OKACK();
STATIC	int		so_init();
STATIC	void		strip_zerolen();
STATIC	void		save_addr();
STATIC	void		snd_ZERO();
STATIC	void		snd_FLUSHR();
STATIC	void		snd_ERRORW();
STATIC	void		snd_IOCNAK();
STATIC	void		snd_HANGUP();
STATIC	void		ux_dellink();
STATIC	void		ux_addlink();
STATIC	struct	so_so	*ux_findlink();
STATIC	void		ux_restoreaddr();
STATIC	void		ux_saveraddr();
STATIC	void		fill_udata_req_addr();
STATIC	void		fill_udata_ind_addr();
STATIC	mblk_t		*_s_makeopt();
STATIC	void		_s_setopt();
STATIC	void		do_ERROR();
STATIC	mblk_t		*_s_getloaned();
STATIC	void		free_urg();
STATIC	void		free_zero();
STATIC	mblk_t		*_s_getband1();
STATIC	int		do_urg_outofline();
STATIC	int		do_urg_inline();
STATIC	mblk_t		*do_esbzero();
#endif

int	socklog = 0;
#define	SOCKLOG(S, A, B)  \
			{ \
			if (((S)->udata.so_options & SO_DEBUG) || \
							socklog & 1) { \
				if (socklog & 2) \
					(void)cmn_err(CE_CONT, (A), (B)); \
				else	(void)strlog(SIMOD_ID, (S)-so_so, \
						0, SL_TRACE, (A), (B)); \
			} \
			}

/*
 * Standard STREAMS templates.
 */
int 	sockmodopen(),
	sockmodclose(),
	sockmodrput(),
	sockmodwput(),
	sockmodwsrv(),
	sockmodrsrv();

STATIC struct module_info sockmod_info = {
	SIMOD_ID,
	"sockmod",
	0,		/* Write side set in sockmodopen() */
	INFPSZ,		/* Write side set in sockmodopen() */
	512,		/* Always left small */
	128		/* Always left small */
};
STATIC struct qinit sockmodrinit = {
	sockmodrput,
	sockmodrsrv,
	sockmodopen,
	sockmodclose,
	nulldev,
	&sockmod_info,
	NULL
};
STATIC struct qinit sockmodwinit = {
	sockmodwput,
	sockmodwsrv,
	sockmodopen,
	sockmodclose,
	nulldev,
	&sockmod_info,
	NULL
};
struct	streamtab sockinfo = {
	&sockmodrinit,
	&sockmodwinit,
	NULL,
	NULL
};


/*
 * sockmodopen - open routine gets called when the
 *	module gets pushed onto the stream.
 */
/*ARGSUSED*/
sockmodopen(q, dev, flag, sflag, crp)
	register queue_t	*q;
	register dev_t		*dev;
	register int		flag;
	register int		sflag;
	register cred_t		*crp;
{
	register struct so_so		*so;
	register struct stroptions	*stropt;
	register mblk_t			*bp;

	ASSERT(q != (queue_t *)NULL);

	if (q->q_ptr)
		return (1);

	for (so = so_so; so < &so_so[so_cnt]; so++)
		if ((so->flags & USED) == 0)
			break;

	if (so >= &so_so[so_cnt])
		return (ENOBUFS);

	so->flags = USED;
	so->rdq = q;
	so->iocsave = (mblk_t *)NULL;
	so->consave = (mblk_t *)NULL;
	so->oob = (mblk_t *)NULL;
	so->so_error = 0;
	so->so_conn = (struct so_so *)NULL;
	so->so_option = 0;
	so->sndbuf = 0;
	so->rcvbuf = 0;
	so->sndlowat = 0;
	so->rcvlowat = 0;
	so->linger = 0;
	so->sndtimeo = 0;
	so->rcvtimeo = 0;
	so->prototype = 0;
	so->hasoutofband = 0;
	so->urg_msg = (mblk_t *)NULL;
	so->esbcnt = 0;
	bzero((caddr_t)&so->udata, sizeof (struct si_udata));

	bzero((caddr_t)&so->lux_dev, sizeof (struct ux_extaddr));
	bzero((caddr_t)&so->rux_dev, sizeof (struct ux_extaddr));

	so->so_ux.next = (struct so_so *)NULL;
	so->so_ux.prev = (struct so_so *)NULL;
	q->q_ptr = (caddr_t)so;
	WR(q)->q_ptr = (caddr_t)so;

SOCKLOG(so, "sockmodopen: Allocated so for q %x\n", q);

	/*
	 * Send down T_INFO_REQ and wait for a reply
	 */
	if ((bp = allocb(sizeof (struct T_info_req) +
		sizeof (struct T_info_ack), BPRI_LO)) == (mblk_t *)NULL) {
		so->flags = 0;
		return (ENOBUFS);
	}

	so->flags |= S_WINFO;
	bp->b_datap->db_type = M_PCPROTO;
	*(long *)bp->b_wptr = (long)T_INFO_REQ;
	bp->b_wptr += sizeof (struct T_info_req);
	putnext(WR(q), bp);

	while (so->flags & S_WINFO) {
		if (sleep((caddr_t)so, (int)(PZERO+1|PCATCH))) {
			/*
			 * Interrupted
			 */
			so->flags = 0;
			return (EINTR);
		}
	}
	if (so->so_error) {
		/*
		 * Some bad error occurred.
		 */
		so->flags = 0;
		return (so->so_error);
	}
	/*
	 * Reserve space for local and remote
	 * addresses.
	 */
	if ((so->laddr.buf = (char *)kmem_alloc(so->udata.addrsize,
						KM_SLEEP)) == (caddr_t)NULL) {
		so->flags = 0;
		return (ENOBUFS);
	}
	so->laddr.maxlen = so->udata.addrsize;
	so->laddr.len = 0;
	(void)bzero(so->laddr.buf, so->udata.addrsize);

	if ((so->raddr.buf = (char *)kmem_alloc(so->udata.addrsize,
						KM_SLEEP)) == (caddr_t)NULL) {
		so->flags = 0;
		kmem_free(so->laddr.buf, so->laddr.maxlen);
		return (ENOBUFS);
	}
	so->raddr.maxlen = so->udata.addrsize;
	so->raddr.len = 0;
	(void)bzero(so->raddr.buf, so->udata.addrsize);

	/*
	 * Set our write maximum and minimum packet sizes
	 * to that of the transport provider. If a
	 * transport provider is to be PR_ATOMIC then
	 * its q_minpsz should be set to 1 for write(2)
	 * to fail messages to large to be sent in a
	 * single crack.
	 */
	OTHERQ(q)->q_minpsz = (OTHERQ(q)->q_next)->q_minpsz;
	OTHERQ(q)->q_maxpsz = (OTHERQ(q)->q_next)->q_maxpsz;

SOCKLOG(so, "sockmodopen: WRq minpsz %x\n", OTHERQ(q)->q_minpsz);
SOCKLOG(so, "sockmodopen: WRq maxpsz %x\n", OTHERQ(q)->q_maxpsz);

	/*
	 * Set stream head option for M_READ messages.
	 */
	if ((bp = allocb(sizeof (*stropt), BPRI_MED)) == (mblk_t *)NULL) {
		so->flags = 0;
		return (ENOBUFS);
	}
	bp->b_datap->db_type = M_SETOPTS;
	stropt = (struct stroptions *)bp->b_rptr;
	stropt->so_flags = SO_MREADON | SO_READOPT;
	stropt->so_readopt = RPROTDIS;
	if (so->udata.servtype == T_CLTS)
		stropt->so_readopt |= RMSGD;
	bp->b_wptr += sizeof (*stropt);
	putnext(q, bp);

	return (0);
}


/*
 * sockmodclose - This routine gets called when the module
 *		gets popped off of the stream.
 */
/*ARGSUSED*/
sockmodclose(q, flag, credp)
	register queue_t	*q;
	register int 		flag;
	cred_t   		*credp;
{
	register struct so_so	*so;
	register mblk_t		*mp;
	register mblk_t		*nmp;

	ASSERT(q != NULL);
	so = (struct so_so *)q->q_ptr;
	ASSERT(so != NULL);

SOCKLOG(so, "sockmodclose: Entered\n", 0);

	/*
	 * Put any remaining messages downstream.
	 */
	while ((mp = getq(OTHERQ(q))) != NULL)
		putnext(OTHERQ(q), mp);

	freemsg(so->iocsave);

	mp = so->consave;
	while (mp) {
		nmp = mp->b_next;
		freemsg(mp);
		mp = nmp;
	}
	if (so->oob)
		freemsg(so->oob);
	if (so->urg_msg)
		freemsg(so->urg_msg);

	kmem_free(so->laddr.buf, so->laddr.maxlen);
	kmem_free(so->raddr.buf, so->raddr.maxlen);

	/*
	 * If this was a UNIX domain endpoint, then
	 * update the linked list.
	 */
#ifdef AF_UNIX
	ux_dellink(so);
#endif /* AF_UNIX */

	SOCKLOG(so, "sockmodclose: so->esbcnt %d\n", so->esbcnt);
	if (so->esbcnt)
		so->flags |= S_WCLOSE;
	else	so->flags = 0;

	return (0);
}


/*
 * sockmodrput - Module read queue put procedure.
 *		 This is called from the module or
 *		 driver downstream.
 *		 Handles data messages if it can and
 *		 queues the rest.
 */
sockmodrput(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register union T_primitives	*pptr;
	register struct so_so		*so;
	register int			s;

	ASSERT(q != NULL);
	so = (struct so_so *)q->q_ptr;
	ASSERT(so != NULL);

SOCKLOG(so, "sockmodrput: Entered - q %x\n", q);

	switch (mp->b_datap->db_type) {
	default:
		putq(q, mp);
		return (0);

	case M_FLUSH:

SOCKLOG(so, "sockmodrput: Got M_FLUSH\n", 0);

		/*
		 * This is a kludge until something better
		 * is done. If this is a AF_UNIX socket,
		 * ignore the M_FLUSH and don't propogate it.
		 */
		if (so->laddr.len > 0 &&
				((sockaddr_t)so->laddr.buf)->sa_family ==
						AF_UNIX) {

SOCKLOG(so, "sockmodrput: Ignoring M_FLUSH\n", 0);

			freemsg(mp);
			return (0);
		}

		if (*mp->b_rptr & FLUSHW)
			flushq(WR(q), FLUSHDATA);
		if (*mp->b_rptr & FLUSHR)
			flushq(q, FLUSHDATA);

		putnext(q, mp);
		return (0);

	case M_IOCACK: {
		register struct iocblk		*iocbp;

SOCKLOG(so, "sockmodrput: Got M_IOCACK\n", 0);

		iocbp = (struct iocblk *)mp->b_rptr;
		switch (iocbp->ioc_cmd) {
		default:
			putq(q, mp);
			return (0);

		case TI_GETPEERNAME:
		case TI_GETMYNAME:
			ASSERT(so->iocsave != NULL);
			freemsg(so->iocsave);

			if (iocbp->ioc_cmd == TI_GETPEERNAME &&
					so->udata.servtype == T_CLTS)
				so->udata.so_state |= SS_ISCONNECTED;

			so->iocsave = NULL;
			so->flags &= ~WAITIOCACK;
			putnext(q, mp);
			return (0);
		}
	}

	case M_IOCNAK: {
		register struct iocblk		*iocbp;

SOCKLOG(so, "sockmodrput: Got M_IOCNAK\n", 0);

		iocbp = (struct iocblk *)mp->b_rptr;
		switch (iocbp->ioc_cmd) {
		default:
			putq(q, mp);
			return (0);

		case TI_GETMYNAME:
			ASSERT(so->iocsave != NULL);
			freemsg(mp);
			mp = so->iocsave;
			so->iocsave = NULL;
			so->flags |= NAMEPROC;
			if (ti_doname(WR(q), mp, so->laddr.buf, so->laddr.len,
				so->raddr.buf, so->raddr.len) != DONAME_CONT) {
				so->flags &= ~NAMEPROC;
			}
			so->flags &= ~WAITIOCACK;
			return (0);

		case TI_GETPEERNAME:
			freemsg(mp);
			ASSERT(so->iocsave != NULL);
			mp = so->iocsave;
			so->iocsave = NULL;
			so->flags |= NAMEPROC;
			if (ti_doname(WR(q), mp, so->laddr.buf, so->laddr.len,
				so->raddr.buf, so->raddr.len) != DONAME_CONT) {
				so->flags &= ~NAMEPROC;
			}
			so->flags &= ~WAITIOCACK;
			if (so->udata.servtype == T_CLTS)
				so->udata.so_state |= SS_ISCONNECTED;
			return (0);
		}
	}

	case M_DATA:

SOCKLOG(so, "sockmodrput: Got M_DATA q %x\n", q);

		/*
		 * If the socket is marked such that we don't
		 * want to get anymore data then free it.
		 */
		if (so->udata.so_state & SS_CANTRCVMORE) {
			freemsg(mp);
			return (0);
		}

		strip_zerolen(mp);

		s = splstr();
		if (so->flags & S_RBLOCKED || !canput(q->q_next)) {
			if ((so->flags & S_RBLOCKED) == 0)
				so->flags |= S_RBLOCKED;
			putq(q, mp);
			(void)splx(s);
		} else	{
			(void)splx(s);
			if (so->urg_msg) {
				putnext(q, so->urg_msg);
				so->urg_msg = (mblk_t *)NULL;
			}
			putnext(q, mp);
		}
		return (0);

	case M_PROTO:
	case M_PCPROTO:
		/*
		 * Assert checks if there is enough data to determine type
		 */
		ASSERT(MSGBLEN(mp) >= sizeof (long));

		pptr = (union T_primitives *)mp->b_rptr;

		switch (pptr->type) {
		default:
			putq(q, mp);
			return (0);

		case T_UDERROR_IND:
			/*
			 * Just set so_error.
			 */
			so->so_error = pptr->uderror_ind.ERROR_type;

SOCKLOG(so, "sockmodrput: Got T_UDERROR_IND error %d\n", so->so_error);

			freemsg(mp);
			return (0);

		case T_UNITDATA_IND: {
			register struct T_unitdata_ind	*udata_ind;

SOCKLOG(so, "sockmodrput: Got T_UNITDATA_IND\n", 0);

			/*
			 * If we are connected, then we must ensure the
			 * source address is the one we connected to.
			 */
			udata_ind = (struct T_unitdata_ind *)mp->b_rptr;
			if (so->udata.so_state & SS_ISCONNECTED) {
				if (((sockaddr_t)so->raddr.buf)->sa_family
								== AF_UNIX) {
#ifdef AF_UNIX
					register char	*addr;

					addr = (char *)(mp->b_rptr +
						udata_ind->SRC_offset);
					if (bcmp(addr,
						(caddr_t)&so->rux_dev.addr,
						so->rux_dev.size) != 0) {
						/*
						 * Log error and free the msg.
						 */
						so->so_error = EINVAL;
						freemsg(mp);
						return (0);
					}
#endif /* AF_UNIX */
				} else	{
					if (bcmp(so->raddr.buf,
				(caddr_t)(mp->b_rptr+udata_ind->SRC_offset),
						so->raddr.len) != 0) {
						/*
						 * Log error and free the msg.
						 */
						so->so_error = EINVAL;
						freemsg(mp);
						return (0);
					}
				}
			}

			/*
			 * If flow control is blocking us then
			 * let the service procedure handle it.
			 */
			s = splstr();
			if (!canput(q->q_next)) {
				putq(q, mp);
				(void)splx(s);
				return (0);
			}
			(void)splx(s);

			if (((sockaddr_t)so->raddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				register struct so_so	*oso;
				register size_t		size;
				register char 		*addr;

				addr = (caddr_t)(mp->b_rptr +
						udata_ind->SRC_offset);
				if ((oso = ux_findlink(addr,
					(size_t)udata_ind->SRC_length)) ==
							NULL) {
					freemsg(mp);
					return (0);
				}

				size = sizeof (*udata_ind) + oso->laddr.len;
				if (MBLKLEN(mp) < size) {
					register mblk_t		*bp;

					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						putq(q, mp);
						return (0);
					}
					*(struct T_unitdata_ind *)bp->b_wptr =
							*udata_ind;

					bp->b_cont = mp->b_cont;
					mp->b_cont = NULL;
					freemsg(mp);
					mp = bp;
				}
				fill_udata_ind_addr(mp, oso->laddr.buf,
							oso->laddr.len);
#endif /* AF_UNIX */
			}
			/*
			 * Check for zero length message and ensure that
			 * we always leave one linked to the header if
			 * there is no "real" data.
			 * This facilitates sending zero length messages
			 * on dgram sockets.
			 */
			if (mp->b_cont && msgdsize(mp->b_cont) == 0) {
				register mblk_t		*bp;

				/*
				 * Zero length message.
				 */

SOCKLOG(so, "sockmodrput: Got zero length msg\n", 0);

				bp = mp->b_cont;
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			} else	strip_zerolen(mp);

			putnext(q, mp);
			return (0);
		}

		case T_EXDATA_IND:

SOCKLOG(so, "sockmodrput: Got T_EXDATA_IND\n", 0);

			/*
			 * We want to queue this, but to maintain
			 * its position in the data stream we have
			 * to make sure any succeeding data is sent
			 * upstream AFTER the expedited data.
			 */
			s = splstr();
			so->flags |= S_RBLOCKED;
			(void)splx(s);
			putq(q, mp);

			return (0);

		case T_DATA_IND:

SOCKLOG(so, "sockmodrput: Got T_DATA_IND q %x\n", q);

			/*
			 * If the socket is marked such that we don't
			 * want to get anymore data then free it.
			 */
			if (so->udata.so_state & SS_CANTRCVMORE) {
				freemsg(mp);
				return (0);
			}

			strip_zerolen(mp);

			s = splstr();
			if (so->flags & S_RBLOCKED || !canput(q->q_next)) {
				if ((so->flags & S_RBLOCKED) == 0)
					so->flags |= S_RBLOCKED;
				putq(q, mp);
				(void)splx(s);
			} else	{
				(void)splx(s);

				if (so->urg_msg) {
					putnext(q, so->urg_msg);
					so->urg_msg = (mblk_t *)NULL;
				}
				putnext(q, mp);
			}
			return (0);
		}
	}
}

/*
 * sockmodrsrv - Module read queue service procedure.
 *		 Handles everything the write put
 *		 procedure dosen't want to.
 */
sockmodrsrv(q)
	register queue_t	*q;
{
	register union T_primitives	*pptr;
	register struct so_so		*so;
	register struct iocblk		*iocbp;
	register mblk_t			*bp;
	register mblk_t			*mp;
	register int			s;
	register size_t			size;

	ASSERT(q != NULL);
	so = (struct so_so *)q->q_ptr;
	ASSERT(so != NULL);

rgetnext:
	if ((mp = getq(q)) == (mblk_t *)NULL) {
		if (so->flags & S_RBLOCKED)
			so->flags &= ~S_RBLOCKED;
		return (0);
	}

	switch (mp->b_datap->db_type) {
	default:
		putnext(q, mp);
		goto rgetnext;

	case M_DATA:

SOCKLOG(so, "sockmodrsrv: Got M_DATA q %x\n", q);

		s = splstr();
		if (canput(q->q_next)) {
			(void)splx(s);
			if (so->urg_msg) {
				putnext(q, so->urg_msg);
				so->urg_msg = (mblk_t *)NULL;
			}
			putnext(q, mp);
		} else	{
			if ((so->flags & S_RBLOCKED) == 0)
				so->flags |= S_RBLOCKED;
			putbq(q, mp);
			(void)splx(s);
			return (0);
		}

		goto rgetnext;

	case M_PROTO:
	case M_PCPROTO:
		/*
		 * Assert checks if there is enough data to determine type
		 */
		ASSERT(MSGBLEN(mp) >= sizeof (long));

		pptr = (union T_primitives *)mp->b_rptr;

		switch (pptr->type) {
		default:
			putnext(q, mp);
			goto rgetnext;

		case T_DISCON_IND: {
			queue_t 	*qp;

			/*
			 * This is a hack to ensure that in UNIX domain,
			 * we allow reads to return 0 bytes rather than
			 * error. This is the easiest way to do it.
			 */
			if (((sockaddr_t)so->laddr.buf)->sa_family != AF_UNIX)
				so->so_error = pptr->discon_ind.DISCON_reason;

SOCKLOG(so, "sockmodrsrv: Got T_DISCON_IND Reason: %d\n", so->so_error);

			/*
			 * If this is in response to a connect,
			 * and the caller is waiting, then send the
			 * disconnect up.
			 */
			if (so->udata.so_state & SS_ISCONNECTING) {
				if ((so->flags & S_WRDISABLE) == 0) {
					/*
					 * Close down the write side and
					 * begin to close down the read side.
					 */
					snd_ERRORW(q);
					if ((bp = do_esbzero(q, mp)) ==
							(mblk_t *)NULL)
						return (0);
					bp->b_wptr = bp->b_rptr;
					linkb(mp, bp);

					/*
					 * Send the disconnect up, so that
					 * the reason can be extracted. An
					 * M_ERROR will be generated when
					 * the message is read.
					 */
					putnext(q, mp);

					mp = (mblk_t *)NULL;

				} else	{
					/*
					 * Enable the write service queue to
					 * be scheduled, and schedule it.
					 */
					enableok(WR(q));
					qenable(WR(q));
				}
			}

			so->udata.so_state |= (SS_CANTRCVMORE|SS_CANTSENDMORE);
			so->udata.so_state &= ~(SS_ISCONNECTED|SS_ISCONNECTING);

			if (mp == (mblk_t *)NULL)
				goto rgetnext;

			/*
			 * We would like to just send up an M_ERROR,
			 * but if there are regular messages waiting
			 * to be read at the stream head they would
			 * be lost. The solution is to see what is
			 * there and either send up an M_ERROR if
			 * there are no regular messages, otherwise
			 * tag on a zero length loaned mblk, whose free
			 * routine will send up the M_ERROR.
			 */
			s = splstr();
			for (qp = q->q_next; qp->q_next; qp = qp->q_next)
				;
			if (qp->q_last && qp->q_last->b_band == 0) {
				/*
				 * Close down the write side and
				 * begin to close down the read side.
				 */
				snd_ERRORW(q);
				if ((bp = do_esbzero(q, mp)) ==
							(mblk_t *)NULL) {
					(void)splx(s);
					return (0);
				} else	{
					freemsg(mp);

					bp->b_wptr = bp->b_rptr;
					linkb(qp->q_last, bp);
					(void)splx(s);
				}
			} else	{
				splx(s);
				do_ERROR(q, mp);
			}

			goto rgetnext;
		}

		case T_ORDREL_IND:

SOCKLOG(so, "sockmodrsrv: Got T_ORDREL_IND\n", 0);

			so->udata.so_state |= SS_CANTRCVMORE;

			/*
			 * Send up zero length message(EOF) to
			 * wakeup anyone in a read(), or select().
			 */
			freemsg(mp);
			snd_ZERO(q);

			goto rgetnext;

		case T_CONN_IND: {
			register mblk_t			*nbp;

SOCKLOG(so, "sockmodrsrv: Got T_CONN_IND\n", 0);

			/*
			 * Make sure we can dup the new
			 * message before proceeding.
			 */
			if ((nbp = dupmsg(mp)) == (mblk_t *)NULL) {
				RECOVER(q, mp, sizeof (mblk_t));
				return (0);
			}

			if (((sockaddr_t)so->laddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				register struct T_conn_ind	*conn_ind;
				register struct T_conn_ind	*nconn_ind;
				register char			*addr;
				register struct so_so		*oso;

				/*
				 * To make sure the user sees a string rather
				 * than a dev/ino pair, we have to find the
				 * source socket structure and copy in the
				 * local (string) address.
				 */
				conn_ind = (struct T_conn_ind *)mp->b_rptr;
				addr = (caddr_t)(mp->b_rptr +
						conn_ind->SRC_offset);
				if ((oso = ux_findlink(addr,
					(size_t)conn_ind->SRC_length)) ==
								NULL) {
					freemsg(mp);
					goto rgetnext;
				}

				size = sizeof (*nconn_ind) + oso->laddr.len;
				if (MBLKLEN(mp) < size) {
					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						freemsg(nbp);
						return (0);
					}

					bp->b_datap->db_type = M_PROTO;

					nconn_ind =
						(struct T_conn_ind *)bp->b_rptr;
					*nconn_ind = *conn_ind;
					freemsg(mp);
					mp = bp;
				}
				conn_ind = (struct T_conn_ind *)mp->b_rptr;
				(void)bcopy(oso->laddr.buf,
					(caddr_t)(mp->b_rptr +
						conn_ind->SRC_offset),
					oso->laddr.len);

				conn_ind->SRC_length = oso->laddr.len;
				mp->b_wptr = mp->b_rptr + size;
#endif /* AF_UNIX */
			}

			/*
			 * Save out dup'ed copy.
			 */
			nbp->b_next = so->consave;
			so->consave = nbp;

			putnext(q, mp);
			goto rgetnext;
		}	/* case T_CONN_IND */

		case T_CONN_CON: {
			register struct T_conn_con	*conn_con;

SOCKLOG(so, "sockmodrsrv: Got T_CONN_CON\n", 0);

			/*
			 * Pass this up only if the user is waiting-
			 * tell the write service procedure to
			 * go for it.
			 */
			so->udata.so_state &= ~SS_ISCONNECTING;
			so->udata.so_state |= SS_ISCONNECTED;

			if (so->flags & S_WRDISABLE) {
				freemsg(mp);

				/*
				 * Enable the write service queue to
				 * be scheduled, and schedule it.
				 */
				enableok(WR(q));
				qenable(WR(q));
				goto rgetnext;
			}

			conn_con = (struct T_conn_con *)mp->b_rptr;
			if (((sockaddr_t)so->raddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				register char		*addr;

				/*
				 * We saved the destination address, when
				 * the T_CONN_REQ was processed, so put
				 * it back.
				 */
				addr = (caddr_t)(mp->b_rptr +
						conn_con->RES_offset);
				size = sizeof (*conn_con) + so->raddr.len;
				if (MBLKLEN(mp) < size) {
					register struct T_conn_con *nconn_con;
					register mblk_t		   *bp;

					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}

					bp->b_datap->db_type = M_PROTO;

					nconn_con =
						(struct T_conn_con *)bp->b_rptr;
					*nconn_con = *conn_con;
					freemsg(mp);
					mp = bp;
				}
				conn_con = (struct T_conn_con *)mp->b_rptr;
				addr = (caddr_t)(mp->b_rptr +
						conn_con->RES_offset);
				(void)bcopy(so->raddr.buf,
						addr, so->raddr.len);

				conn_con->RES_length = so->raddr.len;

				mp->b_wptr = mp->b_rptr + size;
#endif /* AF_UNIX */
			} else	save_addr(&so->raddr,
				(caddr_t)(mp->b_rptr+conn_con->RES_offset),
						(size_t)conn_con->RES_length);

			putnext(q, mp);
			goto rgetnext;
		}

		case T_UNITDATA_IND: {
			register struct T_unitdata_ind	*udata_ind;

SOCKLOG(so, "sockmodrsrv: Got T_UNITDATA_IND\n", 0);

			s = splstr();
			if (!canput(q->q_next)) {
				putbq(q, mp);
				(void)splx(s);
				return (0);
			}
			(void)splx(s);

			udata_ind = (struct T_unitdata_ind *)mp->b_rptr;
			if (((sockaddr_t)so->laddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				/*
				 * UNIX domain, copy useful address.
				 */
				register char 			*addr;
				register struct so_so		*oso;

				addr = (caddr_t)(mp->b_rptr +
						udata_ind->SRC_offset);

				if ((oso = ux_findlink(addr,
					(size_t)udata_ind->SRC_length)) ==
								NULL) {
					freemsg(mp);
					goto rgetnext;
				}

				size = sizeof (*udata_ind) + oso->laddr.len;
				if (MBLKLEN(mp) < size) {
					register mblk_t		*bp;

					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}
					*(struct T_unitdata_ind *)bp->b_wptr =
							*udata_ind;

					bp->b_cont = mp->b_cont;
					mp->b_cont = NULL;
					freemsg(mp);
					mp = bp;
				}
				fill_udata_ind_addr(mp, oso->laddr.buf,
							oso->laddr.len);
#endif /* AF_UNIX */
			}
			/*
			 * Check for zero length message and ensure that
			 * we always leave one linked to the header if
			 * there is no "real" data.
			 * This facilitates sending zero length messages
			 * on dgram sockets.
			 */
			if (mp->b_cont && msgdsize(mp->b_cont) == 0) {
				register mblk_t		*bp;

				/*
				 * Zero length message.
				 */

SOCKLOG(so, "sockmodrput: Got zero length msg\n", 0);

				bp = mp->b_cont;
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			} else	strip_zerolen(mp);

			putnext(q, mp);
			goto rgetnext;
		}

		case T_DATA_IND:
			s = splstr();
			if (canput(q->q_next)) {
				(void)splx(s);
				if (so->urg_msg) {
					putnext(q, so->urg_msg);
					so->urg_msg = (mblk_t *)NULL;
				}
				putnext(q, mp);
			} else	{
				if ((so->flags & S_RBLOCKED) == 0)
					so->flags |= S_RBLOCKED;
				putbq(q, mp);
				(void)splx(s);
				return (0);
			}

			goto rgetnext;

		case T_EXDATA_IND:

SOCKLOG(so, "sockmodrsrv: Got T_EXDATA_IND\n", 0);

			/*
			 * If the socket is marked such that we don't
			 * want to get anymore data then free it.
			 */
			if (so->udata.so_state & SS_CANTRCVMORE) {
				freemsg(mp);
				goto rgetnext;
			}

			s = splstr();
			if (!canput(q->q_next)) {
				putbq(q, mp);
				(void)splx(s);
				return (0);
			}
			(void)splx(s);

			if ((so->udata.so_options & SO_OOBINLINE) == 0) {
				if (do_urg_outofline(q, mp) < 0)
					return (0);
			} else	if (do_urg_inline(q, mp) < 0)
					return (0);
			goto rgetnext;

		case T_ERROR_ACK:
			ASSERT(MSGBLEN(mp) == sizeof (struct T_error_ack));

SOCKLOG(so, "sockmodrsrv: Got T_ERROR_ACK\n", 0);

			if (pptr->error_ack.ERROR_prim == T_CONN_RES) {
				if (so->so_conn) {
					so->so_conn->udata.so_state
							&= ~SS_ISCONNECTED;
					so->so_conn = 0;
				}
			}

			if (pptr->error_ack.ERROR_prim == T_UNBIND_REQ &&
							so->flags & S_WUNBIND) {
				if (pptr->error_ack.TLI_error == TSYSERR)
					so->so_error =
						pptr->error_ack.UNIX_error;
				else	so->so_error =
				tlitosyserr((int)pptr->error_ack.TLI_error);

				/*
				 * The error is a result of
				 * our internal unbind request.
				 */
				so->flags &= ~S_WUNBIND;
				freemsg(mp);

				snd_IOCNAK(q, so->iocsave, so->so_error);
				so->iocsave = (mblk_t *)NULL;

				goto rgetnext;
			}

			if (pptr->error_ack.ERROR_prim == T_INFO_REQ &&
							so->flags & S_WINFO) {
				if (pptr->error_ack.TLI_error == TSYSERR)
					so->so_error =
						pptr->error_ack.UNIX_error;
				else	so->so_error =
				tlitosyserr((int)pptr->error_ack.TLI_error);

				so->flags &= ~S_WINFO;
				wakeup(so);
				freemsg(mp);
				goto rgetnext;
			}

			if (pptr->error_ack.ERROR_prim == T_DISCON_REQ) {
				freemsg(mp);
				goto rgetnext;
			}

			if ((so->flags & WAITIOCACK) == 0) {
				putnext(q, mp);
				goto rgetnext;
			}

			ASSERT(so->iocsave != NULL);
			if (pptr->error_ack.ERROR_prim !=
					*(long *)so->iocsave->b_cont->b_rptr) {
				putnext(q, mp);
				goto rgetnext;
			}

			if (pptr->error_ack.ERROR_prim == T_BIND_REQ) {
				if (so->udata.so_options & SO_ACCEPTCONN)
					so->udata.so_options &= ~SO_ACCEPTCONN;
			} else
			if (pptr->error_ack.ERROR_prim == T_OPTMGMT_REQ) {

SOCKLOG(so, "sockmodrsrv: T_ERROR_ACK-optmgmt- %x\n", so->so_option);

				if (pptr->error_ack.TLI_error == TBADOPT &&
							so->so_option) {
					/*
					 * Must have been a T_NEGOTIATE
					 * for a socket option. Make it
					 * all work.
					 */
					freemsg(mp);
					mp = _s_makeopt(so);
					so->so_option = 0;
					goto out;
				} else	so->so_option = 0;
			}

			switch (pptr->error_ack.ERROR_prim) {
			case T_OPTMGMT_REQ:
			case T_BIND_REQ:
			case T_UNBIND_REQ:
			case T_INFO_REQ:
				/*
				 * Get saved ioctl msg and set values
				 */
				iocbp = (struct iocblk *)so->iocsave->b_rptr;
				iocbp->ioc_error = 0;
				iocbp->ioc_rval = pptr->error_ack.TLI_error;
				if (iocbp->ioc_rval == TSYSERR) {
					iocbp->ioc_rval |=
						pptr->error_ack.UNIX_error << 8;
					so->so_error =
						pptr->error_ack.UNIX_error;
				} else	so->so_error =
						tlitosyserr(iocbp->ioc_rval);

				so->iocsave->b_datap->db_type = M_IOCACK;
				putnext(q, so->iocsave);

				so->iocsave = NULL;
				so->flags &= ~WAITIOCACK;
				freemsg(mp);
				goto rgetnext;
			}	/* switch (pptr->error_ack.ERROR_prim) */

		case T_OK_ACK:

SOCKLOG(so, "sockmodrsrv: Got T_OK_ACK\n", 0);

			if (pptr->ok_ack.CORRECT_prim == T_CONN_RES) {
				register struct so_so	*oso;

				oso = so->so_conn;
				oso->udata.so_state |= SS_ISCONNECTED;
				so->so_conn = NULL;
			}
			if (pptr->ok_ack.CORRECT_prim == T_UNBIND_REQ) {
#ifdef AF_UNIX
				if (((sockaddr_t)so->laddr.buf)->sa_family
						== AF_UNIX)
					ux_dellink(so);
#endif AF_UNIX
				if (so->flags & S_WUNBIND) {
					so->flags &= ~S_WUNBIND;
					freemsg(mp);

					/*
					 * Put the saved TI_BIND request
					 * onto my write queue.
					 */

SOCKLOG(so, "sockmodrsrv: Putting saved TI_BIND request\n", 0);

					putq(WR(q), so->iocsave);
					so->iocsave = (mblk_t *)NULL;
					goto rgetnext;
				}
			}

			if (pptr->ok_ack.CORRECT_prim == T_DISCON_REQ) {
				/*
				 * Don't send it up.
				 */
				freemsg(mp);
				goto rgetnext;
			}

			if (so->flags & WAITIOCACK) {
				ASSERT(so->iocsave != NULL);
				if (pptr->ok_ack.CORRECT_prim !=
					*(long *)so->iocsave->b_cont->b_rptr) {
					putnext(q, mp);
					goto rgetnext;
				}
				goto out;
			}
			putnext(q, mp);
			goto rgetnext;

		case T_BIND_ACK: {
			register struct T_bind_ack	*bind_ack;

SOCKLOG(so, "sockmodrsrv: Got T_BIND_ACK\n", 0);

			if ((so->flags & WAITIOCACK) == 0) {
				putnext(q, mp);
				goto rgetnext;
			}

			ASSERT(so->iocsave != NULL);
			if (*(long *)so->iocsave->b_cont->b_rptr !=
							T_BIND_REQ) {
				putnext(q, mp);
				goto rgetnext;
			}

			bind_ack = (struct T_bind_ack *)mp->b_rptr;
#ifdef AF_UNIX
			if (so->laddr.len &&
				((sockaddr_t)so->laddr.buf)->sa_family ==
					AF_UNIX) {
				register char			*addr;

				addr = (caddr_t)(mp->b_rptr +
						bind_ack->ADDR_offset);

				/*
				 * If we don't have a copy of the actual
				 * address bound to then save one.
				 */
				if (so->lux_dev.size == 0) {
					(void)bcopy(addr,
					(caddr_t)&so->lux_dev.addr,
						(size_t)bind_ack->ADDR_length);
					so->lux_dev.size =
							bind_ack->ADDR_length;
				}

				/*
				 * UNIX domain, we have to put back the
				 * string part of the address as well as
				 * the actual address bound to.
				 */
				size = sizeof (struct bind_ux) +
						sizeof (*bind_ack);
				if (MBLKLEN(mp) < size) {
					register struct T_bind_ack *nbind_ack;
					register mblk_t		*bp;

					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}
					bp->b_datap->db_type = M_PROTO;

					nbind_ack =
						(struct T_bind_ack *)bp->b_wptr;
					*nbind_ack = *bind_ack;

					ux_restoreaddr(so, bp, addr,
						(size_t)bind_ack->ADDR_length);
					bp->b_wptr = bp->b_rptr + size;

					freemsg(mp);
					mp = bp;
				} else	{
					ux_restoreaddr(so, mp, addr,
						(size_t)bind_ack->ADDR_length);
					mp->b_wptr = mp->b_rptr + size;
				}
			}
#endif /* AF_UNIX */
			else	{
				/*
				 * Remember the bound address.
				 */
				save_addr(&so->laddr,
				(caddr_t)(mp->b_rptr+bind_ack->ADDR_offset),
					(size_t)bind_ack->ADDR_length);
			}

			so->udata.so_state |= SS_ISBOUND;

			goto out;
		}

		case T_OPTMGMT_ACK:

SOCKLOG(so, "sockmodrsrv: Got T_OPTMGMT_ACK\n", 0);

			if (so->flags & WAITIOCACK) {
				ASSERT(so->iocsave != NULL);
				if (*(long *)so->iocsave->b_cont->b_rptr !=
								T_OPTMGMT_REQ) {
					putnext(q, mp);
					goto rgetnext;
				}
				if (so->so_option) {

SOCKLOG(so, "sockmodrsrv: T_OPTMGMT_ACK option %x\n", so->so_option);

					/*
					 * Check that the value negotiated is
					 * the one that we stored.
					 */
					_s_setopt(mp, so);
					so->so_option = 0;
				}
				goto out;
			}
			putnext(q, mp);
			goto rgetnext;

		case T_INFO_ACK:

SOCKLOG(so, "sockmodrsrv: Got T_INFO_ACK\n", 0);

			if (so->flags & S_WINFO) {
				so->so_error = so_init(so,
					(struct T_info_ack *)pptr);

				so->flags &= ~S_WINFO;
				wakeup(so);
				freemsg(mp);
				goto rgetnext;
			} else	{
				/*
				 * The library never issues a
				 * direct request for transport
				 * information.
				 */
				putnext(q, mp);
				goto rgetnext;
			}

out:
			iocbp = (struct iocblk *)so->iocsave->b_rptr;
			ASSERT(so->iocsave->b_datap != NULL);
			so->iocsave->b_datap->db_type = M_IOCACK;
			mp->b_datap->db_type = M_DATA;
			freemsg(so->iocsave->b_cont);
			so->iocsave->b_cont = mp;
			iocbp->ioc_error = 0;
			iocbp->ioc_rval = 0;
			iocbp->ioc_count = MSGBLEN(mp);

			putnext(q, so->iocsave);

			so->iocsave = NULL;
			so->flags &= ~WAITIOCACK;
			goto rgetnext;
		}	/* switch (pptr->type) */
	}	/* switch (mp->b_datap->db_type) */
}

/*
 * sockmodwput - Module write queue put procedure.
 *		 Called from the module or driver
 *		 upstream.
 *		 Handles messages that must be passed
 *		 through with minimum delay and queues
 *		 the rest for the service procedure to
 *		 handle.
 */
sockmodwput(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register struct so_so		*so;
	register union T_primitives	*pptr;
	register mblk_t			*bp;
	register size_t			size;
	register int			s;

	ASSERT(q != NULL);
	so = (struct so_so *)q->q_ptr;
	ASSERT(so != NULL);


wputagain:
	switch (mp->b_datap->db_type) {
	default:
		putq(q, mp);
		return (0);

	case M_IOCTL: {
		register struct	iocblk 	*iocbp;

		iocbp = (struct iocblk *)mp->b_rptr;
		switch (iocbp->ioc_cmd) {
		default:
			putq(q, mp);
			return (0);

		case SI_GETINTRANSIT: {
			register int		error;
			register queue_t	*qp;

SOCKLOG(so, "sockmodwput: Got SI_GETINTRANSIT q %x\n", q);

			/*
			 * Return the peer's in-transit count.
			 */
			error = 0;
			if (so->udata.servtype != T_COTS_ORD &&
					so->udata.servtype != T_COTS)
				error = EINVAL;
			else
			if ((so->udata.so_state & SS_ISCONNECTED) == 0)
				error = ENOTCONN;
			else
			if (MSGBLEN(mp->b_cont) < sizeof (ulong))
				error = EINVAL;
			else
			if (((sockaddr_t)so->laddr.buf)->sa_family != AF_UNIX)
				error = EINVAL;

			if (error) {
				snd_IOCNAK(RD(q), mp, error);
				return (0);
			}

			/*
			 * Find our stream head and then
			 * follow it all the way to the
			 * the other end looking for any
			 * message queue that is not empty.
			 */
			s = splstr();
			qp = RD(q);
			for (qp = qp->q_next; qp->q_next; qp = qp->q_next)
				;

			qp = WR(qp);
			while (qp->q_first == (mblk_t *)NULL) {
				if (qp->q_next != (queue_t *)NULL)
					qp = qp->q_next;
				else	break;
			}

			if (qp->q_first)
				error = 1;
			(void)splx(s);

SOCKLOG(so, "sockmodwput: SI_GETINTRANSIT: error = %d\n", error);

			*(int *)mp->b_cont->b_rptr = error;
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			return (0);
		}
		}
	}

	case M_FLUSH:

SOCKLOG(so, "sockmodwput: Got M_FLUSH\n", 0);

		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (*mp->b_rptr & FLUSHR)
			flushq(RD(q), FLUSHDATA);

		putnext(q, mp);
		return (0);

	case M_IOCDATA:

SOCKLOG(so, "sockmodwput: Got M_IOCDATA\n", 0);

		if (so->flags & NAMEPROC) {
			if (ti_doname(q, mp, so->laddr.buf, so->laddr.len,
				so->raddr.buf, so->raddr.len) != DONAME_CONT)
				so->flags &= ~NAMEPROC;
			return (0);
		}
		putnext(q, mp);
		return (0);

	case M_DATA:

SOCKLOG(so, "sockmodwput: Got M_DATA state %x\n", so->udata.so_state);

		if ((so->udata.so_state & SS_ISCONNECTED) == 0) {
			/*
			 * Set so_error, and free the message.
			 */
			so->so_error = ENOTCONN;
			freemsg(mp);
			return (0);
		}
		/*
		 * Pre-pend the M_PROTO header.
		 */
		if (so->udata.servtype == T_CLTS) {
			if (((sockaddr_t)so->raddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				size = sizeof (struct T_unitdata_req) +
							so->rux_dev.size;
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					putq(q, mp);
					return (0);
				}

SOCKLOG(so, "sockmodwput: M_DATA: size %x\n", so->rux_dev.size);

				fill_udata_req_addr(bp,
						(caddr_t)&so->rux_dev.addr,
						so->rux_dev.size);
#endif /* AF_UNIX */
			} else	{
				/*
				 * Not UNIX domain.
				 */
				size = sizeof (struct T_unitdata_req) +
							so->raddr.len;
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					putq(q, mp);
					return (0);
				}
				fill_udata_req_addr(bp, so->raddr.buf,
							so->raddr.len);
			}
			linkb(bp, mp);
			mp = bp;
		}

		s = splstr();
		if (so->flags & S_WBLOCKED || !canput(q->q_next)) {

SOCKLOG(so, "sockmodwput: canput returned false\n", 0);

			if ((so->flags & S_WBLOCKED) == 0)
				so->flags |= S_WBLOCKED;
			putq(q, mp);
			(void)splx(s);
		} else	{

SOCKLOG(so, "sockmodwput: canput returned true\n", 0);

			(void)splx(s);
			putnext(q, mp);
		}
		return (0);

	case M_PROTO:
		/*
		 * Assert checks if there is enough data to determine type
		 */
		ASSERT(MSGBLEN(mp) >= sizeof (long));

		pptr = (union T_primitives *)mp->b_rptr;

		switch (pptr->type) {
		default:
			putq(q, mp);
			return (0);

		case T_DATA_REQ:

SOCKLOG(so, "sockmodwput: Got T_DATA_REQ\n", 0);

			if ((so->udata.so_state & SS_ISCONNECTED) == 0) {
				/*
				 * Set so_error and free the message.
				 */
				so->so_error = ENOTCONN;
				freemsg(mp);
				return (0);
			}

			s = splstr();
			if (so->flags & S_WBLOCKED || !canput(q->q_next)) {
				if ((so->flags & S_WBLOCKED) == 0)
					so->flags |= S_WBLOCKED;
				putq(q, mp);
				(void)splx(s);
			} else	{
				(void)splx(s);

SOCKLOG(so, "sockmodwput: T_DATA_REQ len %d\n", MSGBLEN(mp->b_cont));

				putnext(q, mp);
			}
			return (0);

		case T_UNITDATA_REQ: {
			register struct T_unitdata_req	*udata_req;

SOCKLOG(so, "sockmodwput: Got T_UNITDATA_REQ state %x\n", so->udata.so_state);

			/*
			 * If no destination address then make it look
			 * like a plain M_DATA and try again.
			 */
			udata_req = (struct T_unitdata_req *)mp->b_rptr;
			if (MSGBLEN(mp) < sizeof (*udata_req)) {

SOCKLOG(so, "sockmodwput: Bad unitdata header %d\n", MSGBLEN(mp));

				freemsg(mp);
				return (0);
			}
			if (udata_req->DEST_length == 0) {
				if (mp->b_cont == (mblk_t *)NULL) {
					/*
					 * Zero length message.
					 */
					mp->b_datap->db_type = M_DATA;
					mp->b_wptr = mp->b_rptr;
				} else	{
					bp = mp->b_cont;
					mp->b_cont = NULL;
					freemsg(mp);
					mp = bp;
				}
				goto wputagain;
			}

			s = splstr();
			if (so->flags & S_WBLOCKED || !canput(q->q_next)) {
				if ((so->flags & S_WBLOCKED) == 0)
					so->flags |= S_WBLOCKED;
				putq(q, mp);
				(void)splx(s);
			} else	{
				(void)splx(s);
				putnext(q, mp);
			}

			return (0);
		}	/* case T_UNITDATA_REQ: */
		}	/* case pptr_type */
	}		/* case db_type */
}

/*
 * sockmodwsrv - Module write queue service procedure.
 *		 Handles messages that the put procedure
 *		 couldn't or didn't want to handle.
 */
sockmodwsrv(q)
	register queue_t	*q;
{
	register struct so_so		*so;
	struct iocblk			*iocbp;
	register mblk_t			*mp;
	register mblk_t			*bp;
	register union T_primitives	*pptr;
	register int			s;
	register size_t			size;

	ASSERT(q != NULL);
	so = (struct so_so *)q->q_ptr;
	ASSERT(so != NULL);

wgetnext:
	if ((mp = getq(q)) == (mblk_t *)NULL) {
		/*
		 * If we have been blocking downstream writes
		 * in the put procedure, then re-enable them.
		 */
		if (so->flags & S_WBLOCKED)
			so->flags &= ~S_WBLOCKED;
		return (0);
	}

	/*
	 * If we have been disabled, and the message
	 * we have is the message which caused select
	 * to work then just free it.
	 */
	if (so->flags & S_WRDISABLE) {
		if (mp == so->bigmsg) {

SOCKLOG(so, "sockmodwsrv: Taking big message off write queue\n", 0);

			freemsg(mp);
			so->flags &= ~S_WRDISABLE;
			so->bigmsg = NULL;
			goto wgetnext;
		}
	}

again:
	switch (mp->b_datap->db_type) {
	default:
		putnext(q, mp);
		goto wgetnext;

	case M_IOCDATA:

SOCKLOG(so, "sockmodwsrv: Got M_IOCDATA\n", 0);

		if (so->flags & NAMEPROC) {
			if (ti_doname(q, mp, so->laddr.buf, so->laddr.len,
				so->raddr.buf, so->raddr.len) != DONAME_CONT)
				so->flags &= ~NAMEPROC;
			goto wgetnext;
		}
		putnext(q, mp);
		goto wgetnext;

	case M_DATA:

SOCKLOG(so, "sockmodwsrv: Got M_DATA %x bytes\n", MSGBLEN(mp));

		/*
		 * If CLTS, pre-pend the M_PROTO header.
		 */
		if (so->udata.servtype == T_CLTS) {
			if (((sockaddr_t)so->raddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				size = sizeof (struct T_unitdata_req) +
							so->rux_dev.size;
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					return (0);
				}
				fill_udata_req_addr(bp,
						(caddr_t)&so->rux_dev.addr,
						so->rux_dev.size);
#endif /* AF_UNIX */
			} else	{
				size = sizeof (struct T_unitdata_req) +
							so->raddr.len;
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					return (0);
				}
				fill_udata_req_addr(bp, so->raddr.buf,
							so->raddr.len);
			}
			linkb(bp, mp);
			mp = bp;
		}

		s = splstr();
		if (canput(q->q_next)) {

SOCKLOG(so, "sockmodwsrv: canput returned true\n", 0);

			(void)splx(s);
			putnext(q, mp);
		} else	{
			if ((so->flags & S_WBLOCKED) == 0)
				so->flags |= S_WBLOCKED;
			putbq(q, mp);

SOCKLOG(so, "sockmodwsrv: canput returned false\n", 0);

			(void)splx(s);
			return (0);
		}

		goto wgetnext;

	case M_PROTO:
		/*
		 * Assert checks if there is enough data to determine type
		 */
		ASSERT(MSGBLEN(mp) >= sizeof (long));

		pptr = (union T_primitives *)mp->b_rptr;

		switch (pptr->type) {
		default:
			putnext(q, mp);
			goto wgetnext;

		case T_UNITDATA_REQ:
		case T_DATA_REQ:

SOCKLOG(so, "sockmodwsrv: got T_[UNIT]DATA_REQ\n", 0);

			s = splstr();
			if (canput(q->q_next)) {
				(void)splx(s);
				putnext(q, mp);
			} else	{

SOCKLOG(so, "sockmodwsrv: canput returned false\n", 0);

				if ((so->flags & S_WBLOCKED) == 0)
					so->flags |= S_WBLOCKED;
				putbq(q, mp);
				(void)splx(s);
				return (0);
			}

			goto wgetnext;

		case T_CONN_REQ: {
			struct T_conn_req		*con_req;
			register int			error;
			register struct sockaddr	*addr;
			register struct bind_ux		*bind_ux;

SOCKLOG(so, "sockmodwsrv: Got T_CONN_REQ\n", 0);

			/*
			 * Make sure we can get an mblk large
			 * enough for any eventuality.
			 */
			size = max(sizeof (struct T_error_ack),
					sizeof (struct T_ok_ack));
			if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
							(mblk_t *)NULL) {
				RECOVER(q, mp, size);
				return (0);
			}

			con_req = (struct T_conn_req *)mp->b_rptr;
			if (MSGBLEN(mp) < sizeof (*con_req) ||
				MSGBLEN(mp) < (con_req->DEST_offset +
						con_req->DEST_length)) {
				snd_ERRACK(q, bp, T_CONN_REQ, EINVAL);
				freemsg(mp);
				goto wgetnext;
			}

			addr = (sockaddr_t)(mp->b_rptr + con_req->DEST_offset);
			bind_ux = (struct bind_ux *)addr;

			/*
			 * If CLTS, we have to do the connect.
			 */
			if (so->udata.servtype == T_CLTS) {
				/*
				 * If the destination address is NULL, then
				 * dissolve the association.
				 */
				if (con_req->DEST_length == 0 ||
					addr->sa_family !=
			((sockaddr_t)so->laddr.buf)->sa_family) {

SOCKLOG(so, "sockmodwsrv: CLTS: Invalid address\n", 0);

					so->raddr.len = 0;
					so->udata.so_state &= ~SS_ISCONNECTED;
					snd_ERRACK(q, bp, T_CONN_REQ,
								EAFNOSUPPORT);

					/*
					 * Dissolve any association.
					 */
					so->so_conn = (struct so_so *)NULL;

					freemsg(mp);
					goto wgetnext;
				}

				/*
				 * Remember the destination address.
				 */
				if (con_req->DEST_length > so->udata.addrsize) {
					snd_ERRACK(q, bp, T_CONN_REQ, EPROTO);
					freemsg(mp);
					goto wgetnext;
				}
				if (addr->sa_family == AF_UNIX) {
					struct so_so	*oso;
#ifdef AF_UNIX
SOCKLOG(so, "sockmodwsrv: T_CONN_REQ(CLTS-UX)\n", 0);

					if (con_req->DEST_length !=
							sizeof (*bind_ux) ||
						bind_ux->extsize >
						sizeof (bind_ux->extaddr)) {
						snd_ERRACK(q, bp, T_CONN_REQ,
								EINVAL);
						freemsg(mp);
						goto wgetnext;
					}

					/*
					 * Point this end at the other
					 * end so that netstat will work.
					 */
					if ((oso = ux_findlink(
					(caddr_t)&bind_ux->ux_extaddr.addr,
					(size_t)bind_ux->ux_extaddr.size)) ==
						(struct so_so *)NULL) {
						snd_ERRACK(q, bp, T_CONN_REQ,
								ECONNREFUSED);
						freemsg(mp);
						goto wgetnext;
					}

SOCKLOG(so, "sockmodwsrv: T_CONN_REQ(CLTS-UX) so %x\n", so);
SOCKLOG(so, "sockmodwsrv: T_CONN_REQ(CLTS-UX) oso %x\n", oso);

					so->so_conn = oso;

					ux_saveraddr(so, bind_ux);

SOCKLOG(so, "sockmodwsrv: T_CONN_REQ(CLTS-UX) size %x\n", so->rux_dev.size);
#endif /* AF_UNIX */
				} else	{
					/*
					 * Not UNIX domain.
					 */
					save_addr(&so->raddr, (caddr_t)addr,
						(size_t)con_req->DEST_length);
				}
				so->udata.so_state |= SS_ISCONNECTED;

				/*
				 * Now send back the T_OK_ACK
				 */
				snd_OKACK(q, bp, T_CONN_REQ);
				freemsg(mp);

				goto wgetnext;
			}

			/*
			 * COTS:
			 * Make sure not already connecting/ed.
			 */
			error = 0;
			if (so->udata.so_state &
					(SS_ISCONNECTING|SS_ISCONNECTED)) {
				if (so->udata.so_state & SS_ISCONNECTED)
					error = EISCONN;
				else	error = EALREADY;
			} else
			if (con_req->DEST_length == 0 || addr->sa_family !=
				((sockaddr_t)so->laddr.buf)->sa_family)
				error = EAFNOSUPPORT;
			else
			if ((so->udata.so_state & SS_ISBOUND) == 0)
				error = EPROTO;

			if (error) {
				snd_ERRACK(q, bp, T_CONN_REQ, error);
				freemsg(mp);
				goto wgetnext;
			}

			/*
			 * COTS: OPT_length will be -1 if
			 * user has O_NDELAY set.
			 */
			if (con_req->OPT_length == -1) {
				register mblk_t		*nmp;

				/*
				 * Put a large enough message on my write
				 * queue to cause the stream head
				 * to block anyone doing a write, and also
				 * cause select to work as we want, i.e.
				 * to not return true until a T_CONN_CON
				 * is returned.
				 * Note that no module/driver upstream
				 * can have a service procedure if this
				 * is to work.
				 */
				size = q->q_hiwat;
				if ((nmp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					freemsg(bp);
					return (0);
				}

				con_req->OPT_length = 0;

				nmp->b_datap->db_type = M_PROTO;
				nmp->b_wptr = nmp->b_datap->db_lim;
				so->bigmsg = nmp;
			} else	so->bigmsg = NULL;

			/*
			 * Check for UNIX domain.
			 */
			if (addr->sa_family == AF_UNIX) {
#ifdef AF_UNIX
SOCKLOG(so, "sockmodwsrv: T_CONN_REQ(COTS) on UNIX domain\n", 0);

				if (con_req->DEST_length != sizeof (*bind_ux) ||
						bind_ux->extsize >
						sizeof (bind_ux->extaddr)) {
					snd_ERRACK(q, bp, T_CONN_REQ, EPROTO);
					freemsg(mp);
					goto wgetnext;
				}

				/*
				 * Remember destination and
				 * adjust address.
				 */
				ux_saveraddr(so, bind_ux);

				(void)bcopy((caddr_t)&so->rux_dev.addr,
						(caddr_t)addr,
						(size_t)so->rux_dev.size);

				con_req->DEST_length = so->rux_dev.size;
				mp->b_wptr = mp->b_rptr + con_req->DEST_offset
						+ con_req->DEST_length;
#endif /* AF_UNIX */
			}

			freemsg(bp);	/* No longer needed */

			so->udata.so_state |= SS_ISCONNECTING;
			putnext(q, mp);

			if (so->bigmsg) {
				/*
				 * Prevent the write service procedure
				 * from being enabled so that the large
				 * message that we are about to put on it
				 * will not be lost. The queue will be enabled
				 * when the T_CONN_CON is received.
				 */
				so->flags |= S_WRDISABLE;
				noenable(q);

				/*
				 * Enqueue the large message
				 */

SOCKLOG(so, "sockmodwsrv: Putting big msg %d\n", MSGBLEN(so->bigmsg));

				putq(q, so->bigmsg);
				return (0);
			} else	goto wgetnext;
		}	/* case T_CONN_REQ: */

		case T_CONN_RES: {
			register struct T_conn_res	*conn_res;
			register struct T_conn_ind	*conn_ind;
			register queue_t		*soq;
			register mblk_t			*pmp;
			register struct so_so		*oso;

SOCKLOG(so, "sockmodwsrv: Got T_CONN_RES\n", 0);

			if (MSGBLEN(mp) < sizeof (*conn_res)) {
				size = sizeof (struct T_error_ack);
				if ((bp = _s_getmblk((mblk_t *)mp, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					return (0);
				} else	snd_ERRACK(q, bp, T_CONN_RES, EINVAL);
				goto wgetnext;
			}

			/*
			 * We have to set the local and remote addresses
			 * for the endpoint on which the connection was
			 * accepted on. The endpoint is marked connected
			 * when the T_OK_ACK is received.
			 */
			conn_res = (struct T_conn_res *)mp->b_rptr;

			/*
			 * Find the new endpoints queue_t.
			 */
			soq = conn_res->QUEUE_ptr;
			while ((soq = soq->q_next) != NULL) {
				register struct so_so *oso;

				for (oso=so_so; oso < &so_so[so_cnt]; oso++) {
					if ((oso->flags & USED) == 0)
						continue;
					if (soq->q_ptr == (caddr_t)oso) {
						/*
						 * Found it
						 */
						goto found;
					}
				}
			}
			if (soq == (queue_t *)NULL) {
				/*
				 * Something wrong here
				 * let the transport provider
				 * find it.
				 */

SOCKLOG(so, "sockmodwsrv: No queue_t\n", 0);

				putnext(q, mp);
				goto wgetnext;
			}
found:
			oso = (struct so_so *)soq->q_ptr;
			so->so_conn = oso;

			/*
			 * Set the local address of the
			 * new endpoint to the local address
			 * of the endpoint on which the connect
			 * request was received.
			 */
			save_addr(&oso->laddr, so->laddr.buf, so->laddr.len);

			/*
			 * Set the peer address of the
			 * new endpoint to the source address
			 * of the connect request.
			 * We have to find the saved
			 * T_CONN_IND for this sequence number
			 * to retrieve the correct SRC address.
			 */
			pmp = NULL;
			for (bp = so->consave; bp; bp = bp->b_next) {
				conn_ind = (struct T_conn_ind *)bp->b_rptr;
				if (conn_ind->SEQ_number ==
							conn_res->SEQ_number)
					break;
				pmp = bp;
			}
			if (bp != NULL) {
				if (pmp)
					pmp->b_next = bp->b_next;
				else	so->consave = bp->b_next;
			}
			if (((sockaddr_t)so->laddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				register struct so_so		*nso;

				if ((nso = ux_findlink((caddr_t)(bp->b_rptr +
					conn_ind->SRC_offset),
					(size_t)conn_ind->SRC_length)) ==
							(struct so_so *)NULL) {

SOCKLOG(so, "sockmodwsrv: UNIX: No peer\n", 0);

					oso->raddr.len = 0;
				} else	{
					save_addr(&oso->raddr, nso->laddr.buf,
							nso->laddr.len);

					/*
					 * Give each end of the connection
					 * a pointer to the other.
					 */
					oso->so_conn = nso;
					nso->so_conn = oso;
				}
#endif /* AF_UNIX */
			} else	save_addr(&oso->raddr,
				(caddr_t)(bp->b_rptr+conn_ind->SRC_offset),
					conn_ind->SRC_length);

			/*
			 * The new socket inherits the properties of the
			 * old socket.
			 */
			oso->udata.so_state = so->udata.so_state;
			oso->udata.so_options = so->udata.so_options;
			oso->linger = so->linger;

			freemsg(bp);

			putnext(q, mp);

			goto wgetnext;
		}	/* case T_CONN_RES */
	}	/* switch (pptr->type) */

	case M_READ:

SOCKLOG(so, "sockmodwsrv: Got M_READ\n", 0);

		freemsg(mp);

		/*
		 * If the socket is marked SS_CANTRCVMORE then
		 * send up a zero length message, to make the user
		 * get EOF. Otherwise just forget it.
		 */
		if (so->udata.so_state & SS_CANTRCVMORE)
			snd_ZERO(RD(q));

		goto wgetnext;

	case M_IOCTL:

SOCKLOG(so, "sockmodwsrv: Got M_IOCTL\n", 0);

		ASSERT(MSGBLEN(mp) == sizeof (struct iocblk));

		iocbp = (struct iocblk *)mp->b_rptr;
		if (so->flags & WAITIOCACK) {
			snd_IOCNAK(RD(q), mp, EPROTO);
			goto wgetnext;
		}

		switch (iocbp->ioc_cmd) {
		default:
			putnext(q, mp);
			goto wgetnext;

		case SI_TCL_LINK: {
			register char 			*addr;
			register int			addrlen;
			register struct tcl_sictl	*tcl_sictl;

			if (((sockaddr_t)so->laddr.buf)->sa_family != AF_UNIX) {
				snd_IOCNAK(RD(q), mp, EOPNOTSUPP);
				goto wgetnext;
			}
			if (so->udata.servtype != T_CLTS) {
				snd_IOCNAK(RD(q), mp, EOPNOTSUPP);
				goto wgetnext;
			}

			if (mp->b_cont) {
				/*
				 * Make sure there is a peer.
				 */
				if (ux_findlink((caddr_t)mp->b_cont->b_rptr,
						(size_t)MSGBLEN(mp->b_cont)) ==
								NULL) {
					snd_IOCNAK(RD(q), mp, ECONNREFUSED);
					goto wgetnext;
				}

				size = sizeof (*tcl_sictl)+MSGBLEN(mp->b_cont);
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					return (0);
				}
				addr = (caddr_t)mp->b_cont->b_rptr;
				addrlen = MSGBLEN(mp->b_cont);
			} else	{
				/*
				 * Connected, verify remote address.
				*/
				if (so->rux_dev.size == 0) {
					snd_IOCNAK(RD(q), mp, ECONNREFUSED);
					goto wgetnext;
				}

				size = sizeof (*tcl_sictl) + so->rux_dev.size;
				if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
								NULL) {
					RECOVER(q, mp, size);
					return (0);
				}
				addr = (caddr_t)&so->rux_dev.addr;
				addrlen = so->rux_dev.size;
			}

			tcl_sictl = (struct tcl_sictl *)bp->b_wptr;
			tcl_sictl->type = TCL_LINK;
			tcl_sictl->ADDR_len = addrlen;
			tcl_sictl->ADDR_offset = sizeof (*tcl_sictl);
			(void)bcopy(addr,
				(caddr_t)(bp->b_wptr + tcl_sictl->ADDR_offset),
				(size_t)tcl_sictl->ADDR_len);
			bp->b_datap->db_type = M_CTL;
			bp->b_wptr += (tcl_sictl->ADDR_offset +
							tcl_sictl->ADDR_len);

			putnext(q, bp);

			iocbp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			goto wgetnext;
		}

		case SI_TCL_UNLINK:
			if (((sockaddr_t)so->laddr.buf)->sa_family != AF_UNIX) {
				snd_IOCNAK(RD(q), mp, EOPNOTSUPP);
				goto wgetnext;
			}
			if (so->udata.servtype != T_CLTS) {
				snd_IOCNAK(RD(q), mp, EOPNOTSUPP);
				goto wgetnext;
			}

			/*
			 * Format an M_CTL and send it down.
			 */
			size = sizeof (long);
			if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
						(mblk_t *)NULL) {
				RECOVER(q, mp, size);
				goto wgetnext;
			}
			*(long *)bp->b_wptr = TCL_UNLINK;
			bp->b_datap->db_type = M_CTL;
			bp->b_wptr += sizeof (long);
			putnext(q, bp);

			iocbp->ioc_count = 0;
			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			goto wgetnext;

		case MSG_OOB:
		case MSG_PEEK:
		case MSG_OOB|MSG_PEEK: {
			register int		ilen;
			register int		olen;
			register mblk_t		*ibp;
			register mblk_t		*obp;
			register caddr_t	pos;
			int			error;

			error = 0;
			if (so->udata.etsdusize == 0 ||
				(so->udata.so_state & SS_ISBOUND) == 0 ||
				((sockaddr_t)so->laddr.buf)->sa_family
						== AF_UNIX)
				error = EOPNOTSUPP;
			else
			if (mp->b_cont == (mblk_t *)NULL ||
					so->udata.so_options & SO_OOBINLINE)
				error = EINVAL;
			else
			if (so->oob == (mblk_t *)NULL)
				error = EWOULDBLOCK;

			if (error) {
				snd_IOCNAK(RD(q), mp, error);
				goto wgetnext;
			}

			/*
			 * Process the data.
			 */
			iocbp->ioc_count = 0;
			obp = mp->b_cont;
			ibp = so->oob;
			pos = (caddr_t)ibp->b_rptr;
			for (;;) {
				ilen = MSGBLEN(ibp);
				olen = MSGBLEN(obp);
				size = MIN(olen, ilen);
				obp->b_wptr = obp->b_rptr;

				(void)bcopy(pos, (caddr_t)obp->b_wptr, size);

				pos += size;
				if ((iocbp->ioc_cmd & MSG_PEEK) == 0)
					ibp->b_rptr += size;
				obp->b_wptr += size;
				iocbp->ioc_count += size;
				ilen -= size;
				olen -= size;
				if (olen == 0) {
					/*
					 * This user block is exhausted, see
					 * if there is another.
					 */
					if (obp->b_cont) {
						/*
						 * Keep going
						 */
						obp = obp->b_cont;
						continue;
					}
					/*
					 * No more user blocks, finished.
					 */
					break;
				} else	{
					/*
					 * This oob block is exhausted, see
					 * if there is another.
					 */
					if (ibp->b_cont) {
						ibp = ibp->b_cont;
						pos = (caddr_t)ibp->b_rptr;
						continue;
					}
					/*
					 * No more oob data, finished.
					 */
					break;
				}
			}
			if (ilen == 0 && (iocbp->ioc_cmd & MSG_PEEK) == 0) {
				freemsg(so->oob);
				so->oob = NULL;
			}

			mp->b_datap->db_type = M_IOCACK;
			qreply(q, mp);
			goto wgetnext;
		}

		case SI_LISTEN: {
			register struct T_bind_req	*bind_req;
			register int			error;

SOCKLOG(so, "sockmodwsrv: Got SI_LISTEN\n", 0);

			/*
			 * If we are already bound and the
			 * backlog is 0 then we just change state.
			 *
			 * If we are already bound, and backlog is not
			 * zero, we have to do an
			 * unbind followed by the callers bind, in
			 * order to set the number of connect
			 * indications correctly. When we have done what
			 * we have needed to do we just change the callers
			 * ioctl type and start again.
			 */
			bind_req = (struct T_bind_req *)mp->b_cont->b_rptr;
			if ((so->udata.so_state & SS_ISBOUND) == 0) {
				/*
				 * Change it to a T_BIND_REQ and
				 * try again.
				 */
				iocbp->ioc_cmd = TI_BIND;
				so->udata.so_options |= SO_ACCEPTCONN;
				goto again;
			}
			/*
			 * Don't bother if the backlog is 0, the
			 * original bind got it right.
			 */
			if (bind_req->CONIND_number == 0) {

SOCKLOG(so, "sockmodwsrv: Already bound and backlog = 0\n", 0);

				so->udata.so_options |= SO_ACCEPTCONN;

				mp->b_datap->db_type = M_IOCACK;
				iocbp->ioc_count = 0;
				qreply(q, mp);

				goto wgetnext;
			}
			if (iocbp->ioc_count <
				(sizeof (*bind_req) + so->laddr.len) ||
						mp->b_cont == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, EINVAL);
				goto wgetnext;
			}

			/*
			 * Set up the T_UNBIND_REQ request.
			 */
			size = sizeof (struct T_unbind_req);
			if ((bp = _s_getmblk((mblk_t *)NULL, size)) ==
							(mblk_t *)NULL) {
				RECOVER(q, mp, size);
				return (0);
			}

			bp->b_datap->db_type = M_PROTO;
			*(long *)bp->b_wptr = T_UNBIND_REQ;
			bp->b_wptr += sizeof (struct T_unbind_req);

			/*
			 * Set up the subsequent T_BIND_REQ.
			 */
			error = 0;
			iocbp->ioc_cmd = TI_BIND;
			if (((sockaddr_t)so->laddr.buf)->sa_family == AF_UNIX) {
#ifdef AF_UNIX
				struct bind_ux	bindx;

				/*
				 * UNIX domain.
				 */
				size = so->lux_dev.size;
				if (bind_req->ADDR_length < size)
					error = EINVAL;
				else	{
					bzero((caddr_t)&bindx,
						sizeof (struct bind_ux));
					(void)bcopy((caddr_t)so->laddr.buf,
						(caddr_t)&bindx.name,
						so->laddr.len);
					(void)bcopy((caddr_t)&so->lux_dev,
						(caddr_t)&bindx.ux_extaddr,
						sizeof (struct ux_extaddr));
					(void)bcopy((caddr_t)&bindx,
						(caddr_t)bind_req +
						bind_req->ADDR_offset,
						sizeof (struct bind_ux));
					bind_req->ADDR_length =
						sizeof (struct bind_ux);
				}
#endif /* AF_UNIX */
			} else	{
				size = so->laddr.len;
				if (bind_req->ADDR_length < size)
					error = EINVAL;
				else	{
					(void)bcopy(so->laddr.buf,
						(caddr_t)(mp->b_cont->b_rptr +
						bind_req->ADDR_offset),
						size);
					bind_req->ADDR_length = size;
				}
			}
			if (error) {
				snd_IOCNAK(RD(q), mp, error);
				freemsg(bp);
				goto wgetnext;
			}
			/*
			 * No error, so send down the unbind.
			 */
			so->flags |= S_WUNBIND;
			putnext(q, bp);

			/*
			 * Save the TI_BIND request until the
			 * T_OK_ACK comes back.
			 */
			so->iocsave = mp;
			goto wgetnext;
		}

		case SI_GETUDATA:

SOCKLOG(so, "sockmodwsrv: Got SI_GETUDATA\n", 0);

			if (iocbp->ioc_count < sizeof (struct si_udata) ||
						mp->b_cont == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, EINVAL);
				goto wgetnext;
			}

			(void)bcopy((caddr_t)&so->udata,
						(caddr_t)mp->b_cont->b_rptr,
						sizeof (struct si_udata));
			mp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = sizeof (struct si_udata);
			qreply(q, mp);
			goto wgetnext;

		case TI_GETPEERNAME:

SOCKLOG(so, "sockmodwsrv: Got TI_GETPEERNAME state %x\n", so->udata.so_state);

			if ((so->udata.so_state & SS_ISCONNECTED) == 0) {
				snd_IOCNAK(RD(q), mp, ENOTCONN);
				goto wgetnext;
			}

			/*
			 * See if this is a UNIX
			 * domain endpoint.
			 */
			if (so->raddr.len &&
				((sockaddr_t)so->laddr.buf)->sa_family ==
						AF_UNIX) {
#ifdef AF_UNIX
SOCKLOG(so, "sockmodwsrv: peer len %d\n", so->raddr.len);

				so->flags |= NAMEPROC;
				if (ti_doname(q, mp, so->laddr.buf,
					so->laddr.len, so->raddr.buf,
					so->raddr.len) != DONAME_CONT) {
					so->flags &= ~NAMEPROC;
				}
				goto wgetnext;
#endif /* AF_UNIX */
			}
			/*
			 * See if the transport provider supports it.
			 */
			if ((bp = copymsg(mp)) == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, EAGAIN);
				goto wgetnext;
			}

			so->iocsave = mp;
			so->flags |= WAITIOCACK;

			putnext(q, bp);
			goto wgetnext;

		case TI_GETMYNAME:
			/*
			 * See if we have a copy of the address, or
			 * more importantly, see if this is a UNIX
			 * domain endpoint.
			 */
			if (so->laddr.len &&
				((sockaddr_t)so->laddr.buf)->sa_family ==
						AF_UNIX) {
#ifdef AF_UNIX
				so->flags |= NAMEPROC;
				if (ti_doname(q, mp, so->laddr.buf,
					so->laddr.len, so->raddr.buf,
					so->raddr.len) != DONAME_CONT) {
					so->flags &= ~NAMEPROC;
				}
				goto wgetnext;
#endif /* AF_UNIX */
			}

			/*
			 * See if the transport provider supports it.
			 */
			if ((bp = copymsg(mp)) == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, EAGAIN);
				goto wgetnext;
			}

			so->iocsave = mp;
			so->flags |= WAITIOCACK;

			putnext(q, bp);
			goto wgetnext;

		case SI_SETPEERNAME:

SOCKLOG(so, "sockmodwsrv: Got SI_SETPEERNAME\n", 0);

			if (iocbp->ioc_uid != 0)
				iocbp->ioc_error = EPERM;
			else
			if (so->udata.servtype != T_CLTS &&
				(so->udata.so_state & SS_ISCONNECTED) == 0)
				iocbp->ioc_error = ENOTCONN;
			else
			if (iocbp->ioc_count == 0 ||
				iocbp->ioc_count > so->raddr.maxlen ||
					(bp = mp->b_cont) == (mblk_t *)NULL)
				iocbp->ioc_error = EINVAL;

			if (iocbp->ioc_error) {
				snd_IOCNAK(RD(q), mp, iocbp->ioc_error);
				goto wgetnext;
			}

			so->udata.so_state |= SS_ISCONNECTED;
			save_addr(&so->raddr, (caddr_t)bp->b_rptr,
							iocbp->ioc_count);

			mp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			qreply(q, mp);
			goto wgetnext;

		case SI_SETMYNAME:

SOCKLOG(so, "sockmodwsrv: Got SI_SETMYNAME\n", 0);

			if (iocbp->ioc_uid != 0)
				iocbp->ioc_error = EPERM;
			else
			if (iocbp->ioc_count == 0 ||
				(so->udata.so_state & SS_ISBOUND) == 0 ||
				iocbp->ioc_count > so->laddr.maxlen ||
					(bp = mp->b_cont) == (mblk_t *)NULL)
				iocbp->ioc_error = EINVAL;

			if (iocbp->ioc_error) {
				snd_IOCNAK(RD(q), mp, iocbp->ioc_error);
				goto wgetnext;
			}

			save_addr(&so->laddr, (caddr_t)bp->b_rptr,
					iocbp->ioc_count);

			mp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			qreply(q, mp);
			goto wgetnext;

		case SI_SHUTDOWN: {
			register int	how;

SOCKLOG(so, "sockmodwsrv: Got SI_SHUTDOWN\n", 0);

			if (iocbp->ioc_count < sizeof (int) ||
						mp->b_cont == (mblk_t *)NULL)
				iocbp->ioc_error = EINVAL;

			if ((how = *(int *)mp->b_cont->b_rptr) > 2 || how < 0)
				iocbp->ioc_error = EINVAL;

SOCKLOG(so, "sockmodwsrv: SI_SHUTDOWN how %d\n", how);

			if (iocbp->ioc_error) {
				snd_IOCNAK(RD(q), mp, iocbp->ioc_error);
				goto wgetnext;
			}

			if (how == 0) {
				so->udata.so_state |= SS_CANTRCVMORE;

				/*
				 * Send an M_FLUSH(FLUSHR) message upstream.
				 */
				snd_FLUSHR(RD(q));
			} else if (how == 1) {
				so->udata.so_state |= SS_CANTSENDMORE;

				if (so->udata.servtype == T_COTS_ORD) {
					/*
					 * Send an orderly release.
					 */
					size = sizeof (struct T_ordrel_req);
					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}

					bp->b_datap->db_type = M_PROTO;
					*(long *)bp->b_wptr = T_ORDREL_REQ;
					bp->b_wptr +=
						sizeof (struct T_ordrel_req);
					putnext(q, bp);
				}
			} else if (how == 2) {
				/*
				 * If orderly release is supported then send
				 * one, else send a disconnect.
				 */
				so->udata.so_state |= (SS_CANTRCVMORE|
							SS_CANTSENDMORE);
				if (so->udata.servtype == T_COTS_ORD) {
					size = sizeof (struct T_ordrel_req);
					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}

					bp->b_datap->db_type = M_PROTO;
					*(long *)bp->b_wptr = T_ORDREL_REQ;
					bp->b_wptr +=
						sizeof (struct T_ordrel_req);
					putnext(q, bp);

				} else if (so->udata.servtype == T_COTS) {
					register struct T_discon_req *req;

					size = sizeof (struct T_discon_req);
					if ((bp = _s_getmblk((mblk_t *)NULL,
						size)) == (mblk_t *)NULL) {
						RECOVER(q, mp, size);
						return (0);
					}

					req = (struct T_discon_req *)bp->b_wptr;
					req->PRIM_type = T_DISCON_REQ;
					req->SEQ_number = -1;

					bp->b_datap->db_type = M_PROTO;
					bp->b_wptr += sizeof (*req);
					putnext(q, bp);
				}
				/*
				 * Send an M_FLUSH(FLUSHR) message upstream.
				 */
				snd_FLUSHR(RD(q));
			}

			/*
			 * All is well, send an ioctl ACK back to the user.
			 */
			mp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			qreply(q, mp);

			if (how == 1)
				snd_ERRORW(RD(q));

			if (how == 2)
				snd_HANGUP(RD(q));

			goto wgetnext;
		}	/* case SI_SHUTDOWN: */

		case TI_BIND:
		case TI_UNBIND:
		case TI_OPTMGMT:
			if (mp->b_cont == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, EINVAL);
				goto wgetnext;
			}
			if (!pullupmsg(mp->b_cont, -1)) {
				snd_IOCNAK(RD(q), mp, ENOSR);
				goto wgetnext;
			}
			if (iocbp->ioc_cmd == TI_BIND) {
				register struct T_bind_req	*bind_req;
				register struct sockaddr	*addr;

				bind_req =
					(struct T_bind_req *)mp->b_cont->b_rptr;

				if (MSGBLEN(mp->b_cont) < sizeof (*bind_req)) {
					snd_IOCNAK(RD(q), mp, EPROTO);
					goto wgetnext;
				}
				if (MSGBLEN(mp->b_cont) <
						(bind_req->ADDR_offset +
						bind_req->ADDR_length)) {
					snd_IOCNAK(RD(q), mp, EPROTO);
					goto wgetnext;
				}

				addr = (sockaddr_t)(mp->b_cont->b_rptr +
						bind_req->ADDR_offset);

				if (bind_req->ADDR_length >= 2 &&
					addr->sa_family == AF_UNIX) {
#ifdef AF_UNIX
					register struct bind_ux *bind_ux;

					/*
					 * Sanity check on size.
					 */
					if (bind_req->ADDR_length <
							sizeof (*bind_ux)) {
						snd_IOCNAK(RD(q), mp, EPROTO);
						goto wgetnext;
					}

SOCKLOG(so, "sockmodwsrv: UNIX domain BIND\n", 0);

					/*
					 * Remember the address string
					 */
					bind_ux = (struct bind_ux *)addr;
					save_addr(&so->laddr, (caddr_t)addr,
						sizeof (struct sockaddr_un));

					/*
					 * If the user specified an address
					 * to bind to then save it and adjust
					 * the address so that the transport
					 * provider sees what we want it to.
					 */
					size = bind_ux->extsize;
					if (size) {

SOCKLOG(so, "sockmodwsrv: Non null BIND\n", 0);

						/*
						 * Non-Null bind request.
						 */
						(void)bcopy(
						(caddr_t)&bind_ux->extaddr,
						(caddr_t)&so->lux_dev.addr,
						size);
						so->lux_dev.size = size;

						/*
						 * Adjust destination, by moving
						 * the bind part of bind_ux
						 * to the beginning of the
						 * address.
						 */
						(void)bcopy(
						(caddr_t)&so->lux_dev.addr,
						(caddr_t)bind_ux,
						so->lux_dev.size);

						bind_req->ADDR_length =
							so->lux_dev.size;
						mp->b_cont->b_wptr =
							mp->b_cont->b_rptr +
							sizeof (*bind_req) +
							so->lux_dev.size;
					} else	{
						bind_req->ADDR_length = 0;
						bind_req->ADDR_offset = 0;
						mp->b_cont->b_wptr =
							mp->b_cont->b_rptr +
							sizeof (*bind_req);

						so->lux_dev.size = 0;
					}
					iocbp->ioc_count = MSGBLEN(mp->b_cont);

SOCKLOG(so, "sockmodwsrv: BIND length %d\n", bind_req->ADDR_length);

					/*
					 * Add it to the list of UNIX
					 * domain endpoints.
					 */
					ux_addlink(so);
#endif /* AF_UNIX */
				}
			}
			if (iocbp->ioc_cmd == TI_OPTMGMT) {
				register int	retval;

				/*
				 * Do any socket level options
				 * processing.
				 */
				retval = so_options(q, mp->b_cont);
				if (retval == 1) {
					mp->b_datap->db_type = M_IOCACK;
					qreply(q, mp);
					goto wgetnext;
				}
				if (retval < 0) {
					snd_IOCNAK(RD(q), mp, -retval);
					goto wgetnext;
				}
			}

			if ((bp = copymsg(mp->b_cont)) == (mblk_t *)NULL) {
				snd_IOCNAK(RD(q), mp, ENOSR);
				goto wgetnext;
			}

			so->iocsave = mp;
			so->flags |= WAITIOCACK;

			if (iocbp->ioc_cmd == TI_GETINFO)
				bp->b_datap->db_type = M_PCPROTO;
			else	bp->b_datap->db_type = M_PROTO;

			putnext(q, bp);
			goto wgetnext;
		}	/* switch (iocbp->ioc_cmd) */
	}	/* switch (mp->b_datap->db_type) */
}

/*
 * Returns  -<error number>
 *	0 if option needs to be passed down
 *	1 if option has been serviced
 *
 *	Should not assume the T_OPTMGMT_REQ buffer is large enough to hold
 *	the T_OPTMGMT_ACK message.
 */
STATIC int
so_options(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;

{
	/*
	 * Trap the ones that we must handle directly
	 * or that we must take action on in addition
	 * to sending downstream for the TP.
	 */
	register struct T_optmgmt_req	*opt_req;
	register struct opthdr		*opt;
	register struct so_so		*so;

	so = (struct so_so *)q->q_ptr;
	opt_req = (struct T_optmgmt_req *)mp->b_rptr;
	if (MSGBLEN(mp) < sizeof (*opt_req))
		return (-EINVAL);

	opt = (struct opthdr *)(mp->b_rptr + opt_req->OPT_offset);

	if (MSGBLEN(mp) < (opt_req->OPT_length + sizeof (*opt_req)))
		return (-EINVAL);

	if (opt->level != SOL_SOCKET)
		return (0);

	switch (opt_req->MGMT_flags) {
	case T_CHECK:
		/*
		 * Retrieve current value.
		 */
		switch (opt->name) {
		case SO_ERROR:
			*(int *)OPTVAL(opt) = so->so_error;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			so->so_error = 0;
			return (1);

		case SO_DEBUG:
		case SO_OOBINLINE:
		case SO_REUSEADDR:
		case SO_BROADCAST:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_USELOOPBACK:
			*(int *)OPTVAL(opt) = so->udata.so_options & opt->name;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_LINGER: {
			struct linger  *l;

			if (opt->len != sizeof (struct linger))
				return (-EINVAL);

			l = (struct linger *)OPTVAL(opt);
			if (so->udata.so_options & SO_LINGER) {
				l->l_onoff = 1;
				l->l_linger = so->linger;
			} else	{
				l->l_onoff = 0;
				l->l_linger = 0;
			}
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (struct linger);
			return (1);
		}

		case SO_SNDBUF:
			*(int *)OPTVAL(opt) = so->sndbuf;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_RCVBUF:
			*(int *)OPTVAL(opt) = so->rcvbuf;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_SNDLOWAT:
			*(int *)OPTVAL(opt) = so->sndlowat;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_RCVLOWAT:
			*(int *)OPTVAL(opt) = so->rcvlowat;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_SNDTIMEO:
			*(int *)OPTVAL(opt) = so->sndtimeo;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_RCVTIMEO:
			*(int *)OPTVAL(opt) = so->rcvtimeo;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_PROTOTYPE:
			*(int *)OPTVAL(opt) = so->prototype;
			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		default:
			return (-ENOPROTOOPT);
		}

	case T_NEGOTIATE:
		/*
		 * We wait until the negotiated option comes
		 * back before setting most of these.
		 */
		switch (opt->name) {
		case SO_TYPE:
		case SO_ERROR:
			return (-ENOPROTOOPT);

		case SO_LINGER:
			if (opt->len != OPTLEN(sizeof (struct linger)))
				return (-EINVAL);
			break;

		case SO_OOBINLINE:
			if (*(int *)OPTVAL(opt))
				so->udata.so_options |= SO_OOBINLINE;
			else	so->udata.so_options &= ~SO_OOBINLINE;

			opt_req->PRIM_type = T_OPTMGMT_ACK;
			opt->len = sizeof (int);
			return (1);

		case SO_DEBUG:
		case SO_USELOOPBACK:
		case SO_REUSEADDR:
		case SO_BROADCAST:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
		case SO_SNDTIMEO:
		case SO_RCVTIMEO:
		case SO_PROTOTYPE:
			if (opt->len != OPTLEN(sizeof (int)))
				return (-EINVAL);
			break;

		default:
			return (-ENOPROTOOPT);
		}
	}
	/*
	 * Set so_option so that we know what
	 * we are dealing with.
	 */
	so->so_option = opt->name;
	return (0);
}

/*
 * The transport provider does not support the option,
 * but we must because it is a SOL_SOCKET option.
 * If value is non-zero, then the option should
 * be set, otherwise it is reset.
 */
STATIC mblk_t *
_s_makeopt(so)
	register struct so_so	*so;
{
	register mblk_t		*bp;
	struct T_optmgmt_req	*opt_req;
	struct linger		*l;
	register struct opthdr	*opt;

	/*
	 * Get the saved request.
	 */
	opt_req = (struct T_optmgmt_req *)so->iocsave->b_cont->b_rptr;
	opt = (struct opthdr *)(so->iocsave->b_cont->b_rptr +
					opt_req->OPT_offset);
	switch (opt->name) {
	case SO_LINGER:
		l = (struct linger *)OPTVAL(opt);
		if (l->l_onoff) {
			so->udata.so_options |= SO_LINGER;
			so->linger = l->l_linger;
		} else	{
			so->udata.so_options &= ~SO_LINGER;
			so->linger = 0;
		}

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (struct linger);
		break;

	case SO_DEBUG:
	case SO_KEEPALIVE:
	case SO_DONTROUTE:
	case SO_USELOOPBACK:
	case SO_BROADCAST:
	case SO_REUSEADDR:
		if (*(int *)OPTVAL(opt))
			so->udata.so_options |= opt->name;
		else	so->udata.so_options &= ~opt->name;

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_SNDBUF:
		so->sndbuf = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_RCVBUF:
		so->rcvbuf = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_SNDLOWAT:
		so->sndlowat = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_RCVLOWAT:
		so->rcvlowat = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_SNDTIMEO:
		so->sndtimeo = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_RCVTIMEO:
		so->rcvtimeo = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;

	case SO_PROTOTYPE:
		so->prototype = *(int *)OPTVAL(opt);

		opt_req->PRIM_type = T_OPTMGMT_ACK;
		opt->len = sizeof (int);
		break;
	}

	bp = so->iocsave->b_cont;
	so->iocsave->b_cont = NULL;
	return (bp);
}

/*
 * The transport provider returned T_OPTMGMT_ACK,
 * copy the values it negotiated.
 */
STATIC void
_s_setopt(mp, so)
	register mblk_t		*mp;
	register struct so_so	*so;
{
	struct T_optmgmt_ack	*opt_ack;
	register struct opthdr	*opt;
	register struct linger	*l;

	opt_ack = (struct T_optmgmt_ack *)mp->b_rptr;
	opt = (struct opthdr *)(mp->b_rptr + opt_ack->OPT_offset);

	switch (opt->name) {
	case SO_DEBUG:
	case SO_USELOOPBACK:
	case SO_REUSEADDR:
	case SO_BROADCAST:
	case SO_KEEPALIVE:
	case SO_DONTROUTE:
		if (*(int *)OPTVAL(opt))
			so->udata.so_options |= opt->name;
		else	so->udata.so_options &= ~opt->name;
		break;

	case SO_LINGER:
		l = (struct linger *)OPTVAL(opt);
		if (l->l_onoff) {
			so->udata.so_options |= SO_LINGER;
			so->linger = l->l_linger;
		} else	{
			so->udata.so_options &= ~SO_LINGER;
			so->linger = 0;
		}
		break;

	case SO_SNDBUF:
		so->sndbuf = *(int *)OPTVAL(opt);
		break;

	case SO_RCVBUF:
		so->rcvbuf = *(int *)OPTVAL(opt);
		break;

	case SO_SNDLOWAT:
		so->sndlowat = *(int *)OPTVAL(opt);
		break;

	case SO_RCVLOWAT:
		so->rcvlowat = *(int *)OPTVAL(opt);
		break;

	case SO_SNDTIMEO:
		so->sndtimeo = *(int *)OPTVAL(opt);
		break;

	case SO_RCVTIMEO:
		so->rcvtimeo = *(int *)OPTVAL(opt);
		break;

	case SO_PROTOTYPE:
		so->prototype = *(int *)OPTVAL(opt);
		break;
	}
	return;
}


/*
 * Set sizes of buffers
 */
#define	DEFSIZE	128
static long
_t_setsize(infosize)
	long	infosize;
{
	switch (infosize)
	{
		case -1:
			return (DEFSIZE);
		case -2:
			return (0);
		default:
			return (infosize);
	}
}

/*
 * Translate a TLI error into a system error as best we can.
 */
static ushort tli_errs[] = {
		0,		/* no error	*/
		EADDRNOTAVAIL,  /* TBADADDR	*/
		ENOPROTOOPT,	/* TBADOPT	*/
		EACCES,		/* TACCES	*/
		EBADF,		/* TBADF	*/
		EADDRNOTAVAIL,	/* TNOADDR	*/
		EPROTO,		/* TOUTSTATE	*/
		EPROTO,		/* TBADSEQ	*/
		0,		/* TSYSERR - will never get	*/
		EPROTO,		/* TLOOK - should never be sent by transport */
		EMSGSIZE,	/* TBADDATA	*/
		EMSGSIZE,	/* TBUFOVFLW	*/
		EPROTO,		/* TFLOW	*/
		EWOULDBLOCK,	/* TNODATA	*/
		EPROTO,		/* TNODIS	*/
		EPROTO,		/* TNOUDERR	*/
		EINVAL,		/* TBADFLAG	*/
		EPROTO,		/* TNOREL	*/
		EOPNOTSUPP,	/* TNOTSUPPORT	*/
		EPROTO,		/* TSTATECHNG	*/
};

static int
tlitosyserr(terr)
	register int	terr;
{
	if (terr > (sizeof (tli_errs) / sizeof (ushort)))
		return (EPROTO);
	else	return (int)tli_errs[terr];
}

/*
 * This function will walk through the message block given
 * looking for a single data block large enough to hold
 * size bytes. If it finds one it will free the surrounding
 * blocks and return a pointer to the one of the appropriate
 * size. If no component of the passed in message is large enough,
 * then if the system can't provide one of suitable size the
 * passed in message block is untouched. If the system can provide
 * one then the passed in message block is freed.
 */
STATIC mblk_t *
_s_getmblk(mp, size)
	register mblk_t		*mp;
	register size_t		size;
{
	register mblk_t		*nmp;
	register mblk_t		*bp;

	bp = mp;
	while (bp) {
		if (MBLKLEN(bp) >= (int)size) {
			bp->b_rptr = bp->b_wptr = bp->b_datap->db_base;
			while (mp && bp != mp) {
				/*
				 * Free each block up to the one
				 * we want.
				 */
				nmp = mp->b_cont;
				freeb(mp);
				mp = nmp;
			}
			if (bp->b_cont) {
				/*
				 * Free each block after the one
				 * we want.
				 */
				nmp = bp->b_cont;
				freemsg(nmp);
				bp->b_cont = 0;
			}
			return (bp);
		}
		bp = bp->b_cont;

	}
	if ((bp =  allocb(size, BPRI_MED)) == (mblk_t *)NULL) {
		/*
		 * But we have not touched mp.
		 */

strlog(SIMOD_ID, -1, 0, SL_TRACE, "_s_getmblk: No memory\n", 0);

		return ((mblk_t *)NULL);
	} else	{

#ifdef DEBUG
strlog(SIMOD_ID, -1, 0, SL_TRACE, "_s_getmblk: Allocated %d bytes\n", size);
#endif /* DEBUG */

		freemsg(mp);
		return (bp);
	}
}

STATIC void
snd_ERRACK(q, bp, prim, serr)
	register queue_t	*q;
	register mblk_t 	*bp;
	register int		serr;
	register int		prim;
{
	register struct T_error_ack	*tea;
	register struct so_so		*so;

	so = (struct so_so *)q->q_ptr;

	tea = (struct T_error_ack *) bp->b_rptr;
	bp->b_wptr += sizeof (struct T_error_ack);
	bp->b_datap->db_type = M_PCPROTO;
	tea->ERROR_prim = prim;
	tea->PRIM_type = T_ERROR_ACK;
	tea->TLI_error = TSYSERR;
	tea->UNIX_error = serr;
	qreply(q, bp);

	so->so_error = serr;
	return;
}

STATIC void
snd_OKACK(q, mp, prim)
	register queue_t	*q;
	register mblk_t		*mp;
	register int		prim;
{
	register struct T_ok_ack	*ok_ack;

	mp->b_datap->db_type = M_PCPROTO;
	ok_ack = (struct T_ok_ack *)mp->b_rptr;
	mp->b_wptr += sizeof (struct T_ok_ack);
	ok_ack->CORRECT_prim = prim;
	ok_ack->PRIM_type = T_OK_ACK;
	qreply(q, mp);
	return;
}


STATIC
so_init(so, info_ack)
	register struct so_so		*so;
	register struct T_info_ack	*info_ack;
{
	/*
	 * Common stuff.
	 */
	so->udata.servtype = info_ack->SERV_type;
	so->udata.tidusize = so->tp_info.tsdu =
		_t_setsize(info_ack->TIDU_size);

	so->udata.addrsize = so->tp_info.addr =
		_t_setsize(info_ack->ADDR_size);

	so->udata.optsize = so->tp_info.options =
		_t_setsize(info_ack->OPT_size);

	so->udata.etsdusize = so->tp_info.etsdu =
		_t_setsize(info_ack->ETSDU_size);

	switch (info_ack->SERV_type) {
	case T_CLTS:
		switch (info_ack->CURRENT_state) {
		case TS_UNBND:
			so->udata.so_state = 0;
			so->udata.so_options = 0;
			break;

		case TS_IDLE:
			so->udata.so_state |= SS_ISBOUND;
			so->udata.so_options = 0;
			break;

		default:
			return (EINVAL);
		}
		break;

	case T_COTS:
	case T_COTS_ORD:
		switch (info_ack->CURRENT_state) {
		case TS_UNBND:
			so->udata.so_state = 0;
			so->udata.so_options = 0;
			break;

		case TS_IDLE:
			so->udata.so_state |= SS_ISBOUND;
			so->udata.so_options = 0;
			break;

		case TS_DATA_XFER:
			so->udata.so_state |= (SS_ISBOUND|SS_ISCONNECTED);
			so->udata.so_options = 0;
			break;

		default:
			return (EINVAL);
		}
		break;

	default:
		return (EINVAL);
	}
	return (0);
}

STATIC void
strip_zerolen(mp)
	register mblk_t	*mp;
{
	register mblk_t *bp;

	/*
	 * Assumes the first mblk is never zero length,
	 * and is actually some kind of header.
	 */
	for (bp = mp, mp = mp->b_cont; mp && mp->b_cont; mp = mp->b_cont) {
		if (MSGBLEN(mp) == 0) {
			bp->b_cont = mp->b_cont;
			mp->b_cont = NULL;
			freeb(mp);
			mp = bp;
		} else	bp = mp;
	}
	return;
}

STATIC void
save_addr(save, buf, len)
	register struct netbuf	*save;
	register char		*buf;
	register size_t		len;
{
	register size_t llen;

	llen = min(save->maxlen, len);

#ifdef DEBUG
strlog(SIMOD_ID, -1, 0, SL_TRACE, "save_addr: Copying %d bytes\n", llen);
#endif /* DEBUG */

	(void)bcopy(buf, save->buf, llen);
	save->len = llen;
	return;
}

STATIC void
snd_ZERO(q)
	register queue_t	*q;
{
	register mblk_t		*mp;
	register struct so_so	*so;

	so = (struct so_so *)q->q_ptr;
	if ((mp = _s_getmblk((mblk_t *)NULL, (size_t)1)) == (mblk_t *)NULL) {
		if (!bufcall(1, BPRI_MED, snd_ZERO, (caddr_t)q))
			(void)timeout(snd_ZERO, (caddr_t)q, SIMWAIT);
	} else	{

SOCKLOG(so, "snd_ZERO: Sending up zero length msg\n", 0);

		putnext(q, mp);
	}
	return;
}

STATIC void
snd_ERRORW(q)
	register queue_t	*q;
{
	register mblk_t		*mp;
	register struct so_so	*so;

	if ((mp = _s_getmblk((mblk_t *)NULL, (size_t)2)) == (mblk_t *)NULL) {
		if (!bufcall(2, BPRI_MED, snd_ERRORW, (caddr_t)q))
			(void)timeout(snd_ERRORW, (caddr_t)q, SIMWAIT);
		return;
	}

	mp->b_datap->db_type = M_ERROR;
	*mp->b_wptr++ = NOERROR;
	*mp->b_wptr++ = EPIPE;

	so = (struct so_so *)q->q_ptr;

SOCKLOG(so, "snd_ERRORW: Sending up M_ERROR\n", 0);

	putnext(q, mp);
	return;
}

STATIC void
snd_FLUSHR(q)
	register queue_t	*q;
{
	if (putctl1(q, M_FLUSH, FLUSHR) == 0)
		if (!bufcall(1, BPRI_MED, snd_FLUSHR, (caddr_t)q))
			(void)timeout(snd_FLUSHR, (caddr_t)q, SIMWAIT);
	return;
}

STATIC void
snd_HANGUP(q)
	register queue_t	*q;
{
	if (putctl(q, M_HANGUP) == 0)
		if (!bufcall(sizeof (mblk_t), BPRI_MED, snd_HANGUP, (caddr_t)q))
			(void)timeout(snd_HANGUP, (caddr_t)q, SIMWAIT);
	return;
}

STATIC void
snd_IOCNAK(q, mp, error)
	register queue_t	*q;
	register mblk_t		*mp;
	register int		error;
{
	register struct iocblk	*iocbp;
	register struct so_so	*so;

	mp->b_datap->db_type = M_IOCNAK;

	iocbp = (struct iocblk *)mp->b_rptr;
	so = (struct so_so *)q->q_ptr;
	iocbp->ioc_error = so->so_error = error;
	iocbp->ioc_count = 0;

	putnext(q, mp);
	return;
}

/*
 * The following complicated procedure is an attempt to get the
 * semantics right for closing the socket down.
 */
STATIC void
do_ERROR(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register struct so_so	*so;

	so = (struct so_so *)q->q_ptr;

	/*
	 * Disconnect received. Send up new M_ERROR
	 * with read side set to disconnect error
	 * and write side set to EPIPE.
	 */

SOCKLOG(so, "do_ERROR: Sending up M_ERROR with read error %d\n", so->so_error);

	mp->b_wptr = mp->b_rptr = mp->b_datap->db_base;
	mp->b_datap->db_type = M_ERROR;
	*mp->b_wptr++ = so->so_error ? so->so_error : NOERROR;
	*mp->b_wptr++ = EPIPE;
	putnext(q, mp);
	return;
}

#ifdef AF_UNIX
/*
 * Looks up the socket structure which has as
 * its local dev/ino the same as passed in.
 */
STATIC struct so_so *
ux_findlink(addr, len)
	register char		*addr;
	register size_t		len;
{
	register struct so_so 	*so;

	for (so = so_ux_list; so != NULL; so = so->so_ux.next) {
		if (bcmp(addr, (caddr_t)&so->lux_dev.addr, len) == 0)
			return (so);
	}
	return ((struct so_so *)NULL);
}

STATIC void
ux_dellink(so)
	register struct so_so	*so;
{
	register struct so_so	*oso;

	if ((oso = so->so_ux.next) != (struct so_so *)NULL)
		oso->so_ux.prev = so->so_ux.prev;

	if ((oso = so->so_ux.prev) != (struct so_so *)NULL)
		oso->so_ux.next = so->so_ux.next;
	else	so_ux_list = so->so_ux.next;
	return;
}

STATIC void
ux_addlink(so)
	register struct so_so	*so;
{
	so->so_ux.next = so_ux_list;
	so->so_ux.prev = NULL;
	if (so_ux_list)
		so_ux_list->so_ux.prev = so;
	so_ux_list = so;
	return;
}

/*
 * When a T_BIND_ACK is received, copy back
 * both parts of the address into the right
 * places for the user.
 */
STATIC void
ux_restoreaddr(so, mp, addr, addrlen)
	register struct so_so		*so;
	register mblk_t			*mp;
	register char			*addr;
	register size_t			addrlen;
{
	struct T_bind_ack		*bind_ack;
	struct bind_ux			*bind_ux;

	bind_ack = (struct T_bind_ack *)mp->b_rptr;
	bind_ux = (struct bind_ux *)(mp->b_rptr + bind_ack->ADDR_offset);

	/*
	 * Copy address actually bound to.
	 */
	(void)bcopy(addr, (caddr_t)&bind_ux->extaddr, addrlen);
	bind_ux->extsize = addrlen;

	/*
	 * Copy address the user thought was bound to.
	 */
	(void)bzero((caddr_t)&bind_ux->name, sizeof (bind_ux->name));
	(void)bcopy(so->laddr.buf, (caddr_t)&bind_ux->name,
			so->laddr.len);
	bind_ack->ADDR_length = sizeof (*bind_ux);
	return;
}

/*
 * In a T_CONN_REQ, save both parts
 * of the address.
 */
STATIC void
ux_saveraddr(so, bind_ux)
	register struct so_so		*so;
	register struct bind_ux		*bind_ux;
{
	save_addr(&so->raddr, (caddr_t)&bind_ux->name,
		sizeof (struct sockaddr_un));

	(void)bcopy((caddr_t)&bind_ux->extaddr,
		(caddr_t)&so->rux_dev.addr, bind_ux->extsize);
	so->rux_dev.size = bind_ux->extsize;
	return;
}

/*
 * Fill in a T_UNITDATA_REQ address
 */
STATIC void
fill_udata_req_addr(bp, addr, len)
	register mblk_t			*bp;
	register char			*addr;
	register size_t			len;
{
	register struct T_unitdata_req	*udata_req;

	udata_req = (struct T_unitdata_req *)bp->b_rptr;
	udata_req->DEST_length = len;
	udata_req->DEST_offset = sizeof (*udata_req);
	(void)bcopy(addr, (caddr_t)(bp->b_rptr + udata_req->DEST_offset), len);

	udata_req->PRIM_type = T_UNITDATA_REQ;
	udata_req->OPT_length = 0;
	udata_req->OPT_offset = 0;

	bp->b_datap->db_type = M_PROTO;
	bp->b_wptr = bp->b_rptr + sizeof (*udata_req) + len;
	return;
}

/*
 * Fill in a T_UNITDATA_IND address
 */
STATIC void
fill_udata_ind_addr(bp, addr, len)
	register mblk_t			*bp;
	register char			*addr;
	register size_t			len;
{
	register struct T_unitdata_ind	*udata_ind;

	udata_ind = (struct T_unitdata_ind *)bp->b_rptr;
	udata_ind->SRC_length = len;
	udata_ind->SRC_offset = sizeof (*udata_ind);
	(void)bcopy(addr, (caddr_t)(bp->b_rptr + udata_ind->SRC_offset), len);

	bp->b_datap->db_type = M_PROTO;
	bp->b_wptr = bp->b_rptr + sizeof (*udata_ind) + len;
	return;
}
#endif /* AF_UNIX */

STATIC mblk_t *
_s_getloaned(q, mp, size)
	register queue_t	*q;
	register mblk_t		*mp;
	register int		size;
{
	char			*buf;
	struct free_ptr		*urg_ptr;
	mblk_t			*bp;
	struct so_so		*so;

	so = (struct so_so *)q->q_ptr;

	/*
	 * Allocate the message that will be used
	 * in the callback routine to flush the
	 * band 1 message from the stream head.
	 */
	if ((bp = allocb(2, BPRI_MED)) == (mblk_t *)NULL) {
		RECOVER(q, mp, 2);
		return ((mblk_t *)NULL);
	}

	/*
	 * Allocate the buffer to hold the OOB data.
	 */
	if ((buf = (char *)kmem_alloc(size, KM_NOSLEEP)) == NULL) {
		RECOVER(q, mp, size);
		freeb(bp);
		return ((mblk_t *)NULL);
	}

	if ((so->udata.so_options) & SO_OOBINLINE) {
		register caddr_t	ptr;
		register size_t		left;
		register size_t		count;
		register mblk_t		*nmp;

		/*
		 * Copy the OOB data into
		 * the buffer, skipping the
		 * M_PROTO header.
		 */
		count = 0;
		left = size;
		ptr = buf;
		for (nmp = mp->b_cont; nmp; nmp = nmp->b_cont) {
			count = min(left, (size_t)MSGBLEN(nmp));
			bcopy((caddr_t)nmp->b_rptr, ptr, count);
			ptr += count;
			left -= count;
		}
	}

	/*
	 * Allocate the data structure we need to
	 * be passed to the callback routine when
	 * the esballoc'ed message is freed.
	 */
	if ((urg_ptr = (struct free_ptr *)kmem_alloc(sizeof (*urg_ptr),
				KM_NOSLEEP)) == NULL) {
		RECOVER(q, mp, sizeof (*urg_ptr));
		kmem_free(buf, size);
		freeb(bp);
		return ((mblk_t *)NULL);
	}

	/*
	 * Initialize the free routine data structure,
	 * with the information it needs to do its
	 * tasks.
	 */
	urg_ptr->free_rtn.free_func = free_urg;
	urg_ptr->free_rtn.free_arg = (char *)urg_ptr;
	urg_ptr->buf = buf;
	urg_ptr->buflen = size;
	urg_ptr->rdq = q;
	urg_ptr->mp = bp;

	/*
	 * Need to do the following because a free
	 * routine could be called when the
	 * queue pointer is invalid.
	 */
	urg_ptr->so = so;

	/*
	 * Get a class 0 data block
	 * and attach our buffer.
	 */
	if ((bp = esballoc(buf, size, BPRI_MED, &urg_ptr->free_rtn)) ==
					(mblk_t *)NULL) {
		RECOVER(q, mp, sizeof (mblk_t));
		kmem_free(buf, size);
		freeb(urg_ptr->mp);
		kmem_free(urg_ptr, sizeof (*urg_ptr));

		return ((mblk_t *)NULL);
	}
	bp->b_wptr += size;

	so->esbcnt++;
	return (bp);
}

STATIC void
free_urg(arg)
	register char		*arg;
{
	register struct free_ptr	*urg_ptr;
	register int			s;
	register struct so_so		*so;
	register mblk_t			*bp;

	urg_ptr = (struct free_ptr *)arg;
	so = (struct so_so *)urg_ptr->so;

SOCKLOG(so, "free_urg: hasoutofband %d\n", so->hasoutofband);

	s = splstr();
	so->hasoutofband--;

	if ((so->flags & S_WCLOSE) == 0 && so->hasoutofband == 0) {
		/*
		 * There is no more URG data pending,
		 * so we can flush the band 1 message.
		 */
		bp = urg_ptr->mp;
		bp->b_datap->db_type = M_FLUSH;
		*bp->b_wptr++ = FLUSHR|FLUSHBAND;
		*bp->b_wptr++ = 1;	/* Band to flush */

SOCKLOG(so, "free_urg: sending up M_FLUSH - band 1\n", 0);

		putnext(urg_ptr->rdq, bp);
	} else	{
		/*
		 * Free the message that we would have
		 * done the M_FLUSH with.
		 */
		freeb(urg_ptr->mp);
	}
	/*
	 * Free up the buffers.
	 */
	kmem_free(urg_ptr->buf, urg_ptr->buflen);
	kmem_free(urg_ptr, sizeof (*urg_ptr));

	so->esbcnt--;
	if (so->flags & S_WCLOSE && so->esbcnt == 0) {
SOCKLOG(so, "free_urg: clearing so_so\n", 0);
		so->flags = 0;
	}
	splx(s);
	return;
}

STATIC int
do_urg_outofline(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register struct so_so	*so;
	register mblk_t		*bp;
	int			size;
	int			s;
	register queue_t	*qp;

	so = (struct so_so *)q->q_ptr;
	size = sizeof (struct T_exdata_ind);

	/*
	 * Find the stream head.
	 */
	s = splstr();
	for (qp = q->q_next; qp->q_next; qp = qp->q_next)
		;

	if (qp->q_first == (mblk_t *)NULL) {
		mblk_t	*nbp;
		mblk_t	*nmp;

SOCKLOG(so, "do_urg_outofline: nothing at stream head\n", 0);

		(void)splx(s);

		/*
		 * This is to make SIGURG happen,
		 * and I_ATMARK to return TRUE.
		 */
		if ((nmp = _s_getband1(q, mp)) == (mblk_t *)NULL)
			return (-1);
		nmp->b_flag |= MSGMARK;

		/*
		 * This is just to make SIGIO happen,
		 * it will be flushed immediately.
		 */
		if ((bp = allocb(1, BPRI_MED)) == (mblk_t *)NULL) {
			RECOVER(q, mp, 1);
			freemsg(nmp);
			return (-1);
		}
		bp->b_datap->db_type = M_PROTO;

		/*
		 * Allocate a loaned message for future
		 * use when normal data is received.
		 */
		if ((nbp = _s_getloaned(q, mp, size)) == (mblk_t *)NULL) {
			freemsg(bp);
			freemsg(nmp);
			return (-1);
		}
		nbp->b_datap->db_type = M_PROTO;
		*(long *)nbp->b_rptr = T_EXDATA_IND;

		/*
		 * Change mp into an M_FLUSH,
		 * after first saving the OOB data.
		 */
		if (so->oob)
			freemsg(so->oob);
		so->oob = mp->b_cont;
		mp->b_cont = (mblk_t *)NULL;

		mp->b_datap->db_type = M_FLUSH;
		mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		*mp->b_wptr++ = FLUSHR|FLUSHBAND;
		*mp->b_wptr++ = 0;	/* Band to flush */

		so->urg_msg = nbp;	/* Save free msg	*/

SOCKLOG(so, "do_urg_outofline: sending band 0, 1, M_FLUSH\n", 0);

		putnext(q, bp);		/* Send up band 0	*/
		putnext(q, nmp);	/* Send up band 1	*/
		putnext(q, mp);			/* Flush band 0		*/

		s = splstr();
		so->hasoutofband++;
		(void)splx(s);

	} else  {
		/*
		 * Something on stream head's read queue.
		 */
		if (qp->q_first->b_band == 1 && qp->q_last->b_band == 0) {

SOCKLOG(so, "do_urg_outofline: both bands\n", 0);

			(void)splx(s);

			/*
			 * Both bands at stream head.
			 * Get the marked message buffer,
			 * it is used to make I_ATMARK
			 * work properly and to generate SIGIO.
			 */
			if ((bp = _s_getloaned(q, mp, size)) == (mblk_t *)NULL)
				return (-1);

			/*
			 * _s_getloaned() has already incremented
			 * bp->b_wptr by "size".
			 */
			bp->b_datap->db_type = M_PROTO;
			*(long *)bp->b_rptr = T_EXDATA_IND;
			bp->b_flag |= MSGMARK;

SOCKLOG(so, "do_urg_outofline: sending up band 0\n", 0);

			putnext(q, bp);

			/*
			 * Now save away the OOB data.
			 */
			if (so->oob)
				freemsg(so->oob);
			so->oob = mp->b_cont;
			mp->b_cont = NULL;
			freeb(mp);

			s = splstr();
			so->hasoutofband++;
			(void)splx(s);

		} else	if (qp->q_first->b_band == 0 &&
					qp->q_last->b_band == 0) {

SOCKLOG(so, "do_urg_outofline: Only normal data\n", 0);

			(void)splx(s);

			/*
			 * Only normal data at
			 * stream head.
			 * Get the marked message buffer,
			 * it is used to make I_ATMARK
			 * work properly and to generate SIGIO.
			 */
			if ((bp = _s_getloaned(q, mp, size)) == (mblk_t *)NULL)
				return (-1);

			/*
			 * _s_getloaned() has already incremented
			 * bp->b_wptr by "size".
			 */
			bp->b_datap->db_type = M_PROTO;
			*(long *)bp->b_rptr = T_EXDATA_IND;
			bp->b_flag |= MSGMARK;

			/*
			 * Now save away the OOB data,
			 * and re-use mp for the band 1 message.
			 */
			if (so->oob)
				freemsg(so->oob);
			so->oob = mp->b_cont;
			mp->b_cont = NULL;

			mp->b_band = 1;
			mp->b_flag |= MSGNOGET;

SOCKLOG(so, "do_urg_outline: Sending up band 0, 1\n", 0);

			putnext(q, bp);
			putnext(q, mp);

			s = splstr();
			so->hasoutofband++;
			(void)splx(s);

		} else	if (qp->q_first->b_band == 1 &&
					qp->q_last->b_band == 1) {

SOCKLOG(so, "do_urg_outline: Just band 1 present\n", 0);

			(void)splx(s);

			/*
			 * Only band 1 at stream head -
			 * just save the OOB data.
			 */
			if (so->oob)
				freemsg(so->oob);
			so->oob = mp->b_cont;
			mp->b_cont = NULL;
			freeb(mp);
		}
	}
	return (0);
}

STATIC int
do_urg_inline(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register struct so_so	*so;
	register mblk_t		*bp;
	int			size;
	int			s;

	so = (struct so_so *)q->q_ptr;

SOCKLOG(so, "do_urg_inline: hasoutofband = %d\n", so->hasoutofband);

	/*
	 * Get the class 0 mblk which will
	 * hold the OOB data, and generate
	 * the SIGIO.
	 */
	size = msgdsize(mp);
	if ((bp = _s_getloaned(q, mp, size)) == (mblk_t *)NULL)
		return (-1);
	bp->b_flag |= MSGMARK;

SOCKLOG(so, "do_urg_inline: sending up band 0 msg\n", 0);

	putnext(q, bp);

	if (so->hasoutofband == 0) {
		/*
		 * Re-use mp for the band 1
		 * message. The contents were
		 * copied by _s_getloaned().
		 */
		freeb(mp->b_cont);
		mp->b_cont = (mblk_t *)NULL;
		mp->b_band = 1;
		mp->b_flag |= MSGNOGET;

SOCKLOG(so, "do_urg_inline: sending up band 1 msg\n", 0);

		putnext(q, mp);

	} else	freemsg(mp);

	s = splstr();
	so->hasoutofband++;
	(void)splx(s);

	return (0);
}

STATIC mblk_t *
_s_getband1(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	register mblk_t		*nmp;

	/*
	 * Get the band 1 message block.
	 * This will generate SIGURG and
	 * make select return TRUE on exception
	 * events. It is never actually seen
	 * by the socket library or the application.
	 */
	if ((nmp = _s_getmblk((mblk_t *)NULL, 1)) == (mblk_t *)NULL) {
		RECOVER(q, mp, 1);
		return ((mblk_t *)NULL);
	}

	nmp->b_datap->db_type = M_PROTO;
	nmp->b_band = 1;
	nmp->b_flag |= MSGNOGET;
	nmp->b_wptr += 1;

	return (nmp);
}

STATIC mblk_t *
do_esbzero(q, mp)
	register queue_t	*q;
	register mblk_t		*mp;
{
	char			*buf;
	struct free_ptr		*zero_ptr;
	mblk_t			*bp;
	int			size;
	struct so_so		*so;

	so = (struct so_so *)q->q_ptr;
SOCKLOG(so, "do_esbzero: Entered\n", 0);

	/*
	 * Allocate the M_ERROR message.
	 */
	if ((bp = allocb(2, BPRI_MED)) == (mblk_t *)NULL) {
		RECOVER(q, mp, 2);
		return ((mblk_t *)NULL);
	}

	/*
	 * Allocate the buffer.
	 */
	size = 1;
	if ((buf = (char *)kmem_alloc(size, KM_NOSLEEP)) == NULL) {
		RECOVER(q, mp, size);
		freeb(bp);
		return ((mblk_t *)NULL);
	}

	/*
	 * Allocate the data structure we need to
	 * be passed to the callback routine when
	 * the esballoc'ed message is freed.
	 */
	if ((zero_ptr = (struct free_ptr *)kmem_alloc(sizeof (*zero_ptr),
				KM_NOSLEEP)) == NULL) {
		RECOVER(q, mp, sizeof (*zero_ptr));
		kmem_free(buf, size);
		freeb(bp);
		return ((mblk_t *)NULL);
	}

	/*
	 * Initialize the free routine data structure,
	 * with the information it needs to do its
	 * tasks.
	 */
	zero_ptr->free_rtn.free_func = free_zero;
	zero_ptr->free_rtn.free_arg = (char *)zero_ptr;
	zero_ptr->buf = buf;
	zero_ptr->buflen = size;
	zero_ptr->rdq = q;
	zero_ptr->mp = bp;

	/*
	 * Need to do the following because a free
	 * routine could be called when the
	 * queue pointer is invalid.
	 */
	zero_ptr->so = so;

	/*
	 * Get a class 0 data block
	 * and attach our buffer.
	 */
	if ((bp = esballoc(buf, size, BPRI_MED, &zero_ptr->free_rtn)) ==
					(mblk_t *)NULL) {
		RECOVER(q, mp, sizeof (mblk_t));
		kmem_free(buf, size);
		freeb(zero_ptr->mp);
		kmem_free(zero_ptr, sizeof (*zero_ptr));

		return ((mblk_t *)NULL);
	}
	bp->b_wptr += size;

	so->esbcnt++;
	return (bp);
}

/*
 * This routine is called when a zero length
 * message, sent to the stream head because a
 * disconnect had been received on a stream with
 * read data pending, is freed. It sends up
 * an M_ERROR to close the stream and make
 * reads return the correct error.
 */
STATIC void
free_zero(arg)
	register char		*arg;
{
	register struct free_ptr	*zero_ptr;
	register struct so_so		*so;

	zero_ptr = (struct free_ptr *)arg;
	so = (struct so_so *)zero_ptr->so;
SOCKLOG(so, "free_zero: entered\n", 0);

	if ((so->flags & S_WCLOSE) == 0) {
SOCKLOG(so, "free_zero: calling do_ERROR\n", 0);
		do_ERROR(zero_ptr->rdq, zero_ptr->mp);
	}

	/*
	 * Free up the buffers.
	 */
	kmem_free(zero_ptr->buf, zero_ptr->buflen);
	kmem_free(zero_ptr, sizeof (*zero_ptr));

	so->esbcnt--;
	if ((so->flags & S_WCLOSE) && so->esbcnt == 0) {
SOCKLOG(so, "free_zero: clearing so_so\n", 0);
		so->flags = 0;
	}
	return;
}
