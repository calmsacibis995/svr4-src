/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988 Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/sys/iasy.h	1.3.1.1"

/*
 *	Definitions for generic async support
*/
#ifndef T_CONNECT
#define T_CONNECT 42	/* Augment tty.h */
#endif

#define	SPL	splstr

/*
 *	This is used to remember where the interrupt-time code is for
 *	each async line.
*/
struct iasy_hw {
	int  (*proc)();		/* proc routine does most operations */
	void (*hwdep)();	/* Called as last resort for unknown ioctls */
};

#define	L_BUF		0
#define	L_BREAK		3
