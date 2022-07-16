/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sinf.c	1.4"
/*LINTLIBRARY*/

/* sin and cos routines - algorithm from paper by Peter Teng, UCB
 * single precision version
 * argument is expressed as (n * pi/2 + f + g), where
 * -pi/4 <= f <= pi/4 and g is a correction term giving
 * extra precision to the reduced argument.
 * use table look-up for values less than 64*pi/2;
 * Payne and Hanek range reduction for greater values
 * sinf(x) = _sin(f + g)* -1**(s), for n even
 *      and _cos(f + g)* -1**(s), for n odd (s == floor(n/2))
 * for cosf(x), simply increment n by 1 after range reduction
 * tanf(x) == _sinf(f+g)/_cosf(f+g), for n even,
 * 	  == _-cosf(f+g)/_sinf(f+g), for n odd,
 */

#include <math.h>
#include <values.h>

/* multiples of pi/2 expressed as sum of 3 floats:
 * leading, trailing, middle
 * extended precision multiples of pi/2 (168 bits) were generated
 * using bc and dc
 * leading = (float)(n *pi/2 in extended)
 * middle = (float)(n * pi/2 - leading in extended)
 * trailing = (float)(n*pi/2 - leading - middle in extended)
 *
 * The current tables are valid only for IEEE single precision
 * format
 */

static float trailing[] = {
	0.0,
       -1.71512451e-15,	/* trailing[1] */
       -3.43024902e-15,	/* trailing[2] */
	1.83697015e-16,	/* trailing[3] */
       -6.86049804e-15,	/* trailing[4] */
       -1.47019519e-15,	/* trailing[5] */
	3.67394029e-16,	/* trailing[6] */
	9.31041082e-15,	/* trailing[7] */
       -1.37209961e-14,	/* trailing[8] */
       -1.22526577e-15,	/* trailing[9] */
       -2.94039038e-15,	/* trailing[10] */
	2.44991257e-15,	/* trailing[11] */
	7.34788059e-16,	/* trailing[12] */
	1.32305183e-14,	/* trailing[13] */
	1.86208216e-14,	/* trailing[14] */
	2.69484189e-15,	/* trailing[15] */
       -2.74419922e-14,	/* trailing[16] */
       -2.20516896e-14,	/* trailing[17] */
       -2.45053154e-15,	/* trailing[18] */
       -1.12710837e-14,	/* trailing[19] */
       -5.88078077e-15,	/* trailing[20] */
	5.63529427e-14,	/* trailing[21] */
	4.89982514e-15,	/* trailing[22] */
	1.02901281e-14,	/* trailing[23] */
	1.46957612e-15,	/* trailing[24] */
	2.10707340e-14,	/* trailing[25] */
	2.64610365e-14,	/* trailing[26] */
       -2.49920798e-14,	/* trailing[27] */
	3.72416433e-14,	/* trailing[28] */
       -1.42114730e-14,	/* trailing[29] */
	5.38968377e-15,	/* trailing[30] */
       -3.43086799e-15,	/* trailing[31] */
       -5.48839843e-14,	/* trailing[32] */
	7.34973792e-15,	/* trailing[33] */
       -4.41033793e-14,	/* trailing[34] */
	1.81303438e-14,	/* trailing[35] */
       -4.90106308e-15,	/* trailing[36] */
       -2.79324700e-14,	/* trailing[37] */
       -2.25421674e-14,	/* trailing[38] */
	3.96915539e-14,	/* trailing[39] */
       -1.17615615e-14,	/* trailing[40] */
       -6.37125816e-15,	/* trailing[41] */
	1.12705885e-13,	/* trailing[42] */
	4.40934732e-15,	/* trailing[43] */
	9.79965027e-15,	/* trailing[44] */
	7.20333691e-14,	/* trailing[45] */
	2.05802562e-14,	/* trailing[46] */
       -3.08728601e-14,	/* trailing[47] */
	2.93915223e-15,	/* trailing[48] */
	9.35945860e-14,	/* trailing[49] */
	4.21414680e-14,	/* trailing[50] */
       -9.31164834e-15,	/* trailing[51] */
	5.29220731e-14,	/* trailing[52] */
	1.46895715e-15,	/* trailing[53] */
       -4.99841596e-14,	/* trailing[54] */
	1.22495626e-14,	/* trailing[55] */
	7.44832866e-14,	/* trailing[56] */
       -9.06566692e-14,	/* trailing[57] */
       -2.84229461e-14,	/* trailing[58] */
	3.38107736e-14,	/* trailing[59] */
	1.07793675e-14,	/* trailing[60] */
       -6.90954591e-14,	/* trailing[61] */
       -6.86173598e-15,	/* trailing[62] */
	5.53719871e-14,	/* trailing[63] */
       -1.09767969e-13,	/* trailing[64] */
};

