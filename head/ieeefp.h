/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _IEEEFP_H
#define _IEEEFP_H

#ident	"@(#)head:ieeefp.h	1.18"

/*
 * Floating point enviornment for machines that support
 * the IEEE 754 floating-point standard.  This file currently 
 * supports the 3B2 and 80*87 families.
 *
 * This header defines the following interfaces:
 *	1) Classes of floating point numbers
 *	2) Rounding Control
 *	3) Exception Control
 *	4) Exception Handling
 *	5) Utility Macros
 *	6) Full Exception Environment Control
 */
#if defined(__STDC__)
#define __i386 #machine(i386)
#else
#define __i386 i386
#endif

/* CLASSES of floating point numbers *************************
 * IEEE floating point values fall into 1 of the following 10
 * classes
 */
typedef	enum	fpclass_t {
	FP_SNAN = 0,	/* signaling NaN */
	FP_QNAN = 1,	/* quiet NaN */
	FP_NINF = 2,	/* negative infinity */
	FP_PINF = 3,	/* positive infinity */
	FP_NDENORM = 4, /* negative denormalized non-zero */
	FP_PDENORM = 5, /* positive denormalized non-zero */
	FP_NZERO = 6,	/* -0.0 */
	FP_PZERO = 7,   /* +0.0 */
	FP_NNORM = 8,	/* negative normalized non-zero */
	FP_PNORM = 9	/* positive normalized non-zero */
	} fpclass_t;

#if defined(__STDC__)
extern fpclass_t fpclass(double);	/* get class of double value */
extern int	finite( double );
extern int	unordered( double, double );
#else
extern fpclass_t fpclass();	/* get class of double value */
#endif

/* ROUNDING CONTROL ******************************************
 *
 * At all times, floating-point math is done using one of four
 * mutually-exclusive rounding modes.
 */

#if __i386

/*
 * NOTE: the values given are chosen to match those used by the
 * 80*87 rounding mode field in the control word.
 */
typedef	enum	fp_rnd {
    FP_RN = 0,	/* round to nearest representable number, tie -> even */
    FP_RM = 1,  /* round toward minus infinity                        */
    FP_RP = 2,  /* round toward plus infinity                         */
    FP_RZ = 3	/* round toward zero (truncate)			      */
    } fp_rnd;

#else  /* 3B */

/* NOTE: the values given are chosen to match
 * the values needed by the FPA and MAU rounding-control fields.
 */
typedef	enum	fp_rnd {
    FP_RN = 0,	/* round to nearest representable number, tie -> even */
    FP_RP = 1,	/* round toward plus infinity			      */
    FP_RM = 2,	/* round toward minus infinity			      */
    FP_RZ = 3	/* round toward zero (truncate)			      */
    } fp_rnd;

#endif

#if defined(__STDC__)
extern fp_rnd   fpsetround(fp_rnd);     /* set rounding mode, return previous */
extern fp_rnd   fpgetround(void);       /* return current rounding mode       */

#else
extern fp_rnd	fpsetround();	/* set rounding mode, return previous */
extern fp_rnd	fpgetround();	/* return current rounding mode       */

#endif

/* EXCEPTION CONTROL *****************************************
 *
 */

#define	fp_except	int

#define	FP_DISABLE	0	/* exception will be ignored	*/
#define	FP_ENABLE	1	/* exception will cause SIGFPE	*/
#define	FP_CLEAR	0	/* exception has not occurred	*/
#define	FP_SET		1	/* exception has occurred	*/

#if __i386

/*
 * There are six floating point exceptions, which can be individually
 * ENABLED (== 1) or DISABLED (== 0).  When an exception occurs
 * (ENABLED or not), the fact is noted by changing an associated
 * "sticky bit" from CLEAR (==0) to SET (==1).
 *
 * NOTE: the bit positions in fp_except are chosen to match those of
 * the 80*87 control word mask bits.  Although the 87 chips actually
 * ENABLE exceptions with a mask value of 0 (not 1, as on the 3b), it
 * is felt that switching these values may create more problems than
 * it solves.
 */

/* an fp_except can have the following (not exclusive) values:  */
#define FP_X_INV        0x01    /* invalid operation exception  */
#define FP_X_DNML       0x02    /* denormalization exception    */
#define FP_X_DZ         0x04    /* divide-by-zero exception     */
#define	FP_X_OFL	0x08	/* overflow exception		*/
#define FP_X_UFL        0x10    /* underflow exception          */
#define FP_X_IMP        0x20    /* imprecise (loss of precision)*/

