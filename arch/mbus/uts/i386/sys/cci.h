/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CCI_H
#define _SYS_CCI_H
/*	Copyright (c) 1986, 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/cci.h	1.3"

/* maximum number of control points that can be driven by CCI */
#define MAXCCISERVERS 21

struct cci_server {
	long	c_chan;			/* the open message passing channel for this line */
	int		c_index;		/* index of this control point */
	mb2socid_t c_HisSocket;	/* his socket connection */
	mb2socid_t c_MySocket;	/* my socket connection */
	int		c_lines;		/* the number of lines in this control point */
	char	c_lmsg[18];		/* contents of last message if A_ASYNC */
};

/*
 *	Interrupt action codes:
 *		When a message is sent out, this code gives the action to take
 *		when the response is received.  Kept in BIND_ACTION().
 */ 
#define A_ODD		0	/* won't happen */
#define A_ASYNC		1	/* async command, don't process response */
#define A_SYNC		2	/* sync command, wakeup persone waiting */
#define A_MAX		3

/* fields used in the mb_bind field of the message */
#define BIND_ACTION(mx) (((char *)&(mx)->mb_bind)[0])
#define BIND_CP(mx) (((char *)&(mx)->mb_bind)[1])

#endif	/* _SYS_CCI_H */
