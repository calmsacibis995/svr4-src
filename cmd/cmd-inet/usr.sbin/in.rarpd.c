/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.rarpd.c	1.3.2.1"

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
 * rarpd.c  Reverse-ARP server.
 * Refer to RFC 903 "A Reverse Address Resolution Protocol".
 */

/*
 * XXX
 * This file ported from the SunOS 4.1 version.
 * Things marked with "XXX" need fixing and will be
 * corrected soon.
 * TO DO:
 *  - add real delay code
 *  - get -a option working
 *  - resolve all XXX code
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/stropts.h>
#include <sys/dlpi.h>
#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <dirent.h>
#include <signal.h>
#include <netdb.h>

#ifdef SYSV
#define bzero(s,n)	  memset((s), 0, (n))
#define bcopy(a,b,c)	  memcpy(b,a,c)
#define bcmp(a,b,c)	  memcmp(b,a,c)
#endif /* SYSV */

#define	BOOTDIR		"/tftpboot"	/* boot files directory */
#define	DEVDIR		"/dev"		/* devices directory */
#define	DEVIP		"/dev/ip"	/* path to ip driver */
#define	DEVARP		"/dev/arp"	/* path to arp driver */

#define BUFSIZE		2048		/* max receive frame length */
#define	MAXPATHL	128		/* max path length */
#define	MAXHOSTL	128		/* max host name length */
#define	MAXADDRL	128		/* max address length */
#define	MAXIFS		16		/* max number configured interfaces */

#define	ETHERADDRL	sizeof (ether_addr_t)
#define	IPADDRL		sizeof (struct in_addr)

static int
	received,	/* total good packets read */
	bad,		/* packets not understood */
	unknown,	/* unknown ether -> ip address mapping */
	processed,	/* answer known and sent */
	delayed,	/* answer was delayed before sending */
	weird;		/* unexpected, yet valid */

static ether_addr_t my_etheraddr;

static int	dlfd;			/* datalink provider Stream */
static struct in_addr my_ipaddr;	/* in network order */
static char	*cmdname;		/* command name from av[0] */
static int	dflag = 0;		/* enable diagnostics */
static int	aflag = 0;		/* start rarpd on all interfaces */
static char *alarmmsg;			/* alarm() error message */

static u_long	if_netmask;		/* host order */
static u_long	if_ipaddr;		/* host order */
static u_long	if_netnum;		/* host order, with subnet */

static int	ipalloc ();		/* allocate IP address */

static void	rarp_request ();	/* RARP request handler */
static int	sigchld ();		/* child signal handler */

extern char	*malloc(), *inet_ntoa();
extern char *strcpy(), *strncpy();
extern struct dirent *readdir ();
extern u_long	inet_addr();
extern struct hostent	*gethostbyname();
extern char *ether_ntoa();
void sigalarm();

/*
 * Some datalink providers may pad output shorter than
 * the ethernet minimum frame size, others may not.
 * Rather than taking the time and code to get the
 * dl_min_sdu size from a DL_INFO_ACK, we just
 * make sure the data portions of all putmsg's
 * are at least ETHERMIN.
 */
char	dummy[ETHERMIN];

extern	int	optind;
extern	char	*optarg;

