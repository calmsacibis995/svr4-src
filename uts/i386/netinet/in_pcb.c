/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:in_pcb.c	1.3.1.1"

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
#include <sys/strlog.h>
#include <sys/log.h>
#include <sys/errno.h>
#ifdef SYSV
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#else SYSV
#include <sys/nettli/tihdr.h>
#include <sys/nettli/tiuser.h>
#endif SYSV
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <net/route.h>
#include <netinet/in_pcb.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <sys/protosw.h>
#include <netinet/ip_str.h>
#include <netinet/insrem.h>
#include <sys/kmem.h>

struct in_addr  zeroin_addr;
/*extern void     bcopy();*/

extern struct ip_provider *prov_withaddr(), *prov_withdstaddr();
extern struct ip_provider *in_onnetof();

extern struct ip_provider provider[];
extern struct ip_provider *lastprov;

#define satosin(sa)	((struct sockaddr_in *) (sa))

struct inpcb   *
in_pcballoc(head)
	struct inpcb   *head;
{
	register struct inpcb *inp;

	inp = (struct inpcb *) kmem_alloc(sizeof(struct inpcb), KM_NOSLEEP);
	if (inp == NULL) {
		return ((struct inpcb *) NULL);
	}
	bzero((caddr_t) inp, sizeof(struct inpcb));
	inp->inp_head = head;
	inp->inp_q = NULL;
	inp->inp_addrlen = sizeof(struct sockaddr_in);
	inp->inp_family = AF_INET;

	insque((struct vq *) inp, (struct vq *) head);
	return (inp);
}

in_pcbbind(inp, nam)
	register struct inpcb *inp;
	mblk_t         *nam;
{
	register struct inpcb *head = inp->inp_head;
	register struct sockaddr_in *sin;
	u_short         lport = 0;
	int		len;

	if (inp->inp_lport || inp->inp_laddr.s_addr != INADDR_ANY)
		return (EINVAL);
	if (nam == 0) {
		STRLOG(IPM_ID, 1, 5, SL_TRACE, "null in_pcbbind");
		goto noname;
	}
	sin = (struct sockaddr_in *) nam->b_rptr;
	len = nam->b_wptr - nam->b_rptr;
	if (!in_chkaddrlen(len))
		return (EINVAL);
	inp->inp_addrlen = len;
	inp->inp_family = sin->sin_family;
	STRLOG(IPM_ID, 1, 6, SL_TRACE, "in_pcbbind port %d addr %x",
	       sin->sin_port, sin->sin_addr.s_addr);
	if (sin->sin_addr.s_addr != INADDR_ANY
	    && !prov_withaddr(sin->sin_addr)) {
		if (inp->inp_protoopt & SO_IMASOCKET)
			return (EADDRNOTAVAIL);
		else
			sin->sin_addr.s_addr = INADDR_ANY;
	}
	lport = sin->sin_port;
	if (lport) {
		u_short         aport = ntohs(lport);
		int             wild = 0;

		/* GROSS */
		if (aport < IPPORT_RESERVED && (inp->inp_state & SS_PRIV) == 0)
			return (EACCES);
		/* even GROSSER, but this is the Internet */
		if ((inp->inp_protoopt & SO_REUSEADDR) == 0 &&
		    ((inp->inp_protodef & PR_CONNREQUIRED) == 0 ||
		     (inp->inp_protoopt & SO_ACCEPTCONN) == 0))
			wild = INPLOOKUP_WILDCARD;
		if (in_pcblookup(head,
			      zeroin_addr, 0, sin->sin_addr, lport, wild)) {
			if (inp->inp_protoopt & SO_IMASOCKET)
				return (EADDRINUSE);
			else
				lport = 0;
		}
	}
	inp->inp_laddr = sin->sin_addr;
noname:
	if (lport == 0)
		do {
			if (head->inp_lport++ < IPPORT_RESERVED ||
			    head->inp_lport > IPPORT_USERRESERVED)
				head->inp_lport = IPPORT_RESERVED;
			lport = htons(head->inp_lport);
		} while (in_pcblookup(head,
				      zeroin_addr, 0, inp->inp_laddr,
				      lport, INPLOOKUP_WILDCARD));
	inp->inp_lport = lport;
	return (0);
}

/*
 * Connect from a socket to a specified address. Both address and port must
 * be specified in argument sin. If don't have a local address for this
 * socket yet, then pick one.
 */
