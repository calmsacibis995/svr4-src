/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-io:nxt.c	1.3.2.1"

/*
 * XT --   STREAMS Driver for AT&T windowing terminals (5620, 615, 620, 630)
 *
 *
 * This driver supports two distinct protocols. The "network" protocol
 * is intended for use over end-to-end error correcting networks such
 * as STARLAN. Network xt protocol relies on the underlying network to
 * provide error detection and correction. The "regular" xt protocol is
 * the original xt protocol which was designed for use over RS232.
 * Regular xt protocol provides its own error detection and correction.
 *
 * Since there is currently no host mechanism for determining if the
 * downstream driver provides error correction, the layers command asks
 * the windowing terminal firmware which protocol to use by sending an
 * A_XTPROTO agent command. Layers then makes the final decision about
 * which protocol to use based upon the windowing terminal response and
 * it's command line options. The layers command informs the driver of
 * it's decision through a JXPROTO ioctl.
 *
 * Regular xt packet format is as follows:
 *
 *	Byte 1: 1                 : 1
 *		control flag      : 1
 *		channel number    : 3
 *		sequence number   : 3
 *
 *	Byte 2: size              : 8
 *
 *	Size bytes of data
 *
 *	Two CRC bytes
 *
 * The first bit of the first byte is always 1. The next bit is a
 * control flag with identifies ACK, NAK, UNBLK control packets. The
 * next 3 bits are the xt channel number, and the last 3 bits are
 * sequence number.
 *
 * The second byte is packet data size. In the original xt protocol,
 * maximum packet data size was always 32. With this newer, larger
 * packet xt protocol driver, maximum packet data size is still 32 from
 * the terminal to the host, but it can be up to 252 from the host to
 * the terminal. Larger packets will only be used if the layers command,
 * through the A_XTPROTO agent command, determines that the terminal being
 * talked to has the new larger packet firmware which can handle larger
 * packets.
 *
 * Network xt packet format is as follows:
 *
 *	Byte 1: 0                 : 1
 *		1                 : 1
 *		reset flow control: 1
 *		flow control flag : 1
 *		ACK flag          : 1
 *		channel number    : 3
 *
 *	Byte 2: size high bits    : 8
 *
 *	Byte 3: size low bits     : 8
 *
 *	Size bytes of data
 *
 * In the first byte, the first bit is always zero. This differentiates
 * a network xt packet from a regular xt packet because this bit is
 * always 1 in regular xt.
 *
 * The second bit of the first byte is always 1. This is used as a
 * sanity check, and will also allow the possibility of other new
 * packet types in the future. The next bit is unused.
 *
 * The next bit is the reset flow control flag. It is used to inform
 * the terminal to reset it's flow control count. This bit will be
 * set whenever a packet is sent when chanp->xt_bytesent == 0. In
 * particular, it is sent in the first packet after a channel is
 * opened to initialize the terminals concept of bytes sent.
 * Another special case is that it is possible for an xt channel
 * to be closed and then reopened without being deleted. The
 * layers command does this if an application program does a
 * libwindows New followed by Runlayer (note this is an error in
 * the application - Newlayer should be used if a Runlayer is
 * going to follow - but it is a common bug). When the channel is
 * closed and then reopened, chanp->xt_bytesent gets reset to 0,
 * and the reset bit is used to sync the terminal back up.
 *
 * The next bit is the flow control flag. This bit identifies a flow
 * control packet which expects an eventual ACK response from the
 * receiving side.
 *
 * The next bit is the ACK flag. The ACK flag specifies that the packet
 * is a network xt high/low water flow control ACK packet. An ACK
 * packet contains no data, so the two size bytes are used to specify
 * the number of characters being ACK'ed rather than the number of data
 * bytes.
 *
 * The last three bits of the first byte are channel number. The usage
 * of channel number in network xt is identical to the equivalent field
 * in regular xt.
 *
 * Size is 16 bits in network xt. Following the 2 size bytes are the number
 * of bytes of data specified by the size field. Network xt has no CRC bytes.
 */

#include "sys/types.h"
#include "sys/inline.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "sys/jioctl.h"
#include "sys/termio.h"
#include "sys/stream.h"
#ifdef VPIX
#include "sys/tss.h"
#include "sys/v86.h"
#include "sys/asy.h"
#endif
#include "sys/stropts.h"
#include "sys/nxtproto.h"
#include "sys/nxt.h"
#include "sys/eucioctl.h"
#include "sys/fcntl.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"


#ifdef  DEBUG
/*
** Flags to turn groups of debug statements on and off. These can be
** changed from the DEMON to turn debugging and off without rebooting.
*/
int xtmainflag = 0;	/* debugging initially off */
int xtdebugflag = 1;
int xttraceflag = 0;
int xtpacketflag = 1;
int xtscanflag = 0;
int xtinputflag = 0;
int xtinerrflag = 0;
int xtnetflowflag = 0;
int xtosrvflag = 0;	/* debug stmts in output service procedure */
int xtisblockflag = 0;  /* debug stmts in isblocked() */
int xttmpflag = 1;
int xttmp2flag = 1;
/*
** Expand debug() statements into print statements.
*/
#define debug(args)	   if(xtmainflag && xtdebugflag) xtprintf args
#define tracedebug(args)   if(xtmainflag && xttraceflag) xtprintf args
#define packetdebug(args)  if(xtmainflag && (xtpacketflag||xttraceflag)) xtprintf args
#define scandebug(args)    if(xtmainflag && xtscanflag) xtprintf args
#define inputdebug(args)   if(xtmainflag && xtinputflag) xtprintf args
#define inerrdebug(args)   if(xtinerrflag) xtprintf args
#define netflowdebug(args) if(xtmainflag && xtnetflowflag) xtprintf args
#define osrvdebug(args)	   if(xtmainflag && xtosrvflag) xtprintf args
#define isblockdebug(args) if(xtmainflag && xtisblockflag) xtprintf args
#define tmpdebug(args)	   if(xtmainflag && xttmpflag) xtprintf args
#define tmp2debug(args)	   if(xtmainflag && xttmpflag) xtprintf args
#else
/*
** If DEBUG not defined, debug statements get defined to the
** null statement.
*/
#define debug(args)
#define tracedebug(args)
#define packetdebug(args)
#define scandebug(args)
#define inputdebug(args)
#define inerrdebug(args)
#define netflowdebug(args)
#define osrvdebug(args)
#define isblockdebug(args)
#define tmpdebug(args)
#define tmp2debug(args)
#endif

/* Always define these out of the code. Used to keep debug statements
** around which may be useful in the future, but will only clutter
** debugging output in the meantime.
*/ 
#define nodebug(args)


extern int nxt_count;  		/* number of channel groups - configurable */
extern struct xtctl nxtctl[];	/* xt private data structure - configurable */

STATIC int xt_scanon = 0;	/* turned on if have to scan */


int nxtopen();
int nxtclose();

int nulldev();

STATIC int  xtsetctty();
STATIC int  xtiput();
STATIC int  xtupisrv();
STATIC int  xtisrv();
STATIC void hexdecode();
STATIC int  xtinput();
STATIC int  xtnetinput();
STATIC int  xtsendup();
STATIC void control();
STATIC int  recvpkt();
STATIC void xtack();
STATIC int  xt_crc();
STATIC int  xtwsrv();
STATIC int  xtosrv();
STATIC int  xtqsrv();
STATIC int  xtioccont();
STATIC int  isblocked();
STATIC void makepkt();
STATIC void makenetpkt();
STATIC int  mybcopy ();
STATIC int  xtsend();
STATIC void nxtscan();
STATIC void nxttimeout();
STATIC void logpkt();
STATIC unsigned char bnextchar();
STATIC int  bnextnchars();
STATIC mblk_t *xtallocb();
STATIC mblk_t *xtdupmsg();


/* High and low water marks for streams flow control for data
 * coming upstream (ie from the terminal).
 */
#define INHI	4096
#define INLO	512

/* High and low water marks for data coming downstream (ie from
 * the stream head).
 */
#define OUTHI	1024
#define OUTLO	256


STATIC struct module_info xt_iinfo = {
	49, "xt", 0, INFPSZ, INHI, INLO };


STATIC struct module_info xt_oinfo = {
	49, "xt", 0, INFPSZ, OUTHI, OUTLO };


STATIC struct qinit xtrinit = {
	nulldev, xtupisrv, nxtopen, nxtclose, NULL, &xt_iinfo, NULL };


STATIC struct qinit xtwinit = {
	putq, xtosrv, nxtopen, nxtclose, NULL, &xt_oinfo, NULL };


STATIC struct qinit m_xtrinit = {
	xtiput, xtisrv, nulldev, nulldev, NULL, &xt_iinfo, NULL };


STATIC struct qinit m_xtwinit = {
	nulldev, xtwsrv, nulldev, nulldev, NULL, &xt_oinfo, NULL };


struct streamtab nxtinfo = {
	&xtrinit, &xtwinit, &m_xtrinit, &m_xtwinit };


#define NETXT_HIWAT 384

/* Bytes per second for various baud rates as stored in the
** ctlp->xt_ttycflag variable. The index of this array is
** CBAUD as defined in termios.h. Index 0 (speed B0) is an error,
** and I set it to a reasonable value of 1200 baud to avoid
** the possibility of divide by zero errors.
*/
static short speeds[16] = {
       120,    5,    7,   11,   13,   15,   20,   30,
	60,  120,  180,  240,  480,  960, 1920, 3840
};

/*
 * open routine
 *
 * RETURN VALUES : OPENFAIL
		   	EINVAL : xt being opened as a module
			       : The first channel opened is not a control chan
			EBUSY  : xt control chan opened with the exclusive flag
				 previously
			ENXIO  : channel no. out of range
			EAGAIN : someone is closing the supplied channel
		 : 1   (SUCCESS)
 */
nxtopen(q, dev, flag, sflag)
queue_t *q;
{
	struct xt_chan *chanp;
	int	i;
	long	chan;
	Stats_t	*pstat;
	mblk_t *mop;

	chan = CHAN(dev);

	if (sflag) {
		u.u_error = EINVAL;
		return(OPENFAIL);	/* being opened as a module: no good */
	}
	/*
	 * if the channel is not in INUSE and not a new
	 * control channel open by layers, then fail it.
	 */
	if( ( (nxtctl[LINK(dev)].xt_ctlflg&XT_INUSE) == 0
	    || nxtctl[LINK(dev)].xt_ttyq == (struct queue *)0 )
	    && !(chan == 0 && flag&O_EXCL) )
	{
		u.u_error = EINVAL;
		return(OPENFAIL);
	}

	if ( chan == 0 ) {	/* opening control channel */
		if ( nxtctl[LINK(dev)].xt_ctlflg & XT_INUSE ) {
			/* Can open ctl chan more than once for download */
			/* ioctls, etc.  Just don't let >1 layers processes in. */
			if ( flag & O_EXCL ) {
				u.u_error = EBUSY;
				return(OPENFAIL);
			}
			return(1);
		}

		/* assign active queue to mark business */
		nxtctl[LINK(dev)].xt_ctlflg = XT_INUSE;
		nxtctl[LINK(dev)].xt_instate = PR_NULL;
		nxtctl[LINK(dev)].xt_intime = 0;
		nxtctl[LINK(dev)].xt_inbp = NULL;
		/* default to old style 32 bytes packets */
		nxtctl[LINK(dev)].xt_maxpkt = 32;
		nxtctl[LINK(dev)].xt_outpkts = 0;
		for(i = 0, pstat = &nxtctl[LINK(dev)].stats[0]; i < S_NSTATS; i++)
			*pstat++ = 0;
		chanp = &nxtctl[LINK(dev)].xt_chan[chan];
		q->q_ptr = (char *)chanp;
		WR(q)->q_ptr = q->q_ptr;
		chanp->xt_upq = q;
		for ( i = 0; i < MAXPCHAN; i++) {
			/* init ptr to ctl struct in all chans */
			nxtctl[LINK(dev)].xt_chan[i].xt_ctlp = &nxtctl[LINK(dev)];
			nxtctl[LINK(dev)].xt_chan[i].xt_channo = i;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[0].mp = NULL;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[0].seq = 0;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[0].timestamp = 0;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[1].mp = NULL;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[1].seq = (1) & 07;
			nxtctl[LINK(dev)].xt_chan[i].xt_msg[1].timestamp = 0;
			nxtctl[LINK(dev)].xt_chan[i].xt_outbufs = NPCBUFS;
			nxtctl[LINK(dev)].xt_chan[i].xt_inseq = 0;
			netflowdebug(("nxtopen: bytesent=0 for chan %d\n", i));
			nxtctl[LINK(dev)].xt_chan[i].xt_bytesent = 0;
		}
		chanp->xt_chflg |= (XT_CTL | XT_ON);

		/* Turn on scanning to check for timeouts, if it's
		 * not already running.
		 */
		if ( xt_scanon == 0 )
			nxtscan();

		/* setup control terminal */
		 chanp = &nxtctl[LINK(dev)].xt_chan[0];
		 xtsetctty(q, chanp);
		return(1);
	}
	if ( chan >= MAXPCHAN ) {	/* channel number within legal range? */
		u.u_error = ENXIO;
		return(OPENFAIL);
	}

	chanp = &nxtctl[LINK(dev)].xt_chan[chan];

	if ( chanp->xt_chflg & XT_WCLOSE ) { /* someone else is closing this one */
		u.u_error = EAGAIN;
		return(OPENFAIL);
	}
	if ( chanp->xt_upq ) {	/* open already? */
		/* OK to re-open existing channel */
		return(1);
	}

	/* open a new virtual channel to xt */
	chanp->xt_upq = q;
	chanp->xt_chflg |= XT_ON;
	q->q_ptr = (char *)chanp;
	WR(q)->q_ptr = (char *)chanp;
	/*
	 * make each window a controlling terminal
 	 * so that all the signalling works
	 *
	 */
	xtsetctty(q, chanp);

	return(1);
}
/*
 * makes the channel a controlling stream/terminal
 *
 * RETURNS: none
 */
STATIC int
xtsetctty(q, chanp)
queue_t *q;
struct xt_chan *chanp;
{
	mblk_t *mop;

	if (mop = xtallocb("xtsetctty", sizeof(struct stroptions), BPRI_MED)) {
		register struct stroptions *sop;
		
		mop->b_datap->db_type = M_SETOPTS;
		mop->b_wptr += sizeof(struct stroptions);
		sop = (struct stroptions *)mop->b_rptr;
		sop->so_flags = SO_HIWAT | SO_LOWAT | SO_ISTTY;
		sop->so_hiwat = INHI;
		sop->so_lowat = INLO;
		putnext(q, mop);
	} else {
		u.u_error = EAGAIN;
		return(OPENFAIL);
	}
}


int nxtclose_error;

/*
 * close routine
 *
 * RETURN VALUES : none
 */
nxtclose(q)
queue_t *q;
{
	queue_t * tq;
	struct xt_chan *tchanp;
	struct xt_chan *chanp;
	int i;
	mblk_t * bp;

	chanp = (struct xt_chan *)q->q_ptr;

	if (chanp == 0) {
		nxtclose_error++;
		return;
	}

	chanp->xt_chflg |= XT_WCLOSE; /* don't let a close in now! */

	if ( chanp->xt_chflg & XT_CTL ) {	/* closing control channel */
		/* Rip The Whole Thing Down.
		 * Loop through all channels:
		 *  Disconnect them from the mux,
		 *  Send them a hangup message if still alive.
		 * Then:
		 *  Fall through and release control channel.
		 */
		chanp->xt_ctlp->xt_instate = PR_NULL;
		chanp->xt_ctlp->xt_intime = 0;
		if ( chanp->xt_ctlp->xt_inbp ) {
			freeb(chanp->xt_ctlp->xt_inbp);
			chanp->xt_ctlp->xt_inbp = NULL;
		}

		for ( i = 1; i < MAXPCHAN; i++) {
			tchanp = &chanp->xt_ctlp->xt_chan[i];
			tq = tchanp->xt_upq;
			if (tq == NULL)
				continue;
			if ( tchanp->xt_msg[0].mp ) {
				freemsg(tchanp->xt_msg[0].mp);
				tchanp->xt_msg[0].mp = NULL;
				chanp->xt_ctlp->xt_outpkts -= 1;
			}
			if ( tchanp->xt_msg[1].mp ) {
				freemsg(tchanp->xt_msg[1].mp);
				tchanp->xt_msg[1].mp = NULL;
				chanp->xt_ctlp->xt_outpkts -= 1;
			}

			tchanp->xt_upq->q_ptr = NULL;
			WR(tchanp->xt_upq)->q_ptr = NULL;
			while ( (bp = getq(WR(tq))) != NULL ) {
				freemsg(bp);
				bp = NULL;
			}
			tchanp->xt_upq = NULL;
			tchanp->xt_ctlp = NULL;
			tchanp->xt_chflg = 0;
#ifdef VPIX
           		chanp->xt_v86p = (v86_t *)0;
           		chanp->xt_v86pid = (pid_t)0;
           		chanp->xt_v86procp = (struct proc *) 0;
#endif
			putctl(tq->q_next, M_HANGUP);
		}
		chanp->xt_ctlp->xt_ctlflg = 0;	/* clear "in use" flag */
		chanp->xt_ctlp->xt_hex = 0;	/* clear hex (encoding) flag */
		chanp->xt_ctlp->trace.flags = 0; /* clear trace flag */
	}
	/* release channel */
	if (chanp->xt_upq == NULL) {
		chanp->xt_chflg = 0;
		return;
	}
	chanp->xt_upq->q_ptr = NULL;
	WR(chanp->xt_upq)->q_ptr = NULL;

	if ( chanp->xt_msg[0].mp ) {
		freemsg(chanp->xt_msg[0].mp);
		chanp->xt_msg[0].mp = NULL;
		chanp->xt_msg[0].timestamp = 0;
		chanp->xt_ctlp->xt_outpkts -= 1;
	}
	if ( chanp->xt_msg[1].mp ) {
		freemsg(chanp->xt_msg[1].mp);
		chanp->xt_msg[1].mp = NULL;
		chanp->xt_msg[1].timestamp = 0;
		chanp->xt_ctlp->xt_outpkts -= 1;
	}
	chanp->xt_outbufs = NPCBUFS;
	flushq(WR(chanp->xt_upq), FLUSHDATA);
	chanp->xt_upq = NULL;
	chanp->xt_chflg = 0;
	netflowdebug(("nxtclose: bytesent=0 for chan %x\n", (int)chanp));
	chanp->xt_bytesent = 0;
	return;
}


