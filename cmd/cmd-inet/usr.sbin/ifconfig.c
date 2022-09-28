/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/ifconfig/ifconfig.c	1.2.2.1"

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
 *
 * Copyright 1987, 1988 Lachman Associates, Incorporated (LAI) All Rights Reserved. 
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


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>

#include <net/if.h>
#include <netinet/in.h>
#include <stropts.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

extern int      errno;
struct ifreq    ifr;
struct sockaddr_in sin = {AF_INET};
struct sockaddr_in broadaddr;
struct sockaddr_in netmask = {AF_INET};
struct sockaddr_in ipdst = {AF_INET};
char            name[30];
int             flags;
int             metric;
int             setaddr;
int             setmask;
int             setbroadaddr;
int             setipdst;
int             s;

int             setifflags(), setifaddr(), setifdstaddr(), setifnetmask();
int             setifmetric(), setifbroadaddr(), setifipdst(), ifdetach();

#define	NEXTARG		0xffffff

struct cmd {
	char           *c_name;
	int             c_parameter;	/* NEXTARG means next argv */
	int             (*c_func) ();
}               cmds[] = {
	{
		                "up", IFF_UP, setifflags
	}              ,
	{
		                "down", -IFF_UP, setifflags
	}              ,
	{
		                "trailers", -IFF_NOTRAILERS, setifflags
	}              ,
	{
		                "-trailers", IFF_NOTRAILERS, setifflags
	}              ,
	{
		                "arp", -IFF_NOARP, setifflags
	}              ,
	{
		                "-arp", IFF_NOARP, setifflags
	}              ,
	{
		                "debug", IFF_DEBUG, setifflags
	}              ,
	{
		                "-debug", -IFF_DEBUG, setifflags
	}              ,
	{
		                "netmask", NEXTARG, setifnetmask
	}              ,
	{
		                "metric", NEXTARG, setifmetric
	}              ,
	{
		                "broadcast", NEXTARG, setifbroadaddr
	}              ,
	{
		                "ipdst", NEXTARG, setifipdst
	}              ,
	{
		                "detach", 0, ifdetach
	}              ,
	{
		                0, 0, setifaddr
	}              ,
	{
		                0, 0, setifdstaddr
	}              ,
};

int             in_status(), in_getaddr();

/* Known address families */
struct afswtch {
	char           *af_name;
	short           af_af;
	char           *af_dev;
	int             (*af_status) ();
	int             (*af_getaddr) ();
}               afs[] = {
	{
		                "inet", AF_INET, "/dev/ip", in_status, in_getaddr
	}              ,
	{
		                0, 0, 0, 0, 0
	}
};

struct afswtch *afp;		/* the address family being set or asked
				 * about */

main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             af = AF_INET;

	if (argc < 2) {
		usage();
	}
	argc--, argv++;
	strncpy(name, *argv, sizeof(name));
	argc--, argv++;
	if (argc > 0) {
		struct afswtch *myafp;

		for (myafp = afp = afs; myafp->af_name; myafp++)
			if (strcmp(myafp->af_name, *argv) == 0) {
				afp = myafp;
				argc--;
				argv++;
				break;
			}
		af = ifr.ifr_addr.sa_family = afp->af_af;
	} else {
		afp = afs;
	}
	s = open(afp->af_dev, O_RDONLY);
	if (s < 0) {
		perror("ifconfig: open");
		exit(1);
	}
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
	if (ifioctl(s, SIOCGIFFLAGS, (caddr_t) & ifr) < 0) {
		Perror("ioctl (SIOCGIFFLAGS)");
		exit(1);
	}
	flags = ifr.ifr_flags;
	if (ifioctl(s, SIOCGIFMETRIC, (caddr_t) & ifr) < 0)
		perror("ioctl (SIOCGIFMETRIC)");
	else
		metric = ifr.ifr_metric;
	if (argc == 0) {
		status();
		exit(0);
	}
	while (argc > 0) {
		register struct cmd *p;

		for (p = cmds; p->c_name; p++)
			if (strcmp(*argv, p->c_name) == 0)
				break;
		if (p->c_name == 0 && setaddr)
			p++;	/* got src, do dst */
		if (p->c_func) {
			if (p->c_parameter == NEXTARG) {
				(*p->c_func) (argv[1]);
				argc--, argv++;
			} else
				(*p->c_func) (*argv, p->c_parameter);
		}
		argc--, argv++;
	}
	if ((setmask || setaddr) && (af == AF_INET)) {
		/*
		 * If setting the address and not the mask, clear any
		 * existing mask and the kernel will then assign the default.
		 * If setting both, set the mask first, so the address will
		 * be interpreted correctly. 
		 */
		ifr.ifr_addr = *(struct sockaddr *) & netmask;
		if (ifioctl(s, SIOCSIFNETMASK, (caddr_t) & ifr) < 0)
			Perror("ioctl (SIOCSIFNETMASK)");
	}
	if (setaddr) {
		ifr.ifr_addr = *(struct sockaddr *) & sin;
		if (ifioctl(s, SIOCSIFADDR, (caddr_t) & ifr) < 0)
			Perror("ioctl (SIOCSIFADDR)");
	}
	if (setbroadaddr) {
		ifr.ifr_addr = *(struct sockaddr *) & broadaddr;
		if (ifioctl(s, SIOCSIFBRDADDR, (caddr_t) & ifr) < 0)
			Perror("ioctl (SIOCSIFBRDADDR)");
	}
	exit(0);
}

