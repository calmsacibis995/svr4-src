/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)xt:xtraces.c	2.6.5.1"
/*
 * Routine to print 'xt' driver traces
 */
#include "stdio.h"
#include "errno.h"
#ifdef SVR32
#include "sys/patch.h"
#endif /* SVR32 */
#include "sys/param.h"
#include "sys/types.h"
#include "ctype.h"
#include "sys/tty.h"
#include "sys/jioctl.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/nxtproto.h"
#include "sys/nxt.h"		/* defines XTRACES and XTSTATS to be 1       */

#define Fprintf (void)fprintf

#define HDRSZ		3	/* pkt hdr : control flag, pkt seq,data size */ 
#define SUCCEED		0	/* success of the operation		     */
#define FAIL		-1	/* failure of the operation		     */
#define TRUE		0xff	/* true boolean				     */
#define FALSE		0   	/* false boolean			     */

#if XTRACE == 1
#define min(x, y)	((x) < (y)? (x) : (y))
#define PTYP(tp)	(((tp)->pktpart[0]>>7)&01)
#define CNTL(tp)	(((tp)->pktpart[0]>>6)&01)
#define SEQ(tp)		((tp)->pktpart[0]&07)
#undef  CHAN		/* from xt.h */
#define CHAN(tp)	(((tp)->pktpart[0]>>3)&07)
#define DSIZE(tp)	( ((unsigned short)((tp)->pktpart[1])) << 8 | (tp)->pktpart[2] )

struct Tbuf Traces;
extern long time();
extern char *ctime();
char *msg[] = {
	"", "SENDCHAR", "NEW", "UNBLK", "DELETE", "EXIT",
	"DEFUNCT", "SENDNCHARS", "RESHAPE", "JAGENT", "C_NOFLOW", "C_YESFLOW"
};	

#define MINCTRL		C_SENDCHAR
#define MAXCTRL		C_YESFLOW

#endif


/*****************************************************************************

  NAME 		: xtraces - display   the trace record 

  SYNOPSIS	: int xtraces()

  DESCRIPTION   : 

  RETURNS	: SUCCEED or FAIL 

  NOTES		: 1. The received packet consists of the following :

		     REGULAR XT:

		     <1:1|cntl:1|seq:3|cbits:3><dsize:8><dsize bytes of data>

		     <crc1><crc2>

		     NETWORK XT:

			Byte 1: 0                 : 1
				1                 : 1
				unused            : 1
				flow control flag : 1
				ACK flag          : 1
				channel number    : 3

			Byte 2: size high bits    : 8

			Byte 3: size low bits     : 8

			Size bytes of data

		     HOWEVER, the xt driver delivers packets to xtraces in a
		     modified, semi-protocol independent format. The first
		     byte of tp->pktpart is the first byte of the headers
		     shown above, so this byte has a different format for
		     regular and network xt. The next two bytes are packet
		     size for both regular and network xt. After that are
		     the data bytes for both regular and network xt.

		     Note that the format of ACK, NAK, UNBLK, etc. packets
		     are also different between regular and network xt as can
		     be seen from the code.

		  2. The trace record consists of 40 (PKTHIST) records of 
		     captured packets with each captured packet having a 
		     maximum size of 11 (PKTPTSZ).
 
		  3. The trace data begins from index Traces.used and ends
		     at index Traces.index in the circular log buffer.
		
		  4. The trace is presented in the following format :

		     Time stamp Control/Data Channel Xmt/Rcv Seq data


*****************************************************************************/

