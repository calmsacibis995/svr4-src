/*
 *	Copyrighted as an unpublished work.
 *      (c) Copyright 1990 INTERACTIVE Systems Corporation
 *      All Rights Reserved.
 *
 *	Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */

/*
static char sccsid[] = "bootp.c	1.1 (Stanford) 1/22/86";
static char rcsid[] = "$Header: bootpd.c,v 1.20 89/01/10 15:04:43 ww0n Exp $";
*/


/*
 * BOOTP (bootstrap protocol) server daemon.
 *
 * Answers BOOTP request packets from booting client machines.
 * See [SRI-NIC]<RFC>RFC951.TXT for a description of the protocol.
 * See [SRI-NIC]<RFC>RFC1048.TXT for vendor-information extensions.
 * See accompanying man page -- bootpd.8
 *
 *
 * Copyright 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon and Stanford
 * University.  Carnegie Mellon makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 *
 * HISTORY
 *
 * 01/22/86	Bill Croft at Stanford University
 *		    Created.
 *
 * 07/30/86     David Kovar at Carnegie Mellon University
 *		    Modified to work at CMU.
 *
 * 07/24/87	Drew D. Perkins at Carnegie Mellon University
 *		    Modified to use syslog instead of Kovar's
 *		    routines.  Add debugging dumps.  Many other fixups.
 *
 * 07/15/88	Walter L. Wimer at Carnegie Mellon University
 *		    Added vendor information to conform to RFC1048.
 *		    Adopted termcap-like file format to support above.
 *		    Added hash table lookup instead of linear search.
 *		    Other cleanups.
 *
 *
 * BUGS
 *
 * Currently mallocs memory in a very haphazard manner.  As such, most of
 * the program ends up core-resident all the time just to follow all the
 * stupid pointers around. . . .
 *
 */




#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <net/if.h>
#if defined(SUNOS40) || defined(SYSV)
#include <sys/sockio.h>
#include <net/if_arp.h>
#endif
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#ifndef ARP_IOCTL_FIXED
#include <stropts.h>
#endif

#ifdef SYSLOG
#include <syslog.h>
#endif
#include "bootp.h"
#include "hash.h"
#include "bootpd.h"

#define HASHTABLESIZE		257	/* Hash table size (prime) */
#define DEFAULT_TIMEOUT		 15L	/* Default timeout in minutes */

#ifndef CONFIG_FILE
#define CONFIG_FILE		"/etc/bootptab"
#endif
#ifndef DUMP_FILE
#define DUMP_FILE		"/etc/bootpd.dump"
#endif



/*
 * Externals, forward declarations, and global variables
 */

char Version[] = "bootpd 2.1\n";

extern char *sys_errlist[];
extern int  errno, sys_nerr;

void usage();
void insert_u_long();
void dump_host();
void list_ipaddresses();
#ifdef VEND_CMU
void dovend_cmu();
#endif
void dovend_rfc1048();
boolean hwlookcmp();
boolean iplookcmp();
void insert_generic();
void insert_ip();
int dumptab();
int chk_access();
void report();
char *get_errmsg();

/*
 * IP port numbers for client and server obtained from /etc/services
 */

u_short bootps_port, bootpc_port;


/*
 * Internet socket and interface config structures
 */

struct sockaddr_in sin;
struct sockaddr_in from;	/* Packet source */
struct ifreq ifreq[10];		/* Holds interface configuration */
struct ifconf ifconf;		/* Int. config ioctl block (pnts to ifreq) */
struct arpreq arpreq;		/* Arp request ioctl block */


/*
 * General
 */

int debug = 0;			    /* Debugging flag */
int s;				    /* Socket file descriptor */
byte buf[1024];			    /* Receive packet buffer */
struct timezone tzp;		    /* Time zone offset for clients */
struct timeval tp;		    /* Time (extra baggage) */
long secondswest;		    /* Time zone offset in seconds */

/*
 * Globals below are associated with the bootp database file (bootptab).
 */

char *bootptab = NULL;
#ifdef DEBUG
char *bootpd_dump = NULL;
#endif



/*
 * Vendor magic cookies for CMU and RFC1048
 */

unsigned char vm_cmu[4]	    = VM_CMU;
unsigned char vm_rfc1048[4] = VM_RFC1048;


/*
 * Hardware address lengths (in bytes) based on hardware type code.
 * List in order specified by Assigned Numbers RFC; Array index is
 * hardware type code.  Entries marked as zero are unknown to the author
 * at this time. . . .
 */

unsigned maphaddrlen[MAXHTYPES + 1] = {
     0,	    /* Type 0:	Reserved (don't use this)   */
     6,	    /* Type 1:  10Mb Ethernet (48 bits)	    */
     1,	    /* Type 2:   3Mb Ethernet (8 bits)	    */
     0,	    /* Type 3:  Amateur Radio AX.25	    */
     1,	    /* Type 4:  Proteon ProNET Token Ring   */
     0,	    /* Type 5:  Chaos			    */
     6,	    /* Type 6:  IEEE 802 Networks	    */
     0	    /* Type 7:  ARCNET			    */
};


/*
 * Main hash tables
 */

hash_tbl *hwhashtable;
hash_tbl *iphashtable;
hash_tbl *nmhashtable;




/*
 * Initialization such as command-line processing is done and then the main
 * server loop is started.
 */

