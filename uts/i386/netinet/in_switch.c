/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:in_switch.c	1.3"

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
 * routines for switched slip -- streams version 
 */

#define STRNET

#ifdef INET
#include <netinet/symredef.h>
#endif INET

#include <sys/types.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#ifdef SYSV
#include <sys/cred.h>
#include <sys/proc.h>
#endif /* SYSV */
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif SYSV

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/debug.h>

#include <sys/protosw.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/af.h>
#include <net/route.h>
#include <netinet/in_var.h>
#include <netinet/ip_str.h>

#define mtod(m,t) ((t)((m)->b_rptr))
/* #define ATOL(sa) (((struct sockaddr_in *)(sa))->sin_addr.s_addr) */
#define ATOL(sa) ((sa)->s_addr)

#define satosin(sa)	((struct sockaddr_in *) (sa))

#define SLIP_NODIAL	120	/* dial timeout */
#define SLIP_PEERWAIT	2	/* time to wait after call answer before
				 * sending first packet */
#define SLIP_CHECKWAIT	40	/* check connection after how long */

u_short          Slip_Hangwait = 300;	/* time to wait before hanging up */

 /* static */ struct {
	queue_t        *queue;
	mblk_t         *mp;
	mblk_t         *waiting;
}               Slreq;
mblk_t         *Sldeferred, *Slevents;
int             Slnumqueued = 0, Slnumdeferred = 0, Slnumevents = 0;
int             Slmaxqueued = 10, Slmaxdeferred = 40, Slmaxevents = 15;
int             Slchkqueued = 0, Slchkdeferred = 0, Slchkevents = 0;
extern struct ip_provider *loopprov;

/* static */ void ioc_ack(), ioc_error();

struct slevent {
	int             (*func) ();
	caddr_t         arg;
	unsigned        time;
};

struct defstruct {
	int             (*routine) ();
	unsigned        args[8];
};

/*
 * these routines are called out of the routing code 
 */
