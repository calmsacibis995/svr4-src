/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/aed/linemod.c	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */


#include "aed.h"

/*---------------------------------------------------------
 *	Linemod sets the current line drawing style.
 *
 *	Results:	None.
 *
 *	Side Effects:
 *	The AED line style is set based on string s which
 *	must be one of "dotted", "solid", "longdashed", "shortdashed",
 *	or "dotdashed".  If s isn't recognized, then "solid" is used.
 *---------------------------------------------------------
 */
linemod(s)
char *s;
{
    if (strcmp(s, "dotted") == 0)
	fputs("1AAFF", stdout);
    else if (strcmp(s, "longdashed") == 0)
	fputs("1F055", stdout);
    else if (strcmp(s, "shortdashed") == 0)
	fputs("1F0FF", stdout);
    else if (strcmp(s, "dotdashed") == 0)
	fputs("1E4FF", stdout);
    else fputs("1FFFF", stdout);
    (void) fflush(stdout);
}
