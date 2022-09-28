/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/name.h	1.2.3.1"
/*
 * Definitions of structures for name-value pairs
 * These structures are used for named variables, functions and aliases
 */

#include	"sh_config.h"
#include	"flags.h"

/* Nodes can have all kinds of values */
union Namval
{
	char		*cp;
	int		*ip;
	char		c;
	int		i;
	unsigned	u;
	long		*lp;
	double		*dp;	/* for floating point arithmetic */
	struct Namaray	*aray;	/* for array node */
	union Namval	*up;	/* for indirect node */
	struct Bfunction *fp;	/* builtin-function like $RANDOM */
	struct Ufunction *rp;	/* shell user defined functions */
	void	(*vfp)();	/* function pointer */
};

/* each namnod and each array element has one of these */
struct Nodval
{
	unsigned	namflg; 	/* attributes */
	union Namval	namval; 	/* value field */
	char		*namenv;	/* pointer to environment name */
	short 		namsz;		/* size of item */
};

#ifdef MULTIDIM
#   define NDIM 7
#else
#   define NDIM 1
#endif /* MULTIDIM */

/* This is an array template */
struct Namaray
{
	unsigned short	cur[NDIM+1];	/* current element */
	unsigned short	maxi;		/* maximum index of array */
	unsigned short	nelem;		/* number of elements */
	struct Nodval	*val[1];	/* array of value holders */
};


/* this bits are or'd with adot for a[*] and a[@] */
#define	ARRAY_STAR	010000
#define	ARRAY_AT	020000
#define ARRAY_UNDEF	040000
#define ARRAY_MASK	07777		/* ARRMAX cannot be larger than this */
#define ARRMAX	 1024	/* maximum number of elements in an array */
#define ARRINCR    16	/* number of elements to grow when array bound exceeded
				 Must be a power of 2 */

/* These flags are used as options to array_get() */
#define A_ASSIGN	0
#define A_LOOKUP	1
#define A_DELETE	2

/* This is a template for a storage tree */
struct Amemory
{
	struct Amemory  *nexttree;	/* search trees can be chained */
	short		memsize;	/* number of listheads */
	struct namnod	*memhead[1];	/* listhead pointers   */
};

/* This describes a named node */
struct namnod
{
	struct namnod	*namnxt;	/* pointer to next namnod  */
	char		*namid;		/* pointer to name of item */
	struct Nodval	value;		/* determines value of the item */
};

/* This describes a builtin function node */
struct Bfunction
{
	long	(*f_vp)();		/* value function */
	int	(*f_ap)();		/* assignment function */
};

/* This describes a user defined function node */
struct Ufunction
{
	off_t	hoffset;		/* offset into history file */
	int	*ptree;			/* address of parse tree */
};

#ifndef NULL
#   define NULL	0
#endif

/* types of namenode items */

#define N_DEFAULT 0
#define N_INTGER	I_FLAG	/* integer type */
#define N_AVAIL		B_FLAG	/* node is logically non-existent, blocked */
#define N_CWRIT		C_FLAG	/* make copy of node on assignment */
#define N_ARRAY		F_FLAG	/* node is an array */
#define N_INDIRECT	P_FLAG	/* value is a pointer to a value node */
#define N_ALLOC		V_FLAG	/* don't allocate space for the value */
#define N_FREE		G_FLAG	/* don't free the space when releasing value */
#define N_LTOU		U_FLAG	/* convert to uppercase */
#define N_UTOL		L_FLAG	/* convert to lowercase */
#define N_ZFILL		Z_FLAG	/* right justify and fill with leading zeros */
#define N_RJUST		W_FLAG	/* right justify and blank fill */
#define N_LJUST		S_FLAG	/* left justify and blank fill */
#define N_HOST		M_FLAG	/* convert to host file name in non-unix */
#define N_EXPORT	X_FLAG	/* export bit */
#define N_RDONLY	R_FLAG	/* readonly bit */
#define N_RESTRICT	V_FLAG	/* restricted bit */
#define N_IMPORT	N_FLAG	/* imported from environment */


/* The following are used with INT_FLG */
#define	N_BLTNOD	S_FLAG		/* builtin function flag */
#define N_DOUBLE	M_FLAG		/* for floating point */
#define N_EXPNOTE	L_FLAG		/* for scientific notation */
#define NO_LONG		L_FLAG		/* when integers are not long */
#define N_UNSIGN	U_FLAG		/* for unsigned quantities */
#define N_CPOINTER	W_FLAG		/* for pointer */
#define N_FUNCTION	(N_INTGER|Z_FLAG)/* for function trees */