rtswitch(ro, flags)
	register struct route *ro;
{
	register struct rtentry *rt = RT(ro->ro_rt);
	void            rtnodial();


	if (rt->rt_flags & RTF_TOSWITCH) {
		struct route    route;

		if (flags & SSF_TOSWITCH) {
#ifdef SYSV
			cmn_err(CE_WARN,
				"rtswitch recursion, dest %x gateway %x\n",
				satosin(&rt->rt_dst)->sin_addr.s_addr, 
				satosin(&rt->rt_gateway)->sin_addr.s_addr);
#else
			printf ("rtswitch recursion, dest %x gateway %x\n",
				satosin(&rt->rt_dst)->sin_addr.s_addr, 
				satosin(&rt->rt_gateway)->sin_addr.s_addr);
#endif SYSV
			ro->ro_rt = 0;
			return (RT_FAIL);
		}
		bzero((caddr_t) & route, sizeof(route));
		route.ro_dst = rt->rt_gateway;
		rtalloc(&route, SSF_TOSWITCH);
		if (route.ro_rt) {
			rt = RT(ro->ro_rt);
			rtfree(route.ro_rt);
			if ((RT(route.ro_rt)->rt_flags & RTF_SWITCHED)
			    && !(RT(route.ro_rt)->rt_flags & RTF_TOSWITCH)) {
				ro->ro_rt = route.ro_rt;
			} else {
				ro->ro_rt = 0;
				return (RT_FAIL);
			}
		} else {
			ro->ro_rt = 0;
			return (RT_FAIL);
		}
	}
	if (flags & SSF_REMOTE)
		rt->rt_flags |= RTF_REMOTE;

	if (!(flags & SSF_SWITCH)) {
		switch (SSS_GETSTATE(rt)) {
		case SSS_NOCONN:
		case SSS_DIALING:	/* dialing already */
		case SSS_CALLFAIL:	/* clearing deferred stuff */
			rt->rt_refcnt++;
			return (RT_DEFER);
		case SSS_CLEARWAIT:	/* still have connection! */
			SSS_SETSTATE(rt, SSS_INUSE);	/* fall thru */
		case SSS_OPENWAIT:	/* already dialed but waiting */
		case SSS_INUSE:/* connected */
			rt->rt_refcnt++;
			return (RT_SWITCHED);
		default:
#ifdef SYSV
			cmn_err(CE_WARN, "rtswitch: bad state %x\n",
				rt->rt_flags);
#else
			printf ("rtswitch: bad state %x\n",
				rt->rt_flags);
#endif SYSV
			ro->ro_rt = 0;
			return (RT_FAIL);
		}
	}
	switch (SSS_GETSTATE(rt)) {
	case SSS_NOCONN:
		SSS_SETSTATE(rt, SSS_DIALING);
		sltimeout(rtnodial, (caddr_t) rt, SLIP_NODIAL);
		sldocall(&satosin(&rt->rt_dst)->sin_addr);
		rt->rt_refcnt++;
		return (RT_DEFER);
	case SSS_DIALING:	/* dialing already */
	case SSS_OPENWAIT:	/* already dialed but waiting */
		rt->rt_refcnt++;
		return (RT_DEFER);
	case SSS_INUSE:	/* connected */
		rt->rt_refcnt++;
		return (RT_SWITCHED);
	case SSS_CLEARWAIT:	/* still have connection! */
		SSS_SETSTATE(rt, SSS_INUSE);
		rt->rt_refcnt++;
		return (RT_SWITCHED);
	case SSS_CALLFAIL:	/* clearing deferred stuff */
		ro->ro_rt = 0;
		return (RT_FAIL);
	default:
#ifdef SYSV
		cmn_err(CE_WARN, "rtswitch: bad state %x\n", rt->rt_flags);
#else
		printf ("rtswitch: bad state %x\n", rt->rt_flags);
#endif SYSV
		ro->ro_rt = 0;
		return (RT_FAIL);
	}
}

rtunswitch(rt)
	register struct rtentry *rt;
{
	void            rthangup();

	switch (SSS_GETSTATE(rt)) {
	case SSS_INUSE:	/* connected */
	case SSS_OPENWAIT:	/* connected */
		SSS_SETSTATE(rt, SSS_CLEARWAIT);
		sltimeout(rthangup, (caddr_t) rt, Slip_Hangwait);
		break;
	case SSS_NOCONN:
	case SSS_DIALING:	/* dialing already */
	case SSS_CALLFAIL:	/* clearing deferred stuff */
	case SSS_CLEARWAIT:
		break;
	default:
#ifdef SYSV
		cmn_err(CE_WARN, "rtunswitch: bad state %x\n", rt->rt_flags);
#else
		printf ("rtunswitch: bad state %x\n", rt->rt_flags);
#endif SYSV
		break;
	}
}

/*
 * these routines are called at sltimein (pf_timeout) time 
 */
/*
 * done with active connection 
 */
 /* static */ void
rthangup(rt)
	register struct rtentry *rt;
{

	if (SSS_GETSTATE(rt) != SSS_CLEARWAIT) {
		return;		/* somebody wants it now */
	}
	SSS_SETSTATE(rt, SSS_NOCONN);
	slhangup(rt);		/* tell daemon to detach */
}

/*
 * call timed out (daemon died) 
 */
 /* static */ void
rtnodial(rt)
	register struct rtentry *rt;
{
	if (SSS_GETSTATE(rt) != SSS_DIALING)
		return;
	SSS_SETSTATE(rt, SSS_CALLFAIL);
	slsenddeferred();
	SSS_SETSTATE(rt, SSS_NOCONN);
}

 /* static */ void
slstartconn(rt)
	register struct rtentry *rt;
{
	void            slcheckuse();

	if (SSS_GETSTATE(rt) != SSS_OPENWAIT)
		return;
	SSS_SETSTATE(rt, SSS_INUSE);
	slsenddeferred();
	sltimeout(slcheckuse, (caddr_t) rt, SLIP_CHECKWAIT);
}

