/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.routed/startup.c	1.3.2.1"

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
 * Routing Table Management Daemon
 */
#include "defs.h"
#include <sys/sockio.h>
#include <net/if.h>
#include <syslog.h>

struct	interface *ifnet;
int	lookforinterfaces = 1;
int	externalinterfaces = 0;		/* # of remote and local interfaces */

#define MAXIFS 256

char buf[1024];

extern int iosoc;

/*
 * Find the network interfaces which have configured themselves.
 * If the interface is present but not yet up (for example an
 * ARPANET IMP), set the lookforinterfaces flag so we'll
 * come back later and look again.
 */
ifinit()
{
	struct interface ifs, *ifp;
	int n;
        struct ifconf ifc;
        struct ifreq ifreq, *ifr;
        struct sockaddr_in *sin;
	u_long i;

	bzero((char *) &ifc, sizeof(ifc));
        ifc.ifc_len = sizeof (buf);
        ifc.ifc_buf = &buf[0];
        if (ifioctl(iosoc, SIOCGIFCONF, (char *)&ifc) < 0) {
                perror("ioctl (get interface configuration)");
                return;
        }
        ifr = ifc.ifc_req;
	lookforinterfaces = 0;
        for (n = ifc.ifc_len / sizeof (struct ifreq); n > 0; n--, ifr++) {
		bzero((char *)&ifs, sizeof(ifs));
		ifs.int_addr = ifr->ifr_addr;
		ifreq = *ifr;
                if (ifioctl(iosoc, SIOCGIFFLAGS, (char *)&ifreq) < 0) {
                        perror("ioctl (get interface flags)");
                        continue;
                }
		ifs.int_flags = (ifreq.ifr_flags &IFF_FROMKERNEL) 
			| IFF_INTERFACE;
		if ((ifs.int_flags & IFF_UP) == 0 ||
		    ifr->ifr_addr.sa_family == AF_UNSPEC) {
			if (ifp = if_ifwithname(ifr->ifr_name))
				if_purge(ifp);
			lookforinterfaces = 1;
			continue;
		}
		/* do we already know about this interface? */
		if (if_ifwithname(ifr->ifr_name))
			continue;
		/* argh, this'll have to change sometime */
		if (ifs.int_addr.sa_family != AF_INET)
			continue;
                if (ifs.int_flags & IFF_POINTOPOINT) {
                        if (ifioctl(iosoc, SIOCGIFDSTADDR, (char *)&ifreq) < 0) {
                                perror("ioctl (get dstaddr)");
                                continue;
			}
			ifs.int_dstaddr = ifreq.ifr_dstaddr;
			if (ifs.int_dstaddr.sa_family != AF_INET) continue;
		}
		ifs.int_metric = 0;
		if (ifioctl(iosoc, SIOCGIFNETMASK, (char *)&ifreq) < 0) {
			/*
			 * we allow this to be run on a machine that does
			 * not have this ioctl.
			 */
			 bzero((caddr_t)&ifreq.ifr_addr, 
			 	sizeof(struct sockaddr));
		}
		sin = (struct sockaddr_in *)&ifreq.ifr_addr;
		ifs.int_subnetmask = ntohl(sin->sin_addr.s_addr);
		sin = (struct sockaddr_in *)&ifs.int_addr;
		i = ntohl(sin->sin_addr.s_addr);
		if (IN_CLASSA(i))
			ifs.int_netmask = IN_CLASSA_NET;
		else if (IN_CLASSB(i))
			ifs.int_netmask = IN_CLASSB_NET;
		else
			ifs.int_netmask = IN_CLASSC_NET;
		if (ifs.int_subnetmask == 0) 
			ifs.int_subnetmask = ifs.int_netmask;
		ifs.int_net = i & ifs.int_netmask;
		ifs.int_subnet = i & ifs.int_subnetmask;
		if (ifs.int_subnetmask != ifs.int_netmask)
			ifs.int_flags |= IFF_SUBNET;
		/* no one cares about software loopback interfaces */
		if (ifs.int_net == LOOPBACKNET)
			continue;
                if (ifs.int_flags & IFF_BROADCAST) {
                        if (ifioctl(iosoc, SIOCGIFBRDADDR, (char *)&ifreq) < 0) {
			  /*
			   * presume old-style if new ioctl not supported
			   */
			    sin = (struct sockaddr_in *)&ifs.int_broadaddr;
			    bzero((caddr_t)sin, sizeof(ifs.int_broadaddr) );
			    sin->sin_family = AF_INET;
			    sin->sin_addr.s_addr = ifs.int_subnet;
			}
			else ifs.int_broadaddr = ifreq.ifr_addr;
		} 
		ifp = (struct interface *)malloc(sizeof (struct interface));
		if (ifp == 0) {
			printf("routed: out of memory\n");
			break;
		}
		*ifp = ifs;
		/*
		 * Count the # of directly connected networks
		 * and point to point links which aren't looped
		 * back to ourself.  This is used below to
		 * decide if we should be a routing ``supplier''.
		 */
		if ((ifs.int_flags & IFF_POINTOPOINT) == 0 ||
		    if_ifwithaddr(&ifs.int_dstaddr) == 0)
			externalinterfaces++;
		/*
		 * If we have a point-to-point link, we want to act
		 * as a supplier even if it's our only interface,
		 * as that's the only way our peer on the other end
		 * can tell that the link is up.
		 */
		if ((ifs.int_flags & IFF_POINTOPOINT) && supplier < 0)
			supplier = 1;
		ifp->int_name = malloc(strlen(ifr->ifr_name) + 1);
		if (ifp->int_name == 0) {
			fprintf(stderr, "routed: ifinit: out of memory\n");
			goto bad;		/* ??? */
		}
		strcpy(ifp->int_name, ifr->ifr_name);
		ifp->int_next = ifnet;
		ifnet = ifp;
		traceinit(ifp);
		addrouteforif(ifp);
	}
	supplier = maysupply;
	if (supplier < 0)
		supplier = externalinterfaces > 1;
	return;
bad:
	sleep(60);
	execv(argv0[0], argv0);
	_exit(0177);
}