/*
 * input side put procedure
 *
 * only queue messages when allocation fails, or
 * when others are already queued
 *
 * RETURN VALUES : none 
 */
STATIC
xtiput(q, bp)
queue_t *q;
mblk_t *bp;
{
	struct xtctl *ctlp;


	if ( bp == NULL )
		return;
	switch (bp->b_datap->db_type) {

	case M_BREAK: /* new M_SIG - send them up! */
	case M_PCSIG:
	case M_SIG:
	case M_ERROR:
	case M_HANGUP:
		/* 
		 * errors and hangups are routed to
		 * the control channel
		 *
		 */
		debug(("xtiput: case M_HANGUP\n"));
		ctlp = (struct xtctl *)WR(q)->q_ptr;
		if ( q = ctlp->xt_chan[0].xt_upq)
			putnext(q, bp);
		else
			freemsg(bp);
		break;

	case M_FLUSH: /* Loop these back down, if necessary */
		debug(("xtiput: case M_FLUSH\n"));
		if (*bp->b_rptr & FLUSHR) {
			flushq(q, FLUSHDATA);
			*bp->b_rptr &= ~FLUSHR;

		}
		if (*bp->b_rptr & FLUSHW) {
			/* nothing to flush on the lower write side */
			qreply(q,bp);
		}
		else
			freemsg(bp);
		break;

	case M_DATA:
		debug(("In xtiput M_DATA: q_first = %d\n", q->q_first));

		if ( q->q_first ) {
			putq(q, bp);
		}
		else if( xtinput(q, &bp) == 0 ) {
			debug(("xtiput: xtinput returned 0 - "));
			if(bp) {
				debug(("doing putq\n"));
				putq(q, bp);
			}
			else {
				debug(("no putq\n"));
			}
		}
		break;
	default:
		debug(("xtiput - freeing unknown mesg type: db_type = %d\n", bp->b_datap->db_type));
		freemsg(bp);
		break;

	}
}


/*
 * Service procedure for upper read q. This, rather than xtisrc(), will be
 * enabled by xt_upq after canput() failure in recvpkt() because this is the
 * backq of xt_upq.
 *
 * RETURN VALUES : none
 */
STATIC
xtupisrv(q)
queue_t *q;
{
	struct xtctl *ctlp;

	debug(("xtupisrv called\n"));

	if(q->q_ptr) {
		if( ctlp = ((struct xt_chan *)q->q_ptr)->xt_ctlp ) {
			if(ctlp->xt_ttyq) {
				/* RD(ctlp->xt_ttyq) is the lower read
				** queue, which is the queue which actually
				** needs to be woken up.
				*/
				qenable( RD(ctlp->xt_ttyq) );
			}
		}
	}
}

/*
 * input side lower service procedure
 * processes messages received from the tty driver
 * calls xtinput if there is any message on the queue
 *
 * This is also the service procedure scheduled by nxtscan() for
 * timeout processing. If timeout processing in needed, it calls
 * nxttimeout().
 *
 * RETURN VALUES : none
 */
STATIC
xtisrv(q)
queue_t *q;
{
	mblk_t *bp;
	struct xtctl *ctlp;
	int first;

	debug(("Entered xtisrv\n"));
#ifdef DEBUG
	if(q == 0) cmn_err(CE_CONT, "xtisrc: PANIC: q is 0\n");
#endif
	/* q->q_ptr could be NULL if already I_UNLINK'ed
	 */
	if( (ctlp = (struct xtctl *)WR(q)->q_ptr) == NULL)
		return;

	first = 1;
	while ( (bp = getq(q)) != NULL || (first && ctlp->xt_inbp != NULL) ) {
		first = 0;
		debug(("xtisrc calling xtinput\n"));
		if ( xtinput(q, &bp) == 0 ) {
			/* buffer allocation failure or upqueue full */
			debug(("xtisrv: xtinput returned 0 - "));
			if(bp) {
				debug(("doing putbq\n"));
/* XXX */			putbq(q, bp);
			}
			else {
				debug(("no putbq\n"));
			}
			goto dotimeout;
		}
	}

dotimeout:
	/* If first is still set, there was no input, and if there was no
	 * input there is a potential for input timeouts (we may have been
	 * enabled by nxtscan()). Also, need to check for output timeouts
	 * if XT_WANTTIMEOUT was set by nxtscan().
	 */
	if( first || (ctlp->xt_ctlflg&XT_WANTTIMEOUT) )
		nxttimeout(ctlp);
}


/*
 * hex decoding :
 *
 * format of input data (cp2) :
 *
 *      -------------------------------------------------------
 *	| 0x |  2 MSB's of c1  | 2 MSB's of c2 | 2 MSB's of c3 |
 *	-------------------------------------------------------
 *	| 01 |  		6 LSB's of c1          	       |
 *	-------------------------------------------------------
 *	| 01 |  		6 LSB's of c2                  |
 *	-------------------------------------------------------
 *	| 01 |  		6 LSB's of c3                  |
 *	-------------------------------------------------------
 *
 *	x = 0 at the beginning of a packet and 1 thereafter.
 *
 *	count = 2 for just c1
 *	      = 4 for c1 and c2
 *	      = 6 for c1, c2 and c3
 *
 * format of output data (cp1) :
 *
 *      ------
 *	| c1 |
 *	-----
 *	| c2 |
 *	-----
 *	| c3 |
 *	-----
 *
 * hex encoding as done by hexencode is exactly the reverse of this 
 * 
 * RETURN VALUES : none
 */
STATIC void
hexdecode(q, bp)
queue_t *q;
mblk_t **bp;
{
	struct xtctl *ctlp;
	mblk_t * xp, *yp;
	unsigned char	*cp1, *cp2;
	static unsigned char	temp;
	static short	count = 6;

	ctlp = (struct xtctl *)WR(q)->q_ptr;

	xp = yp = *bp;
	cp1 = cp2 = (*bp)->b_rptr;
	/* decode the message in cp2 and put it in cp1		    */
	for (; ; ) {
		/* Ignore bytes with value < 0x40 because all hex   */
		/* encoded bytes are ored with 0x40. The starting   */
		/* of a new sequence could be signified by 0x20 */

		if (*cp2 < 0x40 && ((*cp2 & 0xe0) != 0x20)) {
			cp2++;
			if (cp2 >= yp->b_wptr) {
				if (yp->b_cont == NULL) {
					break;
				} else {
					yp = yp->b_cont;
					cp2 = yp->b_rptr;
				}
			}
			continue;
		}
		count += 2;
		/* if a new sequence of 3 bytes (count = 8) or if 0x20   */

		if (((*cp2 & 0xe0) == 0x20) || (count == 8)) {
			count = 0;
			temp = *cp2; /* This is the first data byte  */
				     /* and contains the MSB's       */
		} else {
			/* the byte pointed to by cp2 contains 6 LSB's
			   and temp contains the 2 MSB's             */
			*cp1++ = (*cp2 & 0x3f) | ((temp << count) & 0xc0);
			/* get next output byte			     */
			if (cp1 >= xp->b_wptr) {
				if (xp->b_cont == NULL) {
					break;
				} else {
					xp = xp->b_cont;
					cp1 = xp->b_rptr;
				}
			}
		}
		cp2++;		/* get next input byte    	     */
		if (cp2 >= yp->b_wptr) {
			if (yp->b_cont == NULL)
				break;
			else {
				yp = yp->b_cont;
				cp2 = yp->b_rptr;
			}
		}
	}
	xp->b_wptr = cp1;
	if (xp->b_cont != NULL) {
		freemsg(xp->b_cont);
		xp->b_cont = NULL;
	}
}

/*
 * input processing at bottom of multiplexor
 *
 * input state machine  :
 *
 *	PR_NULL 	: expecting a new packet
 *	PR_SIZE 	: expecting data byte count
 *	PR_DATA 	: expecting actual data
 * 	PR_GETBUF	: get buffer for putting the received message
 *
 * states added for network xt :
 *
 *	PR_SIZE1   	: expecting high byte of data count
 * 	PR_SIZE2   	: expecting low byte of data count
 *	PR_NETDATA 	: expecting data 
 *	PR_NETSENDUP  	: send data upto the proper channel
 * 	PR_NETGETBUF	: get buffer for putting the received message
 *
 * Please note the following :
 *
 * Even though the terminal sends a packet as a whole, the tty driver may
 * send the packet in various chunks and there is a possibility of the 
 * various bytes being received in different M_DATA messages. The state machine
 * therefore, preserves the state expected for the new message.
 *	
 * RETURN VALUES : 0 - Failure to allocate buffer to store the input data 
 *		   1 - Success
 */
STATIC int
xtinput(q, bp)
queue_t *q;
mblk_t **bp;
{
	struct xtctl *ctlp;
	mblk_t *bpn;
	extern unsigned char bnextchar();
	int retval;
#ifdef DEBUG
	mblk_t *bpdebug;
#endif

	ctlp = (struct xtctl *)WR(q)->q_ptr;

	/* Throw away any input which arrives after we get a valid exit
	 * packet. See comment in case C_EXIT of recvpkts(). Note that
	 * ctlp could be NULL if already I_UNLINK'ed.
	 */
	if( ctlp == NULL || (ctlp->xt_ctlflg&XT_EXIT) ) {
		if(*bp) {
			freemsg(*bp);
			*bp = 0;
		}
		return(1);
	}

#ifdef DEBUG
	for (bpdebug = *bp ; bpdebug ; bpdebug = bpdebug->b_cont) {
		unsigned char *s;
		if (bpdebug->b_datap->db_type != M_DATA)
			cmn_err(CE_CONT, "xtinput: Panic Non M_DATA type\n");
		inputdebug(("RAW INPUT:"));
		for(s = bpdebug->b_rptr ; s < bpdebug->b_wptr ; ++s) {
			char tmp[2];
			tmp[0] = *s>0x20 && s[0]<0x80 ? *s : '^';
			tmp[1] = '\0';
			inputdebug(("%x(%s)", (int)*s, tmp));
		}
	}
	inputdebug(("\n"));
#endif

	bpn = ctlp->xt_inbp;

	/* Handle LAN encoding mode.
	 *
	 * ctlp->xt_hex == 1	Encoding is set.
	 *
	 * ctlp->xt_hex == 2	Encoding is set, but this *bp
	 *			has already been decoded.
	 */
	if (ctlp->xt_hex == 1 && *bp)
		hexdecode(q, bp);

	while ( *bp || (ctlp->xt_instate & PR_NOINPUT) ) {

		switch (ctlp->xt_instate) {

		/* In the middle of processing a network xt packet */
		case PR_NETSIZE1:
		case PR_NETSIZE2:
		case PR_NETGETBUF:
		case PR_NETGETCMD:
		case PR_NETDATA:
		case PR_NETLOGPKT:
		case PR_NETSENDUP:
		case PR_NETERROR:
			retval = xtnetinput(q, bp);
			if(ctlp->xt_instate != PR_NULL) {
				debug(("xtnetinput returned %d\n", retval));
				return(retval);
			}
			bpn = NULL;
			continue;

		/* New packets are expected in this state */
		case PR_NULL: 
			ctlp->xt_firstchar = bnextchar(bp);

			if ( GET_PTYP(ctlp->xt_firstchar) != 1 ) {
				if( ctlp->xt_maxpkt == 1) { /* network xt */
					debug(("Found a net xt packet\n"));
					ctlp->xt_instate = PR_NETNULL;
					retval = xtnetinput(q, bp);
					if(ctlp->xt_instate != PR_NULL) {
						debug(("xtnetinput returned %d (2)\n", retval));
						return(retval);
					}
					bpn = NULL;
					continue;
				}
				else {
					if (*bp && ISTRACEON(ctlp))
						logpkt(ctlp->xt_firstchar,
						 (unsigned short)0, (char)0,
						 (unsigned char *)0,
						 ctlp, (RECVLOG|TRACE_BADPKT) );
					ctlp->trace.flags |= TRACELOCK;
					inerrdebug(("B"));
					debug(("xtinput: Bad Header\n"));
					STATS(ctlp, S_BADHDR);
					break;
				}
			}

			ctlp->xt_instate = PR_GETBUF;
			/* no break - fall through */

		case PR_GETBUF:
			if ( bpn ) {
			/*
			 * If bpn != NULL and it's in this state, we
			 * got problems.  Throw the existing part of
			 * the buffer away and start over.
			 */
				cmn_err(CE_NOTE, "xtinput: non-NULL bpn");
				bpn->b_wptr = bpn->b_rptr;
			} else if ( (bpn = xtallocb("xtinput", (PKTHEADSIZE + MAXPKTDSIZE + EDSIZE), BPRI_MED)) == NULL ) {
				STATS(ctlp, S_NOMBLK);
				ctlp->xt_inbp = bpn;
				if( ctlp->xt_hex ) {
					if( *bp ) ctlp->xt_hex = 2;
				}
				(void)bufcall((PKTHEADSIZE + MAXPKTDSIZE + EDSIZE),
				    BPRI_MED, qenable, q);
				return(0);
			}

			*bpn->b_wptr++ = ctlp->xt_firstchar;
			ctlp->xt_instate = PR_SIZE;
			ctlp->xt_intime = lbolt;
			break;

		case PR_SIZE:

			/* Next byte in the buffer is dsize byte of the protocol*/
			ctlp->xt_insize = ctlp->xt_incount = (unsigned short)bnextchar(bp);
			*bpn->b_wptr++ = (unsigned char)ctlp->xt_insize;

			if ( ctlp->xt_insize > MAXPKTDSIZE ) {
				if (*bp && ISTRACEON(ctlp))
					logpkt(ctlp->xt_firstchar,
					 ctlp->xt_insize, (char)0,
					 (unsigned char *)0, ctlp,
					 (RECVLOG|TRACE_BADPKT) );
				ctlp->trace.flags |= TRACELOCK;
				inerrdebug(("A"));
				debug(("xtinput: Bad Size\n"));
				STATS(ctlp, S_BADSIZE);
				freemsg(bpn);
				ctlp->xt_instate = PR_NULL;
				ctlp->xt_intime = 0;
				bpn = NULL;
				break;
			}
			ctlp->xt_incount += EDSIZE; /* crc bytes */
			ctlp->xt_instate = PR_DATA;
			break;

		case PR_DATA:
			/* Data receiving mode */

			retval = bnextnchars(bp, bpn->b_wptr, ctlp->xt_incount);
			bpn->b_wptr += retval;
			ctlp->xt_incount -= retval;
			if ( ctlp->xt_incount > 0 ) {
				goto out;
			}

			ctlp->xt_intime = 0;

			if ( xt_crc(bpn->b_rptr, 2 + (int)(bpn->b_rptr[1])) ) {
				inerrdebug(("Y"));
				debug(("xtinput: crc error, packet discarded\n"));
				STATS(ctlp, S_CRCERR);
				freemsg(bpn);
				ctlp->xt_instate = PR_NULL;
				bpn = NULL;
				break;
			}

			if( ISTRACEON(ctlp) )
				logpkt(ctlp->xt_firstchar, ctlp->xt_insize,
				       (char)0, bpn->b_rptr+2, ctlp, RECVLOG);

			bpn->b_wptr -= EDSIZE; /* don't send the crc bytes up! */

			ctlp->xt_instate = PR_SENDUP;
			/* no break - fall through */

		case PR_SENDUP:
			/* Good packet - send it upstream */

			if( xtsendup(ctlp, bpn) == 0 ) {
				/* Couldn't put upstream. Try again later.
				 */
				if( ctlp->xt_hex ) {
					if( *bp ) ctlp->xt_hex = 2;
				}
				ctlp->xt_inbp = bpn;
				return(0);
			}

			/* Made it! */
			ctlp->xt_instate = PR_NULL;
			bpn = NULL;
			break;
		}
	}
out:
	ctlp->xt_inbp = bpn;
	if (ctlp->xt_hex == 2) 
		ctlp->xt_hex = 1;
	return(1);
}


