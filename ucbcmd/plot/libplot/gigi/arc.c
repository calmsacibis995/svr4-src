/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/gigi/arc.c	1.1.3.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


#include "gigi.h"

/* 
 * gigi requires knowing the anlge of arc.  To do this, the triangle formula
 *	c^2 = a^2 + b^2 - 2*a*b*cos(angle)
 * is used where "a" and "b" are the radius of the circle and "c" is the
 * distance between the beginning point and the end point.
 *
 * This gives us "angle" or angle - 180.  To find out which, draw a line from
 * beg to center.  This splits the plane in half.  All points on one side of the
 * plane will have the same sign when plugged into the equation for the line.
 * Pick a point on the "right side" of the line (see program below).  If "end"
 * has the same sign as this point does, then they are both on the same side
 * of the line and so angle is < 180.  Otherwise, angle > 180.
 */
   
#define side(x,y)	(a*(x)+b*(y)+c > 0.0 ? 1 : -1)

arc(xcent,ycent,xbeg,ybeg,xend,yend)
int xcent,ycent,xbeg,ybeg,xend,yend;
{
	double radius2, c2;
	double a,b,c;
	int angle;

	/* Probably should check that this is really a circular arc.  */
	radius2 = (xcent-xbeg)*(xcent-xbeg) + (ycent-ybeg)*(ycent-ybeg);
	c2 = (xend-xbeg)*(xend-xbeg) + (yend-ybeg)*(yend-ybeg);
	angle = (int) ( 180.0/PI * acos(1.0 - c2/(2.0*radius2)) + 0.5 );

	a = (double) (ycent - ybeg);
	b = (double) (xcent - xbeg);
	c = (double) (ycent*xbeg - xcent*ybeg);
	if (side(xbeg + (ycent-ybeg), ybeg - (xcent-xbeg)) != side(xend,yend))
		angle += 180;
	
	move(xcent, ycent);
	printf("C(A%d c)[%d,%d]", angle, xbeg, ybeg);
}