in_pcbconnect(inp, nam)
	struct inpcb   *inp;
	mblk_t         *nam;
{
	register struct ip_provider *prov;
	struct ip_provider *first_prov = (struct ip_provider *) NULL, tempprov;
	register struct sockaddr_in *sin = (struct sockaddr_in *) nam->b_rptr;
	int	len;

	len = nam->b_wptr - nam->b_rptr;
	if (!in_chkaddrlen(len))
		return (EINVAL);
	if (sin->sin_family != AF_INET && sin->sin_family != htons(AF_INET))
		return (EAFNOSUPPORT);
	if (sin->sin_port == 0)
		return (EADDRNOTAVAIL);
	inp->inp_addrlen = len;
	inp->inp_family = sin->sin_family;

	/*
	 * If destination is INADDR_ANY, find loopback.
	 */
	if (sin->sin_addr.s_addr == INADDR_ANY) {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot != NULL
			  && (prov->if_flags & (IFF_UP | IFF_LOOPBACK))
			     == (IFF_UP | IFF_LOOPBACK)) {
				first_prov = prov;
				break;
			}
		}
	}
	if (first_prov == NULL) {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot != NULL && (prov->if_flags & IFF_UP)
			  && !(prov->if_flags & (IFF_LOOPBACK | IFF_POINTOPOINT))) {
				first_prov = prov;
				break;
			}
		}
	}
	if (first_prov == NULL) {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot != NULL && (prov->if_flags & IFF_UP)
			    && !(prov->if_flags & IFF_LOOPBACK)) {
				first_prov = prov;
				break;
			}
		}
	}
	if (first_prov) {
		/*
		 * If the destination address is INADDR_ANY, use loopback
		 * or primary local address.  If the supplied address is
		 * INADDR_BROADCAST, and the primary interface supports
		 * broadcast, choose the broadcast address for that
		 * interface.
		 */
		if (sin->sin_addr.s_addr == INADDR_ANY)
			sin->sin_addr = *PROV_INADDR(first_prov);
		else if (sin->sin_addr.s_addr == (u_long) INADDR_BROADCAST &&
			 (first_prov->if_flags & IFF_BROADCAST))
			sin->sin_addr = *SOCK_INADDR(&first_prov->if_broadaddr);
	}
	if (inp->inp_laddr.s_addr == INADDR_ANY) {
		register struct route *ro;

		prov = (struct ip_provider *) 0;
		/*
		 * If route is known or can be allocated now, our src addr is
		 * taken from the i/f, else punt.
		 */
		ro = &inp->inp_route;
		if (ro->ro_rt &&
		    (satosin(&ro->ro_dst)->sin_addr.s_addr != 
		     sin->sin_addr.s_addr ||
		     inp->inp_protoopt & SO_DONTROUTE)) {
			RTFREE(ro->ro_rt);
			ro->ro_rt = (mblk_t *) 0;
		}
		if ((inp->inp_protoopt & SO_DONTROUTE) == 0 &&	/* XXX */
		    (ro->ro_rt == (mblk_t *) 0
		   || RT(ro->ro_rt)->rt_prov == (struct ip_provider *) 0)) {
			/* No route yet, so try to acquire one */
			satosin(&ro->ro_dst)->sin_addr = sin->sin_addr;
			if (rtalloc(ro, SSF_SWITCH) == RT_DEFER) {
				*PROV_INADDR(&tempprov) =
					satosin (&(RT(ro->ro_rt)->rt_gateway))->sin_addr;
				prov = &tempprov;	/* switched: gateway is
							 * laddr */
				RTFREE(ro->ro_rt);
				ro->ro_rt = 0;	/* don't cache till complete */
			}
		}
		/*
		 * If we found a route, use the address corresponding to the
		 * outgoing interface unless it is the loopback (in case a
		 * route to our address on another net goes to loopback).
		 */
		if (ro->ro_rt && (prov = RT(ro->ro_rt)->rt_prov) &&
		    (prov->if_flags & IFF_LOOPBACK))
			prov = 0;
		if (prov == 0) {
			prov = prov_withdstaddr(sin->sin_addr);
			if (prov == 0)
				prov = in_onnetof(in_netof(sin->sin_addr));
			if (prov == 0)
				prov = first_prov;
			if (prov == 0)
				return (EADDRNOTAVAIL);
		}
	}
	if (in_pcblookup(inp->inp_head,
			 sin->sin_addr,
			 sin->sin_port,
		inp->inp_laddr.s_addr ? inp->inp_laddr : *PROV_INADDR(prov),
			 inp->inp_lport,
			 0))
		return (EADDRINUSE);
	if (inp->inp_laddr.s_addr == INADDR_ANY && inp->inp_lport != 0) {
		inp->inp_laddr = *PROV_INADDR(prov);
	}
	if (inp->inp_laddr.s_addr == INADDR_ANY) {
		mblk_t         *bp = allocb(inp->inp_addrlen, BPRI_HI);
		struct sockaddr_in *sin1 = (struct sockaddr_in *) bp->b_rptr;
		int             error;

		if (bp == (mblk_t *) NULL) {
			return (ENOSR);
		}
		bp->b_wptr += inp->inp_addrlen;
		sin1->sin_family = inp->inp_family;
		sin1->sin_addr = *PROV_INADDR(prov);
		sin1->sin_port = inp->inp_lport;
		inp->inp_lport = 0;
		if (error = in_pcbbind(inp, bp)) {
			freeb(bp);
			return (error);
		}
		freeb(bp);
	}
	inp->inp_faddr = sin->sin_addr;
	inp->inp_fport = sin->sin_port;
	return (0);
}