/*
 * xtnetinput(): Network xt version of xtinput().
 * 
 *  RETURN VALUES : 0 - Failure to allocate buffer to store input data 
 *		  : 1 - Success
 */
STATIC int
xtnetinput(q, bp)
queue_t *q;
mblk_t **bp;
{
	struct xtctl *ctlp;
	mblk_t *bpn;
	extern unsigned char bnextchar();
	int retval;
	short bufsize;

	ctlp = (struct xtctl *)WR(q)->q_ptr;

	bpn = ctlp->xt_inbp;

#ifdef DEBUG
	if(ctlp->xt_intime != 0)
		debug(("xtnetinput: xt_intime not 0\n"));
#endif

	while ( *bp || (ctlp->xt_instate & PR_NOINPUT) ) {

		switch (ctlp->xt_instate) {

		case PR_NETNULL: 
			/* New packets are expected in this state */
			tracedebug(("NETINPUT: header: "));
			tracedebug(("%x ", (int)ctlp->xt_firstchar & 0xff ));

			/* Check the top bits are "01". This test should
			** never fail.
			*/
			if( (ctlp->xt_firstchar & 0xC0) != 0x40 ) {
				/** This can happen if someone turns their terminal
				*   off in layers or if firmware crashes. That
				*   action should not cause console messages,
				*   so I reluctantly comment out this warning
				*   message. After this, if bugs are found,
				*   the only symptom will be the xt session
				*   hanging.
				cmn_err(CE_NOTE, "XT: xtnetinput: bad header");
				**/
				ctlp->xt_instate = PR_NETERROR;
				break;
			}

			/* Check if this is a flow control ACK packet */
			if(ctlp->xt_firstchar&0x8)
				ctlp->xt_ctlflg |= XT_NETACK;

			ctlp->xt_inchan = ctlp->xt_firstchar & 0x7;
			ctlp->xt_instate = PR_NETSIZE1;
			break;

		case PR_NETSIZE1: 
			/* Get the first size byte */
			ctlp->xt_insize = (((unsigned short)bnextchar(bp)) << 8) & 0xff00;
			tracedebug(("%x ", (ctlp->xt_insize>>8) & 0xff ));
			ctlp->xt_instate = PR_NETSIZE2;
			break;

		case PR_NETSIZE2:
			/* Get the second size byte */
			ctlp->xt_insize |= ((unsigned short)bnextchar(bp)) & 0xff;
			tracedebug(("%x: ", ctlp->xt_insize&0xff ));

			/* Process flow control ACK packet. Size is number
			 * of bytes which are being acknowledged, and there
			 * is no data part.
			 */
			if(ctlp->xt_ctlflg & XT_NETACK) {
				struct xt_chan *chanp;

				ctlp->xt_ctlflg &= ~XT_NETACK;

				chanp = &(ctlp->xt_chan[ctlp->xt_inchan]);

				/* If insize is greater than bytesent,
				 * assume that bytesent has been reset by
				 * nxtclose() and this is a needless ACK
				 * sent by the terminal because it has not
				 * received a packet with the reset flow
				 * control flag set yet (see "reset flow
				 * control packet" in the big comment at
				 * the beginning of this file). Just set
				 * bytesent to 0. The "reset flow control
				 * bit" will sync things back up
				 * eventually. The only bad effect of this
				 * is that the host could end up sending
				 * some extra characters to the terminal
				 * before things sync up, but the problem
				 * is obscure and this effect is not
				 * critical
				 */
				if(ctlp->xt_insize > chanp->xt_bytesent) {
					chanp->xt_bytesent = 0;
				}
				else {  /* Normal case - decrement bytesent */
					chanp->xt_bytesent -= ctlp->xt_insize;
				}

				netflowdebug(("NETACK: xt_bytesent chan %d -= %d to %d\n",
				    chanp - ctlp->xt_chan, ctlp->xt_insize,
				    chanp->xt_bytesent));

				if (chanp->xt_upq && WR(chanp->xt_upq))
					qenable(WR(chanp->xt_upq));

				if( ISTRACEON(ctlp) )
					logpkt(ctlp->xt_firstchar, ctlp->xt_insize,
					       (char)0, (unsigned char *)0,
					       ctlp, RECVLOG);
				STATS(ctlp, S_RACK);

				ctlp->xt_instate = PR_NULL;
				goto out; /* new packets start in xtinput() */
			}

			ctlp->xt_incount = ctlp->xt_insize;

			if ( ctlp->xt_insize > 1025 ) {
				/** See comment with "bad header" message above.
				cmn_err(CE_NOTE, "XT: xtnetinput: bad packet size");
				**/
				ctlp->xt_instate = PR_NETERROR;
				break;
			}

			ctlp->xt_instate = PR_NETGETBUF;
			/* no break - fall through */

		case PR_NETGETBUF:
			/* allocb a new buffer */
			if ( bpn ) {
				/* If bpn != NULL and it's in this state, we
				 * got problems. Should never happen.
				 */
				cmn_err(CE_NOTE, "xtnetinput: bpn non-NULL");
			}

			/* ctlp->xt_insize includes the command byte which
			** we will store in ctlp->xt_inpktcmd (this is
			** an efficiency hack to prevent going to larger
			** buffers to handle the command byte). So, decrement
			** insize. If, however, there are no data bytes leave
			** the size as one, allocb a buffer of one byte, but
			** never use it. This just gives recvpkt() something
			** to chew on, thereby saving a lot of tests for
			** no bp in recvpkt().
			*/
			bufsize = ctlp->xt_insize;
			if(bufsize > 1)
				--bufsize;

			if ( (bpn = xtallocb("xtnetinput", bufsize, BPRI_MED)) == NULL ) {
				STATS(ctlp, S_NOMBLK);
				(void)bufcall(bufsize, BPRI_MED, qenable, q);
				return(0);
			}
			ctlp->xt_instate = PR_NETGETCMD;
			break;

		case PR_NETGETCMD:
			/* Get the command byte */
			ctlp->xt_inpktcmd = bnextchar(bp);
			tracedebug(("%x ", (int)ctlp->xt_inpktcmd & 0xff));
			if(--ctlp->xt_incount > 0)
				ctlp->xt_instate = PR_NETDATA;
			    else
				ctlp->xt_instate = PR_NETLOGPKT;
			break;

		case PR_NETDATA:
			/* Data receiving mode */

			retval = bnextnchars(bp, bpn->b_wptr, ctlp->xt_incount);
#ifdef DEBUG
			{
			unsigned char *p;
			unsigned char s[2];
			tracedebug(("DATA: "));
			for(p = (bpn->b_wptr - retval) ; p < bpn->b_wptr ; ++p) {
				s[0] = *p>0x20 && *p<0x80 ? *p : '^';
				s[1] = '\0';
				tracedebug(("%x(%s)", (unsigned)(*p), s));
			}
			tracedebug(("\n"));
			}
#endif
			bpn->b_wptr += retval;
			ctlp->xt_incount -= retval;
			if ( ctlp->xt_incount > 0 ) {
				debug(("send: ran out of chars before packet\n"));
				goto out;
			}

			ctlp->xt_instate = PR_NETLOGPKT;
			/* no break - fall through */

		case PR_NETLOGPKT:
			if( ISTRACEON(ctlp) )
				logpkt(ctlp->xt_firstchar, ctlp->xt_insize,
				       ctlp->xt_inpktcmd, bpn->b_rptr,
				       ctlp, (RECVLOG|TRACE_CMDBYTE));
			STATS(ctlp, S_RPKTS);

			ctlp->xt_instate = PR_NETSENDUP;
			/* no break - fall through */

		case PR_NETSENDUP:
			/* Send the message upstream.
			**
			** Return 0 if recvpkt returned 0 because it
			** could not sent the data upstream. Processing
			** will eventually get started again by streams
			** flow control as a result of the canput() failure
			** in recvpkt().
			*/
			debug(("xtnetinput: calling recvpkt: "));
			debug(("insize %d inchan %d\n", (int)ctlp->xt_insize,
				(int)ctlp->xt_inchan));
			if(recvpkt(ctlp, bpn, (int)ctlp->xt_insize,
			     (int)ctlp->xt_inchan, ctlp->xt_inpktcmd) == 0) {
				debug(("recvpkt returned 0 - xtnetinput returning 0\n"));
				ctlp->xt_inbp = bpn;
				return(0);
			}
			debug(("recvpkt returns success\n"));

			ctlp->xt_instate = PR_NULL;
			bpn = NULL;
			goto out; /* new packets start in xtinput() */

		case PR_NETERROR:
			/* If network xt gets bad input, there is a problem.
			** Possibly a firmware bug. Possibly someone went
			** through starlan to a 3B2, cu'ed to another
			** machine over RS232, and executed layers on that
			** remote machine. Either way, it is an error and
			** I don't think that code to attemt to re-sync
			** is justified. So, instead I only attempt to
			** detect errors, print a message, and then eat any
			** additional input. The main point is to prevent
			** crashing the host. Above cases print the error.
			** This case eats the additional input.
			*/
			if(*bp) {
				freemsg(*bp);
				*bp = 0;
			}
			debug(("In xtnetintput case PR_NETERROR\n"));
			goto out;
		}
	}
out:
	ctlp->xt_inbp = bpn;
	return(1);
}


/*
 * Handles an XT packet received from the terminal.
 *
 * RETURN VALUES : 1 - Complete.
 *		   0 - Recvpkt returned 0 because of canput failure.
 *		       Try again later.
 */
STATIC int
xtsendup(ctlp, bp)
struct xtctl *ctlp;
mblk_t *bp;
{
	unsigned char	header, count, channo;
	struct xt_chan *tchanp;
	int lastseq1, lastseq2;

	if (!bp)
		return(1);

	/* start deciphering packet---read header containing chan num, seq num
	 * other control info and read count, containing size of packet
	 */
	header = *bp->b_rptr++;
	count = *bp->b_rptr++;
	channo = GET_CHAN(header);
	tchanp = &ctlp->xt_chan[channo];

	if ( channo >= MAXPCHAN ) {
		inerrdebug(("D"));
		debug(("xtsendup: bad channel number\n"));
		STATS(ctlp, S_BADCHAN);
		freemsg(bp);
		return(1);	/* channel number out of range */
	}

	/* now basically switch on msg type---there are 3 possible actions:
	 *   (1)  packet is a control packet
	 *   (2)  packet is data, and it is proper sequence for input channel
	 *   (3)  packet is out of sequence - could be either a valid
	 *	  retransmission or an out of sequence error condition.
	 *
	 * all ack'ing is done from lower level functions called from here
	 */

	if (GET_CNTL(header) ) {			  /* CONTROL MESSAGE */
		control(ctlp, bp, (int) count, header, channo);
	}
	else if ( GET_SEQ(header) == tchanp->xt_inseq ) { /* GOOD DATA PACKET */
		ctlp->xt_inpktcmd = *bp->b_rptr++;
		if ( recvpkt(ctlp, bp, (int)count, (int)channo, ctlp->xt_inpktcmd) ) {
			STATS(ctlp, S_RPKTS);
			tchanp->xt_inseq = ((tchanp->xt_inseq) + 1) & 07;
			xtack(ctlp, header, ACK);
		} else {
			/* Canput failure in recvpkt. Restore bp to the state
			 * it was in before this routine was called and return
			 * 0 to tell xtinput to try again later.
			 */
			inerrdebug(("U"));
			debug(("xtsendup: Upstream blocked\n"));
			bp->b_rptr -= 3;  
			return(0);
		}
	}
	else {						/* OUT OF SEQUENCE PACKET */

		/* Figure out the last two sequence numbers previous to
		 * tchanp->xt_inseq, taking care of wrap around.
		*/
		lastseq1 = (int)(tchanp->xt_inseq + (SEQMOD-1)) % SEQMOD;
		lastseq2 = (int)(tchanp->xt_inseq + (SEQMOD-2)) % SEQMOD;

		/* If this is the previous or second previous sequence
		 * number, assume this is a previously received valid
		 * retransmission from the terminal. Ack it in case the
		 * previous ack was lost on the way to the terminal. Note
		 * that only the last two sequence numbers are considered
		 * retranmissions because xt is a double buffered protocol
		 * (sliding window of two).
		 */
		if(GET_SEQ(header) == lastseq1 || GET_SEQ(header) == lastseq2) {
			inerrdebug(("R"));
			debug(("xtsendup: Valid retransmission seq %d\n",
				GET_SEQ(header)));
			STATS(ctlp, S_RDUP);
			xtack(ctlp, header, ACK);
		}

		/* Otherwise this is an out of sequence error condition,
		 * so nak the packet.
		 */
		else {
			inerrdebug(("S"));
			debug(("xtsendup: Out of sequence packet seq %d\n",
				GET_SEQ(header)));
			STATS(ctlp, S_OUTSEQ);
			xtack(ctlp, header, NAK);
		}

		/* Either way, we have no further use for this packet,
		 * so free it.
		 */
		freemsg(bp);
	}

	return(1);
}
/*
 * control - deal with received control packet from the DMD
 * The following control types are recognised :
 * ACK with or without UNBLK  
 * NAK
 * no data (i.e., data-size = 0)  is treated the same as ACK 
 *
 * RETURN VALUES : none
 */
STATIC void
control(ctlp, bp, count, header, channo)
struct xtctl *ctlp;
mblk_t *bp;
int	count;
unsigned char	header, channo;
{
	struct xt_chan *chanp;
	mblk_t * bp1;
	unsigned char	ctltype;

	if (!bp)
		return;
	chanp = &ctlp->xt_chan[channo];
	if ( (chanp->xt_chflg & XT_ON) == 0 ) {
		freemsg(bp);
		return;
	}

	if ( count == 0 ) /* if datacount = 0, it's an ACK */
		ctltype = ACK;
	else /* otherwise, type = the first data byte */
		ctltype = *bp->b_rptr;

	switch ( ctltype ) {
	case ACK:
		/* ACK could be received with or without UNBLK. If the ACK is 
		   accompanied by an UNBLK, the no. of available buffers
		   (NPCBUFS) gets incremented. Note that messages are
		   freed up regardless of whether UNBLK was received or not.
		*/
		STATS(ctlp, S_RACK);

		/* If the message that was ACKed corresponds to the message 
  		   in buffer 0,  free that message and move any outstanding
		   message in buffer 1 to buffer 0.
		*/
		if ( (chanp->xt_msg[0].seq == GET_SEQ(header)) &&  (chanp->xt_msg[0].mp != NULL) ) {
			if ( (*(bp->b_rptr + 1) == C_UNBLK) &&  ((bp->b_wptr - bp->b_rptr) > 1) )
				if ( ++chanp->xt_outbufs > NPCBUFS )
					chanp->xt_outbufs = NPCBUFS;

			freemsg(chanp->xt_msg[0].mp);
			ctlp->xt_outpkts -= 1;
			if ( chanp->xt_msg[1].mp != NULL ) {
				chanp->xt_msg[0].mp = chanp->xt_msg[1].mp;
				chanp->xt_msg[0].seq = chanp->xt_msg[1].seq;
				chanp->xt_msg[0].timestamp = chanp->xt_msg[1].timestamp;
				chanp->xt_msg[1].mp = NULL;
				chanp->xt_msg[1].seq = (chanp->xt_msg[1].seq + 1) & 07;
				chanp->xt_msg[1].timestamp = 0;
			} else {
				chanp->xt_msg[0].mp = NULL;
				chanp->xt_msg[0].seq = chanp->xt_msg[1].seq;
				chanp->xt_msg[0].timestamp = 0;
				chanp->xt_msg[1].seq = (chanp->xt_msg[1].seq + 1) & 07;
			}
		/* If the message that was ACKed corresponds to the message in
		   buffer 1, it indicates that the message in buffer 0 was 
	  	   never ACKed (remember that as soon as the message in buffer
 		   0 is ACKed, outstanding buffer 1 message is moved to buffer 
		   0). Both buffer 0 and buffer 1 messages are freed up.
		*/
		} else if ( (chanp->xt_msg[1].seq == GET_SEQ(header)) &&  (chanp->xt_msg[1].mp != NULL) ) {
			STATS(ctlp, S_LOSTACK);
			if ( (*(bp->b_rptr + 1) == C_UNBLK) &&  ((bp->b_wptr - bp->b_rptr) > 1) )
				if ( (chanp->xt_outbufs += 2) > NPCBUFS )
					chanp->xt_outbufs = NPCBUFS;
			if (chanp->xt_msg[0].mp) {
				freemsg(chanp->xt_msg[0].mp);
				ctlp->xt_outpkts -= 1;
			}
			if (chanp->xt_msg[1].mp) {
				freemsg(chanp->xt_msg[1].mp);
				ctlp->xt_outpkts -= 1;
			}
			chanp->xt_msg[0].mp = NULL;
			chanp->xt_msg[1].mp = NULL;
			chanp->xt_msg[0].seq = (chanp->xt_msg[1].seq + 1) & 07;
			chanp->xt_msg[1].seq = (chanp->xt_msg[0].seq + 1) & 07;
			chanp->xt_msg[0].timestamp = 0;
			chanp->xt_msg[1].timestamp = 0;
		} else {
			STATS(ctlp, S_BADACK);
		}
		freemsg(bp);
		qenable(WR(chanp->xt_upq));
		return;

	case NAK:
		STATS(ctlp, S_RNAK);
		/* If NAK was received for the message in buffer 0, retransmit
		   that message 
		*/
		if ( (chanp->xt_msg[0].seq == GET_SEQ(header)) &&  (chanp->xt_msg[0].mp != NULL) ) {
			bp1 = xtdupmsg("control_NAK", chanp->xt_msg[0].mp);
			STATS(ctlp, S_NAKRETRYS);
			chanp->xt_msg[0].timestamp = lbolt;
			if ( bp1 )
				putnext(ctlp->xt_ttyq, bp1);
			/* See note on dupmsg() in xtsend() */

		/* If NAK was received for the message in buffer 1, retransmit
		   message 0 and message 1. 
		*/
		} else if ( (chanp->xt_msg[1].seq == GET_SEQ(header)) &&  (chanp->xt_msg[1].mp != NULL) ) {
			if ( chanp->xt_msg[0].mp != NULL ) {
				bp1 = xtdupmsg("control_NAK2", chanp->xt_msg[0].mp);
				STATS(ctlp, S_NAKRETRYS);
				chanp->xt_msg[0].timestamp = lbolt;
				if ( bp1 )
					putnext(ctlp->xt_ttyq, bp1);
				/* See note on dupmsg() in xtsend() */
			}
			bp1 = xtdupmsg("control_NAK3", chanp->xt_msg[1].mp);
			STATS(ctlp, S_NAKRETRYS);
			chanp->xt_msg[1].timestamp = lbolt;
			if ( bp1 )
				putnext(ctlp->xt_ttyq, bp1);
			/* See note on dupmsg() in xtsend() */
		} else {
			STATS(ctlp, S_BADNAK);
		}
		freemsg(bp);
		return;

	case PCDATA:	/* This is an obsolete message type */
		freemsg(bp);
		return;

	default:
		STATS(ctlp, S_BADCNTL);
		freemsg(bp);
		return;
	}
}

