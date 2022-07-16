/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libmp:madd.c	1.1.1.1"

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
static char sccsid[] = "@(#)madd.c 1.1 89/03/10 Copyr 1986 Sun Micro";
#endif
#ifndef lint
/* from UCB 5.1 4/30/85 */
#endif

/* LINTLIBRARY */

#include <rpc/mp.h>

m_add(a,b,c) 
	register struct mint *a,*b,*c;
{	
	register int carry,i;
	register int x;
	register short *cval;

	cval = xalloc(a->len + 1,"m_add");
	carry = 0;
	for (i = 0; i < b->len; i++) {	
		x = carry + a->val[i] + b->val[i];
		if ( x & 0100000) {	
			carry = 1;
			cval[i] = x & 077777;
		} else {	
			carry = 0;
			cval[i] = x;
		}
	}
	for (; i < a->len; i++) {	
		x = carry + a->val[i];
		if (x & 0100000) {
			cval[i] = x & 077777;
		} else {	
			carry = 0;
			cval[i] = x;
		}
	}
	if (carry == 1) {	
		cval[i] = 1;
		c->len = i + 1;
	} else {
		c->len = a->len;
	}
	c->val = cval;
	if (c->len == 0) {
		free((char *) cval);
	}
}



madd(a,b,c) 
	register struct mint *a,*b,*c;
{	

	struct mint x,y;
	int sign;

	x.len = y.len = 0;
	_mp_move(a, &x);
	_mp_move(b, &y);
	xfree(c);
	sign = 1;
	if (x.len >= 0) {
		if(y.len >= 0) {
			if (x.len >= y.len) {
				m_add(&x,&y,c);
			} else {
				m_add(&y,&x,c);
			}
		} else {	
			y.len = -y.len;
			msub(&x,&y,c);
		} 
	} else {
		if (y.len <= 0) {	
			x.len = -x.len;
			y.len = -y.len;
			sign = -1;
			madd(&x,&y,c);
		} else {	
			x.len = -x.len;
			msub(&y,&x,c);
		}
	}
	c->len = sign * c->len;
	xfree(&x);
	xfree(&y);
}



m_sub(a,b,c) 
	register struct mint *a,*b,*c;
{	
	register int x,i;
	register int borrow;
	short one;
	struct mint mone;

	one = 1; 
	mone.len = 1; 
	mone.val = &one;
	c->val = xalloc(a->len,"m_sub");
	borrow = 0;
	for (i = 0; i < b->len; i++) {	
		x = borrow + a->val[i] - b->val[i];
		if (x & 0100000) {	
			borrow = -1;
			c->val[i] = x & 077777;
		} else {	
			borrow = 0;
			c->val[i] = x;
		}
	}
	for(; i < a->len; i++) {
		x = borrow + a->val[i];
		if (x & 0100000) {
			c->val[i] = x & 077777;
		} else {	
			borrow = 0;
			c->val[i] = x;
		}
	}
	if (borrow < 0) {	
		for (i = 0; i < a->len; i++) {
			c->val[i] ^= 077777;
		}
		c->len = a->len;
		madd(c,&mone,c);
	}
	for(i = a->len-1; i >= 0; --i) {
		if (c->val[i] > 0) {	
			if (borrow == 0) {
				c->len = i + 1;
			} else {
				c->len= -i - 1;
			}
			return;
		}
	}
	free((char *) c->val);
}


msub(a,b,c) 
	register struct mint *a,*b,*c;
{	
	struct mint x,y;
	int sign;

	x.len = y.len = 0;
	_mp_move(a, &x);
	_mp_move(b, &y);
	xfree(c);
	sign = 1;
	if (x.len >= 0) {
		if (y.len >= 0) {
			if (x.len >= y.len) {
				m_sub(&x,&y,c);
			} else {	
				sign = -1;
				msub(&y,&x,c);
			}
		} else {	
			y.len = -y.len;
			madd(&x,&y,c);
		}
	} else {
		if (y.len <= 0) {	
			x.len = -x.len;
			y.len = -y.len;
			msub(&y,&x,c);
		} else {	
			x.len = -x.len;
			madd(&x,&y,c);
			sign = -1;
		}
	}
	c->len = sign * c->len;
	xfree(&x);
	xfree(&y);
}


