/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libm:port/sin.c	1.2"
/*LINTLIBRARY*/

/* sin, cos and tan routines - algorithm from paper by Peter Teng, UCB
 * argument is expressed as (n * pi/2 + f + g), where
 * -pi/4 <= f <= pi/4 and g is a correction term giving
 * extra precision to the reduced argument.
 * use table look-up for values less than 64*pi/2;
 * Payne and Hanek range reduction for greater values
 * sin(x) = _sin(f + g)* -1**(s), for n even
 *      and _cos(f + g)* -1**(s), for n odd (s == floor(n/2))
 * for cos(x), simply increment n by 1 after range reduction
 * tan(x) == _sin(f+g)/_cos(f+g), for n even,
 * 	  == -_cos(f+g)/_sin(f+g), for n odd,
 */

#include <math.h>
#include <values.h>

/* multiples of pi/2 expressed as sum of 3 doubles:
 * leading, trailing, middle
 * extended precision multiples of pi/2 (168 bits) were generated
 * using bc and dc
 * leading = (double)(n *pi/2 in extended)
 * middle = (double)(n * pi/2 - leading in extended)
 * trailing = (double)(n * pi/2 - leading - middle in extended)
 *
 * The current tables are valid only for IEEE double precision
 * format
 */
static double trailing[] = {
	0.0,				/* trailing[0] */
       -1.49738490485916980e-33,	/* trailing[1] */
       -2.99476980971833970e-33,	/* trailing[2] */
	7.83379692950080060e-33,	/* trailing[3] */
       -5.98953961943667930e-33,	/* trailing[4] */
       -1.98128761683741590e-32,	/* trailing[5] */
	1.56675938590016010e-32,	/* trailing[6] */
	1.84425731006412100e-33,	/* trailing[7] */
       -1.19790792388733590e-32,	/* trailing[8] */
       -2.58024157878108330e-32,	/* trailing[9] */
       -3.96257523367483190e-32,	/* trailing[10] */
       -5.34490888856857930e-32,	/* trailing[11] */
	3.13351877180032030e-32,	/* trailing[12] */
       -8.10957619835607530e-32,	/* trailing[13] */
	3.68851462012824190e-33,	/* trailing[14] */
       -1.01348219288092360e-32,	/* trailing[15] */
       -2.39581584777467170e-32,	/* trailing[16] */
       -3.77814950266841970e-32,	/* trailing[17] */
       -5.16048315756216660e-32,	/* trailing[18] */
	3.31794450280673180e-32,	/* trailing[19] */
       -7.92515046734966370e-32,	/* trailing[20] */
	5.53277193019236120e-33,	/* trailing[21] */
       -1.06898177771371590e-31,	/* trailing[22] */
       -2.21139011676825960e-32,	/* trailing[23] */
	6.26703754360064050e-32,	/* trailing[24] */
       -4.56767689244315390e-34,	/* trailing[25] */
       -1.62191523967121510e-31,	/* trailing[26] */
       -7.74072473634325100e-32,	/* trailing[27] */
	7.37702924025648390e-33,	/* trailing[28] */
	5.55800501608116910e-36,	/* trailing[29] */
       -2.02696438576184720e-32,	/* trailing[30] */
       -1.32700593559182420e-31,	/* trailing[31] */
       -4.79163169554934350e-32,	/* trailing[32] */
       -1.24358469281176740e-32,	/* trailing[33] */
       -7.55629900533683950e-32,	/* trailing[34] */
       -1.87993939754932340e-31,	/* trailing[35] */
       -1.03209663151243330e-31,	/* trailing[36] */
       -1.84253865475543480e-32,	/* trailing[37] */
	6.63588900561346370e-32,	/* trailing[38] */
	1.51143166659823620e-31,	/* trailing[39] */
       -1.58503009346993270e-31,	/* trailing[40] */
	3.20711719867201500e-31,	/* trailing[41] */
	1.10655438603847220e-32,	/* trailing[42] */
       -2.98580632146432160e-31,	/* trailing[43] */
       -2.13796355542743170e-31,	/* trailing[44] */
	6.82031473661987640e-32,	/* trailing[45] */
       -4.42278023353651920e-32,	/* trailing[46] */
	4.05564742683238150e-32,	/* trailing[47] */
	1.25340750872012810e-31,	/* trailing[48] */
	2.10125027475701800e-31,	/* trailing[49] */
       -9.13535378488630780e-34,	/* trailing[50] */
	3.79693580683079810e-31,	/* trailing[51] */
       -3.24383047934243010e-31,	/* trailing[52] */
       -4.23835450253010650e-32,	/* trailing[53] */
       -1.54814494726865020e-31,	/* trailing[54] */
       -7.00302181231760250e-32,	/* trailing[55] */
	1.47540584805129680e-32,	/* trailing[56] */
	9.95383350842019550e-32,	/* trailing[57] */
	1.11160100321623380e-35,	/* trailing[58] */
	2.69106888291579970e-31,	/* trailing[59] */
       -4.05392877152369440e-32,	/* trailing[60] */
	4.42449888884520460e-32,	/* trailing[61] */
       -2.65401187118364840e-31,	/* trailing[62] */
	1.65983157905770920e-32,	/* trailing[63] */
       -9.58326339109868690e-32,	/* trailing[64] */
};

