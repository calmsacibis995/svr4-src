/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libl:lib/reject.c	1.8"
# include <stdio.h>
extern FILE *yyout, *yyin;
extern int yyprevious , *yyfnd;
extern char yyextra[];
extern char yytext[];
extern int yyleng;
extern struct {int *yyaa, *yybb; int *yystops;} *yylstate [], **yylsp, **yyolsp;
#if defined(__cplusplus) || defined(__STDC__)
int yyreject(void)
#else
yyreject ()
#endif
{
	for( ; yylsp < yyolsp; yylsp++)
		yytext[yyleng++] = yyinput();
	if (*yyfnd > 0)
		return(yyracc(*yyfnd++));
	while (yylsp-- > yylstate) {
		yyunput(yytext[yyleng-1]);
		yytext[--yyleng] = 0;
		if (*yylsp != 0 && (yyfnd= (*yylsp)->yystops) && *yyfnd > 0)
			return(yyracc(*yyfnd++));
	}
	if (yytext[0] == 0)
		return(0);
	yyleng=0;
	return(-1);
}
#if defined(__cplusplus) || defined(__STDC__)
int yyracc(int m)
#else
yyracc(m)
#endif
{
	yyolsp = yylsp;
	if (yyextra[m]) {
		while (yyback((*yylsp)->yystops, -m) != 1 && yylsp>yylstate) {
			yylsp--;
			yyunput(yytext[--yyleng]);
		}
	}
	yyprevious = yytext[yyleng-1];
	yytext[yyleng] = 0;
	return(m);
}
