/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-inet:in.c	1.3.1.1"

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

#include <sys/param.h>
#include <sys/sockio.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <net/af.h>
#include <net/route.h>
#include <netinet/ip_str.h>

#define satosin(sa)	((struct sockaddr_in *) (sa))

#ifdef	STRINGS	/* that is we compile without RFS streams */
int	 in_strcmp();
int	 in_strlen();
int	 in_strncmp();
char	*in_strncpy();
char	*in_strcpy();

#define strcmp	in_strcmp
#define strlen	in_strlen
#define strncmp in_strncmp
#define strncpy	in_strncpy
#define strcpy	in_strcpy

#endif	/* STRINGS */

extern struct ip_provider provider[];
extern struct ip_pcb ip_pcb[];
extern struct ip_provider *lastprov;

extern struct ifstats *ifstats;		/* from /etc/master.d/kernel */

inet_hash(in, hp)
	struct in_addr  in;
	struct afhash  *hp;
{
	register u_long n;

	n = in_netof(in);
	if (n)
		while ((n & 0xff) == 0)
			n >>= 8;
	hp->afh_nethash = n;
	hp->afh_hosthash = ntohl(in.s_addr);
}

inet_netmatch(in1, in2)
	struct in_addr  in1, in2;
{

	return (in_netof(in1) == in_netof(in2));
}

/*
 * Formulate an Internet address from network + host. 
 */
struct in_addr
in_makeaddr(net, host)
	u_long          net, host;
{
	register u_long mask;
	register struct ip_provider *prov;
	u_long          addr;

	if (IN_CLASSA(net))
		mask = IN_CLASSA_HOST;
	else if (IN_CLASSB(net))
		mask = IN_CLASSB_HOST;
	else
		mask = IN_CLASSC_HOST;
	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
			continue;
		}
		if ((prov->ia.ia_netmask & net) == prov->ia.ia_net) {
			mask = ~prov->ia.ia_subnetmask;
			break;
		}
	}
	addr = htonl(net | (host & mask));
	return (*(struct in_addr *) & addr);
}

/*
 * Return the network number from an internet address. 
 */
u_long
in_netof(in)
	struct in_addr  in;
{
	register u_long i = ntohl(in.s_addr);
	register struct ip_provider *prov;
	register u_long net;

	if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET;
	else if (IN_CLASSC(i))
		net = i & IN_CLASSC_NET;
	else
		return 0;

	/*
	 * Check whether network is a subnet; if so, return subnet number. 
	 */
	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
			continue;
		}
		if (net == prov->ia.ia_net) {
			return (i & prov->ia.ia_subnetmask);
		}
	}
	return (net);
}

/*
 * Return the host portion of an internet address. 
 */
u_long
in_lnaof(in)
	struct in_addr  in;
{
	register u_long i = ntohl(in.s_addr);
	register struct ip_provider *prov;
	register u_long net, host;

	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		host = i & IN_CLASSA_HOST;
	} else if (IN_CLASSB(i)) {
		net = i & IN_CLASSB_NET;
		host = i & IN_CLASSB_HOST;
	} else if (IN_CLASSC(i)) {
		net = i & IN_CLASSC_NET;
		host = i & IN_CLASSC_HOST;
	} else
		return i;

	/*
	 * Check whether network is a subnet; if so, use the modified
	 * interpretation of `host'. 
	 */
	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
			continue;
		}
		if (net == prov->ia.ia_net) {
			return (host & ~prov->ia.ia_subnetmask);
		}
	}
	return (host);
}

#ifndef SUBNETSARELOCAL
#define	SUBNETSARELOCAL	1
#endif
int             subnetsarelocal = SUBNETSARELOCAL;
/*
 * Return 1 if an internet address is for a ``local'' host (one to which we
 * have a connection).  If subnetsarelocal is true, this includes other
 * subnets of the local net. Otherwise, it includes only the
 * directly-connected (sub)nets. 
 */
in_localaddr(in)
	struct in_addr  in;
{
	register u_long i = ntohl(in.s_addr);
	register struct ip_provider *prov;

	if (subnetsarelocal) {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
				continue;
			}
			if ((i & prov->ia.ia_netmask) == prov->ia.ia_net)
				return 1;
		}
	} else {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
				continue;
			}
			if ((i & prov->ia.ia_subnetmask) == prov->ia.ia_subnet)
				return 1;
		}
	}
	return (0);
}