main(argc, argv)
    int argc;
    char **argv;
{
    struct timeval actualtimeout, *timeout;
    struct bootp *bp = (struct bootp *) buf;
    struct servent *servp;
    char *stmp;
    int n, tolen, fromlen;
    int nfound, readfds;
    int standalone;

    stmp = NULL;
    standalone = FALSE;
    actualtimeout.tv_usec = 0L;
    actualtimeout.tv_sec  = 60 * DEFAULT_TIMEOUT;
    timeout = &actualtimeout;

    /*
     * Read switches.
     */
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (argv[0][0] == '-') {
	    switch (argv[0][1]) {
		case 't':
		    if (argv[0][2]) {
			stmp = &(argv[0][2]);
		    } else {
			argc--;
			argv++;
			stmp = argv[0];
		    }
		    if (!stmp || (sscanf(stmp, "%d", &n) != 1) || (n < 0)) {
			fprintf(stderr,
				"bootpd: invalid timeout specification\n");
			break;
		    }
		    actualtimeout.tv_sec = (long) (60 * n);
		    /*
		     * If the actual timeout is zero, pass a NULL pointer
		     * to select so it blocks indefinitely, otherwise,
		     * point to the actual timeout value.
		     */
		    timeout = (n > 0) ? &actualtimeout : NULL;
		    break;
		case 'd':
		    debug++;
		    break;
		case 's':
		    standalone = TRUE;
		    break;
		default:
		    fprintf(stderr,
			    "bootpd: unknown switch: -%c\n",
			    argv[0][1]);
		    usage();
		    break;
	    }
	} else {
	    if (!bootptab) {
		bootptab = argv[0];
#ifdef DEBUG
	    } else if (!bootpd_dump) {
		bootpd_dump = argv[0];
#endif
	    } else {
		fprintf(stderr, "bootpd: unknown argument: %s\n", argv[0]);
		usage();
	    }
	}
    }

    /*
     * Set default file names if not specified on command line
     */
    if (!bootptab) {
	bootptab = CONFIG_FILE;
    }
#ifdef DEBUG
    if (!bootpd_dump) {
	bootpd_dump = DUMP_FILE;
    }
#endif

    /*
     * Get our timezone offset so we can give it to clients if the
     * configuration file doesn't specify one.
     */
    if (gettimeofday(&tp, &tzp) < 0) {
	secondswest = 0L;	/* Assume GMT for lack of anything better */
	report(LOG_ERR, "gettimeofday: %s\n", get_errmsg());
    } else {
	secondswest = 60L * tzp.tz_minuteswest;	    /* Convert to seconds */
    }

    /*
     * Allocate hash tables for hardware address, ip address, and hostname
     */
    hwhashtable = hash_Init(HASHTABLESIZE);
    iphashtable = hash_Init(HASHTABLESIZE);
    nmhashtable = hash_Init(HASHTABLESIZE);
    if (!(hwhashtable && iphashtable && nmhashtable)) {
	fprintf(stderr, "Unable to allocate hash tables.\n");
	exit(1);
    }

    if (standalone) {
	/*
	 * Go into background and disassociate from controlling terminal.
	 */
	if (debug < 3) {
	    if (fork())
		exit(0);
	    for (n = 0; n < 10; n++)
		(void) close(n);
	    (void) open("/", O_RDONLY);
	    (void) dup2(0, 1);
	    (void) dup2(0, 2);
	    n = open("/dev/tty", O_RDWR);
	    if (n >= 0) {
		ioctl(n, TIOCNOTTY, (char *) 0);
		(void) close(n);
	    }
	}
	/*
	 * Nuke any timeout value
	 */
	timeout = NULL;
    }


#ifdef SYSLOG
    /*
     * Initialize logging.
     */
#ifndef LOG_CONS
#define LOG_CONS	0	/* Don't bother if not defined... */
#endif
#ifndef LOG_DAEMON
#define LOG_DAEMON	0
#endif
    openlog("bootpd", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "%s", Version);
#endif

    if (standalone) {
	/*
	 * Get us a socket.
	 */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
	    report(LOG_ERR, "socket: %s\n", get_network_errmsg());
	    exit(1);
	}

	/*
	 * Get server's listening port number
	 */
	servp = getservbyname("bootps", "udp");
	if (servp) {
	    bootps_port = ntohs((u_short) servp->s_port);
	} else {
	    report(LOG_ERR,
		   "udp/bootps: unknown service -- assuming port %d\n",
		   IPPORT_BOOTPS);
	    bootps_port = (u_short) IPPORT_BOOTPS;
	}

	/*
	 * Bind socket to BOOTPS port.
	 */
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(bootps_port);
	if (bind(s, &sin, sizeof(sin)) < 0) {
	    report(LOG_ERR, "bind: %s\n", get_network_errmsg());
	    exit(1);
	}
    } else {
	/*
	 * Assume socket was passed to us from inetd
	 */
	s = 0;
	tolen = sizeof(sin);
	memset((char *) &sin, 0, tolen);
	if (getsockname(s, &sin, &tolen) < 0) {
	    report(LOG_ERR, "getsockname: %s\n", get_network_errmsg());
	    exit(1);
	}
	bootps_port = ntohs(sin.sin_port);
    }

    /*
     * Get destination port number so we can reply to client
     */
    servp = getservbyname("bootpc", "udp");
    if (servp) {
	bootpc_port = ntohs(servp->s_port);
    } else {
	report(LOG_ERR,
	       "udp/bootpc: unknown service -- assuming port %d\n",
		IPPORT_BOOTPC);
	bootpc_port = (u_short) IPPORT_BOOTPC;
    }


    /*
     * Determine network configuration.
     */
    ifconf.ifc_len = sizeof(ifreq);
    ifconf.ifc_req = ifreq;
    if ((ipioctl(s, SIOCGIFCONF, (caddr_t) &ifconf) < 0) ||
	(ifconf.ifc_len <= 0)) {
	    report(LOG_ERR, "ioctl: %s\n", get_network_errmsg());
	    exit(1);
    }

    /*
     * Read the bootptab file once immediately upon startup.
     */
    readtab();

    /*
     * Set up signals to read or dump the table.
     */
    if ((int) sigset(SIGHUP, (void (*)()) readtab) < 0) {
	report(LOG_ERR, "signal: %s\n", get_errmsg());
	exit(1);
    }