#define NO_CHANGE	(N_EXPORT|N_IMPORT|N_RDONLY|E_FLAG|E_FLAG)
#define NO_PRINT	(N_LTOU|N_UTOL)
#define NO_ALIAS	(NO_PRINT|N_FLAG)
#define N_BLTIN		(NO_PRINT|N_EXPORT)
#define BLT_SPC		(Z_FLAG)
#define BLT_ENV		(M_FLAG)
#define BLT_DCL		(S_FLAG)
#define is_abuiltin(n)	(nam_istype(n,N_BLTIN)==N_BLTIN)
#define is_afunction(n)	(nam_istype(n,N_FUNCTION)==N_FUNCTION)
#define	funtree(n)	((n)->value.namval.rp->ptree)
#define	funptr(n)	((n)->value.namval.vfp)


/* namnod lookup options */

#define N_ADD		1	/* add node if not found */
#define N_CHECK		2	/* look for only if valid name */
#define N_ANYNAME	8	/* no name check done */
#define N_NULL		16	/* null value returns NULL */
#define N_NOSCOPE	G_FLAG	/* look only in current scope */

/* NAMNOD MACROS */

/* ... for attributes */

#define namflag(n)	(n)->value.namflg
#define nam_istype(n,f)	(namflag(n) & (f))
#ifdef KSHELL
#   define nam_ontype(n,f)	((n)->value.namflg |= f)
#   define nam_typeset(n,f)	((n)->value.namflg = f)
#   define nam_offtype(n,f)	((n)->value.namflg &= f)
#   ifdef PROTO
	extern char	*nam_fstrval(struct namnod*);
#   else
	extern char	*nam_fstrval();
#   endif /* PROTO */
#else
#   define nam_ontype(n,f)	(nam_newtype (n, namflag(n)|(f)))
#   define nam_typeset(n,f)	(nam_newtype (n, f))
#   define nam_offtype(n,f)	(nam_newtype (n, namflag(n)&(f)))
#endif	/* KSHELL */

/* ... etc */

#define isnull(n)	((n)->value.namval.cp == NULL)  /* strings only */
#define freeble(nv)	(((int)(nv)) & 01)
#define mrkfree(nv)	((struct Nodval*)(((int)(nv)) | 01))
#define unmark(nv)	((struct Nodval*)(((int)(nv)) & ~01))

/* ...	for arrays */

#define array_ptr(n)	((n)->value.namval.aray)
#define array_elem(n)	array_ptr(n)->nelem
#ifdef PROTO
    extern void 		array_dotset(struct namnod*,int);
    extern struct Nodval	*array_find(struct namnod*,int);
    extern struct Namaray	*array_grow(struct Namaray*,int);
    extern char 		*array_subscript(struct namnod*,char*);
    extern int			array_next(struct namnod*);
#else
    extern void 		array_dotset();
    extern struct Nodval	*array_find();
    extern struct Namaray	*array_grow();
    extern char 		*array_subscript();
    extern int			array_next();
#endif /* PROTO */

#ifdef NAME_SCOPE
   extern struct namnod *nam_copy();
#endif /* NAME_SCOPE */
#define new_of(type,x)	((type*)malloc((unsigned)sizeof(type)+(x)))
#ifdef PROTO
    extern char 		*malloc(unsigned);
    extern struct namnod	*nam_alloc(const char*);
    extern void 		nam_free(struct namnod*);
    extern void 		nam_fputval(struct namnod*,char*);
    extern int			nam_hash(const char*);
    extern void 		nam_init(void);
    extern void 		nam_link(struct namnod*,struct Amemory*);
    extern void 		nam_longput(struct namnod*,long);
    extern void 		nam_newtype(struct namnod*,unsigned,int);
    extern void 		nam_putval(struct namnod*,char*);
    struct argnod;		/* struct not declared yet */
    extern void 		nam_scope(struct argnod*);
    extern struct namnod	*nam_search(const char*,struct Amemory*,int);
    extern char 		*nam_strval(struct namnod*);
    extern void 		nam_unscope(void);
#else
    extern char 		*malloc();
    extern struct namnod	*nam_alloc();
    extern void 		nam_free();
    extern void 		nam_fputval();
    extern int			nam_hash();
    extern void 		nam_init();
    extern void 		nam_link();
    extern void 		nam_longput();
    extern void 		nam_newtype();
    extern void 		nam_putval();
    extern void 		nam_scope();
    extern struct namnod	*nam_search();
    extern char 		*nam_strval();
    extern void 		nam_unscope();
#endif /* PROTO */

extern const char e_synbad[];
extern const char e_subscript[];
extern const char e_number[];
extern const char e_nullset[];
extern const char e_notset[];
extern const char e_readonly[];
extern const char e_restricted[];
extern const char e_ident[];
extern const char e_intbase[];
extern const char e_format[];
extern const char e_aliname[];
extern int sh_lastbase;
