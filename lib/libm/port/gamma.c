/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/gamma.c	1.7"
/*LINTLIBRARY*/
/*
  This routine calculates the LOG(GAMMA) function for a real argument
      X.  Computation is based on an algorithm outlined in references
      1 and 2.  The program uses rational functions that approximate
      LOG(GAMMA) to at least 18 significant decimal digits.  The
      approximation for X >= 12 is from reference 3.  Approximations
      for X <= 12.0 are unpublished.  Lower order approximations can
      be substituted on machines with less precise arithmetic.
 **************************************************************
 
  Explanation of machine-dependent constants
 
  XBIG   - largest argument for which LN(GAMMA(X)) is representable
           in the machine, i.e., the solution to the equation
                   LN(GAMMA(XBIG)) = beta**maxexp
  EPS    - The smallest positive floating-point number such that
           1.0+EPS .GT. 1.0
  FRTBIG - Rough estimate of the fourth root of XBIG
 
 **************************************************************
 
  References:
 
   1) W. J. Cody and K. E. Hillstrom, 'Chebyshev Approximations for
      the Natural Logarithm of the Gamma Function,' Math. Comp. 21,
      1967, pp. 198-203.
 
   2) K. E. Hillstrom, ANL/AMD Program ANLC366S, DGAMMA/DLGAMA, May,
      1969.
  
   3) Hart, Et. Al., Computer Approximations, Wiley and sons, New
      York, 1968.
 
 
   Authors: W. J. Cody and Laura Stoltz  (Original Fortran Version)
            Argonne National Laboratory
 
 */ 

#ifdef __STDC__
	#pragma weak gamma = _gamma
	#pragma weak lgamma = _lgamma
#endif

#ifdef __STDC__
#define CONST const
#else
#define CONST
#endif

static CONST double sqrtpi = 0.9189385332046727417803297;

static CONST double xbig = 2.556348e+305;
static CONST double eps = 2.22E-16;	
static CONST double frtbig = 2.248562656203491e76;

/*
 * Numerator and denominator coefficients for rational minimax
 *    approximation over (0.5,1.5).
 */

static CONST double d1 = -5.772156649015328605195174e-1;
static CONST double p1[] = {
	4.945235359296727046734888e0,
	2.018112620856775083915565e2,
	2.290838373831346393026739e3,
	1.131967205903380828685045e4,
	2.855724635671635335736389e4,
	3.848496228443793359990269e4,
	2.637748787624195437963534e4,
	7.225813979700288197698961e3
};
static CONST double q1[] = {
	6.748212550303777196073036e1,
	1.113332393857199323513008e3,
	7.738757056935398733233834e3,
	2.763987074403340708898585e4,
	5.499310206226157329794414e4,
	6.161122180066002127833352e4,
	3.635127591501940507276287e4,
	8.785536302431013170870835e3
};
/*
 * Numerator and denominator coefficients for rational minimax
 *    Approximation over (1.5,4.0).
 */
static CONST double d2 = 4.227843350984671393993777e-1;
static CONST double p2[]={
	4.974607845568932035012064e0,
	5.424138599891070494101986e2,
	1.550693864978364947665077e4,
	1.847932904445632425417223e5,
	1.088204769468828767498470e6,
	3.338152967987029735917223e6,
	5.106661678927352456275255e6,
	3.074109054850539556250927e6
};
static CONST double q2[]={
	1.830328399370592604055942e2,
	7.765049321445005871323047e3,
	1.331903827966074194402448e5,
	1.136705821321969608938755e6,
	5.267964117437946917577538e6,
	1.346701454311101692290052e7,
	1.782736530353274213975932e7,
	9.533095591844353613395747e6
};

/*
 * Numerator and denominator coefficients for rational minimax
 *    Approximation over (4.0,12.0).
 */
static CONST double d4=1.791759469228055000094023e0;
static CONST double p4[]={
	1.474502166059939948905062e4,
	2.426813369486704502836312e6,
	1.214755574045093227939592e8,
	2.663432449630976949898078e9,
	2.940378956634553899906876e10,
	1.702665737765398868392998e11,
	4.926125793377430887588120e11,
	5.606251856223951465078242e11
};
static CONST double q4[]={
	2.690530175870899333379843e3,
	6.393885654300092398984238e5,
	4.135599930241388052042842e7,
	1.120872109616147941376570e9,
	1.488613728678813811542398e10,
	1.016803586272438228077304e11,
	3.417476345507377132798597e11,
	4.463158187419713286462081e11
};
/*
 * Coefficients for minimax approximation over (12, INF).
 */
static CONST double c[]={
	-1.910444077728e-03,
	8.4171387781295e-04,
	-5.952379913043012e-04,
	7.93650793500350248e-04,
	-2.777777777777681622553e-03,
	8.333333333333333331554247e-02,
	5.7083835261e-03
};

#include "synonyms.h"
#include <math.h>
#include <values.h>
#include <errno.h>

int signgam;

static int gammaflag = 0; /*indicates lgamma */

