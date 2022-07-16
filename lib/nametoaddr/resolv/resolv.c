/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nametoaddr:resolv/resolv.c	1.3.3.2"


#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
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
#ifdef SYSLOG
#include <sys/syslog.h>
#else
#define LOG_ERR 3
#endif /* SYSLOG */
#include "resolvabi.h"

#define	MAXALIASES	35

extern char	*malloc(), *calloc();
char	*_rs_inet_ntoa();
extern struct in_addr _rs_inet_makeaddr();
extern int	_nderror;

static char 	*localaddr[] = {"\000\000\000\000", NULL};

static struct hostent localent = {
		"Localhost",
		NULL,
		AF_INET,
		4,
		localaddr
};
#define MAXBCAST	10

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
_netdir_getbyname(tp, serv)
	struct netconfig *tp;
	struct nd_hostserv *serv;
{
	struct hostent	*he;
	struct hostent	h_broadcast;
	struct servent	*se;
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
	 * Find the port number for the service. If the service is
	 * not found, check for some special cases. These are :
	 *	rpcbind - The portmapper's address
	 *	A number - We don't have a name just a number so use it
	 */
	se = (struct servent *)_rs_getservbyname(serv->h_serv,
		(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");

	if (!se) {
		if (strcmp(serv->h_serv, "rpcbind") == 0) {
			server_port = 111;	/* Hard coded */
		} else if (strspn(serv->h_serv, "0123456789")
				== strlen(serv->h_serv)) {
			/* Its a port number */
			server_port = atoi(serv->h_serv);
		} else {
			_nderror = ND_NOSERV;
			return (NULL);
		}
	} else {
		server_port = se->s_port;
	}

	if ((strcmp(serv->h_host, HOST_SELF) == 0)) {
		he = &localent;
	} else if ((strcmp(serv->h_host, HOST_BROADCAST) == 0)) {
		char name[MAXHOSTNAMELEN];
		int bnets, i;

		memset((char *)inaddrs, 0, sizeof(struct in_addr) * MAXBCAST);
		bnets = _rs_getbroadcastnets(tp, inaddrs);
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
		he = (struct hostent *)_rs_gethostbyname(serv->h_host);
	}
	
	if (!he) {
		_nderror = ND_NOHOST;
		return (NULL);
	}

	result = (struct nd_addrlist *)(malloc(sizeof(struct nd_addrlist)));
	if (!result) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/* Count the number of addresses we have */
	for (num = 0, t = he->h_addr_list; *t; t++, num++) 
			;

	result->n_cnt = num;
	result->n_addrs = (struct netbuf *) 
				(calloc(num, sizeof(struct netbuf)));
	if (!result->n_addrs) {
		_nderror = ND_NOMEM;
		return (NULL);
	}

	/* build up netbuf structs for all addresses */
	for (na = result->n_addrs, t = he->h_addr_list; *t; t++, na++) {
		sa = (struct sockaddr_in *)calloc(1, sizeof(*sa));
		if (!sa) {
			_nderror = ND_NOMEM;
			return (NULL);
		}
		/* Vendor specific, that is why it's here and hard coded */
		na->maxlen = sizeof(struct sockaddr_in);
		na->len = sizeof(struct sockaddr_in);
		na->buf = (char *)sa;
		sa->sin_family = AF_INET;
		sa->sin_port = htons(server_port);
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
_netdir_getbyaddr(tp, addr)
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
	he = (struct hostent *)_rs_gethostbyaddr(&(sa->sin_addr.s_addr),
			4, sa->sin_family);
	if (!he) {
		_nderror = ND_NOHOST;
		return (NULL);
	}

	/* Now determine the service */
	se = (struct servent *)_rs_getservbyport(sa->sin_port,
		(strcmp(tp->nc_proto, NC_TCP) == 0) ? "tcp" : "udp");
	if (!se) {
		/* It is not a well known service */
		servs = 1;
	}

	/* now build the result for the client */
	result = (struct nd_hostservlist *)malloc(sizeof(struct nd_hostservlist));
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

	hs = (struct nd_hostserv *)calloc(hosts*servs, sizeof(struct nd_hostserv));
	if (!hs) {
		_nderror = ND_NOMEM;
		free((char *)result);
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

					(void) sprintf(stmp, "%d", sa->sin_port);
					hs->h_serv = strdup(stmp);
				}
			} else {
				hs->h_serv = strdup(*sn++);
			}

			if (!(hs->h_host) || !(hs->h_serv)) {
				_nderror = ND_NOMEM;
				free(result->h_hostservs); /* Free memory */
				free((char *)result);
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
	char			*ruaddr;/* remote uaddr of the caller */
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
		he = (struct hostent *)_rs_gethostbyname(u.nodename);
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
			addrlist[i] = strdup(_rs_inet_ntoa(inaddr));
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
	(void) sprintf(tmp, "%s.%s", addrlist[index], portptr);
	return (strdup(tmp));
}

int
_netdir_options(tp, opts, fd, par)
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
		optionsret = (struct t_optmgmt *)t_alloc(fd, T_OPTMGMT, T_OPT);
		if ((options == NULL) || (optionsret == NULL))
			return (ND_NOMEM);
		sochdr.opthdr.level = SOL_SOCKET;
		sochdr.opthdr.name = SO_BROADCAST;
		sochdr.opthdr.len = 4;
		sochdr.value = 1;		/* ok to broadcast */
		options->opt.maxlen = sizeof(sochdr);
		options->opt.len = sizeof(sochdr);
		options->opt.buf =  (char *) &sochdr;
		options->flags = T_NEGOTIATE;
		if (t_optmgmt(fd, options, optionsret) == -1) {
			/*
			 * maybe shouldn't quit  if some this just
			 * lets you broadcast
			 */
		}
		options->opt.buf = 0;
		(void) t_free((char *)options, T_OPTMGMT);
		(void) t_free((char *)optionsret, T_OPTMGMT);
#endif
		return (ND_OK);
	case ND_SET_RESERVEDPORT:	/* bind to a resered port */
		return (_rs_bindresvport(fd, (struct netbuf *)par));
	case ND_CHECK_RESERVEDPORT:	/* check if reserved prot */
		return (_rs_checkresvport((struct netbuf *)par));
	case ND_MERGEADDR:	/* Merge two addresses */
		ma = (struct nd_mergearg *)(par);
		ma->m_uaddr = _netdir_mergeaddr(tp, ma->c_uaddr, ma->s_uaddr);
		return(_nderror);
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
_taddr2uaddr(tp, addr)
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
	myport = _rs_ntohs(sa->sin_port);
	(void) sprintf(tmp,"%s.%d.%d", _rs_inet_ntoa(sa->sin_addr),
			myport >> 8, myport & 255);
	return (strdup(tmp));	/* Doesn't return static data ! */
}

/* 
 * This internal routine will convert one of those "universal" addresses
 * to the internal format used by the Sun TLI TCP/IP provider. 
 */

struct netbuf *
_uaddr2taddr(tp, addr)
	struct netconfig	*tp;	/* the transport provider */
	char			*addr;	/* the address 		 */
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
	result = (struct netbuf *) malloc(sizeof(struct netbuf));
	if (!result) {
		_nderror = ND_NOMEM;
		return (0);
	}

	sa = (struct sockaddr_in *)calloc(1, sizeof(*sa));
	if (!sa) {
		free((char *)result);		/* free previous result */
		_nderror = ND_NOMEM;
		return (0);
	}
	result->buf = (char *)(sa);
	result->maxlen = sizeof(struct sockaddr_in);
	result->len = sizeof(struct sockaddr_in);

	/* XXX there is probably a better way to do this. */
	sscanf(addr,"%d.%d.%d.%d.%d.%d", &h1, &h2, &h3, &h4, &p1, &p2);
	
	/* convert the host address first */
	inaddr = (h1 << 24) + (h2 << 16) + (h3 << 8) + h4;
	sa->sin_addr.s_addr = _rs_htonl(inaddr);
	
	/* convert the port */
	inport = (p1 << 8) + p2;
	sa->sin_port = _rs_htons(inport);

	sa->sin_family = AF_INET;
	
	return (result);
}



static
_rs_ifioctl(s, cmd, arg, len)
	char *arg;
{
	struct strioctl ioc;

	ioc.ic_cmd = cmd;
	ioc.ic_timout = 0;
	if (len)
		ioc.ic_len = len;
	else
		ioc.ic_len = sizeof(struct ifreq);
	ioc.ic_dp = arg;
	return (_rs_ioctl(s, I_STR, (char *) &ioc));
}


/*
 * The services routines.
 */

static struct servdata {
	FILE	*servf;
	char	*current;
	int	currentlen;
	int	stayopen;
	char	*serv_aliases[MAXALIASES];
	struct	servent serv;
	char	line[BUFSIZ+1];
} *servdata, *_rs__servdata();

extern struct servent *_rs_getservent();
static char SERVDB[] = "/etc/services";

static void _rs_setservent(), _rs_endservent();


static struct servdata *
_rs__servdata()
{
	register struct servdata *d = servdata;

	if (d == 0) {
		d = (struct servdata *)calloc(1, sizeof (struct servdata));
		servdata = d;
	}
	return (d);
}



static
_rs_checkresvport(addr)
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

/*
 * Convert network-format internet address
 * to base 256 d.d.d.d representation.
 */

char *
_rs_inet_ntoa(in)
	struct in_addr in;
{
	static char b[18];
	register char *p;