#ifdef DEBUG
    if ((int) sigset(SIGUSR1, (void (*)()) dumptab) < 0) {
	report(LOG_ERR, "signal: %s\n", get_errmsg());
	exit(1);
    }
#endif

    /*
     * Process incoming requests.
     */
    for (;;) {
	readfds = 1 << s;
	nfound = select(s + 1, &readfds, NULL, NULL, timeout);
	if (nfound < 0) {
	    if (errno != EINTR) {
		report(LOG_ERR, "select: %s\n", get_errmsg());
	    }
	    continue;
	}
	if (!(readfds & (1 << s))) {
	    report(LOG_INFO, "exiting after %ld minutes of inactivity\n",
		   actualtimeout.tv_sec / 60);
	    exit(0);
	}
	fromlen = sizeof(from);
	n = recvfrom(s, buf, sizeof(buf), 0, &from, &fromlen);
	if (n <= 0) {
	    continue;
	}

	if (n < sizeof(struct bootp)) {
	    if (debug) {
		report(LOG_INFO, "received short packet\n");
	    }
	    continue;
	}

	readtab();	/* maybe re-read bootptab */
	switch (bp->bp_op) {
	    case BOOTREQUEST:
		request();
		break;

	    case BOOTREPLY:
		reply();
		break;
	}
    }
}




/*
 * Print "usage" message and exit
 */

void usage()
{
    fprintf(stderr,
	   "usage:  bootpd [-d] [-s] [-t timeout] [configfile [dumpfile]]\n");
    fprintf(stderr, "\t -d\tincrease debug verbosity\n");
    fprintf(stderr, "\t -s\trun standalone (without inetd)\n");
    fprintf(stderr, "\t -t n\tset inetd exit timeout to n minutes\n");
    exit(1);
}



/*
 * Process BOOTREQUEST packet.
 *
 * (Note, this version of the bootpd.c server never forwards 
 * the request to another server.  In our environment the 
 * stand-alone gateways perform that function.)
 *
 * (Also this version does not interpret the hostname field of
 * the request packet;  it COULD do a name->address lookup and
 * forward the request there.)
 */
request()
{
    register struct bootp *bp = (struct bootp *) buf;
    register struct host *hp;
    register int n;
    char path[128];
    struct host dummyhost;
    long bootsize;
    unsigned hlen, hashcode;
    char **addr;
    struct hostent *hdbp;
    struct ifreq *ifrq;

    bp->bp_op = BOOTREPLY;

    if (bp->bp_sname[0] && (hdbp = gethostbyname(bp->bp_sname))) {
    	ifrq = &ifreq[0];
    	n = ifconf.ifc_len;
    	for (; n > 0; n -= sizeof(ifreq[0]), ifrq++) {
    		for (addr = hdbp->h_addr_list; *addr; addr++)
    			if (nmatch(*addr, &((struct sockaddr_in *)
    				   	  (&ifrq->ifr_addr))->sin_addr) == 4)	
    				break;
    		if (*addr)
    			break;
    	}
    	if (n <= 0) {
		if (debug)
			report(LOG_INFO, "%s request not for us\n",
								bp->bp_sname);
    		return;
	}
    }
    if (bp->bp_ciaddr.s_addr == 0) { 
	/*
	 * client doesnt know his IP address, 
	 * search by hardware address.
	 */
	if (debug) {
	    report(LOG_INFO, "request from hardware address %s\n",
		    haddrtoa(bp->bp_chaddr, bp->bp_htype));
	}

	dummyhost.htype = bp->bp_htype;
	hlen = haddrlength(bp->bp_htype);
	memcpy(dummyhost.haddr, bp->bp_chaddr, hlen);
	hashcode = hash_HashFunction(bp->bp_chaddr, hlen);
	hp = (struct host *) hash_Lookup(hwhashtable, hashcode, hwlookcmp,
					 &dummyhost);
	if (hp == NULL) {
	    report(LOG_NOTICE, "hardware address not found: %s\n",
		    haddrtoa(bp->bp_chaddr, bp->bp_htype));
	    return;	/* not found */
	}
	(bp->bp_yiaddr).s_addr = hp->iaddr.s_addr;

    } else {

	/*
	 * search by IP address.
	 */
	if (debug) {
	    report(LOG_INFO, "request from IP addr %s\n",
		    inet_ntoa(bp->bp_ciaddr));
	}
	dummyhost.iaddr.s_addr = bp->bp_ciaddr.s_addr;
	hashcode = hash_HashFunction((char *) &(bp->bp_ciaddr.s_addr), 4);
	hp = (struct host *) hash_Lookup(iphashtable, hashcode, iplookcmp,
					 &dummyhost);
	if (hp == NULL) {
	    report(LOG_NOTICE,
		    "IP address not found: %s\n", inet_ntoa(bp->bp_ciaddr));
	    return;
	}
    }

    if (debug) {
	report(LOG_INFO, "found %s %s\n", inet_ntoa(hp->iaddr.s_addr),
		hp->hostname->string);
    }

    /*
     * Fill in the client's proper bootfile.
     *
     * If the client specifies an absolute path, try that file with a
     * ".host" suffix and then without.  If the file cannot be found, no
     * reply is made at all.
     *
     * If the client specifies a null or relative file, use the following
     * table to determine the appropriate action:
     *
     *  Homedir      Bootfile    Client's file
     * specified?   specified?   specification   Action
     * -------------------------------------------------------------------
     *      No          No          Null         Send null filename
     *      No          No          Relative     Discard request
     *      No          Yes         Null         Send if absolute else null
     *      No          Yes         Relative     Discard request
     *      Yes         No          Null         Send null filename
     *      Yes         No          Relative     Lookup with ".host"
     *      Yes         Yes         Null         Send home/boot or bootfile
     *      Yes         Yes         Relative     Lookup with ".host"
     *
     */

    /*
     * Who knows. . . . . vestigial code (from Stanford?) which should
     * probably be axed.
     */
    if (strcmp((char *) bp->bp_file, "sunboot14") == 0)
	bp->bp_file[0] = 0;	/* pretend it's null */

    /*
     * This is the code which actually implements the above truth table.
     */
    if (bp->bp_file[0]) {
	/*
	 * The client specified a file.
	 */
	if (bp->bp_file[0] == '/') {
	    strcpy(path, (char *) bp->bp_file);		/* Absolute pathname */
	} else {
	    if (hp->flags.homedir) {
		strcpy(path, hp->homedir->string);
		strcat(path, "/");
		strcat(path, (char *) bp->bp_file);
	    } else {
		report(LOG_NOTICE,
		    "requested file \"%s\" not found: hd unspecified\n",
		    bp->bp_file);
		return;
	    }
	}
    } else {
	/*
	 * No file specified by the client.
	 */
	if (hp->flags.bootfile && ((hp->bootfile->string)[0] == '/')) {
	    strcpy(path, hp->bootfile->string);
	} else if (hp->flags.homedir && hp->flags.bootfile) {
	    strcpy(path, hp->homedir->string);
	    strcat(path, "/");
	    strcat(path, hp->bootfile->string);
	} else {
	    memset(bp->bp_file, 0, sizeof(bp->bp_file));
	    goto skip_file;	/* Don't bother trying to access the file */
	}
    }

    /*
     * First try to find the file with a ".host" suffix
     */
    n = strlen(path);
    strcat(path, ".");
    strcat(path, hp->hostname->string);
    if (chk_access(path, &bootsize) < 0) {
	path[n] = 0;			/* Try it without the suffix */
	if (chk_access(path, &bootsize) < 0) {
	    if (bp->bp_file[0]) {
		/*
		 * Client wanted specific file
		 * and we didn't have it.
		 */
		report(LOG_NOTICE,
			"requested file not found: \"%s\"\n", path);
		return;
	    } else {
		/*
		 * Client didn't ask for a specific file and we couldn't
		 * access the default file, so just zero-out the bootfile
		 * field in the packet and continue processing the reply.
		 */
		memset(bp->bp_file, 0,  sizeof(bp->bp_file));
		goto skip_file;
	    }
	}
    }
    strcpy((char *) bp->bp_file, path);

skip_file:  ;


    /*
     * Zero the vendor information area, except for the magic cookie.
     */
    memset((bp->bp_vend) + 4, 0,  (sizeof(bp->bp_vend)) - 4);

    if (debug > 1) {
	report(LOG_INFO, "vendor magic field is %d.%d.%d.%d\n",
		(int) ((bp->bp_vend)[0]),
		(int) ((bp->bp_vend)[1]),
		(int) ((bp->bp_vend)[2]),
		(int) ((bp->bp_vend)[3]));
    }

    /*
     * If this host isn't set for automatic vendor info then copy the
     * specific cookie into the bootp packet, thus forcing a certain
     * reply format.
     */
    if (!hp->flags.vm_auto) {
	memcpy(bp->bp_vend, hp->vm_cookie, 4);
    }

    /*
     * Figure out the format for the vendor-specific info.
     */
    if (memcmp(bp->bp_vend, vm_rfc1048, 4)) {
	/* Not an RFC1048 bootp client */
#ifdef VEND_CMU
	dovend_cmu(bp, hp);
#else
	dovend_rfc1048(bp, hp, bootsize);
#endif
    } else {
	/* RFC1048 conformant bootp client */
	dovend_rfc1048(bp, hp, bootsize);
    }
    sendreply(0);
}



