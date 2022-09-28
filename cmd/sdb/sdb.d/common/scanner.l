%{
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1988 AT&T	*/
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

%a 9800
%o 15000
%p 7600
%n 820
%{
#ident	"@(#)sdb:sdb.d/common/scanner.l	1.13"
#include <string.h>
#include "utility.h"
#include "str.h"
typedef struct SDBinfo SDBinfo;
#include "scanner.h"
#undef YYLMAX
#define YYLMAX	5120	/* same as MAXTOK in parser.Y */
#undef input
#define input() (*lineptr++)
#undef unput
#define unput(c) {*--lineptr=(c);}
#define yywrap() 1
extern char *lineptr;
extern int toknum;
YYSTYPE yylval;
#define RETURN	return ++toknum,
%}
%START SHCMD SLCMD SLFMT QCMD QFMT FCMD BCMD RCMD ECMD
dnum			([1-9][0-9]*)
xnum			(0[xX][a-fA-F0-9]+)
onum			(0[0-7]*)
num			({dnum}|{xnum}|{onum})
fnum			("-"?[0-9]+("."[0-9]*)?([eE][+-]?[0-9]+)?)
FNAME			([_a-zA-Z][._a-zA-Z0-9]*)
PVNAME			([_*a-zA-Z][_?*a-zA-Z0-9]*)
vlook			[-.)[!$=/,]
plook			[(]
format			[a-zA-Z]+
cmdchar			([tTxXepzwrRcCgsSiIvkbBdDlaMqVY])
%%
"#".*				{ yylval.sval = 0; BEGIN 0; RETURN COMMENT; }
<SLFMT>{dnum}?{format}  	{ yylval.sval = str(yytext); BEGIN 0; RETURN SLASHFORM; }
<QFMT>{format}			{ yylval.sval = str(yytext); BEGIN 0; RETURN QMARKFORM; }
<BCMD>.*			{ yylval.sval = str(yytext); BEGIN 0; RETURN b_CMD; }
<RCMD>.*			{ char *p = yytext;
				  while ( *p ) {
					if ( *p == '"' ) {
				  more_string:	++p;
						while ( *p && *p != '"' ) {
							++p;
						}
						if ( *p == '"' && *(p-1) == '\\' )
							goto more_string;
					}
					if ( *p == ';' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed semi-colon is part of cmd */
							char *t = p-1;
							char *f = p;
							while ( *f ) *t++ = *f++;
							*t = '\0';
						} else {
							/* naked semi-colon is command separator */
							yyless( p-yytext );
							break;
						}
					}
					++p;
				  }
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN r_CMD;
				}
<SLCMD>.*			{ char *p = yytext;
				  while ( *p ) {
					if ( *p == '/' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed slash is part of expr */
						} else {
							/* naked slash is terminator */
							yyless( p-yytext+1 );
							yytext[yyleng-1] = '\0';
							break;
						}
					}
					++p;
				  }
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN FSEARCH;
				}
<QCMD>.*			{ char *p = yytext;
				  while ( *p ) {
					if ( *p == '?' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed q-mark is part of expr */
						} else {
							/* naked q-mark is terminator */
							yyless( p-yytext+1 );
							yytext[yyleng-1] = '\0';
							break;
						}
					}
					++p;
				  }
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN BSEARCH;
				}
<ECMD>.*			{ char *p = yytext;
				  while ( *p ) {
					if ( *p == '"' ) {
				  more_string1:	++p;
						while ( *p && *p != '"' ) {
							++p;
						}
						if ( *p == '"' && *(p-1) == '\\' )
							goto more_string1;
					}
					if ( *p == '#' ) { /* comment */
						yyless( p-yytext );
						break;
					} else if ( *p == ';' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed semi-colon is part of cmd */
							char *t = p-1;
							char *f = p;
							while ( *f ) *t++ = *f++;
							*t = '\0';
						} else {
							/* naked semi-colon is command separator */
							yyless( p-yytext );
							break;
						}
					}
					++p;
				  }
				  --p;
				  while ( *p == ' ' || *p == '\t' )
					*p-- = '\0';		/* remove trailing whitespace */
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN e_CMD;
				}
<FCMD>.*			{ char *p = yytext;
				  while ( *p == ' ' || *p == '\t' )
					++p;	/* skip initial whitespace */
				  while ( *p ) {
					if ( *p == ' ' || *p == '\t' || *p == '#' ) {
						/* end of filename */
						yyless( p-yytext );
						break;
					} else if ( *p == ';' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed semi-colon is part of filename */
							char *t = p-1;
							char *f = p;
							while ( *f ) *t++ = *f++;
							*t = '\0';
						} else {
							/* naked semi-colon is command separator */
							yyless( p-yytext );
							break;
						}
					}
					++p;
				  }
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN FINP_CMD;
				}
<SHCMD>.*			{ char *p = yytext;
				  while ( *p ) {
					if ( *p == '"' ) {
				  more_string2:	++p;
						while ( *p && *p != '"' ) {
							++p;
						}
						if ( *p == '"' && *(p-1) == '\\' )
							goto more_string2;
					}
					if ( *p == ';' ) {
						if ( p>yytext && *(p-1) == '\\' ) {
							/* backslashed semi-colon is part of cmd */
							char *t = p-1;
							char *f = p;
							while ( *f ) *t++ = *f++;
							*t = '\0';
						} else {
							/* naked semi-colon is command separator */
							yyless( p-yytext );
							break;
						}
					}
					++p;
				  }
				  yylval.sval = str(yytext);
				  BEGIN 0;
				  RETURN SHELL_CMD;
				}