main(ac, av)
int ac;
char *av[];
{
	struct in_addr addr;
	struct hostent *hp;
	char *hostname;
	int	c;
	int	fd;
	struct ifconf ifc;
	struct ifreq reqbuf[MAXIFS];
	struct ifreq *ifr;
	int	n;

	cmdname = av[0];

	while ((c = getopt(ac, av, "ad")) != -1)
		switch (c) {
		case 'a':
			aflag = 1;
			break;

		case 'd':
			dflag = 1;
			break;

		default:
			usage();
		}

	if (aflag) {
/* XXX */
fprintf(stderr, "-a option not yet implemented\n");
exit(1);
		if (ac > 1)
			usage ();

		/*
		 * Open the IP provider.
		 */
		if ((fd = open(DEVIP, 0)) < 0)
			syserr(DEVIP);

		/*
		 * Ask IP for the list of configured interfaces.
		 */
		ifc.ifc_buf = (caddr_t) reqbuf;
		ifc.ifc_len = sizeof (reqbuf);
		if (strioctl(fd, SIOCGIFCONF, -1, sizeof (struct ifconf),
			(char*) &ifc) < 0)
			syserr("SIOCGIFCONF");

		/*
		 * Start a rarpd service on each of the configured interfaces.
		 */
		ifr = ifc.ifc_req;
		n = ifc.ifc_len / sizeof (struct ifreq);
		for (; n > 0 ; n--, ifr++) {
			if (ioctl (fd, SIOCGIFFLAGS, (char *) ifr) < 0) {
				perror ("ioctl SIOCGIFFLAGS");
				exit (1);
			}
			if ((ifr->ifr_flags & IFF_LOOPBACK) ||
			    !(ifr->ifr_flags & IFF_BROADCAST) ||
			    !(ifr->ifr_flags & IFF_UP) ||
			    (ifr->ifr_flags & IFF_NOARP) ||
			    (ifr->ifr_flags & IFF_POINTOPOINT))
				continue;

			addr.s_addr = 0;
			do_rarp(ifr->ifr_name,  addr);
		}
	}
	else switch (ac - optind) {
	case 1:	/* device only specified */
		addr.s_addr = 0;
		do_rarp(av[optind], addr);
		break;

	case 2:	/* device and hostname or IP address given */
		hostname = av[optind + 1];

		if ((hp = gethostbyname(hostname)) == NULL) {
			if ((addr.s_addr = inet_addr(hostname)) == -1)
				error("cannot get IP address for %s",
					hostname);
		}
		else {
			if (hp->h_length != IPADDRL)
				error("cannot find host entry for %s", hostname);
			else
				bcopy((char *) hp->h_addr, (char *) &addr.s_addr, IPADDRL);
		}
		do_rarp(av[optind], addr);
		break;
		
	default:
		usage();
	}

	exit(0);
	/* NOTREACHED */
}

do_rarp(device, addr)
char *device;
struct in_addr addr;
{
	char	ctlbuf[BUFSIZE];
	char	databuf[BUFSIZE];
	char	*cause;
	struct	ether_arp	ans;
	ether_addr_t	shost;
	struct strbuf ctl, data;
	int	flags;
	int	i;
	union	DL_primitives	*dlp;

	/*
	 * Open datalink provider and get our ethernet address.
	 */
	dlfd = rarp_open(device, htons(ETHERTYPE_REVARP), my_etheraddr);

	/*
	 * Get our IP address and netmask from directory service.
	 */
	get_ifdata (device, &if_ipaddr, &if_netmask);

	/* 
	 * Use IP address from hostname on the commandline, if the
	 * user supplied it.  Otherwise, use IP address from the
	 * interface.
	 */
	if (addr.s_addr == 0) {
		if_netnum = if_ipaddr & if_netmask;
		if_ipaddr = htonl (if_ipaddr);
		bcopy ((char *)&if_ipaddr, (char *) &my_ipaddr, IPADDRL);
		if_ipaddr = ntohl(if_ipaddr);
	} else {
		bcopy ((char *) &addr.s_addr, (char *) &my_ipaddr, IPADDRL);
		bcopy ((char *) &addr.s_addr, (char *) &if_netnum, sizeof (u_long));
		if_netnum &= if_netmask;
	}

	if (dflag)
		debug("starting rarp service on device %s address %s",
			 device, inet_ntoa(my_ipaddr));
		
