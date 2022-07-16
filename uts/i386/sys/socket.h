/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_SYS_SOCKET_H
#define	_SYS_SOCKET_H

#ident	"@(#)head.sys:sys/socket.h	1.10.4.1"

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
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ifndef _KERNEL
#include <sys/netconfig.h>
#endif

/*
 * Definitions related to sockets: types, address families, options.
 */

#ifndef NC_TPI_CLTS
#define NC_TPI_CLTS	1		/* must agree with netconfig.h */
#define NC_TPI_COTS	2		/* must agree with netconfig.h */
#define NC_TPI_COTS_ORD	3		/* must agree with netconfig.h */
#define	NC_TPI_RAW	4		/* must agree with netconfig.h */
#endif /* !NC_TPI_CLTS */

/*
 * Types
 */
#define	SOCK_STREAM	NC_TPI_COTS	/* stream socket */
#define	SOCK_DGRAM	NC_TPI_CLTS	/* datagram socket */
#define	SOCK_RAW	NC_TPI_RAW	/* raw-protocol interface */
#define	SOCK_RDM	5		/* reliably-delivered message */
#define	SOCK_SEQPACKET	6		/* sequenced packet stream */

/*
 * Option flags per-socket.
 */
#define	SO_DEBUG	0x0001		/* turn on debugging info recording */
#define	SO_ACCEPTCONN	0x0002		/* socket has had listen() */
#define	SO_REUSEADDR	0x0004		/* allow local address reuse */
#define	SO_KEEPALIVE	0x0008		/* keep connections alive */
#define	SO_DONTROUTE	0x0010		/* just use interface addresses */
#define	SO_BROADCAST	0x0020		/* permit sending of broadcast msgs */
#define	SO_USELOOPBACK	0x0040		/* bypass hardware when possible */
#define	SO_LINGER	0x0080		/* linger on close if data present */
#define	SO_OOBINLINE	0x0100		/* leave received OOB data in line */
#define SO_ORDREL	0x0200		/* give use orderly release */
#define SO_IMASOCKET	0x0400		/* use socket semantics */

/*
 * N.B.: The following definition is present only for compatibility
 * with release 3.0.  It will disappear in later releases.
 */
#define	SO_DONTLINGER	(~SO_LINGER)	/* ~SO_LINGER */

/*
 * Additional options, not kept in so_options.
 */
#define	SO_SNDBUF	0x1001		/* send buffer size */
#define	SO_RCVBUF	0x1002		/* receive buffer size */
#define	SO_SNDLOWAT	0x1003		/* send low-water mark */
#define	SO_RCVLOWAT	0x1004		/* receive low-water mark */
#define	SO_SNDTIMEO	0x1005		/* send timeout */
#define	SO_RCVTIMEO	0x1006		/* receive timeout */
#define	SO_ERROR	0x1007		/* get error status and clear */
#define	SO_TYPE		0x1008		/* get socket type */
#define SO_PROTOTYPE	0x1009		/* get/set protocol type */

/*
 * Structure used for manipulating linger option.
 */
struct	linger {
	int	l_onoff;		/* option on/off */
	int	l_linger;		/* linger time */
};

/*
 * Level number for (get/set)sockopt() to apply to socket itself.
 */
#define	SOL_SOCKET	0xffff		/* options for socket level */

/*
 * Address families.
 */
#define AF_UNSPEC       0		/* unspecified */
#define AF_UNIX         1		/* local to host (pipes, portals) */
#define AF_INET         2		/* internetwork: UDP, TCP, etc. */
#define AF_IMPLINK      3		/* arpanet imp addresses */
#define AF_PUP          4		/* pup protocols: e.g. BSP */
#define AF_CHAOS        5		/* mit CHAOS protocols */
#define AF_NS           6		/* XEROX NS protocols */
#define AF_NBS          7		/* nbs protocols */
#define AF_ECMA         8		/* european computer manufacturers */
#define AF_DATAKIT      9		/* datakit protocols */
#define AF_CCITT        10		/* CCITT protocols, X.25 etc */
#define AF_SNA          11		/* IBM SNA */
#define AF_DECnet       12		/* DECnet */
#define AF_DLI          13		/* Direct data link interface */
#define AF_LAT          14		/* LAT */
#define AF_HYLINK       15		/* NSC Hyperchannel */
#define AF_APPLETALK    16		/* Apple Talk */
#define AF_NIT          17		/* Network Interface Tap */
#define AF_802          18		/* IEEE 802.2, also ISO 8802 */
#define AF_OSI          19		/* umbrella for all families used
#define AF_X25          20		/* CCITT X.25 in particular */
#define AF_OSINET       21		/* AFI = 47, IDI = 4 */
#define AF_GOSIP        22		/* U.S. Government OSI */
#define	AF_MAX		22

/*
 * Structure used by kernel to store most
 * addresses.
 */
