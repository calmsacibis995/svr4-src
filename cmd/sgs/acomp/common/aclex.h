/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acomp:common/aclex.h	52.6"
/* aclex.h */

extern int lx_getcur;		/* 1 if next token should reset db_curline */
#define LX_GETCURLINE()		lx_getcur = 1

/* Externally visible routines. */

extern void lx_s_getc();
extern void lx_e_getc();
extern int lx_getc();
extern void lx_ungetc();
extern void lx_errtoken();
extern int yylex();
#ifdef	MERGED_CPP
extern void lx_s_sharp();
extern void lx_e_sharp();
extern int lx_sh_yylex();
#endif

/* Dummy token codes when processing # lines. */
#define	L_ILL_TOKEN	-1	/* illegal token */
#define	L_ISHARP	-2	/* lines has initial # (aclex.l only) */
