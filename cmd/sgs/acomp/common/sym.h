/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/sym.h	55.2"
/* sym.h */

/* These are definitions for the string-handling portion of
** the symbol table stuff.
*/

extern char * st_lookup();
extern char * st_nlookup();


/* Define the symbol table and its flags. */

/* These definitions refer to the various name spaces.
** If none of these bits is set, the symbol lives in the "normal"
** identifier (and typedef) name space.
*/
#define	SY_NORMAL	(0)		/* normal name space for IDs,
					** typedef names
					*/
#define SY_TAG		(1<<0)		/* structure/union/enum tag name */
#define SY_MOSU	 	(1<<1)		/* name of member of struct/union */
/* 1<<2 available */
#define SY_LABEL 	(1<<3)		/* label */

/* These are flag bits that describe properties of symbols. */
#define	SY_ISDOUBLE	(1<<4)		/* parameter, declared as float, comes
					** in as double
					*/
#define	SY_ISFIELD	(1<<5)		/* symbol is field name */
#define SY_ISREG	(1<<6)		/* symbol declared as REG */

#define	SY_INSCOPE	(1<<7)		/* symbol is now in scope (visible) */
#define	SY_DEFINED	(1<<8)		/* static/extern had definition */
#define	SY_TENTATIVE	(1<<9)		/* static/extern had tentative def. */
#define	SY_TOMOVE	(1<<10)		/* move sym. to top level on blk exit */
#define	SY_MOVED	(1<<11)		/* extern was moved to level 0 */
#define	SY_DBOUT	(1<<12)		/* debug info done for this symbol */

#define	SY_UAND		(1<<13)		/* symbol had & applied */

#define SY_SET		(1<<14)		/* symbol had value set */
#define SY_REF		(1<<15)		/* symbol was referenced */
#define	SY_AMBIG	(1<<16)		/* symbol has changed (ambiguous) type */
typedef U32 SY_FLAGS_t;			/* type of flags field */
typedef I7 SY_CLASS_t;			/* type of storage class field */
typedef U16 SY_LEVEL_t;			/* type of symbol level field */

/* Describe a symbol table entry. */

struct symtab {
    char *sy_name;		/* pointer to (hashed) name string */
    int sy_lineno;		/* lineno symbol defined at */
#ifdef LINT
    char *sy_file;		/* filename defined in	*/
#endif
    T1WORD sy_type;		/* type word */
    SY_CLASS_t sy_class;	/* (pseudo) storage class */
    SY_LEVEL_t sy_level;	/* scope level */
    SY_FLAGS_t sy_flags;	/* flags:  symbol class, other flag bits */

    int sy_regno;		/* register number of allocated reg. var. */
    SX sy_sameas;		/* symbol at more outer scope that this is
				** linked to
				*/
    SX sy_scopelink;		/* index of next symbol on scope list */
    SX sy_next;			/* index of next symbol on hash chain */
#ifdef FAT_ACOMP
    int sy_weight;		/* calculated weight (for auto/param) */
#endif
    I32 sy_offset;		/* offset or value; also, packed field info */
};

/* Symbols get referred to by symbol index, which is an
** integer index into the symbol table.  It's declared in p1.h.
*/

extern struct td td_stab;
#define SY_SIZE (td_stab.td_allo)

/* This macro accesses the i-th element of the symbol table.
** If debugging, check for zero symbol index -- error.
*/

#ifdef	NODBG
#define	SY(i) (TD_ELEM(td_stab, struct symtab, (i)))
#else
#define	SY(i) (TD_ELEM(td_stab, struct symtab, ((i) <= 0 ? \
	(cerror("bad symbol index in SY()"),1) : (i))))
#endif

/* These macros produce lvalues that access information in the
** symbol table.
*/
#define SY_NAME(sid)	(SY(sid).sy_name)	/* char * SY_NAME(SX sid) */
#define SY_LINENO(sid)  (SY(sid).sy_lineno)     /* int SY_LINENO(SX sid)  */
#ifdef LINT
#define SY_FILE(sid)	(SY(sid).sy_file)	/* char * SY_FILE(SX sid) */
#endif
#define	SY_TYPE(sid)	(SY(sid).sy_type)	/* T1WORD SY_TYPE(SX sid) */
#define	SY_CLASS(sid)	(SY(sid).sy_class)	/* SY_CLASS_t SY_CLASS(SX sid) */
#define	SY_LEVEL(sid)	(SY(sid).sy_level)	/* SY_LEVEL_t SY_LEVEL(SX sid) */
#define	SY_OFFSET(sid)	(SY(sid).sy_offset)	/* I32 SY_OFFSET(SX sid) */
#define	SY_FLAGS(sid)	(SY(sid).sy_flags)	/* SY_FLAGS_t SY_FLAGS(SX sid) */
#define	SY_REGNO(sid)	(SY(sid).sy_regno)	/* int SY_REGNO(SX sid) */
#define	SY_SAMEAS(sid)	(SY(sid).sy_sameas)	/* SX SY_SAMEAS(SX sid) */
#define	SY_WEIGHT(sid)	(SY(sid).sy_weight)	/* int SY_WEIGHT(SX sid) */

/* Field operations. */
/* Determine maximum shift width required to store maximum field
** width, which is width of long.
*/
#if SZLONG <= 32
# define	SY_WSHIFT 6	/* field width shift */
#else
# if SZLONG <= 64
#  define SY_WSHIFT 7
# endif
#endif

