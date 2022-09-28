/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ctrace:global.h	1.9"
/*	ctrace - C program debugging tool
 *
 *	global type and data definitions
 *
 */

#include <stdio.h>
#include "constants.h"
#include <string.h>
#ifdef __STDC__
#include <stdlib.h>
#endif


enum	bool {no, yes};

enum	trace_type {
	normal, assign, prefix, postfix, string, strres
	};

struct symbol_struct {
	int	start, end;
	enum	symbol_type {
		constant, variable, repeatable, side_effect
		} type;
	};

/* main.c global data */
extern	enum	bool suppress;	/* suppress redundant trace output (-s) */
extern	enum	bool pound_line;/* input preprocessed so suppress #line	*/
extern	int	tracemax;	/* maximum traced variables per statement (-t number) */
extern	char	*filename;	/* input file name */
extern	enum bool trace_fcn;  /* indicates if this function should be traced */

/* parser.y global data */
extern	enum	bool fcn_body;	/* function body indicator */

/* scanner.l global data */
extern	char	indentation[];	/* left margin indentation (blanks and tabs ) */
extern	char	yytext[];	/* statement text */
extern	enum	bool too_long;  /* statement too long to fit in buffer */
extern	int	last_yychar;	/* used for parser error handling */
extern	int	token_start;	/* start of this token in the text */
extern	int	yyleng;		/* length of the text */
extern	int	yylineno;	/* number of current input line */
extern	FILE	*yyin;		/* input file descriptor */

/* lookup.c global data */
extern	enum	bool stdio_preprocessed;	/* stdio.h already preprocessed */
extern	enum	bool setjmp_preprocessed;	/* setjmp.h already preprocessed */
extern	enum	bool signal_preprocessed;	/* signal.h already preprocessed */
extern	enum	bool syssig_preprocessed;	/* signal.h already preprocessed */
extern	enum	bool types_preprocessed;	/* types.h already preprocessed */
extern	enum	bool select_preprocessed;	/* select.h already preprocessed */
extern	enum	bool timet_preprocessed;	/* time_t already preprocessed */
extern	enum	bool clockt_preprocessed;	/* clock_t already preprocessed */
extern	enum	bool sizet_preprocessed;	/* size_t already preprocessed */


extern int 	atoi(),	
		add_fcn(),
		lookup(),
		add_symbol();

extern void	exit(),		
		init_symtab(),	
		error(),	
		fatal(),	
		puttext(),	
		tr_fcn(),
		tr_stmt(),
		tr_vars(),
		reset(),
		rm_trace(),
		expand_fcn(),
		rm_all_trace(),
		warning();