/*
 * Determine whether an IP address is in a reserved set of addresses
 * that may not be forwarded, or whether datagrams to that destination
 * may be forwarded.
 */
in_canforward(in)
        struct in_addr in;
{
        register u_long i = ntohl(in.s_addr);
        register u_long net;

        if (IN_EXPERIMENTAL(i))
                return (0);
        if (IN_CLASSA(i)) {
                net = i & IN_CLASSA_NET;
                if (net == 0 || net == IN_LOOPBACKNET)
                        return (0);
        }
        return (1);
}

/*
 * Return 1 if the internet address in our own. 
 */

in_ouraddr(addr)
	struct in_addr  addr;
{
	register struct ip_provider *prov;

	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
			continue;
		}
		if (addr.s_addr == PROV_INADDR(prov)->s_addr) {
			return (1);
		}
	}
	return (0);
}

int             in_interfaces;	/* number of external internet interfaces */
struct ip_provider *loopprov;	/* loopback provider (for switched slip) */

/*
 * Generic internet control operations (ioctl's). 
 */
in_control(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	register struct ip_provider *prov = NULL;
	register struct iocblk *iocbp;
	register struct ifreq *ifr;
	u_short          size;
	u_long          tmp;

	iocbp = (struct iocblk *) bp->b_rptr;
	size = (iocbp->ioc_cmd & ~(IOC_INOUT|IOC_VOID)) >> 16;
	if (!(iocbp->ioc_cmd & (IOC_INOUT | IOC_VOID))
	    || iocbp->ioc_count != size
	    || (size && bp->b_cont == NULL)
	    || (size && msgblen(bp->b_cont) != size)) {
		iocbp->ioc_error = EINVAL;
		bp->b_datap->db_type = M_IOCNAK;
		qreply(q, bp);
		return;
	}
	if (size)
		ifr = (struct ifreq *) (bp->b_cont->b_rptr);
	else
		ifr = NULL;

	if (iocbp->ioc_cmd == SIOCSIFNAME) {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot && ifr->ifr_metric == prov->l_index) {
				break;
			}
		}
	} else {
		for (prov = provider; prov <= lastprov; prov++) {
			if (prov->qbot &&
			    !strncmp(ifr->ifr_name, prov->name,
				     sizeof(ifr->ifr_name))) {
				break;
			}
		}
	}
	if (prov > lastprov)
		prov = NULL;

	/*
	 * restrict privileged ioctl's. 
	 */
	switch (iocbp->ioc_cmd) {
	case SIOCSIFNAME:
	case SIOCSIFADDR:
	case SIOCSIFBRDADDR:
	case SIOCSIFDSTADDR:
	case SIOCSIFFLAGS:
	case SIOCSIFNETMASK:
	case SIOCSIFMETRIC:
	case SIOCIFDETACH:
	case SIOCSLGETREQ:
	case SIOCSLSTAT:
		if (iocbp->ioc_uid != 0) {
			iocbp->ioc_error = EPERM;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		break;
	default:
		break;
	}

	if (msgblen(bp) < sizeof(struct iocblk_in)) {
		if (bpsize(bp) < sizeof(struct iocblk_in)) {
			mblk_t         *nbp;

			nbp = allocb(sizeof(struct iocblk_in), BPRI_MED);
			if (!nbp) {
				iocbp->ioc_error = ENOSR;
				bp->b_datap->db_type = M_IOCNAK;
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
	if (prov != (struct ip_provider *) NULL
	    && iocbp->ioc_cmd != SIOCSIFFLAGS) {
		((struct iocblk_in *) iocbp)->ioc_ifflags = prov->if_flags;
	}
	((struct iocblk_in *) iocbp)->ioc_network_client = RD(q);

	/*
	 * switched ioctls don't always have a valid interface 
	 */
	switch (iocbp->ioc_cmd) {
	case SIOCSLGETREQ:
		slgetreq(q, bp, prov);
		return;
	case SIOCSLSTAT:
		slstat(q, bp, prov);
		return;
	default:
		break;
	}

	if (prov == (struct ip_provider *) NULL) {
		iocbp->ioc_error = EINVAL;
		bp->b_datap->db_type = M_IOCNAK;
		qreply(q, bp);
		return;
	}
	switch (iocbp->ioc_cmd) {
	case SIOCSIFNAME:
		for (tmp = 0; ifr->ifr_name[tmp]; tmp++);
		if (tmp == 0 || tmp >= IFNAMSIZ) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		break;

	case SIOCSIFADDR:
		break;

	case SIOCGIFADDR:
		ifr->ifr_addr = prov->if_addr;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = prov->if_metric;
		break;

	case SIOCGIFFLAGS:
		ifr->ifr_flags = prov->if_flags;
		break;

	case SIOCSIFDSTADDR:
		if ((prov->if_flags & IFF_POINTOPOINT) == 0) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		if (prov->ia.ia_flags & IFA_ROUTE) {
			rtinit(*SOCK_INADDR(&prov->if_dstaddr), *PROV_INADDR(prov),
			       (int) SIOCDELRT, RTF_HOST);
			rtinit(*SOCK_INADDR(&ifr->ifr_addr), *PROV_INADDR(prov),
			       (int) SIOCADDRT, RTF_HOST | RTF_UP);
		}
		prov->if_dstaddr = ifr->ifr_addr;
		break;

	case SIOCGIFDSTADDR:
		if ((prov->if_flags & IFF_POINTOPOINT) == 0) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		ifr->ifr_addr = prov->if_dstaddr;
		ifr->ifr_addr.sa_family = AF_INET;
		break;

	case SIOCSIFBRDADDR:
		if ((prov->if_flags & IFF_BROADCAST) == 0) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		prov->if_broadaddr = ifr->ifr_addr;
		break;

	case SIOCGIFBRDADDR:
		if ((prov->if_flags & IFF_BROADCAST) == 0) {
			iocbp->ioc_error = EINVAL;
			bp->b_datap->db_type = M_IOCNAK;
			qreply(q, bp);
			return;
		}
		ifr->ifr_addr = prov->if_broadaddr;
		ifr->ifr_addr.sa_family = AF_INET;
		break;

	case SIOCSIFNETMASK:
		prov->ia.ia_subnetmask = 
			ntohl(SOCK_INADDR(&ifr->ifr_addr)->s_addr);
		break;

	case SIOCGIFNETMASK:
		SOCK_INADDR(&ifr->ifr_addr)->s_addr = 
			htonl(prov->ia.ia_subnetmask);
		ifr->ifr_addr.sa_family = AF_INET;
		break;

	case SIOCSIFFLAGS:
		ifr->ifr_flags = (prov->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags & ~IFF_CANTCHANGE);
		((struct iocblk_in *) iocbp)->ioc_ifflags = ifr->ifr_flags;
		break;

	case SIOCSIFMETRIC:
		prov->if_metric = ifr->ifr_metric;
		break;

	case SIOCIFDETACH:
		swdetach(prov);
		rtdetach(prov);
		if (prov != loopprov)
			in_interfaces--;
		break;

	default:
		break;
	}
	/*
	 * Now send the command down to the arp module, if it approves, it
	 * will ack it and the other side of the stream will recognize it and
	 * return it to the user. 
	 */

	putnext(prov->qbot, bp);
}


/*
 * Initialize an interface's internet address and routing table entry. This
 * routine is called after the convergence module has decided that it likes a
 * setaddr request. 
 */
in_ifinit(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	register struct ip_provider *prov;
	register struct ifreq *ifr;
	struct in_addr  netaddr;
	register u_long  i;

	prov = (struct ip_provider *) q->q_ptr;
	ifr = (struct ifreq *) bp->b_cont->b_rptr;

	i = ntohl(SOCK_INADDR(&ifr->ifr_addr)->s_addr);

	/*
	 * Delete any previous route for the old address. 
	 */

	if (prov->ia.ia_flags & IFA_ROUTE) {
		if (prov->if_flags & IFF_LOOPBACK) {
			rtinit(*PROV_INADDR(prov), *PROV_INADDR(prov),
			       (int) SIOCDELRT, RTF_HOST);
		} else if (prov->if_flags & IFF_POINTOPOINT) {
			rtinit(*SOCK_INADDR(&prov->if_dstaddr), *PROV_INADDR(prov),
			       (int) SIOCDELRT, RTF_HOST);
		} else {
			netaddr = in_makeaddr(prov->ia.ia_subnet, INADDR_ANY);
			rtinit(netaddr, *PROV_INADDR(prov), (int) SIOCDELRT, 0);
		}
		prov->ia.ia_flags &= ~IFA_ROUTE;
	}
	prov->if_addr = ifr->ifr_addr;	/* set the address */

	if (IN_CLASSA(i))
		prov->ia.ia_netmask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		prov->ia.ia_netmask = IN_CLASSB_NET;
	else
		prov->ia.ia_netmask = IN_CLASSC_NET;
	prov->ia.ia_net = i & prov->ia.ia_netmask;
	/*
	 * The subnet mask includes at least the standard network part, but
	 * may already have been set to a larger value. 
	 */
	prov->ia.ia_subnetmask |= prov->ia.ia_netmask;
	prov->ia.ia_subnet = i & prov->ia.ia_subnetmask;
	if (prov->if_flags & IFF_BROADCAST) {
		*SOCK_INADDR(&prov->if_broadaddr) = 
			in_makeaddr(prov->ia.ia_subnet, INADDR_BROADCAST);
		prov->ia.ia_netbroadcast.s_addr = 
			htonl(prov->ia.ia_net |
			      (INADDR_BROADCAST & ~prov->ia.ia_netmask));
	}
	/*
	 * Add route for the network. 
	 */
	prov->if_flags |= IFF_UP;	/* interface must be up to add route */
	if (prov->if_flags & IFF_LOOPBACK)
		in_rtinit(*PROV_INADDR(prov), prov, RTF_HOST | RTF_UP);
	else if (prov->if_flags & IFF_POINTOPOINT)
		in_rtinit(*SOCK_INADDR(&prov->if_dstaddr), prov, RTF_HOST | RTF_UP);
	else {
		netaddr = in_makeaddr(prov->ia.ia_subnet, INADDR_ANY);
		in_rtinit(netaddr, prov, RTF_UP);
	}
	prov->ia.ia_flags |= IFA_ROUTE;
	return;
}

/*
 * Add a route for an interface without looking for one already.
 * That way, two interfaces on the same network will work.
 */

in_rtinit(dst, prov, flags)
	struct in_addr		dst;
	struct ip_provider	*prov;
	int			flags;
{
	struct rtentry		*route;
	struct afhash		h;
	register mblk_t		*m, **mprev;
	mblk_t			**mfirst;
	u_long			hash;

	/*
	 * Make sure that there is no route for this destination yet
	 */

	inet_hash(dst, &h);

	if (flags & RTF_HOST) {
		hash = h.afh_hosthash;
		mprev = &rthost[RTHASHMOD(hash)];
	} else {
		hash = h.afh_nethash;
		mprev = &rtnet[RTHASHMOD(hash)];
	}

	for (mfirst = mprev; m = *mprev; mprev = &m->b_cont) {
		route = RT(m);
		if (route->rt_hash != hash)
			continue;
		if (flags & RTF_HOST) {
			if (satosin(&route->rt_dst)->sin_addr.s_addr != dst.s_addr)
				continue;
		} else {
			if (inet_netmatch(route->rt_dst, dst) == 0)
				continue;
		}
		if (satosin(&route->rt_gateway)->sin_addr.s_addr == PROV_INADDR(prov)->s_addr)
			break;
	}
	/*
	 * Already the same interface for same net
	 */

	if (m)
		return;

	/*
	 * Allocate a new route entry
	 */

	m = allocb(sizeof(struct rtentry), BPRI_HI);
	if (!m)
		return;

	/*
	 * Link this entry into the list.
	 */

	m->b_cont = *mfirst;
	*mfirst = m;

	m->b_wptr += sizeof(struct rtentry);

	route = RT(m);
	bzero((caddr_t) route, sizeof(route));

	/*
	 * Fill in remainder of route information.
	 */

	route->rt_hash = hash;
	satosin(&route->rt_dst)->sin_addr.s_addr = dst.s_addr;
	satosin(&route->rt_gateway)->sin_addr.s_addr = PROV_INADDR(prov)->s_addr;
	route->rt_flags = RTF_UP | (flags & (RTF_USERMASK | RTF_TOSWITCH | RTF_DYNAMIC));
	route->rt_prov = prov;
	route->rt_use = 0;
	route->rt_refcnt = 0;
}

/*
 * ioctls sent downstream by in_control come back upstream through
 * in_upstream. 
 */

in_upstream(q, bp)
	queue_t        *q;
	mblk_t         *bp;
{
	struct iocblk  *iocbp = (struct iocblk *) bp->b_rptr;
	struct ip_provider *prov = (struct ip_provider *) q->q_ptr;

	/* if it failed, just pass it up */
	if (bp->b_datap->db_type == M_IOCNAK) {
		putnext(((struct iocblk_in *) iocbp)->ioc_network_client, bp);
		return;
	}
	/* get flag updates */
	prov->if_flags = ((struct iocblk_in *) iocbp)->ioc_ifflags;
	switch (iocbp->ioc_cmd) {
	case SIOCSIFNAME:{
			struct ifstats *ifs;
			char            name[IFNAMSIZ+1];
			char            unit[2*sizeof(short)+1];

			name[IFNAMSIZ] = NULL;
			bcopy(((struct ifreq *) bp->b_cont->b_rptr)->ifr_name,
			      prov->name,
			      sizeof(prov->name));

			if (strcmp(prov->name, "lo0") == 0)
				loopprov = prov;

			for (ifs = ifstats; ifs; ifs = ifs->ifs_next) {
				strncpy(name, ifs->ifs_name, IFNAMSIZ);
				itox(ifs->ifs_unit, unit);
                                if (strlen(name) + strlen(unit) > IFNAMSIZ)
                                        continue;
				strcpy(&name[strlen(name)], unit);
				if (strncmp(name, prov->name, IFNAMSIZ) == 0) {
					prov->ia.ia_ifa.ifa_next =
						ifs->ifs_addrs;
					ifs->ifs_addrs = &prov->ia.ia_ifa;
					prov->ia.ia_ifa.ifa_ifs = ifs;
					break;
				}
			}
			break;
		}

	case SIOCSIFADDR:
		in_ifinit(q, bp);
		if (!(prov->if_flags & IFF_LOOPBACK))
			in_interfaces++;
		break;

	case SIOCGIFFLAGS:
		if (bp->b_cont == NULL)
			break;
		((struct ifreq *) (bp->b_cont->b_rptr))->ifr_flags = prov->if_flags;
		break;

	default:
		break;
	}
	putnext(((struct iocblk_in *) iocbp)->ioc_network_client, bp);
}

/*
 * return a pointer to the link provider structure for a given net 
 */

struct ip_provider *
in_onnetof(net)
	u_long          net;
{
	register struct ip_provider *prov;

	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot == NULL || !(prov->if_flags & IFF_UP)) {
			continue;
		}
		if (prov->qbot && prov->ia.ia_subnet == net) {
			return (prov);
		}
	}
	return ((struct ip_provider *) 0);
}

/*
 * Return 1 if the address is a local broadcast address. 
 */
in_broadcast(in)
	struct in_addr  in;
{
	register struct ip_provider *prov;
	u_long t;

	/*
	 * Look through the list of addresses for a match with a broadcast
	 * address. 
	 */
	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot) {
			if ((prov->if_flags & IFF_BROADCAST) && 
				(prov->if_flags & IFF_UP)) {
			    if (SOCK_INADDR(&prov->if_broadaddr)->s_addr 
				== in.s_addr)
				    return 1;
			}

			/*
			** Check for old-style (host 0) broadcast.
			*/
			if ((t = ntohl(in.s_addr)) == prov->ia.ia_subnet ||
				t == prov->ia.ia_net)
				return 1;
		}
	}
	if (in.s_addr == INADDR_BROADCAST || in.s_addr == INADDR_ANY)
		return 1;
	return (0);
}


