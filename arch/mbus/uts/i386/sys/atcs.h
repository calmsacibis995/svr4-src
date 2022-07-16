/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	
#ifndef _SYS_ATCS_H
#define _SYS_ATCS_H
#ident	"@(#)mbus:uts/i386/sys/atcs.h	1.3.1.3"

#define MAXBAUDS 16

/* bits in the minor number */
#define ATCSMINORMSK	0x7F		/* reserve bit 7 */
#define	RAWMSK		0x80		/* bit 7, open as raw device */


/* flags in l_state */
#define OUTBUSY		0x02		/* outstanding output request */
#define INSTOP		0x04		/* input interrupts stopped */
#define INBUSY		0x08		/* output stopped while input busy */
#define INCLOSE		0x10		/* driver is currently in the
								 * close routine */
#define WANTLINE	0x20		/* driver has been called to
								 * open a line but the INCLOSE bit
								 * is set */
#define CARRF		0x40		/* actual carrier state */
#define RECFF		0x80		/* received parity marking FF */
#define RECFFNUL	0x01		/* received parity marking FF and NUL */

/* flags in l_cci_state */
#define SWITCHED		0x01		/* line switched to another host */
#define ATTACHED		0x02		/* line is attached to a subchannel  */
#define ATTACH_SENT		0x04		/* attach sent for a subchannel */
#define DETACH_SENT		0x08		/* detach sent for a subchannel */
#define ATTACH_CANCEL	0x10		/* attach cancelled for a subchannel */

/* ioctl  command code for switch */
#define ATCS_SWITCH		0x0100		/* command code */

/* interrupt action codes:
 *  When a message is sent out, this code gives the action to take
 *  when the response is received.  Kept in BIND_ACTION().
 */ 
#define A_ODD		0	/* won't happen */
#define A_ASYNC		1	/* async command, don't process response */
#define A_SYNC		2	/* sync command, wakeup persone waiting */
#define A_SPCHAR	3	/* special char received, process event */
#define A_CD		4	/* carrier detect state change */
#define A_CLEARBUSY 5	/* output done, clear busy */
#define A_OUTFLOW	6	/* output flow control, push output */
#define A_OUTBUFC	7	/* output write complete */
#define A_INCHARS	8	/* input characters received */
#define A_CLOSE		9	/* in the close routine */
#define A_MAX		10

/* Code used in Min2Line to say empty entry */
#define M2LNone 0xff

/*
 * line structure, one per line on a board
 */
struct atcs_lines {
	char	l_state;		/* driver specific line state */
	unchar	l_cci_state;	/* driver specific cci state */
	unchar	l_ucodeVer;		/* version of atcs code we're dealing with */
	unchar	l_sess_stat;	/* session status for next host */
	unchar	l_mask;			/* output delay timeout value */
	minor_t l_minor;		/* the minor number associated with this line */
	struct strtty *l_tty;	/* pointer to tty structure for this line */
	long	l_chan;			/* the open message passing channel for this line */
	mb2socid_t l_CCISocket;	/* socket to controlling CCI system */
	mb2socid_t l_HisSocket;	/* his socket connection */
	mb2socid_t l_MySocket;	/* my socket connection */
	int		l_rbufsiz;		/* number of chars in receive buffer */
	int		l_tbufsiz;		/* size of the buffer in the controller */
	int		l_tbufavail;	/* number of bytes unfilled in the controller */
	char	l_lmsg[18];		/* contents of last response if A_ASYNC */
	unchar	l_actions[A_MAX];	/* current actions outstanding */
	unchar	l_MQSize;		/* output message queue size */
	unchar	l_reserved1;	/* for alignment */
};

/* fields used in the mb_bind field of the message */
#define BIND_ACTION(mx) (((char *)&(mx)->mb_bind)[0])
#define BIND_MINOR(mx) (((char *)&(mx)->mb_bind)[1])

/*
 *	The contents of atcs_info are initialized in the atcs space.c or
 *	the BPS.
*/
struct atcs_info {
	unsigned short slot;	/* Slot where server resides */
	dev_t beg_minor_num;	/* First minor number for server */
	dev_t end_minor_num;	/* Last minor number for server */
	int   beg_port_num;		/* First line to use */
};
#endif	/* _SYS_ATCS_H */