/*
 * Deal with data packet.
 *
 * RETURN VALUES : 1 - Completed.
 *		 : 0 - Canput failure in C_SENDCHAR or C_SENDNCHARS.
 *		       Try again later.
 *
 *
 * The following explains the need for the XT_HOLDFLOW flag which is
 * used in case SENDNCHARS in recvpkt() below.
 *
 * A problem was encountered while paging through a unix word processor
 * program by holding down the arrow key. The terminal would send the
 * arrows to the host, the word processor would read the arrow keys and
 * sent output to the terminal to scroll the screen. This is like a big
 * pipe where the arrow keys sent from the terminal shoves bits into
 * the host, up to the word processor, back down through the host and
 * back to the terminal. The problem occurred when the pipe got backed
 * up because the arrow keys were sent too fast for the rest of the
 * pipe to clear itself out. A situation could occur where the host
 * would not be able to send any more keys to the terminal because it
 * is blocked by network xt flow control. However, there were too many
 * arrow keys in the queue from the terminal to the host, so the
 * underlying network (starlan) was flow controlled off, so the network
 * xt ack packet was not able to get through to unclog the host to
 * terminal direction. The word processor on the host was sitting in
 * a write() system call, so it never read off the arrow keys
 * to unclog things. And that was a deadlock situation so the xt
 * session would hang.
 *
 * The solution decided upon is to disable network xt flow control when
 * things get clogged up. This is not a pretty solution, but it had
 * reasonable implementation complexity and it should be effective. The
 * only bad effect is that network xt flow control gets disabled
 * temporarily. The effect of this is that other windows besides the
 * misbehaving window could have their input blocked. However, in the
 * situation where this is a problem, the terminal is so backed up and
 * slow in scrolling through the file anyway that I doubt if a user
 * will notice this. Besides, all they have to do is lift their finger
 * from the arrow key and everything will get back to normal again.
 *
 * The fix works as follows.
 *
 * When upstream canput fails, set the XT_HOLDFLOW flag to temporarily
 * disable flow control. Look and see if downstream is blocked by network
 * xt flow control. If so, enable the downstream service procedure (xtosrv)
 * so that, now that network xt flow control is disable, it can restart
 * sending to the terminal and unclog the pipe.
 *
 * As soon as upstream unclogs, remove the flag. This will lead to
 * downstream starting to restrict itself to HIWAT characters again and
 * things will go back to normal network xt with flow control mode.
 *
 * If the upstream gets clogged again, it just sets the XT_HOLDFLOW flag
 * again and we start over. This can lead to a constant back and forth
 * in the situation of holding down the arrow key, but that should do
 * no harm.
 *
 * Or let me put it another way, just hope that this fix works so
 * that nobody will have to actually try and understand this comment.
 *
 */
STATIC int
recvpkt(ctlp, bp, count, channo, command)
struct xtctl *ctlp;
mblk_t *bp;
int count, channo;
int command;
{
	queue_t * upq;
	struct xt_chan *chanp = &ctlp->xt_chan[channo];
	queue_t * tupq;
	mblk_t * bpn;
	extern unsigned char	bnextchar();
	int	i;
	struct copyreq *reqp;
	struct bagent *bagp;
	struct xtioctl *xtip;
	mblk_t *bptmp;

	if (!bp) {
		return(1);
	}

	switch (command) { /* rptr points to first data byte now */
	/*
	 * For C_SENDCHAR[S] and C_DELETE, we need to check to see that
	 * the queue is active.  A user is able to "exit" from the program
	 * in a window (like a shell), causing the channel to be closed.
	 * In that case, the DMD will still send up chars typed into the
	 * window until the user deletes it.  If we don't check for the
	 * queue, the system will crash.
	 */
	case C_SENDCHAR:
		/* obsolete: DMD 5620 should never send these */
		/* Turns out that ATT 630 sends them often... */
		count = 2;
		/* WARNING: MUST FALL THRU HERE! */
		/* COUNT WILL BE DECREMENTED BELOW, SO IT */
		/* WILL END UP BEING 1!  THIS FALL-THRU */
		/* SAVES SOME DUPLICATED CODE */

	case C_SENDNCHARS: 

		/* Send the remaining data upstream */

		debug(("recvpkt: case C_SENDNCHARS: "));
		upq = chanp->xt_upq;
		count--;

		if (((chanp->xt_chflg & XT_ON) == 0) || !chanp->xt_upq) {
			freemsg(bp);
			return(1);
		}
#ifdef VPIX
           	if (chanp->xt_v86p && validproc(chanp->xt_v86procp,chanp->xt_v86pid)) {
                        v86setint(chanp->xt_v86p, V86VI_KBD);
		}
#endif
		if (upq && canput(upq->q_next) != 0 ) {
			chanp->xt_chflg &= ~XT_HOLDFLOW;
			debug(("about to put next\n"));
			bp->b_wptr = bp->b_rptr + count;
			putnext(upq, bp);
			bp = NULL;
		}
		else {
			/* See the big comment at the beginning of this
			 * routine for an explaination of this code.
			 */
			chanp->xt_chflg |= XT_HOLDFLOW;
			if(chanp->xt_bytesent >= NETXT_HIWAT) {
				/* enable xtosrv() */
				qenable( WR(chanp->xt_upq) );
			}

			debug(("about to return 0\n"));
			return(0);
		}
		break;

	case C_UNBLK:
		/* unblock transmission to the layer */
		if ( (++chanp->xt_outbufs) > NPCBUFS )
			chanp->xt_outbufs = NPCBUFS;
		if (chanp->xt_upq && WR(chanp->xt_upq))
			qenable(WR(chanp->xt_upq));
		freemsg(bp);
		break;

	case C_EXIT:
		ctlp->xt_ctlflg |= XT_EXIT;
		/* Flush all the open layers and send hangups */
		/* leave out the control channel */
		for ( i = 1; i < MAXPCHAN; i++)  { /* DON'T HUP the control chan! */
			/*
 			* don't signal on a channel currently being closed!
 			*/
			if ((ctlp->xt_chan[i].xt_upq != NULL) &&  (!(ctlp->xt_chan[i].xt_chflg & XT_WCLOSE))) {
				putctl1(ctlp->xt_chan[i].xt_upq->q_next, M_FLUSH, FLUSHW);
				putctl(ctlp->xt_chan[i].xt_upq->q_next, M_HANGUP);
			}
		}

		/* Empty the input queue in case it is flow controlled
		 * off. We want things to unblock so the terminal can
		 * flush it's queues before the layers command tears
		 * apart xt. Since we are exiting, it is ok to just
		 * throw away the input. xtinput() will throw away
		 * any addition input which shows up after this.
		 */
		if( ctlp->xt_ttyq )
			while( (bptmp=getq( RD(ctlp->xt_ttyq) )) != NULL )
				freemsg(bptmp);

		/* fall thru... */
	
	case C_DELETE:
		/* flush the layers and send hangup to the process */
		/* close should be called after hangup and do the layer */
		/* clean up */
		if ( chanp->xt_upq && command == C_DELETE ) {
			putctl1(chanp->xt_upq->q_next, M_FLUSH, FLUSHW);
			putctl(chanp->xt_upq->q_next, M_HANGUP);
		}
		/*  fall thru.... */
	case C_NEW:
		/* instucts layers to invoke a new process in the window */
		/* get upqueue of control channel (0) */
		if (tupq = ctlp->xt_chan[0].xt_upq) {
			if ( (bpn = xtallocb("DELETE/EXIT/NEW", 4, BPRI_HI)) == NULL ) {
				STATS(ctlp, S_NOMBLK);
				return(1);
			}
			*bpn->b_wptr++ = command;
			*bpn->b_wptr++ = (unsigned char)channo;
			putnext(tupq, bpn);/* Send this upstream even if upq is full. */
		}
		if ( command != C_NEW ) {
			freemsg(bp);
			break;
		}

	case C_RESHAPE:
		/* reshape the layers */
		chanp->xt_jwinsize.bytesx = bnextchar(&bp);
		chanp->xt_jwinsize.bytesy = bnextchar(&bp);
		chanp->xt_jwinsize.bitsx = bnextchar(&bp);
		chanp->xt_jwinsize.bitsx |= bnextchar(&bp) << 8;
		chanp->xt_jwinsize.bitsy = bnextchar(&bp);
		chanp->xt_jwinsize.bitsy |= bnextchar(&bp) << 8;
		if ( bp ) {
			freemsg(bp);
		}
		/*
		 * Send SIGWINCH upstream
		 * otherwise, if vi is running in the window
		 * it can get confused
		 */
		if ( chanp->xt_upq && command == C_RESHAPE ) {
			putctl1(chanp->xt_upq->q_next, M_PCSIG, SIGWINCH);
		}
		break;

	case C_DEFUNCT:
 		/* layers is dead. Send SIGTERM to the process */
		chanp = &ctlp->xt_chan[0];
		upq = chanp->xt_upq;
		if (upq) {
			flushq(upq, 0);
			putctl1(upq->q_next, M_SIG, SIGTERM);
		}
		freemsg(bp);
		break;

	case JAGENT & 0xFF:
		/* response from terminal to a JAGENT ioctl */
		if (ctlp->xt_pendjagent == NULL) {
			debug(("recvpkt: Agent response with nothing pending\n"));
			return(1);
		}
		i = (int)*bp->b_rptr++; /* this is a count byte */
		bp->b_wptr = bp->b_rptr + i;

		/* only the data is left in the block now... */
		reqp = (struct copyreq *)ctlp->xt_pendjagent->b_rptr;
		xtip = (struct xtioctl *)reqp->cq_private->b_rptr;
		bagp = (struct bagent *)xtip->xti_bp->b_rptr;
		reqp->cq_size = i;
		reqp->cq_flag = 0;
		reqp->cq_addr = bagp->dest;
		ctlp->xt_pendjagent->b_datap->db_type = M_COPYOUT;

		/* JAGENT returns bytecount read */
		bagp->size = i;
		ctlp->xt_pendjagent->b_cont = bp;
		xtip->xti_seg = 3;
		putnext(ctlp->xt_chan[0].xt_upq, ctlp->xt_pendjagent);
		/* send this up the control channel */
		ctlp->xt_pendjagent = NULL;
		return(1);

	case C_NOFLOW: /* disable network xt flow control */
		debug(("%s", ctlp->xt_maxpkt != 1 ?
			"WARNING: recvpkt: C_NOFLOW in regular xt" : ""));
		chanp->xt_chflg |= XT_NONETFLOW;
		freemsg(bp);
		break;

	case C_YESFLOW: /* enable network xt flow control */
		debug(("%s", ctlp->xt_maxpkt != 1 ?
			"WARNING: recvpkt: C_YESFLOW in regular xt" : ""));
		chanp->xt_chflg &= ~XT_NONETFLOW;
		freemsg(bp);
		break;

	default:
		debug(("recvpkt: Bad C type from terminal\n"));
		STATS(ctlp, S_BADCTYPE);
		return(1);
	}

	return(1);
}

/*
 * send acknowledgement control packet to the DMD :
 * 
 * ACK or NAK
 *
 * RETURN VALUES : none
 */
STATIC void
xtack(ctlp, header, acknak)
struct xtctl *ctlp;
unsigned char	header;
int	acknak;
{
	mblk_t * bp;
	int	count;
	unsigned char	*c;

	/*
	 * allocate the largest buffer you could need here:
	 * 1 byte header + 2 bytes NAK + 2 bytes CRC = 5
	 * This will lead to max size of 7 for encoded pkts.
	 */
	if ( (bp = xtallocb("xtack", 8, BPRI_HI)) == NULL ) {
		/* If no buffer is available for ACK/NAK here,
		 * just forget it.  The protocol should recover
		 * if an ACK gets lost, and if a NAK is lost,
		 * another one will be sent soon.
		 */
		STATS(ctlp, S_NOMBLK);
		return;
	}

	SET_CNTL(header);
	*bp->b_wptr++ = header;
	if (acknak == (int) ACK) {
		count = 2;
		*bp->b_wptr++ = 0;
		STATS(ctlp, S_XACK);
	} else {
		count = 3;
		*bp->b_wptr++ = 1;
		*bp->b_wptr++ = NAK;
		STATS(ctlp, S_XNAK);
	}
	c = bp->b_rptr;
	(void) xt_crc(c, count);
	bp->b_wptr += EDSIZE;

	if( ISTRACEON(ctlp) )
		logpkt(*bp->b_rptr, (unsigned short)(*(bp->b_rptr+1)),
		       (char)0, bp->b_rptr+2, ctlp, XMITLOG);

	if (ctlp->xt_hex) {
		bp->b_wptr = bp->b_rptr;
		bp->b_wptr += mybcopy(bp->b_rptr, bp->b_rptr, count + EDSIZE);
	}

	if (bp)
		putnext(ctlp->xt_ttyq, bp);
}


/*
 * CRC16:  x**16 + x**15 + x**2 + 1 : using 64 byte look-up table
 */


static ushort	crc16t_32[2][16] = {
	0, 0140301, 0140601, 0500, 0141401, 01700, 01200, 0141101,
	0143001, 03300, 003600, 0143501, 02400, 0142701, 0142201, 02100,
	0, 0146001, 0154001, 012000, 0170001, 036000, 024000, 0162001,
	0120001, 066000, 074000, 0132001, 050000, 0116001, 0104001, 042000
};

/*
 * This function calclulates the crc value for the bytes supplied in the buffer
 * and appends the CRC bytes to the buffer
 *
 * RETURN VALUES : 1 or 2 : depending on how many CRC bytes (out of the 2 in 
  		  	    the input in the end, i.e., after nbytes) don't
			    match the calculated CRC.

		   No relevance for output CRC calculation  
*/
STATIC int	
xt_crc(buffer, nbytes)
unsigned char	*buffer;
int	nbytes;
{
	ushort	tcrc = 0;
	long	temp, i;

	if ( (i = nbytes) > 0 ) {
		do {
			temp = tcrc ^ *buffer++;
			tcrc = crc16t_32[0][temp & 017] ^ crc16t_32[1][(temp>>4) & 017] ^ (tcrc >> 8);
		}		 while ( --i > 0 );
	}

	/* The following checks are done only for input data under the      */
	/* assumption that the remaining two bytes in the buffer are the    */
	/* CRC bytes which must match the CRC calculated above.		    */

	if ( (tcrc & 0xff) != *buffer )
		i++;
	*buffer++ = (tcrc & 0xff);

	if ( (tcrc >> 8) != *buffer )
		i++;
	*buffer++ = (tcrc >> 8);

	return(i);
}


/*
 * write queue service procedure for the write queue just below
 * the XT multiplexor.  This routine exists to maintain flow control
 * across the multiplexor.  Enabling any one of the upper write
 * queues will start things rolling again.
 *
 * RETURN VALUES : none
 */
