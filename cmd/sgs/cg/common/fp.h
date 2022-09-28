/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nifg:cg/common/fp.h	1.3"
/* originally:  #ident	"@(#)stincc:m32/fp.h	1.2" */
/*
 *	parameters of the implementation
 */
#define EXPSIZE		11		/* double precision exponent size */
#define S_EXPSIZE	8		/* single precision exponent size */
#define GBITS		3		/* guard bits */
#define EXPOFFSET	((1 << (EXPSIZE - 1)) - 1)   /* exponent bias */
#define INFEXP		((1 << EXPSIZE) - 1)	     /* infinity exp */
#define MAXEXP		0x7ff			     /* double maximum exp */
#define SMAXEXP		255			     /* single maximum exp */
#define DEXP_BIAS	1023			     /* double exp bias */
#define SEXP_BIAS	127			     /* single exp bias */
#define FRACSIZE	52		/* size of double fraction */

/* masks to retrieve or test certain bits */
#define VALMASK		0x000fffffffffffff	/* takes fraction part */
#define SIGNMASK	0x8000000000000000 	/* mask off sign */
#define HIDDENBIT	0x0010000000000000	/* implied bit */
#define CARRYBIT	0x0020000000000000 << GBITS	/* carry bit */
#define NORMMASK	0xfff0000000000000 << GBITS	/* to normalize */
#define NORMBIT		0x0010000000000000 << GBITS	/* implied bit */
#define NBIT		0x0010000000000000	/* normalizing bit w/o GBITS */
#define RSBITS		0x3			/* to get round and sticky */
#define R_BIT		0x2			/* round bit */
#define S_BIT		0x1			/* sticky bit */
#define L_BIT		0x1			/* least significant bit */
#define V_BIT		0x0020000000000000	/* overflow bit */
#define SV_BIT		0x01000000		/* single prec. overflow bit */

/* definitions for special IEEE floating point cases */
#define SINFINITY	0x7f800000		/* single prec. infinity */	
#define INFINITY 	0x7ff0000000000000	/* double prec. infinity */
#define NEGINFINITY	0xfff0000000000000	/* negative infinity */
#define SNOTANUMBER	0x7f800000		/* single prec. NaN */
#define NOTANUMBER	0x7ff8000000000000	/* double prec. NaN */
#define NEGNOTANUMBER	0xfff8000000000000	/* negative NaN */
#define NEGZERO		0x8000000000000000	/* negative zero */

/* definitions for switch statements to define argument types */
#define NONE		0		/* nothing special */
#define INF		1		/* infinity */
#define NEGINF		2		/* negative infinity */
#define NAN		3		/* Not a number */
#define DZERO		4		/* zero */
#define DNEGZERO	5		/* negative zero */
#define NEGNAN		6		/* negative Not a Number */
#define NEGNONE		7		/* negative nothing special */
#define ARG1		8		/* argument one */
#define ARG2		9		/* argument two */

/* operand types */
#define ADD		1	
#define MUL		2
#define DIV		3

/*
 *	macros to pick pieces out of long integers
 *	this must change if FRACSIZE changes
 *	presently we assume two 24-bit pieces
 */
#define CHUNK 24 
#define lmul(x,y) ((long long) (x) * (long long) (y))
#define lo(x) ((x) & 0xffffff)
#define hi(x) ((x) >> CHUNK)
#define hibit(x) (((long)(x) >> (CHUNK - 1)) & 1)
#define lowbit(x) (((x) & 1) << 63)

/* These values are suitable for a machine with:
**      32 bit ints/longs, 2's Complement, with
**      characters that are "naturally" unsigned
*/

#define T_LONG_MIN      -2147483648
#define T_LONG_MAX      2147483647
#define T_ULONG_MAX     4294967295

#define T_MAXDOUBLE	1.79769313486231470e+308
#define T_MAXFLOAT	((float)3.40282346638528860e+38
#define T_MINDOUBLE	4.94065645841246544e-324
#define T_MINFLOAT	((float)1.40129846432481707e-45

/* union to define representations of a double precision number            */
/* this employs the 64 bit integer (long long) which is unique only to the */
/* Amdahl C compiler.  To make the floating point routines portable to     */
/* other machines this structure must be altered.                 	   */
union fpnum {
	double dval;
	struct fprep {
		unsigned sign:1;
		unsigned exp:EXPSIZE;
		unsigned long long frac:FRACSIZE;
	} bval;
	struct fltrep {
		unsigned sign:1;
		unsigned exp:S_EXPSIZE;
		unsigned long frac1:23;
		unsigned long frac2:32;
	} fbval;
	struct {
		unsigned long word1,word2;
	} lg;
	long long ll;
};

typedef union fpnum fp;
extern int errno;

double fladd(), flsub(), flmul(), fldiv(), flneg(), fp_addmag(), fp_submag();
double ltod(), ultod(), dround(), dtofp(); 
long dtol();
long dtof();			/* long is type pun for float */
unsigned long dtoul();
