/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acpp:common/syms.h	1.19"
/* syms.h - symbol table interfaces	*/

/* bits in Macro.flag (see struct _macro_ definition below) */
#define	M_HIDDEN	(1)		/* may not be expanded	*/
#define	M__LINE		(1<<1)		/* name is `__LINE__'	*/
#define M__FILE		(1<<2)		/* name is `__FILE__'	*/
#define M__STDC		(1<<3)		/* name is `__STDC__'	*/
#define M__DATE		(1<<4)		/* name is `__DATE__'	*/
#define M__TIME		(1<<5)		/* name is `__TIME__'	*/
#define M_DEFINED	(1<<6)		/* name is `defined'	*/
#define M_PREDEFINED	(M__LINE|M__FILE|M__STDC|M__DATE|M__TIME)
#define M_CONSTRAINED	(M__LINE|M__FILE|M__STDC|M__DATE|M__TIME|M_DEFINED)
#define M_MANIFEST	(1<<7)		/* macro is a manifest constant:
					** no further preprocessing operations
					** can be done on the replacement list.
					*/
#define ST_ISMANIFEST(mp)	((mp)->flags & M_MANIFEST)	/* is a manifest constant */
#define ST_ISPREDEFINED(mp)	((mp)->flags & M_PREDEFINED)	/* is predefined by ANSI */
#define ST_ISCONSTRAINED(mp)	((mp)->flags & M_CONSTRAINED)	/* is constrained by ANSI */
#define ST_ISUNARYOP(mp)	((mp)->flags & M_DEFINED)	/* is unary op: `defined' */

/* possible values for Macro.nformals (see struct _macro_ definition below)
** (Macro.nformals >=0 indicates number of args in a function-like macro)
*/
#define OBJECT_LIKE	-1	/* an object-like macro		*/
#define UNDEFINED	-2	/* macro definition out of scope	*/
#define ST_ISFUNCTION(mp) ((mp)->nformals >= 0) 	/* is a function-like macro	*/
#define ST_ISOBJECT(mp)	  ((mp)->nformals == -1)	/* is an object-like macro	*/
#define ST_ISUNDEF(mp)	  ((mp)->nformals == -2)	/* is an undefined macro	*/

struct _macro_	/* information about 1 macro definition */
{
	char *	name;		/* macro name */
	short	namelen;	/* distance to packed formals */
	short	nformals;	/* number of formals */
	Token *	replist;	/* list of replacement tokens */
	Macro * next;		/* others in hash bucket */
	int	flags;		/* holds M_* values; for now - maybe should save space */
#ifdef INCCOMP
	int	lineno;		/* line number of #define (0 means predefined
				** or from -D command line option)
				*/
	Macro * prevdef;	/* previous definition of same macro */
#endif
};

extern	char	st_chars[];

extern	void	st_argdef(	/* const char * */	);
extern	void	st_argundef(	/* const char * */	);
extern	Token*	st_define(	/* Token *	*/	);
extern	int	st_hidden(	/* Macro *	*/	);
extern	void	st_hide(	/* Macro *	*/	);
extern	void	st_init(	/* int		*/	);
extern	Macro *	st_ismacro(	/* Token *	*/	);
#ifdef DEBUG
extern	void	st_mprint(	/* Macro *	*/	);
#endif
#ifdef TRANSITION
extern	int	st_nmacros(	/* void		*/	);
#endif
#ifdef PERF
extern	void	st_perf(	/* void		*/	);
#endif
extern	Token*	st_undef(	/* const char *	*/	);
extern	void	st_unhide(	/* Macro *	*/	);
