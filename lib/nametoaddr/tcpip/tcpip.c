/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:tcpip/tcpip.c	1.5.4.2"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
/*
 * TCP/IP name to address translation routines. These routines are written
 * to the getXXXbyYYY() interface that the BSD routines use. This allows
 * us to simply rewrite those routines to get various flavors of translation
 * routines. Thus while they look like they have socket dependencies (the
 * sockaddr_in structures) in fact this is simply the internal netbuf
 * representation that the TCP and UDP transport providers use.
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/byteorder.h>
#include <netinet/in.h>
#include <netdb.h>
#include <tiuser.h>
#include <netconfig.h>
#include <netdir.h>
#include <string.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <rpc/types.h>
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define	LOG_ERR 3
#endif /* SYSLOG */

extern char	*malloc(), *calloc();
static char	*inet_ntoa();
static struct in_addr inet_makeaddr();
extern int	_nderror;

static char 	*localaddr[] = {"\000\000\000\000", NULL};

static struct hostent localent = {
		"Localhost",
		NULL,
		AF_INET,
		4,
		localaddr
};
#define	MAXBCAST	10

#ifdef undef
/*
 * XXX: May return more than one address. INADDR_BROADCAST used here
 */
static char 	*broadaddr[] = {"\377\377\377\377", NULL};

static struct hostent broadent = {
		"broadcast",
		NULL,
		AF_INET,
		4,
		broadaddr
};
#endif

/*
 * This routine is the "internal" TCP/IP routine that will build a
 * host/service pair into one or more netbufs depending on how many
 * addresses the host has in the host table.
 * If the hostname is HOST_SELF, we return 0.0.0.0 so that the binding
 * can be contacted through all interfaces.
 * If the hostname is HOST_ANY, we return no addresses because IP doesn't
 * know how to specify a service without a host.
 * And finally if we specify HOST_BROADCAST then we ask a tli fd to tell
 * us what the broadcast addresses are for any udp interfaces on this
 * machine.
 */
struct nd_addrlist *
#ifdef PIC
_netdir_getbyname(tp, serv)
#else
#ifdef YP
nis_netdir_getbyname(tp, serv)
#else
tcp_netdir_getbyname(tp, serv)
#endif YP
#endif PIC
	struct netconfig *tp;
	struct nd_hostserv *serv;
{
	struct hostent	*he;
	struct hostent	h_broadcast;
	struct nd_addrlist *result;
	struct netbuf	*na;
	char		**t;
	struct sockaddr_in	*sa;
	int		num;
	int		server_port;
	char		*baddrlist[MAXBCAST + 1];
	struct in_addr	inaddrs[MAXBCAST];

	if (!serv || !tp) {
		_nderror = ND_BADARG;
		return (NULL);
	}
	_nderror = ND_OK;	/* assume success */

	/* NULL is not allowed, that returns no answer */
	if (! (serv->h_host)) {
		_nderror = ND_NOHOST;
		return (NULL);
	}