	p = (char *)&in;
#define	UC(b)	(((int)b)&0xff)
	(void) sprintf(b, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
	return (b);
}


static int
_rs_getbroadcastnets(tp, addrs)
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
		(void) syslog(LOG_ERR, "broadcast: ioctl (get interface configuration): %m");
		return (0);
	}
	ifc.ifc_len = 8800;
	ifc.ifc_buf = buf;
	/*
	 * Ideally, this ioctl should also tell me, how many bytes were
	 * finally allocated, but it doesnt.
	 */
	if (_rs_ifioctl(fd, SIOCGIFCONF, buf, 8800) < 0) {
		(void) syslog(LOG_ERR, "broadcast: ioctl (get interface configuration): %m");
		close(fd);
		return (0);
	}
	ifr = (struct ifreq *)buf;
	for (i = 0, n = ifc.ifc_len/sizeof (struct ifreq);
		n > 0 && i < MAXBCAST; n--, ifr++) {
		ifreq = *ifr;
		if (_rs_ifioctl(fd, SIOCGIFFLAGS, (char *)&ifreq, 0) < 0) {
			(void) syslog(LOG_ERR, "broadcast: ioctl (get interface flags): %m");
			continue;
		}
		if ((ifreq.ifr_flags & IFF_BROADCAST) &&
		    (ifreq.ifr_flags & IFF_UP) &&
		    (ifr->ifr_addr.sa_family == AF_INET)) {
			sin = (struct sockaddr_in *)&ifr->ifr_addr;
			if (_rs_ifioctl(fd, SIOCGIFBRDADDR, (char *)&ifreq, 0) < 0) {
				/* May not work with other implementation */
				addrs[i++] = _rs_inet_makeaddr(_rs_inet_netof(
					sin->sin_addr.s_addr), INADDR_ANY);
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

