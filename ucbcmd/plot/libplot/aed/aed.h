/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/aed/aed.h	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Displays plot files on an AED512 graphics terminal.
 */

#include <stdio.h>
#include <sgtty.h>

extern char dbuf[BUFSIZ];	/* Used to buffer display characters */
extern struct sgttyb sgttyb;	/* Used to save terminal control bits */
extern curx, cury;		/* Current screen position */
extern int xbot, ybot;		/* Coordinates of screen lower-left corner */
extern int scale;		/* The number of pixels per 2**12 units
				 * of world coordinates.
				 */

/* The following variables describe the screen. */

#define GRXMAX	511	/* Maximum x-coordinate of screen */
#define GRYMAX	482	/* Maximum y-coordinate of screen */
