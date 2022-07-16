/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/aed/space.c	1.1.3.1"

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
 *	Space sets up the world-to-screen transformation so
 *	that the rectangular area described by (x0, y0) and
 *	(x1, y1) will all be on-screen.
 *
 *	Results:	None.
 *
 *	Side Effects:
 *	Our own variables scale, xbot, and ybot are changed.
 *---------------------------------------------------------
 */
space(x0, y0, x1, y1)
int x0, y0, x1, y1;
{
    int xscale, yscale, xsize, ysize;
    xscale = (GRXMAX<<12)/(x1-x0);
    yscale = (GRYMAX<<12)/(y1-y0);
    if (xscale > yscale) scale = yscale;
    else scale = xscale;
    scale = (scale*9)/10 - 1;
    if (scale<1) scale = 1;
    xsize = (2048*GRXMAX)/scale + 1;
    xbot = (x1+x0)/2 - xsize;
    ysize = (2048*GRYMAX)/scale + 1;
    ybot = (y1+y0)/2 - ysize;
}