STATIC
xtwsrv(q)
queue_t *q;
{
	struct xtctl *ctlp;

	ctlp = (struct xtctl *)q->q_ptr;
	qenable(WR(ctlp->xt_chan[0].xt_upq));
	return;
}


/*
 * write queue service procedure for all write queues at the
 * top of the XT multiplexor
 *
 * causes all upper write queues to be emptied in round-robin fashion
 *
 * RETURN VALUES : none
 */
STATIC
xtosrv(q)
queue_t *q;
{
	struct xt_chan *chanp;
	int	i, activity, start;
	struct xtctl *ctlp;

	osrvdebug(("Entered xtosrv\n"));

	if ( q->q_ptr == NULL )	
		return;

	chanp = (struct xt_chan *)q->q_ptr;
	ctlp = chanp->xt_ctlp;

	/* Empty queues in round-robin fashion.
	 *
	 * If a queue blocked before because the downstream TTY queue
	 *  was full, start with that channel.
	 *
	 * Otherwise, start with whichever channel's queue was passed
	 *  in to the routine.
	 */
	if (ctlp->xt_next) {
		start = ctlp->xt_next - 1;
		ctlp->xt_next = 0;
	} else
		start = chanp->xt_channo;

loop:
	activity = 0;
	for ( i = start; i < MAXPCHAN; i++) {
		chanp = &ctlp->xt_chan[i];
		if ( chanp->xt_upq ) {
			activity += xtqsrv(chanp);
			/* if we're blocking, give up and try again later
			 * when downstream unclogs.
			 */
			if ( ctlp->xt_next )
				return;
		}
	}

	/* if anyone did anything useful, or if we didn't try everyone */
	if ( activity || start ) {
		start = 0;
		goto loop;
	}
}

/*
 * empties one of the write queues above the XT
 * called by the output service procedure
 * 
 * RETURN VALUES : 0 - waiting for an ACK/NAK : no processing done
 *		     - NULL up queue
 * 		     - canput failure
 *			
 *		    1 - unknown message type
 *		      - successful completion of message processing
 *		      - M_IOCDATA without an intervening ioctl command 
 */
STATIC int
xtqsrv(chanp)
struct xt_chan *chanp;
{
	mblk_t * bp, *bpt, *bpr;
	struct xtctl *ctlp = chanp->xt_ctlp;
	struct copyreq *reqp;
	queue_t * downq, *tq;
	struct linkblk *linkblkp;
	struct iocblk *iocbp;
	struct termio *cb;
	struct jwinsize *win;
	struct winsize *wing;
	short  n;
	long tdev;
	struct xtioctl *xtip;
	short  bytepersec;

	osrvdebug(("xtqsrv: chan=%d\n", chanp - ctlp->xt_chan));

	if ( chanp->xt_upq == NULL )
		return(0);

	tq = WR(chanp->xt_upq);

	if ( (bp = getq(tq)) == NULL )
		return(0); /* to outer loop */

	/* Only process high priority messages (i.e.queclass is set)
	 * while waiting for an M_COPYIN request if the M_COPYIN
	 * processing leads to sending a packet to the terminal.
	 * This is needed because we have to make sure we don't
	 * get blocked by xt flow control while waiting for
	 * the M_COPYIN. Getting blocked by xt flow control would be
	 * a problem because M_COPYIN is a high priority message and
	 * streams rules are that you can't do putbq() of high
	 * priority messges.
	 */
	if((chanp->xt_chflg&XT_WAIT4COPYIN) && !queclass(bp)) {
		putbq(tq, bp);
		return(0);
	}
		
	downq = ctlp->xt_ttyq;

	osrvdebug(("xtqsrv: type=%d\n", bp->b_datap->db_type));

	switch ( bp->b_datap->db_type ) {

	default: /* xt discards unknown messages */
		if (bp)
			freemsg(bp);
		return(1);

	case M_DATA:
		if( isblocked(chanp) || (chanp->xt_chflg&XT_M_STOPPED) ) {
			putbq(tq, bp);
			return(0);
		}
		return( xtsend(downq, chanp, bp) );

	case M_STOP:
		/* Message from LDTERM that user typed ^S */
		chanp->xt_chflg |= XT_M_STOPPED;
		freemsg(bp);
		return(1);

	case M_START:
		/* Message from LDTERM that user wants to restart output (^Q) */
		chanp->xt_chflg &= ~XT_M_STOPPED;
		freemsg(bp);
		return(1);

	case M_FLUSH: /* Loop these back up, if necessary.  Don't pass down. */
		if (*bp->b_rptr & FLUSHW) {
			flushq(WR(chanp->xt_upq), FLUSHDATA);
			*bp->b_rptr &= ~FLUSHW;
		}
		if (*bp->b_rptr & FLUSHR) {
			flushq(chanp->xt_upq, FLUSHDATA);
			putnext(chanp->xt_upq, bp);
		}
		else
			freemsg(bp);
		return(1);

	case M_IOCDATA:
		if (!(chanp->xt_chflg & XT_IOCTL)) {
			freemsg(bp);
			return(1);
		}
		xtioccont(bp, chanp);
		return (1);

	case M_IOCTL:
		iocbp = (struct iocblk *)bp->b_rptr;
		if ((((iocbp->ioc_cmd & JTYPE) == JTYPE) ||  ((iocbp->ioc_cmd & XTIOCTYPE) == XTIOCTYPE))  &&  iocbp->ioc_count != TRANSPARENT) {
			iocbp->ioc_error = EINVAL;
			iocbp->ioc_count = 0;
			iocbp->ioc_rval = 0;
			bp->b_datap->db_type = M_IOCNAK;
			if (bp->b_cont) {
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			}
			putnext(chanp->xt_upq, bp);
			return(1);
		}
		switch ( iocbp->ioc_cmd ) {
		case I_LINK:
			/* I_LINK is called with a pointer to the link block */
			/* in the data buffer. The link block contains a     */
			/* pointer to the lower write queue. This queue      */
			/* pointer is preserved in ctlp->ttyq. The private   */
			/* data structure pointer for the lower write queue  */
			/* is set to point to the control structure.         */
			
			/* Please note that the read queue private data      */
			/* structure is not set to point anywhere 	     */

			/* hook up tty q to ctl struct */
			linkblkp = (struct linkblk *) bp->b_cont->b_rptr;
			ctlp->xt_ttyq = linkblkp->l_qbot;
			ctlp->xt_ttyq->q_ptr = (char *)ctlp;
			downq = ctlp->xt_ttyq;
			/* fall thru to JTIMOM */

		case JTIMOM:
			if( isblocked(chanp) ) {
				putbq(tq, bp);
				return(0);
			}

			/* Find baud rate in bytes per sec */
			if( !(ctlp->xt_ttycflag&CBAUD) )
				cmn_err(CE_NOTE, "XT: JTIMOM: unknown baud rate");
			bytepersec = speeds[ctlp->xt_ttycflag&CBAUD];
			debug(("JTIMOM: bytepersec %d: ", bytepersec));

			/* Calculate receive timeout. Note that for receive
			** packets, maximum packet size is always 36. Slop
			** factor for received packets is 1 second.
			*/
			ctlp->xt_recvtimo = ((36 * (long)HZ) / bytepersec) + HZ;
			debug(("Receive timo %d: ", ctlp->xt_recvtimo)); 

			/* HZperpkt variable which will be multiplied by
			** number of outstanding packets in nxtscan()
			** to determine host to terminal send timeout.
			*/
			ctlp->xt_HZperpkt =
				((((long)ctlp->xt_maxpkt+PKTHEADSIZE+EDSIZE) * (long)HZ)
				/ bytepersec) + XTSCANSLOP;
			debug(("HZperpkt %d\n", ctlp->xt_HZperpkt));

			/* Receive timeout for the terminal to use. Same
			** formula as receive timeout host uses but must
			** account for potentially larger than 36 byte
			** packets and convert to milliseconds.
			*/
			n = ((((long)ctlp->xt_maxpkt+PKTHEADSIZE+EDSIZE) * (long)HZ) / bytepersec) + HZ;
			n = (n * (long)1000) / HZ;
			debug(("JTIMOM: Terminal receive timo %d: ", n));

			/* Send timeout for the terminal to use. Same formula
			** which the host uses except maximum packet size
			** terminal to host is always 36 and worst case for
			** number of outstanding packets (2 packets for
			** each of 8 channels) is used. It is necessary
			** to use the worse case assumption for compatibility
			** reasons, but with 36 byte maximum packets using
			** worst case still leads to reasonable timeouts, so
			** this really isn't a problem.
			*/
			tdev = ((36 * 8 * 2 * (long)HZ) / bytepersec) + XTSCANSLOP;
			if(tdev < (3*HZ)) tdev = (3*HZ);
			tdev = (tdev * (long)1000) / HZ; /* in milliseconds */
			debug(("send timo %d\n", tdev));

			if ( (bpt = xtallocb("JTIMOM", 13, BPRI_HI)) == NULL ) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
			} else {
				*bpt->b_wptr++ = JTIMOM;
				*bpt->b_wptr++ = n;
				*bpt->b_wptr++ = n >> 8;
				*bpt->b_wptr++ = tdev;
				*bpt->b_wptr++ = tdev >> 8;
				if ( xtsend(downq, chanp, bpt) ) {
					bp->b_datap->db_type = M_IOCACK;

				} else { /* buffer allocation failure */
					iocbp->ioc_error = EAGAIN;
					bp->b_datap->db_type = M_IOCNAK;
				}
			}
			iocbp->ioc_count = 0;
			break;

		case I_UNLINK:
			linkblkp = (struct linkblk *)bp->b_cont->b_rptr;
			ctlp->xt_ctlflg |= XT_UNLINK;
			ctlp->xt_ttyq->q_ptr =  0;
			ctlp->xt_ttyq = NULL;
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			iocbp->ioc_error = 0;
			break;

		case JTYPE:
			iocbp->ioc_cmd = JMPX;
		case XTIOCTYPE:
		case JMPX:
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			iocbp->ioc_rval = iocbp->ioc_cmd;
			break;

		case JWINSIZE:
			if ((bpr = xtallocb("JWINSIZE1", strlen("ccss") + 1, BPRI_MED)) == NULL) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}

			if ((bpt = xtallocb("JWINSIZE2", sizeof(struct jwinsize ), BPRI_MED)) == NULL) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
				freeb(bpr);
				break;
			}

			bp->b_datap->db_type = M_COPYOUT;
			bp->b_wptr = bp->b_rptr + sizeof(struct copyreq );
			reqp = (struct copyreq *)bp->b_rptr;

			/* cq_cmd overlays ioc_cmd */
			/* cq_id overlays ioc_id */

			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			reqp->cq_size = sizeof(struct jwinsize );
			reqp->cq_private = NULL;
			reqp->cq_flag = STRCANON;
			if (bp->b_cont)
				freeb(bp->b_cont);
			bpr->b_cont = bpt;
			bp->b_cont = bpr;
			strcpy(bpr->b_wptr, "ccss");
			bpr->b_wptr += strlen("ccss") + 1;
			win = (struct jwinsize *)bpt->b_rptr;
			win->bytesx = chanp->xt_jwinsize.bytesx;
			win->bytesy = chanp->xt_jwinsize.bytesy;
			win->bitsx = chanp->xt_jwinsize.bitsx;
			win->bitsy = chanp->xt_jwinsize.bitsy;
			bpt->b_wptr += sizeof(struct jwinsize );
			chanp->xt_chflg |= XT_IOCTL;
			break;

		case TIOCGWINSZ:
			if ((bpr = xtallocb("TIOCGWINSZ1", strlen("ssss") + 1, BPRI_MED)) == NULL) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}

			if ((bpt = xtallocb("TIOCGWINSZ2", sizeof(struct winsize ), BPRI_MED)) == NULL) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
				freeb(bpr);
				break;
			}

			bp->b_datap->db_type = M_COPYOUT;
			bp->b_wptr = bp->b_rptr + sizeof(struct copyreq );
			reqp = (struct copyreq *)bp->b_rptr;

			/* cq_cmd overlays ioc_cmd */
			/* cq_id overlays ioc_id */

			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			reqp->cq_size = sizeof(struct winsize );
			reqp->cq_private = NULL;
			reqp->cq_flag = STRCANON;
			freeb(bp->b_cont);
			bpr->b_cont = bpt;
			bp->b_cont = bpr;
			strcpy(bpr->b_wptr, "ssss");
			bpr->b_wptr += strlen("ssss") + 1;
			wing = (struct winsize *)bpt->b_rptr;
			/*
			*wing = chanp->xt_winsize;
			wing = (struct winsize *)bpt->b_rptr;
			win = (struct jwinsize *)bpt->b_rptr;
			win->bytesx = chanp->winsize.bytesx;
			win->bytesy = chanp->winsize.bytesy;
			win->bitsx = chanp->winsize.bitsx;
			win->bitsy = chanp->winsize.bitsy;
*/
			wing->ws_col = (char)chanp->xt_jwinsize.bytesx;
			wing->ws_row = (char)chanp->xt_jwinsize.bytesy;
			wing->ws_xpixel = chanp->xt_jwinsize.bitsx;
			wing->ws_ypixel = chanp->xt_jwinsize.bitsy;
			bpt->b_wptr += sizeof(struct winsize );
			chanp->xt_chflg |= XT_IOCTL;
			break;

		case JBOOT:
		case JZOMBOOT:
		case JTERM:
			if( isblocked(&ctlp->xt_chan[0]) ) {
				putbq(tq, bp);
				return(0);
			}
			if ( (bpt = xtallocb("JBOOT", 4, BPRI_MED)) == NULL ) {
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			*bpt->b_wptr++ = (unsigned char)iocbp->ioc_cmd;
			*bpt->b_wptr++ = (unsigned char)chanp->xt_channo;
			if ( xtsend(ctlp->xt_ttyq, &ctlp->xt_chan[0], bpt) ) {
				/* It's sent to the terminal. ACK it next. */
				iocbp->ioc_rval = 0;
				/* iocbp->ioc_rval = chanp->xt_channo; */
				bp->b_datap->db_type = M_IOCACK;
			} else { /* buffer allocation failure */
				iocbp->ioc_error = EAGAIN;
				bp->b_datap->db_type = M_IOCNAK;
			}
			iocbp->ioc_count = 0;
			break;

		case JXTPROTO:
			if( isblocked(&ctlp->xt_chan[0]) ) {
				putbq(tq, bp);
				return(0);
			}
			debug(("Got JXTPROTO request\n"));

			chanp->xt_chflg |= XT_IOCTL; 
			reqp = (struct copyreq *)bp->b_rptr;
			reqp->cq_private = NULL;
			reqp->cq_size = 1;
			reqp->cq_flag = 0;
			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));

			freeb(bp->b_cont);
			bp->b_datap->db_type = M_COPYIN;
			ctlp->xt_chan[0].xt_chflg |= XT_WAIT4COPYIN;
			break;

		case XTIOCHEX:
			ctlp->xt_hex = 1;
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			break;
#ifdef VPIX
           	case AIOCDOSMODE:
		{
                 	struct v86blk	*p_v86;
                        p_v86 = (struct v86blk *)bp->b_cont->b_rptr;
                        chanp->xt_v86p =  p_v86->v86_p_v86;
                        chanp->xt_v86pid =  p_v86->v86_p_pid;
                        chanp->xt_v86procp =  p_v86->v86_u_procp;
                        bp->b_datap->db_type = M_IOCACK;
                        break;
		}
                case AIOCNONDOSMODE:
                        chanp->xt_v86p =  (v86_t *)0;
                        chanp->xt_v86pid = (pid_t) 0;
                        chanp->xt_v86procp = (struct proc *) NULL;
                        bp->b_datap->db_type = M_IOCACK;
                        break;
		case AIOCINTTYPE:
                        if ((* (int *)bp->b_cont->b_cont->b_rptr) != V86VI_KBD)
{
                                iocbp->ioc_error = EINVAL;
                                bp->b_datap->db_type = M_IOCNAK;
                        } else{
                                iocbp->ioc_rval = 0;
                                bp->b_datap->db_type = M_IOCACK;
                        }
                        break;