#else /* 3B */

/*
 * There are five floating-point exceptions, which can be individually
 * ENABLED (== 1) or DISABLED (== 0).  When an exception occurs
 * (ENABLED or not), the fact is noted by changing an associated
 * "sticky bit" from CLEAR (==0) to SET (==1).
 *
 * NOTE: the bit positions in an fp_except are chosen to match that in
 * the MAU and the FPA hardware mask bits.
 */

/* an fp_except can have the following (not exclusive) values:  */
#define	FP_X_INV	0x10	/* invalid operation exception	*/
#define	FP_X_OFL	0x08	/* overflow exception		*/
#define	FP_X_UFL	0x04	/* underflow exception		*/
#define	FP_X_DZ		0x02	/* divide-by-zero exception	*/
#define	FP_X_IMP	0x01	/* imprecise (loss of precision)*/

#endif

#if defined(__STDC__)
extern fp_except fpgetmask(void);               /* current exception mask       */
extern fp_except fpsetmask(fp_except);          /* set mask, return previous    */
extern fp_except fpgetsticky(void);             /* return logged exceptions     */
extern fp_except fpsetsticky(fp_except);        /* change logged exceptions     */

#else
extern fp_except fpgetmask();	/* current exception mask       */
extern fp_except fpsetmask();	/* set mask, return previous    */
extern fp_except fpgetsticky();	/* return logged exceptions     */
extern fp_except fpsetsticky();	/* change logged exceptions     */

#endif 

/* UTILITY MACROS ********************************************
 */

#if defined(__STDC__)
extern int isnanf(float);               
extern int isnand(double);

#else
extern int isnand();
#define isnanf(x)	(((*(long *)&(x) & 0x7f800000L)==0x7f800000L)&& \
			 ((*(long *)&(x) & 0x007fffffL)!=0x00000000L) )
#endif

#if __i386

#ifndef SS
#include <sys/reg.h>
#endif
/* EXCEPTION HANDLING ****************************************
 *
 * When a signal handler catches an FPE, it will have a freshly initialized
 * coprocessor.  This allows signal handling routines to make use of
 * floating point arithmetic, if need be.  The previous state of the 87
 * chip is available, however.  There are two ways to get at this information,
 * depending on how the signal handler was set up.
 *
 * If the handler was set via signal() or sigset(), the old, SVR3, method
 * should be used: the signal handler assumes that it has a single parameter,
 * which is of type struct _fpstackframe, defined below.  By investigating
 * this parameter, the cause of the FPE may be determined.  By modifying it,
 * the state of the coprocessor can be changed upon return to the main task.
 * THIS METHOD IS OBSOLETE, AND MAY NOT BE SUPPORTED IN FUTURE RELEASES.
 *
 * If the handler was set via sigaction(), the new, SVR4, method should be
 * used: the third argument to the handler will be a pointer to a ucontext
 * structure (see sys/ucontext.h).  The uc_mcontext.fpregs member of the
 * ucontext structure holds the saved floating-point registers.  This can be
 * examined and/or modified.  By modifying it, the state of the coprocessor
 * can be changed upon return to the main task.
 */

struct _fpstackframe {  /* signal handler's argument                    */
	long signo;             /* signal number arg                    */
	long regs[SS+1];        /* all registers                        */
	struct _fpstate *fpsp;  /* address of saved 387 state           */
	char *wsp;              /* address of saved Weitek state        */
};

struct _fpreg {         /* structure of a temp real fp register         */
	unsigned short significand[4];  /* 64 bit mantissa value        */
	unsigned short exponent;        /* 15 bit exponent and sign bit */
};

struct _fpstate {       /* saved state info from an exception           */
	unsigned long cw,       /* cotrol word                          */
		      sw,       /* status word after fnclex-not useful  */
		      tag,      /* tag word                             */
		      ipoff,    /* %eip register                        */
		      cssel,    /* code segment selector                */
		      dataoff,  /* data operand address                 */
		      datasel;  /* data operand selector                */
	struct _fpreg _st[8];   /* saved register stack                 */
	unsigned long status;   /* status word saved at exception       */
};

