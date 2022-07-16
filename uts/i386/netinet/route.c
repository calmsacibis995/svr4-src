/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:route.c	1.3"

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
#include <sys/systm.h>
#include <sys/stream.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/errno.h>

#include <sys/stropts.h>
#include <sys/strlog.h>
#include <sys/log.h>
#include <net/if.h>
#include <net/af.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <net/route.h>
#ifdef SYSV
#include <sys/cmn_err.h>
#endif SYSV

#include <netinet/ip_str.h>

int             rttrash;	/* routes not in table but not freed */
struct in_addr  wildcard;	/* zero valued cookie for wildcard searches */
int             rthashsize = RTHASHSIZ;	/* for netstat, etc. */
int             inet_netmatch();
struct rtstat   rtstat;

extern struct ip_provider *
prov_withaddr(), *prov_withnet(), *loopprov;

#define satosin(sa)	((struct sockaddr_in *) (sa))

/*
 * Packet routing routines. 
 */
rtalloc(ro, switchflag)
	register struct route *ro;
{
	register struct rtentry *rt;
	register mblk_t *bp;
	register u_long hash;
	struct in_addr  dst;
	int             (*match) (), doinghost;
	struct afhash   h;
	mblk_t        **table;

	STRLOG(IPM_ID, 0, 9, SL_TRACE, "rtalloc to %x", 
	       satosin(&ro->ro_dst)->sin_addr.s_addr);
	dst.s_addr = satosin(&ro->ro_dst)->sin_addr.s_addr;
	if (ro->ro_rt && RT(ro->ro_rt)->rt_prov
	    && (RT(ro->ro_rt)->rt_flags & RTF_UP))
		return (RT_OK);	/* XXX */
	inet_hash(dst, &h);
	match = inet_netmatch;
	hash = h.afh_hosthash;
	table = rthost;
	doinghost = 1;
again:
	for (bp = table[RTHASHMOD(hash)]; bp; bp = bp->b_cont) {
		rt = RT(bp);
		if (rt->rt_hash != hash)
			continue;
		if (rt->rt_prov == 0) {
#ifdef u3b2
			cmn_err(CE_WARN, "rtalloc: null prov");
#else
			printf ("rtalloc: null prov");
#endif
			continue;
		}
		if ((rt->rt_flags & RTF_UP) == 0 ||
		    (rt->rt_prov->if_flags & IFF_UP) == 0)
			continue;
		if (doinghost) {
			if (satosin(&rt->rt_dst)->sin_addr.s_addr != dst.s_addr)
				continue;
		} else {
			if (!(*match) (satosin(&rt->rt_dst)->sin_addr, dst))
				continue;
		}
		if (dst.s_addr == wildcard.s_addr)
			rtstat.rts_wildcard++;
		ro->ro_rt = bp;
		if (rt->rt_flags & RTF_SWITCHED)
			return (rtswitch(ro, switchflag));	/* switched SLIP */
		rt->rt_refcnt++;
		return (RT_OK);
	}
	if (doinghost) {
		doinghost = 0;
		hash = h.afh_nethash, table = rtnet;
		goto again;
	}
	/*
	 * Check for wildcard gateway, by convention network 0. 
	 */
	if (dst.s_addr != wildcard.s_addr) {
		dst.s_addr = wildcard.s_addr, hash = 0;
		goto again;
	}
	rtstat.rts_unreach++;
	return (RT_FAIL);
}

rtfree(bp)
	mblk_t         *bp;
{
	struct rtentry *rt = RT(bp);

	if (rt == 0)
#ifdef u3b2
		cmn_err(CE_PANIC, "rtfree");
#else
		panic ("rtfree");
#endif
	rt->rt_refcnt--;
	if (rt->rt_refcnt == 0) {
		if (rt->rt_flags & RTF_SWITCHED)
			rtunswitch(rt);	/* switched SLIP */
		else if ((rt->rt_flags & RTF_UP) == 0) {
			rttrash--;
			(void) freeb(bp);
		}
	}
}


/*
 * Force a routing table entry to the specified destination to go through the
 * given gateway. Normally called as a result of a routing redirect message
 * from the network layer. 
 *
 */