/* ARGSUSED */
setifaddr(addr, param)
	char           *addr;
	short           param;
{
	/*
	 * Delay the ioctl to set the interface addr until flags are all set.
	 * The address interpretation may depend on the flags, and the flags
	 * may change when the address is set. 
	 */
	setaddr++;
	(*afp->af_getaddr) (addr, &sin);
}

setifnetmask(addr)
	char           *addr;
{
	in_getaddr(addr, &netmask);
	setmask++;
}

setifbroadaddr(addr)
	char           *addr;
{
	(*afp->af_getaddr) (addr, &broadaddr);
	setbroadaddr++;
}

setifipdst(addr)
	char           *addr;
{
	in_getaddr(addr, &ipdst);
	setipdst++;
}

/* ARGSUSED */
setifdstaddr(addr, param)
	char           *addr;
	int             param;
{

	(*afp->af_getaddr) (addr, &ifr.ifr_addr);
	if (ifioctl(s, SIOCSIFDSTADDR, (caddr_t) & ifr) < 0)
		Perror("ioctl (SIOCSIFDSTADDR)");
}

setifflags(vname, value)
	char           *vname;
	short           value;
{
	if (ifioctl(s, SIOCGIFFLAGS, (caddr_t) & ifr) < 0) {
		Perror("ioctl (SIOCGIFFLAGS)");
		exit(1);
	}
	flags = ifr.ifr_flags;

	if (value < 0) {
		value = -value;
		flags &= ~value;
	} else
		flags |= value;
	ifr.ifr_flags = flags;
	if (ifioctl(s, SIOCSIFFLAGS, (caddr_t) & ifr) < 0)
		Perror(vname);
}

setifmetric(val)
	char           *val;
{
	ifr.ifr_metric = atoi(val);
	if (ifioctl(s, SIOCSIFMETRIC, (caddr_t) & ifr) < 0)
		perror("ioctl (set metric)");
}

ifdetach()
{
#ifdef NOTDEF
	if (ifioctl(s, SIOCIFDETACH, (caddr_t) & ifr) < 0)
		perror("ioctl (detach)");
#endif NOTDEF
}

#define	IFFBITS \
"\020\1UP\2BROADCAST\3DEBUG\4LOOPBACK\5POINTOPOINT\6NOTRAILERS\7RUNNING\10NOARP\
\011INTELLIGENT"

/*
 * Print the status of the interface.  If an address family was specified,
 * show it and it only; otherwise, show them all. 
 */
status()
{
	register struct afswtch *p = afp;
	short           af = ifr.ifr_addr.sa_family;

	printf("%s: ", name);
	printb("flags", flags, IFFBITS);
	if (metric)
		printf(" metric %d", metric);
	putchar('\n');
	if ((p = afp) != NULL) {
		(*p->af_status) (1);
	} else
		for (p = afs; p->af_name; p++) {
			ifr.ifr_addr.sa_family = p->af_af;
			afp = p;
			close(s);
			s = open(afp->af_dev, O_RDONLY);
			if (s < 0) {
				if (errno == EPROTONOSUPPORT || errno == ENODEV)
					continue;
				Perror("ifconfig: status open");
			}
			(*p->af_status) (0);
		}
}

