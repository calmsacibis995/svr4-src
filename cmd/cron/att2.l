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
#ident	"@(#)cron:att2.l	1.2.2.3"
%}

%{

#include "att1.h"
#define	LL(t,v)	return(yylval = v, t)
#undef getc
#define	getc(x)	(*argp?tolower(*argp++):EOF)
#undef feof
#define	feof(x)	(*argp?0:1)
extern	int yylval;
char	*argp = "";
%}
%%
[ \t\n]+		;
"jan"|"january"		{ LL(MONTH, 0); }
"feb"|"february"	{ LL(MONTH, 1); }
"mar"|"march"		{ LL(MONTH, 2); }
"apr"|"april"		{ LL(MONTH, 3); }
"may"			{ LL(MONTH, 4); }
"jun"|"june"		{ LL(MONTH, 5); }
"jul"|"july"		{ LL(MONTH, 6); }
"aug"|"august"		{ LL(MONTH, 7); }
"sep"|"september"	{ LL(MONTH, 8); }
"oct"|"october"		{ LL(MONTH, 9); }
"nov"|"november"	{ LL(MONTH, 10); }
"dec"|"december"	{ LL(MONTH, 11); }
"sun"|"sunday"		{ LL(DAY, 0); }
"mon"|"monday"		{ LL(DAY, 1); }
"tue"|"tuesday"		{ LL(DAY, 2); }
"wed"|"wednesday"	{ LL(DAY, 3); }
"thu"|"thursday"	{ LL(DAY, 4); }
"fri"|"friday"		{ LL(DAY, 5); }
"sat"|"saturday"	{ LL(DAY, 6); }
"today"			{ LL(DAY, 7); }
"tomorrow"		{ LL(DAY, 8); }
"noon"			{ LL(TIME, NOON); }
"midnight"		{ LL(TIME, MIDNIGHT); }
"now"			{ LL(TIME, NOW); }
"am"			{ LL(SUFF, AM); }
"pm"			{ LL(SUFF, PM); }
"zulu"			{ LL(SUFF, ZULU); }
"next"			{ LL(NEXT, 1); }
"min"s?|"minute"s?		{ LL(UNIT, MINUTE); }
"hour"s?		{ LL(UNIT, HOUR); }
"day"s?			{ LL(UNIT, DAY); }
"week"s?		{ LL(UNIT, WEEK); }
"month"s?		{ LL(UNIT, MONTH); }
"year"s?		{ LL(UNIT, YEAR); }
[0-9]			{ yylval = yytext[0] - '0'; return(NUMB); }
[:]			{ LL(COLON, 0); }
[,]			{ LL(COMMA, 0); }
[+]			{ LL(PLUS, 0); }
.			{ LL(UNKNOWN, 0); }
%%