int
xtraces(ofd)
register FILE *ofd;
{
#if XTRACE == 1
	register struct Tpkt *tp;
	register int istart;
	register int ntrace; 
	long t;


	/* It is assumed that an XTIOCTRACE ioctl with parameter &Traces
	 * has been done by the calling routine before xtraces() is called.
	 */

	/* return if trace is not on or no trace has been recorded yet       */

	if (((ntrace = Traces.used) == 0)  || ((Traces.flags & TRACEON) == 0))
		return(SUCCEED);

	/* display the current time and Trace Lock status		     */
 
	(void)time(&t);
	(void)fprintf(ofd, "\n%15.15s %sXT Packet Traces:-\n", ctime(&t)+4,
		(Traces.flags&TRACE_NETXT) ? "Network " : "");
	if (Traces.flags&TRACELOCK)
		(void)fprintf(ofd, "(Trace is locked by an error!)\n");

	/* The trace buffer is circular. Trace ends at traces.index and total*/
	/*  no. of traces is Traces.used 				     */

	if ((istart = (Traces.index - Traces.used)) < 0) 
		istart += PKTHIST; 

	tp = &Traces.log[istart];	/* points to the first captured pkt  */ 

	if(Traces.flags&TRACE_NETXT) net_xtraces(ofd, tp, ntrace);
		else reg_xtraces(ofd, tp, ntrace);

	(void)fflush(ofd);
	return(SUCCEED);
}

/* xtraces for regular (not network) xt protocol.
*/
reg_xtraces(ofd, tp, ntrace)
register FILE *ofd;
register struct Tpkt *tp;
register int ntrace; 
{
	register Pbyte *pdata;
	register int tmdelta;
	register time_t tmstart;
	register int ndata; 
	unsigned dsize;

	tmstart = tp->time;

	(void)fprintf(ofd, "\n  TIME ");
	(void)fprintf(ofd, "C/D ");
	(void)fprintf(ofd, "CHAN  ");
	(void)fprintf(ofd, "FLOW  ");
	(void)fprintf(ofd, "SEQ ");
	(void)fprintf(ofd, "SIZE  ");
	(void)fprintf(ofd, "DATA\n\n");

	/* keep displaying the trace until there is no more data	     */
 
	do {
		/* display   the time stamp 				     */

		tmdelta = tp->time - tmstart;
		(void)fprintf(ofd, "%3d.%2.2d  ", tmdelta/HZ, tmdelta%HZ);

		dsize = DSIZE(tp);

		/* If this is a bad packet, dump header in hex since
		** it may be garbage.
		*/
		if(tp->flag&TRACE_BADPKT) {
			(void)fprintf(ofd, "BAD RECEIVE PACKET: HEADER ");
			(void)fprintf(ofd, "<%02x>",
				(unsigned)(tp->pktpart[0]&0xff));
			if(dsize != 0)
				(void)fprintf(ofd, ": SIZE %d", dsize);
			(void)fprintf(ofd, "\n");
			goto nextpkt;
		}

		(void)fprintf(ofd,  CNTL(tp) ? "C" : "D");

		/* display   the channel number 			     */

		(void)fprintf(ofd, "   %d    ", CHAN(tp));

		/* display   the direction of transmission : xmt/rcv  	     */

		(void)fprintf(ofd, (tp->flag&RECVLOG) ? "Rcv " : "Xmt ");

		/* display   the rcv/xmt sequence			     */

		(void)fprintf(ofd, "   %d  ", SEQ(tp));

		/* display data size					     */

		(void)fprintf(ofd, "%3d   ", dsize);

		/* display control byte (ACK) with no data		     */
		if (CNTL(tp) && dsize == 0) {
			(void)fprintf(ofd, "ACK\n");
			goto nextpkt;
		}


#ifdef i386
		/* a max of 8 characters (PKTPTSZ) - HDRSZ displayed */ 
		ndata = min(DSIZE(tp), sizeof tp->pktpart - HDRSZ);
#else
		/* a max of PKTPTSZ - HDRSZ = 8 displayed */ 

		ndata = min(dsize, sizeof tp->pktpart - HDRSZ);
#endif

		/* get the pointer to first byte of data 	     */

		pdata = &tp->pktpart[HDRSZ];

		/* display ACK, NAK or hex value of control byte     */

		if (CNTL(tp)) /*   && dsize > 0  */		{
			(void)fprintf(ofd, *pdata == ACK ? "ACK " :
				*pdata == NAK ? "NAK " : 
				"<%02x>", *pdata & 0xff);
			pdata++;
			ndata--;
			dsize--;
		}

		/* control msgs sent to the UNIX environment 	     */ 

		if ((tp->flag&RECVLOG) && dsize > 0
		    && MINCTRL <= *pdata && *pdata <= MAXCTRL){
			(void)fprintf(ofd, "%s ", msg[*pdata++]);
			ndata--;
			dsize--;
		} 

		/* graphic chars printed as %c and non-graphic as %x */
		/* while data in the packet part of the current  rec */ 

		while (ndata > 0) {
			ndata--;
			(void)fprintf(ofd, isprint(*pdata & 0xff) ? 
				"%c" : "<%02x>",*pdata & 0xff);
			pdata++;
			dsize--;
		}

		/* display "more" if more data left		     */

		(void)fprintf(ofd, dsize > 0 ? " [more]" : "");

		fprintf(ofd, "\n");
	
nextpkt:
  		if (++tp >= &Traces.log[PKTHIST]) 
			tp = Traces.log; 

	} while (--ntrace);
}