/*
 * Add route for interface if not currently installed.
 * Create route to other end if a point-to-point link,
 * otherwise a route to this (sub)network.
 * INTERNET SPECIFIC.
 */
addrouteforif(ifp)
	struct interface *ifp;
{
	struct sockaddr_in net;
	struct sockaddr *dst;
	struct rt_entry *rt;

	if (ifp->int_flags & IFF_POINTOPOINT)
		dst = &ifp->int_dstaddr;
	else {
		bzero((char *)&net, sizeof (net));
		net.sin_family = AF_INET;
		net.sin_addr = inet_makeaddr(ifp->int_subnet, INADDR_ANY);
		dst = (struct sockaddr *)&net;
	}
	rt = rtlookup(dst);
	if (rt)
		rtdelete(rt);
	/*
	 * If interface on subnetted network,
	 * install route to network as well.
	 * This is meant for external viewers.
	 */
	if ((ifp->int_flags & IFF_SUBNET) == IFF_SUBNET) {
		struct sockaddr_in subnet_sa;

		bzero((char *)&subnet_sa, sizeof(subnet_sa));
		subnet_sa.sin_family = AF_INET;
		subnet_sa.sin_addr = inet_makeaddr(ifp->int_net, INADDR_ANY);
		rt = rtfind((struct sockaddr *)&subnet_sa);
		if (rt == 0)
			rtadd((struct sockaddr *)&subnet_sa, &ifp->int_addr, ifp->int_metric,
			    ((ifp->int_flags & (IFF_INTERFACE|IFF_REMOTE|IFF_PRIVATE)) |
				RTS_PASSIVE | RTS_INTERNAL | RTS_SUBNET));

	}
	if (ifp->int_transitions++ > 0) {
		printf("re-installing interface %s", ifp->int_name);
		fflush(stdout);
	}
	rtadd(dst, &ifp->int_addr, ifp->int_metric,
	    ifp->int_flags & (IFF_INTERFACE|IFF_PASSIVE|IFF_REMOTE|IFF_PRIVATE|IFF_SUBNET|IFF_POINTOPOINT));
}

/*
 * As a concession to the ARPANET we read a list of gateways
 * from /etc/gateways and add them to our tables.  This file
 * exists at each ARPANET gateway and indicates a set of ``remote''
 * gateways (i.e. a gateway which we can't immediately determine
 * if it's present or not as we can do for those directly connected
 * at the hardware level).  If a gateway is marked ``passive''
 * in the file, then we assume it doesn't have a routing process
 * of our design and simply assume it's always present.  Those
 * not marked passive are treated as if they were directly
 * connected -- they're added into the interface list so we'll
 * send them routing updates.
 */
