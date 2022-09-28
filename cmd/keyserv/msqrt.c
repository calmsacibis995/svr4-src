/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:msqrt.c	1.1.2.1"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*	PROPRIETARY NOTICE (Combined)
*
* This source code is unpublished proprietary information
* constituting, or derived under license from AT&T's UNIX(r) System V.
* In addition, portions of such source code were derived from Berkeley
* 4.3 BSD under license from the Regents of the University of
* California.
*
*
*
*	Copyright Notice 
*
* Notice of copyright on this source code product does not indicate 
*  publication.
*
*	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
*	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
*          All rights reserved.
*/ 
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)msqrt.c 1.1 89/03/10 Copyr 1986 Sun Micro";
#endif
/* from UCB 5.1 4/30/85 */

/* LINTLIBRARY */

#include "mp.h"
msqrt(a, b, r) 
	MINT *a, *b, *r;
{
	MINT a0, x, junk, y;
	int j;

	a0.len = x.len = junk.len = y.len = 0;
	if (a->len < 0)
		_mp_fatal("msqrt: neg arg");
	if (a->len == 0) {
		b->len = 0;
		r->len = 0;
		return (0);
	}
	if (a->len % 2 == 1)
		x.len = (1 + a->len) / 2;
	else
		x.len = 1 + a->len / 2;
	x.val = xalloc(x.len, "msqrt");
	for (j = 0; j < x.len; x.val[j++] = 0);
	if (a->len % 2 == 1)
		x.val[x.len - 1] = 0400;
	else
		x.val[x.len - 1] = 1;
	_mp_move(a, &a0);
	xfree(b);
	xfree(r);
loop:
	mdiv(&a0, &x, &y, &junk);
	xfree(&junk);
	madd(&x, &y, &y);
	sdiv(&y, 2, &y, (short *) &j);
	if (mcmp(&x, &y) > 0) {
		xfree(&x);
		_mp_move(&y, &x);
		xfree(&y);
		goto loop;
	}
	xfree(&y);
	_mp_move(&x, b);
	mult(&x, &x, &x);
	msub(&a0, &x, r);
	xfree(&x);
	xfree(&a0);
	return (r->len);
}
