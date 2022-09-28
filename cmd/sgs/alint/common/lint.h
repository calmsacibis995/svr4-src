/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)alint:common/lint.h	1.9"

#define CC		(1)	/* part of a conditional	*/
#define EFF		(1<<1)	/* used for effects		*/
#define VAL		(1<<2)	/* used for its value		*/
#define AVAL		(1<<3)	/* used for its (address) value	*/
#define ASSG		(1<<4)	/* gets a value			*/
#define LN_LEFT		0	/* left side of tree		*/
#define LN_RIGHT	1	/* right side of tree		*/

/*
** From directives.c
*/
#ifdef __STDC_
extern void ln_initdir(void);
extern void ln_setflags(char *, int);
#else
extern void ln_initdir();
extern void ln_setflags();
#endif

extern int ln_directs[];
#define ARGSUSED		0
#define VARARGS			1
#define PRINTFLIKE		2
#define SCANFLIKE		3
#define PROTOLIB		4
#define CONSTANTCONDITION	5
#define LINTLIBRARY 		6
#define NOTREACHED 		7
#define IKWID 			8
#define EMPTY 			9
#define FALLTHRU 		10
#define LN_DIR(x)	ln_directs[(x)]


/*
** From lint.c
*/
extern int ln_ifflg; 	/* presently in an if 			*/
extern int ln_elseflg;	/* presently in an else 		*/
extern int ln_walk;	/* do lint checks during optim		*/
extern int ln_sidptr;	/* for evaluation order undefined	*/
extern int ln_stmt;	/* used for fallthru on cases		*/

#ifdef __STDC__
extern int isnegcon(ND1 *);
extern int iscon(ND1 *);
extern int iszero(ND1 *);
extern int issameid(ND1 *, ND1 *);
extern int notequal(ND1 *, ND1 *);
extern void ln_symunused(int, int);
extern void ln_params(int);
extern int ln_sides(ND1 *);
extern void ln_funcdef(SX, int);
extern int ln_rch(void);
extern void ln_chconst(int);
extern void ln_expr(ND1 *);
extern void ln_endexpr(void);
extern void ln_if_else(void);
#ifndef NODBG
extern char *pgoal();
#endif
extern int ln_getgoal(ND1 *, int, int);
extern void ln_paren(ND1 *);
extern SX ln_curfunc(void);
extern void ln_chkfall(int);
extern void ln_setgoal(int);
extern int ln_dflgoal(void);
extern void ln_inclev(void);
extern void ln_declev(void);
#else
extern int isnegcon();		/* general utility functions	*/
extern int iscon();		/*   "       "        "		*/
extern int iszero();		/*   "       "        "		*/
extern int issameid();		/*   "       "        "		*/
extern int notequal();		/*   "       "        "		*/
extern void ln_symunused();	/* check for unused symbols	*/
extern void ln_params();	/* check for unused parameters	*/
extern int ln_sides();		/* check for side effects	*/
extern void ln_assign();	/* check assignments		*/
extern void ln_funcdef();	/* write function defs		*/
extern int ln_rch();
extern void ln_chconst();	/* check illegal char constants	*/
extern void ln_expr();		/* call when expr. seen		*/
extern void ln_if_else();	/* check if/else consequent	*/
#ifndef NODBG
extern char *pgoal();		/* print goal (for debugging)	*/
#endif
extern int ln_getgoal();	/* return goal for node, goal	*/
extern void ln_paren();		/* expression has parens	*/
extern SX ln_curfunc();		/* return current function id	*/
extern void ln_chkfall();	/* check for fallthru on cases  */
extern void ln_setgoal();
extern int ln_dflgoal();
extern void ln_inclev();
extern void ln_declev();
#endif

/*
** From lmain.c
*/
#ifdef __STDC__
extern const char *ln_optstring(void);
extern int ln_opt(int, char *);
extern int ln_cleanup(void);
#else
extern const char *ln_optstring();/* return option string to main	*/
extern int ln_opt();		/* called from main.c with options 	*/
extern int ln_cleanup();	/* called when done			*/
#endif

typedef struct portsize {
	T1WORD ty;
	BITOFF size, align;
} PORTSIZE;


/*
** From postopt.c
*/
#ifdef __STDC__
extern void ln_postop(ND1 *, int, int, int);
#else
extern void ln_postop();	/* main routine to call from optim	*/
#endif


/*
** From cxref.c
*/
#ifdef __STDC__
extern void cx_init(char *);
extern void cx_ident(SX, int);
extern void cx_inclev(void);
extern void cx_declev(void);
extern void cx_use(SX, int);
extern void cx_end(void);
#else
extern void cx_init();
extern void cx_ident();
extern void cx_inclev();
extern void cx_declev();
extern void cx_use();
extern void cx_end();
#endif


/*
** From lnstuff.c
*/
#ifdef __STDC__
extern void ln2_funccall(ND1 *, int);
extern void ln2_funcdef(SX, int, int);
extern void ln2_ident(SX);
extern void ln2_suedef(T1WORD);
extern void ln2_retval(void);
extern void ln2_outdef(SX, int, int);
extern void ln2_endmark(void);
extern void ln2_def(void);
#else
extern void ln2_funccall();
extern void ln2_funcdef();
extern void ln2_ident();
extern void ln2_suedef();
extern void ln2_retval();
extern void ln2_outdef();
extern void ln2_endmark();
extern void ln2_def();
#endif

#define ISSET(i)        (SY_FLAGS(i)&SY_SET)  /* ISSET(int)-symbol have value?*/
#define NOTSET(i)        (! (SY_FLAGS(i)&SY_SET))
#define ISREF(i)        (SY_FLAGS(i)&SY_REF)  /* ISREF(int)-symbol referenced?*/
#define NOTREF(i)        (! (SY_FLAGS(i)&SY_REF))

#define ISVOIDSTAR(p)   (TY_ISPTR(p) && (TY_DECREF(p) == TY_VOID))
#define IS_SYMBOL(p)	(((p->op == NAME) || (p->op == ICON)) \
			 && (p->rval != ND_NOSYMBOL))
#define isposcon(p)	(iscon(p) && (! isnegcon(p)))
#define notzerocon(p)	(iscon(p) && (! iszero(p)))
#define NOTDEFINED(sid) (! (SY_FLAGS(sid)&SY_DEFINED))
#define unconv(p)	while (p->op == CONV) p = p->left
