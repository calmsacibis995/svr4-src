/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/net.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)net.c	3.39	LCC);	/* Modified: 16:24:38 2/26/90 */

/*****************************************************************************

	Copyright (c) 1984 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

*****************************************************************************/

/*
 * History:
 *	[10/25/88 efh] Eugene Hu - added fix to myhostent from 2.8.6 which
 *		checks for NULL return by gethostbyname().
 */
#if defined(BSD43)
#	include <sys/ioctl.h>
#	if defined(SYS5_4)
#		include <sys/sockio.h>
#	endif
#endif	/* BSD43 */

#if defined(EXL316CMC) || defined(ISC386)
#	include <net/errno.h>
#elif defined(MICOM)
#	include <interlan/il_errno.h>
#elif defined(SYS19)
#	include <h/errno42.h>
#elif defined(EXCELAN)
#	include <ex_errno.h>
#endif	/* EXL316MC || ISC386 */

#if !defined(EXL316CMC)
#	include	<errno.h>
#endif	/* !EXL316MC */

#if		defined(ISC386)
#include	<sys/sioctl.h>
#endif		/* ISC386 */

#include	"pci_types.h"
#include	<fcntl.h>

#include	<memory.h>
#include	<flip.h>

#include	<ctype.h>

#ifdef	RS232PCI
#include	<termio.h>
#endif	  /* RS232PCI  */

#ifdef STREAMNET
#include <stropts.h>
#endif

#define	r232dbg(X)	debug(DBG_RS232,X)

/*			External Variables			*/

extern int
	errno;			/* Error number from system call */

static int
	theNetDesc = -1,	/* Current network file descriptor */
	savedNetDesc = -1;	/* Saved network descriptor */

#ifdef	TLI
extern int t_errno;		/* System errno var for TLI calls */
#endif	/* TLI */

#ifdef	MICOM
static 	int 	micomPort;
#endif

#ifdef		ATT3B2
#include	<sys/inet.h>
#endif




#define	MAXRRETRY	3	/* Maximum receive retries */
#define	MAXXRETRY	3	/* Maximum xmit retries */
#define MAXORETRY	5	/* Maximum open retries */


#if defined(EXCELAN)

char *inet_ntoa(in)
	struct in_addr	in;
{
	static char	buf[] = "255.255.255.255";

	sprintf(buf, "%uh.%uh.%uh.%uh", (unsigned short)in.S_un.S_un_b.s_b1,
					(unsigned short)in.S_un.S_un_b.s_b2,
					(unsigned short)in.S_un.S_un_b.s_b3,
					(unsigned short)in.S_un.S_un_b.s_b4);
	return buf;
}

unsigned long inet_addr(cp)
	char		*cp;
{
	unsigned short	byte0, byte1, byte2, byte3;
	int		numBytes;

	byte1 = byte2 = byte3 = 0;
	numBytes = sscanf(cp, "%hx.%hx.%hx.%hx",
					&byte0, &byte1, &byte2, &byte3);

	if (!numBytes || numBytes == EOF || byte0 > 0xff || byte1 > 0xff
					 || byte2 > 0xff || byte3 > 0xff)
		return (unsigned long)-1;	/* malformed address */
	return (byte0 + (byte1 + (byte2 + (byte3 << 8) << 8) << 8));
}

#endif	/* EXCELAN */


/*
   netSave: Tell network driver what descriptor we're talking on
*/

int				/* Returns current network file descriptor */
netSave()
{
	return savedNetDesc = theNetDesc;
}


/*
   netUse: Tell network driver what descriptor to talk on
*/

int				/* Returns new network file descriptor */
netUse(newNetDesc)
int
	newNetDesc;		/* New network file descriptor */
{
#ifdef	TLI
	if (newNetDesc < 0) {
		log("netUse: Invalid net descriptor\n");
		exit(1);
	}
	if (newNetDesc != theNetDesc && newNetDesc != savedNetDesc) {
		if (t_sync(newNetDesc) < 0) {
			log("netUse: t_sync failed t_errno %d errno %d\n",
				t_errno, errno);
			exit(1);
		}
	}
#endif	/* TLI */

	return theNetDesc = newNetDesc;
}



/* Start of network dependent code */


#ifdef	UDP42
#ifdef TLI

/*
   netOpen:  Get access to a network port  (TLI version)
*/

int				/* Returns file descriptor of network port */
				/* < 0 signals error */
netOpen(netDev, portNum)
char
	*netDev;		/* Name of network interface device */
