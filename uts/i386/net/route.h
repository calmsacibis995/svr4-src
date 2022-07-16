/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ifndef _NET_ROUTE_H
#define _NET_ROUTE_H

#ident	"@(#)kern-net:route.h	1.3"

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
 * Kernel resident routing tables.
 * 
 * The routing tables are initialized when interface addresses
 * are set by making entries for all directly connected interfaces.
 */

/*
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
#ifdef STRNET
	mblk_t	*ro_rt;
#else
	struct	rtentry *ro_rt;
#endif /* STRNET */
	struct	sockaddr ro_dst;
};

/*
 * We distinguish between routes to hosts and routes to networks,
 * preferring the former if available.  For each route we infer
 * the interface to use from the gateway address supplied when
 * the route was entered.  Routes that forward packets through
 * gateways are marked so that the output routines know to address the
 * gateway rather than the ultimate destination.
 */
struct rtentry {
	u_long	rt_hash;		/* to speed lookups */
	struct	sockaddr rt_dst;	/* key */
	struct	sockaddr rt_gateway;	/* value */
	short	rt_flags;		/* up/down?, host/net */
	short	rt_refcnt;		/* # held references */
	u_long	rt_use;			/* raw # packets forwarded */
#ifdef STRNET
	struct	ip_provider *rt_prov;	/* the answer: provider to use */
#else
	struct	ifnet *rt_ifp;		/* the answer: interface to use */
#endif /* STRNET */
};

#define	RTF_UP		0x1		/* route useable */
#define	RTF_GATEWAY	0x2		/* destination is a gateway */
#define	RTF_HOST	0x4		/* host entry (net otherwise) */
#define RTF_REINSTATE	0x8		/* re-instate route after timeout */
#define	RTF_DYNAMIC	0x10		/* created dynamically (by redirect) */
#define	RTF_MODIFIED	0x20		/* modified dynamically (by redirect) */

#define RTF_SWITCHED	0x40		/* this route must be switched */
#define RTF_SLAVE	0x80		/* slave switched route */
#define RTF_REMOTE	0x100		/* route usedfor forwarded packets */
#define RTF_TOSWITCH	0x200		/* gateway is switched route */

#define RTF_SSSTATE	0x700	/* switched SLIP state */
#define SSS_NOCONN	0x000	/* unconnected */
#define SSS_DIALING	0x100	/* switch in progress */
#define SSS_OPENWAIT	0x200	/* short delay */
#define SSS_INUSE	0x300	/* in use locally */
#define SSS_CLEARWAIT	0x400	/* delayed hangup */
#define SSS_CALLFAIL	0x500	/* call failed recently */

#define RTF_USERMASK	(RTF_GATEWAY|RTF_HOST|RTF_SWITCHED)
/* flags settable by user */
#define SSS_GETSTATE(rt) ((rt)->rt_flags & RTF_SSSTATE)
#define SSS_SETSTATE(rt, new) \
	(rt)->rt_flags = (((rt)->rt_flags & ~RTF_SSSTATE) | new)

/* flags passed to rtalloc */
#define SSF_SWITCH	0x001	/* do SLIP switching if needed */
#define SSF_REMOTE	0x002	/* rtalloc on behalf of remote system */
#define SSF_TOSWITCH	0x004	/* recursive call */

/* returns from rtalloc */
#define RT_OK	 0		/* normal route */
#define RT_DEFER 1		/* defer for SLIP dialing */
#define RT_SWITCHED 2		/* route is switched: to prevent caching */
#define RT_FAIL  3		/* for completeness; unused */

/*
 * Routing statistics.
 */
struct	rtstat {
	short	rts_badredirect;	/* bogus redirect calls */
	short	rts_dynamic;		/* routes created by redirects */
	short	rts_newgateway;		/* routes modified by redirects */
	short	rts_unreach;		/* lookups which failed */
	short	rts_wildcard;		/* lookups satisfied by a wildcard */
};

#ifdef _KERNEL

#ifdef STRNET
#define RT(x) ((struct rtentry *) (x)->b_rptr)
#define	RTFREE(rt) \
	if (RT(rt)->rt_refcnt == 1) \
		rtfree(rt); \
	else \
		RT(rt)->rt_refcnt--;
#else
#define	RTFREE(rt) \
	if ((rt)->rt_refcnt == 1) \
		rtfree(rt); \
	else \
		(rt)->rt_refcnt--;
#endif /* STRNET */


#ifdef	GATEWAY
#define	RTHASHSIZ	64
#else
#define	RTHASHSIZ	8
#endif	/* GATEWAY */
#if	(RTHASHSIZ & (RTHASHSIZ - 1)) == 0
#define RTHASHMOD(h)	((h) & (RTHASHSIZ - 1))
#else
#define RTHASHMOD(h)	((h) % RTHASHSIZ)
#endif

#ifdef	STRNET
mblk_t	*rthost[RTHASHSIZ];
mblk_t	*rtnet[RTHASHSIZ];
#else
struct	mbuf *rthost[RTHASHSIZ];
struct	mbuf *rtnet[RTHASHSIZ];
#endif	/* STRNET */

struct	rtstat	rtstat;
#endif	/* _KERNEL */
#endif	/* _NET_ROUTE_H */
