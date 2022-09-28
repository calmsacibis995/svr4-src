/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/types.h	52.25"
/* types.h */

/* Definitions for the types package. */

/* Basic types */

#define	TY_CHAR		1	/* plain char */
#define	TY_UCHAR	2	/* unsigned char */
#define	TY_SCHAR	3	/* signed char */
#define	TY_SHORT	4	/* plain short */
#define	TY_USHORT	5	/* unsigned short */
#define	TY_SSHORT	6	/* signed short */
#define	TY_INT		7	/* plain int */
#define	TY_UINT		8	/* unsigned int */
#define	TY_SINT		9	/* signed int */
#define	TY_AINT		10	/* ambiguously int (parameter) */
#define	TY_AUINT	11	/* ambiguously unsigned int (parameter) */
#define	TY_LONG		12	/* plain long */
#define	TY_ULONG	13	/* unsigned long */
#define	TY_SLONG	14	/* signed long */
#define	TY_LLONG	15	/* plain long-long (unused) */
#define	TY_ULLONG	16	/* unsigned long-long (unused) */
#define	TY_SLLONG	17	/* signed long-long (unused) */
#define	TY_ENUM		18	/* generic enum */
#define	TY_FLOAT	19	/* float */
#define	TY_DOUBLE	20	/* double */
#define	TY_LDOUBLE	21	/* long double */
#define	TY_VOID		22	/* void */
#define	TY_STRUCT	23	/* generic struct */
#define	TY_UNION	24	/* generic union */
#define	TY_PTR		25	/* generic pointer */
#define	TY_ARY		26	/* generic array */
#define	TY_FUN		27	/* generic function */

#define	TY_MINSHIFT	5	/* shift that puts bits beyond basic types */
#define	TY_NBASE	(1<<TY_MINSHIFT) /* number of base types */

/* The type table is an array of unsigned longs.  It's presumed that
** unsigned long is big enough to hold a T1WORD, an SX, and a BITOFF.
** (After all, what's larger?)  The following struct declarations are
** imposed upon the simple array to make the accesses simpler and
** more readable.
**
** All the struct declarations have a specification word at the
** beginning.
*/

typedef unsigned long ty_typetab;
#ifndef LINT
extern const BITOFF _ty_size[TY_NBASE];	/* sizes of basic types */
#else
extern BITOFF _ty_size[TY_NBASE];	/* sizes of basic types */
#endif

/* Base type */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
} ts_base;

/* Generic derived type.
** IMPORTANT:  the ts_spec and ts_type (or equivalent)
** fields must all line up with one another in any struct that
** uses them.
*/
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_type;		/* T1WORD type derived from */
} ts_generic;

/* Qualified type. */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_unqual;	/* unqualified type */
} ts_qualtype;

/* Array specifier */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_type;		/* T1WORD array-of type */
    ty_typetab ts_nelem;	/* number of array elements */
    ty_typetab ts_size;		/* BITOFF size of array in bits */
} ts_ary;

/* Pointer specifier */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_type;		/* T1WORD pointer-to type */
} ts_ptr;

/* Function specifier */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_rettype;	/* T1WORD function-returning type */
    ty_typetab ts_erettype;	/* T1WORD effective return type */
    ty_typetab ts_prmlist;	/* index of parameter list */
} ts_fun;

/* Struct/union/enum specifier */
typedef struct {
    ty_typetab ts_spec;		/* specifier word */
    ty_typetab ts_ptr;		/* type number for pointer to this type */
    ty_typetab ts_size;		/* BITOFF size of s/u/e in bits */
    ty_typetab ts_align;	/* BITOFF alignment in bits */
    ty_typetab ts_tag;		/* SX tag symbol ID */
    ty_typetab ts_mbrlist;	/* index of member list */
    ty_typetab ts_lastmbr;	/* index of last (zero) member */
} ts_sue;

extern struct td typetab;

/* Access the i-th table element. */
#define	TE(i)	(TD_ELEM(typetab, ty_typetab, (i)))

/* Create a pointer to type table entry i as a pointer to
** struct st.
*/
#define	TT(i,st) ((st *)&(TE(i)))

/* Signify that array type has null top dimension. */
#define	TY_NULLDIM	((SIZE) (ty_typetab) ~0L)
/* Signify an error in array dimension to suppress various
** cascading messages.
*/
#define	TY_ERRDIM	((SIZE) (ty_typetab) (~0L<<1))

