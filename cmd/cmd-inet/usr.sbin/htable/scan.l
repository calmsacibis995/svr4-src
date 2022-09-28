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
#ident	"@(#)cmd-inet:usr.sbin/htable/scan.l	1.1.4.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


#include "y.tab.h"
#include "htable.h"
%}

BLANK	[ \t]
DIGIT	[0-9]
ALPHA	[A-Za-z]
ANUM	[0-9A-Za-z]
NAMECHR	[0-9A-Za-z./-]

%%
"NET"		{
			yylval.number = KW_NET;
			return (KEYWORD);
		}

"GATEWAY"	{
			yylval.number = KW_GATEWAY;
			return (KEYWORD);
		}

"HOST"		{
			yylval.number = KW_HOST;
			return (KEYWORD);
		}

{ALPHA}{NAMECHR}*{ANUM}	{
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{ALPHA}		{
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{DIGIT}+{ALPHA}{NAMECHR}* {
			fprintf(stderr, "Warning: nonstandard name \"%s\"\n",
				yytext);
			yylval.namelist = newname(yytext);
			return (NAME);
		}

{DIGIT}+	{
			yylval.number = atoi(yytext);
			return (NUMBER);
		}

"."		return ('.');
":"		return (':');
","		return (',');
"/"		return ('/');
";".*		;
"\n"{BLANK}+	;
{BLANK}+	;
"\n"		return (END);
.		fprintf(stderr, "Illegal char: '%s'\n", yytext);

%%

yywrap()
{
	return (1);
}