double gamma(y)
double y;
{
	
	gammaflag = 1; /*signal gamma */
	return lgamma(y);
}

double lgamma(y)
double y;
{
	double res, xm1, corr, xden, xnum, xm2, xm4, ysq;
	int i, sign, gflag;
	struct exception exc;
	sign = 1;
	gflag = gammaflag;
	gammaflag = 0;
 	if (y <= 0) {
		double ival;
		int I;
		double sinepiy, fract;
		double lgamma();
		int write();

		/* let y = I/2 + fract, I=integer, 0<=fract <0.5*/

		fract = modf(-2.0 * y, &ival) / 2.0;
		I = ival;
		/*use trig identity to reduce pi*y.*/
		if (fract == 0 && I % 2 == 0) {
			/*sin(pi*y)is zero*/
			/* negative integer - singularity */
			exc.name = (gflag ? "gamma" : "lgamma");
			exc.arg1 = y;
			exc.type = SING;
			if (_lib_version == c_issue_4)
				exc.retval = HUGE;
			else
				exc.retval = HUGE_VAL;
			if (_lib_version == strict_ansi)
				errno = EDOM;
			else if (!matherr(&exc)) {
				if (_lib_version == c_issue_4)
					if (gflag)
						(void)write(2, "gamma: SING error\n", 18);
					else
						(void)write(2, "lgamma: SING error\n", 19);
				errno = EDOM;
			}
			return exc.retval;
		} else {
			switch (I % 4) {
			case 0:	sinepiy = sin(M_PI * fract);
				sign = -1;
				break;
			case 1:	sinepiy = cos(M_PI * fract);
				sign= -1;
				break;
			case 2:	sinepiy = sin(M_PI * fract);
				break;
			case 3:	sinepiy = cos(M_PI * fract);
				break;
			}
			/* The following statement can cause
			 * cancelation and hence result in loss of accuracy
			 * near negative zeros of lgamma(x).
			 */
			res = -lgamma(-y) + log(M_PI / (-y * sinepiy));
		}
	} else if (y <= eps) {	
		/*y>0*/
 	      	res = -log(y);
       	} else if (y <= 1.5) {
		/*eps < y <= 1.5 */
	     	if (y < 0.75) {
               	   	corr = -log(y);
               	  	xm1 = y;
            	} else {
                	corr = 0.0;
                 	xm1 = (y - 0.5) - 0.5;
             	}
	        if ((y <= 0.5) || (y >= 0.75)) {
	        	xden = 1.0;
	            	xnum = 0.0;
	         	for (i = 0;i <= 7;i++){
 	                 	xnum = xnum * xm1 + p1[i];
	                      	xden = xden * xm1 + q1[i];
			}
 	          	res = corr + (xm1 * (d1 + xm1 * (xnum / xden)));
	    	} else {
 	          	xm2 = (y - 0.5) - 0.5;
  	          	xden = 1.0;
   	          	xnum = 0.0;
    	          	for (i = 0;i <= 7;i++) {
	     	          	xnum = xnum * xm2 + p2[i];
      	             		xden = xden * xm2 + q2[i];
			}
                        res = corr + xm2 * (d2 + xm2 * (xnum / xden));
               	}
	} else if (y <= 4.0) {
		/*   1.5 < y <= 4.0*/
          	xm2 = y - 2.0;
             	xden = 1.0;
              	xnum = 0.0;
              	for (i = 0;i <= 7;i++) {
	   		xnum = xnum * xm2 + p2[i];
	     		xden = xden * xm2 + q2[i];
		}
               	res = xm2 * (d2 + xm2 * (xnum / xden));
	} else if (y <= 12.0) {
		/*   4.0 < y <= 12.0*/
               	xm4 = y - 4.0;
               	xden = -1.0;
               	xnum = 0.0;
                for (i = 0;i <= 7;i++) {
			xnum = xnum * xm4 + p4[i];
	    		xden = xden * xm4 + q4[i];
		}
	        res = d4 + xm4 * (xnum / xden);
	} else  if (y < xbig) {
		/*   evaluate for 12.0< y < xbig */
	 	res = 0.0;
               	if (y <= frtbig) {
                       	res = c[6];
                       	ysq = y * y;
                       	for (i = 0;i <= 5;i++){
                       		res = res / ysq + c[i];
			}
    		}
               	res = res / y;
               	corr = log(y);
               	res = res + sqrtpi - 0.5 * corr;
                res = res + y * (corr - 1.0);
       	} else {
		/* xbig < y */
		exc.name = (gflag ? "gamma" : "lgamma");
		exc.arg1 = y;
		exc.type = OVERFLOW;
		if (_lib_version == c_issue_4)
			exc.retval = HUGE;
		else
			exc.retval = HUGE_VAL;
		if (_lib_version == strict_ansi)
			errno = ERANGE;
		else if (!matherr(&exc))
			errno = ERANGE;
		return (exc.retval);
	}
	/* Only set external at very end to avoid side effects */

	signgam = sign;	
	return res;
}