struct sockaddr {
	u_short	sa_family;		/* address family */
	char	sa_data[14];		/* up to 14 bytes of direct address */
};

/*
 * Structure used by kernel to pass protocol
 * information in raw sockets.
 */
struct sockproto {
	u_short	sp_family;		/* address family */
	u_short	sp_protocol;		/* protocol */
};

/*
 * Protocol families, same as address families for now.
 */
#define	PF_UNSPEC	AF_UNSPEC
#define	PF_UNIX		AF_UNIX
#define	PF_INET		AF_INET
#define	PF_IMPLINK	AF_IMPLINK
#define	PF_PUP		AF_PUP
#define	PF_CHAOS	AF_CHAOS
#define	PF_NS		AF_NS
#define	PF_NBS		AF_NBS
#define	PF_ECMA		AF_ECMA
#define	PF_DATAKIT	AF_DATAKIT
#define	PF_CCITT	AF_CCITT
#define	PF_SNA		AF_SNA
#define	PF_DECnet	AF_DECnet
#define	PF_DLI		AF_DLI
#define	PF_LAT		AF_LAT
#define	PF_HYLINK	AF_HYLINK
#define	PF_APPLETALK	AF_APPLETALK
#define	PF_NIT		AF_NIT
#define	PF_802		AF_802
#define	PF_OSI		AF_OSI
#define	PF_X25		AF_X25
#define	PF_OSINET	AF_OSINET
#define	PF_GOSIP	AF_GOSIP

#define	PF_MAX		AF_MAX

/*
 * Maximum queue length specifiable by listen.
 */
#define	SOMAXCONN	5

/*
 * Message header for recvmsg and sendmsg calls.
 */
struct msghdr {
	caddr_t	msg_name;		/* optional address */
	int	msg_namelen;		/* size of address */
	struct	iovec *msg_iov;		/* scatter/gather array */
	int	msg_iovlen;		/* # elements in msg_iov */
	caddr_t	msg_accrights;		/* access rights sent/received */
	int	msg_accrightslen;
};

#define	MSG_OOB		0x1		/* process out-of-band data */
#define	MSG_PEEK	0x2		/* peek at incoming message */
#define	MSG_DONTROUTE	0x4		/* send without using routing tables */

#define	MSG_MAXIOVLEN	16

/*
 * An option specification consists of an opthdr, followed by the value of
 * the option.  An options buffer contains one or more options.  The len
 * field of opthdr specifies the length of the option value in bytes.  This
 * length must be a multiple of sizeof(long) (use OPTLEN macro).
 */

struct opthdr {
	long            level;	/* protocol level affected */
	long            name;	/* option to modify */
	long            len;	/* length of option value */
};

#define OPTLEN(x) ((((x) + sizeof(long) - 1) / sizeof(long)) * sizeof(long))
#define OPTVAL(opt) ((char *)(opt + 1))

/*
 * the optdefault structure is used for internal tables of option default
 * values.
 */
struct optdefault {
	int             optname;/* the option */
	char           *val;	/* ptr to default value */
	int             len;	/* length of value */
};

/*
 * the opproc structure is used to build tables of options processing
 * functions for dooptions().
 */
struct opproc {
	int             level;	/* options level this function handles */
	int             (*func) ();	/* the function */
};

/*
 * This structure is used to encode pseudo system calls
 */
struct socksysreq {
	int             args[7];
};

/*
 * This structure is used for adding new protocols to the list supported by
 * sockets.
 */

struct socknewproto {
	int             family;	/* address family (AF_INET, etc.) */
	int             type;	/* protocol type (SOCK_STREAM, etc.) */
	int             proto;	/* per family proto number */
	dev_t           dev;	/* major/minor to use (must be a clone) */
	int             flags;	/* protosw flags */
};


/* defines for user/kernel interface */

#if (INTEL == 31) || (ATT == 31)
#define SOCKETSYS	88	/* MUST BE CHANGED DEPENDING ON OS/SYSENT.C!! */
#else
#define SOCKETSYS	83	/* MUST BE CHANGED DEPENDING ON OS/SYSENT.C!! */
#endif

#define  SO_ACCEPT	1
#define  SO_BIND	2
#define  SO_CONNECT	3
#define  SO_GETPEERNAME	4
#define  SO_GETSOCKNAME	5
#define  SO_GETSOCKOPT	6
#define  SO_LISTEN	7
#define  SO_RECV	8
#define  SO_RECVFROM	9
#define  SO_SEND	10
#define  SO_SENDTO	11
#define  SO_SETSOCKOPT	12
#define  SO_SHUTDOWN	13
#define  SO_SOCKET	14
#define  SO_SOCKPOLL	15
#define  SO_GETIPDOMAIN	16
#define  SO_SETIPDOMAIN	17
#define  SO_ADJTIME	18

#endif	/* _SYS_SOCKET_H */
