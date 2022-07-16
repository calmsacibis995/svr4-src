/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_NXTPROTO_H
#define _SYS_NXTPROTO_H

#ident	"@(#)head.sys:sys/nxtproto.h	1.7.3.1"

/*
 * nxtproto.h -- xt packet protocol definitions.
 *
 * For additional information on xt packet structure, see the big comment
 * at the beginning of xt.c.
 */

typedef	unsigned char	Pbyte;			/* The unit of communication */

#define	NPCBUFS		2			/* Double buffered protocol */
#define	MAXPCHAN	8			/* Maximum channel number */
#define	CHANBITS	3			/* Bits for channel number */
#define	CHANMASK	07			/* 2**CHANBITS - 1 */
#define PKTHEADSIZE	(2 * sizeof(Pbyte))	/* Header size */
#define NETHEADSIZE	(3 * sizeof(Pbyte))	/* Header size for network xt */
#define	MAXPKTDSIZE	(32 * sizeof(Pbyte))	/* Maximum data part size 
						   for incoming packets. */
#define MAXOUTDSIZE	(252 * sizeof(Pbyte))	/* Maximum data part size
						   for outgoing packets. */
#define	EDSIZE		(2 * sizeof(Pbyte))	/* Error detection part size */
#define	SEQMOD		8			/* Sequence number modulus */
#define	SEQBITS		3			/* Bits for sequence number */
#define	SEQMASK		07			/* 2**SEQBITS - 1 */

/*
 * Control codes.
 */

#define	PCDATA		(Pbyte)002		/* Data only control packet */
#define	ACK		(Pbyte)006		/* Last packet with same sequence ok and in sequence */
#define	NAK		(Pbyte)025		/* Last packet with same sequence received out of sequence */

/*
 * Receive packet states.
 */

#define PR_NOINPUT	0x80		/* additional input not needed */

#define	PR_NULL		(1)		/* New packet expected */
#define	PR_GETBUF	(2|PR_NOINPUT)	/* About to get buffer */
#define	PR_SIZE		(3)		/* Size byte next */
#define	PR_DATA		(4)		/* Receiving data */
#define	PR_SENDUP	(5|PR_NOINPUT)	/* Send valid packet upstream */

#define	PR_NETNULL	(6|PR_NOINPUT)	/* Same states for network xt */
#define	PR_NETSIZE1	(7)
#define	PR_NETSIZE2	(8)		/* Two size bytes for network xt */
#define	PR_NETGETBUF	(9|PR_NOINPUT)
#define	PR_NETGETCMD	(10)		/* Get the command byte next */
#define	PR_NETDATA	(11)		/* Get the rest of the data */
#define PR_NETLOGPKT	(12|PR_NOINPUT)	/* About to log the packet */
#define	PR_NETSENDUP	(13|PR_NOINPUT)	/* Send buffer upstream */
#define	PR_NETERROR	(14)		/* Got an error in network xt */

#endif	/* _SYS_NXTPROTO_H */