int
	portNum;		/* The port to assign */
{
int
	retryCount,		/* Used for opens and binds */
	netDesc;		/* Network file descriptor */
struct t_bind 
	*req, *ret;		/* Bind request and return addresses */
struct t_bind 
	retstruct, reqstruct;
struct t_info 
	tinfo;			/* Returns default parms of transport protcol */
struct sockaddr_in
	vPort;			/* Network driver virtual port structure */
char    pad[10000];
int 	one = 1;
int	memret;

#ifdef BROADCAST_OPTION
#ifdef BELLTLI
struct t_optmgmt *topt;
struct opthdr *oh;
#else
struct  t_optmgmt reqopt;
struct  t_optmgmt retopt;
static char    optbuf[10000];
struct inetopt netopt;
#endif
#endif


extern char *nAddrFmt();

	for (retryCount = 0; retryCount < MAXORETRY;) {
		if ((netDesc = t_open(netDev, O_RDWR, &tinfo)) >= 0)
			break; 
		else if ((netDesc = t_open("/dev/inet/udp", O_RDWR, &tinfo)) >= 0)
			break;
		log("netOpen: open failed: errno %d\n", errno);
		if (t_errno == TSYSERR)
		switch(errno) {
		case ENOBUFS:
			(void) sleep(10);
			break;
		case EISCONN:
		case ENOTCONN:
		case EADDRINUSE:
		case EADDRNOTAVAIL:
		default:
			log("netOpen: open failed: errno %d t_errno %d\n",
					errno, t_errno);
			return -1;
		}
		retryCount++;
	}

	if (tinfo.tsdu > 0 &&  tinfo.tsdu < MAX_FRAME)  {
		log("netOpen: open returned max data < MAX_FRAME\n");
		fatal("netOpen: open: max data < MAX_FRAME -- PCI Aborting");
	}

	memset((char *)&vPort, '\0', sizeof(struct sockaddr_in));
	vPort.sin_family = AF_INET;
	vPort.sin_port = htons(portNum);

	/* Allocate bind structures */
	req = &reqstruct;
	ret = &retstruct;

	req->addr.len = SZNADDR;
	req->addr.buf = (char *)&vPort;
	/* memcpy(req->addr.buf, (char *)&vPort, SZNADDR); */
	req->qlen = 0;

	ret->addr.maxlen = SZNADDR;
	ret->addr.buf = (char *)&vPort;

	if (t_bind(netDesc, req, ret) < 0) { 
		log("netOpen: can't bind endpoint: t_errno %d errno %d\n",
				t_errno, errno);
	        t_close(netDesc);
		return -1;
	}

	vPort = *(struct sockaddr_in *)ret->addr.buf;
	memret = memcmp( ret->addr.buf, req->addr.buf, sizeof(struct sockaddr_in));
	if ( memret ) {
	        t_close(netDesc);
		log("netOpen: bind of wrong address\n");
		return -1;
	}

#ifdef BROADCAST_OPTION
#ifdef BELLTLI
	if ((topt = (struct t_optmgmt *) t_alloc(netDesc, T_OPTMGMT, T_ALL)) == NULL) {
		log("netOpen: t_alloc() failed: t_errno=%d errno=%d\n",
			t_errno, errno);
		return(-1);
	}
	oh = (struct opthdr *)topt->opt.buf;
	oh->level = SOL_SOCKET;
	oh->name = SO_BROADCAST;
	oh->len = OPTLEN(sizeof(int));
	*((int *)OPTVAL(oh)) = 1;
	topt->opt.len = sizeof(struct opthdr) + OPTLEN(sizeof(int));
	topt->flags = T_NEGOTIATE;
	if (t_optmgmt(netDesc, topt, topt) < 0) {
		log("netOpen: t_optmgmt() failed: t_errno %d errno %d\n",
			t_errno, errno);
		t_close(netDesc);
		t_free(topt, T_OPTMGMT);
		return(-1);
	}
	t_free(topt, T_OPTMGMT);
#else
	/* Option management for debugging */
	netopt.len = (short)(IOPT_LEADIN_LEN + sizeof(long));
	netopt.name = (ushort)SO_BROADCAST;
	netopt.level = (ushort)SOL_SOCKET;
	netopt.value.ival = 1;

	reqopt.opt.len = sizeof(struct inetopt);
	reqopt.opt.buf = (char *)&netopt;
	reqopt.flags = (long)T_NEGOTIATE;

	retopt.opt.maxlen = 10000;
	retopt.opt.buf = &optbuf[0];
	if (t_optmgmt(netDesc, &reqopt, &retopt) < 0) {
		log("netOpen: t_optmgmt() failed: t_errno %d errno %d\n",
			t_errno, errno);
		return -1;
	}
#endif
#endif

	return theNetDesc = netDesc;
}


/*
   rcvPacket():	Read packet from ethernet device into buffer;
		Swap bytes if necessary.  Retry on transient
		errors.
*/

int
rcvPacket(inPkt)
register struct input
	*inPkt;			/* Pointer to input buffer */
{
int
	nRead,				/* Return from read()	*/
	retryCount,			/* Number of times retried */
	fromLength,
	flipCode;			/* Byte ordering code */

	int    rcvFlags;
	struct t_unitdata *ud;		/* datagram unit */
	struct t_uderr *uderr;		/* datagram error unit */
	struct t_unitdata udstruct;
	struct t_uderr uderrstruct;

	ud = &udstruct;
	uderr = &uderrstruct;

	rcvFlags = 0;
	ud->opt.maxlen = 0;
	ud->addr.maxlen = (unsigned int)SZNADDR;
	ud->udata.maxlen = (unsigned int)MAX_FRAME;
	ud->addr.buf = (char *)&inPkt->net.src_sin;
	ud->udata.buf = (char *)&inPkt->pre;

	for (retryCount = 0; retryCount < MAXRRETRY;) {
		if ((t_rcvudata(theNetDesc, ud, &rcvFlags)) < 0) {

			if (t_errno == TSYSERR && errno == EINTR)
				continue;

			log("rcvPacket: t_rcvudata() failed: t_errno = %d, errno = %d\n", 
				t_errno, errno);
			if (t_errno == TLOOK) {
			    /* Error on previous datagram */
			    log("rcvPacket: error on previous datagram sent\n");
			    if (t_rcvuderr(theNetDesc, uderr) < 0) {
				log("rcvPacket: error on t_rcvuderr()\n");
			    }

			    log("rcvPacket: bad datagram: ud_error = %d\n", 
				uderr->error);
			    continue;

			}

			retryCount++;
			continue;
		}

		flipCode = input_swap(inPkt, inPkt->hdr.pattern);
		logPacket(inPkt, PLOG_RCV, LOG_SOME);
		return flipCode;

	}
	fatal("rcvPacket: Retry limit exceeded\n");
}


