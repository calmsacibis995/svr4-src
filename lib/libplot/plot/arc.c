/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libplot:plot/arc.c	1.3"
#include <stdio.h>
arc(xi,yi,x0,y0,x1,y1){
	putc('a',stdout);
	putsi(xi);
	putsi(yi);
	putsi(x0);
	putsi(y0);
	putsi(x1);
	putsi(y1);
}