/*
 * make sure connection actually is used (ie user did not time out) 
 */
 /* static */ void
slcheckuse(rt)
	register struct rtentry *rt;
{
	void            rthangup();

	switch (SSS_GETSTATE(rt)) {
	case SSS_INUSE:
	case SSS_CLEARWAIT:
		break;

	default:
		return;
	}
	if (rt->rt_refcnt || (rt->rt_flags & RTF_REMOTE))
		return;
	SSS_SETSTATE(rt, SSS_CLEARWAIT);
	rthangup(rt);
}

/*
 * sldocall and slhangup queue requests for the daemon 
 */
/* static */
sldocall(addr)
	register struct in_addr *addr;
{
	struct sockaddr_in sock;
	register mblk_t *m, **mpp;
	register int    s;

	bzero((caddr_t) & sock, sizeof(struct sockaddr_in));
	sock.sin_family = AF_INET;
	sock.sin_addr = *addr;
	s = splstr();
	if (Slreq.mp) {
		((struct ifreq *) (Slreq.mp->b_cont->b_rptr))->ifr_addr =
			*(struct sockaddr *) & sock;
		ioc_ack(Slreq.queue, Slreq.mp);
		Slreq.mp = 0;
		splx(s);
		return;
	}
	if (Slnumqueued >= Slmaxqueued) {
		splx(s);
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: dropping request\n");
#else
		printf ("switched slip: dropping request\n");
#endif SYSV
		return;
	}
	if ((m = allocb(sizeof(struct sockaddr_in), BPRI_MED)) == NULL) {
		splx(s);
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: no space\n");
#else
		printf ("switched slip: no space\n");
#endif SYSV
		return;
	}
	m->b_wptr += sizeof(struct sockaddr_in);
	*(struct sockaddr_in *) (m->b_wptr) = sock;
	m->b_cont = 0;
	for (mpp = &Slreq.waiting; *mpp; mpp = &((*mpp)->b_cont));
	*mpp = m;
	Slnumqueued++;
	if (Slnumqueued > Slchkqueued)
		Slchkqueued = Slnumqueued;
	splx(s);
	return;
}

/*
 * restart sleeping daemon child or slave by sending ioctl ack 
 */
/* static */
slhangup(rt)
	register struct rtentry *rt;
{
	if (!(rt->rt_prov) || !(rt->rt_prov->unswitch)) {
#ifdef SYSV
		cmn_err(CE_WARN, "rtunswitch: null pointer");
#else
		printf ("rtunswitch: null pointer");
#endif SYSV
		return;
	}
	putnext(rt->rt_prov->qbot, rt->rt_prov->unswitch);
	rt->rt_prov->unswitch = 0;
	if (!(rt->rt_flags & RTF_SLAVE))
		rt->rt_prov = loopprov;
	return;
}

/*
 * called when slstat interrupted 
 */
swdetach(prov)
	struct ip_provider *prov;
{
	struct route    route;
	struct rtentry *rt;

	if (prov->unswitch == NULL)
		return;

	bzero((caddr_t)&route, sizeof(route));
	bcopy((caddr_t)&(prov->if_dstaddr), (caddr_t)&(route.ro_dst), 
	  sizeof(route.ro_dst));
	rtalloc(&route, 0);
	if (route.ro_rt) {
		rt = RT(route.ro_rt);
		rtfree(route.ro_rt);
		SSS_SETSTATE(rt, SSS_CLEARWAIT);
		rthangup(rt);
		if (!(rt->rt_flags & RTF_SLAVE))
			rt->rt_prov = loopprov;
	}
	prov->unswitch = NULL;
}

/*
 * get a connection request (SIOCSLGETREQ ioctl): daemon only 
 */
