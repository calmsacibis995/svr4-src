/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/erf.c	1.4"
/*LINTLIBRARY*/

/*
 *	erf returns the error function of its double-precision argument.
 *	erfc(x) returns 1 - erf(x).
 *
 *	erf(x) is defined by
 *	${2 over sqrt pi} int from 0 to x e sup {- t sup 2} dt$.
 *
 *	The entry for erfc is provided because of the
 *	extreme loss of relative accuracy if erf(x) is
 *	called for large x and the result subtracted
 *	from 1 (e.g. for x = 5, 12 places are lost).
 */

#ifdef __STDC__
	#pragma weak erf = _erf
	#pragma weak erfc = _erfc
#endif

#include "synonyms.h"
#include <math.h>

static double	
/* tiny must convert exactly right; jam it in hex if it doesn't   */
tiny   = 2.225073858507201383E-308,  /* 2^-1022 */
small  = 2.77555756156289135106E-17,   /* 2^-55 */
medium = 5.9375,
large  = 27.25,
torp   = 1.12837916709551257839616;   /*  2/sqrt(pi)  */

/* Coefficients #0006 from Robert Morris paper; 17.0 digit accuracy.      */
static double
A0 = 3.7485478058818868930320E+2,
A1 = 1.5913837767775692188824E+1,   	
A2 = 1.0732796627991997342982E+1,   	
A3 = -4.4434564385918256074083E-1,
A4 = 2.5619892703185897445915E-2;
static double
B0 = 3.3220639969193865595272E+2,
B1 = 1.2483873808107114329804E+2,
B2 = 1.7903966081480707266105E+1;

/* Coefficients #0010 from Robert Morris paper; 17.1 digit accuracy.      */
static double 
P0 = 2.25390631503290875460748E+3,
P1 = 4.53624059180908409310079E+3,  	
P2 = 4.51446861966548949058326E+3,  	
P3 = 2.82508628365117088133287E+3,  	
P4 = 1.20347712108046029234728E+3,  	
P5 = 3.56344790133815926013389E+2,  	
P6 = 7.18167275099773648435685E+1,  	
P7 = 9.08782044132044197499316E+0,  	
P8 = 5.64189583799855010110876E-1;
static double
Q0 = 2.25390631491569922714853E+3,
Q1 = 7.07950152396136880872480E+3,
Q2 = 1.02489243245808802278410E+4,
Q3 = 9.00576480008025407017372E+3,
Q4 = 5.31509151575154402517653E+3,
Q5 = 2.19625295849473184616639E+3,
Q6 = 6.39658602461599387276899E+2,
Q7 = 1.27791833775121888864583E+2,
Q8 =1.61077423748281328083836E+1;

/* Coefficients #0030 from Robert Morris paper; 16.7 digit accuracy.      */
static double	
R0 =  4.7434228095675782026852E-2,   	S0 =  8.4074980252911164750820E-2,
R1 =  4.5836740975373824550730E-1,   	S1 =  8.5447257067402741867692E-1,
R2 =  9.6082816377612127197741E-1,   	S2 =  2.0672036291373934816958E+0,
R3 =  2.5366643779890533396123E-1;



double	erf(x)
register double	x;
{
	register double xsq;
	double	num, denom;
	int sign = 0;

	if (x == 0.0) 
		return (x);
	if (x < 0.0) {
		sign = 1;
		x = -x;
	}
	if (x < small) {
		if ((x = x * torp) < tiny )
			x = x + tiny * tiny;
		else
			x = x * (1.0 - tiny);
	}
	else if (x < 0.5) {
		xsq = x * x;
		num = (((A4 * xsq + A3) * xsq + A2) * xsq + A1) * xsq + A0;
		denom = ((xsq + B2) * xsq + B1) * xsq + B0;
		x = num / denom * x;
	}
	else if (x < medium) 
		x = (1.0 - erfc(x));
	else x = 1.0;
	return (sign ? -x : x);
}

double	erfc(x)
register double	x;
{
	register double xsq;
	double	xhi, xlo, num, denom;
	int neg = 0;

	if (x < -medium) 
		return 2.0;
	if (x <= -0.5) { 
		x = -x;
		neg = 1;
	}
	if (_ABS(x) < small) 
		return (1.0 - x); /* exact if and only if x = 0 */
	if (x < 0.5) 
		return (1.0 - erf(x));
	if (x < 10.0) {
		num = (((((((P8 * x + P7) * x + P6) * x + P5) * x + P4) * x
			+ P3) * x + P2) * x + P1) * x + P0;
		denom = ((((((((x + Q8) * x + Q7) * x + Q6) * x + Q5) * x +
			Q4) * x + Q3) * x + Q2) * x + Q1) * x + Q0;
		/* Rounding error in x*x loses too much accuracy 
		 * in value from exp(-x*x), so we must break it
		 * up into two parts to preserve accuracy.
		 */
		xlo = modf(x * 64.0, &xhi);
		xlo = xlo / 64.0; 
		xhi = xhi / 64.0;
		x = (num / denom * exp(-xlo * (x + xhi)) * exp(-xhi * xhi));
		return (neg ? 2.0 - x : x);
	}
	if (x < large) {
		xsq = x * x;
		num = ((R3 / xsq + R2) / xsq + R1) / xsq + R0;
		denom = ((1.0 / xsq + S2) / xsq + S1) / xsq + S0;
		xlo = modf(x * 64.0, &xhi);
		xlo = xlo / 64.0; 
		xhi = xhi / 64.0;
		return (num / denom / x * exp(-xlo * (x + xhi)) * exp(-xhi *
		    xhi));
	}
	return 0.0;
}
