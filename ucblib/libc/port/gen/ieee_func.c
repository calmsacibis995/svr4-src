/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucblibc:port/gen/ieee_func.c	1.1.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/* IEEE functions
 *	copysign()
 *	isnan()
 *	scalbn()
 */

#include <math.h>
#include "libm.h"

#define divbyz (1<<(int)fp_division)
#define unflow (1<<(int)fp_underflow)
#define ovflow (1<<(int)fp_overflow)
#define iexact (1<<(int)fp_inexact)
#define ivalid (1<<(int)fp_invalid)

static double setexception();
extern _swapTE(),_swapEX();
extern enum fp_direction_type _swapRD();

double copysign(x,y)
double x,y;
{
static	double	_m_	= 1.0;
static	int	n0, n1;

	long *px = (long *) &x;
	long *py = (long *) &y;

	if ((* (int *) &_m_) != 0) { n0 = 0; n1 = 1; }	/* not a i386 */
	else { n0 = 1; n1 = 0; }			/* is a i386  */

	px[n0] = (px[n0]&0x7fffffff)|(py[n0]&0x80000000);
	return x;
}

int isnan(x)
double x;
{
static	double	_m_	= 1.0;
static	int	n0, n1;

	long *px = (long *) &x;

	if ((* (int *) &_m_) != 0) { n0 = 0; n1 = 1; } /* not a i386 */
	else { n0 = 1; n1 = 0; }                       /* is a i386  */

	if((px[n0]&0x7ff00000)!=0x7ff00000) return FALSE;
	else return ((px[n0]&0x000fffff)|px[n1]) != 0;
}

double scalbn(x,n)
double x; int n;
{
static	double	_m_	= 1.0;
static	int	n0, n1;

	long *px = (long *) &x, k;
	double twom54=twom52*0.25;

	if ((* (int *) &_m_) != 0) { n0 = 0; n1 = 1; } /* not a i386 */
	else { n0 = 1; n1 = 0; }                       /* is a i386  */

	k = (px[n0]&0x7ff00000)>>20;
	if (k==0x7ff) return x+x;
	if ((px[n1]|(px[n0]&0x7fffffff))==0) return x;
	if (k==0) {x *= two52; k = ((px[n0]&0x7ff00000)>>20) - 52;}
	k = k+n;
	if (n >  5000) return setexception(2,x);
	if (n < -5000) return setexception(1,x);
	if (k >  0x7fe ) return setexception(2,x);
	if (k <= -54   ) return setexception(1,x);
	if (k > 0) {
		px[n0] = (px[n0]&0x800fffff)|(k<<20);
		return x;
	}
	k += 54;
	px[n0] = (px[n0]&0x800fffff)|(k<<20);
	return x*twom54;
}

static double setexception(n,x)
int n; double x;
{
    /* n = 1     --- underflow
	 = 2     --- overflow
     */
	int te,ex,k;
	enum fp_direction_type rd;
	te = _swapTE(0); if(te!=0) _swapTE(te);
	rd = _swapRD(fp_nearest); if(rd!=fp_nearest) _swapRD(rd);
	switch(n) {
	    case 1:			/* underflow */
		if((te&unflow)==0&&rd==fp_nearest) 
		    {ex= _swapEX(0); _swapEX(ex|unflow|iexact); 
		     return copysign(0.0,x);}
		else return fmin*copysign(fmin,x);
	    case 2:			/* overflow  */
		if((te&ovflow)==0&&rd==fp_nearest) 
		    {ex= _swapEX(0); _swapEX(ex|ovflow|iexact); 
		     return copysign(Inf,x);}
		else return fmax*copysign(fmax,x);
	}
}
