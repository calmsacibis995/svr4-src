/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lpass2.h	1.5"

#define DPRINTF dprintf
#define DEFN	(LDI|LDS|LIB)	/* definition of entity		*/
#define DECL	(LDX|LDC)	/* declaration of entity	*/
#define USE	(LUE|LUV)	/* use of function		*/

#define USED	01
#define VUSED	02	/* function used for value		*/
#define EUSED	04	/* function used for effect		*/
#define RVAL	010	/* function has a return value in it	*/
#define VARARGS2	0100	/* function takes variable # of args	*/
#define OKGLOB	0200	/* used in more than 1 file (ok-global)	*/

#define MAXPRNT	512
#define BWERROR bwerror

#define LN_TYPE(ty)	(ty&LNQUAL)	/* type w/o qualifiers	*/

typedef unsigned short MODULE;

/* 
** STYPE
*/
typedef struct sty { 
	ATYPE t; 		/* holds a single type with modifiers	*/
	char *tag_name; 	/* tag name of type (if applicable)	*/
	struct sty *next; 	/* next entry				*/
} STYPE;


/* STAB - symbol table entry */
typedef struct sym {
	char *name;		/* symbol name				*/
	short nargs;		/* # args (or # members for s/u/e)	*/
	short decflag;		/* LDI/LIB/LDC/LDX/LRV/.....		*/
	int fline;		/* line symbol defined at		*/
	STYPE symty;		/* argument (member) list		*/
	char *fno;		/* file name				*/
	MODULE mno;		/* module # (translation unit)		*/
	char use;		/* symbol used				*/
	char more;
	int useline;		/* line no. used at (if LDX)		*/
	char *usefno;		/* file used in (if LDX)		*/
	struct sym *next;	/* next entry with same hash index	*/
} STAB;


typedef struct entry {
	T1WORD ty;		/* type # as defined in types.c		*/
	STAB *st;		/* symbol table entry 			*/
	unsigned short mno;	/* module number			*/
	struct entry *eqty;	/* this struct is same as equiv		*/
	struct uneq  *neqty;	/* this struct is not equiv to neqty	*/
	struct entry *next;	/* next type in this module		*/
} ENT;


typedef struct uneq {
	struct entry *neqty;
	struct uneq *next;
} UNEQ;


#define getarg(n)	(*atyp)[n]	/* return arg # 1	*/
#define argname(n)	(*strp)[n]	/* return arg name #1	*/
#define FMTSIZE		512
#define LN_TYPE(ty)	(ty&LNQUAL)	/* type w/o qualifiers	*/

/*
** From lpass2.c
*/
extern union rec r;
extern char *cfno;
extern unsigned short cmno;
#ifndef NODBG
extern int ln_dbflag;
#endif
extern ATYPE (*atyp)[];
extern FILEPOS curpos;
extern FLENS lout;
extern char str1[];
extern char str2[];
extern void ptype();
extern char *getstr();
extern TY promote();
extern void lastone();
extern void formerr();

/*
** From formchk.c
*/
#ifdef __STDC__
extern void fmtinit(void);
extern void fmtcheck(STAB *);
#else
void fmtinit();
void fmtcheck();
#endif