struct ni2
	lastXmtHeader;		/* Remember address for reXmt() */

/*
   xmtPacket():	Sends a packet onto the ethernet. Returns the length
  		of the packet sent, or 0 on error.
*/

int
xmtPacket(outPacket, niHeader, flipCode)
register struct output
	*outPacket;		/* Outgoing packet */
struct ni2
	*niHeader;		/* Ni driver header */
register int
	flipCode;		/* Flipping sense pattern */
{
register int
	pktLength,		/* Packet length */
	pktSent,		/* Number of bytes successfully transmitted */
	retryCount;		/* Number of times retried */

	struct t_unitdata *ud;		/* datagram unit */

	if ((ud = (struct t_unitdata *)t_alloc(theNetDesc, T_UNITDATA, T_OPT))
	    == (struct t_unitdata *)NULL) {
		log("xmtPacket: theNetDesc = %d\n", theNetDesc);
		log("xmtPacket: alloc of unit-data struct failed: t_error %d\n",
				t_errno);
		return 0;
	}

	pktLength = HEADER + outPacket->hdr.t_cnt;

	/* If the length is invalid send an error messsge */
	if (pktLength > MAX_FRAME) {
		log("xmtPacket: Packet too big: %d\n", pktLength);
		outPacket->hdr.res = INTERNAL_ERRORS;
		pktLength = HEADER;
	}

	logPacket(outPacket, PLOG_XMT, LOG_SOME);
	output_swap(outPacket, flipCode);
	lastXmtHeader = *niHeader;

	/* Assign the dst sockaddr struct to the addr.buf for UDP */
	ud->addr.len = SZNADDR;
	ud->addr.buf = (char *)&niHeader->dst_sin;

	/* No special options */
	ud->opt.len = 0;

	/* Packet goes in udata */
	ud->udata.len = (unsigned int)pktLength;
	ud->udata.buf = (char *)&outPacket->pre;

	/* Write the packet onto the ethernet device */
	for (retryCount = 0; retryCount < MAXXRETRY;) {
		if ((t_sndudata(theNetDesc, ud)) < 0) {
			log("xmtPacket: t_sndudata() failed: t_errno = %d\n", 
				t_errno);

			if (t_errno == TSYSERR) {
			log("xmtPacket: t_sndudata() failed: errno = %d\n", 
				errno);
			switch(errno) {
			case EINTR:		/* Retry forever */
				log("xmtPacket: Interrupted system call\n");
				continue;
			}
			}


			log("xmtPacket: send of datagram failed\n");
			switch(t_errno) {
			    case TFLOW:	/* Retry forever */
				continue;
			    default:
				log("xmtPacket: UDP Write err: %d\n", t_errno);
				(void)t_free((char *)ud, T_UNITDATA);
				return 0;
			}
		}
		else {
			(void)t_free((char *)ud, T_UNITDATA);
			return pktLength;
		}

/*		retryCount++;	NOTREACHED */
	}
}


/*****************************************************************************
 *  reXmt: retransmit last packet
 */

reXmt(rePacket, reLength)
struct output
	*rePacket;
int
	reLength;
{
unsigned
	retryCount,
	reSent;

	struct t_unitdata *ud;		/* datagram unit */

	if ((ud = (struct t_unitdata *)t_alloc(theNetDesc, T_UNITDATA, T_OPT))
	    == (struct t_unitdata *)NULL) {
		log("reXmtPacket: alloc of unit-data structure failed\n");
		return 0;
	}

	/* Assign the dst sockaddr struct to the addr.buf for UDP */
	ud->addr.len = SZNADDR;
	ud->addr.buf = (char *)&lastXmtHeader.dst_sin;

	/* No special options */
	ud->opt.len = 0;

	/* Packet goes in udata */
	ud->udata.len = (unsigned int)reLength;
	ud->udata.buf = (char *)&rePacket->pre;

	log("reXmt: seq %d\n", rePacket->hdr.seq);

	/* Send the packet */
	for (retryCount = 0; retryCount < MAXXRETRY;) {
		if ((t_sndudata(theNetDesc, ud)) < 0) {
			if (t_errno == TSYSERR) {
			log("reXmtPacket: t_sndudata() failed: errno = %d\n", 
				errno);
			switch(errno) {
			case EINTR:		/* Retry forever */
				log("reXmtPacket: Interrupted system call\n");
				continue;
			}
			}


			log("reXmtPacket: send of datagram failed\n");
			switch(t_errno) {
			    case TFLOW:	/* Retry forever */
				continue;
			    default:
				log("reXmtPacket: UDP Write err: %d\n", t_errno);
				(void)t_free((char *)ud, T_UNITDATA);
				return 0;
			}
		}
		else {
			(void)t_free((char *)ud, T_UNITDATA);
			return reLength;
		}

/*		retryCount++;	NOTREACHED */
	}
}
#else	/* !TLI */


/*
   netOpen:  Get access to a network port (UDP version)
*/

int				/* Returns file descriptor of network port */
				/* < 0 signals error */
netOpen(netDev, portNum)
char
	*netDev;		/* Name of network interface device */