"b"/[ \t]+[^ \t;#]		{ yylval.sval = str(yytext); BEGIN BCMD; RETURN yytext[0]; }
"b"/[;\n]			{ yylval.sval = str(yytext); RETURN yytext[0]; }
"r"/[ \t]+			{ yylval.sval = str(yytext); BEGIN RCMD; RETURN yytext[0]; }
"r"/[;\n]			{ yylval.sval = str(yytext); RETURN yytext[0]; }
"e"/[ \t]+			{ yylval.sval = str(yytext); BEGIN ECMD; RETURN yytext[0]; }
{cmdchar}/{plook}		{ yylval.sval = str(yytext); RETURN PNAME; }
{cmdchar}/{vlook}		{ yylval.sval = str(yytext); RETURN VNAME; }
{cmdchar}			{ yylval.sval = str(yytext); RETURN yytext[0]; }
\".*				{ char *p = yytext+1;	/* skip initial " */
				  int len;
				  while ( *p ) {
					if ( *p == '"' ) {
						if ( *(p-1) == '\\' ) {
							/* backslashed quote */
						} else {
							/* terminating quote */
							break;
						}
					}
					p++;
				  }
				  /* *p is now terminal quote or null */
				  len = p - (yytext + 1);
				  yylval.sval = strn(yytext+1, len);
					/* swallow terminating quote */
				  yyless( p - yytext + (*p=='"') );
				  RETURN STRING;
				}
"/"				{ if ( toknum == 0 ) {
					BEGIN SLCMD;
				  } else {
					BEGIN SLFMT;
				  }
				  yylval.sval = str(yytext);
				  RETURN yytext[0];
				}
"="{format}?			{ yylval.sval = str(yytext+1); RETURN EQUALFORM; }
"->"				{ yylval.sval = str(yytext); RETURN ARROW; }
"."				{ yylval.sval = str(yytext); RETURN yytext[0]; }
\'[^'\n]+\'?			{ /* match ' */ yylval.sval = str(yytext); RETURN SCHAR; }
"!"				{ yylval.sval = str(yytext);
				  if ( toknum == 0 ) {
					BEGIN SHCMD;
				  }
				  RETURN yytext[0];
				}
"<"				{ yylval.sval = str(yytext); BEGIN FCMD; RETURN yytext[0]; }
"%"[a-zA-Z0-9]+			{ yylval.sval = str(yytext); RETURN REGNAME; }
[a-zA-Z0-9]+"%"			{ yylval.sval = sf("%%%.*s", yyleng-1, yytext);
				  RETURN REGNAME; }
"$m"				{ yylval.sval = str(yytext); RETURN dm_CMD; }
{num}":m"			{ yylval.sval = str(yytext); RETURN cm_CMD; }
{num}				{ yylval.sval = str(yytext); RETURN NUM; }
"-"{dnum}			{ yylval.sval = str(yytext); RETURN NNUM; }
{fnum}				{ yylval.sval = str(yytext); RETURN FNUM; }
"*"/[ \t]*"]"			{ yylval.sval = str(yytext); RETURN yytext[0]; }
{PVNAME}":"{num}		{ yylval.sval = str(yytext); RETURN PLINENUM; }
{FNAME}":"{num}			{ yylval.sval = str(yytext); RETURN FLINENUM; }
{PVNAME}":"			{ yylval.sval = str(yytext); RETURN PROCPREF; }
{PVNAME}/{plook}		{ yylval.sval = str(yytext); RETURN PNAME; }
{PVNAME}/{vlook}		{ yylval.sval = str(yytext); RETURN VNAME; }
"?"{PVNAME}?			{ char *p = lineptr;
				  yylval.sval = str(yytext);
				  if ( toknum == 0 ) {
					if ( *p == '/' || *p == '=' ) {
						RETURN VNAME;
					} else {
						yyless(1);
						BEGIN QCMD;
						yylval.sval[1] = '\0';
					}
				  } else {
					while ( *p ) {		/* scan for / or = */
						if ( *p == '/' || *p == '=' ) {
							RETURN VNAME;
						} else if ( *p == ' ' || *p == '\t' ) {
							;
						} else {
							/* no / or = */
							BEGIN QFMT;
							yyless(1);
							yylval.sval[1] = '\0';
							RETURN yytext[0];
						}
						++p;
					}
					printf("CAN'T HAPPEN!  Fell out of while()!\n");
				  }
				  RETURN yytext[0];
				}
{PVNAME}			{ yylval.sval = str(yytext); RETURN VNAME; }
[ \t]+				;
";"				{ yylval.sval = str(";"); BEGIN 0; toknum = 0; return yytext[0]; }
.				{ yylval.sval = str(yytext); RETURN yytext[0]; }
"\n"				{ yylval.sval = str("\\n"); BEGIN 0; toknum = 0; return yytext[0]; }