/* The structure of the 80*87 status and control words are given by the
 * following structs.
 */
struct _cw87 {
	unsigned    mask:   6,  /* exception masks                      */
		    res1:   2,  /* not used                             */
		    prec:   2,  /* precision control field              */
		    rnd:    2,  /* rounding control field               */
		    inf:    1,  /* infinity control (not on 387)        */
		    res2:   3;  /* not used                             */
};

struct _sw87 {
	unsigned    excp:   6,  /* exception sticky bits                */
		    res1:   1,  /* not used                             */
		    errs:   1,  /* error summary-set if unmasked excep  */
		    c012:   3,  /* condition code bits 0..2             */
		    stkt:   3,  /* stack top pointer                    */
		    c3:     1,  /* condition code bit 3                 */
		    busy:   1;  /* coprocessor busy                     */
};

#else /* 3B */

/* EXCEPTION HANDLING ****************************************
 * When a floating-point exception handler is entered, the variables
 * _fpftype -- floating-point fault type, and
 * _fpfault -- pointer to floating-point exception structure
 * will be established. _fpftype identifies the primary exception
 * (if both underflow and inexact are generated, _fpftype is underflow)
 * and _fpfault points to all the other information needed.
 * Two integer faults also use this mechanism: "integer overflow" and
 * "integer divide by zero". If either of these faults is indicated,
 * or if _fpftype == UNKNOWN, _fpfault will be NULL.
 */

#undef UNKNOWN

typedef	enum	fp_ftype {
    UNKNOWN	= 0,	/* library code could not determine type      */
    INT_DIVZ	= 1,	/* integer divide by zero		      */
    INT_OVFLW	= 2,	/* integer overflow, including float->int conv*/
    FP_OVFLW	= 3,	/* floating-point overflow		      */
    FP_UFLW	= 4,	/* floating-point underflow		      */
    FP_INXACT	= 5,	/* floating-point loss of precision	      */
    FP_DIVZ	= 6,	/* floating-point divide by zero	      */
    FP_INVLD	= 7,	/* floating-point invalid operation	      */
    FP_IN_OVFLW = 8,	/* inexact AND overflow                       */
    FP_IN_UFLW  = 9	/* inexact AND underflow                      */
    } fp_ftype;

extern fp_ftype _fpftype;

/* reporting a floating-point exception requires a discriminated union
 * of all relevant data-types. The following defines the discriminant.
 * The particular values are arbitrary.
 */
typedef	enum	fp_type {
    FP_NULL	= 0,	/* uninitialized data			      */
    FP_C	= 1,	/* result of comparison, returned from trap   */
    FP_L	= 2,	/* long int data, being converted to/from flt.*/
    FP_F	= 3,	/* single-precision floating-point	      */
    FP_D	= 4,	/* double-precision floating-point	      */
    FP_X	= 5,	/* double-extended-precision floating-point   */
    FP_P	= 6,	/* packed-decimal floating-point	      */
    FP_U	= 7,	/* unsigned int data, being converted 	      */
    FP_S	= 8,	/* short int data			      */
    FP_DEC	= 9	/* decimal ascii data being converted	      */
    }	fp_type;

/* floating-point data types					*/
typedef struct extended { /* double-extended floating point	*/
    unsigned long	w[3];
    } extended;

typedef struct packdec { /* packed-decimal floating point	*/
    unsigned long	w[3];
    } packdec;

typedef struct decimal { /* ascii-decimal floating point	*/
    char *	i;		/* significand ascii digit string	*/
    char *	e;		/* exponent ascii digit string		*/
    char	sign;		/* sign of number			*/
				/* 	0	+			*/
				/* 	1	-			*/
    char	esign;		/* sign of exponent			*/
				/* 	0	+			*/
				/* 	1	-			*/
				/*	2	NaN			*/
				/*	3	infinity		*/
    int		ilen;		/* # digits in significand		*/
    int		elen;		/* # digits in exponent			*/
    } decimal;

typedef	enum	fp_cmp { /* result of comparison		*/
    FP_LT	= -1,	/* less than	*/
    FP_EQ	=  0,	/* equal	*/
    FP_GT	=  1,	/* greater than	*/
    FP_UO	=  2	/* unordered	*/
    } fp_cmp;