int
	portNum;		/* The port to assign */
{
int
	retryCount,		/* Used for opens and binds */
	netDesc;		/* Network file descriptor */
struct sockaddr_in
	vPort;			/* Network driver virtual port structure */
int 	one = 1;


#ifdef MICOM
	micomPort = portNum;
#endif
	vPort.sin_family = AF_INET;
	vPort.sin_addr.s_addr = INADDR_ANY;
	vPort.sin_port = htons(portNum);

	for (retryCount = 0; retryCount < MAXORETRY;) {
#ifdef EXCELAN
		if ((netDesc = socket(SOCK_DGRAM, (struct sockproto *)NULL,
				&vPort, 0)) >= 0)	/* ERP */
#else
		if ((netDesc = socket(AF_INET, SOCK_DGRAM, 0)) != -1)
#endif	/* EXCELAN */
			break; 
		switch(errno) {
		case ENOBUFS:
			(void) sleep(10);
			break;
		case EISCONN:
		case ENOTCONN:
		case EADDRINUSE:
		case EADDRNOTAVAIL:
		default:
			log("netOpen: socket failed: %d\n",errno);
			return -1;
		}
		retryCount++;
	}
#ifdef EXCELAN
	socketaddr(netDesc,&vPort);
	log("Just did socket call... got sock desc=<%d>, port=<%d>\n",
			netDesc,ntohs(vPort.sin_port));
#else
	if (bind(netDesc, &vPort, sizeof vPort) < 0) {
		log("netOpen: can't bind socket %d\n", errno);
		return -1;
	}
#endif	/* EXCELAN */

#if defined(BSD43) && !defined(DGUX)
/* This is for 4.3BSD Unix systems which must have socket broadcast permissions
   explicitly set with a setsockopt() system call */

	if (setsockopt(netDesc,SOL_SOCKET,SO_BROADCAST,
			&one,sizeof(one)) < 0)
		log("SETSOCKOPT failed; errno: %d.\n",errno);

#endif /* BSD43 && !DGUX */

	return theNetDesc = netDesc;
}

/*
   rcvPacket():	Read packet from ethernet device into buffer;
		Swap bytes if necessary.  Retry on transient
		errors.
*/