in_pcbdisconnect(inp)
	struct inpcb   *inp;
{

	inp->inp_faddr.s_addr = INADDR_ANY;
	inp->inp_fport = 0;
	if (inp->inp_state & SS_NOFDREF)
		in_pcbdetach(inp);
}

in_pcbdetach(inp)
	struct inpcb   *inp;
{
	STRLOG(IPM_ID, 1, 4, SL_TRACE, "in_pcbdetach wq %x pcb %x",
	       inp->inp_q ? WR(inp->inp_q) : 0, inp);
	if (inp->inp_options)
		(void) freeb(inp->inp_options);
	if (inp->inp_route.ro_rt)
		rtfree(inp->inp_route.ro_rt);
	remque((struct vq *) inp);
	if (inp->inp_q) {
		inp->inp_q->q_ptr = (char *) NULL;
	}
	(void) kmem_free(inp, sizeof(struct inpcb));
}

in_setsockaddr(inp, nam)
	register struct inpcb *inp;
	mblk_t         *nam;
{
	register struct sockaddr_in *sin = (struct sockaddr_in *) nam->b_rptr;

	nam->b_wptr = nam->b_rptr + inp->inp_addrlen;
	bzero((caddr_t) sin, inp->inp_addrlen);
	sin->sin_family = inp->inp_family;
	sin->sin_port = inp->inp_lport;
	sin->sin_addr = inp->inp_laddr;
}

in_setpeeraddr(inp, nam)
	register struct inpcb *inp;
	mblk_t         *nam;
{
	register struct sockaddr_in *sin = (struct sockaddr_in *) nam->b_rptr;

	nam->b_wptr = nam->b_rptr + inp->inp_addrlen;
	bzero((caddr_t) sin, inp->inp_addrlen);
	sin->sin_family = inp->inp_family;
	sin->sin_port = inp->inp_fport;
	sin->sin_addr = inp->inp_faddr;
}

/*
 * Pass some notification to all connections of a protocol associated with
 * address dst.  Call the protocol specific routine (if any) to handle each
 * connection.
 * If portmatch arg is set, only pass to matching port.  Prevents denial of
 * service attacks via bogus icmp error messages.
 */
in_pcbnotify(head, src, dst, errno, notify, portmatch)
	struct inpcb   *head;
	register struct sockaddr_in *src, *dst;
	int             errno, (*notify) (), portmatch;
{
	register struct inpcb *inp, *oinp;

	STRLOG(IPM_ID, 3, 4, SL_TRACE,
	     "in_pcbnotify: sending error %d to pcbs from %x", errno, head);

	for (inp = head->inp_next; inp != head;) {
		if (inp->inp_faddr.s_addr != dst->sin_addr.s_addr) {
			inp = inp->inp_next;
			continue;
		}
		if ( portmatch ) {
		    if ( inp->inp_fport != dst->sin_port ) {
			inp = inp->inp_next;
			continue;
		    }
		    if ( src && inp->inp_lport != src->sin_port ) {
			inp = inp->inp_next;
			continue;
		    }
		}
		if (errno)
			inp->inp_error = errno;
		oinp = inp;
		inp = inp->inp_next;
		if (notify)
			(*notify) (oinp);
	}
}

