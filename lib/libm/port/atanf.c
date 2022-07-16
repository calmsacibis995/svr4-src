/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/atanf.c	1.5"
/*LINTLIBRARY*/
/* 
 * 
 */


/* atanf returns the arctangent of its single-precision argument,
 * in the range [-pi/2, pi/2].
 * There are no error returns.
 * atan2f(y, x) returns the arctangent of y/x,
 * in the range (-pi, pi].
 * atan2f discovers what quadrant the angle is in and calls atan.
 * atan2f returns EDOM error and value 0 if both arguments are zero.
 */

/* atanf(x)
 * Method :
 *	1. Reduce x to positive by atan(x)=-atan(-x).
 *	2. According to the integer k=4x+0.0625 truncated ,  the argument 
 *	   is further reduced to one of the following intervals and the 
 *	   arctangent of x is evaluated by the corresponding formula:
 *
 *         [0,7/16]	   atan(x) = x - x^3*(a1+x^2*(a2+...(a10+x^2*a11)...)
 *	   [7/16,11/16]    atan(x) = atan(1/2) + atan( (x-1/2)/(1+x/2) )
 *	   [11/16.19/16]   atan(x) = atan(1) + atan( (x-1)/(1+x) )
 *	   [19/16,39/16]   atan(x) = atan(3/2) + atan( (x-1.5)/(1.5x) )
 *	   [39/16,INF]     atan(x) = atan(INF) + atan( -1/x )
 */

#include "synonyms.h"
#include <errno.h>
#include <math.h>
#include <values.h>
#include "fpparts.h"

static double  /* use double precision here to maintain accuracy */
	athfhi =  4.6364760900080609352E-1    , 
	athflo =  4.6249969567426939759E-18   , 
	at1fhi =  9.8279372324732905408E-1    , 
	at1flo = -2.4407677060164810007E-17   ;
static float 
	a[] = {
		 -5.8358371008508623523E-2    , 
		  6.6614695906082474486E-2    , 
		 -7.6919217767468239799E-2    , 
		  9.0908906105474668324E-2    , 
		 -1.1111110579344973814E-1    , 
		  1.4285714278004377209E-1    , 
		 -1.9999999999979536924E-1    , 
		  3.3333333333333942106E-1    , 
	};

static float one = 1.0, zero = 0.0, two = 2.0;

/* special value of PI, 1 ulp less than M_PI rounded to float
 * to satisfy range requirements - (float)M_PI is actually greater
 * than true pi
 */
#define F_PI (float)3.1415925



float atanf(register float x)
{  
	static float big = 1.0E8;
	register float z; 
	register double tmp;
	double hi, lo;
	register int k, signx = 0;
	
	if (x < zero) {
		signx = 1;
		x = -x;
	}
	if (x < (float)2.4375) {		 
		/* truncate 4(x+1/16) to integer for branching */
		k = 4 * (x + (float)0.0625);
		switch (k) {
		/* x is in [0,7/16] */
		case 0:                    
		case 1:
			if (x < FX_EPS) 
				return(signx ? -x : x);
			hi = 0.0;
			lo = 0.0;
			break;
		    /* x is in [7/16,11/16] */
		case 2:                    
			hi = athfhi; 
			lo = athflo;
			x = ((x + x) - one ) / ( two +  x );
			break;
		    /* x is in [11/16,19/16] */
		case 3:                    
		case 4:
			hi = M_PI_4; 
			lo = 0.0;
			x = ( x - one ) / ( x + one );
			break;
		    /* x is in [19/16,39/16] */
		default:                   
			hi = at1fhi;
			lo = at1flo;
			z = x - one;
			x = x + x + x;
			x = ((z + z) - one ) / ( two + x ); 
			break;
		} /* switch */
	}
	/* end of if (x < 2.4375) */
	else {                    
		if (x <= big) {
		/* x is in [2.4375, big] */
			x = - one / x;
			hi = M_PI_2;
			lo = 0.0;
		}
		/* x is in [big, INF] */
	else          
		return(signx ? -(float)M_PI_2 : (float)M_PI_2);
	}
    /* end of argument reduction */

    /* compute atan(x) for x in [-.4375, .4375] */
	z = x * x;
	z = x * z * _POLY7(z, a);
	tmp = lo - z; /* done in double precision for accuracy */
	tmp += x; 
	tmp += hi;
	return(signx ? -(float)tmp : (float)tmp);
}

float
atan2f(register float y, register float x)
{
	register int neg_y = 0;
	struct exception exc;
	float tmp, at;

	if (!x && !y) {
		exc.type = DOMAIN;
		exc.name = "atan2f";
		exc.arg1 = (double)y;
		exc.arg2 = (double)x;
		exc.retval = 0.0;
		if (_lib_version == strict_ansi)
			errno = EDOM;
		else if (!matherr(&exc)) {
			if (_lib_version == c_issue_4)
				(void) write(2, "atan2f: DOMAIN error\n", 21);
			errno = EDOM;
		}
		return (float)exc.retval;
	}
	/*
	 * The next lines determine if |x| is negligible compared to |y|,
	 * without dividing, and without adding values of the same sign.
	 */
	if (y < 0) {
		tmp = -y;
		neg_y++;
	}
	else tmp = y;
	if (tmp - _ABS(x) == tmp) 
		return (neg_y ? -(float)M_PI_2 : (float)M_PI_2);
	/*
	 * The next line assumes that if y/x underflows the result
	 * is zero with no error indication, so it's safe to divide.
	 */
	at = atanf(y/x);
	if (x > zero) 
		return at;
	if (!at)
		return F_PI;	    /* special value of PI, 1 ulp
				     * less than M_PI rounded to float
				     * to satisfy range requirements - 
				     * (float)M_PI is actually greater
				     * than true pi
				     */
	if (neg_y)		    /* x < 0, adjust arctangent for */
		return (at - (float)M_PI);        /* correct quadrant */
	else
		return (at + (float)M_PI);
}
