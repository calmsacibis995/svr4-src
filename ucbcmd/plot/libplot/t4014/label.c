/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/t4014/label.c	1.1.3.1"

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */

#define N 0104
#define E 0101
#define NE 0105
#define S 0110
#define W 0102
#define SW 0112
/*	arrange by incremental plotting that an initial
 *	character such as +, X, *, etc will fall
 *	right on the point, and undo it so that further
 *	labels will fall properly in place
 */
char lbl_mv[] = {
	036,040,S,S,S,S,S,S,SW,SW,SW,SW,SW,SW,SW,SW,SW,SW,037,0
};
char lbl_umv[] = {
	036,040,N,N,N,N,N,N,NE,NE,NE,NE,NE,NE,NE,NE,NE,NE,037,0
};
label(s)
char *s;
{
	register i,c;
	for(i=0; c=lbl_mv[i]; i++)
		putch(c);
	for(i=0; c=s[i]; i++)
		putch(c);
	for(i=0; c=lbl_umv[i]; i++)
		putch(c);
}