rtredirect(dst, gateway, flags, src)
	struct in_addr  dst, gateway, src;
	int             flags;
{
	struct route    ro;
	register struct rtentry *rt;
	register mblk_t *bp;

	/* verify the gateway is directly reachable */
	if (prov_withnet(gateway) == 0) {
		rtstat.rts_badredirect++;
		return;
	}
	satosin(&ro.ro_dst)->sin_addr.s_addr = dst.s_addr;
	ro.ro_rt = 0;
	rtalloc(&ro, 0);	/* no dialing */
	bp = ro.ro_rt;
	rt = RT(bp);

	/*
	 * If the redirect isn't from our current router for this dst, it's
	 * either old or wrong.  If it redirects us to ourselves, we have a
	 * routing loop, perhaps as a result of an interface going down
	 * recently. 
	 */
	if ((bp && src.s_addr != satosin(&rt->rt_gateway)->sin_addr.s_addr)
	    || prov_withaddr(gateway)) {
		rtstat.rts_badredirect++;
		if (bp)
			rtfree(bp);
		return;
	}
	/*
	 * Create a new entry if we just got back a wildcard entry or the the
	 * lookup failed.  This is necessary for hosts which use routing
	 * redirects generated by smart gateways to dynamically build the
	 * routing tables. 
	 */
	if (bp &&
	    inet_netmatch(wildcard, satosin(&rt->rt_dst)->sin_addr)) {
		rtfree(bp);
		bp = 0;
	}
	if (bp == 0) {
		rtinit(dst, gateway, (int) SIOCADDRT,
		       (flags & RTF_HOST) | RTF_GATEWAY | RTF_DYNAMIC);
		rtstat.rts_dynamic++;
		return;
	}
	/*
	 * Don't listen to the redirect if it's for a route to an interface. 
	 */
	if (rt->rt_flags & RTF_GATEWAY) {
		if (((rt->rt_flags & RTF_HOST) == 0) && (flags & RTF_HOST)) {
			/*
			 * Changing from route to net => route to host.
			 * Create new route, rather than smashing route to
			 * net. 
			 */
			rtinit(dst, gateway, (int) SIOCADDRT,
			       flags | RTF_DYNAMIC);
			rtstat.rts_dynamic++;
		} else {
			/*
			 * Smash the current notion of the gateway to this
			 * destination. 
			 */
			satosin(&rt->rt_gateway)->sin_addr = gateway;
		}
		rt->rt_flags |= RTF_MODIFIED;
		rtstat.rts_newgateway++;
	} else
		rtstat.rts_badredirect++;
	rtfree(bp);
}

/*
 * Routing table ioctl interface. 
 */
rtioctl(cmd, data)
	int             cmd;
	mblk_t         *data;
{

	if (cmd != SIOCADDRT && cmd != SIOCDELRT)
		return (EINVAL);
	return (rtrequest(cmd, data));
}

/*
 * Carry out a request to change the routing table.  Called by interfaces at
 * boot time to make their ``local routes'' known, for ioctl's, and as the
 * result of routing redirects. 
 */