#endif /* VPIX */

		case JAGENT:

			/* The format of the command is as follows : 	     */

			/* bp data block contains the copy request structure */
			/* bp cont data block contains the canonical "ill"   */
			/*    string 					     */
			/* copy request private pointer points to the xtioctl*/
			/*    structure					     */
			/* The source for copyin is extracted from the addr  */
			/*    of the argument as passed in bp cont data block*/
			/* The size of copyin is the size of structure bagent*/

			/* In response to the first copyin request, STREAMS  */
			/* will pass a bagent structure that indicates the   */
			/* source of the actual copy. This bagent structure  */
			/* will be saved in the xtioctl bp member.	     */

			/* The next copyin request will actually copy the    */
			/*   data, if any, required for the particular JAGENT*/
			/*   call.                                    	     */
			/* The size of copyin will be the size indicated by  */
			/*   bagent size member.			     */
			/* The source for copyin would be as indicated by    */
			/*   bagent src member.				     */

			/* xti_seg will indicate which of the two copyin     */
			/* requests is being processed.			     */

			if( isblocked(chanp) ) {
				putbq(tq, bp);
				return(0);
			}
			if ((bpt = xtallocb("JAGENT1", sizeof(struct xtioctl ))) == NULL) {
				iocbp->ioc_count = 0;
				iocbp->ioc_error = EAGAIN;
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			chanp->xt_chflg |= XT_IOCTL;
			reqp = (struct copyreq *)bp->b_rptr;
			bpt->b_wptr += sizeof(struct xtioctl );
			xtip = (struct xtioctl *)bpt->b_rptr;
			xtip->xti_seg = 1;
			xtip->xti_bp = NULL;
			reqp->cq_private = bpt;
			reqp->cq_size = sizeof(struct bagent );
			reqp->cq_flag = STRCANON;
			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			bp->b_cont->b_rptr = bp->b_cont->b_datap->db_base;
			strcpy(bp->b_cont->b_rptr, "ill");
			bp->b_cont->b_wptr = bp->b_cont->b_rptr + strlen("ill") + 1;
			bp->b_datap->db_type = M_COPYIN;
			chanp->xt_chflg |= XT_WAIT4COPYIN;
			break;

		case JTRUN:

			/* The format of the command is as follows : 	    */
			/* bp data block contains the copy request structure*/
			/* bp cont data block contains the canonical "c1"   */
			/* copy request private pointer points to xtioctl   */
			/*    structure					    */
			/* The source for the first copyin is extracted from*/
			/*    the bp cont data block 			    */
			/* The size of all copyins is 1 byte till a 0 byte  */
			/*    is received.				    */

			/* With each successive copyin response, the source */
			/*    address is incremented by 1 to point to next  */
			/*    source byte.				    */
			/* xtioctl seg member keeps a track of the next addr*/
			/*    which is a kluge to avoid an extra member     */
			/* xtioctl bp points to the data block for storing  */
			/*    data byte by byte as received after copy in   */
			/*    bp cont data block 			    */

		
 
			if ((bpr = xtallocb("JTRUN1", sizeof(struct xtioctl ))) == NULL) {
				iocbp->ioc_count = 0;
				iocbp->ioc_error = EAGAIN;
				bp->b_datap->db_type = M_IOCNAK;
				break;
			}
			if ((bpt = xtallocb("JTRUN2", TTYHOG + 1, BPRI_MED)) == NULL) {
				freeb(bpr);
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				iocbp->ioc_count = 0;
				break;
			}
			chanp->xt_chflg |= XT_IOCTL;
			/* copy user data in bp->b_cont->b_rptr)	     */

			reqp = (struct copyreq *)bp->b_rptr;

			/* The first byte in bp->b-cont->b_rptr indicates the*/
			/* address of the argument for this command which is */
			/* the string we want to copy			     */


			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			bp->b_cont->b_rptr = bp->b_cont->b_datap->db_base;

			xtip = (struct xtioctl *)bpr->b_rptr;
			bpr->b_wptr += sizeof(struct xtioctl );
			reqp->cq_private = bpr;

			/* xti_seg saves the source address for the next     */
			/* copyin request.				     */

			xtip->xti_seg = (int)reqp->cq_addr + 1;
			xtip->xti_bp = bpt;

			*bpt->b_wptr++ = C_RUN;
			reqp->cq_flag = STRCANON;
			reqp->cq_size = 1;
			strcpy(bp->b_cont->b_rptr, "c1");
			bp->b_cont->b_wptr = bp->b_cont->b_rptr + 
						strlen("c1") + 1;
			bp->b_datap->db_type = M_COPYIN;
			break;
		case XTIOCSTATS:  /* Copy out stored XT statistics. */
			n = sizeof(Stats_t) * S_NSTATS;
			if ((bpt = xtallocb("XTIOCSTATS", n, BPRI_MED)) == NULL) {
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				break;
			}
			reqp = (struct copyreq *)bp->b_rptr;

			/* cq_cmd overlays ioc_cmd */
			/* cq_id overlays ioc_id */

			/* save the copy out address */
			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			reqp->cq_size = n;
			reqp->cq_private = NULL;
			reqp->cq_flag = 0;
			chanp->xt_chflg |= XT_IOCTL;

			/* free it because it might not be enough */
			freeb(bp->b_cont);	

			bcopy((char *)ctlp->stats, (char *)bpt->b_wptr, n);
			bpt->b_wptr += n;
			bp->b_cont = bpt;

			bp->b_datap->db_type = M_COPYOUT;
			bp->b_wptr = bp->b_rptr + sizeof(struct copyreq );
			break;

		case XTIOCTRACE:  /* Copy out stored trace records. */
			n = sizeof(struct Tbuf );
			if ((bpt = xtallocb("XTIOCTRACE", n, BPRI_MED)) == NULL) {
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				break;
			}
			reqp = (struct copyreq *)bp->b_rptr;

			/* cq_cmd overlays ioc_cmd */
			/* cq_id overlays ioc_id */

			/* save the copy out address */
			reqp->cq_addr = (caddr_t)(*(long *)(bp->b_cont->b_rptr));
			reqp->cq_size = n;
			reqp->cq_private = NULL;
			reqp->cq_flag = 0;
			chanp->xt_chflg |= XT_IOCTL;

			/* free it because it might not be enough */
			freeb(bp->b_cont);	

			bcopy((char *) & ctlp->trace, (char *)bpt->b_wptr, n);
			bpt->b_wptr += n;

			ctlp->trace.flags = TRACEON;
			ctlp->trace.used = 0;

			bp->b_cont = bpt;
			bp->b_datap->db_type = M_COPYOUT;
			bp->b_wptr = bp->b_rptr + sizeof(struct copyreq );
			break;

		case XTIOCNOTRACE:  /* Turn tracing off. */
			ctlp->trace.flags = 0;
			bp->b_datap->db_type = M_IOCACK;
			break;

		case TCGETA:

			if (bp->b_cont)
				freemsg (bp->b_cont);

			if ( (bpt = xtallocb("TCGETA", sizeof(struct termio ), BPRI_MED)) == NULL ) {
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				break;
			}
			bp->b_cont = bpt;

			cb = (struct termio *)bpt->b_rptr;
			if(chanp->xt_channo == 0)
				cb->c_cflag = (ushort) ctlp->xt_ttycflag;
			else
				cb->c_cflag = 0;
			bp->b_cont->b_wptr += sizeof(struct termio );
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = sizeof(struct termio );
			break;

		case TCGETS: { /* termios GET ioctl */

			struct termios *cb;

			if (bp->b_cont)
				freemsg (bp->b_cont);

			if ( (bpt = xtallocb("TCGETS", sizeof(struct termios ), BPRI_MED)) == NULL ) {
				bp->b_datap->db_type = M_IOCNAK;
				iocbp->ioc_error = EAGAIN;
				STATS(ctlp, S_NOMBLK);
				break;
			}
			bp->b_cont = bpt;
			cb = (struct termios *)bpt->b_rptr;
			if(chanp->xt_channo == 0)
				cb->c_cflag = ctlp->xt_ttycflag;
			else
				cb->c_cflag = 0;
			bp->b_cont->b_wptr += sizeof(struct termios );
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = sizeof(struct termios );
			break;
		}

		case TCSETAF:
		case TCSETAW:
		case TCSETA:
			cb = (struct termio *)bp->b_cont->b_rptr;

			/* Set the cflags only for control channel */
			/* Ignore cflag changes on other channels */
			/* Let  ldterm send what it has stored */

			if(chanp->xt_channo == 0)
				ctlp->xt_ttycflag = 
				(ctlp->xt_ttycflag & 0xffff0000 | cb->c_cflag);

			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			break;
			/* This c_flag is a dummy.  */

		case TCSETSF:
		case TCSETSW:
		case TCSETS: {
			struct termios *cb;
			cb = (struct termios *)bp->b_cont->b_rptr;
			if(chanp->xt_channo == 0)
				ctlp->xt_ttycflag = cb->c_cflag;
			/* This c_flag is a dummy.  */
		}
		case TCSBRK:
		case TCXONC:
		case TCFLSH:
		case EUC_WSET:	/* just ACK these EUC ioctls... */
		case EUC_WGET:
		case EUC_MSAVE:
		case EUC_MREST:
		case EUC_IXLOFF:
		case EUC_IXLON:
		case EUC_OXLOFF:
		case EUC_OXLON:
			/* This won't allow the hardware settings    */
			/* (the cflags) to be changed in the driver. */
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			break;

#if 0
		case TIOCSETP:
			bp->b_datap->db_type = M_IOCACK;
			iocbp->ioc_count = 0;
			break;

		case TIOCGETP:
			if (bp->b_cont || (bp->b_cont = xtallocb("TIOCGETP", sizeof (struct sgttyb ), BPRI_HI)) == 0) {
				bp->b_datap->db_type = M_IOCNAK;
			} else {
				bp->b_cont->b_wptr = bp->b_cont->b_rptr + sizeof (struct sgttyb );
				bzero(bp->b_cont, sizeof (struct sgttyb ));
				bp->b_datap->db_type = M_IOCACK;
				iocbp->ioc_count = sizeof (struct sgttyb );
			}
			break;
#endif
		default:
			bp->b_datap->db_type = M_IOCNAK;
			break;
		}
	}
	putnext(RD(tq), bp);
	return(1);
}

/*
 * Continues the processing of a previously received ioctl. Typically, this 
 * will handle the copyin or copyout request generated by the ioctl.
 * 
*/
STATIC int
xtioccont(bp, chanp)		/* continue ioctl processing */
mblk_t *bp;
struct xt_chan *chanp;
{
	struct xtctl *ctlp = chanp->xt_ctlp;
	struct copyreq *reqp;
	struct copyresp *resp;
	struct iocblk *iocbp;
	struct bagent *bagp;
	mblk_t * mp, *tmp;
	struct xtioctl *xtip;
	extern unsigned char	bnextchar();

	resp = (struct copyresp *)bp->b_rptr;
	if (resp->cp_rval) {		/* failure */
		chanp->xt_chflg &= ~XT_IOCTL;
		if (resp->cp_private) {
			xtip = (struct xtioctl *)resp->cp_private->b_rptr;
			if (xtip->xti_bp)
				freeb(xtip->xti_bp);
			freeb(resp->cp_private);
		}
		freemsg(bp);
		return(1);
	}
	switch (resp->cp_cmd) {

	case XTIOCTRACE:
	case XTIOCSTATS:
	case TIOCGWINSZ:
	case JWINSIZE:
		iocbp = (struct iocblk *)bp->b_rptr;
		iocbp->ioc_count = 0;
		iocbp->ioc_error = 0;
		iocbp->ioc_rval = 0;
		bp->b_datap->db_type = M_IOCACK;
		chanp->xt_chflg &= ~XT_IOCTL;
		break;


	case JXTPROTO:
		{
		unsigned char maxpkt;

		/* The argument for the command is returned in b_cont. */
		maxpkt = *bp->b_cont->b_rptr;
		debug(("JXTPROTO maxpkt = %d\n", (unsigned int)maxpkt));

		freeb(bp->b_cont);
		bp->b_cont = NULL;
		bp->b_datap->db_type = M_IOCACK;

		iocbp = (struct iocblk *)bp->b_rptr;
		iocbp->ioc_count = 0;
		iocbp->ioc_rval = 0;
		iocbp->ioc_error = 0;
		chanp->xt_chflg &= ~XT_IOCTL;
		ctlp->xt_chan[0].xt_chflg &= ~XT_WAIT4COPYIN;

		/* force sane values */
		if(maxpkt > MAXOUTDSIZE) maxpkt = MAXOUTDSIZE;
		/* note: if(maxpkt == 1) means network xt */
		if(maxpkt < 32 && maxpkt != 1) maxpkt = 32;

		if ( (mp = xtallocb("JXTPROTO", 4, BPRI_HI)) == NULL ) {
			iocbp->ioc_error = EAGAIN;
			STATS(ctlp, S_NOMBLK);
			bp->b_datap->db_type = M_IOCNAK;
			break;
		}
		*mp->b_wptr++ = (unsigned char)JXTPROTO;
		*mp->b_wptr++ = (unsigned char)maxpkt;
		if ( xtsend(ctlp->xt_ttyq, &ctlp->xt_chan[0], mp) ) {
			ctlp->xt_maxpkt = maxpkt;
			/* no need for network xt flow control on channel 0 */
			if(maxpkt == 1)
				ctlp->xt_chan[0].xt_chflg |= XT_NONETFLOW;
		} else { /* buffer allocation failure */
			iocbp->ioc_error = EAGAIN;
			bp->b_datap->db_type = M_IOCNAK;
		}
		break;

		}

	case JTRUN:
		xtip = (struct xtioctl *)resp->cp_private->b_rptr;
		if (xtip->xti_bp->b_wptr == xtip->xti_bp->b_datap->db_lim) {
			freeb(xtip->xti_bp);
			freeb(resp->cp_private);
			bp->b_datap->db_type = M_IOCNAK;
			iocbp = (struct iocblk *)bp->b_rptr;
			iocbp->ioc_count = 0;
			iocbp->ioc_rval = 0;
			iocbp->ioc_error = EINVAL;
			chanp->xt_chflg &= ~XT_IOCTL;
			break;
		}
		*(xtip->xti_bp->b_wptr)++ = *bp->b_cont->b_rptr;
		if (*bp->b_cont->b_rptr) {	/* still need more */
			bp->b_datap->db_type = M_COPYIN;
			reqp = (struct copyreq *)bp->b_rptr;
			reqp->cq_addr = (caddr_t)xtip->xti_seg++;
			reqp->cq_size = 1;
			reqp->cq_flag = STRCANON;
			bp->b_cont->b_rptr = bp->b_cont->b_datap->db_base;
			strcpy(bp->b_cont->b_rptr, "c1");
			bp->b_cont->b_wptr = bp->b_cont->b_rptr + 
						strlen("c1") + 1;
		} else {
			/* Send the command upto the control channel preceded*/ 
			/* by the C_RUN byte and ACK the channel that        */
			/* generated the JTRUN request. The command is execed*/
			/* by the layers process.		     	     */ 

			mblk_t	*bp1;
			
			iocbp = (struct iocblk *)bp->b_rptr;

			if ((bp1 = xtdupmsg("JTRUN", xtip->xti_bp)) == (mblk_t *) NULL) {
 				bp->b_datap->db_type = M_IOCNAK;
 				iocbp->ioc_error = EAGAIN;
 			} else {
				putnext(ctlp->xt_chan[0].xt_upq, bp1);
				bp->b_datap->db_type = M_IOCACK;
				iocbp->ioc_error = 0;
			}

			freeb(xtip->xti_bp);
			freeb(resp->cp_private);
			freeb(bp->b_cont);
			bp->b_cont = NULL;

			iocbp->ioc_count = 0;
			iocbp->ioc_rval = 0;
			chanp->xt_chflg &= ~XT_IOCTL;
		}
		break;

	case JAGENT:
		xtip = (struct xtioctl *)resp->cp_private->b_rptr;
		if (xtip->xti_seg == 1) {	/* save struct bagent */
			xtip->xti_bp = bp->b_cont;
			bagp = (struct bagent *)xtip->xti_bp->b_rptr;
			if (bagp->size < 0 || bagp->size > MAXPKTDSIZE) {
				bp->b_cont = NULL;
				bp->b_datap->db_type = M_IOCNAK;
				iocbp = (struct iocblk *)bp->b_rptr;
				iocbp->ioc_count = 0;
				iocbp->ioc_rval = 0;
				iocbp->ioc_error = EINVAL;
				freeb(xtip->xti_bp);
				freeb(resp->cp_private);
				chanp->xt_chflg &= ~XT_WAIT4COPYIN;
				chanp->xt_chflg &= ~XT_IOCTL;
				break;
			}
			bp->b_cont = NULL;
			bp->b_datap->db_type = M_COPYIN;
			reqp = (struct copyreq *)bp->b_rptr;
			reqp->cq_addr = (caddr_t)bagp->src;
			reqp->cq_size = bagp->size;
			reqp->cq_flag = 0;
			xtip->xti_seg = 2;
			break;
		} else if (xtip->xti_seg == 2) { /* get data */
			chanp->xt_chflg &= ~XT_WAIT4COPYIN;
			bagp = (struct bagent *)xtip->xti_bp->b_rptr;
			if ((mp = xtallocb("JAGENT2", bagp->size + 2, BPRI_MED)) == NULL) {
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
				bp->b_datap->db_type = M_IOCNAK;
				iocbp = (struct iocblk *)bp->b_rptr;
				iocbp->ioc_count = 0;
				iocbp->ioc_rval = 0;
				iocbp->ioc_error = EAGAIN;
				freeb(xtip->xti_bp);
				freeb(resp->cp_private);
				chanp->xt_chflg &= ~XT_IOCTL;
				break;
			}
			tmp = bp->b_cont;
			bp->b_cont = NULL;
			*mp->b_wptr++ = JAGENT;
			*mp->b_wptr++ = bagp->size;
			while ((tmp) && (mp->b_wptr <= mp->b_datap->db_lim))
				*mp->b_wptr++ = bnextchar(&tmp);
			if (xtsend(ctlp->xt_ttyq, chanp, mp)) {
				ctlp->xt_pendjagent = bp;
				return(1);
				/* recvpkt() will handle the IOCACK for this one */
			} else {		/* buffer allocation failure */
				if (tmp)
					freemsg(tmp);
				bp->b_datap->db_type = M_IOCNAK;
				iocbp = (struct iocblk *)bp->b_rptr;
				iocbp->ioc_count = 0;
				iocbp->ioc_rval = 0;
				iocbp->ioc_error = EAGAIN;
				freeb(xtip->xti_bp);
				freeb(resp->cp_private);
				chanp->xt_chflg &= ~XT_IOCTL;
			}
		} else {			/* ack copyout */
			bp->b_datap->db_type = M_IOCACK;
			if (bp->b_cont) {
				freemsg(bp->b_cont);
				bp->b_cont = NULL;
			}
			iocbp = (struct iocblk *)bp->b_rptr;
			iocbp->ioc_count = 0;
			iocbp->ioc_error = 0;
			bagp = (struct bagent *)xtip->xti_bp->b_rptr;
			iocbp->ioc_rval = bagp->size;
			freeb(xtip->xti_bp);
			freeb(resp->cp_private);
			chanp->xt_chflg &= ~XT_IOCTL;
		}
		break;
	default:
		freemsg(bp);
		return(1);
	}
	putnext(chanp->xt_upq, bp);
	return(1);
}