typedef union fp_union { /* union of all types			*/
    fp_cmp	c;
    long	l;
    unsigned	u;
    float	f;
    double	d;
    extended	x;	/* not used on 3B20  */
    packdec	p;	/* not used on 3B20  */
    short	s;  	/* only used on 3B20 */
    decimal	*dec;
    } fp_union;

typedef struct fp_dunion { /* discriminated union		*/
    fp_type	type;
    fp_union	value;
    } fp_dunion;

/* the rest of the information pointed to by _fp_fault includes:
 *	the type of operation being performed,
 *	the types and values of the operands,
 *	the type of a trapped value (if any), and
 *	the desired type of the result.
 * the following defines and struct give the information layout.
 */

typedef	enum	fp_op {	/* floating-point operations		      */
    FP_ADD  = 0,	/* floating-point addition		      */
    FP_SUB  = 1,	/* floating-point subtraction		      */
    FP_MULT = 2,	/* floating-point multiplication	      */
    FP_DIV  = 3,	/* floating-point division		      */
    FP_SQRT = 4,	/* floating-point square root		      */
    FP_REM  = 5,	/* floating-point remainder		      */
    FP_CONV = 6,	/* data movement, including changing format   */
    FP_RNDI = 7,	/* round to integer (keep in floating format) */
    FP_CMPT = 8,	/* trapping floating-point compare	      */
    FP_CMP  = 9,	/* non-trapping floating-point compare	      */
    FP_NEG  = 10,	/* (MAU) negate operation		      */
    FP_ABS  = 11,	/* (MAU) absolute value operation	      */
    FP_SIN  = 12,	/* floating-point sine			      */
    FP_COS  = 13,	/* floating-point cosine		      */
    FP_ATN  = 14	/* floating-point arc tangent		      */
    }	fp_op;

struct fp_fault {
    fp_op	operation;
    fp_dunion	operand[2];
    fp_dunion	t_value;
    fp_dunion	result;
    };

extern struct fp_fault	* _fpfault;


/* FULL EXCEPTION ENVIRONMENT CONTROL ************************
 *
 * The complete exception environment defined above requires
 * a significant amount of code to be picked up from the libraries.
 * The "glue" routines _getfltsw() and _getflthw() are provided
 * in two versions, one with full capability and one without.
 * The version without full capability has the following behavior:
 *	floating-point faults will raise SIGFPE, but
 *	the user handler will have _fpftype == UNKNOWN, and
 *	_fpfault == NULL. If SIGFPE is set to SIG_DFL, a core dump
 *	will be produced.
 * If ANY module in the final a.out #includes ieeefp.h (except as noted
 * below), the code below will force in the FULL environment,
 * 
 * NOTES: 1) code in the library (e.g. for setround(), etc.) which
 *	needs the definitions above, but does not care which fault
 *	environment is picked up SHOULD #define P754_NOFAULT
 *	before #including ieeefp.h.
 *	2) If a user program does not wish to do any fault recovery,
 *	just produce a core dump, it MAY also #define P754_NOFAULT,
 *	and may gain a code size reduction. However, if it uses a
 *	routine	in the math library that does request the full fault
 *	environment then the full environment WILL be included.
 */
#ifndef P754_NOFAULT
#define	P754_NOFAULT	1

#if defined(__STDC__)
extern void	_getflthw(int *, int (*)());
extern fp_union	_getfltsw(fp_ftype,fp_op,char *,fp_union *,fp_union *,fp_union *);
static void	(* _p754_1)(int *, int (*)()) = _getflthw;
static fp_union	(* _p754_2)(fp_ftype,fp_op,char *,fp_union *,fp_union *,fp_union *) =	_getfltsw;

extern void _s2dec(float *, decimal *, int);
extern void _d2dec(double *, decimal *, int);
extern void _dec2s(decimal *, float *, int);
extern void _dec2d(decimal *, double *, int);

#else
extern void	_getflthw();
extern fp_union	_getfltsw();
static void	(* _p754_1)() = _getflthw;
static fp_union	(* _p754_2)() =	_getfltsw;

extern void _s2dec();
extern void _d2dec();
extern void _dec2s();
extern void _dec2d();
#endif

#endif  /* ndef P&%$_NOFAULT 1 */

#endif  /* __i386 */

#endif  /* _IEEEFP_H */