static double middle[] = {
	0.0,				/* middle[0] */
	6.12323399573676600e-17,	/* middle[1] */
	1.22464679914735320e-16,	/* middle[2] */
	1.83697019872102970e-16,	/* middle[3] */
	2.44929359829470640e-16,	/* middle[4] */
	3.06161699786838310e-16,	/* middle[5] */
	3.67394039744205940e-16,	/* middle[6] */
	4.28626379701573610e-16,	/* middle[7] */
	4.89858719658941280e-16,	/* middle[8] */
	5.51091059616308960e-16,	/* middle[9] */
	6.12323399573676630e-16,	/* middle[10] */
       -1.10280109986920620e-15,	/* middle[11] */
	7.34788079488411880e-16,	/* middle[12] */
       -9.80336419954470820e-16,	/* middle[13] */
	8.57252759403147220e-16,	/* middle[14] */
       -8.57871740039735570e-16,	/* middle[15] */
	9.79717439317882570e-16,	/* middle[16] */
       -7.35407060125000230e-16,	/* middle[17] */
	1.10218211923261790e-15,	/* middle[18] */
       -6.12942380210264980e-16,	/* middle[19] */
	1.22464679914735330e-15,	/* middle[20] */
       -4.90477700295529630e-16,	/* middle[21] */
       -2.20560219973841230e-15,	/* middle[22] */
	3.18470065841970660e-15,	/* middle[23] */
	1.46957615897682380e-15,	/* middle[24] */
       -2.45548340466058990e-16,	/* middle[25] */
       -1.96067283990894160e-15,	/* middle[26] */
	3.42963001824917730e-15,	/* middle[27] */
	1.71450551880629440e-15,	/* middle[28] */
       -6.18980636588357710e-19,	/* middle[29] */
       -1.71574348007947110e-15,	/* middle[30] */
       -3.43086797952235380e-15,	/* middle[31] */
	1.95943487863576510e-15,	/* middle[32] */
	2.44310379192882290e-16,	/* middle[33] */
       -1.47081412025000050e-15,	/* middle[34] */
       -3.18593861969288310e-15,	/* middle[35] */
	2.20436423846523580e-15,	/* middle[36] */
	4.89239739022352930e-16,	/* middle[37] */
       -1.22588476042053000e-15,	/* middle[38] */
       -2.94100925986341280e-15,	/* middle[39] */
	2.44929359829470650e-15,	/* middle[40] */
       -6.37125825874917860e-15,	/* middle[41] */
       -9.80955400591059270e-16,	/* middle[42] */
	4.40934745756706010e-15,	/* middle[43] */
       -4.41120439947682470e-15,	/* middle[44] */
	9.79098458681294120e-16,	/* middle[45] */
	6.36940131683941330e-15,	/* middle[46] */
       -2.45115054020447150e-15,	/* middle[47] */
	2.93915231795364750e-15,	/* middle[48] */
       -5.88139953909023720e-15,	/* middle[49] */
       -4.91096680932117990e-16,	/* middle[50] */
	4.89920617722600070e-15,	/* middle[51] */
       -3.92134567981788330e-15,	/* middle[52] */
	1.46895717834023550e-15,	/* middle[53] */
	6.85926003649835470e-15,	/* middle[54] */
       -1.96129182054553010e-15,	/* middle[55] */
	3.42901103761258890e-15,	/* middle[56] */
       -5.39154081943129590e-15,	/* middle[57] */
       -1.23796127317671540e-18,	/* middle[58] */
	5.38906489688494210e-15,	/* middle[59] */
       -3.43148696015894230e-15,	/* middle[60] */
	1.95881589799917670e-15,	/* middle[61] */
       -6.86173595904470770e-15,	/* middle[62] */
       -1.47143310088658890e-15,	/* middle[63] */
	3.91886975727153030e-15,	/* middle[64] */
};
static double leading[] = {
	0.0,				/* leading[0] */
	1.57079632679489660e0,		/* leading[1] */
	3.14159265358979310e0,		/* leading[2] */
	4.71238898038468970e0,		/* leading[3] */
	6.28318530717958620e0,		/* leading[4] */
	7.85398163397448280e0,		/* leading[5] */
	9.42477796076937930e0,		/* leading[6] */
	1.09955742875642760e1,		/* leading[7] */
	1.25663706143591720e1,		/* leading[8] */
	1.41371669411540690e1,		/* leading[9] */
	1.57079632679489660e1,		/* leading[10] */
	1.72787595947438640e1,		/* leading[11] */
	1.88495559215387590e1,		/* leading[12] */
	2.04203522483336570e1,		/* leading[13] */
	2.19911485751285520e1,		/* leading[14] */
	2.35619449019234500e1,		/* leading[15] */
	2.51327412287183450e1,		/* leading[16] */
	2.67035375555132430e1,		/* leading[17] */
	2.82743338823081380e1,		/* leading[18] */
	2.98451302091030360e1,		/* leading[19] */
	3.14159265358979310e1,		/* leading[20] */
	3.29867228626928290e1,		/* leading[21] */
	3.45575191894877280e1,		/* leading[22] */
	3.61283155162826190e1,		/* leading[23] */
	3.76991118430775170e1,		/* leading[24] */
	3.92699081698724160e1,		/* leading[25] */
	4.08407044966673140e1,		/* leading[26] */
	4.24115008234622050e1,		/* leading[27] */
	4.39822971502571040e1,		/* leading[28] */
	4.55530934770520020e1,		/* leading[29] */
	4.71238898038469000e1,		/* leading[30] */
	4.86946861306417990e1,		/* leading[31] */
	5.02654824574366900e1,		/* leading[32] */
	5.18362787842315880e1,		/* leading[33] */
	5.34070751110264870e1,		/* leading[34] */
	5.49778714378213850e1,		/* leading[35] */
	5.65486677646162760e1,		/* leading[36] */
	5.81194640914111740e1,		/* leading[37] */
	5.96902604182060730e1,		/* leading[38] */
	6.12610567450009710e1,		/* leading[39] */
	6.28318530717958620e1,		/* leading[40] */
	6.44026493985907680e1,		/* leading[41] */
	6.59734457253856590e1,		/* leading[42] */
	6.75442420521805500e1,		/* leading[43] */
	6.91150383789754560e1,		/* leading[44] */
	7.06858347057703470e1,		/* leading[45] */
	7.22566310325652380e1,		/* leading[46] */
	7.38274273593601440e1,		/* leading[47] */
	7.53982236861550350e1,		/* leading[48] */
	7.69690200129499400e1,		/* leading[49] */
	7.85398163397448310e1,		/* leading[50] */
	8.01106126665397230e1,		/* leading[51] */
	8.16814089933346280e1,		/* leading[52] */
	8.32522053201295190e1,		/* leading[53] */
	8.48230016469244110e1,		/* leading[54] */
	8.63937979737193160e1,		/* leading[55] */
	8.79645943005142070e1,		/* leading[56] */
	8.95353906273091130e1,		/* leading[57] */
	9.11061869541040040e1,		/* leading[58] */
	9.26769832808988950e1,		/* leading[59] */
	9.42477796076938010e1,		/* leading[60] */
	9.58185759344886920e1,		/* leading[61] */
	9.73893722612835970e1,		/* leading[62] */
	9.89601685880784880e1,		/* leading[63] */
	1.00530964914873380e2,		/* leading[64] */
};

