/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/t4014/scale.c	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

extern float scalex;
extern float scaley;
extern int scaleflag;
scale(i,x,y)
char i;
float x,y;
{
	switch(i) {
	default:
		return;
	case 'c':
		x *= 2.54;
		y *= 2.54;
	case 'i':
		x /= 200;
		y /= 200;
	case 'u':
		scalex = 1/x;
		scaley = 1/y;
	}
	scaleflag = 1;
}