static float middle[] = {
	0.0,
       -4.37113883e-08,	/* middle[1] */
       -8.74227766e-08,	/* middle[2] */
       -1.19248806e-08,	/* middle[3] */
       -1.74845553e-07,	/* middle[4] */
	1.39070920e-07,	/* middle[5] */
       -2.38497613e-08,	/* middle[6] */
	2.90066708e-07,	/* middle[7] */
       -3.49691106e-07,	/* middle[8] */
       -3.57746401e-08,	/* middle[9] */
	2.78141840e-07,	/* middle[10] */
	5.92058313e-07,	/* middle[11] */
       -4.76995226e-08,	/* middle[12] */
       -6.87457373e-07,	/* middle[13] */
	5.80133417e-07,	/* middle[14] */
       -5.96244050e-08,	/* middle[15] */
       -6.99382213e-07,	/* middle[16] */
	5.68208577e-07,	/* middle[17] */
       -7.15492803e-08,	/* middle[18] */
       -7.11307109e-07,	/* middle[19] */
	5.56283680e-07,	/* middle[20] */
	1.82387441e-06,	/* middle[21] */
	1.18411663e-06,	/* middle[22] */
	5.44358784e-07,	/* middle[23] */
       -9.53990451e-08,	/* middle[24] */
       -7.35156902e-07,	/* middle[25] */
       -1.37491475e-06,	/* middle[26] */
	1.80002473e-06,	/* middle[27] */
	1.16026683e-06,	/* middle[28] */
	5.20509047e-07,	/* middle[29] */
       -1.19248810e-07,	/* middle[30] */
       -7.59006639e-07,	/* middle[31] */
       -1.39876443e-06,	/* middle[32] */
	1.77617494e-06,	/* middle[33] */
	1.13641715e-06,	/* middle[34] */
	4.96659254e-07,	/* middle[35] */
       -1.43098561e-07,	/* middle[36] */
       -7.82856375e-07,	/* middle[37] */
       -1.42261422e-06,	/* middle[38] */
	1.75232515e-06,	/* middle[39] */
	1.11256736e-06,	/* middle[40] */
	4.72809518e-07,	/* middle[41] */
	3.64774883e-06,	/* middle[42] */
       -8.06706169e-07,	/* middle[43] */
	2.36823325e-06,	/* middle[44] */
       -2.08622191e-06,	/* middle[45] */
	1.08871757e-06,	/* middle[46] */
       -3.36573748e-06,	/* middle[47] */
       -1.90798090e-07,	/* middle[48] */
	2.98414125e-06,	/* middle[49] */
       -1.47031380e-06,	/* middle[50] */
	1.70462567e-06,	/* middle[51] */
       -2.74982949e-06,	/* middle[52] */
	4.25109988e-07,	/* middle[53] */
	3.60004947e-06,	/* middle[54] */
       -8.54405698e-07,	/* middle[55] */
	2.32053367e-06,	/* middle[56] */
       -2.13392127e-06,	/* middle[57] */
	1.04101809e-06,	/* middle[58] */
       -3.41343707e-06,	/* middle[59] */
       -2.38497620e-07,	/* middle[60] */
	2.93644189e-06,	/* middle[61] */
       -1.51801328e-06,	/* middle[62] */
	1.65692609e-06,	/* middle[63] */
       -2.79752885e-06,	/* middle[64] */
};
static float leading[] = {
	0.0,
	1.57079637e0,	/* leading[1] */
	3.14159274e0,	/* leading[2] */
	4.71238899e0,	/* leading[3] */
	6.28318548e0,	/* leading[4] */
	7.85398149e0,	/* leading[5] */
	9.42477798e0,	/* leading[6] */
	1.09955740e1,	/* leading[7] */
	1.25663710e1,	/* leading[8] */
	1.41371670e1,	/* leading[9] */
	1.57079630e1,	/* leading[10] */
	1.72787590e1,	/* leading[11] */
	1.88495560e1,	/* leading[12] */
	2.04203529e1,	/* leading[13] */
	2.19911480e1,	/* leading[14] */
	2.35619450e1,	/* leading[15] */
	2.51327419e1,	/* leading[16] */
	2.67035370e1,	/* leading[17] */
	2.82743340e1,	/* leading[18] */
	2.98451309e1,	/* leading[19] */
	3.14159260e1,	/* leading[20] */
	3.29867210e1,	/* leading[21] */
	3.45575180e1,	/* leading[22] */
	3.61283150e1,	/* leading[23] */
	3.76991119e1,	/* leading[24] */
	3.92699089e1,	/* leading[25] */
	4.08407059e1,	/* leading[26] */
	4.24114990e1,	/* leading[27] */
	4.39822960e1,	/* leading[28] */
	4.55530930e1,	/* leading[29] */
	4.71238899e1,	/* leading[30] */
	4.86946869e1,	/* leading[31] */
	5.02654839e1,	/* leading[32] */
	5.18362770e1,	/* leading[33] */
	5.34070740e1,	/* leading[34] */
	5.49778709e1,	/* leading[35] */
	5.65486679e1,	/* leading[36] */
	5.81194649e1,	/* leading[37] */
	5.96902618e1,	/* leading[38] */
	6.12610550e1,	/* leading[39] */
	6.28318520e1,	/* leading[40] */
	6.44026489e1,	/* leading[41] */
	6.59734421e1,	/* leading[42] */
	6.75442429e1,	/* leading[43] */
	6.91150360e1,	/* leading[44] */
	7.06858368e1,	/* leading[45] */
	7.22566299e1,	/* leading[46] */
	7.38274307e1,	/* leading[47] */
	7.53982239e1,	/* leading[48] */
	7.69690170e1,	/* leading[49] */
	7.85398178e1,	/* leading[50] */
	8.01106110e1,	/* leading[51] */
	8.16814117e1,	/* leading[52] */
	8.32522049e1,	/* leading[53] */
	8.48229980e1,	/* leading[54] */
	8.63937988e1,	/* leading[55] */
	8.79645920e1,	/* leading[56] */
	8.95353928e1,	/* leading[57] */
	9.11061859e1,	/* leading[58] */
	9.26769867e1,	/* leading[59] */
	9.42477798e1,	/* leading[60] */
	9.58185730e1,	/* leading[61] */
	9.73893738e1,	/* leading[62] */
	9.89601669e1,	/* leading[63] */
	1.00530968e2	/* leading[64] */
};
#define MAXLOOKUP (float)100.530968 