/* maximum value for which table lookup is used */
#define MAXLOOKUP 100.530964914873380

static double _sin(), _cos();
extern void _reduce();

double sin(x)
double x;
{
	register double y, tmp;
	double p, q;
	int nn;
	register int n, sign;

	y = _ABS(x);
	if (y > MAXLOOKUP) {
		_reduce(y,1,&nn,&q,&p); /* range reduction, the 1
					* indicates reduce to [-pi/4,
					* pi/4] rather than [0,pi/2]
					*/
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * M_2_PI; /* find correct multiple of pi/2 */
		n = (int)(tmp + 0.5); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	sign = n >> 1; /* floor(n/2) */
	if (n % 2)
		y = _cos(q,p);
	else	y = _sin(q,p);
	return((sign % 2) ? -y : y);
}

double cos(x)
double x;
{
	register double y, tmp;
	double p, q;
	int nn;
	register int n, sign;

	y = _ABS(x);
	if (y > MAXLOOKUP) {
		_reduce(y,1,&nn,&q,&p); /* range reduction, the 1
					* indicates reduce to [-pi/4,
					* pi/4] rather than [0,pi/2]
					*/
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * M_2_PI; /* find correct multiple of pi/2 */
		n = (int)(tmp + 0.5); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	n += 1;  /* increment quadrant for cos */
	sign = n >> 1; /* floor(n/2) */
	if (n % 2)
		y = _cos(q,p);
	else	y = _sin(q,p);
	return((sign % 2) ? -y : y);
}
double tan(x)
double x;
{
	register double y, tmp;
	double p, q;
	int nn;
	register int n;

	y = _ABS(x);
	if (y > MAXLOOKUP) {
		_reduce(y,1,&nn,&q,&p); /* range reduction, the 1
					* indicates reduce to [-pi/4,
					* pi/4] rather than [0,pi/2]
					*/
		n = nn;
	}
	else {   /* table look-up */
		tmp = y * M_2_PI; /* find correct multiple of pi/2 */
		n = (int)(tmp + 0.5); /* for [0,pi/2] reduction, remove
				       * addition of 0.5
				       */
		p = y - leading[n];
		q = p - middle[n];
		p -= q;
		p -= middle[n];
		p -= trailing[n];
	}
	if (x < 0.0) {
		q = -q;
		p = -p;
		n = 4 - n; /* n is quadrant */
	}
	y = _cos(q, p);
	tmp = _sin(q, p);
	if (n % 2)	 /* odd quadrant: tan == -cos/sin*/
		return(-y / tmp);
	else		 /* even quadrant: tan == sin/cos*/
		return(tmp / y);
}

/* double _sin(x, a)
 * double _cos(x, a)
 * double x, a;
 * -p/4 <= x <= pi/4, a is a correction term for extra precision.
 * Calculate the sin or cos of a double precision number in the
 * interval [-pi/4,pi/4].  Algorithm and coefficients are
 * from a paper by Peter Teng.
 */

/* coefficients are valid only for IEEE double precision format */

static double p[] = {
	 1.59108690260756780e-10,
	-2.50510254394993115e-8,
	 2.75573156035895542e-6,
	-1.98412698361250482e-4,
	 8.33333333333178931e-3,
	-1.66666666666666796e-1
};
static double q[] = {
	-1.13599319556004135e-11,
	 2.08757292566166689e-9,
	-2.75573144009911252e-7,
	 2.48015872896717822e-5,
	-1.38888888888744744e-3,
	 4.16666666666666019e-2
};

static
double _sin(x, a)
register double x, a;
{
	register double xsq;
	double b, sx;

	xsq = x * x;
	sx = xsq * _POLY5(xsq, p);
	b = 3.0 * a * sx;
	sx = (a + b) + x * sx;
	return(x + sx);
}

static double	three_8 = 3.0/8.0,
		five_8 = 5.0/8.0,
		three_16 = 3.0/16.0,
		thirteen_16 = 13.0/16.0;

#define THRESH1 5.22344792962423750e-01
#define THRESH2	2.55389245354663900e-01

static
double _cos(x, a)
register double x, a;
{
	register double xsq;
	double b, qx, tx;
	
	xsq = x * x;
	b = x * a;
	qx = xsq * xsq * _POLY5(xsq, q);
	if (xsq >= THRESH1) {
		tx = (three_8 - xsq/2.0) + (qx - b);
		return(five_8 + tx);
	}
	if (xsq >= THRESH2) {
		tx = (three_16 - xsq/2.0) + (qx - b);
		return(thirteen_16 + tx);
	}
	tx = xsq/2.0 + (b - qx);
	return(1.0 - tx);
}
