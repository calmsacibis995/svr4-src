/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ws/chan.h	1.1.2.1"

#ifndef	_SYS_WS_CHAN_H
#define	_SYS_WS_CHAN_H

/*
 * Channel protocol definitions for the IWE. All control/special data messages
 * exchanged between IWE modules/drivers are of this format.
 */

struct ch_protocol {
	long	chp_type,	/* type of message */
		chp_tstmp,	/* timestamp of message */
		chp_stype,	/* message sub-type (used for control msgs) */
		chp_stype_cmd,	/* message sub-type cmd (for control msgs) */
		chp_stype_arg,	/* message sub-type arg (for control msgs) */
		chp_chan;	/* channel ID -- will be set by CHANMUX
				 *    and is write-side only */
};

typedef struct ch_protocol ch_proto_t;

/* chp_type ID */
#define CH_DATA	1	/* ch_protocol data message identifier */
#define CH_CTL	2	/* ch_protocol control message identifier */

/* messages from CHANMUX to principal stream; chp_stype CH_CHAN and
 * its chp_stype_cmds. chp_type should be CH_CTL
 */
#define CH_CHAN	( ('C'<<16) | ('H'<<8) | ('N') )
#define CH_CHANCLOSE	1
#define CH_CHANOPEN	2

/* chp_stype CH_MSE to indicate mouse events; chp_type should be CH_DATA */
#define CH_MSE	( ('M'<<16) | ('S'<<8) | ('E') )

/* chp_stype CH_NOSCAN to indicate already-scanned data. The attached message
 * block is raw data suitable to be forwarded on the read side to upper
 * modules of the STREAMS TTY sub-system. chp_type is CH_DATA
 */
#define CH_NOSCAN	( ('N'<<16) | ('S'<<8) | ('C') )

/* messages to the CHANMUX driver from the principal stream;
 * chp_stype CH_PRINCSTRM and its chp_stype_cmds. chp_type should be CH_CTL
 */
#define CH_PRINC_STRM	( ('P'<<16) | ('S'<<8) | ('T') )
#define CH_CHANGE_CHAN	1
#define CH_CLOSE_ACK	2
#define CH_OPEN_RESP	3

/* CH_CTL messages for CHAR module; chp_stype CH_CHR and its chp_stype_cmds */
#define CH_CHR	( ('C' << 16) | ('H' << 8) | 'R')
#define CH_CHRMAP	( ('C'<<24) | ('M'<<16) | ('A'<<8) | 'P' )
#define CH_SCRMAP	( ('S'<<24) | ('M'<<16) | ('A'<<8) | 'P' )
#define CH_LEDSTATE	('L' << 8 | 'D')
#ifdef MERGE386
#define CH_SETMVPI	( ('M'<<24) | ('3'<<16) | ('8'<<8) | '6' )
#define CH_DELMVPI	( ('3'<<24) | ('8'<<16) | ('6'<<8) | 'M' )
#endif /* MERGE386 */

/* chp_stype CH_TCL; its stype_cmds are in sys/ws/tcl.h. These commands
 * come from the ANSI module or its equivalent. All are ch_type CH_CTL
 */
#define CH_TCL ( ('T' << 16) | ('C' << 8) | 'L')

/* chp_stype CH_XQ and its chp_stype_cmds. They are part of the X queue handling
 * message protocol and are exchanged between the principal stream and CHAR or
 * its equivalent. All are of ch_type CH_CTL */

#define CH_XQ ( ('X' << 8) | 'Q')
#define	CH_XQENAB	1	/* from principal stream to CHAR */
#define CH_XQDISAB	2	/* from principal stream to CHAR */
#define CH_XQENAB_ACK	3	/* from CHAR to principal stream */
#define CH_XQENAB_NACK	4	/* from CHAR to principal stream */
#define CH_XQDISAB_ACK	5	/* from CHAR to principal stream */
#endif /* _SYS_WS_CHAN_H */
