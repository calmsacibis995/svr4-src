/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.usr:deflt.h	1.1.8.1"
/*	Copyright (c) 1987, 1988 Microsoft Corporation	*/
/*	  All Rights Reserved	*/

/*	This Module contains Proprietary Information of Microsoft  */
/*	Corporation and should be treated as Confidential.	   */

/*
 *	@(#) deflt.h 1.1 86/10/07 
 */
/***	deflt.h -- include file for deflt(3).
 *
 */

#define	DEFLT	"/etc/default"

/*
 * Following for defcntl(3).
 * If you add new args, make sure that the default is:
 *	OFF	new-improved-feature-off, i.e. current state of affairs
 *	ON	new-improved-feature-on
 * or that you change the code for deflt(3) to have the old value as the
 * default.  (for compatibility).
 */

/* ... cmds */
#define	DC_GETFLAGS	0	/* get current flags */
#define	DC_SETFLAGS	1	/* set flags */

/* ... args */
#define	DC_CASE		0001	/* ON: respect case; OFF: ignore case */

#define	DC_STD		((0) | (DC_CASE))

extern char *defread();

#define	TURNON(flags, mask)	flags |= mask
#define	TURNOFF(flags, mask)	flags &= ~(mask)
#define	ISON(flags, mask)	(((flags) & (mask)) == (mask))
#define	ISOFF(flags, mask)	(((flags) & (mask)) != (mask))