#define SY_sz_mask	((I32) (~((~0L) << SY_WSHIFT)))
/* make sure high bit is zero for right shift */
#define SY_off_mask	((I32) ((~(unsigned long) 0) >> SY_WSHIFT))

/* sid is symbol ID, sz is field size, off is its offset. */
				/* void SY_FLDPACK(SX sid, int sz, int off) */
#define	SY_FLDPACK(sid,sz,off) 						\
			(SY_FLAGS(sid) |= SY_ISFIELD,			\
			  (off) > SY_off_mask) ?			\
			    cerror("field offset won't fit") :		\
			  (void) (SY_OFFSET(sid) = (((off) << SY_WSHIFT)|(sz)))
			
				/* void SY_FLDUPACK(SX sid, int sz, int off) */
#define	SY_FLDUPACK(sid,sz,off)						\
			((SY_FLAGS(sid) & SY_ISFIELD) == 0) ?		\
			    cerror("SY_FLDUPACK():  not a field") :	\
			  (void) ((sz) = SY_OFFSET(sid) & SY_sz_mask,	\
			  ((off) = ( (SY_OFFSET(sid) >> SY_WSHIFT)	\
				& SY_off_mask) ))
			  
#define	SY_EMPTY ((char *) 0)	/* in sy_name field */

/* New symbols have SY_INSCOPE still clear. */
/* int SY_ISNEW(SX sid) */
#define	SY_ISNEW(sid) ((SY_FLAGS(sid) & SY_INSCOPE) == 0)

/* These two are the third argument to sy_lookup() */
#define	SY_LOOKUP	0	/* lookup only */
#define	SY_CREATE	1	/* lookup/create */

/* These are the scope levels:
** SL_EXTERN:	top-level -- outside a function
** SL_FUNARG	declaring function arguments:  either a prototype or old-style
** SL_INFUNC	at or above this level, inside a function body
*/
#define	SL_EXTERN	((SY_LEVEL_t) 0)
#define	SL_FUNARG	((SY_LEVEL_t) 1)
#define	SL_INFUNC	((SY_LEVEL_t) 2)

#define	SY_NOREG	(-1)	/* sy_regno value for "none" */

extern SX sy_lookup();
extern SX sy_hide();
extern SX sy_hidelev0();
extern SX sy_sumbr();
extern void sy_clear();
extern SX sy_chkfunext();
extern void sy_inclev();
extern void sy_declev();
extern SY_LEVEL_t sy_getlev();
#ifdef	FAT_ACOMP
extern void sy_discard();
#endif

#ifndef	SY_HASHSZ		/* number of symbol hash buckets */
#define	SY_HASHSZ	511
#endif

#define	SY_NOSYM	((SX) 0)	/* non-existent symbol ID */

/* The two sets of definitions that follow define bits in a
** type specifier/storage class word.  A zero word (SC_NONE)
** means no explicit storage class or type specifier has been
** provided.
*/

/* These are definitions for the various type specifier words.
** Except for struct/union/enum and typename, the lexer considers
** them to be L_TYPE.  These are "keyword types", as opposed to
** the types used by the rest of the front end.
*/

#define	KT_FIRST	1		/* first of these values */
#define	KT_CHAR		1
#define	KT_CONST	2
#define	KT_DOUBLE	3
#define	KT_ENUM		4
#define	KT_FLOAT	5
#define	KT_INT		6
#define	KT_LONG		7
#define	KT_NOALIAS	8
#define	KT_SHORT	9
#define	KT_SIGNED	10
#define	KT_STRUCT	11
#define	KT_UNION	12
#define	KT_UNSIGNED	13
#define	KT_VOID		14
#define	KT_VOLATILE	15

#define	KT_TYPENAME	16		/* user-defined typename */
#define	KT_LAST		16		/* last type number */

/* These are definitions of storage classes, and since they
** are used together with type specifier words, their numbers
** follow in sequence.
*/

#define	SC_NONE		((SY_CLASS_t) 0)	/* no storage class specified */
#define	SC_FIRST	((SY_CLASS_t) 17)	/* first storage class value */

#define	SC_AUTO		((SY_CLASS_t) 17)
#define	SC_EXTERN	((SY_CLASS_t) 18)
#define	SC_REGISTER	((SY_CLASS_t) 19)
#define	SC_STATIC	((SY_CLASS_t) 20)
#define	SC_TYPEDEF	((SY_CLASS_t) 21)
#define SC_ASM		((SY_CLASS_t) 22)	/* for new-style asm functions */

#define	SC_LAST		((SY_CLASS_t) 22)	/* last value for storage class
						** keywords
						*/

/* The following definitions are for pseudo-storage classes that
** are used internally.  Since these values show up in the symbol
** table sy_class field, make sure the values are disjoint from SC_
** values.
*/

#define	SC_MOS		((SY_CLASS_t) 30)
#define	SC_STRUCT	((SY_CLASS_t) 31)
#define	SC_MOU		((SY_CLASS_t) 32)
#define	SC_UNION	((SY_CLASS_t) 33)
#define	SC_MOE		((SY_CLASS_t) 34)
#define	SC_ENUM		((SY_CLASS_t) 35)
#define	SC_LABEL	((SY_CLASS_t) 36)
#define	SC_PARAM	((SY_CLASS_t) 37)