/* ARGSUSED */
slgetreq(wrq, mp, prov)
	queue_t        *wrq;
	mblk_t         *mp;
	struct ip_provider *prov;
{
	register struct ifreq *ifr = (struct ifreq *) (mp->b_cont->b_rptr);
	register mblk_t *mp1;
	register int    s;

	s = splstr();
	if (ifr->ifr_name[0]) {
		Slreq.mp = NULL;
		ioc_ack(wrq, mp);
	} else if (Slreq.mp) {
		ioc_error(wrq, mp, EBUSY);
	} else if (mp1 = Slreq.waiting) {
		ifr->ifr_addr = *(struct sockaddr *) (mp1->b_rptr);
		Slreq.waiting = mp1->b_cont;
		mp1->b_cont = 0;
		freemsg(mp1);
		Slnumqueued--;
		ioc_ack(wrq, mp);
	} else {
		Slreq.mp = mp;
		Slreq.queue = wrq;
	}
	splx(s);
	return;
}

/*
 * stat ioctl (SIOSLSTAT ioctl): inform kernel of call status 
 */
slstat(wrq, mp, prov)
	queue_t        *wrq;
	mblk_t         *mp;
	struct ip_provider *prov;
{
	register struct ifreq *ifr;
	struct route    route;
	register struct rtentry *rt;
	void            slstartconn();
	register int    s;

	ifr = (struct ifreq *) mp->b_cont->b_rptr;
	route.ro_dst = ifr->ifr_addr;
	route.ro_rt = 0;
	rtalloc(&route, 0);
	if (!route.ro_rt) {
		ioc_error(wrq, mp, ENOENT);
		return;
	}
	rt = RT(route.ro_rt);
	rtfree(route.ro_rt);
	s = splstr();
	if (prov == NULL) {
		if ((rt->rt_flags & RTF_SWITCHED)	/* call failed */
		    &&SSS_GETSTATE(rt) == SSS_DIALING) {
			rtnodial(rt);
			ioc_ack(wrq, mp);
		} else {	/* slave cant fail */
			ioc_error(wrq, mp, EINVAL);
		}
		return;
	}
	switch (SSS_GETSTATE(rt) | (rt->rt_flags & RTF_SWITCHED)) {
	case RTF_SWITCHED | SSS_DIALING:	/* master */
		break;
	case SSS_NOCONN:	/* 1-way slave */
		rt->rt_flags |= (RTF_SWITCHED | RTF_SLAVE);
	case RTF_SWITCHED | SSS_NOCONN:	/* 2-way slave */
		if (rt->rt_prov && rt->rt_prov != loopprov && rt->rt_prov != prov) {
			ioc_error(wrq, mp, EEXIST);
			return;
		}
		break;
	default:
		ioc_error(wrq, mp, EBADF);
		return;
	}
	rt->rt_prov = prov;
	SSS_SETSTATE(rt, SSS_OPENWAIT);
	sltimeout(slstartconn, rt, SLIP_PEERWAIT);
	prov->unswitch = mp;
	return;
}

/*
 * higher level routines call sldefer with packets to be retried 
 */
sldefer(routine, a0, a1, a2, a3, a4, a5, a6, a7)
	int             (*routine) ();
{
	register mblk_t *m;
	struct defstruct *dp;
	register mblk_t **mpp;
	register int    s;

	if (Slnumdeferred >= Slmaxdeferred) {
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: dropping deferral\n");
#else
		printf ("switched slip: dropping deferral\n");
#endif SYSV
		return;
	}
	if ((m = allocb(sizeof(struct defstruct), BPRI_MED)) == NULL) {
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: no space\n");
#else
		printf ("switched slip: no space\n");
#endif SYSV
		return;
	}
	m->b_wptr += sizeof(struct defstruct);
	m->b_cont = 0;
	dp = mtod(m, struct defstruct *);
	dp->routine = routine;
	dp->args[0] = a0;
	dp->args[1] = a1;
	dp->args[2] = a2;
	dp->args[3] = a3;
	dp->args[4] = a4;
	dp->args[5] = a5;
	dp->args[6] = a6;
	dp->args[7] = a7;
	s = splstr();
	for (mpp = &Sldeferred; *mpp; mpp = &((*mpp)->b_cont));
	*mpp = m;
	Slnumdeferred++;
	if (Slnumdeferred > Slchkdeferred)
		Slchkdeferred = Slnumdeferred;
	splx(s);
}