/* isblocked()
 *
 * Checks if blocked by either xt protocol flow control or streams flow
 * control. First checks if the given channel is blocked by network xt or
 * regular xt flow control, whichever is appropriate. If not blocked, then
 * streams flow control is tested with canput(). Note that streams flow
 * control applies to the whole channel group where xt flow control is per
 * channel.
 *
 * This function must be called from xtqsrv() or xtioccont() before anything
 * is sent downstream.
 */
STATIC int
isblocked(chanp)
struct xt_chan *chanp;
{
	struct xtctl *ctlp = chanp->xt_ctlp;

	/*
	 * If network xt protocol, check if blocked waiting for
	 * a network xt ACK packet.
	 */
	if(ctlp->xt_maxpkt == 1) {
		if(chanp->xt_bytesent >= NETXT_HIWAT && !(chanp->xt_chflg&XT_HOLDFLOW) ) {
			isblockdebug(("BLOCKED chanp %d bytesent %d\n",
		    	    chanp - ctlp->xt_chan, (unsigned)chanp->xt_bytesent));
			return(1);	/* Blocked by flow control, Joel. */
		}
	}

	/*
	 * Else if regular xt, check if waiting for an ACK/NAK. This will
	 * be the case if there are already two outstanding packets.
	 */
	else if( (chanp->xt_outbufs < 1 ) ||
	       ((chanp->xt_msg[0].mp != NULL) &&  (chanp->xt_msg[1].mp != NULL)) ) {

		isblockdebug(("RBLOCKED chanp %d outbufs %d mp0 %x mp1 %x\n",
		    chanp - ctlp->xt_chan, chanp->xt_outbufs,
		    chanp->xt_msg[0].mp, chanp->xt_msg[1].mp ));

		return(1);	/* Waiting for an ACK, Jack. */
	}

	/* Check if blocked by streams flow control.
	 *
	 * Note that if ctlp->xt_ttyq is not set, we return saying
	 * we are not blocked. This will lead to xtsend() being called
	 * and xtsend() will end up discarding the packet. If we
	 * returned 1 on this condition, we would end up doing a
	 * putbq rather than discarding the message, and that would
	 * be the wrong thing to do.
	 */
	if( ctlp->xt_ttyq )
		if( canput(ctlp->xt_ttyq->q_next) == 0 ) {
			ctlp->xt_next = chanp->xt_channo + 1;
			return(1);
		}

	return(0); /* not blocked */
}


/*
 * manufactures a packet for output to the DMD.
 *
 * writes into the message block whose ptr it is passed:
 * a header byte that it computes
 * a data count byte that is passed in
 * "count" bytes of data
 * two CRC bytes at the end of the message
 *
 * RETURN VALUES : none
 */
STATIC void
makepkt(chanp, from_bp, to_bp, m)
struct xt_chan *chanp;
mblk_t **from_bp;
mblk_t *to_bp;
unsigned int	m;
{
	unsigned int	i;
	struct xtctl *ctlp = chanp->xt_ctlp;

	/* header byte */
	if ( chanp->xt_msg[0].mp == NULL )
		*to_bp->b_wptr++ = 0200 | (chanp->xt_channo << 3) | (chanp->xt_msg[0].seq);
	else
		*to_bp->b_wptr++ = 0200 | (chanp->xt_channo << 3) | (chanp->xt_msg[1].seq);

	/* count byte */
	*to_bp->b_wptr++ = m;

	/* data bytes */
	if( bnextnchars(from_bp, to_bp->b_wptr, m) != m )
		cmn_err(CE_NOTE, "XT: makepkt: out of sync");
	to_bp->b_wptr += m;

	/* CRC bytes */
	xt_crc(to_bp->b_rptr, (int)(m + 2));	/* 2 = header + dcount bytes */

	/* Moved this here from xtsend() to log packet before it is encoded */
	if( ISTRACEON(chanp->xt_ctlp) )
		logpkt(*to_bp->b_rptr, (unsigned short)m, (char)0,
			to_bp->b_rptr+2, chanp->xt_ctlp, XMITLOG);

	/* don't chk retval w/outpkts */
	if (ctlp->xt_hex) {
		to_bp->b_wptr = to_bp->b_rptr;
		to_bp->b_wptr += mybcopy(to_bp->b_rptr, to_bp->b_rptr, (int) (m + 2 + EDSIZE));
	} else {
		to_bp->b_wptr += EDSIZE;
	}
}

/*
 * makenetpkt: Network xt version of makepkt().
 *  RETURN VALUES : none
 */
STATIC void
makenetpkt(chanp, from_bp, to_bp, m)
struct xt_chan *chanp;
mblk_t **from_bp;
mblk_t *to_bp;
unsigned int m;
{
	/* Header bytes.
	*/
	*to_bp->b_wptr = 0x40 | (chanp->xt_channo & 0x7);

	if( !(chanp->xt_chflg&XT_NONETFLOW) && !(chanp->xt_chflg&XT_HOLDFLOW) ) {
		if(chanp->xt_bytesent == 0)
			*to_bp->b_wptr |= 0x20;
		*to_bp->b_wptr |= 0x10;
	}
	to_bp->b_wptr++;

	*to_bp->b_wptr++ = ((m>>8) & 0xFF);
	*to_bp->b_wptr++ = m & 0xFF;

	/* Data bytes - note that from_bp == (mblk_t **)0 means don't copy
	** any data bytes. Used by fast track in xtsend().
	*/
	if(from_bp != (mblk_t **)0) {
		if( bnextnchars(from_bp, to_bp->b_wptr, m) != m )
			cmn_err(CE_NOTE, "XT: makenetpkt: out of sync");
		to_bp->b_wptr += m;
	}
}

/* the following routine copies one buffer to another, while encoding    */
/* from an 8-bit path to 6 bits.  As a result, the length of the new     */
/* buffer (which is returned with the function) is roughly 4/3 times     */
/* the original buffer size(n).                                  */
/* This is probably ok, as MAX PKT SIZE is 36 bytes (2 + 32 + 2) */
/* giving us a new maximium buffer size of 48, which is less     */
/* the allowable 64 bytes (from the tty structure)               */

/* refer to xtinput for a description of the hex encoding scheme */
/* RETURN VALUES : no. of bytes copied 				 */

STATIC int	
mybcopy (from, to, n)                   /* for hex paths */
unsigned char	*from, *to;
int	n;
{
	unsigned char	c1, c2, c3, *fp, *tp;
	short	i;
	unsigned char	tmpbuf[PKTHEADSIZE + MAXOUTDSIZE + EDSIZE];
	unsigned char	*tmpptr;
	tmpptr = tmpbuf;
	for (fp = from, i = 0; i < n; i++)
		*tmpptr++ = *fp++;
	for (fp = tmpbuf, tp = to, i = 0 ; i < n ; i += 3) {
		c1 = *fp++;
		if (i + 1 < n)
			c2 = *fp++;
		if (i + 2 < n)
			c3 = *fp++;

		*tp++ = 0x40 | (c1 & 0xc0) >> 2 | (c2 & 0xc0) >> 4 | (c3 & 0xc0) >> 6;
		*tp++ = 0x40 | (c1 & 0x3f);
		if (i + 1 < n)
			*tp++ = 0x40 | (c2 & 0x3f);
		if (i + 2 < n)
			*tp++ = 0x40 | (c3 & 0x3f);
	}
	*to &= 0x3f;                  /* beginning of packet indicator */
	return(tp - to);
}

/*
 * Break the buffer pointed to by bp into xt packets and send them
 * downstream.
 *
 * There are three paths through this routine as documented
 * in the code.
 * 
 * RETURN VALUES : 0 - NULL bp
 *		     - allocation failure
 *		   1 - NULL down queue  
 *		     - successful processing
 */
STATIC int
xtsend(downq, chanp, bp)
queue_t *downq;
struct xt_chan *chanp;
mblk_t *bp;
{
	unsigned int m;
	mblk_t *bp1, *bp2;
	int k;
	struct xtctl *ctlp = chanp->xt_ctlp;
	int size;
	int i;

	if (!bp)
		return(0);

	if(!downq) {
		freemsg(bp);
		return(1);
	}

	/* Find how many bytes are available to send downstream.
	*/
	size = msgdsize(bp);

	/* For some reason, emacs sends down type M_DATA message blocks
	 * with msgdsize of 0. Since size is 0 there is nothing to send,
	 * so just free the mesage and return.
	 */
	if(size == 0) {
		freemsg(bp);
		return(1);
	}

	/*
	** PATH 1: This is network xt and we are able to send the
	** entire mblk_t as one packet because either flow control is
	** not enabled for this channel or flow control is enabled
	** but the whole mblk can be sent because it fits within
	** high water limits.
	**
	** This "fast track" simply linkb's a small header message to the
	** head of bp, sends the whole shot downstream, and returns. This
	** implies no allocb of extra (large) streams buffers and no
	** copying of every data character.
	*/
	if( ctlp->xt_maxpkt == 1 &&
	   ( (chanp->xt_chflg&XT_NONETFLOW)
	      || (int)(size + chanp->xt_bytesent) <= NETXT_HIWAT
	      || (chanp->xt_chflg&XT_HOLDFLOW) ) ) {
		packetdebug(("SEND PATH 1: size %d\n", size));
		if ( (bp1 = xtallocb("xtsend1", NETHEADSIZE, BPRI_MED)) == NULL ) {
			STATS(chanp->xt_ctlp, S_NOMBLK);
			(void)bufcall(NETHEADSIZE, BPRI_MED, qenable, WR(chanp->xt_upq));
			putbq(WR(chanp->xt_upq), bp);
			return(0);
		}
		makenetpkt(chanp, (mblk_t **)0, bp1, size);
#ifdef DEBUG
		tracedebug(("TRACE: %x %x %x: ", (int)(*(bp1->b_wptr-3)),
			(int)(*(bp1->b_wptr-2)), (int)(*(bp1->b_wptr-1)) ));
		for (bp2 = bp ; bp2 ; bp2 = bp2->b_cont) {
			unsigned char *p;
			if (bp2->b_datap->db_type != M_DATA)
				cmn_err(CE_CONT, "xtsend: Panic Non M_DATA type\n");
			for(p = bp2->b_rptr ; p < bp2->b_wptr ; ++p) {
				char tmp[2];
				tmp[0] = *p>0x20 && *p<0x80 ? *p : '^';
				tmp[1] = '\0';
				tracedebug(("%x(%s)", (unsigned)*p, tmp));
			}
		}
		tracedebug(("\n"));
#endif
		linkb(bp1, bp);
		if( !(chanp->xt_chflg&XT_NONETFLOW) && !(chanp->xt_chflg&XT_HOLDFLOW) ) {
			chanp->xt_bytesent += size;
			netflowdebug(("xtsend: xt_bytesent chan %d now %d\n",
		    	    chanp - ctlp->xt_chan, (unsigned)chanp->xt_bytesent));
		}

		if( ISTRACEON(chanp->xt_ctlp) ) {
			unsigned char buf[PKTPTSZ-3];
			unsigned char *p;

			for (i=0, bp2 = bp ; bp2 ; bp2 = bp2->b_cont) {
				for(p = bp2->b_rptr ; p < bp2->b_wptr ; ++p) {
					buf[i++] = *p;
					if(i == PKTPTSZ-3) goto breakbreak;
				}
			}
breakbreak:
			logpkt(*bp1->b_rptr, (unsigned short)size, (char)0,
				buf, chanp->xt_ctlp, XMITLOG);
		}
		STATS(chanp->xt_ctlp, S_XPKTS);

		putnext(downq, bp1);
		return(1);
	}

	/*
	** PATH 2: Network xt with flow control and we could not send
	** the entire mblk_t as one packet. This means we have to allocb
	** a new buffer and copy data bytes to the new buffer, similar to
	** regular xt.
	*/ 
	else if(ctlp->xt_maxpkt == 1) {
		/*
		 * We could not send the entire mblk as one packet, so send
		 * what we can.
		 */
		m = NETXT_HIWAT - chanp->xt_bytesent;
		if(m == 0) {
			cmn_err(CE_NOTE, "xtsend: path 2 out of sync");
			return(0);
		}

		packetdebug(("SEND PATH 2: %d of %d\n", m, size));
		k = m + NETHEADSIZE;
		if ( (bp1 = xtallocb("xtsend2", k, BPRI_MED)) == NULL ) {
			STATS(chanp->xt_ctlp, S_NOMBLK);
			(void)bufcall(k, BPRI_MED, qenable, WR(chanp->xt_upq));
			putbq(WR(chanp->xt_upq), bp);
			return(0);
		}
		makenetpkt(chanp, &bp, bp1, m);
#ifdef DEBUG
		{
		unsigned char *p;
		unsigned char s[2];
		tracedebug(("TRACE: "));
		for(p = bp1->b_rptr ; p < bp1->b_wptr ; ++p) {
			s[0] = *p>0x20 && *p<0x80 ? *p : '^';
			s[1] = '\0';
			tracedebug(("%x(%s)", (unsigned)(*p), s));
		}
		tracedebug(("\n"));
		}
#endif
		chanp->xt_bytesent += m;
		netflowdebug(("xtsend: xt_bytesent chan %d now %d\n",
		    chanp - ctlp->xt_chan, (unsigned)chanp->xt_bytesent));

		if( ISTRACEON(chanp->xt_ctlp) )
			logpkt(*bp1->b_rptr, (unsigned short)m, (char)0,
				bp1->b_rptr+3, chanp->xt_ctlp, XMITLOG);
		STATS(chanp->xt_ctlp, S_XPKTS);

		putnext(downq, bp1);

		/* Putbq bp if the whole message has not been sent.
		*/
		if(bp) putbq(WR(chanp->xt_upq), bp);
		return(1);
	}

	/*
	** PATH 3: Regular (not network) xt. Allocb a new buffer and
	** copy data bytes to the new buffer. Also, this is more
	** complicated because we have have to save packets for
	** potential retransmittion.
	*/
	else {
		m = min(size, ctlp->xt_maxpkt);
		packetdebug(("SEND PATH 3: %d of %d\n", m, size));

		if (ctlp->xt_hex)
			k = 4 * (m + PKTHEADSIZE + EDSIZE) / 3 + 1;
		else
			k = m + PKTHEADSIZE + EDSIZE;

		if ( (bp1 = xtallocb("xtsend3", k, BPRI_MED)) == NULL ) { /*4=pkt header+2 CRC bytes*/
			STATS(chanp->xt_ctlp, S_NOMBLK);
			(void)bufcall(k, BPRI_MED, qenable, WR(chanp->xt_upq));
			putbq(WR(chanp->xt_upq), bp);
			return(0);
		}

		makepkt(chanp, &bp, bp1, m);
#ifdef DEBUG
		{
		unsigned char *p;
		unsigned char s[2];
		tracedebug(("TRACE: "));
		for(p = bp1->b_rptr ; p < bp1->b_wptr ; ++p) {
			s[0] = *p>0x20 && *p<0x80 ? *p : '^';
			s[1] = '\0';
			tracedebug(("%x(%s)", (unsigned)(*p), s));
		}
		tracedebug(("\n"));
		}
#endif
		if ( chanp->xt_msg[0].mp == NULL ) {
			chanp->xt_msg[0].mp = bp1;
			chanp->xt_msg[0].timestamp = lbolt;
			ctlp->xt_outpkts += 1;
			chanp->xt_msg[0].xt_saveoutpkts = ctlp->xt_outpkts;
		} else if ( chanp->xt_msg[1].mp == NULL ) {
			chanp->xt_msg[1].mp = bp1;
			chanp->xt_msg[1].timestamp = lbolt;
			ctlp->xt_outpkts += 1;
			chanp->xt_msg[1].xt_saveoutpkts = ctlp->xt_outpkts;
		} else
			cmn_err(CE_NOTE, "XT: xtsend out of state badly!");

		chanp->xt_outbufs--;
		bp2 = xtdupmsg("xtsend3", bp1);	/* get a copy to send */

/***************************************************************************/
/*  A Note on dupb()/dupmsg() Failures.					   */
/*  When dupb() (called by dupmsg()) fails, it returns NULL.  There is no  */
/*  way to guarantee that dupb() will not fail, and the only recovery mech-*/
/*  anism available is bufcall(), which also makes no guarantees.  When    */
/*  dupb() fails in the XT driver, the message is time-stamped, but not    */
/*  sent.  Later, the nxtscan() routine will notice that an ACK or NAK has */
/*  not been received, and will attempt to re-send the message.  This is   */
/*  all that would be done with bufcall() anyway, so it is not used.       */
/***************************************************************************/

		if ( bp2 ) {
			putnext(downq, bp2);
			STATS(chanp->xt_ctlp, S_XPKTS);
		}

		/* Putbq bp if the whole message has not been sent.
		*/
		if(bp) putbq(WR(chanp->xt_upq), bp);
		return(1);
	}
}