	/*
	 * Find the port number for the service. We look for some
	 * special cases first and on failing go into getservbyname().
	 * The special cases :
	 * 	NULL - 0 port number.
	 *	rpcbind - The rpcbind's address
	 *	A number - We don't have a name just a number so use it
	 * ifdef YP part would used by nis/Makefile that builds
	 * nis.so.
	 */
	if (!(serv->h_serv)) {
		server_port = htons(0);
	} else if (strcmp(serv->h_serv, "rpcbind") == 0) {
		server_port = htons(111);	/* Hard coded */
	} else if (strspn(serv->h_serv, "0123456789")
			== strlen(serv->h_serv)) {
		/* It's a port number */
		server_port = htons(atoi(serv->h_serv));
	} else {
		struct servent	*se;

#ifdef YP
		se = (struct servent *)_nis_getservbyname(serv->h_serv,
			(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");
#else YP
		se = (struct servent *)_tcpip_getservbyname(serv->h_serv,
			(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");
#endif YP

		if (!se) {
			_nderror = ND_NOSERV;
			return (NULL);
		}
		server_port = se->s_port;
	}

	if (!strcmp(serv->h_host, HOST_SELF)) {
		he = &localent;
	} else if ((strcmp(serv->h_host, HOST_BROADCAST) == 0)) {
		int bnets, i;

		memset((char *)inaddrs, 0, sizeof (struct in_addr) * MAXBCAST);
		bnets = getbroadcastnets(tp, inaddrs);
		if (bnets == 0)
			return (NULL);
		he = &h_broadcast;
		he->h_name = "broadcast";
		he->h_aliases = NULL;
		he->h_addrtype = AF_INET;
		he->h_length = 4;
		for (i = 0; i < bnets; i++)
			baddrlist[i] = (char *)&inaddrs[i];
		baddrlist[i] = NULL;
		he->h_addr_list = baddrlist;
	} else {
#ifdef YP
		he = (struct hostent *)_nis_gethostbyname(serv->h_host);
#else YP
		he = (struct hostent *)_tcpip_gethostbyname(serv->h_host);
#endif YP
	}

	if (!he) {
		_nderror = ND_NOHOST;
		return (NULL);
	}

	result = (struct nd_addrlist *)(malloc(sizeof (struct nd_addrlist)));
	if (!result) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/* Count the number of addresses we have */
	for (num = 0, t = he->h_addr_list; *t; t++, num++)
			;

	result->n_cnt = num;
	result->n_addrs = (struct netbuf *)
				(calloc(num, sizeof (struct netbuf)));
	if (!result->n_addrs) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/* build up netbuf structs for all addresses */
	for (na = result->n_addrs, t = he->h_addr_list; *t; t++, na++) {
		sa = (struct sockaddr_in *)calloc(1, sizeof (*sa));
		if (!sa) {
			_nderror = ND_NOMEM;
			return (NULL);
		}
		/* Vendor specific, that is why it's here and hard coded */
		na->maxlen = sizeof (struct sockaddr_in);
		na->len = sizeof (struct sockaddr_in);
		na->buf = (char *)sa;
		sa->sin_family = AF_INET;
		sa->sin_port = server_port;
		sa->sin_addr = *((struct in_addr *)(*t));
	}

	return (result);
}

/*
 * This routine is the "internal" TCP/IP routine that will build a
 * host/service pair from the netbuf passed. Currently it only
 * allows one answer, it should, in fact allow several.
 */
struct nd_hostservlist *
#ifdef PIC
_netdir_getbyaddr(tp, addr)
#else
#ifdef YP
nis_netdir_getbyaddr(tp, addr)
#else
tcp_netdir_getbyaddr(tp, addr)
#endif YP
#endif PIC
	struct netconfig	*tp;
	struct netbuf		*addr;
{
	struct sockaddr_in	*sa;		/* TCP/IP temporaries */
	struct servent		*se;
	struct hostent		*he;
	struct nd_hostservlist	*result;	/* Final result		*/
	struct nd_hostserv	*hs;		/* Pointer to the array */
	int			servs, hosts;	/* # of hosts, services */
	char			**hn, **sn;	/* host, service names */
	int			i, j;		/* some counters	*/

	if (!addr || !tp) {
		_nderror = ND_BADARG;
		return (NULL);
	}
	_nderror = ND_OK; /* assume success */

	/* XXX how much should we trust this ? */
	sa = (struct sockaddr_in *)(addr->buf);

	/* first determine the host */
#ifdef YP
	he = (struct hostent *)_nis_gethostbyaddr(&(sa->sin_addr.s_addr),
			4, sa->sin_family);
#else
	he = (struct hostent *)_tcpip_gethostbyaddr(&(sa->sin_addr.s_addr),
			4, sa->sin_family);
#endif YP
	if (!he) {
		_nderror = ND_NOHOST;
		return (NULL);
	}

	/* Now determine the service */
	if (sa->sin_port == 0) {
		/*
		 * The port number 0 is a reserved port for both UDP & TCP.
		 * We are assuming that this is used to just get
		 * the host name and to bypass the service name.
		 */
		servs = 1;
		se = NULL;
	} else {
#ifdef YP
		se = (struct servent *)_nis_getservbyport(sa->sin_port,
			(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");
#else
		se = (struct servent *)_tcpip_getservbyport(sa->sin_port,
			(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");
#endif YP
		if (!se) {
			/* It is not a well known service */
			servs = 1;
		}
	}

	/* now build the result for the client */
	result = (struct nd_hostservlist *)
			malloc(sizeof (struct nd_hostservlist));
	if (!result) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/*
	 * We initialize the counters to 1 rather than zero because
	 * we have to count the "official" name as well as the aliases.
	 */
	for (hn = he->h_aliases, hosts = 1; hn && *hn; hn++, hosts++)
		;

	if (se)
		for (sn = se->s_aliases, servs = 1; sn && *sn; sn++, servs++)
			;

	hs = (struct nd_hostserv *)calloc(hosts * servs,
			sizeof (struct nd_hostserv));
	if (!hs) {
		_nderror = ND_NOMEM;
		free((void *)result);
		return (NULL);
	}

	result->h_cnt	= servs * hosts;
	result->h_hostservs = hs;

	/* Now build the list of answers */

	for (i = 0, hn = he->h_aliases; i < hosts; i++) {
		sn = se ? se->s_aliases : NULL;

		for (j = 0; j < servs; j++) {
			if (! i)
				hs->h_host = strdup(he->h_name);
			else
				hs->h_host = strdup(*hn);
			if (! j) {
				if (se) {
					hs->h_serv = strdup(se->s_name);
				} else {
					/* Convert to a number string */
					char stmp[16];

					sprintf(stmp, "%d", sa->sin_port);
					hs->h_serv = strdup(stmp);
				}
			} else {
				hs->h_serv = strdup(*sn++);
			}

			if (!(hs->h_host) || !(hs->h_serv)) {
				_nderror = ND_NOMEM;
				free((void *)result->h_hostservs);
				free((void *)result);
				return (NULL);
			}
			hs ++;
		}
		if (i)
			hn++;
	}

	return (result);
}

/*
 * This internal routine will merge one of those "universal" addresses
 * to the one which will make sense to the remote caller.
 */
static char *
_netdir_mergeaddr(tp, ruaddr, uaddr)
	struct netconfig	*tp;	/* the transport provider */
	char			*ruaddr; /* remote uaddr of the caller */
	char			*uaddr;	/* the address */
{
	static char		**addrlist;
	char			tmp[32], nettmp[32];
	char			*hptr, *netptr, *portptr;
	int			i, j;
	struct in_addr		inaddr;
	int			index = 0, level = 0;

	if (!uaddr || !ruaddr || !tp) {
		_nderror = ND_BADARG;
		return ((char *)NULL);
	}
	if (strncmp(ruaddr, "0.0.0.0.", strlen("0.0.0.0.")) == 0)
		/* thats me: return the way it is */
		return (strdup(uaddr));

	if (addrlist == (char **)NULL) {
		struct utsname	u;
		struct hostent	*he;

		if (uname(&u) < 0) {
			_nderror = ND_NOHOST;
			return ((char *)NULL);
		}
#ifdef YP
		he = (struct hostent *)_nis_gethostbyname(u.nodename);
#else
		he = (struct hostent *)_tcpip_gethostbyname(u.nodename);
#endif YP
		if (he == NULL) {
			_nderror = ND_NOHOST;
			return ((char *)NULL);
		}
		/* make a list of all the legal host addrs */
		for (i = 0; he->h_addr_list[i]; i++)
			;
		if (i == 0) {
			_nderror = ND_NOHOST;
			return ((char *)NULL);
		}
		addrlist = (char **)malloc(sizeof (char *) * (i + 1));
		if (addrlist == NULL) {
			_nderror = ND_NOMEM;
			return ((char *)NULL);
		}
		addrlist[i] = NULL;
		for (i = 0; he->h_addr_list[i]; i++) {
			inaddr = *((struct in_addr *)he->h_addr_list[i]);
			addrlist[i] = strdup(inet_ntoa(inaddr));
		}
	}

	if (addrlist[1] == NULL) {
		/* Only one address. So, dont compare */
		i = 0;
		goto reply;
	}
	/* Get the host part of the remote uaddress assuming h.h.h.h.p.p */
	/* XXX: May be there is a better way of doing all this */
	(void) strcpy(tmp, ruaddr);
	for (hptr = tmp, j = 0; j < 4; j++) {
		hptr = strchr(hptr, '.');
		hptr++;
	}
	*(hptr - sizeof (char)) = NULL;

	/* class C networks - for gateways the common address is h.h.h */
	/* class B networks - for gateways the common address is h.h */
	/* class A networks - for gateways the common address is h */
	(void) strcpy(nettmp, tmp);
	netptr = strrchr(nettmp, '.');
	*netptr = NULL;
	netptr = nettmp;

	/* Compare and get the right one */
	for (i = 0; addrlist[i]; i++) {
		char *t1, *t2;

		if (strcmp(tmp, addrlist[i]) == 0)
			return (strdup(uaddr)); /* the same one */
		if (strncmp(netptr, addrlist[i], strlen(netptr)) == 0) {
			index = i;
			level = 3;
			break; /* A hit */
		}
		t1 = strrchr(netptr, '.');
		*t1 = NULL;
		if (strncmp(netptr, addrlist[i], strlen(netptr)) == 0) {
			index = i;
			level = 2;
			*t1 = '.';
			continue; /* A partial hit */
		}
		t2 = strrchr(netptr, '.');
		*t2 = NULL;
		if (strncmp(netptr, addrlist[i], strlen(netptr)) == 0) {
			if (level < 1) {
				index = i;
				level = 1;
			}
			*t1 = '.';
			*t2 = '.';
			continue; /* A partial hit */
		}
		*t1 = '.';
		*t2 = '.';
	}
reply:
	/* Get the port number */
	for (portptr = uaddr, j = 0; j < 4; j++) {
		portptr = strchr(portptr, '.');
		portptr++;
	}
	sprintf(tmp, "%s.%s", addrlist[index], portptr);
	return (strdup(tmp));
}

int
#ifdef PIC
_netdir_options(tp, opts, fd, par)
#else
#ifdef YP
nis_netdir_options(tp, opts, fd, par)
#else
tcp_netdir_options(tp, opts, fd, par)
#endif YP
#endif PIC
	struct netconfig *tp;
	int opts;
	int fd;
	char *par;
{
	struct t_optmgmt *options;
	struct t_optmgmt *optionsret;
	struct nd_mergearg *ma;

#ifndef SUNOS
	struct sochdr {
		struct opthdr opthdr;
		long value;
	} sochdr;
#endif

	switch (opts) {
	case ND_SET_BROADCAST:
		/* enable for broadcasting */
#ifndef SUNOS
		options = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, 0);
		if (options == (struct t_optmgmt *) NULL)
			return (ND_NOMEM);
		optionsret = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, T_OPT);
		if (optionsret == (struct t_optmgmt *) NULL) {
			(void) t_free((char *) options, T_OPTMGMT);
			return (ND_NOMEM);
		}
		sochdr.opthdr.level = SOL_SOCKET;
		sochdr.opthdr.name = SO_BROADCAST;
		sochdr.opthdr.len = 4;
		sochdr.value = 1;		/* ok to broadcast */
		options->opt.maxlen = sizeof (sochdr);
		options->opt.len = sizeof (sochdr);
		options->opt.buf =  (char *) &sochdr;
		options->flags = T_NEGOTIATE;
		if (t_optmgmt(fd, options, optionsret) == -1) {
			/*
			 *	Should we return an error here, or ignore it
			 *	in case the provider allows broadcasting but
			 *	doesn't know about this option?  For now, we
			 *	silently ignore the error.
			 */
		}
		options->opt.buf = (char *) NULL;
		(void) t_free((char *)options, T_OPTMGMT);
		(void) t_free((char *)optionsret, T_OPTMGMT);
#endif
		return (ND_OK);
	case ND_SET_RESERVEDPORT:	/* bind to a resered port */
		return (bindresvport(fd, (struct netbuf *)par));
	case ND_CHECK_RESERVEDPORT:	/* check if reserved prot */
		return (checkresvport((struct netbuf *)par));
	case ND_MERGEADDR:	/* Merge two addresses */
		ma = (struct nd_mergearg *)(par);
		ma->m_uaddr = _netdir_mergeaddr(tp, ma->c_uaddr, ma->s_uaddr);
		return (_nderror);
	default:
		return (ND_NOCTRL);
	}
}


/*
 * This internal routine will convert a TCP/IP internal format address
 * into a "universal" format address. In our case it prints out the
 * decimal dot equivalent. h1.h2.h3.h4.p1.p2 where h1-h4 are the host
 * address and p1-p2 are the port number.
 */
char *
#ifdef PIC
_taddr2uaddr(tp, addr)
#else
#ifdef YP
nis_taddr2uaddr(tp, addr)
#else
tcp_taddr2uaddr(tp, addr)
#endif YP
#endif PIC
	struct netconfig	*tp;	/* the transport provider */
	struct netbuf		*addr;	/* the netbuf struct */
{
	struct sockaddr_in	*sa;	/* our internal format */
	char			tmp[32];
	unsigned short		myport;

	if (!addr || !tp) {
		_nderror = ND_BADARG;
		return (NULL);
	}
	sa = (struct sockaddr_in *)(addr->buf);
	myport = ntohs(sa->sin_port);
	sprintf(tmp, "%s.%d.%d", inet_ntoa(sa->sin_addr),
			myport >> 8, myport & 255);
	return (strdup(tmp));	/* Doesn't return static data ! */
}

/*
 * This internal routine will convert one of those "universal" addresses
 * to the internal format used by the Sun TLI TCP/IP provider.
 */

struct netbuf *
#ifdef PIC
_uaddr2taddr(tp, addr)
#else
#ifdef YP
nis_uaddr2taddr(tp, addr)
#else
tcp_uaddr2taddr(tp, addr)
#endif YP
#endif PIC
	struct netconfig	*tp;	/* the transport provider */
	char			*addr;	/* the address */
{
	struct sockaddr_in	*sa;
	unsigned long		inaddr;
	unsigned short		inport;
	int			h1, h2, h3, h4, p1, p2;
	struct netbuf		*result;

	if (!addr || !tp) {
		_nderror = ND_BADARG;
		return (0);
	}
	result = (struct netbuf *) malloc(sizeof (struct netbuf));
	if (!result) {
		_nderror = ND_NOMEM;
		return (0);
	}

	sa = (struct sockaddr_in *)calloc(1, sizeof (*sa));
	if (!sa) {
		free((void *)result);
		_nderror = ND_NOMEM;
		return (0);
	}
	result->buf = (char *)(sa);
	result->maxlen = sizeof (struct sockaddr_in);
	result->len = sizeof (struct sockaddr_in);

	/* XXX there is probably a better way to do this. */
	sscanf(addr, "%d.%d.%d.%d.%d.%d", &h1, &h2, &h3, &h4, &p1, &p2);

	/* convert the host address first */
	inaddr = (h1 << 24) + (h2 << 16) + (h3 << 8) + h4;
	sa->sin_addr.s_addr = htonl(inaddr);

	/* convert the port */
	inport = (p1 << 8) + p2;
	sa->sin_port = htons(inport);

	sa->sin_family = AF_INET;

	return (result);
}



/* (#)inet_ntoa.c 1.7 88/02/08 SMI";  from UCB 4.1 83/06/12 */

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */
static char *
inet_ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}

static int
getbroadcastnets(tp, addrs)
	struct netconfig *tp;
	struct in_addr *addrs;
{
	struct ifconf ifc;
	struct ifreq ifreq, *ifr;
	struct sockaddr_in *sin;
	int fd;
	int n, i;
	char buf[8800];

	_nderror = ND_SYSTEM;
	fd = open(tp->nc_device, O_RDONLY);
	if (fd < 0) {
		(void) syslog(LOG_ERR,
		"broadcast: ioctl (get interface configuration): %m");
		return (0);
	}
	ifc.ifc_len = 8800;
	ifc.ifc_buf = buf;
	/*
	 * Ideally, this ioctl should also tell me, how many bytes were
	 * finally allocated, but it doesnt.
	 */
	if (ifioctl(fd, SIOCGIFCONF, buf, 8800) < 0) {
		(void) syslog(LOG_ERR,
		"broadcast: ioctl (get interface configuration): %m");
		close(fd);
		return (0);
	}
	ifr = (struct ifreq *)buf;
	for (i = 0, n = ifc.ifc_len/sizeof (struct ifreq);
		n > 0 && i < MAXBCAST; n--, ifr++) {
		ifreq = *ifr;
		if (ifioctl(fd, SIOCGIFFLAGS, (char *)&ifreq, 0) < 0) {
			(void) syslog(LOG_ERR,
			"broadcast: ioctl (get interface flags): %m");
			continue;
		}
		if ((ifreq.ifr_flags & IFF_BROADCAST) &&
		    (ifreq.ifr_flags & IFF_UP) &&
		    (ifr->ifr_addr.sa_family == AF_INET)) {
			sin = (struct sockaddr_in *)&ifr->ifr_addr;
			if (ifioctl(fd, SIOCGIFBRDADDR,
				(char *)&ifreq, 0) < 0) {
				/* May not work with other implementation */
				addrs[i++] = inet_makeaddr(inet_netof
					(sin->sin_addr.s_addr), INADDR_ANY);
			} else {
				addrs[i++] = ((struct sockaddr_in*)
						&ifreq.ifr_addr)->sin_addr;
			}
		}
	}
	close(fd);
	if (i)
		_nderror = ND_OK;
	return (i);
}

/* "@(#)inet_makeaddr.c 1.8 88/02/08 SMI"; from UCB 4.4 85/06/02 */

/*
 * Formulate an Internet address from network + host.  Used in
 * building addresses stored in the ifnet structure.
 */
static struct in_addr
inet_makeaddr(net, host)
	int net, host;
{
	u_long addr;

	if (net < 128)
		addr = (net << IN_CLASSA_NSHIFT) | (host & IN_CLASSA_HOST);
	else if (net < 65536)
		addr = (net << IN_CLASSB_NSHIFT) | (host & IN_CLASSB_HOST);
	else
		addr = (net << IN_CLASSC_NSHIFT) | (host & IN_CLASSC_HOST);
	addr = htonl(addr);
	return (*(struct in_addr *)&addr);
}

static
ifioctl(s, cmd, arg, len)
	char *arg;
{
	struct strioctl ioc;

	ioc.ic_cmd = cmd;
	ioc.ic_timout = 0;
	if (len)
		ioc.ic_len = len;
	else
		ioc.ic_len = sizeof (struct ifreq);
	ioc.ic_dp = arg;
	return (ioctl(s, I_STR, (char *) &ioc));
}

static
bindresvport(fd, addr)
	int fd;
	struct netbuf *addr;
{
	int res;
	static short port;
	struct sockaddr_in myaddr;
	struct sockaddr_in *sin;
	extern int errno;
	extern int t_errno;
	int i;
	struct t_bind *tbind, *tres;

#define	STARTPORT 600
#define	ENDPORT (IPPORT_RESERVED - 1)
#define	NPORTS	(ENDPORT - STARTPORT + 1)

	_nderror = ND_SYSTEM;
	if (geteuid()) {
		errno = EACCES;
		return (-1);
	}
	if ((i = t_getstate(fd)) != T_UNBND) {
		if (t_errno == TBADF)
			errno = EBADF;
		if (i != -1)
			errno = EISCONN;
		return (-1);
	}
	if (addr == NULL) {
		sin = &myaddr;
		(void)memset((char *)sin, 0, sizeof (*sin));
		sin->sin_family = AF_INET;
	} else {
		sin = (struct sockaddr_in *)addr->buf;
		if (sin->sin_family != AF_INET) {
			errno = EPFNOSUPPORT;
			return (-1);
		}
	}
	if (port == 0)
		port = (getpid() % NPORTS) + STARTPORT;
	res = -1;
	errno = EADDRINUSE;
	/* Transform sockaddr_in to netbuf */
	tbind = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tbind == NULL) {
		if (t_errno == TBADF)
			errno = EBADF;
		_nderror = ND_NOMEM;
		return (-1);
	}
	tres = (struct t_bind *)t_alloc(fd, T_BIND, T_ADDR);
	if (tres == NULL) {
		(void) t_free((char *)tbind, T_BIND);
		_nderror = ND_NOMEM;
		return (-1);
	}

	tbind->qlen = 0; /* Always 0; user should change if he wants to */
	(void) memcpy(tbind->addr.buf, (char *)sin, (int)tbind->addr.maxlen);
	tbind->addr.len = tbind->addr.maxlen;
	sin = (struct sockaddr_in *)tbind->addr.buf;

	for (i = 0; i < NPORTS && errno == EADDRINUSE; i++) {
		sin->sin_port = htons(port++);
		if (port > ENDPORT)
			port = STARTPORT;
		res = t_bind(fd, tbind, tres);
		if ((res == 0) && (memcmp(tbind->addr.buf, tres->addr.buf,
					(int)tres->addr.len) == 0))
			break;
	}

	(void) t_free((char *)tbind, T_BIND);
	(void) t_free((char *)tres, T_BIND);
	if (i != NPORTS) {
		_nderror = ND_OK;
	} else {
		_nderror = ND_FAILCTRL;
		res = 1;
	}
	return (res);
}

static
checkresvport(addr)
	struct netbuf *addr;
{
	struct sockaddr_in *sin;

	if (addr == NULL) {
		_nderror = ND_FAILCTRL;
		return (-1);
	}
	sin = (struct sockaddr_in *)addr->buf;
	if (sin->sin_port < IPPORT_RESERVED)
		return (0);
	return (1);
}
