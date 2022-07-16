/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */
/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)ucblibmp:mdiv.c	1.2.3.1"

/* LINTLIBRARY */

#include <mp.h>
#include <stdio.h>

mdiv(a, b, q, r) 
	MINT *a, *b, *q, *r;
{
	MINT x, y;
	int sign;

	sign = 1;
	x.len = y.len = 0;
	_mp_move(a, &x);
	_mp_move(b, &y);
	if (x.len < 0) {
		sign = -1;
		x.len = -x.len;
	}
	if (y.len < 0) {
		sign = -sign;
		y.len = -y.len;
	}
	xfree(q);
	xfree(r);
	m_div(&x, &y, q, r);
	if (sign == -1) {
		q->len = -q->len;
		r->len = -r->len;
	}
	xfree(&x);
	xfree(&y);
}


m_dsb (qx, n, a, b) 
	register short qx;
	int n;
	short *a, *b;
{
	register long borrow;
	register long s3b2shit;
	register int j;
	register short fifteen = 15; 
	register short *aptr, *bptr;
#ifdef DEBUGDSB
	printf("m_dsb %d %d %d %d\n",qx,n,*a,*b);
#endif

	borrow = 0;
	aptr = a;
	bptr = b;
	for (j = n; j > 0; j--) {
#ifdef DEBUGDSB
		printf("1 borrow=%x %d %d %d\n",borrow, (*aptr *qx),*bptr, *aptr);
#endif
		borrow -= (*aptr++) * qx - *bptr;
#ifdef DEBUGDSB
		printf("2 borrow=%x %d %d %d\n",borrow, (*aptr *qx),*bptr, *aptr);
#endif
		*bptr++ = borrow & 077777;
#ifdef DEBUGDSB
		printf("3 borrow=%x %d %d %d\n",borrow, (*aptr *qx),*bptr, *aptr);
#endif
		if (borrow>=0) borrow >>= fifteen; /*3b2*/
		else borrow= 0xfffe0000 | (borrow >>fifteen);
#ifdef DEBUGDSB
		printf("4 borrow=%x %d %d %d\n",borrow, (*aptr *qx),*bptr, *aptr);
#endif
	}
	borrow += *bptr;
	*bptr = borrow & 077777;
		if (borrow>=0) s3b2shit= borrow >> fifteen; /*3b2*/
		else s3b2shit= 0xfffe0000 | (borrow >>fifteen);
	if (s3b2shit == 0) {
#ifdef DEBUGDSB
	printf("mdsb 0\n");
#endif
		return(0);
	}
	borrow = 0;
	aptr = a;
	bptr = b;
	for (j = n; j > 0; j--) {
		borrow += *aptr++ + *bptr;
		*bptr++ = borrow & 077777;
		if (borrow>=0) borrow >>= fifteen; /*3b2*/
		else borrow= 0xfffe0000 | (borrow >>fifteen);
	}
#ifdef DEBUGDSB
	printf("mdsb 1\n");
#endif
	return(1);
}



m_trq (v1, v2, u1, u2, u3) 
	register short v1;
	register short v2;
	short u1;
	short u2;
	short u3;
{
	register short d;
	register long x1;
	register long c1;

	c1 = u1 * 0100000 + u2;
	if (u1 == v1) {
		d = 077777;
	} else {
		d = c1 / v1;
	}
	do {	
		x1 = c1 - v1 * d;
		x1 = x1 * 0100000 + u3 - v2 * d;
		 --d;
	} while (x1 < 0);
#ifdef DEBUGMTRQ
	printf ("mtrq %d %d %d %d %d %d\n",v1, v2, u1, u2, u3, (d+1)) ;
#endif
	return(d + 1);
}


m_div (a, b, q, r) 
	MINT *a, *b, *q, *r;
{
	MINT u, v, x, w;
	short d;
	register short *qval;
	register short *uval;
	register int j;
	register int qq;
	register int n;
	register int v1;
	register int v2;

	u.len = v.len = x.len = w.len = 0;
	if (b->len == 0) {
		_mp_fatal("mdiv divide by zero");
		return;
	}
	if (b->len == 1) {
		r->val = xalloc(1, "m_div1");
		sdiv (a, b->val[0], q, r->val);
		if (r->val[0] == 0) {
			free ((char *) r->val);
			r->len = 0;
		} else {
			r->len = 1;
		}
		return;
	}
	if (a -> len < b -> len) {
		q->len = 0;
		r->len = a->len;
		r->val = xalloc(r->len, "m_div2");
		for (qq = 0; qq < r->len; qq++) {
			r->val[qq] = a->val[qq];
		}
		return;
	}
	x.len = 1;
	x.val = &d;
	n = b->len;
	d = 0100000L / (b->val[n - 1] + 1L);
	mult(a, &x, &u);	/* subtle: relies on mult allocing extra space */
	mult(b, &x, &v);
#ifdef DEBUG_MDIV
	printf("  u=%s\n",mtox(&u));
	printf("  v=%s\n",mtox(&v));
#endif
	v1 = v.val[n - 1];
	v2 = v.val[n - 2];
	qval = xalloc(a -> len - n + 1, "m_div3");
	uval = u.val;
	for (j = a->len - n; j >= 0; j--) {
		qq = m_trq(v1, v2, uval[j + n], uval[j + n - 1], uval[j + n - 2]);
		if (m_dsb(qq, n, v.val, uval + j))
			qq -= 1;
		qval[j] = qq;
	}
	x.len = n;
	x.val = u.val;
	_mp_mcan(&x);
#ifdef DEBUG_MDIV
	printf("  x=%s\n",mtox(&x));
	printf("  d(in)=%d\n",(d));
#endif
	sdiv(&x, d, &w, &d);
#ifdef DEBUG_MDIV
	printf("  w=%s\n",mtox(&w));
	printf("  d(out)=%d\n",(d));
#endif
	r->len = w.len;
	r->val = w.val;
	q->val = qval;
	qq = a->len - n + 1;
	if (qq > 0 && qval[qq - 1] == 0)
		qq -= 1;
	q->len = qq;
	if (qq == 0)
		free ((char *) qval);
	if (x.len != 0)
		xfree (&u);
	xfree (&v);
}