/*
 * This call checks read access to a file.  It returns 0 if the file given
 * by "path" exists and is publically readable.  A value of -1 is returned if
 * access is not permitted or an error occurs.  Successful calls also
 * return the file size in bytes using the long pointer "filesize".
 *
 * The read permission bit for "other" users is checked.  This bit must be
 * set for tftpd(8) to allow clients to read the file.
 */

int chk_access(path, filesize)
char *path;
long *filesize;
{
    struct stat buf;

    if ((stat(path, &buf) == 0) && (buf.st_mode & (S_IREAD >> 6))) {
	*filesize = (long) buf.st_size;
	return 0;
    } else {
	return -1;
    }
}



/*
 * Process BOOTREPLY packet (something is using us as a gateway).
 */

reply()
{
	if (debug) {
	    report(LOG_INFO, "processing boot reply\n");
	}
	sendreply(1);
}



/*
 * Send a reply packet to the client.  'forward' flag is set if we are
 * not the originator of this reply packet.
 */
sendreply(forward)
    int forward;
{
	register struct bootp *bp = (struct bootp *) buf;
	struct in_addr dst;
	struct sockaddr_in to;

	to = sin;

	to.sin_port = htons(bootpc_port);
	/*
	 * If the client IP address is specified, use that
	 * else if gateway IP address is specified, use that
	 * else make a temporary arp cache entry for the client's NEW 
	 * IP/hardware address and use that.
	 */
	if (bp->bp_ciaddr.s_addr) {
		dst = bp->bp_ciaddr;
	} else if (bp->bp_giaddr.s_addr && forward == 0) {
		dst = bp->bp_giaddr;
		to.sin_port = htons(bootps_port);
	} else {
		dst = bp->bp_yiaddr;
		setarp(&dst, bp->bp_chaddr, bp->bp_hlen);
	}

	if (forward == 0) {
		/*
		 * If we are originating this reply, we
		 * need to find our own interface address to
		 * put in the bp_siaddr field of the reply.
		 * If this server is multi-homed, pick the
		 * 'best' interface (the one on the same net
		 * as the client).
		 */
		int maxmatch = 0;
		int len, m;
		register struct ifreq *ifrq, *ifrmax;

		ifrmax = ifrq = &ifreq[0];
		len = ifconf.ifc_len;
		for (; len > 0; len -= sizeof(ifreq[0]), ifrq++) {
			m = nmatch(&dst, &((struct sockaddr_in *)
					  (&ifrq->ifr_addr))->sin_addr);
			if (m > maxmatch) {
				maxmatch = m;
				ifrmax = ifrq;
			}
		}
		if (bp->bp_giaddr.s_addr == 0) {
			if (maxmatch == 0) {
				return;
			}
			bp->bp_giaddr = ((struct sockaddr_in *)
				(&ifrmax->ifr_addr))->sin_addr;
		}
		bp->bp_siaddr = ((struct sockaddr_in *)
			(&ifrmax->ifr_addr))->sin_addr;
	}

	to.sin_addr = dst; 
	if (sendto(s, bp, sizeof(struct bootp), 0, &to, sizeof(to)) < 0) {
	    report(LOG_ERR, "sendto: %s\n", get_network_errmsg());
	}
}