/* Flag two types that are equivalent but that would have
** been different if the type information in one of them
** had not been hidden.
*/
#define	TY_HIDNONCOMPAT	(-2)	/* return value from TY_EQTYPE() */

extern void ty_print();		/* print type "in English" */
extern void tt_init();		/* types initialization */
#ifdef LINT
extern void lntt_init();
#endif
extern T1WORD ty_mkptrto();	/* make pointer to type */
extern T1WORD ty_mkaryof();	/* make array of */
extern T1WORD ty_mkfunc();	/* make function returning */
extern T1WORD ty_mkcomposite();	/* make composite type */
extern T1WORD ty_mkhidden();	/* make type with hidden info */
extern T1WORD ty_mkqual();	/* make qualified at one shot */
extern T1WORD ty_unqual();	/* remove qualifiers */
extern T1WORD ty_type();	/* return top-level type for t */
extern T1WORD ty_decref();	/* return next-level type for t */
extern T1WORD ty_mktag();	/* make struct/union/enum tag */
extern T1WORD ty_proprm();	/* return function prototype parameter type */
extern T1WORD ty_erettype();	/* return effective return type for function */
extern BITOFF ty_size();	/* return size of object of indicated type */
extern BITOFF ty_align();	/* return alignment of object of type */
extern void  ty_mkparm();	/* add parameter to function */
extern void ty_mkmbr();		/* add member to s/u */
extern void ty_e_sue();		/* end s/u/e set-up */
extern SX ty_g_mbr();		/* get s/u/e member */
extern int ty_haslist();	/* test whether s/u/e has member list */
extern int ty_eqtype();		/* test for type equivalence */
extern int ty_hasproto();	/* test whether function-type has prototype */
extern int ty_hashidden();	/* test whether function-type has hidden info */
extern int ty_nparam();		/* return number of params for function type */
extern int ty_isvararg();	/* test whether function type has variable
				** number of args
				*/
extern int ty_istype();		/* test for integral/numeric type */
extern int ty_isunsigned();	/* test whether type is unsigned */
extern int ty_issigned();	/* test whether type is signed */
extern int ty_isptrobj();	/* test whether is pointer to object */
extern SX ty_suembr();		/* return s/u/e member by name */
extern SX ty_suetag();		/* return s/u/e tag symbol */
extern SIZE ty_nelem();		/* return number of s/u/e members */
extern T1WORD ty_chksize();	/* check type where size is required */
#define	TY_CVOID	(1<<0)	/* check void */
#define	TY_CTOPNULL	(1<<1)	/* check for null top array dimension */
#define	TY_CSUE		(1<<2)	/* check for incomplete struct/union/enum */

#define	TY_ISFTN(t) ((int)(ty_istype((t),TY_TFUNC)))
				/* int TY_ISFTN(T1WORD t) */
#define	TY_ISARY(t) ((int) (ty_istype((t),TY_TARRAY)))
				/* int TY_ISARY(T1WORD t) */
#define	TY_ISPTR(t) ((int) (ty_istype((t),TY_TPTR)))
				/* int TY_ISPTR(T1WORD t) */
#if lint
/* lint has trouble with t when t is a known type:  gives
** "constant in conditional context".  Be sure to reference
** both _ty_size and ty_size(), and to use the returned value.
*/
#define TY_SIZE(t) (((t) < ty_size(t)) ? _ty_size[t] : _ty_size[t])
#else
#define	TY_SIZE(t) ((t) < TY_NBASE ? _ty_size[t] : ty_size(t))
						/* BITOFF TY_SIZE(T1WORD t) */
#endif
#define	TY_ALIGN(t) (ty_align(t))		/* BITOFF TY_ALIGN(T1WORD t) */
#ifdef NODBG
#define	TY_TYPE(t) ((T1WORD) (TT((t),ts_base)->ts_spec & TY_BTMASK))
#define	TY_DECREF(t) ((T1WORD) (TT((t),ts_generic)->ts_type))
#define	ty_istype(t,bit) ((int)(TT((t),ts_base)->ts_spec&(bit)))
#else
#define	TY_TYPE(t) (ty_type(t))			/* T1WORD TY_TYPE(T1WORD t) */
#define	TY_DECREF(t) (ty_decref(t))		/* T1WORD TY_DECREF(T1WORD t) */
#endif
#define	TY_EQTYPE(tl,tr) (ty_eqtype(tl,tr))
				/* int TY_EQTYPE(T1WORD tl, T1WORD tr) */