/* Routine called to check for protocol timeouts.
 *
 * First part checks that an incoming packet is totally received
 * within given time limits, and if not, enables a service procedure
 * to discard the packet.
 *
 * Second part goes through all channels awaiting ack from terminal,
 * and if the ack hasn't been received in the requisite time, enables
 * a service procedure to retransmit the packet.
 *
 * The timer interrupt occurs at a higher interrupt level but it is scheduled
 * at a software interrupt priority. Finally, the timeout routine is called by 
 * the timein routine at a high priority level. 
 *
 * The user system calls cause a lower level interrupt that cannot preempt
 * the timeout routine. But this routine can be preempted by another timeout
 * call. It can also be preempted by hardware interrupts when it is run at 
 * splstr. 
 *
 * Note that this is not strictly a timeout routine. This is a routine
 * that is called regularly after every XTSCANRATE interval. All the channels
 * are checked against the xmission/reception time to find out if a timeout
 * has actually occured. 
 * 
 * RETURN VALUES : none
 */
STATIC void
nxtscan()
{
	struct xtctl *ctlp;
	struct xt_chan *chanp;
	short i, j, k;
	time_t time_now;
	long outtime;

	xt_scanon = 0;
	time_now = lbolt;

	scandebug(("Entered nxtscan\n"));
	for ( i = 0 ; i < nxt_count ; i++ ) {
		ctlp = &nxtctl[i];

		/* don't scan if not in use or network xt or exiting or unlinked */
		if (((ctlp->xt_ctlflg & XT_INUSE) == 0) || (ctlp->xt_maxpkt == 1))
			continue;
		if (ctlp->xt_ctlflg & (XT_EXIT|XT_UNLINK))
			continue;

		scandebug(("nxtscan: found an open channel\n"));
		xt_scanon++; /* This means someone's open. */

		/* First, check for input packet timeout. If there is a timeout
		 * in need of processing, qenable xtisrv() to handle it.
		 */
		if ( ctlp->xt_intime && ctlp->xt_intime + ctlp->xt_recvtimo <= time_now ) {
			debug(("nxtscan: scaned input\n"));
			if(ctlp->xt_ttyq) /* qenable xtisrv() */
				qenable( RD(ctlp->xt_ttyq) );
		}

		/* Next, if there are outstanding packets, run through all
		 *    channels looking for output ACK timeouts.
		 * If one is found, set XT_WANTTIMEOUT flag and qenable xtisrv().
		 */
#ifdef DEBUG
		if(ctlp->xt_outpkts < 0 || ctlp->xt_outpkts > 16)
			debug(("nxtscan: OUTPKTS IS OUT OF SYNC\n"));
#endif
		if(ctlp->xt_outpkts > 0) { /* if there are any outstanding packets */
		    for ( j = 0; j < MAXPCHAN ; j++ ) {
			chanp = &ctlp->xt_chan[j];
			if ( !(chanp->xt_chflg & XT_ON) || (chanp->xt_chflg & XT_WCLOSE) )
				continue;

			for (k = 0 ; k < 2 ; k++) {
			    if (chanp->xt_msg[k].mp) {
				outtime = chanp->xt_msg[k].xt_saveoutpkts * ctlp->xt_HZperpkt;
				if(outtime < (3*HZ)) outtime = (3*HZ);
				if(chanp->xt_msg[k].timestamp + outtime <= time_now) {
				    debug(("nxtscan: scaned output chan %d\n", (int)j));
				    ctlp->xt_ctlflg |= XT_WANTTIMEOUT;
				    if(ctlp->xt_ttyq) /* enable xtisrv() */
					qenable( RD(ctlp->xt_ttyq) );
				    break;
				}
			    }
			}
		    }
		}
	}

	if ( xt_scanon )    /* Reschedule if anyone's still alive. */
		timeout(nxtscan, 0, XTSCANRATE);
}


/* nxttimeout() - process xt protocol timeouts.
 *
 * Nxtscan() looks around for potential timeouts and qenables xtisrv() if a
 * timeout is needed. Nxttimeout() is called from xtisrv() to do the actual
 * timeout processing. Timeouts are processed this roundabout way to:
 *
 *	1) keep nxtscan() short because it is called by timeout() with a
 *	   high interrupt priority level and it is best to do as little
 *	   processing as possible with high interrupt priority.
 *
 *	2) to prevent concurrency problems which result from modifying
 *	   data structures from a routine such as nxtscan() which can
 *	   preempt other xt routines.
*/
STATIC void
nxttimeout(ctlp)
struct xtctl *ctlp;
{
	struct xt_chan *chanp;
	mblk_t *bp1;
	time_t time_now;
	short j, k;
	unsigned char *mylastscan;
	long outtime;
	unsigned char firstchar;
	unsigned short size;

	time_now = lbolt;

	scandebug(("Entered nxttimeout\n"));

	/* don't timeout if not in use or exiting or unlinked or network xt */
	if (((ctlp->xt_ctlflg & XT_INUSE) == 0) || (ctlp->xt_maxpkt == 1))
		return;
	if (ctlp->xt_ctlflg & (XT_EXIT|XT_UNLINK))
		return;

	/* First, check for input packet timeout.
	 * If necessary, discard the packet by resetting input state.
	 */
	if ( ctlp->xt_intime && ctlp->xt_intime + ctlp->xt_recvtimo <= time_now ) {
		inerrdebug(("Z"));
		debug(("nxttimeout: scaned input\n"));
		STATS(ctlp, S_RTIMO);
		ctlp->xt_instate = PR_NULL;
		ctlp->xt_intime = 0;
		if ( ctlp->xt_inbp ) {
			freemsg(ctlp->xt_inbp);
			ctlp->xt_inbp = NULL;
		}
	}

	/* Next, if nxtscan() set XT_WANTTIMEOUT flag, run through all channels
	 *   looking for output ACK timeouts.
	 * Resend the packets that have timed out.
	 * If canput fails, abort but remember channel it failed on. Start
	 *   on that channel next time.
	 */
	if(ctlp->xt_ctlflg & XT_WANTTIMEOUT) {
	    mylastscan = &ctlp->xt_lastscan;
	    for ( j = *mylastscan, *mylastscan = 0 ; j < MAXPCHAN && *mylastscan == 0 ; j++) {
		chanp = &ctlp->xt_chan[j];
		if ( !(chanp->xt_chflg & XT_ON) || (chanp->xt_chflg & XT_WCLOSE) )
			continue;

		for (k = 0 ; k < 2 ; k++) {
		    if (chanp->xt_msg[k].mp) {
			outtime = chanp->xt_msg[k].xt_saveoutpkts * ctlp->xt_HZperpkt;
			if(outtime < (3*HZ)) outtime = (3*HZ);
			if(chanp->xt_msg[k].timestamp + outtime <= time_now) {
			    debug(("nxttimeout: scaned output chan %d\n", (int)j));

			    if(canput(ctlp->xt_ttyq->q_next) == 0) {
				debug(("nxttimeout output: canput failed\n"));
				*mylastscan = j;
				break;
			    }

			    /* See note on dupmsg() in xtsend() */
			    bp1 = xtdupmsg("nxttimeout", chanp->xt_msg[k].mp);
			    if ( bp1 ) {
				/* If encoding is set can't log now because
				 * packet is already encoded. This is not
				 * worth the code it would take to fix.
				 */
				if( ISTRACEON(ctlp) & !ctlp->xt_hex )
				    logpkt(*bp1->b_rptr,
					(unsigned short)(*(bp1->b_rptr+1)),
					(char)0,
					bp1->b_rptr+2, ctlp, XMITLOG);
				STATS(ctlp, S_XTIMO);

				chanp->xt_msg[k].timestamp = lbolt;
				putnext(ctlp->xt_ttyq, bp1);
				bp1 = NULL;
			    }
			}
		    }
		}
	    }
	    ctlp->xt_ctlflg &= ~XT_WANTTIMEOUT;
	}
}

/*
 * packet logger
 *
 * The first byte of tp->pktpart is the first byte of the header which is
 *   a different format for regular and network xt. The next two bytes are
 *   packet size for both regular and network xt. After that are up to
 *   PKTPTSZ-3 = 8 data bytes.
 * 
 * RETURNS: none
 *
 * NOTE: If you have a need for an xtt trace which shows the whole
 * packet rather than just the first few bytes, it is easy:
 *
 * +) Change the define of PKTPTSZ in nxt.h to the max width
 *    packet you want to be able to capture.
 *
 * +) Change the define PKTHIST in nxt.h so that the structure Tbuf
 *    is less that the size of the largest streams buffer (currently
 *    4096). This is required because of the implementation of
 *    case XTIOCTRACE in xtqsrv().
 *
 * +) Change /etc/master.d/xt to reflect the new size of the nxtctl
 *    structure.
 *
 * +) Recompile xt and reboot. Recompile xtt.
 *
 * A nice enhancement would be a new ioctl and parameters to xtt
 * to make these changes dynamically.
 *
 */
STATIC void
logpkt(firstchar, size, command, ptr, ctlp, flag)
unsigned char firstchar;
unsigned short size;
unsigned char command;
unsigned char *ptr;
struct xtctl *ctlp;
int flag;
{
	struct Tpkt *tp;
	unsigned char *pbuf;
	int tracesize;

	if(ctlp->xt_maxpkt == 1) ctlp->trace.flags |= TRACE_NETXT;

	tp = &ctlp->trace.log[ctlp->trace.index];
	pbuf = tp->pktpart;

	*pbuf++ = firstchar;
	*pbuf++ = (size >> 8) & 0xFF;
	*pbuf++ = size & 0xFF;
	tracesize = min((unsigned int)size, PKTPTSZ-3);
	if(flag&TRACE_CMDBYTE) {
		*pbuf++ = command;
		--tracesize;
		flag &= ~TRACE_CMDBYTE;
	}
	if( ptr )
		bcopy((char *)ptr, (char *)pbuf, tracesize);

	tp->flag = flag;
	tp->time = lbolt;

	if ( ++ctlp->trace.index >= PKTHIST )
		ctlp->trace.index = 0;
	if ( ctlp->trace.used < PKTHIST )
		ctlp->trace.used++;
}

/* This function picks up the next character from the supplied buffer. It takes
 * care of the continuation blocks in the supplied buffer.
 *
 * RETURN VALUES : the next character from the supplied buffer
*/

STATIC unsigned char	
bnextchar(pbp)
mblk_t **pbp;
{
	unsigned char	c = '\0';
	mblk_t * bp;

	if (!pbp || !(*pbp)) {
		cmn_err(CE_NOTE, "XT bnextchar: no pbp");
		return c;
	}
	bp = *pbp;
	if ( bp->b_rptr < bp->b_wptr )
		c = *bp->b_rptr++;
	if ( bp->b_rptr >= bp->b_wptr ) {
		*pbp = bp->b_cont;
		freeb(bp);
	}

	return(c);
}

/* bnextnchars: Get n bytes from mblk *pbp and put them in buf.
** If n bytes are not available, get as many as are available.
** Return the number of bytes which were found. This is much more
** efficient than multiple calls to bnextchar(). In fact, benchmarks
** show that it measurably increases the speed of xt over starlan.
**
** RETURN VALUES : no. of bytes copied 
*/
STATIC int
bnextnchars(pbp, buf, n)
mblk_t **pbp;
register char *buf;
register int n;
{
	register mblk_t *bp;
	register int count = 0;

	if (!pbp || !(*pbp)) {
		cmn_err(CE_NOTE, "XT bnextnchars: no pbp");
		return(0);
	}

	for( bp = *pbp ; bp && count < n ; ) {
		while ( bp->b_rptr < bp->b_wptr ) {
			*buf++ = *bp->b_rptr++;
			if( ++count >= n)
				break;
		}
		if ( bp->b_rptr >= bp->b_wptr ) {
			*pbp = bp->b_cont;
			freeb(bp);
			bp = *pbp;
		}
	}
	return(count);
}

STATIC mblk_t *
xtallocb(where, size, pri)
char *where; /* string identifying which allocb failed */
int size, pri;
{
	mblk_t *retval;

	if( (retval = allocb(size, pri)) == NULL ) {
		cmn_err(CE_NOTE,
		    "XT: %s: allocb of %d bytes failed: no streams buffers",
		    where, size);
	}
	return(retval);
}

STATIC mblk_t *
xtdupmsg(where, mp)
char *where; /* string identifying which allocb failed */
mblk_t *mp;
{
	mblk_t *retval;

	if( (retval = dupmsg(mp)) == NULL ) {
		cmn_err(CE_NOTE,
		    "XT: %s: dupmsg failed: no streams message blocks", where);
	}
	return(retval);
}


#ifdef DEBUG

/* Adaptation of kernel printf code to call cmn_err() once for every
** character. This will cause interrupt priority to get lowered between
** every character, where the kernel printf will keep interrupt priority
** high for a whole string of characters. Lowering priority for every
** character prevents loss of RS232 interrupts. This version of printf
** also inserts delays after every line of output to prevent overflowing
** the console.
*/

int xtbufonly = 0; /* can change from the DEMON */

xtprintf(fmt, x1)
register char *fmt;
unsigned x1;
{
	register int	c;
	register uint	*adx;
	register char	*s;
	register int	opri;
	char tmpbuf[2];

	adx = &x1;
	tmpbuf[1] = '\0';

loop:
	while ((c = *fmt++) != '%') {
		if (c == '\0') {
			if(xtbufonly == 0) xtnap();
			return;
		}
		tmpbuf[0] = c;
		if(xtbufonly)
			cmn_err(CE_CONT, "!%s", tmpbuf);
		   else
			cmn_err(CE_CONT, "%s", tmpbuf);
	}
	c = *fmt++;
	if (c == 'd' || c == 'u' || c == 'o' || c == 'x')
		xtprintn((long)*adx, c=='o'? 8: (c=='x'? 16:10));
	else if (c == 's') {
		s = (char *)*adx;
		while (c = *s++) {
			tmpbuf[0] = c;
			if(xtbufonly)
				cmn_err(CE_CONT, "!%s", tmpbuf);
			   else
				cmn_err(CE_CONT, "%s", tmpbuf);
		}
	} else if (c == 'D') {
		xtprintn(*(long *)adx, 10);
		adx += (sizeof(long) / sizeof(int)) - 1;
	}
	adx++;
	goto loop;
}

xtprintn(n, b)
long n;
register b;
{
	register i, nd, c;
	int	flag;
	int	plmax;
	char d[12];
	char tmpbuf[2];

	tmpbuf[1] = '\0';
	c = 1;
	flag = n < 0;
	if (flag)
		n = (-n);
	if (b==8)
		plmax = 11;
	else if (b==10)
		plmax = 10;
	else if (b==16)
		plmax = 8;
	if (flag && b==10) {
		flag = 0;
		tmpbuf[0] = '-';
		if(xtbufonly)
			cmn_err(CE_CONT, "!%s", tmpbuf);
		   else
			cmn_err(CE_CONT, "%s", tmpbuf);
	}
	for (i=0;i<plmax;i++) {
		nd = n%b;
		if (flag) {
			nd = (b - 1) - nd + c;
			if (nd >= b) {
				nd -= b;
				c = 1;
			} else
				c = 0;
		}
		d[i] = nd;
		n = n/b;
		if ((n==0) && (flag==0))
			break;
	}
	if (i==plmax)
		i--;
	for (;i>=0;i--) {
		tmpbuf[0] = "0123456789ABCDEF"[d[i]];
		if(xtbufonly)
			cmn_err(CE_CONT, "!%s", tmpbuf);
		   else
			cmn_err(CE_CONT, "%s", tmpbuf);
	}
}

/*
** nap routine to prevent console overflow.
*/
int xtdelay = 0x20; /* can change from the DEMON */
xtnap()
{
	int i, j;
	for(i=0 ; i<xtdelay ; ++i)
		for(j=0 ; j<500 ; ++j);
}

#endif /* DEBUG */
