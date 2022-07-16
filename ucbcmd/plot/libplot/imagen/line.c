/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/imagen/line.c	1.1.3.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "imp.h"
#include "imPcodes.h"
float obotx = 0.;
float oboty = 0.;
float botx = 2.;
float boty = 2.;
float scalex = 1.;
float scaley = 1.;
line(x0,y0,x1,y1)
{
	putch(imP_CREATE_PATH);
	putwd(2);		/* two coordinates follow */
	putwd((int)((x0-obotx)*scalex+botx));	
	putwd((int)((y0-oboty)*scaley+boty));	
	putwd((int)((x1-obotx)*scalex+botx));	
	putwd((int)((y1-oboty)*scaley+boty));	
	putch(imP_DRAW_PATH);
	putch(15);		/* "black" lines */
	imPx = x1;
	imPy = y1;
}
putch(c)
{
	putc(c, stdout);
}
putwd(w)
{
	putch(w>>8);
	putch(w);
}