/* xtraces for network xt protocol.
*/
net_xtraces(ofd, tp, ntrace)
register FILE *ofd;
register struct Tpkt *tp;
register int ntrace; 
{
	register Pbyte *pdata;
	register int tmdelta;
	register time_t tmstart;
	register int ndata; 
	unsigned dsize;

	tmstart = tp->time;

	(void)fprintf(ofd, "\n  TIME ");
	(void)fprintf(ofd, "C/D ");
	(void)fprintf(ofd, "CHAN  ");
	(void)fprintf(ofd, "FLOW  ");
	(void)fprintf(ofd, "SIZE  ");
	(void)fprintf(ofd, "DATA\n\n");

	/* keep displaying the trace until there is no more data	     */
 
	do {
		/* display   the time stamp 				     */

		tmdelta = tp->time - tmstart;
		(void)fprintf(ofd, "%3d.%2.2d  ", tmdelta/HZ, tmdelta%HZ);

		dsize = DSIZE(tp);

		(void)fprintf(ofd, (tp->pktpart[0]&0x8) ? "C" : "D");

		/* display   the channel number 			     */

		(void)fprintf(ofd, "   %d    ", (tp->pktpart[0]&0x7) );

		/* display   the direction of transmission : xmt/rcv  	     */

		(void)fprintf(ofd, (tp->flag&RECVLOG) ? "Rcv " : "Xmt ");

		/* display data size					     */

		(void)fprintf(ofd, "  %3d   ", dsize);

		if( tp->pktpart[0]&0x8 ) { /* flow control ACK packet */
			(void)fprintf(ofd, "FLOW CONTROL ACK\n");
			goto nextpkt;
		}

		/* a max of PKTPTSZ - HDRSZ = 8 displayed */ 

		ndata = min(dsize, sizeof tp->pktpart - HDRSZ);

		/* get the pointer to first byte of data 	     */

		pdata = &tp->pktpart[HDRSZ];

		/* control msgs sent to the UNIX environment 	     */ 

		if ((tp->flag&RECVLOG) && MINCTRL <= *pdata && 
			*pdata <= MAXCTRL){
			(void)fprintf(ofd, "%s ", msg[*pdata++]);
			ndata--;
			dsize--;
		} 

		/* graphic chars printed as %c and non-graphic as %x */
		/* while data in the packet part of the current  rec */ 

		while (ndata-- > 0) {

			(void)fprintf(ofd, isprint(*pdata & 0xff) ? 
				"%c" : "<%02x>",*pdata & 0xff);
			pdata++;
			dsize--;
		}

		/* display "more" if more data left		     */

		(void)fprintf(ofd, dsize > 0 ? " [more]" : "");

		fprintf(ofd, "\n");
nextpkt:
	
  		if (++tp >= &Traces.log[PKTHIST]) 
			tp = Traces.log; 

#ifdef i386
	} while (-ntrace);
#else
	} while (--ntrace);
#endif

}

#else  /* XTRACE == 1 */
}
#endif /* XTRACE == 1 */
