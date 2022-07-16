/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)head.sys:sys/xt.h	11.1.7.1"

/*
**	Multiplexed channels driver header
*/

#define	XTRACE		1		/* 1 to include tracing */
#define	XTSTATS		1		/* 1 to enable statistics */
#define	XTDATA		1		/* 1 to enable Link table extraction */

#define	MAXLINKS	32
#define	CHAN(dev)	(dev & ((1<<CHANSIZ) - 1))
#define	LINK(dev)	((dev>>CHANSIZ)&(0xff>>CHANSIZ))

#if	(MAXPCHAN*MAXLINKS) > 256
	ERROR -- product cannot be greater than minor(dev)
#endif

#if	XTRACE == 1
#define	PKTPTSZ		8			/* Packet part to be captured for trace */
#define	PKTHIST		40			/* Size of trace history */

struct Tpkt
{
	Pbyte		pktpart[PKTPTSZ-1];	/* Record of a packet captured */
	Pbyte		flag;			/* Direction */
	time_t		time;			/* Log time in ticks */
};

#define	XMITLOG		0			/* Transmitted packet */
#define	RECVLOG		1			/* Received packet */

struct Tbuf
{
	struct Tpkt	log[PKTHIST];		/* A history of transactions */
	short		flags;			/* Flags */
	char		index;			/* Next slot */
	char		used;			/* Number of slots used */
};

#define	TRACEON		1			/* Trace enabled */
#define	TRACELOCK	2			/* Trace locked */

static void	logpkt();
#define		Logpkt(A,B,C)	if (((B)->l.trace.flags&(TRACEON|TRACELOCK))==TRACEON)logpkt(A,B,C)
#define		LOCKTRACE(A)	(A)->l.trace.flags|=TRACELOCK
#else
#define		Logpkt(A,B,C)
#define		LOCKTRACE(A)
#endif

#if	XTSTATS == 1
enum stats_t
{
	 xpkts, rpkts, crcerr, badack, badnak, outseq
	,nakretrys, rdup, rnak, xnak, rack, xack
	,badhdr, badsize, lostack, badcntl, badcdata
	,nocfree, badcount, badchan, badctype, norbuf
	,rtimo, xtimo
	,wiow, woas
	,nstats
};

#define	S_XPKTS		(int)xpkts
#define	S_RPKTS		(int)rpkts
#define	S_CRCERR	(int)crcerr
#define	S_BADACK	(int)badack
#define	S_BADNAK	(int)badnak
#define	S_OUTSEQ	(int)outseq
#define	S_NAKRETRYS	(int)nakretrys
#define	S_RDUP		(int)rdup
#define	S_RNAK		(int)rnak
#define	S_XNAK		(int)xnak
#define	S_RACK		(int)rack
#define	S_XACK		(int)xack
#define	S_BADHDR	(int)badhdr
#define	S_BADSIZE	(int)badsize
#define	S_LOSTACK	(int)lostack
#define	S_BADCNTL	(int)badcntl
#define	S_BADCDATA	(int)badcdata
#define	S_NOCFREE	(int)nocfree
#define	S_BADCOUNT	(int)badcount
#define	S_BADCHAN	(int)badchan
#define	S_BADCTYPE	(int)badctype
#define	S_NORBUF	(int)norbuf
#define	S_RTIMO		(int)rtimo
#define	S_XTIMO		(int)xtimo
#define	S_WIOW		(int)wiow
#define	S_WOAS		(int)woas
#define	S_NSTATS	(int)nstats

typedef long		Stats_t;

#define	STATS(A,B)	(A)->l.stats[B]++
#else
#define	STATS(A,B)
#endif

struct Channel
{
	struct Pchannel	chan;		/* Protocol state information for this channel */
	struct tty	tty;		/* Virtual tty for this channel */
	struct jwinsize	winsize;	/* Layer parameters for JWINSIZE ioctl */
#ifdef VPIX
	v86_t		*v86p;		/* Pointer to v86 proc in DOS mode */
#endif
};

typedef struct Channel *Ch_p;

/*
**	JAGENT ioctl structure
*/
struct hagent
{
	struct bagent desc;	/* WARNING!!this better be 12 bytes long!!!
					defined in jioctl.h */
	struct cblock *retcb;		/* returning cblock from rcvpkt */
	short flag;			/* flag to synchronise ioctl's */
};

/* flags for hagent struct */
#define	AGASLP	01
#define	AGBUSY	02

struct Linkinfo
{
	struct tty *	line;		/* Real tty for this link */
	char *		rdatap;		/* Pointer into rpkt */
	struct Pktstate	rpkt;		/* Packet being received for this link */
	short		lihiwat;	/* High water mark for real line */
	short		xtimo;		/* Transmission timeout */
	char		rtimo;		/* Receive timeout */
	char		old;		/* Old line discipline for line */
	char		nchans;		/* Number of channels allowed */
	char		lchan;		/* Last channel started */
	char		open;		/* Channel open bits */
	char		xopen;		/* Exclusive open bits */
	struct hagent	agent;		/* ioctl agent structure */
	short		pid;		/* real channel control process */
	short		hex;		/* 1 if 6-bit path, 0 if 8-bit  */
#if	XTRACE == 1
	struct Tbuf	trace;		/* Tracks */
#endif
#if	XTSTATS == 1
	Stats_t		stats[S_NSTATS];/* Statistics */
#endif
};

struct Link
{
	struct Linkinfo	l;		/* Link info */
	struct Channel	chans[1];	/* Array of channels for this link */
};

typedef	struct Link *	Link_p;

/*
**	Ioctl args
*/

#define	XTIOCTYPE	('b'<<8)
#define	XTIOCLINK	(XTIOCTYPE|1)
#define	XTIOCSTATS	(XTIOCTYPE|2)
#define	XTIOCTRACE	(XTIOCTYPE|3)
#define	XTIOCNOTRACE	(XTIOCTYPE|4)
#define	XTIOCDATA	(XTIOCTYPE|5)
#define	HXTIOCLINK	(XTIOCTYPE|6)

/** Link set up request **/

struct xtioclm
{
	char		fd;		/* File descriptor for 'real' 'tty' line */
	char		nchans;		/* Maximum channels that will be used */
};

/** Data descriptor **/

struct xtiocdd
{
	int		size;		/* Sizeof buffer */
	char *		addr;		/* Address of buffer */
};

#define	PCHANMATCH(tp)	((Pch_p)(tp)-1)
#define	LINKMATCH(p)	linkTable[(p)->link]
#define	t_link		t_dstat		/* Use dstat in real tty for linknumber */