rtrequest(req, bp)
	mblk_t         *bp;
	int             req;
{
	register struct rtentry *entry = RT(bp);
	register mblk_t *m, **mprev;
	mblk_t        **mfirst;
	register struct rtentry *rt;
	struct afhash   h;
	int             error = 0;
	u_long          hash;
	struct ip_provider *prov = 0, *prov_withdstaddr();
	extern int      in_interfaces;

	inet_hash(satosin(&entry->rt_dst)->sin_addr, &h);
	if (entry->rt_flags & RTF_HOST) {
		hash = h.afh_hosthash;
		mprev = &rthost[RTHASHMOD(hash)];
	} else {
		hash = h.afh_nethash;
		mprev = &rtnet[RTHASHMOD(hash)];
	}
	for (mfirst = mprev; m = *mprev; mprev = &m->b_cont) {
		rt = RT(m);
		if (rt->rt_hash != hash)
			continue;
		if (entry->rt_flags & RTF_HOST) {
			if (satosin(&rt->rt_dst)->sin_addr.s_addr != 
			    satosin(&entry->rt_dst)->sin_addr.s_addr)
				continue;
		} else {
			if (inet_netmatch(satosin(&rt->rt_dst)->sin_addr, 
					  satosin(&entry->rt_dst)->sin_addr) 
			    == 0)
				continue;
		}
		if (satosin(&rt->rt_gateway)->sin_addr.s_addr == 
		    satosin(&entry->rt_gateway)->sin_addr.s_addr)
			break;
	}
	switch (req) {

	case SIOCDELRT:
		if (m == 0) {
			error = ESRCH;
			goto bad;
		}
		/* dont inadvertently delete switched routes */
		if (rt->rt_flags & RTF_SWITCHED
		    && !(rt->rt_flags & (RTF_SLAVE | RTF_TOSWITCH))) {
			if (!(entry->rt_flags & RTF_SWITCHED)) {
				error = EINVAL;
				goto bad;
			}
			in_interfaces--;
		}
		*mprev = m->b_cont;
		if (rt->rt_refcnt > 0) {
			rt->rt_flags &= ~RTF_UP;
			rttrash++;
			m->b_cont = 0;
		} else
			(void) freeb(m);
		break;

	case SIOCADDRT:
		if (m) {
			error = EEXIST;
			goto bad;
		}
		/* first check for switched route, and use local as dummy */
		if (entry->rt_flags & RTF_SWITCHED) {
			prov = loopprov;
			in_interfaces++;
		}
		/*
		 * If we are adding a route to an interface, and the
		 * interface is a pt to pt link we should search for the
		 * destination as our clue to the interface.  Otherwise we
		 * can use the local address (below). 
		 */
		if (prov == 0 && (entry->rt_flags & RTF_GATEWAY) == 0
		    && (entry->rt_flags & RTF_HOST))
			prov = prov_withdstaddr(satosin(&entry->rt_dst)->sin_addr);
		if (prov == 0 && (entry->rt_flags & RTF_GATEWAY) == 0)
			prov = prov_withaddr(satosin(&entry->rt_gateway)->sin_addr);
		if (prov == 0)
			prov = prov_withnet(satosin(&entry->rt_gateway)->sin_addr);
		if (prov == 0) {
			error = ENETUNREACH;
			goto bad;
		}
		m = allocb(sizeof(struct rtentry), BPRI_MED);
		if (m == 0) {
			error = ENOSR;
			goto bad;
		}
		m->b_cont = *mfirst;
		*mfirst = m;
		m->b_wptr += sizeof(struct rtentry);
		rt = RT(m);
		rt->rt_hash = hash;
		rt->rt_dst = entry->rt_dst;
		rt->rt_gateway = entry->rt_gateway;
		rt->rt_flags = RTF_UP |
			(entry->rt_flags & (RTF_USERMASK | RTF_TOSWITCH | RTF_DYNAMIC));
		rt->rt_refcnt = 0;
		rt->rt_use = 0;
		rt->rt_prov = prov;
		break;
	default:
		error = EINVAL;
		break;
	}
bad:
	return (error);
}

/*
 * Set up a routing table entry, normally for an interface. 
 */
rtinit(dst, gateway, cmd, flags)
	struct in_addr  dst, gateway;
	int             cmd, flags;
{
	mblk_t         *bp;
	struct rtentry *route;

	bp = allocb(sizeof(struct rtentry), BPRI_HI);
	if (bp == 0) {
		return;
	}
	bp->b_wptr += sizeof(struct rtentry);
	route = RT(bp);
	bzero((caddr_t) route, sizeof(route));
	satosin(&route->rt_dst)->sin_addr = dst;
	satosin(&route->rt_gateway)->sin_addr = gateway;
	route->rt_flags = flags;
	(void) rtrequest(cmd, bp);
	(void) freeb(bp);
}

/*
 * flush all routing table entries for an interface being detached 
 */
rtdetach(prov)
	struct ip_provider *prov;
{
	register mblk_t *m, *mnext;
	register mblk_t **table;
	register ushort i, j;

	for (j = 0, table = rthost; j < 2; j++, table = rtnet) {
		for (i = 0; i < RTHASHSIZ; i++) {
			if (table[i] == NULL)
				continue;
			for (mnext = table[i]; m = mnext;) {
				mnext = m->b_cont;
				if (RT(m)->rt_prov == prov)
					(void) rtrequest(SIOCDELRT, m);
			}
		}
	}
}