static float _sinf(float, float), _cosf(float, float);
extern void _reducef(float, int, int *,float *, float *);

float
sinf(register float x)
{
	register float y;
	float p,q,tmp;
	int nn;
	register int n, sign;

	if (x < 0.0f)
		y = -x;
	else y = x;
	if (y > MAXLOOKUP){
		_reducef(y,1,&nn,&q,&p);
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * (float)M_2_PI;
		n = (int)(tmp + 0.5f); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0f) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	sign = n >> 1;
	if (n % 2)
		y = _cosf(q,p);
	else	y = _sinf(q,p);
	return((sign % 2) ? -y : y);
}

float
cosf(register float x)
{
	register float y;
	float p,q,tmp;
	int nn;
	register int n, sign;

	if (x < 0.0f)
		y = -x;
	else y = x;
	if (y > MAXLOOKUP){
		_reducef(y,1,&nn,&q,&p);
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * (float)M_2_PI;
		n = (int)(tmp + 0.5f); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0f) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	n += 1; /* increment quadrant for cos */
	sign = n >> 1;
	if (n % 2)
		y = _cosf(q,p);
	else	y = _sinf(q,p);
	return((sign % 2) ? -y : y);
}

float
tanf(register float x)
{
	register float y;
	float p,q,tmp;
	int nn;
	register int n;

	if (x < 0.0f)
		y = -x;
	else y = x;
	if (y > MAXLOOKUP){
		_reducef(y,1,&nn,&q,&p);
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * (float)M_2_PI;
		n = (int)(tmp + 0.5f); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0f) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	y = _cosf(q, p);
	tmp = _sinf(q, p);
	if (n % 2)	 /* odd quadrant: tan == -cos/sin*/
		return(-y / tmp);
	else		 /* even quadrant: tan == sin/cos*/
		return(tmp / y);
}

/* float _sinf(float x, float a)
 * float _cosf(float x, float a)
 * -p/4 <= x <= pi/4, a is a correction term for extra precision.
 * Calculate the sin or cos of a single precision number in the
 * interval [-pi/4,pi/4].  Algorithm and coefficients are
 * from a paper by Peter Teng.
 */

/* Current coefficients are valid only for IEEE single 
 * precision format 
 */


static float p[] = {
	-1.9518790263e-4,
	 8.3321919665e-3,
	-1.6666655242e-1
};
static float q[] = {
	 2.4430990379e-5,
	-1.3887303279e-3,
	 4.1666645556e-2
};

static
float _sinf(register float x, register float a)
{
	register float xsq;
	float b, sx;

	xsq = x * x;
	sx = xsq * _POLY2(xsq, p);
	b = (float)3.0 * a * sx;
	sx = (a + b) + x * sx;
	return(x + sx);
}

#define THRESH1  (float)5.44689655e-01 
#define THRESH2  (float)2.60778546e-01 

static
float _cosf(register float x, register float a)
{
	register float xsq;
	float b, qx, tx;
	
	xsq = x * x;
	b = x * a;
	qx = xsq * xsq * _POLY2(xsq, q);
	if (xsq >= THRESH1) {
		tx = (0.375f - xsq/2.0f) + (qx - b);
		return(0.625f + tx);
	}
	if (xsq >= THRESH2) {
	tx = (0.1875f - xsq/2.0f) + (qx - b);
		return(0.8125f + tx);
	}
	tx = xsq/2.0f + (b - qx);
	return(1.0f - tx);
}