#define	TY_GETQUAL(t) (TY_ISQUAL(t))		/* get qualifier bits */
						/* int ty_getqual(T1WORD t) */
#define	TY_ISCONST(t) (ty_istype((t),TY_CONST))	/* int TY_ISCONST(T1WORD t) */
#define	TY_ISVOLATILE(t) (ty_istype((t),TY_VOLATILE))
						/* int TY_ISVOLATILE(T1WORD t) */
#define	TY_ISNOALIAS(t) (ty_istype((t),TY_NOALIAS))
						/* int TY_ISNOALIAS(T1WORD t) */
#define	TY_ISQUAL(t) (ty_istype((t),TY_TQUAL))
#define	TY_ISMBRCONST(t) (ty_istype((t),TY_CONST|TY_TMCONST))
						/* int TY_ISMBRCONST(T1WORD t) */
#define	TY_ISMBRVOLATILE(t) (ty_istype((t),TY_VOLATILE|TY_TMVOLATILE))
						/* int TY_ISMBRVOLATILE(T1WORD t) */
#define	TY_ISMBRNOALIAS(t) (ty_istype((t),TY_NOALIAS|TY_TMNOALIAS))
						/* int TY_ISMBRNOALIAS(T1WORD t) */
#define	TY_ISMBRQUAL(t) (ty_ismbrqual((t),TY_TQUAL|TY_TMQUAL))
						/* int TY_ISMBRQUAL(T1WORD t) */
#define	TY_UNCONST(t) \
((TT((t),ts_base)->ts_spec & TY_CONST) == 0 ? \
			(T1WORD) t : ty_unqual((t),TY_CONST))
						/* T1WORD TY_UNCONST(T1WORD t) */
#define	TY_UNVOLATILE(t) \
((TT((t),ts_base)->ts_spec & TY_VOLATILE) == 0 ? \
			(T1WORD) t : ty_unqual((t),TY_VOLATILE))
						/* T1WORD TY_UNVOLATILE(T1WORD t) */
#define	TY_UNQUAL(t) \
((TT((t),ts_base)->ts_spec & TY_TQUAL) == 0 ? \
			(T1WORD) t : ty_unqual((t),TY_TQUAL))
						/* int TY_UNQUAL(T1WORD t) */
#define	TY_HASPROTO(t) (ty_hasproto(t))		/* int TY_HASPROTO(T1WORD t) */
#define	TY_HASHIDDEN(t) (ty_hashidden(t))	/* int TY_HASHIDDEN(T1WORD t) */
#define	TY_PROPRM(t,pn) (ty_proprm(t,pn))
				/* T1WORD TY_PROPRM(T1WORD t, int pn) */
#define	TY_NPARAM(t) (ty_nparam(t))		/* int TY_NPARAM(T1WORD t) */
#define	TY_ISVARARG(t) (ty_isvararg(t))		/* int TY_ISVARARG(T1WORD t) */
#define	TY_ERETTYPE(t) (ty_erettype(t))		/* T1WORD TY_ERETTYPE(T1WORD t) */

#define	TY_HASLIST(t) (ty_haslist(t))		/* int TY_HASLIST(T1WORD t) */
#define	TY_NELEM(t) (ty_nelem(t))		/* SIZE TY_NELEM(T1WORD t) */
#define	TY_G_MBR(t,mbrno) (ty_g_mbr(t,mbrno))
				/* SX TY_G_MBR(T1WORD t, SIZE mbrno) */
#define	TY_SUEMBR(t,mbr) (ty_suembr(t,mbr))
				/* SX TY_SUEMBR(T1WORD t, char *mbr) */
#define	TY_SUETAG(t) (ty_suetag(t))		/* SX TY_SUETAG(T1WORD t) */

#define	TY_ISINTTYPE(t) (ty_istype((t),TY_TINTTYPE))
				/* int TY_ISINTTYPE(T1WORD t) */