/*
 * Return the number of leading bytes matching in the
 * internet addresses supplied.
 */
nmatch(ca,cb)
    register char *ca, *cb;
{
    register n,m;

    for (m = n = 0 ; n < 4 ; n++) {
	if (*ca++ != *cb++)
	    return(m);
	m++;
    }
    return(m);
}



/*
 * Setup the arp cache so that IP address 'ia' will be temporarily
 * bound to hardware address 'ha' of length 'len'.
 */
setarp(ia, ha, len)
	struct in_addr *ia;
	byte *ha;
	int len;
{
	struct sockaddr_in *si;
	
#ifdef ARP_IOCTL_FIXED
	memset((caddr_t)&arpreq, 0, sizeof(arpreq));
	
	arpreq.arp_pa.sa_family = AF_INET;
	si = (struct sockaddr_in *) &arpreq.arp_pa;
	si->sin_addr = *ia;

	arpreq.arp_flags = ATF_INUSE | ATF_COM;
	
	memcpy(arpreq.arp_ha.sa_data, ha, len);

	if (ioctl(s, SIOCSARP, (caddr_t)&arpreq) < 0) {
	    report(LOG_ERR, "ioctl(SIOCSARP): %s\n", get_network_errmsg());
	}
#else
	int d;
	struct strioctl sti;

	memset((caddr_t)&arpreq, 0, sizeof(arpreq));
	
	arpreq.arp_pa.sa_family = AF_INET;
	si = (struct sockaddr_in *) &arpreq.arp_pa;
	si->sin_addr = *ia;

	arpreq.arp_flags = ATF_INUSE | ATF_COM;
	
	memcpy(arpreq.arp_ha.sa_data, ha, len);


	d=open("/dev/arp", O_RDWR);
	if(d==-1)
	{
		report(LOG_ERR,"can't open /dev/arp. reason=%d\n", errno);
	}
	sti.ic_cmd = SIOCSARP;
	sti.ic_timout = 0;
	sti.ic_len = sizeof (struct arpreq);
	sti.ic_dp = (caddr_t) &arpreq;
	if( ioctl(d, I_STR, (caddr_t)&sti) == -1)
	{
		report(LOG_ERR,"ioctl failed. reason=%d\n", errno);
	}
	close(d);
#endif

}



#ifdef DEBUG

/*
 * Dump the internal memory database to bootpd_dump.
 */

dumptab()
{
    register int n;
    register struct host *hp;
    register FILE *fp;
    long t;

    /*
     * Open bootpd.dump file.
     */
    if ((fp = fopen(bootpd_dump, "w")) == NULL) {
	report(LOG_ERR, "error opening \"%s\": %s\n", bootpd_dump,
			get_errmsg());
	exit(1);
    }

    t = time(NULL);
    fprintf(fp, "\n# %s\n", Version);
    fprintf(fp, "# %s: dump of bootp server database.\n", bootpd_dump);
    fprintf(fp, "#\n# Dump taken %s", ctime(&t));
    fprintf(fp, "#\n#\n# Legend:\n");
    fprintf(fp, "#\thd -- home directory\n");
    fprintf(fp, "#\tbf -- bootfile\n");
    fprintf(fp, "#\tbs -- bootfile size in 512-octet blocks\n");
    fprintf(fp, "#\tcs -- cookie servers\n");
    fprintf(fp, "#\tds -- domain name servers\n");
    fprintf(fp, "#\tgw -- gateways\n");
    fprintf(fp, "#\tha -- hardware address\n");
    fprintf(fp, "#\thd -- home directory for bootfiles\n");
    fprintf(fp, "#\tht -- hardware type\n");
    fprintf(fp, "#\tim -- impress servers\n");
    fprintf(fp, "#\tip -- host IP address\n");
    fprintf(fp, "#\tlg -- log servers\n");
    fprintf(fp, "#\tlp -- LPR servers\n");
    fprintf(fp, "#\tns -- IEN-116 name servers\n");
    fprintf(fp, "#\trl -- resource location protocol servers\n");
    fprintf(fp, "#\tsm -- subnet mask\n");
    fprintf(fp, "#\tto -- time offset (seconds)\n");
    fprintf(fp, "#\tts -- time servers\n\n\n");

    n = 0;
    for (hp = (struct host *) hash_FirstEntry(nmhashtable); hp != NULL;
	 hp = (struct host *) hash_NextEntry(nmhashtable)) {
	    dump_host(fp, hp);
	    fprintf(fp, "\n");
	    n++;
    }
    fclose(fp);

    report(LOG_INFO, "dumped %d entries to \"%s\".\n", n, bootpd_dump);
}



/*
 * Dump all the available information on the host pointed to by "hp".
 * The output is sent to the file pointed to by "fp".
 */

void dump_host(fp, hp)
FILE *fp;
struct host *hp;
{
    register int i;
    register byte *dataptr;

