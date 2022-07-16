/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_NXT_H
#define _SYS_NXT_H

#ident	"@(#)head.sys:sys/nxt.h	1.12.6.1"

/*
 * nxt.h -- Defines for the windowing terminal streams xt driver. See xt(7).
 */

#define	NPCBUFS	2			/* number of saved packets in proto*/
#define	SEQMOD	8			/* sequence number modulus */
#define	SEQBITS	3			/* bits for sequence number */
#define	MAXPCHAN	8		/* number of chans per xt */
#define	CHANBITS	3		/* number of bits for channel number */
#define	LINK(dev)	((dev >> CHANBITS) & (0xff >> CHANBITS))
#define	SEQMOD		8		/* Seq num modulus */

#define	XTSCANRATE	(2*HZ)		/* call xtscan @ 2 seconds */
#define XTSCANSLOP	(HZ + (HZ/2)) 	/* Increment to exact calculated
					   retransmission timeout
					   to allow for turnaround, the
					   terminal being slow, etc. */

#define	SET_CNTL(x)	(x)|=0100
#define	CHANMASK	07
#define	CHAN(dev)	(dev&CHANMASK)
#define	SEQMASK		07		/* 2**SEQBITS - 1 */

/* define to get packet type  (control or not) */
#define	GET_CNTL(x)	(((x) >> 6) &1)
#define	GET_PTYP(x)	(((x) >> 7) &1)
#define	GET_SEQ(x)	((x) & 7)
#define	GET_CHAN(x)	(((x) >> 3) & 7)


/* These are no longer used by nxt.c since they are always
** enabled. The defines are retained, however, for other commands
** like xtt.
*/
#define XTRACE		1	/* 1 to enable tracing */
#define XTSTATS		1	/* 1 to enable statistics */

typedef long Stats_t;

#define PKTPTSZ 11		/* Pkt part captured for trace */
#define PKTHIST 40		/* Size of trace history */

struct Tpkt
{
	unsigned char	pktpart[PKTPTSZ];	/* record of captured pkts */
	unsigned char	flag;			/* packet direction */
	clock_t		time;			/* log time (ticks) */
};

struct Tbuf
{
	struct Tpkt	log[PKTHIST];	/* history of transactions */
	short		flags;		/* assorted status flags */
	char 		index;		/* next slot */
	char		used;		/* # of slots used */
};


#define S_XPKTS		0
#define S_RPKTS		1
#define	S_CRCERR	2
#define	S_BADACK	3
#define	S_BADNAK	4
#define	S_OUTSEQ	5
#define	S_NAKRETRYS	6
#define	S_RDUP		7
#define	S_RNAK		8
#define	S_XNAK		9
#define	S_RACK		10
#define	S_XACK		11
#define	S_BADHDR	12
#define	S_BADSIZE	13
#define	S_LOSTACK	14
#define	S_BADCNTL	15
#define	S_BADCDATA	16
#define	S_NOMBLK	17
#define	S_BADCOUNT	18
#define	S_BADCHAN	19
#define	S_BADCTYPE	20
#define	S_NORBUF	21
#define	S_RTIMO		22
#define	S_XTIMO		23
#define	S_WIOW		24 /* not used in STREAMS XT driver */
#define	S_WOAS		25 /* not used in STREAMS XT driver */
#define	S_NSTATS	26	/* 'count' macro used only for size below */
				/* make sure this one remains last in the list */

#define STATS(A,B)	(A)->stats[B]++	/* A is a channel pointer, B an offset */


/*
 *	XT Driver Control Structures
 *
 *	One "xtctl" structure is allocated for each instantiation
 *	of the XT driver.
 *
 *	One "xtchan" structure is allocated for each window on
 *	the user's terminal.
 *
 */
struct xt_msg {
	mblk_t *mp;			/* ptr to message */
	clock_t timestamp;		/* stamp for ACK/NAK timeout */
	unsigned char seq;		/* sequence number */
	unsigned char xt_saveoutpkts;	/* outpkts when packet was sent */
};


struct xt_chan {
	queue_t *xt_upq;		/* upstream read queue */
	struct xtctl *xt_ctlp;		/* ptr to ctl struct */
	struct jwinsize xt_jwinsize;	/* Layer parms for JWINSIZE ioctl */
	short xt_chflg;			/* flags */
#ifdef SVR32
	t_pid_t xt_pgrp;		/* proc grp of first opening proc*/
#endif /* SVR32 */
#ifdef SVR40
	pid_t xt_pgrp;			/* proc grp of first opening proc*/
#endif /* SVR40 */
	short xt_channo;		/* channel number for easy ref*/
	short xt_outbufs;		/* slots for outpackets */
	struct xt_msg xt_msg[2];	/* outpacket awaiting ACK */
	unsigned char xt_inseq;		/* expected inpacket sequence num */
	unsigned short xt_bytesent;	/* for flow control in network xt */
#ifdef VPIX
	v86_t *xt_v86p;
	pid_t *xt_v86pid;
	struct proc *xt_v86procp;
#endif /* VPIX */
};