#define	TY_ISNUMTYPE(t) (ty_istype((t),TY_TNUMTYPE))
				/* int TY_ISNUMTYPE(T1WORD t) */
#define	TY_ISSCALAR(t) (ty_istype((t),TY_TSCALAR))
				/* int TY_ISSCALAR(T1WORD t) */
#define	TY_ISFPTYPE(t) (ty_istype((t),TY_TFP))
				/* int TY_ISFPTYPE(T1WORD t) */
#define	TY_ISSU(t) (ty_istype((t),TY_TSTRUCT|TY_TUNION))
				/* int TY_ISSU(T1WORD t) */
#define	TY_ISSUE(t) (ty_istype((t),TY_TSTRUCT|TY_TUNION|TY_TENUM))
				/* int TY_ISSUE(T1WORD t) */
#define	TY_ISARYORFTN(t) (ty_istype((t),TY_TARRAY|TY_TFUNC))
				/* int TY_ISARYORFTN(T1WORD t) */
#define TY_ISPTROBJ(t) (ty_isptrobj(t))		/* int TY_ISPTROBJ(T1WORD t) */
#define TY_ISUNSIGNED(t) (ty_isunsigned(t))	/* int TY_ISUNSIGNED(T1WORD t) */
#define TY_ISSIGNED(t) (ty_issigned(t))	/* int TY_ISSIGNED(T1WORD t) */

#define	TY_NONE	((T1WORD) 0)	/* no type specified */
#define	TY_DOTDOTDOT	TY_NONE	/* represent ... in prototype */

extern T1WORD ty_frint;			/* contains function-returning-int type */
extern T1WORD ty_voidstar;		/* void * type */
#define TY_VOIDSTAR (ty_voidstar)
#define	TY_FRINT (ty_frint)

/* These bits are establish characteristics of a type.
** They actually occur in the specification word of each
** type, along with the generic type.
*/
#define	TY_TSIGNED	(3L<<(TY_MINSHIFT+0))
#define	TY_TUNSIGNED	(2L<<(TY_MINSHIFT+0))
#define	TY_TPLAIN	(1L<<(TY_MINSHIFT+0))
#define	TY_TSGMASK	(3L<<(TY_MINSHIFT+0))
/* plain/unsigned/signed takes two bits */

/* Make these three early in the list so their bits will fit
** readily in a 16-bit int, if necessary.
*/
#define	TY_CONST	(1L<<(TY_MINSHIFT+2))
#define	TY_VOLATILE	(1L<<(TY_MINSHIFT+3))
#define	TY_NOALIAS	(1L<<(TY_MINSHIFT+4))

#define	TY_QUALSHIFT(qual) ((qual)<<3)
#define	TY_TMCONST	(1L<<(TY_MINSHIFT+2+3))
#define	TY_TMVOLATILE	(1L<<(TY_MINSHIFT+3+3))
#define	TY_TMNOALIAS	(1L<<(TY_MINSHIFT+4+3))

#define	TY_TPTR		(1L<<(TY_MINSHIFT+8))
#define	TY_TFP		(1L<<(TY_MINSHIFT+9))
#define	TY_TENUM	(1L<<(TY_MINSHIFT+10))
#define	TY_TSTRUCT	(1L<<(TY_MINSHIFT+11))
#define	TY_TUNION	(1L<<(TY_MINSHIFT+12))
#define	TY_TARRAY	(1L<<(TY_MINSHIFT+13))
#define	TY_TFUNC	(1L<<(TY_MINSHIFT+14))
#define	TY_THIDDEN	(1L<<(TY_MINSHIFT+15))

#define	TY_TQUAL	(TY_CONST|TY_VOLATILE|TY_NOALIAS)
#define	TY_TMQUAL	(TY_TMCONST|TY_TMVOLATILE|TY_TMNOALIAS)

#define	TY_TINTTYPE	(TY_TSGMASK|TY_TENUM)
#define	TY_TNUMTYPE	(TY_TINTTYPE|TY_TFP)
#define	TY_TSCALAR	(TY_TNUMTYPE|TY_TPTR)

#define	TY_BTMASK	(~((~0L)<<TY_MINSHIFT))

/* Types each have a unique integral identifying number. */
typedef T1WORD TY_TYPEID;
#define TY_MAPID(t) ((TY_TYPEID)(t))	/* mapping of type # to type ID */
