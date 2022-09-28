/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lnstuff.h	1.3.1.2"

#define LDI 01		/* defined and initialized: storage set aside   */
#define LIB 02		/* defined on a library				*/
#define LDC 04		/* defined as a common region on UNIX		*/
#define LDX 010		/* defined by an extern: if ! pflag, same as LDI*/
#define LRV 020		/* function returns a value			*/
#define LUV 040		/* function used in a value context		*/
#define LUE 0100	/* function used in effects context		*/
#define LUM 0200	/* mentioned somewhere other than at the declaration*/
#define LDS 0400	/* defined static object (like LDI)		*/
#define LFN 01000	/* filename record 				*/
#define LSU 02000	/* struct/union def				*/
#define LPR 04000	/* prototype declaration			*/
#define LND 010000	/* end module marker				*/
#define LPF 020000	/* printf like					*/
#define LSF 040000	/* scanf like					*/

#define LNQUAL		00037		/* type w/o qualifiers		*/
#define LNUNQUAL	0174000		/* remove type, keep other info */
#define LCON		(1<<15)		/* type qualified by const	*/
#define LVOL		(1<<14)		/* type qualified by volatile	*/
#define LNOAL		(1<<13)		/* not used */
#define LCONV		(1<<12)		/* type is an integer constant	*/
#define LPTR		(1<<11)		/* last modifier is a pointer	*/
#define LINTVER		4

typedef unsigned short ushort;
typedef long FILEPOS;
typedef short TY;

typedef struct flens {
	long f1,f2,f3,f4;
	ushort ver, mno;
} FLENS;

typedef struct {
	TY aty;			/* base type 			*/
	unsigned long dcl_mod;	/* ptr/ftn/ary modifiers 	*/
	ushort dcl_con;		/* const qualifiers		*/
	ushort dcl_vol;		/* volatile qualifiers		*/
	union {
		T1WORD ty;
		FILEPOS pos;
	} extra;
} ATYPE;

typedef struct {
	short decflag;		/* what type of record is this	*/
	short nargs;		/* # of args (or members)	*/
	int fline;		/* line defined/used in		*/
	ATYPE type;		/* type information		*/
} LINE;

union rec {
	LINE l;
	struct {
		short decflag;
		char *fn;
	} f;
};


/* type modifiers */
# define LN_PTR 1
# define LN_FTN 2
# define LN_ARY 3

# define LN_TMASK 3

# define LN_ISPTR(x)	(((x)&LN_TMASK) == LN_PTR)
# define LN_ISFTN(x)	(((x)&LN_TMASK) == LN_FTN)  /* is x a function type */
# define LN_ISARY(x)	(((x)&LN_TMASK) == LN_ARY)  /* is x an array type */



/* type numbers for pass2 */

#define	LN_CHAR		1	/* plain char */
#define	LN_UCHAR	2	/* unsigned char */
#define	LN_SCHAR	3	/* signed char */
#define	LN_SHORT	4	/* plain short */
#define	LN_USHORT	5	/* unsigned short */
#define	LN_SSHORT	6	/* signed short */
#define	LN_INT		7	/* plain int */
#define	LN_UINT		8	/* unsigned int */
#define	LN_SINT		9	/* signed int */
#define	LN_LONG		10	/* plain long */
#define	LN_ULONG	11	/* unsigned long */
#define	LN_SLONG	12	/* signed long */
#define	LN_LLONG	13	/* plain long-long (unused) */
#define	LN_ULLONG	14	/* unsigned long-long (unused) */
#define	LN_SLLONG	15	/* signed long-long (unused) */
#define	LN_ENUM		16	/* generic enum */
#define	LN_FLOAT	17	/* float */
#define	LN_DOUBLE	18	/* double */
#define	LN_LDOUBLE	19	/* long double */
#define	LN_VOID		20	/* void */
#define	LN_STRUCT	21	/* generic struct */
#define	LN_UNION	22	/* generic union */

/* "generic" type - used for format checking */
#define LN_GCHAR	23
#define LN_GSHORT	24
#define LN_GINT		25
#define LN_GLONG	26
#define LN_GLLONG	27