    if (hp) {
	if (hp->hostname) {
	    fprintf(fp, "%s:", hp->hostname->string);
	}
	if (hp->flags.iaddr) {
	    fprintf(fp, "ip=%s:", inet_ntoa(hp->iaddr.s_addr));
	}
	if (hp->flags.htype) {
	    fprintf(fp, "ht=%u:", (unsigned) hp->htype);
	    if (hp->flags.haddr) {
		fprintf(fp, "ha=%s:", haddrtoa(hp->haddr, hp->htype));
	    }
	}
	if (hp->flags.subnet_mask) {
	    fprintf(fp, "sm=%s:", inet_ntoa(hp->subnet_mask.s_addr));
	}
	if (hp->flags.cookie_server) {
	    fprintf(fp, "cs=");
	    list_ipaddresses(fp, hp->cookie_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.domain_server) {
	    fprintf(fp, "ds=");
	    list_ipaddresses(fp, hp->domain_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.gateway) {
	    fprintf(fp, "gw=");
	    list_ipaddresses(fp, hp->gateway);
	    fprintf(fp, ":");
	}
	if (hp->flags.impress_server) {
	    fprintf(fp, "im=");
	    list_ipaddresses(fp, hp->impress_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.log_server) {
	    fprintf(fp, "lg=");
	    list_ipaddresses(fp, hp->log_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.lpr_server) {
	    fprintf(fp, "lp=");
	    list_ipaddresses(fp, hp->lpr_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.name_server) {
	    fprintf(fp, "ns=");
	    list_ipaddresses(fp, hp->name_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.rlp_server) {
	    fprintf(fp, "rl=");
	    list_ipaddresses(fp, hp->rlp_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.time_server) {
	    fprintf(fp, "ts=");
	    list_ipaddresses(fp, hp->time_server);
	    fprintf(fp, ":");
	}
	if (hp->flags.time_offset) {
	    if (hp->flags.timeoff_auto) {
		fprintf(fp, "to=auto:");
	    } else {
		fprintf(fp, "to=%ld:", hp->time_offset);
	    }
	}
	if (hp->flags.homedir) {
	    fprintf(fp, "hd=%s:", hp->homedir->string);
	}
	if (hp->flags.bootfile) {
	    fprintf(fp, "bf=%s:", hp->bootfile->string);
	}
	if (hp->flags.bootsize) {
	    fprintf(fp, "bs=");
	    if (hp->flags.bootsize_auto) {
		fprintf(fp, "auto:");
	    } else {
		fprintf(fp, "%d:", hp->bootsize);
	    }
	}
	if (hp->flags.name_switch && hp->flags.send_name) {
	    fprintf(fp, "hn:");
	}
	if (hp->flags.vendor_magic) {
	    fprintf(fp, "vm=");
	    if (hp->flags.vm_auto) {
		fprintf(fp, "auto:");
	    } else if (!memcmp(hp->vm_cookie, vm_cmu, 4)) {
		fprintf(fp, "cmu:");
	    } else if (!memcmp(hp->vm_cookie, vm_rfc1048, 4)) {
		fprintf(fp, "rfc1048");
	    } else {
		fprintf(fp, "%d.%d.%d.%d:",
			    (int) ((hp->vm_cookie)[0]),
			    (int) ((hp->vm_cookie)[1]),
			    (int) ((hp->vm_cookie)[2]),
			    (int) ((hp->vm_cookie)[3]));
	    }
	}
	if (hp->flags.generic) {
	    fprintf(fp, "generic=");
	    dataptr = hp->generic->data;
	    for (i = hp->generic->length; i > 0; i--) {
		fprintf(fp, "%02X", (int) *dataptr++);
	    }
	    fprintf(fp, ":");
	}
    }
}



/*
 * Dump an entire struct in_addr_list of IP addresses to the indicated file.
 *
 * The addresses are printed in standard ASCII "dot" notation and separated
 * from one another by a single space.  A single leading space is also
 * printed before the first adddress.
 *
 * Null lists produce no output (and no error).
 */

void list_ipaddresses(fp, ipptr)
    FILE *fp;
    struct in_addr_list *ipptr;
{
    register unsigned count;
    register struct in_addr *addrptr;

    if (ipptr) {
	count = ipptr->addrcount;
	addrptr = ipptr->addr;
	if (count-- > 0) {
	    fprintf(fp, "%s", inet_ntoa(*addrptr++));
	    while (count-- > 0) {
		fprintf(fp, " %s", inet_ntoa(*addrptr++));
	    }
	}
    }
}
#endif		/* DEBUG */



#ifdef VEND_CMU

/*
 * Insert the CMU "vendor" data for the host pointed to by "hp" into the
 * bootp packet pointed to by "bp".
 */

void dovend_cmu(bp, hp)
    register struct bootp *bp;
    register struct host *hp;
{
    struct cmu_vend *vendp;
    register struct in_addr_list *taddr;

    /* Fill in vendor information. Subnet mask, default gateway,
	domain name server, ien name server, time server */
    vendp = (struct cmu_vend *) bp->bp_vend;
    if (hp->flags.subnet_mask) {
	(vendp->v_smask).s_addr = hp->subnet_mask.s_addr;
	(vendp->v_flags) |= VF_SMASK;
	if (hp->flags.gateway) {
	    (vendp->v_dgate).s_addr = hp->gateway->addr->s_addr;
	}
    }
    if (hp->flags.domain_server) {
	taddr = hp->domain_server;
	if (taddr->addrcount > 0) {
	    (vendp->v_dns1).s_addr = (taddr->addr)[0].s_addr;
	    if (taddr->addrcount > 1) {
		(vendp->v_dns2).s_addr = (taddr->addr)[1].s_addr;
	    }
	}
    }
    if (hp->flags.name_server) {
	taddr = hp->name_server;
	if (taddr->addrcount > 0) {
	    (vendp->v_ins1).s_addr = (taddr->addr)[0].s_addr;
	    if (taddr->addrcount > 1) {
		(vendp->v_ins2).s_addr = (taddr->addr)[1].s_addr;
	    }
	}
    }
    if (hp->flags.time_server) {
	taddr = hp->time_server;
	if (taddr->addrcount > 0) {
	    (vendp->v_ts1).s_addr = (taddr->addr)[0].s_addr;
	    if (taddr->addrcount > 1) {
		(vendp->v_ts2).s_addr = (taddr->addr)[1].s_addr;
	    }
	}
    }
    strcpy((char *) vendp->v_magic, (char *) vm_cmu);	

    if (debug > 1) {
	report(LOG_INFO, "sending CMU-style reply\n");
    }
}

#endif /* VEND_CMU */



/*
 * Insert the RFC1048 vendor data for the host pointed to by "hp" into the
 * bootp packet pointed by "bp".
 */

void dovend_rfc1048(bp, hp, bootsize)
    register struct bootp *bp;
    register struct host *hp;
    long bootsize;
{
    int bytesleft, len;
    byte *vp;
    char *tmpstr;

    vp = bp->bp_vend;
    bytesleft = sizeof(bp->bp_vend);	/* Initial vendor area size */
    memcpy(vp, vm_rfc1048, 4);		/* Copy in the magic cookie */
    vp += 4;
    bytesleft -= 4;

    if (hp->flags.time_offset) {
	*vp++ = TAG_TIME_OFFSET;			/* -1 byte  */
	*vp++ = 4;					/* -1 byte  */
	if (hp->flags.timeoff_auto) {
	    insert_u_long(htonl(secondswest), &vp);
	} else {
	    insert_u_long(htonl(hp->time_offset), &vp);		/* -4 bytes */
	}
	bytesleft -= 6;
    }
    if (hp->flags.subnet_mask) {
	*vp++ = TAG_SUBNET_MASK;			/* -1 byte  */
	*vp++ = 4;					/* -1 byte  */
	insert_u_long(hp->subnet_mask.s_addr, &vp);	/* -4 bytes */
	bytesleft -= 6;					/* Fix real count */
	if (hp->flags.gateway) {
	    insert_ip(TAG_GATEWAY, hp->gateway, &vp, &bytesleft);
	}
    }

    if (hp->flags.bootsize) {
	bootsize = (hp->flags.bootsize_auto) ?
		   (bootsize / 512 + 1) : (hp->bootsize);   /* Round up */
	*vp++ = TAG_BOOTSIZE;
	*vp++ = 2;
	*vp++ = (byte) ((bootsize >> 8) & 0xFF);
	*vp++ = (byte) (bootsize & 0xFF);
	bytesleft -= 4;		/* Tag, length, and 16 bit blocksize */
    }

    if (hp->flags.domain_server) {
	insert_ip(TAG_DOMAIN_SERVER, hp->domain_server, &vp, &bytesleft);
    }
    if (hp->flags.name_server) {
	insert_ip(TAG_NAME_SERVER, hp->name_server, &vp, &bytesleft);
    }
    if (hp->flags.rlp_server) {
	insert_ip(TAG_RLP_SERVER, hp->rlp_server, &vp, &bytesleft);
    }
    if (hp->flags.time_server) {
	insert_ip(TAG_TIME_SERVER, hp->time_server, &vp, &bytesleft);
    }
    if (hp->flags.log_server) {
	insert_ip(TAG_LOG_SERVER, hp->log_server, &vp, &bytesleft);
    }
    if (hp->flags.lpr_server) {
	insert_ip(TAG_LPR_SERVER, hp->lpr_server, &vp, &bytesleft);
    }
    if (hp->flags.cookie_server) {
	insert_ip(TAG_COOKIE_SERVER, hp->cookie_server, &vp, &bytesleft);
    }

    if (hp->flags.name_switch && hp->flags.send_name) {
	/*
	 * Check for room for hostname.  Add 2 to account for
	 * TAG_HOSTNAME and length.
	 */
	len = strlen(hp->hostname->string);
	if ((len + 2) > bytesleft) {
	    /*
	     * Not enough room for full (domain-qualified) hostname, try
	     * stripping it down to just the first field (host).
	     */
	    tmpstr = hp->hostname->string;
	    len = 0;
	    while (*tmpstr && (*tmpstr != '.')) {
		tmpstr++;
		len++;
	    }
	}
	if ((len + 2) <= bytesleft) {
	    *vp++ = TAG_HOSTNAME;
	    *vp++ = (byte) (len & 0xFF);
	    memcpy(vp, hp->hostname->string, len);
	    vp += len;
	}
    }
    if (hp->flags.generic) {
	insert_generic(hp->generic, &vp, &bytesleft);
    }
    if (bytesleft >= 1) {
	*vp = TAG_END;	
    }

    if (debug > 1) {
	report(LOG_INFO, "sending RFC1048-style reply\n");
    }
}



/*
 * Compare function to determine whether two hardware addresses are
 * equivalent.  Returns TRUE if "host1" and "host2" are equivalent, FALSE
 * otherwise.
 *
 * This function is used when retrieving elements from the hardware address
 * hash table.
 */

boolean hwlookcmp(host1, host2)
    struct host *host1, *host2;
{
    if (host1->htype != host2->htype) {
	return FALSE;
    }
    if (memcmp(host1->haddr, host2->haddr, haddrlength(host1->htype))) {
	return FALSE;
    }
    return TRUE;
}




/*
 * Compare function for doing IP address hash table lookup.
 */

boolean iplookcmp(host1, host2)
    struct host *host1, *host2;
{
    return (host1->iaddr.s_addr == host2->iaddr.s_addr);
}



/*
 * Insert a tag value, a length value, and a list of IP addresses into the
 * memory buffer indirectly pointed to by "dest", without going past "stop".
 * "ipst_prt" is a pointer to an ipaddr_t (i.e. a structure containing a
 * pointer to a linked list of IP addresses and a linkcount specifying how
 * many hosts are using this list).
 *
 * This is used to fill the vendor-specific area of a bootp packet in
 * conformance to RFC1048.
 */

void insert_ip(tag, iplist, dest, bytesleft)
    byte tag;
    struct in_addr_list *iplist;
    byte **dest;
    int *bytesleft;
{
    register struct in_addr *addrptr;
    register unsigned addrcount;
    byte *d;

    if (iplist && (*bytesleft >= 6)) {
	d = *dest;				/* Save pointer for later */
	**dest = tag;
	(*dest) += 2;
	(*bytesleft) -= 2;		    /* Account for tag and length */
	addrptr = iplist->addr;
	addrcount = iplist->addrcount;
	while ((*bytesleft >= 4) && (addrcount > 0)) {
	    insert_u_long(addrptr->s_addr, dest);
	    addrptr++;
	    addrcount--;
	    (*bytesleft) -= 4;			/* Four bytes per address */
	}
	d[1] = (byte) ((*dest - d - 2) & 0xFF);
    }
}



/*
 * Insert generic data into a bootp packet.  The data is assumed to already
 * be in RFC1048 format.  It is inserted using a first-fit algorithm which
 * attempts to insert as many tags as possible.  Tags and data which are
 * too large to fit are skipped; any remaining tags are tried until they
 * have all been exhausted.
 */

void insert_generic(gendata, buff, bytesleft)
    struct shared_bindata *gendata;
    byte **buff;
    int *bytesleft;
{
    byte *srcptr;
    register int length, numbytes;

    if (gendata) {
	srcptr = gendata->data;
	length = gendata->length;
	while ((length > 0) && (*bytesleft > 0)) {
	    switch (*srcptr) {
		case TAG_END:
		    length = 0;		/* Force an exit on next iteration */
		    break;
		case TAG_PAD:
		    *(*buff)++ = *srcptr++;
		    (*bytesleft)--;
		    length--;
		    break;
		default:
		    numbytes = srcptr[1] + 2;
		    if (*bytesleft >= numbytes) {
			memcpy(*buff, srcptr, numbytes);
			(*buff) += numbytes;
			(*bytesleft) -= numbytes;
		    }
		    srcptr += numbytes;
		    length -= numbytes;
		    break;
	    }
	}
    }
}




/*
 * Convert a hardware address to an ASCII string.
 */

char *haddrtoa(haddr, htype)
    register byte *haddr;
    byte htype;
{
    static char haddrbuf[2 * MAXHADDRLEN + 1];
    register char *bufptr;
    register unsigned count;

    bufptr = haddrbuf;
    for (count = haddrlength(htype); count > 0; count--) {
	sprintf(bufptr, "%02X",	(unsigned) (*haddr++ & 0xFF));
	bufptr += 2;
    }
    return (haddrbuf);
}



/*
 * Insert the unsigned long "value" into memory starting at the byte
 * pointed to by the byte pointer (*dest).  (*dest) is updated to
 * point to the next available byte.
 *
 * Since it is desirable to internally store network addresses in network
 * byte order (in struct in_addr's), this routine expects longs to be
 * passed in network byte order.
 *
 * However, due to the nature of the main algorithm, the long must be in
 * host byte order, thus necessitating the use of ntohl() first.
 */

void insert_u_long(value, dest)
    unsigned long value;
    byte **dest;
{
    register byte *temp;
    register int n;

    value = ntohl(value);	/* Must use host byte order here */
    temp = (*dest += 4);
    for (n = 4; n > 0; n--) {
	*--temp = (byte) (value & 0xFF);
	value >>= 8;
    }
    /* Final result is network byte order */
}



/*
 * Return pointer to static string which gives full filesystem error message.
 */

char *get_errmsg()
{
    static char errmsg[80];

    if (errno < sys_nerr) {
	return sys_errlist[errno];
    } else {
	sprintf(errmsg, "Error %d", errno);
	return errmsg;
    }
}



/*
 * This routine reports errors and such via stderr and syslog() if
 * appopriate.  It just helps avoid a lot of "#ifdef SYSLOG" constructs
 * from being scattered throughout the code.
 *
 * The syntax is identical to syslog(3), but %m is not considered special
 * for output to stderr (i.e. you'll see "%m" in the output. . .).  Also,
 * control strings should normally end with \n since newlines aren't
 * automatically generated for stderr output (whereas syslog strips out all
 * newlines and adds its own at the end).
 */

/*VARARGS2*/
void report(priority, fmt, p0, p1, p2, p3, p4)
    int priority;
    char *fmt;
{
    /*
     * Print the message
     */
    if (debug > 2) {
	fprintf(stderr, "bootpd: ");
	fprintf(stderr, fmt, p0, p1, p2, p3, p4);
    }
#ifdef SYSLOG
    syslog(priority, fmt, p0, p1, p2, p3, p4);
#endif
}


int
ipioctl( int fd, int cmd, void* datap )
{
	struct strioctl sti;
	struct ifconf *ifconfp = (struct ifconf *) datap;

	if( (fd = open("/dev/ip", 0)) < 0 )
		report(LOG_ERR,"can't open /dev/ip. reason=%d\n", errno);
	sti.ic_cmd = cmd;
	sti.ic_timout = -1;
	sti.ic_len = ifconfp->ifc_len;
	sti.ic_dp = (char *) ifconfp->ifc_req;
	if( ioctl(fd, I_STR, (caddr_t)&sti) == -1)
		report(LOG_ERR,"ioctl failed. reason=%d\n", errno);
	close(fd);
	ifconfp->ifc_len = sti.ic_len;
	return sti.ic_len;
}
