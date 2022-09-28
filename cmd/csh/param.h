/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:param.h	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * Despite its name, this file has nothing in particular to
 * do with <sys/param.h>.  It contains definitions that the
 * C version of printf needs.  (The Vax version doesn't need
 * this file, but instead uses an assembly language version
 * of _doprnt, with printf simply arranging to call this
 * version.)
 *
 * This business of the C shell having a private copy of
 * printf is a real pain and should be fixed.
 */

/* Maximum number of digits in any integer (long) representation */
#define	MAXDIGS	11

/* Convert a digit character to the corresponding number */
#define	tonumber(x)	((x)-'0')

/* Convert a number between 0 and 9 to the corresponding digit */
#define	todigit(x)	((x)+'0')

/* Data type for flags */
typedef	char	bool;

/* Maximum total number of digits in E format */
#define	MAXECVT	17

/* Maximum number of digits after decimal point in F format */
#define	MAXFCVT	60

/* Maximum significant figures in a floating-point number */
#define	MAXFSIG	17

/* Maximum number of characters in an exponent */
#define	MAXESIZ	4

/* Maximum (positive) exponent or greater */
#define	MAXEXP	40