int
rcvPacket(inPkt)
register struct input
	*inPkt;			/* Pointer to input buffer */
{
int
	nRead,				/* Return from read()	*/
	retryCount,			/* Number of times retried */
	fromLength,
	flipCode;			/* Byte ordering code */

	for (retryCount = 0; retryCount < MAXRRETRY;) {
		fromLength = SZNADDR;
#if defined(EXCELAN)
		if ((nRead = receive(theNetDesc, inPkt->net.src, &inPkt->pre,
			MAX_FRAME)) > 0) {
#else
		if ((nRead = recvfrom(theNetDesc, &inPkt->pre, MAX_FRAME,
			0, inPkt->net.src, &fromLength)) > 0) {
#endif	/* EXCELAN */
			flipCode = input_swap(inPkt, inPkt->hdr.pattern);
			logPacket(inPkt, PLOG_RCV, LOG_SOME);
			return flipCode;
		}

		switch(errno) {
		case EINTR:		/* Retry forever */
			continue;

#ifdef MICOM
		/* apparent bug in MICOM TCP	*/
		case ENOTCONN:	close(theNetDesc);
				netOpen("", micomPort);
				continue;
#endif
		case ENOTSOCK:
		case EBADF:
		case EFAULT:
		case EWOULDBLOCK:
		default:
			log("rcvPacket: UDP Read err: %d\n", errno);
		}

		retryCount++;
	}
	fatal("rcvPacket: UDP Retry limit exceeded\n");
}


struct ni2
	lastXmtHeader;		/* Remember address for reXmt() */

/*
   xmtPacket():	Sends a packet onto the ethernet. Returns the length
  		of the packet sent, or 0 on error.
*/

int
xmtPacket(outPacket, niHeader, flipCode)
register struct output
	*outPacket;		/* Outgoing packet */
struct ni2
	*niHeader;		/* Ni driver header */
register int
	flipCode;		/* Flipping sense pattern */
{
register int
	pktLength,		/* Packet length */
	pktSent,		/* Number of bytes successfully transmitted */
	retryCount;		/* Number of times retried */

	pktLength = HEADER + outPacket->hdr.t_cnt;

	/* If the length is invalid send an error messsge */
	if (pktLength > MAX_FRAME) {
		log("xmtPacket: Packet too big: %d\n", pktLength);
		outPacket->hdr.res = INTERNAL_ERRORS;
		pktLength = HEADER;
	}

	logPacket(outPacket, PLOG_XMT, LOG_SOME);
	output_swap(outPacket, flipCode);
	lastXmtHeader = *niHeader;

	/* Write the packet onto the ethernet device */
	for (retryCount = 0; retryCount < MAXXRETRY;) {
#if defined(EXCELAN)
		/* 
		log("descriptor = %d \n", theNetDesc);
		log("target address %s \n", nAddrFmt(niHeader->dst));
		*/
		pktSent = send(theNetDesc, &niHeader->dst_sin, 
			&outPacket->pre, pktLength); 
#else
		pktSent = sendto(theNetDesc, &outPacket->pre, pktLength, 0, 
				niHeader->dst, SZNADDR);
#endif	/* EXCELAN */

		if (pktSent == pktLength)
			return pktLength;

		switch(errno) {
		case EINTR:	/* Retry forever */
			continue;

		case ENOTSOCK:
		case EBADF:
		case EFAULT:
		case EWOULDBLOCK:
		default:
			log("xmtPacket: UDP Write err: %d\n", errno);
			return 0;
		}

/*		retryCount++;	NOTREACHED */
	}
}


/*****************************************************************************
 *  reXmt: retransmit last packet
 */

reXmt(rePacket, reLength)
struct output
	*rePacket;
int
	reLength;
{
unsigned
	retryCount,
	reSent;

	log("reXmt: seq %d\n", rePacket->hdr.seq);

	/* Send the packet */
	for (retryCount = 0; retryCount < MAXXRETRY;) {
#if defined(EXCELAN)
		reSent = send(theNetDesc, lastXmtHeader.dst, &rePacket->pre,
								reLength);
#else
		reSent = sendto(theNetDesc, &rePacket->pre, reLength, 0, 
				lastXmtHeader.dst, SZNADDR);
#endif	/* EXCELAN */

		if (reSent == reLength)
			return reLength;

		switch(errno) {
		case EINTR:	/* Retry forever */
			continue;

		case ENOTSOCK:
		case EBADF:
		case EFAULT:
		case EWOULDBLOCK:
		default:
			log("reXmt: UDP Write err: %d\n", errno);
			return 0;
		}

/*		retryCount++;	NOTREACHED */
	}
}

#endif 	/* TLI */



/*
   nAddrFmt: Format a network address for human consumption
		Can be called several times in the same printf
		argument list.
*/

#define		NNADDRMSG	4

static int	nAddrIndx;
static char	nAddrMsg[NNADDRMSG][256];

char *
nAddrFmt(nAddr)
unsigned char
	*nAddr;
{
	nAddrIndx = ++nAddrIndx % NNADDRMSG;

	sprintf(nAddrMsg[nAddrIndx], "%02x.%02x.%02x.%02x.%02x.%02x.%02x.%02x",
		nAddr[0], nAddr[1], nAddr[2], nAddr[3],
		nAddr[4], nAddr[5], nAddr[6], nAddr[7]);
	return nAddrMsg[nAddrIndx];
}

/*
 *  nAddrGet: Read an ethernet address (%x.%x.%x.%x.%x.%x.%x.%x)
 */

nAddrGet(addrStr, nAddr)
char
	*addrStr;
register char
	*nAddr;
{
register int
	i;			/* Count bytes of Ethernet address */

	nAddrClr(nAddr);

	for (i = 0; i < SZNADDR; i++) {
		if (!isxdigit(*addrStr))
			return 1;
		*nAddr++ = (char)strtol(addrStr, &addrStr, 16);
		if (*addrStr != '\0')
			addrStr++;
	}

	return 0;
}


hostaddr(sa)
register struct sockaddr_in
	*sa;
{
static struct hostent
	*uName; 
extern struct hostent
	*myhostent();

	if (!uName)
		uName = myhostent();

	memcpy(&sa->sin_addr.s_addr, uName->h_addr, uName->h_length);

}


netaddr(intFaceListP, intFaceNumP, mode)
netIntFace	*intFaceListP;
int	*intFaceNumP;
int mode;
{
	int n;
	static struct hostent *uName; 
	extern struct hostent *myhostent();
	long	*inAddrP;
	long	*inSubnetMaskP;
#ifdef BSD43
#if !defined(SYS5_4)
	struct ifconf *ifc;
#endif
	struct ifreq *ifr;
	struct sockaddr_in *ifr_in;
	char buf[BUFSIZ];
#ifdef STREAMNET
	struct strioctl sic;
#else
	struct ifconf ifc_data;
#endif
#endif /* BSD43 */

	if (!uName)
		uName = myhostent();

#ifdef BSD43
	if (mode & BCAST43) {
		
#ifdef STREAMNET
		sic.ic_cmd = SIOCGIFCONF;
		sic.ic_len = sizeof (buf);
		sic.ic_dp = (char *)buf;
#   if !defined(SYS5_4)
		ifc = (struct ifconf *)buf;
		ifc->ifc_len = sizeof (buf) - sizeof (struct ifconf);
#	if defined(BELLTLI)
		sic.ic_len = ifc->ifc_len;
		sic.ic_dp += sizeof(struct ifconf);
		ifc->ifc_buf = sic.ic_dp;
#	endif
#   endif   /* !SYS5_4 */
		if (ioctl(theNetDesc, I_STR, (char *)&sic) < 0)
#else
		ifc = &ifc_data;
		ifc->ifc_len = sizeof (buf);
		ifc->ifc_buf = buf;
		if (ioctl(theNetDesc, SIOCGIFCONF, (char *)ifc) < 0)
#endif
		{
			log("netaddr: ioctl(SIOCGIFCONF) failed: errno %d\n",
				errno);
		}
		*intFaceNumP = 0;

#if defined(SYS5_4)
		ifr = (struct ifreq *)buf;
#else
		ifr = ifc->ifc_req;
#endif

#if defined(SYS5_4) || defined(BELLTLI)
		n =   sic.ic_len / sizeof(*ifr);
#else
		n = ifc->ifc_len / sizeof(*ifr);
#endif

		for (; n > 0; n--, ifr++) {

			log("\"%.*s\" if\n", IFNAMSIZ, &ifr->ifr_name);
			ifr_in = (struct sockaddr_in *)&ifr->ifr_addr;

			/* Copy the local address */

			intFaceListP->localAddr.s_addr =ifr_in->sin_addr.s_addr;
			log("netaddr: local address: %s\n",
				nAddrFmt(&intFaceListP->localAddr));

#ifdef DGUX
#define ifr_broadaddr ifr_broadcast	/* may be fixed in future versions */
			ifr_in = (struct sockaddr_in *)&ifr->ifr_broadaddr;
#endif	/* DGUX */

			/* Determine the broadcast address */
#ifdef STREAMNET
			sic.ic_cmd = SIOCGIFBRDADDR;
			sic.ic_len = sizeof (struct ifreq);
			sic.ic_dp = (char *)ifr;
			if (ioctl(theNetDesc, I_STR, &sic) < 0)
#else
			if (ioctl(theNetDesc, SIOCGIFBRDADDR, ifr) < 0)
#endif
			{
				log("netaddr:SIOCGIFBRDADDR failed; errno %d\n",
					errno);
				continue;
			}

			intFaceListP->broadAddr.s_addr =ifr_in->sin_addr.s_addr;

			log("netaddr: broadcast address: %s\n",
				nAddrFmt(&intFaceListP->broadAddr));

			if (mode & USESUBNETS) {
			/* Determine the subnet mask */

#ifdef STREAMNET
				sic.ic_cmd = SIOCGIFNETMASK;
				sic.ic_len = sizeof (struct ifreq);
				sic.ic_dp = (char *)ifr;
				if (ioctl(theNetDesc, I_STR, &sic) < 0)
#else
#ifdef DGUX	/* DGUX doesn't know how to set subnet mask (yet) */
				if (ioctl(theNetDesc, SIOCGIFADDR, ifr) < 0)
#else
				if (ioctl(theNetDesc, SIOCGIFNETMASK, ifr) < 0)
#endif	/* DGUX HACK */
#endif
				{
					log("netaddr:SIOCGIFNETMASK failed; errno %d\n",
						errno);
					continue;
				}

				intFaceListP->subnetMask.s_addr =
#ifdef DGUX
					ifr->ifr_mask;
#else
					ifr_in->sin_addr.s_addr;
#endif
			} else {
				inAddrP = (long *) &(intFaceListP->localAddr);
				inSubnetMaskP = (long *) &(intFaceListP->subnetMask);
#if defined(EXCELAN)
				*inSubnetMaskP = !(*inAddrP & IN_CLASSA)
						? IN_CLASSA_NET
						: !(*inAddrP & IN_CLASSB)
							? IN_CLASSB_NET
							: IN_CLASSC_NET;
#else
				*inAddrP = ntohl(*inAddrP);
				if (IN_CLASSA(*inAddrP))
					*inSubnetMaskP = IN_CLASSA_NET;
				else if (IN_CLASSB(*inAddrP))
					*inSubnetMaskP = IN_CLASSB_NET;
				else if (IN_CLASSC(*inAddrP))
					*inSubnetMaskP = IN_CLASSC_NET;

				 *inAddrP = htonl(*inAddrP);
				 *inSubnetMaskP = htonl(*inSubnetMaskP);
#endif	/* EXCELAN */
			}

			log("netaddr: subnet mask: %s\n",
				nAddrFmt(&intFaceListP->subnetMask));

			++intFaceListP;
			++*intFaceNumP;
		}
	} else
#endif /* BSD43 */
	       {
		*intFaceNumP = 1;

		memcpy(&intFaceListP->localAddr, uName->h_addr, uName->h_length);
		memcpy(&intFaceListP->broadAddr, uName->h_addr, uName->h_length);

		inAddrP = (long *) &(intFaceListP->broadAddr);
		inSubnetMaskP = (long *) &(intFaceListP->subnetMask);
#if defined(EXCELAN)
		*inSubnetMaskP = !(*inAddrP & IN_CLASSA) ? IN_CLASSA_NET :
				 !(*inAddrP & IN_CLASSB) ? IN_CLASSB_NET :
							   IN_CLASSC_NET;
		*inAddrP |=      !(*inAddrP & IN_CLASSA) ? IN_CLASSA_LNA :
				 !(*inAddrP & IN_CLASSB) ? IN_CLASSB_LNA :
							   IN_CLASSC_LNA;
#else
		*inAddrP = ntohl(*inAddrP);

		if (IN_CLASSA(*inAddrP)) {
			 *inAddrP |= ~IN_CLASSA_NET;
			 *inSubnetMaskP = IN_CLASSA_NET;
		} else if (IN_CLASSB(*inAddrP)) {
			 *inAddrP |= ~IN_CLASSB_NET;
			 *inSubnetMaskP = IN_CLASSB_NET;
		} else if (IN_CLASSC(*inAddrP)) {
			 *inAddrP |= ~IN_CLASSC_NET;
			 *inSubnetMaskP = IN_CLASSC_NET;
		}

		 *inAddrP = htonl(*inAddrP);
		 *inSubnetMaskP = htonl(*inSubnetMaskP);
#endif	/* EXCELAN */

		log("netaddr: local address: %s\n",
			nAddrFmt(&intFaceListP->localAddr));
		log("netaddr: broadcast address: %s\n",
			nAddrFmt(&intFaceListP->broadAddr));
		log("netaddr: subnet mask: %s\n",
			nAddrFmt(&intFaceListP->subnetMask));
	}
}


struct hostent
*myhostent()
{
#if defined(EXCELAN)
	static unsigned long	myHostAddr;
	static struct hostent	myhostentry = {
		(char *)NULL, 4, (char *)&myHostAddr
	};
	extern unsigned long	rhost();
#else
	static struct hostent	myhostentry;
	struct hostent		*tmpentry;
	extern struct hostent	*gethostbyname();
#endif

	static int		gotmyhostent = FALSE;
	extern char		*myhostname();

	if (!gotmyhostent) {
#if defined(EXCELAN)
		myhostentry.h_name = myhostname();
		myHostAddr = rhost(&myhostentry.h_name);
		if (myHostAddr != (unsigned long)-1)
			gotmyhostent++;
#else
		tmpentry = gethostbyname(myhostname());
		if (tmpentry) {
			myhostentry = *tmpentry;
			gotmyhostent++;
		}
#endif	/* EXCELAN */
		else
			fatal("Cannot find hostname '%s' in /etc/hosts.  PCI aborting.\n",myhostname());
	}
	return &myhostentry;
}

#endif	/* UDP42 */



#ifdef	RS232PCI

/* rs232 network stuff */

#ifdef RS232_7BIT
extern int using_7_bits;
#endif /* RS232_7BIT */

/*****************************************************************************
 *    rcvPacket() -
 *        is the receive portion of the DATALINK LAYER between
 *        UNIX and a remote PC.  It uses an RS-232 terminal line
 *        at the physical layer.  Getframe() searches for the start
 *        of a data frame by looking for a  SYNC NULL character
 *        sequence.  Once it finds this sequence it reads  four
 *        more "PROTECTED" bytes which contain the checksum and
 *        frame count.  These four bytes are not to be "destuffed"
 *        and are not examined for SYNC's.  If the frame count in
 *        the header is too large it waits in a "tight" loop
 *        looking for the begining of the next frame, awaiting the
 *        sending PC to timeout.  If at any time the pattern SYNC NULL
 *        is encountered all previous state information is purged
 *        and the new data is taken as an entirely new request.
 *        Getframe() writes the data frame into a buffer whose address
 *        is supplied by the caller and it returns a frame byte count.
 *
 */

int
rcvPacket(inPkt)			/* Returns byte count of frame	*/
struct input
	*inPkt;				/* Pointer to input buffer	*/
{
register int
	i,				/* Loop counter in SYNC search	*/
	bytes,				/* Bytes returned in read call	*/
	count,
	framesize;			/* Flag & temporary storage	*/
int
	flipCode;			/* Byte flipping code */
unsigned short
	tmpshort;

extern ttymodes;
char
	buf[2 * MAX_FRAME];		/* Lowest layer input buffer	*/
/* */
	while (0)
	{
resync:
		nak();
#ifdef	TIMEOUT
		timeout_on();
#endif	/* TIMEOUT */
	}

	count = 0;

	r232dbg(("receive: resync: count = %d\n", count));

	/* Find begining of frame by searching for SYN-NULL sequence. */
	for (;;) {
		if ((bytes = read(theNetDesc, buf, 1)) <= 0) {
			if (errno != EINTR)
				log("receive: TTY Read err: %d\n", errno);
			continue;
		}

#ifdef RS232_7BIT
		if (using_7_bits)
			buf[0] &= 0x7f;
#endif /* RS232_7BIT */

		r232dbg(("want %#x (SYNC), got %#x\n", SYNC, buf[0] & 0xff));
		r232dbg(("bytes = %#x; count = %#x\n", bytes, count));

		if (buf[0] == SYNC) {
			if ((bytes = read(theNetDesc, &buf[1], 1)) <= 0) {
				if (errno != EINTR)
					log("receive: TTY Read err: %d\n",
						errno);
				continue;
			}

#ifdef RS232_7BIT
			if (using_7_bits)
				buf[1] &= 0x7f;
#endif /* RS232_7BIT */

			r232dbg(("want %#x (NULL), got %#x\n", NULL,
				buf[1] & 0xff));
			r232dbg(("bytes = %#x; count = %#x\n", bytes, count));

			if (buf[1] == NULL) {
				count = 2;
				break;
			}
		}
	}

#ifdef	TIMEOUT
	timeout_on(theNetDesc, &ttymodes);
#endif	/* TIMEOUT */
	/* Read in PROTECTED portion of frame */
	while (count < PROTECTED) {
		r232dbg(("SyncNull: bytes = %#x count = %#x\n", bytes, count));

		bytes = read(theNetDesc, &buf[count], PROTECTED - count);
		if (bytes > 0) {
#ifdef RS232_7BIT
			if (using_7_bits) {
				for (i = count; i < count+bytes; i++)
					buf[i] &= 0x7f;
			}
#endif /* RS232_7BIT */
			r232dbg(("count %d; bytes %d;\n", count, bytes));
			count += bytes;
		}
	}

	/* Get FRAME SIZE from protected portion of header. */
#if	HOW_TO_FLIP
	dosflipm(((struct rs232 *)buf)->f_cnt, tmpshort);
#endif	  /* HOW_TO_FLIP	  */
	framesize = ((struct rs232 *)buf)->f_cnt;
#ifdef RS232_7BIT
	if (using_7_bits)
		framesize = ((framesize & 0x7f00) >> 1) | (framesize & 0x7f);
#endif /* RS232_7BIT */

	r232dbg(("framesize %#x\n", framesize));

	/* Do range check on frame size */
	if ((framesize > 2*MAX_FRAME) || (framesize == 0))
		goto resync;

	/* Read in rest of frame */
	while (count < framesize) {
		bytes = read(theNetDesc, &buf[count], framesize - count);
		if (bytes <= 0) {
			if (errno == EINTR) {
				/* It is unlikely that any data has been lost;
				 * just restart the read.
				 */
				continue;
			}
			log("receive: TTY Read err: %d\n", errno);
			goto resync;
		}
#ifdef RS232_7BIT
		if (using_7_bits) {
			for (i = count; i < count+bytes; i++)
				buf[i] &= 0x7f;
		}
#endif /* RS232_7BIT */
		r232dbg(("bytes %d; count %d; chars: %.*s\n", bytes, count,
			bytes, buf));
		count += bytes;
	}

#ifdef RS232_PKTDBG
	rs232_log_packet("Received", buf, count);
#endif /* RS232_PKTDBG */

#if	HOW_TO_FLIP
	dosflipm(((struct rs232 *)buf)->chks, tmpshort);
#endif	  /* HOW_TO_FLIP	  */

	if (((
#ifdef RS232_7BIT
	      using_7_bits ? 0x7f7f :
#endif /* RS232_7BIT */
				      0xffff) & chksum(buf, count)) !=
		((struct rs232 *)buf)->chks) {
		log("chksum: expected %x, got %x\n",
			((struct rs232 *)buf)->chks,
			((
#ifdef RS232_7BIT
			  using_7_bits ? 0x7f7f :
#endif /* RS232_7BIT */
						  0xffff) & chksum(buf,count)));
		goto resync;
	}

	/* Unstuff input buffer */
	count = unstuff(buf, (char *)inPkt, count);

	/* Now do proper range check on frame size */
	if ((count > MAX_FRAME) || (count == 0)) {
		log("bad framesize: %d\n", count);
		goto resync;
	}

#ifdef RS232_7BIT
	/* Convert to 8 bit data if needed */
	if (using_7_bits)
		count = convert_from_7_bits((char *)inPkt, count);
#endif /* RS232_7BIT */

#ifdef	TIMEOUT
	timeout_off(theNetDesc, &ttymodes);
#endif	/* TIMEOUT */
	flipCode = input_swap(inPkt, inPkt->hdr.pattern);

#ifdef RS232_PKTDBG
	rs232_log_packet("Processed", (char *)inPkt, count);
#endif /* RS232_PKTDBG */

	logPacket(inPkt, PLOG_RCV, LOG_SOME);
	return flipCode;
}


/*****************************************************************************
 *  xmtPacket()  takes an output frame and sends it to the appropriate
 *          device driver (rs232) for delivery.
 *
 */

int
xmtPacket(outPacket, niUnused, flipCode)
register struct output
	*outPacket;
struct ni2
	*niUnused;			/* place holder */
register int
	flipCode;
{
#ifdef	RS232_COPY_PROTECTION
extern int	rs232_violation;
#endif
register int
	status,			/* Number of characters written */
	i;
int
	retryCount;
long
	length;
short
	tmpshort;

#ifdef	RS232_COPY_PROTECTION
	if (rs232_violation == TRUE) {
		log("xmtPacket: sending PC_CRASH message.\n");
		outPacket->hdr.req = PC_CRASH;
		outPacket->pre.select = UNSOLICITED;
		outPacket->hdr.t_cnt = 0;
	}
#endif

	length = outPacket->hdr.t_cnt + HEADER;
	outPacket->rs232.syn = SYNC;
	outPacket->rs232.null = NULL;
	outPacket->rs232.f_cnt = length;
	outPacket->rs232.chks = chksum(outPacket, length);

	logPacket(outPacket, PLOG_XMT, LOG_SOME);
	output_swap(outPacket, flipCode);

#ifdef RS232_7BIT
	/* Handle 7 bit output */
	if (using_7_bits) {
		length = convert_to_7_bits(outPacket, length);
		outPacket->rs232.f_cnt = ((length&0x3f80) << 1) | (length&0x7f);
		outPacket->rs232.chks = chksum(outPacket, length) & 0x7f7f;
		sflipm(outPacket->rs232.f_cnt, tmpshort, flipCode);
		sflipm(outPacket->rs232.chks, tmpshort, flipCode);
	}
#endif /* RS232_7BIT */

#ifdef RS232_PKTDBG
	rs232_log_packet("Sending", outPacket, length);
#endif /* RS232_PKTDBG */

	/* Send response */
	for (retryCount = 0; retryCount < MAXXRETRY; retryCount++) {
		if ((status = write(theNetDesc, outPacket, length)) == length)
			break;
		log("send: TTY Write err: %d\n", errno);
	}

#ifdef	RS232_COPY_PROTECTION
	if (rs232_violation ==TRUE)
		fatal("RS232 copy protection violation.\n");
#endif
	return length;
}


/*****************************************************************************
 *  reXmt: retransmit last packet
 */

reXmt(rePacket, reLength)
struct output
	*rePacket;
int
	reLength;
{
	log("reXmt: seq %d\n", rePacket->hdr.seq);

	for (;;) {
		if (write(theNetDesc, rePacket, reLength) == reLength)
			return reLength;

		if (errno == EINTR)
			continue;

		log("reXmt: TTY write err: %d", errno);
		return 0;
	}
}

nak()
{
extern int brg_seqnum;		/* Current sequence number */
extern int swap_how;		/* How to swap bytes */
struct
	output retryPkt;
struct ni2
	*niUnused;			/* place holder */
/* */
	memset(&retryPkt.hdr, 0, sizeof(struct header));

	retryPkt.hdr.seq = ~brg_seqnum;
	retryPkt.hdr.res = FAILURE;
	xmtPacket(&retryPkt, niUnused, swap_how);

	return;
}

#ifdef RS232_PKTDBG

rs232_log_packet(desc, pkt, len)
char *desc;
unsigned char *pkt;
register int len;
{
	register int i;

	r232dbg(("%s packet, len = %d\ndata:\n", desc, len));
	while (len > 0) {
		for (i = 0; i < 16 && len-- > 0; i++)
			r232dbg((" %02x", *pkt++));
		r232dbg(("\n"));
	}
}
#endif /* RS232_PKTDBG */
#endif	/* RS232PCI  */
