/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)head.sys:sys/xtproto.h	11.1.7.1"

/*
**	Bx -- Blit packet protocol definition
*/

typedef	unsigned char	Pbyte;			/* The unit of communication */

#define	NPCBUFS		2			/* Double buffered protocol */
#define	MAXPCHAN	8			/* Maximum channel number */

/*	Packet Header:
 *
 *      High order bits                                      Low order bits
 *	____________________________________________________________________
 *	|       |       |        |       ||                                |
 *	| ptyp  | cntl  | chan   | seq   || dsize                          |
 *	|       |       |        |       ||                                |
 *	--------------------------------------------------------------------
 *	 15      14      13    11 10    8  7                              0
 *
 *	ptyp  - always 1.
 *	cntl  - TRUE if control packet.
 *	chan  - channel number.
 *	seq   - sequence number.
 *	dsize - size of the data part.
 *
 *	Note:   The following macros are used to set the bits in the packet
 *	        header for portability reasons. The bit fields that can be
 *	        defined in C are preferable for simplicity reasons, but are
 *	        highly machine dependent.
 */

#define GET_BITS(byte,pos,len,mask)     ((byte & mask) >> pos)
#define SET_BITS(byte,pos,len,mask,val) (byte = (byte &(~mask))|((val<<pos)&mask))

#define PTYPPOS             7
#define PTYPSIZ             1
#define PTYPMASK            0x80
#define GET_PTYP(pkt)       GET_BITS(pkt.header[0],PTYPPOS,PTYPSIZ,PTYPMASK)
#define SET_PTYP(pkt,val)   SET_BITS(pkt.header[0],PTYPPOS,PTYPSIZ,PTYPMASK,val)

#define CNTLPOS             6
#define CNTLSIZ             1
#define CNTLMASK            0x40
#define GET_CNTL(pkt)       GET_BITS(pkt.header[0],CNTLPOS,CNTLSIZ,CNTLMASK)
#define SET_CNTL(pkt,val)   SET_BITS(pkt.header[0],CNTLPOS,CNTLSIZ,CNTLMASK,val)

#define CHANPOS             3
#define CHANSIZ             3
#define CHANMASK            0x38
#define GET_CHAN(pkt)       GET_BITS(pkt.header[0],CHANPOS,CHANSIZ,CHANMASK)
#define SET_CHAN(pkt,val)   SET_BITS(pkt.header[0],CHANPOS,CHANSIZ,CHANMASK,val)

#define SEQPOS              0
#define SEQSIZ              3
#define	SEQMOD              8
#define SEQMASK             0x07
#define GET_SEQ(pkt)        GET_BITS(pkt.header[0],SEQPOS,SEQSIZ,SEQMASK)
#define SET_SEQ(pkt,val)    SET_BITS(pkt.header[0],SEQPOS,SEQSIZ,SEQMASK,val)

#define HEADER_DSIZE        header[1]

/*
**	Packet definition for maximum sized packet
*/
#define	PKTHDRSIZE	(2 * sizeof(Pbyte))	/* packet header part size */
#define	MAXPKTDSIZE	(32 * sizeof(Pbyte))	/* Maximum data part size */
#define	EDSIZE		(2 * sizeof(Pbyte))	/* Error detection part size */

struct Packet
{
	Pbyte	header[PKTHDRSIZE];	/* Packet Header */
	Pbyte	data[MAXPKTDSIZE];	/* Data part */
	Pbyte	edb[EDSIZE];		/* Error detection part */
};

typedef struct Packet *	Pkt_p;

/*
**	Control codes
*/

#define	PCDATA		(Pbyte)002		/* Data only control packet       */
#define	ACK		(Pbyte)006		/* Last packet ok and in sequence */
#define	NAK		(Pbyte)025		/* Last packet out of sequence    */

/*
**	Definition of a structure to hold status information
**	for a conversation with a channel.
*/

struct Pktstate
{
	struct Packet	pkt;			/* The packet */
	short		timo;			/* Timeout count */
	unsigned char	state;			/* Protocol state */
	unsigned char	size;			/* Packet size */
};

typedef struct Pktstate *Pks_p;

struct Pchannel
{
	struct Pktstate	pkts[NPCBUFS];		/* The packets */
	Pks_p		nextpkt;		/* Next packet to be acknowledged */
	Pbyte		cdata[SEQMOD];		/* Remember transmitted control data */
	Pbyte		rseq	:SEQSIZ;	/* Next receive sequence number */
	Pbyte		xseq	:SEQSIZ;	/* Next transmit sequence number */
	char		outen;			/* Output packets enabled */
	char		flags;			/* Control flags */
#if XTDRIVER == 1
	char		channo;			/* This channel's number */
	char		link;			/* This channel's link */
#endif
};

#define	XACK		1			/* Send ACK */
#define	XNAK		2			/* Send NAK */
#define	XCDATA		4			/* Send control data in ACK packet */

typedef struct Pchannel *Pch_p;

/**	Transmit packet states	**/

enum {	px_null, px_ready, px_wait, px_ok	};

#define	PX_NULL		(int)px_null		/* Empty packet */
#define	PX_READY	(int)px_ready		/* Full packet awaiting transmission */
#define	PX_WAIT		(int)px_wait		/* Packet awaiting acknowledgement */
#define	PX_OK		(int)px_ok		/* Packet has been acknowledged */

/**	Receive packet states	**/

enum { pr_null, pr_size, pr_data };

#define	PR_NULL		(int)pr_null		/* New packet expected */
#define	PR_SIZE		(int)pr_size		/* Size byte next */
#define	PR_DATA		(int)pr_data		/* Receiving data */