/* static */
slsenddeferred()
{
	register mblk_t *m, *m0;
	register struct defstruct *dp;
	register int    i;
	register int    s;

	s = splstr();
	m = Sldeferred;
	Sldeferred = 0;
	Slnumdeferred = 0;
	splx(s);
	while (m) {
		dp = mtod(m, struct defstruct *);
		(*(dp->routine)) (dp->args[0], dp->args[1], dp->args[2],
				  dp->args[3], dp->args[4], dp->args[5],
				  dp->args[6], dp->args[7]);
#ifdef undef
		if (dp->freemask)
			for (i = 0; i < 8; i++)
				if (dp->freemask & 1 << i)
					freemsg(dp->args[i]);
#endif
		m0 = m->b_cont;
		m->b_cont = 0;
		freemsg(m);
		m = m0;
	}
}

/*
 * timeout that heeds the network semaphore and allows only one call with
 * given func and arg to be queued 
 */
/* static */
sltimeout(func, arg, secs)
	int             (*func) ();
caddr_t         arg;
{
	register mblk_t *m;
	register struct slevent *sep;
	register int    s;

	s = splstr();
	for (m = Slevents; m; m = m->b_cont) {
		sep = mtod(m, struct slevent *);
		if (sep->func == func && sep->arg == arg) {
			sep->time = lbolt + secs * HZ;
			splx(s);
			return;
		}
	}
	if (Slnumevents >= Slmaxevents) {
		splx(s);
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: dropping timeout\n");
#else
		printf ("switched slip: dropping timeout\n");
#endif SYSV
		return;
	}
	if ((m = allocb(sizeof(struct slevent), BPRI_MED)) == NULL) {
		splx(s);
#ifdef SYSV
		cmn_err(CE_WARN, "switched slip: no space\n");
#else
		printf ("switched slip: no space\n");
#endif SYSV
		return;
	}
	m->b_wptr += sizeof *sep;
	sep = mtod(m, struct slevent *);
	sep->time = lbolt + secs * HZ;
	sep->func = func;
	sep->arg = arg;
	m->b_cont = Slevents;
	Slevents = m;
	Slnumevents++;
	if (Slnumevents > Slchkevents)
		Slchkevents = Slnumevents;
	splx(s);
}

/*
 * called from ip_slowtimo to process slip timeouts 
 */
sltimein()
{
	register mblk_t *m, *mnext, **mprev = &Slevents;
	register struct slevent *sep;

	for (m = Slevents; m; m = mnext) {
		mnext = m->b_cont;
		sep = mtod(m, struct slevent *);
		if (sep->time > lbolt) {
			mprev = &(m->b_cont);
			continue;
		}
		*mprev = mnext;
		(*(sep->func)) (sep->arg);
		m->b_cont = 0;
		freemsg(m);
		Slnumevents--;
	}
}

 /* static */ void
ioc_error(wrq, mp, errno)
	register queue_t *wrq;
	register mblk_t *mp;
{
	register struct iocblk *iocpb = (struct iocblk *) mp->b_rptr;

	iocpb->ioc_error = errno;
	mp->b_datap->db_type = M_IOCNAK;
	qreply(wrq, mp);
}

 /* static */ void
ioc_ack(wrq, mp)
	register queue_t *wrq;
	register mblk_t *mp;
{
	mp->b_datap->db_type = M_IOCACK;
	qreply(wrq, mp);
}
