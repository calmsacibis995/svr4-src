/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)keyserv:gcd.c	1.1.2.1"

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
static char sccsid[] = "@(#)gcd.c 1.1 89/03/10 Copyr 1986 Sun Micro";
#endif
/* from UCB 5.2 3/13/18 */

/* LINTLIBRARY */

#include "mp.h"

gcd(a,b,c) 
	MINT *a,*b,*c;
{	
	MINT x,y,z,w;

	x.len = y.len = z.len = w.len = 0;
	_mp_move(a,&x);
	_mp_move(b,&y);
	while (y.len != 0) {	
		mdiv(&x,&y,&w,&z);
		_mp_move(&y,&x);
		_mp_move(&z,&y);
	}
	_mp_move(&x,c);
	xfree(&x);
	xfree(&y);
	xfree(&z);
	xfree(&w);
}




invert(x1, x0, c)
	MINT *x1;
	MINT *x0;
	MINT *c;
{	
	MINT u2, u3;
	MINT v2, v3;
	MINT zero;
	MINT q, r;
	MINT t;
	MINT x0_prime;
	static MINT *one = (MINT *)0;

	/* 
	 * Minimize calls to allocators.  Don't use pointers for local
	 * variables, for the one "initialized" multiple precision 
	 * variable, do it just once.
	 */
	if (one == (MINT *)0)
		one = itom((short)1);

	zero.len = q.len = r.len = t.len = 0;

	x0_prime.len = u2.len = u3.len = 0;
	_mp_move(x0, &u3);
	_mp_move(x0, &x0_prime);

	v2.len = v3.len = 0;
	_mp_move(one, &v2);
	_mp_move(x1, &v3);

	while (mcmp(&v3,&zero) != 0) {
		/* invariant: x0*u1 + x1*u2 = u3 */
		/* invariant: x0*v1 + x2*v2 = v3 */
		/* invariant: x(n+1) = x(n-1) % x(n) */
		mdiv(&u3,&v3,&q,&r);
		_mp_move(&v3,&u3);
		_mp_move(&r,&v3);

		mult(&q,&v2,&t);
		msub(&u2,&t,&t);
		_mp_move(&v2,&u2);
		_mp_move(&t,&v2);
	}
	/* now x0*u1 + x1*u2 == 1, therefore,  (u2*x1) % x0  == 1 */
	_mp_move(&u2,c);
	if (mcmp(c,&zero) < 0) {
		madd(&x0_prime, c, c);
	}
	xfree(&zero);
	xfree(&v2);
	xfree(&v3);
	xfree(&u2);
	xfree(&u3);
	xfree(&q);
	xfree(&r);
	xfree(&t);
}