	if (!dflag) {
		/*
		 * Background
		 */
		while (dlfd < 3) 
			dlfd = dup(dlfd);
		switch (fork ()) {
		case -1:			/* error */
			syserr("fork");

		case 0:				/* child */
			break;

		default:			/* parent */
			return;
		}
		for (i = 0; i < 3; i++) {
			(void) close(i);
		}
		(void) open("/", O_RDONLY, 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		/*
		 * Detach terminal
		 */
		if (setsid() < 0)
			syserr("setsid");
	}

	(void) openlog(cmdname, LOG_PID, LOG_DAEMON);

	/*
	 * read RARP packets and respond to them.
	 */
	while (1) {
		ctl.len = 0;
		ctl.maxlen = BUFSIZE;
		ctl.buf = ctlbuf;
		data.len = 0;
		data.maxlen = BUFSIZ;
		data.buf = databuf;
		flags = 0;
		if (getmsg(dlfd, &ctl, &data, &flags) < 0)
			syserr("getmsg");

		/*
		 * Validate DL_UNITDATA_IND.
		 */

		dlp = (union DL_primitives*) ctlbuf;

		bcopy (databuf, (char *) &ans, sizeof (struct ether_arp));

		cause = NULL;
		if (flags & MORECTL)
			cause = "MORECTL flag";
		else if (flags & MOREDATA)
			cause = "MOREDATA flag";
		else if (ctl.len == 0)
			cause = "missing control part of message";
		else if (ctl.len < 0)
			cause = "short control part of message";
		else if (dlp->dl_primitive != DL_UNITDATA_IND)
			cause = "not unitdata_ind";
		else if (ctl.len < DL_UNITDATA_IND_SIZE)
			cause = "short unitdata_ind";
		/* XXX would be nice to check the ether_type here */
		else if (data.len < sizeof (struct ether_arp))
			cause = "short ether_arp";
		else if (ans.arp_hrd != htons(ARPHRD_ETHER))
			cause = "hrd";
		else if (ans.arp_pro != htons(ETHERTYPE_IP))
			cause = "pro";
		else if (ans.arp_hln != ETHERADDRL)
			cause = "hln";
		else if (ans.arp_pln != IPADDRL)
			cause = "pln";
		if (cause) {
			if (dflag)
				debug("receive check failed: cause: %s",
					cause);
			bad++;
			continue;
		}

		/*
		 * Good request.
		 */
		received++;

		/*
		 * Pull out source ethernet address.
		 * XXX Assumes dl_src_addr is pure ethernet address.
		 *
		 * XXX BUG: k13 emd driver sends originating address
		 * up in dl_dest_addr and destination address (us)
		 * up in dl_src_addr.  Grrrr.
		 */
		bcopy((char *) ctlbuf + dlp->unitdata_ind.dl_dest_addr_offset,
			(char *) shost, ETHERADDRL);

		/*
		 * Handle the request.
		 */
		switch (ntohs(ans.arp_op)) {
		case REVARP_REQUEST:
			rarp_request(&ans, shost);
			break;

		case ARPOP_REQUEST:
			arp_request(&ans, shost);
			break;

		case REVARP_REPLY:
			if (dflag)
				debug("REVARP_REPLY ignored");
			break;

		case ARPOP_REPLY:
			if (dflag)
				debug("ARPOP_REPLY ignored");
			break;

		default:
			if (dflag)
				debug("unknown opcode 0x%x", ans.arp_op);
			bad++;
			break;
		}
	}
}

/* 
 * Reverse address determination and allocation code.
 */
static void
rarp_request (r, shost)
	struct ether_arp *r;
	ether_addr_t	shost;
{
	u_long tpa;

	if (dflag)
		debug("RARP_REQUEST for %s",
			 ether_ntoa(r->arp_tha));
    
	/* 
	 * third party lookups are rare and wonderful
	 */
	if (bcmp((char *) r->arp_sha, (char *) r->arp_tha, ETHERADDRL) || 
	    bcmp((char *) r->arp_sha, (char *) shost, ETHERADDRL)) {
		if (dflag)
			debug("weird (3rd party lookup)");
		weird++;
	}

	/*
	 * fill in given parts of reply packet
	 */
	bcopy ((char *) my_etheraddr, (char *) r->arp_sha, ETHERADDRL);
	bcopy ((char *) &my_ipaddr, (char *) r->arp_spa, IPADDRL);

	/*
	 * If a good address is stored in our lookup tables, return it
	 * immediately or after a delay.  Store it our kernel's ARP cache.
	 */
	if (get_ipaddr(r->arp_tha, r->arp_tpa) == 0) {
		add_arp((char *) r->arp_tpa, (char *) r->arp_tha);

		r->arp_op = htons (REVARP_REPLY);

		if (dflag) {
			struct in_addr addr;

			bcopy(r->arp_tpa, (char *) &addr, IPADDRL);
			debug("good lookup, maps to %s", inet_ntoa(addr));
		}

		/*
		 * If this is diskless and we're not its bootserver, let the
		 * bootserver reply first.  Load dependent performance hack.
		 */
		bcopy ((char *) r->arp_tpa, (char *)&tpa, IPADDRL);
		if (!mightboot(ntohl(tpa))) {
			/*
			 * XXX Replace this with the real hrt-based code
			 * once high resolution timers are working.
			 */
sleep(3);
			if (rarp_write(dlfd, r, shost) < 0)	/* XXX */
				syslog(LOG_ERR, "Bad rarp_write:  %m");
			if (dflag)
				debug("delayed reply sent");
			delayed++;
		} else {
			if (rarp_write(dlfd, r, shost) < 0)
				syslog(LOG_ERR, "Bad rarp_write:  %m");
			if (dflag)
				debug("immediate reply sent");
		}
		processed++;
		return;
	} else
		unknown++;
}

/*
 * down loads regular ARP entries to the kernel.
 * NB: Load even if it already exists
 */
static
add_arp(ipap, eap)
	char *ipap;  /* IP address pointer */
	char	*eap;
{
	struct arpreq ar;
	struct sockaddr_in	*sin;
	int	fd;

	/*
	 * Common part of query or set
	 */
	bzero((caddr_t)&ar, sizeof (ar));
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	bcopy((char*) ipap, (char*) &sin->sin_addr, IPADDRL);

	/*
	 * Open the IP provider.
	 */
	if ((fd = open(DEVARP, 0)) < 0)
		syserr(DEVIP);

	/*
	 * Set the entry
	 */
	bcopy((char*) eap, ar.arp_ha.sa_data, ETHERADDRL);
	ar.arp_flags = 0;
	strioctl(fd, SIOCDARP, -1, sizeof (struct arpreq), (char *) &ar);
	if (strioctl(fd, SIOCSARP, -1, sizeof (struct arpreq),
		(char *) &ar) < 0)
		syserr("SIOCSARP");

	(void) close(fd);
}

/*
 * The RARP spec says we must be able to process ARP requests,
 * even through the packet type is RARP.  Let's hope this feature
 * is not heavily used.
 */
static
arp_request(r, shost)
	struct ether_arp *r;
	ether_addr_t	shost;
{
	if (dflag)
		debug("ARPOP_REQUEST");

	if (bcmp((char *) &my_ipaddr, (char *) r->arp_tpa, IPADDRL))
		return;

	r->arp_op = ARPOP_REPLY;
	bcopy((char *) my_etheraddr, (char *) r->arp_sha, ETHERADDRL);
	bcopy((char *) &my_ipaddr, (char *) r->arp_spa, IPADDRL);
	bcopy((char *) my_etheraddr, (char *) r->arp_tha, ETHERADDRL);

	add_arp ((char *) r->arp_tpa, r->arp_tha);

	(void) rarp_write(dlfd, r, shost);
}

strioctl(fd, cmd, timout, len, dp)
int	fd;
int	cmd;
int	timout;
int	len;
char	*dp;
{
	struct	strioctl	si;

	si.ic_cmd = cmd;
	si.ic_timout = timout;
	si.ic_len = len;
	si.ic_dp = dp;
	return (ioctl(fd, I_STR, &si));
}

usage()
{
	extern	char	*cmdname;

	error("Usage:  %s [ -ad ] device [ hostname ]", cmdname);
}

syserr(s)
char	*s;
{
	extern	char	*cmdname;

	(void) fprintf(stderr, "%s:  ", cmdname);
	perror(s);
	syslog(LOG_ERR, s);
	exit(1);
}

/* VARARGS1 */
error(fmt, a1, a2, a3, a4)
char	*fmt, *a1, *a2, *a3, *a4;
{
	extern	char	*cmdname;

	(void) fprintf(stderr, "%s:  ", cmdname);
	(void) fprintf(stderr, fmt, a1, a2, a3, a4);
	(void) fprintf(stderr, "\n");
	syslog(LOG_ERR, fmt, a1, a2, a3, a4);
	exit(1);
}

/* VARARGS1 */
debug(fmt, a1, a2, a3, a4)
char	*fmt, *a1, *a2, *a3, *a4;
{
	extern	char	*cmdname;

	(void) fprintf(stderr, "%s:  ", cmdname);
	(void) fprintf(stderr, fmt, a1, a2, a3, a4);
	(void) fprintf(stderr, "\n");
}

/*
 * Open the datalink provider device and bind to the REVARP type.
 * Return the resulting descriptor.
 */
static int
rarp_open(device, type, e)
	char *device;
	u_short type;
	ether_addr_t	e;
{
	register int fd;
	char	path[MAXPATHL];
	union DL_primitives *dlp;
	char	buf[BUFSIZE];
	struct	strbuf	ctl, data;
	int	flags;

	(void) sprintf(path, "%s/%s", DEVDIR, device);

	/*
	 * Open the datalink provider.
	 */
	if ((fd = open(path, O_RDWR)) < 0)
		syserr(path);

	/*
	 * Issue DL_BIND_REQ
	 */
	dlp = (union DL_primitives*) buf;
	dlp->bind_req.dl_primitive = DL_BIND_REQ;
	dlp->bind_req.dl_sap = type;
	dlp->bind_req.dl_max_conind = 0;
	dlp->bind_req.dl_service_mode = DL_CLDLS;
	dlp->bind_req.dl_conn_mgmt = 0;

	ctl.buf = (char*) dlp;
	ctl.len = DL_BIND_REQ_SIZE;
	data.buf = dummy;
	data.len = ETHERMIN;	/* pad to min frame size */

	if (putmsg(fd, &ctl, &data, 0) < 0)
		syserr("putmsg");

	(void ) signal(SIGALRM, sigalarm);

	alarmmsg = "bind failed:  timeout waiting for bind acknowledgement";
	(void) alarm(10);

	ctl.buf = (char*) dlp;
	ctl.len = 0;
	ctl.maxlen = BUFSIZE;
	data.buf = NULL;
	data.len = 0;
	data.maxlen = 0;
	flags = 0;
	if (getmsg(fd, &ctl, &data, &flags) < 0)
		syserr("getmsg");

	(void) alarm(0);
	(void) signal(SIGALRM, SIG_DFL);

	/*
	 * Validate DL_BIND_ACK reply.
	 */
	if (ctl.len < sizeof (ulong))
		error("bind failed:  short reply to bind request");

	if (dlp->dl_primitive == DL_ERROR_ACK)
		error("bind failed:  dl_errno %d unix_errno %d",
			dlp->error_ack.dl_errno, dlp->error_ack.dl_unix_errno);

	if (dlp->dl_primitive != DL_BIND_ACK)
		error("bind failed:  unrecognizable dl_primitive %d received",
			dlp->dl_primitive);

	if (ctl.len < DL_BIND_ACK_SIZE)
		error("bind failed:  short bind acknowledgement received");

	if (dlp->bind_ack.dl_sap != type)
		error("bind failed:  returned dl_sap %d != requested sap %d",
			dlp->bind_ack.dl_sap, type);

	/*
	 * Save our ethernet address XXX
	 */
	bcopy((char *) (buf + dlp->bind_ack.dl_addr_offset),
		(char *) e, ETHERADDRL);

	if (dflag)
		debug("device %s ethernetaddress %s",
			device, ether_ntoa(e));

	return (fd);
}

static int
rarp_write(fd, r, dhost)
int	fd;
struct	ether_arp	*r;
ether_addr_t	dhost;
{
	struct	strbuf	ctl, data;
	union	DL_primitives	*dlp;
	char	ctlbuf[BUFSIZE];
	char	databuf[BUFSIZE];

	/*
	 * Construct DL_UNITDATA_REQ.
	 */
	dlp = (union DL_primitives*) ctlbuf;
	dlp->unitdata_req.dl_primitive = DL_UNITDATA_REQ;
	dlp->unitdata_req.dl_dest_addr_length = ETHERADDRL;
	dlp->unitdata_req.dl_dest_addr_offset = DL_UNITDATA_REQ_SIZE;
	/*
	 * XXX THIS IS A HACK THAT SHOULD GET FIXED IN A FUTURE
	 * K-LOAD OF SVR4.  THE DLSAP ADDRESS INCLUDES A
	 * PPA (MAC-ADDRESS) PART and a SAP (TYPE) PART
	 * BUT THE CURRENT EMD ETHERNET DRIVER IS BROKEN
	 * AND JUST USES THE SAP YOU'VE PREVIOUSLY BOUND AS
	 * THE ETHERNET TYPE FIELD.
	 * A FIX TO DLPI NEEDS TO BE MADE BEFORE THIS
	 * CAN BE CORRECTED.
	 *
	 * neal nuckolls
	 */
	bcopy((char *) dhost, (char*) ctlbuf+DL_UNITDATA_REQ_SIZE,
		ETHERADDRL);
	
	/*
	 * Send DL_UNITDATA_REQ.
	 */
	ctl.len = DL_UNITDATA_REQ_SIZE + ETHERADDRL;
	ctl.buf = (char*) dlp;
	bcopy((char *) r, databuf, sizeof (struct ether_arp));
	data.len = ETHERMIN;
	data.buf = databuf;
	return (putmsg(fd, &ctl, &data, 0));
}
 
/*
 * See if we have a TFTP boot file for this guy. Filenames in TFTP 
 * boot requests are of the form <ipaddr> for Sun-3's and of the form
 * <ipaddr>.<arch> for all other architectures.  Since we don't know
 * the client's architecture, either format will do.
 */
int
mightboot(ipa)
	u_long ipa;
{
	char path[MAXPATHL];
	DIR *dirp;
	struct dirent *dp;
(void) sprintf(path, "%s/%08X", BOOTDIR, ipa);

	/*
	 * Try a quick access() first.
	 */
	if (access(path, 0) == 0)
		return (1);

	/*
	 * Not there, do it the slow way by
	 * reading through the directory.
	 */

	(void) sprintf(path, "%08X", ipa);

	if (!(dirp = opendir(BOOTDIR)))
		return 0;

	while ((dp = readdir (dirp)) != (struct dirent *) 0) {
		if (strncmp(dp->d_name, path, 8) != 0)
			continue;
		if ((strlen(dp->d_name) != 8) && (dp->d_name[8] != '.'))
			continue;
		break;
	}
	
	(void) closedir (dirp);

	return (dp? 1: 0);
}

/*
 * Get our IP address and local netmask.
 */
get_ifdata (dev, ipp, maskp)
	char	*dev;
	u_long *ipp, *maskp;
{
	int	fd;
	struct	ifreq	ifr;
	struct	sockaddr_in	*sin = (struct sockaddr_in *) &ifr.ifr_addr;

	(void) strcpy(ifr.ifr_name, dev);

	/*
	 * Open the IP provider.
	 */
	if ((fd = open(DEVIP, 0)) < 0)
		syserr(DEVIP);

	/*
	 * Ask IP for our IP address.
	 */
	if (strioctl(fd, SIOCGIFADDR, -1, sizeof (struct ifreq),
		(char*) &ifr) < 0)
		syserr("SIOCGIFADDR");
	*ipp = ntohl(sin->sin_addr.s_addr);

	if (dflag)
		debug("device %s address %s",
			dev, inet_ntoa(sin->sin_addr.s_addr));

	/*
	 * Ask IP for our netmask.
	 */
	if (strioctl(fd, SIOCGIFNETMASK, -1, sizeof (struct ifreq),
		(char*) &ifr) < 0)
		syserr("SIOCGIFNETMASK");
	*maskp = ntohl(sin->sin_addr.s_addr);

	if (dflag)
		debug("device %s subnet mask %s",
			dev, inet_ntoa(sin->sin_addr.s_addr));

	/*
	 * Thankyou ip.
	 */
	(void) close (fd);
}

/*
 * Translate ethernet address to IP address.
 */
static int
get_ipaddr (e, ipp)
	ether_addr_t e;
	u_char *ipp;
{
	char host [MAXHOSTL];
	struct hostent *hp;
	struct in_addr addr;

	/* if no data stored, immediate failure */
	if (ether_ntohost(host, e) != 0
	    || !(hp = gethostbyname(host))
	    || hp->h_addrtype != AF_INET
	    || hp->h_length != IPADDRL) {
		if (dflag) 
			debug("could not map hardware address to IP address");
		return 1;
	}

	/* if stored addr is on the wrong net, ditto */
	bcopy ((char *) hp->h_addr, (char *) &addr, IPADDRL);
	if ((ntohl(addr.s_addr) & if_netmask) != if_netnum) {
		if (dflag) 
			debug("IP address is not on this net");
		return 1;
	}

	/* else return the stored address */
	bcopy((char *) hp->h_addr, (char *) ipp, IPADDRL);
	return 0;
}

void
sigalarm()
{
	extern	char	*alarmmsg;

	error(alarmmsg);
}