/*
 * Check for alternatives when higher level complains about service problems.
 * For now, invalidate cached routing information.  If the route was created
 * dynamically (by a redirect), time to try a default gateway again.
 */
in_losing(inp)
	struct inpcb   *inp;
{
	register mblk_t *rt;

	if ((rt = inp->inp_route.ro_rt)) {
		if (RT(rt)->rt_flags & RTF_DYNAMIC)
			(void) rtrequest((int) SIOCDELRT, rt);
		rtfree(rt);
		inp->inp_route.ro_rt = 0;
		/*
		 * A new route can be allocated the next time output is
		 * attempted.
		 */
	}
}

/*
 * After a routing change, flush old routing and allocate a (hopefully)
 * better one.
 */
in_rtchange(inp)
	register struct inpcb *inp;
{
	if (inp->inp_route.ro_rt) {
		rtfree(inp->inp_route.ro_rt);
		inp->inp_route.ro_rt = 0;
		/*
		 * A new route can be allocated the next time output is
		 * attempted.
		 */
	}
}

struct inpcb   *
in_pcblookup(head, faddr, fport, laddr, lport, flags)
	struct inpcb   *head;
	struct in_addr  faddr, laddr;
	unsigned short  fport, lport;
	int             flags;
{
	register struct inpcb *inp, *match = 0;
	int             matchwild = 3, wildcard;
	register int    s;

	s = splstr();
	for (inp = head->inp_next; inp != head; inp = inp->inp_next) {
		if (inp->inp_lport != lport)
			continue;
		wildcard = 0;
		if (inp->inp_laddr.s_addr != INADDR_ANY) {
			if (laddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->inp_laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if (laddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			if (faddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->inp_faddr.s_addr != faddr.s_addr ||
				 inp->inp_fport != fport)
				continue;
		} else {
			if (faddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		if (wildcard && (flags & INPLOOKUP_WILDCARD) == 0)
			continue;
		if (wildcard < matchwild) {
			match = inp;
			matchwild = wildcard;
			if (matchwild == 0)
				break;
		}
	}
	splx(s);
	return (match);
}

/*
 * the following defines default values for most socket options
 */
static int      opt_on = 1;
static int      opt_off = 0;
static struct linger lingerdef = {0, 0};
static struct optdefault sockdefault[] = {
	SO_LINGER, (char *) &lingerdef, sizeof(struct linger),
	SO_DEBUG, (char *) &opt_off, sizeof(int),
	SO_KEEPALIVE, (char *) &opt_off, sizeof(int),
	SO_DONTROUTE, (char *) &opt_off, sizeof(int),
	SO_USELOOPBACK, (char *) &opt_off, sizeof(int),
	SO_BROADCAST, (char *) &opt_off, sizeof(int),
	SO_REUSEADDR, (char *) &opt_off, sizeof(int),
	SO_OOBINLINE, (char *) &opt_off, sizeof(int),
	SO_IMASOCKET, (char *) &opt_off, sizeof(int),
	/* defaults for these have to be taken from elsewhere */
	SO_SNDBUF, (char *) 0, sizeof(int),
	SO_RCVBUF, (char *) 0, sizeof(int),
	SO_SNDLOWAT, (char *) 0, sizeof(int),
	SO_RCVLOWAT, (char *) 0, sizeof(int),
	0, (char *) 0, 0
};

/*
 * in_pcboptmgmt handles "socket" level options management The return value
 * is 0 if ok, or a T-error, or a negative E-error. Note that if T_CHECK sets
 * T_FAILURE in the message, the return value will still be 0. The list of
 * options is built in the message pointed to by mp.
 */

int
in_pcboptmgmt(q, req, opt, mp)
	queue_t        *q;
	struct T_optmgmt_req *req;
	struct opthdr  *opt;
	mblk_t         *mp;
{
	struct inpcb   *inp = qtoinp(q);
	int            *optval;

	switch (req->MGMT_flags) {

	case T_NEGOTIATE:
		switch (opt->name) {
		case SO_LINGER:{
				struct linger  *l = (struct linger *) OPTVAL(opt);
				if (opt->len != OPTLEN(sizeof(struct linger)))
					return TBADOPT;
				if (l->l_onoff) {
					inp->inp_protoopt |= SO_LINGER;
					inp->inp_linger = l->l_linger;
				} else
					inp->inp_protoopt &= ~SO_LINGER;
				break;
			}

		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_USELOOPBACK:
		case SO_BROADCAST:
		case SO_REUSEADDR:
		case SO_OOBINLINE:
		case SO_IMASOCKET:
		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
		case SO_PROTOTYPE:
			if (opt->len != OPTLEN(sizeof(int)))
				return TBADOPT;
			optval = (int *) OPTVAL(opt);
			switch (opt->name) {
			case SO_DEBUG:
			case SO_KEEPALIVE:
			case SO_DONTROUTE:
			case SO_USELOOPBACK:
			case SO_BROADCAST:
			case SO_REUSEADDR:
			case SO_OOBINLINE:
			case SO_IMASOCKET:
				if (*(int *) optval)
					inp->inp_protoopt |= opt->name;
				else
					inp->inp_protoopt &= ~opt->name;
				break;
			case SO_SNDBUF:
				q->q_hiwat = *(int *) optval;
				break;
			case SO_RCVBUF:
				RD(q)->q_hiwat = *(int *) optval;
				break;
			case SO_SNDLOWAT:
				q->q_lowat = *(int *) optval;
				break;
			case SO_RCVLOWAT:
				RD(q)->q_lowat = *(int *) optval;
				break;
			case SO_PROTOTYPE:
				qtoinp(q)->inp_proto = *(int *) optval;
				break;
			}
			break;

		default:
			return TBADOPT;
		}

		/* fall through to retrieve value */

	case T_CHECK:
		switch (opt->name) {
		case SO_LINGER:{
				struct linger   l;
				if (inp->inp_protoopt & SO_LINGER) {
					l.l_onoff = 1;
					l.l_linger = inp->inp_linger;
				} else
					l.l_onoff = l.l_linger = 0;
				if (!makeopt(mp, SOL_SOCKET, SO_LINGER, &l, sizeof(l)))
					return -ENOSR;
				break;
			}

		case SO_DEBUG:
		case SO_KEEPALIVE:
		case SO_DONTROUTE:
		case SO_USELOOPBACK:
		case SO_BROADCAST:
		case SO_REUSEADDR:
		case SO_OOBINLINE:
		case SO_IMASOCKET:
		case SO_SNDBUF:
		case SO_RCVBUF:
		case SO_SNDLOWAT:
		case SO_RCVLOWAT:
		case SO_PROTOTYPE:{
				int             val;

				switch (opt->name) {
				case SO_DEBUG:
				case SO_KEEPALIVE:
				case SO_DONTROUTE:
				case SO_USELOOPBACK:
				case SO_BROADCAST:
				case SO_REUSEADDR:
				case SO_OOBINLINE:
				case SO_IMASOCKET:
					val = (inp->inp_protoopt & opt->name) != 0;
					break;

				case SO_SNDBUF:
					val = q->q_hiwat;
					break;
				case SO_RCVBUF:
					val = RD(q)->q_hiwat;
					break;
				case SO_SNDLOWAT:
					val = q->q_lowat;
					break;
				case SO_RCVLOWAT:
					val = RD(q)->q_lowat;
					break;
				case SO_PROTOTYPE:
					val = qtoinp(q)->inp_proto;
					break;
				}

				if (!makeopt(mp, SOL_SOCKET, opt->name, &val, sizeof(int)))
					return -ENOSR;
				break;
			}

		default:
			req->MGMT_flags = T_FAILURE;
			break;
		}
		break;

	case T_DEFAULT:{
			struct optdefault *o;
			int             val;

			/* get default values from table */
			for (o = sockdefault; o->optname; o++) {
				if (o->val) {
					if (!makeopt(mp, SOL_SOCKET, o->optname,
						     o->val, o->len))
						return -ENOSR;
				}
			}

			/* add default values that aren't in the table */
			val = q->q_qinfo->qi_minfo->mi_hiwat;
			if (!makeopt(mp, SOL_SOCKET, SO_SNDBUF, &val, sizeof(int)))
				return -ENOSR;
			val = RD(q)->q_qinfo->qi_minfo->mi_hiwat;
			if (!makeopt(mp, SOL_SOCKET, SO_RCVBUF, &val, sizeof(int)))
				return -ENOSR;
			val = q->q_qinfo->qi_minfo->mi_lowat;
			if (!makeopt(mp, SOL_SOCKET, SO_SNDLOWAT, &val, sizeof(int)))
				return -ENOSR;
			val = RD(q)->q_qinfo->qi_minfo->mi_lowat;
			if (!makeopt(mp, SOL_SOCKET, SO_RCVLOWAT, &val, sizeof(int)))
				return -ENOSR;

			break;
		}

	}

	return 0;
}


/*
 * IP socket option processing. This function is actually called from higher
 * level protocols which have an inpcb structure (e.g., TCP).
 */

int
ip_options(q, req, opt, mp)
	queue_t        *q;
	struct T_optmgmt_req *req;
	struct opthdr  *opt;
	mblk_t         *mp;
{
	struct inpcb   *inp = qtoinp(q);
	int             error;

	switch (req->MGMT_flags) {

	case T_NEGOTIATE:
		switch (opt->name) {
		case IP_OPTIONS:
			if ((error = ip_pcbopts(inp, OPTVAL(opt), opt->len, 1)))
				return error;
			break;
		default:
			return TBADOPT;
		}

		/* fall through to retrieve value */

	case T_CHECK:
		switch (opt->name) {
		case IP_OPTIONS:{
				mblk_t         *opts;

				if (opts = inp->inp_options) {
					/* don't copy first 4 bytes */
					if (!makeopt(mp, IPPROTO_IP, IP_OPTIONS,
						     opts->b_rptr + sizeof(struct in_addr),
					 opts->b_wptr - (opts->b_rptr + 4)))
						return -ENOSR;
				} else {
					if (!makeopt(mp, IPPROTO_IP, IP_OPTIONS,
						     (char *) 0, 0))
						return -ENOSR;
				}
				break;
			}
		default:
			req->MGMT_flags = T_FAILURE;
			break;
		}
		break;

	case T_DEFAULT:
		if (!makeopt(mp, IPPROTO_IP, IP_OPTIONS, (char *) 0, 0))
			return -ENOSR;
		break;
	}

	return 0;

}

/*
 * Set up IP options in pcb for insertion in output packets. Store in message
 * block with pointer in inp->inp_options, adding pseudo-option with
 * destination address if source routed. If set is non-zero, set new options,
 * otherwise just check.
 */
ip_pcbopts(inp, optbuf, cnt, set)
	struct inpcb   *inp;
	char           *optbuf;
	int             cnt, set;
{
	register        optlen;
	register u_char *cp;
	register mblk_t *bp1 = (mblk_t *) NULL;
	u_char          opt;

	if (cnt == 0) {
		if (set) {
			if (inp->inp_options)
				freeb(inp->inp_options);
			inp->inp_options = NULL;
		}
		return 0;
	}
	/*
	 * IP first-hop destination address will be stored before actual
	 * options; move other options back and clear it when none present.
	 */

	if ((bp1 = allocb((int) (cnt + sizeof(struct in_addr)), BPRI_LO))
	    == NULL) {
		return -ENOSR;
	}
	cp = bp1->b_wptr += sizeof(struct in_addr);
	in_ovbcopy(optbuf, (char *) bp1->b_wptr, (unsigned) cnt);
	bp1->b_wptr += cnt;
	bzero((char *) bp1->b_rptr, sizeof(struct in_addr));

	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= IPOPT_OLEN || optlen > cnt)
				goto bad;
		}
		switch (opt) {

		default:
			break;

		case IPOPT_LSRR:
		case IPOPT_SSRR:
			/*
			 * user process specifies route as: ->A->B->C->D D
			 * must be our final destination (but we can't check
			 * that since we may not have connected yet). A is
			 * first hop destination, which doesn't appear in
			 * actual IP option, but is stored before the
			 * options.
			 */
			if (optlen < IPOPT_MINOFF - 1 + sizeof(struct in_addr))
				goto bad;
			bp1->b_wptr -= sizeof(struct in_addr);
			cnt -= sizeof(struct in_addr);
			optlen -= sizeof(struct in_addr);
			cp[IPOPT_OLEN] = optlen;
			/*
			 * Move first hop before start of options.
			 */
			bcopy((caddr_t) & cp[IPOPT_OFFSET + 1],
			      (char *) bp1->b_rptr, sizeof(struct in_addr));
			/*
			 * Then copy rest of options back to close up the
			 * deleted entry.
			 */
			in_ovbcopy((caddr_t) (&cp[IPOPT_OFFSET + 1] +
					 sizeof(struct in_addr)),
			      (caddr_t) & cp[IPOPT_OFFSET + 1],
			      (unsigned) cnt - (IPOPT_OFFSET + 1));
			break;
		}
	}
	if (set)
		inp->inp_options = bp1;
	else
		freeb(bp1);
	return 0;

bad:
	freeb(bp1);
	return TBADOPT;
}