in_status(force)
	int             force;
{
	struct sockaddr_in *sin;
	char           *inet_ntoa();

	if (ifioctl(s, SIOCGIFADDR, (caddr_t) & ifr) < 0) {
		if (errno == EADDRNOTAVAIL || errno == EPROTONOSUPPORT) {
			if (!force)
				return;
			memset((char *) &ifr.ifr_addr, '\0',
				sizeof(ifr.ifr_addr));
		} else
			perror("ioctl (SIOCGIFADDR)");
	}
	sin = (struct sockaddr_in *) & ifr.ifr_addr;
	printf("\tinet %s ", inet_ntoa(sin->sin_addr));
	if (ifioctl(s, SIOCGIFNETMASK, (caddr_t) & ifr) < 0) {
		if (errno != EADDRNOTAVAIL)
			perror("ioctl (SIOCGIFNETMASK)");
		memset((char *) &ifr.ifr_addr, '\0', sizeof(ifr.ifr_addr));
	} else
		netmask.sin_addr =
			((struct sockaddr_in *) & ifr.ifr_addr)->sin_addr;
	if (flags & IFF_POINTOPOINT) {
		if (ifioctl(s, SIOCGIFDSTADDR, (caddr_t) & ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
				memset((char *) &ifr.ifr_addr,
				       '\0', sizeof(ifr.ifr_addr));
			else
				perror("ioctl (SIOCGIFDSTADDR)");
		}
		sin = (struct sockaddr_in *) & ifr.ifr_addr;
		printf("--> %s ", inet_ntoa(sin->sin_addr));
	}
	printf("netmask %x ", ntohl(netmask.sin_addr.s_addr));
	if (flags & IFF_BROADCAST) {
		if (ifioctl(s, SIOCGIFBRDADDR, (caddr_t) & ifr) < 0) {
			if (errno == EADDRNOTAVAIL)
				memset((char *) &ifr.ifr_addr,
				       '\0', sizeof(ifr.ifr_addr));
			else
				perror("ioctl (SIOCGIFADDR)");
		}
		sin = (struct sockaddr_in *) & ifr.ifr_addr;
		if (sin->sin_addr.s_addr != 0)
			printf("broadcast %s", inet_ntoa(sin->sin_addr));
	}
	putchar('\n');
}

Perror(cmd)
	char           *cmd;
{
	extern int      errno;

	fprintf(stderr, "ifconfig: ");
	switch (errno) {

	case ENXIO:
		fprintf(stderr, "%s: no such interface\n", cmd);
		break;

	case EPERM:
		fprintf(stderr, "%s: permission denied\n", cmd);
		break;

	case ENODEV:
		fprintf(stderr, "%s: family not loaded\n", cmd);
		break;

	default:
		perror(cmd);
	}
	exit(1);
}

struct in_addr  inet_makeaddr();

in_getaddr(s, saddr)
	char           *s;
	struct sockaddr *saddr;
{
	register struct sockaddr_in *sin = (struct sockaddr_in *) saddr;
	struct hostent *hp;
	struct netent  *np;
	int             val;

	if (s == NULL) {
		fprintf(stderr, "ifconfig: address argument required\n");
		usage();
	}
	sin->sin_family = AF_INET;
	val = inet_addr(s);
	if (val != -1) {
		sin->sin_addr.s_addr = val;
		return;
	}
	hp = gethostbyname(s);
	if (hp) {
		sin->sin_family = hp->h_addrtype;
		memcpy((char *) &sin->sin_addr, hp->h_addr, hp->h_length);
		return;
	}
	np = getnetbyname(s);
	if (np) {
		sin->sin_family = np->n_addrtype;
		sin->sin_addr = inet_makeaddr(np->n_net, INADDR_ANY);
		return;
	}
	fprintf(stderr, "ifconfig: %s: bad value\n", s);
	exit(1);
}

/*
 * Print a value a la the %b format of the kernel's printf 
 */
printb(s, v, bits)
	char           *s;
	register char  *bits;
	register unsigned short v;
{
	register int    i, any = 0;
	register char   c;

	if (bits && *bits == 8)
		printf("%s=%o", s, v);
	else
		printf("%s=%x", s, v);
	bits++;
	if (bits) {
		putchar('<');
		while (i = *bits++) {
			if (v & (1 << (i - 1))) {
				if (any)
					putchar(',');
				any = 1;
				for (; (c = *bits) > 32; bits++)
					putchar(c);
			} else
				for (; *bits > 32; bits++);
		}
		putchar('>');
	}
}

usage()
{
	fprintf(stderr, "usage: ifconfig interface\n%s%s%s%s",
		"\t[ af [ address [ dest_addr ] ] [ up ] [ down ]",
		"[ netmask mask ] ]\n",
		"\t[ metric n ]\n",
		"\t[ trailers | -trailers ]\n",
		"\t[ arp | -arp ]\n");
	exit(1);
}

ifioctl(s, cmd, arg)
	char           *arg;
{
	struct strioctl ioc;

	ioc.ic_cmd = cmd;
	ioc.ic_timout = 0;
	ioc.ic_len = sizeof(struct ifreq);
	ioc.ic_dp = arg;
	return (ioctl(s, I_STR, (char *) &ioc));
}
