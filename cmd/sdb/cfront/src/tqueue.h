/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/tqueue.h	1.1"
/*ident	"@(#)cfront:src/tqueue.h	1.4" */
#ifndef EOF
#include <stdio.h>
#else
extern printf(const char* ...);
extern fprintf(const FILE*, const char* ...);

#endif
struct toknode {
	TOK      tok;			/* token for parser */
	YYSTYPE  retval;			/* $arg */
	toknode* next;
	toknode* last;
		toknode(TOK,YYSTYPE);
		inline ~toknode();
};
extern toknode* front;
extern toknode* rear;

extern void addtok(TOK,YYSTYPE);	/* add tok to rear of Q */
extern TOK  deltok();			/* take tok from front of Q */

// extern char* image(TOK);
extern void tlex();
extern TOK lalex();
extern void* malloc(unsigned);

extern YYSTYPE yylval;
extern TOK tk;				// last token returned;

extern char* image(TOK);
