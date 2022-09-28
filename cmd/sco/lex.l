%{
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
%}
%{
/*	UNIX System Laboratories, Inc.                     	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
%}

%{
#ident	"@(#)sco:lex.l	1.2"
%}

%{
/*
 *	@(#) lex.l 1.1 86/12/12 
 *
 *	Copyright (C) The Santa Cruz Operation, 1984-7.
 *	Copyright (C) Microsoft Corporation, 1984-7.
 *	This Module contains Proprietary Information of
 *	The Santa Cruz Operation, Microsoft Corporation
 *	and AT&T, and should be treated as Confidential.
 */
%}

%{
/*
 * not too much to say here
 * see the design document for the tokens
 * that are expected in the mapfile
 */
#include "defs.h"
%}
%%
input	return(INPUT);
output	return(OUTPUT);
dead    return(DEAD);
compose return(COMPOSE);
beep	return(BEEP);
null	return(NULL_KEYWORD);
Input	return(INPUT);
Output	return(OUTPUT);
Dead    return(DEAD);
Compose return(COMPOSE);
Beep	return(BEEP);
Null	return(NULL_KEYWORD);
INPUT	return(INPUT);
OUTPUT	return(OUTPUT);
DEAD    return(DEAD);
COMPOSE return(COMPOSE);
BEEP	return(BEEP);
NULL	return(NULL_KEYWORD);

0[0-7]+ return(getnum(yytext+1, 8));
[0-9]+  return(getnum(yytext, 10));
0[xX][0-9a-fA-F]+ {
	return(getnum(yytext+2, 16));
	}
'[^'\\]' {
	return(yytext[1]);
	}
'\\[0-9]+' {
	yytext[yyleng-1] = CNULL;
	return(getnum(yytext+2, 8));
	}
'\\[xX][0-9a-fA-F]+' {
	yytext[yyleng-1] = CNULL;
	return(getnum(yytext+3, 16));
	}
'\\.'	return(escape(yytext[2]));
\n	return(NEWLINE);
#.*\n	return(NEWLINE);
[ \t]+  ;
.	return(ERROR);
%%

int eof;

int
token()
{
	int i;

	i = yylex();
	if (i > 255)
		return(TOOBIG);
	return((i == 0 && eof)? LEXEOF: i);
}
	
yywrap()
{
	extern int eof;

	eof = 1;
	return(1);
}

escape(c)
char c;
{
	return((c == 'n')? '\n':
	       (c == 't')? '\t':
	       (c == 'r')? '\r':
	       (c == 'b')? '\b':
			   c
	      );
}

int
getnum(s, base)
char *s;
int base;
{
	char c;
	int val, i;

	val = 0;
	while (c = *s) {
		i = c - (('0' <= c && c <= '9')? ('0'):
			 ('a' <= c && c <= 'f')? ('a' - 10):
						 ('A' - 10));
		val = val*base + i;
		++s;
	}
	return(val);
}
