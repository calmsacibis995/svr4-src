/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/vt0/dot.c	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

extern vti;
dot(xi,yi,dx,n,pat)
int pat[];
{
	struct {char pad,c; int xi,yi,dx;} p;
	p.c = 7;
	p.xi = xsc(xi);
	p.yi = ysc(yi);
	p.dx = xsc(dx);
	write(vti,&p.c,7);
	write(vti,pat,n?n&0377:256);
}