gwkludge()
{
	struct sockaddr_in dst, gate;
	FILE *fp;
	char *type, *dname, *gname, *qual, buf[BUFSIZ];
	struct interface *ifp;
	int metric;
	struct rt_entry route;

	fp = fopen("/etc/gateways", "r");
	if (fp == NULL)
		return;
	qual = buf;
	dname = buf + 64;
	gname = buf + ((BUFSIZ - 64) / 3);
	type = buf + (((BUFSIZ - 64) * 2) / 3);
	bzero((char *)&dst, sizeof (dst));
	bzero((char *)&gate, sizeof (gate));
	bzero((char *)&route, sizeof(route));
	/* format: {net | host} XX gateway XX metric DD [passive]\n */
#define	readentry(fp) \
	fscanf((fp), "%s %s gateway %s metric %d %s\n", \
		type, dname, gname, &metric, qual)
	for (;;) {
		if (readentry(fp) == EOF)
			break;
		if (!getnetorhostname(type, dname, &dst))
			continue;
		if (!gethostnameornumber(gname, &gate))
			continue;
		if (strcmp(qual, "passive") == 0) {
			/*
			 * Passive entries aren't placed in our tables,
			 * only the kernel's, so we don't copy all of the
			 * external routing information within a net.
			 * Internal machines should use the default
			 * route to a suitable gateway (like us).
			 */
			route.rt_dst = *(struct sockaddr *) &dst;
			route.rt_router = *(struct sockaddr *) &gate;
			route.rt_flags = RTF_UP;
			if (strcmp(type, "host") == 0)
				route.rt_flags |= RTF_HOST;
			if (metric)
				route.rt_flags |= RTF_GATEWAY;
			(void) rtioctl(iosoc, SIOCADDRT, (char *)&route.rt_rt);
			continue;
		}
		if (strcmp(qual, "external") == 0) {
			/*
			 * Entries marked external are handled
			 * by other means, e.g. EGP,
			 * and are placed in our tables only
			 * to prevent overriding them
			 * with something else.
			 */
			rtadd(&dst, &gate, metric, RTS_EXTERNAL|RTS_PASSIVE);
			continue;
		}
		/* assume no duplicate entries */
		externalinterfaces++;
		ifp = (struct interface *)malloc(sizeof (*ifp));
		bzero((char *)ifp, sizeof (*ifp));
		ifp->int_flags = IFF_REMOTE;
		/* can't identify broadcast capability */
		ifp->int_net = inet_netof(dst.sin_addr);
		if (strcmp(type, "host") == 0) {
			ifp->int_flags |= IFF_POINTOPOINT;
			ifp->int_dstaddr = *((struct sockaddr *)&dst);
		}
		ifp->int_addr = *((struct sockaddr *)&gate);
		ifp->int_metric = metric;
		ifp->int_next = ifnet;
		ifnet = ifp;
		addrouteforif(ifp);
	}
	fclose(fp);
}

getnetorhostname(type, name, sin)
	char *type, *name;
	struct sockaddr_in *sin;
{

	if (strcmp(type, "net") == 0) {
		struct netent *np;
		int n;

		n = inet_network(name);
		if (n == -1) {
			np = getnetbyname(name);
			if (np == NULL || np->n_addrtype != AF_INET)
				return (0);
			n = np->n_net;
			/*
			 * getnetbyname returns right-adjusted value.
			 */
			if (n < 128)
				n <<= IN_CLASSA_NSHIFT;
			else if (n < 65536)
				n <<= IN_CLASSB_NSHIFT;
			else
				n <<= IN_CLASSC_NSHIFT;
		}
		sin->sin_family = AF_INET;
		sin->sin_addr = inet_makeaddr(n, INADDR_ANY);
		return (1);
	}
	if (strcmp(type, "host") == 0) {
		struct hostent *hp;

		sin->sin_addr.s_addr = inet_addr(name);
		if ((int)sin->sin_addr.s_addr == -1) {
			hp = gethostbyname(name);
			if (hp == NULL || hp->h_addrtype != AF_INET)
				return (0);
			bcopy(hp->h_addr, &sin->sin_addr, hp->h_length);
		}
		sin->sin_family = AF_INET;
		return (1);
	}
	return (0);
}

gethostnameornumber(name, sin)
	char *name;
	struct sockaddr_in *sin;
{
	struct hostent *hp;

	sin->sin_addr.s_addr = inet_addr(name);
	sin->sin_family = AF_INET;
	if ((int)sin->sin_addr.s_addr != -1)
		return(1);
	hp = gethostbyname(name);
	if (hp) {
		bcopy(hp->h_addr, &sin->sin_addr, hp->h_length);
		sin->sin_family = hp->h_addrtype;
		return (1);
	}
	return(0);
}