/*
 * The following routines find a link level provider based on various
 * addressing criteria. 
 */

struct ip_provider *
prov_withaddr(addr)
	struct in_addr  addr;
{
	register struct ip_provider *prov;

	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot && (prov->if_flags & IFF_UP)) {
			if (PROV_INADDR(prov)->s_addr == addr.s_addr) {
				return (prov);
			}
			if ((prov->if_flags & IFF_BROADCAST) &&
			    (SOCK_INADDR(&prov->if_broadaddr)->s_addr 
			     == addr.s_addr)) {
				return (prov);
			}
		}
	}
	return ((struct ip_provider *) 0);
}

struct ip_provider *
prov_withdstaddr(addr)
	struct in_addr  addr;
{
	register struct ip_provider *prov;

	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot && (prov->if_flags & IFF_UP)) {
			if ((prov->if_flags & IFF_POINTOPOINT) &&
			    (SOCK_INADDR(&prov->if_dstaddr)->s_addr 
			     == addr.s_addr)) {
				return (prov);
			}
		}
	}
	return ((struct ip_provider *) 0);
}

struct ip_provider *
prov_withnet(addr)
	struct in_addr  addr;
{
	register struct ip_provider *prov;

	for (prov = provider; prov <= lastprov; prov++) {
		if (prov->qbot
		    && (prov->if_flags & IFF_UP)
		    && inet_netmatch(*PROV_INADDR(prov), addr)) {
			return (prov);
		}
	}
	return ((struct ip_provider *) 0);
}