/* flags for channels */
#define	XT_CTL		0x1		/* control channel */
#define	XT_ON		0x2		/* channel has been opened */
#define	XT_WCLOSE	0x4		/* channel being closed */
#define XT_IOCTL	0x8		/* channel processing ioctl */
#define XT_NONETFLOW	0x10		/* network xt flow control disabled */
#define XT_M_STOPPED	0x20		/* channel stopped by user ^S */
#define XT_HOLDFLOW	0x40		/* temp disable net flow - see recvpkt() */
#define XT_WAIT4COPYIN  0x80            /* M_COPYIN for ioctl is pending */

struct xtioctl {
	int xti_seg;			/* which segment of ioctl to process */
	mblk_t *xti_bp;			/* state information for ioctl */
};


struct xtctl {
	struct queue *xt_ttyq;			/* downstream write queue */
	struct xt_chan xt_chan[MAXPCHAN];	/* channels per active tty */
	unsigned char xt_next;			/* sched this chan next */
	unsigned char xt_lastscan;		/* chan which got canput failure */
	short xt_ctlflg;			/* control flags */
	mblk_t *xt_inbp;			/* block for incoming packet */
	mblk_t *xt_pendjagent;			/* pending JAGENT ioctl pkt */
	unsigned short xt_insize;		/* number data bytes expected */
	unsigned short xt_incount;		/* number data bytes left */
	clock_t xt_intime;			/* timestamp for input timeout */
	short xt_instate;			/* state of incoming packet */
	unsigned char xt_inchan;		/* chan number of incoming packet */
	unsigned char xt_maxpkt;		/* max packet data size to terminal -
						   equals 1 for network xt */
	unsigned long xt_ttycflag;		/* dummy c_cflag (termio struct) */
	unsigned char xt_hex;			/* 1 if 6-bit path, 0 if 8-bit	*/
	unsigned char xt_firstchar;		/* first char in pkt on input */
	unsigned char xt_inpktcmd;		/* input packet command byte */
	unsigned char xt_outpkts;		/* total outstanding pkts waiting
						   for an ACK */
	short xt_recvtimo;			/* receive timeout in HZ for
						   current baud rate */
	short xt_HZperpkt;			/* time in HZ to transfer 1
						   outgoing pkt at current baud
						   rate */
	struct Tbuf trace;			/* trace strings stored here */
	Stats_t stats[S_NSTATS];		/* usage statistics */
};

/* xt_ctlflg flags for control struct */
#define	XT_INUSE	0x1	/* xt dev is in use */
#define XT_NETACK	0x2	/* processing incoming network xt ack packet */
#define XT_WANTTIMEOUT	0x4	/* nxtscan() found a chan that needs timeout */
#define XT_EXIT		0x8	/* C_EXIT in progress*/
#define XT_UNLINK	0x10	/* UNLINK in progress*/


/* The following are used for XT tracing. */


#define XMITLOG		0	/* Transmitted packet */
#define RECVLOG		1	/* Received packet */
#define TRACE_BADPKT	0x2	/* Bad packet flag */
#define TRACE_CMDBYTE	0x4	/* Packet has a command byte */

#define TRACEON		0x1	/* tracing enabled */
#define TRACELOCK	0x2	/* tracing locked */
#define TRACE_NETXT	0x4	/* tracing network xt protocol */

static void logpkt();

#define ISTRACEON(CTLP) (((CTLP)->trace.flags&(TRACEON|TRACELOCK))==TRACEON)


/*
 * ioctls for the XT driver
 *
 * Numbers 1, 5 and 6 have been used in old versions of the XT
 * driver and should not be re-used.
 *
 */
#define XTIOCTYPE ('b'<<8)
#define XTIOCSTATS (XTIOCTYPE|2)	/* Get xts statistics */
#define XTIOCTRACE (XTIOCTYPE|3)	/* Enable/Get xtt statistics */
#define XTIOCNOTRACE (XTIOCTYPE|4)	/* Disable xtt tracing */
#define XTIOCHEX (XTIOCTYPE|7)		/* Set LAN encoding */


#endif	/* _SYS_NXT_H */
