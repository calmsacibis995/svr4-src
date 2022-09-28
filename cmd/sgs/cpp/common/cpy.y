/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

%{
#ident	"@(#)cpp:common/cpy.y	1.5"
%}
%{
%}
%term number stop DEFINED
%term EQ NE LE GE LS RS
%term ANDAND OROR
%left ','
%right '='
%right '?' ':'
%left OROR
%left ANDAND
%left '|' '^'
%left '&'
%binary EQ NE
%binary '<' '>' LE GE
%left LS RS
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' UMINUS
%left '(' '.'
%%
S:	e stop	={return($1);}


e:	  e '*' e
		={$$ = $1 * $3;}
	| e '/' e
		={
			if ($3 == 0) {
				ppwarn("division by zero");
				$$ = 0;
			}
			else
				$$ = $1 / $3;
		}
	| e '%' e
		={
			if ($3 == 0) {
				ppwarn("remainder by zero");
				$$ = 0;
			}
			else
				$$ = $1 % $3;
		}
	| e '+' e
		={$$ = $1 + $3;}
	| e '-' e
		={$$ = $1 - $3;}
	| e LS e
		={$$ = $1 << $3;}
	| e RS e
		={$$ = $1 >> $3;}
	| e '<' e
		={$$ = $1 < $3;}
	| e '>' e
		={$$ = $1 > $3;}
	| e LE e
		={$$ = $1 <= $3;}
	| e GE e
		={$$ = $1 >= $3;}
	| e EQ e
		={$$ = $1 == $3;}
	| e NE e
		={$$ = $1 != $3;}
	| e '&' e
		={$$ = $1 & $3;}
	| e '^' e
		={$$ = $1 ^ $3;}
	| e '|' e
		={$$ = $1 | $3;}
	| e ANDAND e
		={$$ = $1 && $3;}
	| e OROR e
		={$$ = $1 || $3;}
	| e '?' e ':' e
		={$$ = $1 ? $3 : $5;}
	| e ',' e
		={$$ = $3;}
	| term
		={$$ = $1;}
term:
	  '-' term %prec UMINUS
		={$$ = -$2;}
	| '!' term
		={$$ = !$2;}
	| '~' term
		={$$ = ~$2;}
	| '(' e ')'
		={$$ = $2;}
	| DEFINED '(' number ')'
		={$$= $3;}
	| DEFINED number
		={$$ = $2;}
	| number
		={$$= $1;}
%%
