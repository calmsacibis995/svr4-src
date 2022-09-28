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
#ident	"@(#)cmd-inet:usr.sbin/slink/slink.l	1.1.4.1"

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

/*
 *
 *	Copyright 1987, 1988 Lachman Associates, Incorporated (LAI)
 *	All Rights Reserved.
 *
 *	The copyright above and this notice must be preserved in all
 *	copies of this source code.  The copyright above does not
 *	evidence any actual or intended publication of this source
 *	code.
 *
 *	This is unpublished proprietary trade secret source code of
 *	Lachman Associates.  This source code may not be copied,
 *	disclosed, distributed, demonstrated or licensed except as
 *	expressly authorized by Lachman Associates.
 *
 *	System V STREAMS TCP was jointly developed by Lachman
 *	Associates and Convergent Technologies.
 */
#include "defs.h"
#define yywrap() (1)
%}

%%

[_A-Za-z][_A-Za-z0-9]*			{
						strval = savestr(yytext);
						return(T_NAME);
					}

\$[1-9][0-9]?				{
						intval = atoi(&yytext[1]) - 1;
						return(T_PARAM);
					}

\"([^"\\\n]|(\\[^\n]))*\"		{
						dostring();
						return(T_STRING);
					}

([^"={}\\ \t\n]|(\\[^ \t\n]))+		{
						dostring();
						return(T_STRING);
					}

(#[^\n]*)?\n				{
						line++;
						return(T_EOL);
					}


\\\n					{
						line++;
					}

[ \t]					;

.					{
						return(yytext[0]);
					}

%%

dostring()
{
	char *p, *ep, *s;

	strval = xmalloc(yyleng+1);
	/* copy string, removing quotes & backslashes */
	for (p = yytext, ep = p + yyleng, s = strval; p < ep; p++) {
		if (*p == '"')
			continue;
		if (*p == '\\')
			p++;
		*s++ = *p;
	}
	*s = '\0';
}
