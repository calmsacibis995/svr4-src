/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ctrace:trace.c	1.8"
/*	ctrace - C program debugging tool
 *
 *	trace code generation routines
 *
 */
#include "global.h"
#include <ctype.h>

#define max(X, Y)	(((int) X > (int) Y) ? X : Y)

/* boolean fields in the marked array */
#define	VAR_START	1
#define	VAR_END		2
#define	EXP_START	4
#define	EXP_END		8

static	enum	bool too_many_vars = no;	/* too many variables to trace */
static	int	marked[STMTMAX];
static	int	vtrace_count = 0;

static	enum bool simple_trace();	/* these necessary for ANSI C */
static  void dump_code();
static  void putvar();
static  void transvar();
static  void transchar();

struct	text {
	int	start, end;
};
static	struct {
	struct	text var;
	struct	text exp;
	enum	trace_type ttype; /* old C compiler does not allow structure member names to be the same */
} vtrace[TRACEMAX];

/* output the statement text as is */
void
puttext(end)
register int	end;
{
	register int	i;

	if (end >= yyleng) {	/* a #if in dcl init list can make end > yyleng */
		(void)fputs(yytext, stdout); /* cannot use puts because it adds a newline */
		yyleng = token_start = 0;
	}
	else {
		for (i = 0; i < end; ++i)
			(void)putchar(yytext[i]);
		(void)strcpy(yytext, yytext + end);
		yyleng = token_start = yyleng - end;
	}
	vtrace_count = 0; /* declarations can contain expressions */
}

/* cleanup after tracing a statement */
void
reset()
{
	vtrace_count = yyleng = token_start = 0;

	/* warn of too many variables to trace in this statement */
	if (too_many_vars) {
		warning("some variables are not traced in this statement");
		too_many_vars = no;
	}
	/* warn of a statement that was too long to fit in the buffer */
	if (too_long) {
		warning("statement too long to trace");
		puttext(yyleng);
		too_long = no;
	}
	return ;
}
/* functions to maintain the variable trace list */

add_fcn(sym, exp_start, exp_end, type)
struct	symbol_struct sym;
int	exp_start, exp_end;
enum	trace_type type;
{
	register int	i;

	/* see if this function is being traced */
	if (!trace_fcn)
		return (0);	/* prevent unnecessary "too many vars" messages */
	
	/* see if the variable is already being traced */
	for (i = vtrace_count - 1; i >= 0; --i)
		if (vtrace[i].exp.start == sym.start && vtrace[i].exp.end == sym.end) {
			break;
		}
	/* if not, use the next trace element */
	if (i < 0)
		if (vtrace_count < tracemax)
			i = vtrace_count++;
		else {
			too_many_vars = yes;
			return (0);
		}
	/* save the trace information */
	vtrace[i].var.start = sym.start;
	vtrace[i].var.end   = sym.end;
	vtrace[i].exp.start = exp_start;
	vtrace[i].exp.end   = exp_end;
	vtrace[i].ttype	   = type;
	return (0);
}
void
expand_fcn(sym, new_start, new_end)
struct	symbol_struct sym;
int	new_start, new_end;
{
	register int	i;

	for (i = vtrace_count - 1; i >= 0; --i)
		if (vtrace[i].exp.start == sym.start && vtrace[i].exp.end == sym.end) {

			/* don't expand ++i to *++i */
			if (sym.type != side_effect) {
				vtrace[i].exp.start = vtrace[i].var.start = new_start;
				vtrace[i].exp.end = vtrace[i].var.end = new_end;
			}
			break;
		}
}
void
rm_trace(sym)
struct	symbol_struct sym;
{
	register int	i;

	for (i = vtrace_count - 1; i >= 0; --i)
		if (vtrace[i].exp.start == sym.start && vtrace[i].exp.end == sym.end) {
			--vtrace_count;
			
			/* eliminate the hole */
			for ( ; i < vtrace_count; ++i)
				vtrace[i] = vtrace[i + 1];
			break;
		}
}
void
rm_all_trace(sym)
struct	symbol_struct sym;
{
	register int	i, j;

	for (i = vtrace_count - 1; i >= 0; --i) {
		if (vtrace[i].exp.start >= sym.start && vtrace[i].exp.start <= sym.end) {
			--vtrace_count;
			
			/* eliminate the hole */
			for (j = i; j < vtrace_count; ++j)
				vtrace[j] = vtrace[j + 1];
		}
	}
}
/* functions to trace the variables in a statement */

void
tr_vars(start, end)
int	start;
register int	end;	/* register variables are in order of use */
{
	register int	i, j;
	static 	enum	 bool	simple_trace();
	
	/* don't trace a statement that was too long to fit in the buffer */
	if (too_long) {
		return ;
	}
	/* mark the significant characters */
	for (i = start; i < end; ++i)
		marked[i] = 0;
	for (i = 0; i < vtrace_count; ++i) {
		marked[vtrace[i].exp.start] |= EXP_START;
		marked[vtrace[i].exp.end]   |= EXP_END;
		marked[vtrace[i].var.end]   |= VAR_END;
	}
	/* for each character in the statement */
	for (i = start; i < end; ++i) {
		if (marked[i] & EXP_END)

			/* check for the end of a traced expression */
			for (j = 0; j < vtrace_count; ++j)
				if (vtrace[j].exp.end == i) {
					if (!simple_trace(j)) {
						(void)printf(", ");
						dump_code(j);
						transvar(vtrace[j].var);
					}
					(void)printf("), ");
					if (vtrace[j].ttype == strres)
						(void)printf("_ct");
					else
						transvar(vtrace[j].var);
	
					/* check for a postfix ++/-- operator */
					if (vtrace[j].ttype == postfix) {
						if (yytext[vtrace[j].exp.end - 1] == '+')
							(void)putchar('-');
						else
							(void)putchar('+');
						(void)putchar('1');
					}
					(void)putchar(')');
				}
		if (marked[i] & EXP_START)
	
			/* check for the start of a traced expression */
			for (j = 0; j < vtrace_count; ++j)
				if (vtrace[j].exp.start == i) {
					(void)putchar('(');
					if (simple_trace(j))
						dump_code(j);
					else if (vtrace[j].ttype == strres)
						(void)printf("_ct = ");
				}
		/* output this character */
		(void)putchar(yytext[i]);
	}
	return ;
}
/* check for a simple variable trace */

static enum bool 
simple_trace(j)
register int	j;
{
	register int	k;
	
	if (vtrace[j].exp.start == vtrace[j].var.start &&
	    vtrace[j].exp.end == vtrace[j].var.end) {
		for (k = 0; k < vtrace_count; ++k)
			if (k != j) {	/* if var is not this var */
				if (vtrace[k].exp.start >= vtrace[j].var.start &&
				    vtrace[k].exp.end <= vtrace[j].var.end)
				break;	/* this var has an embedded var */
				if (vtrace[k].exp.start == vtrace[j].var.start ||
				    vtrace[k].exp.end == vtrace[j].var.end)
				break;	/* this var is part of a larger var */
			}
		if (k == vtrace_count)
			return(yes);
	}
	return(no);
}
/* dump routine call */

static void
dump_code(j)
register int	j;
{
	register int	k;
	register char	c;
	
	if (vtrace[j].ttype == string || vtrace[j].ttype == strres)
		(void)printf("s_ct_(\"");
	else	/* unknown type */
		(void)printf("u_ct_(\"");
	for (k = 0; (c = indentation[k]) != '\0'; ++k)
		transchar(c);
	(void)printf("/* ");
	putvar(vtrace[j].var);
	(void)printf("\",");
	
	/* size of variable code */
	if (vtrace[j].ttype != string && vtrace[j].ttype != strres) {
		(void)printf("sizeof(");
		transvar(vtrace[j].var);
		(void)printf("),");
	}
}
/* output the variable's name */

static void
putvar(var)
struct	text var;
{
	register int	i;
	
	for (i = var.start; i < var.end; ++i)
		transchar(yytext[i]);
}
/* translate sub-exps containing '=', '++', and '--' ops into repeatable exps */

static void
transvar(var)
struct	text var;
{
	register int	i, j;
	
	/* don't check the first character for start of an expression */
	(void)putchar(yytext[var.start]);

	for (i = var.start + 1; i < var.end; ++i) {
		if (marked[i] & VAR_END)
		
			/* check for the end of a traced variable */
			for (j = 0; j < vtrace_count; ++j)
				if (vtrace[j].var.end == i) {

					/* transform a postfix ++/-- operator into -/+ 1 */
					if (vtrace[j].ttype == postfix) {
						if (yytext[vtrace[j].exp.end - 1] == '+')
							(void)putchar('-');
						else
							(void)putchar('+');
						(void)printf("1)");

						/* skip the postfix operator */
						i = max(i, vtrace[j].exp.end - 1);
						goto skip;
					}
					/* skip the rest of a assignment expression */
					if (vtrace[j].ttype == assign) {
						i = max(i, vtrace[j].exp.end - 1);
						goto skip;
					}
				}
		if (marked[i] & EXP_START)

			/* check for the start of a traced expression */
			for (j = 0; j < vtrace_count; ++j)
				if (vtrace[j].exp.start == i) {

					/* insert a '(' in front of a postfix ++/-- expression */
					if (vtrace[j].ttype == postfix)
						(void)putchar('(');

					/* skip a prefix ++/-- operator */
					if (vtrace[j].ttype == prefix) {
						i = max(i, vtrace[j].var.start - 1);
						goto skip;
					}
				}
		/* output this character */
		(void)putchar(yytext[i]);
skip:		;
	}
}
/* trace the statement text */

void
tr_stmt(lineno, stmt, putsemicolon)
int	lineno;
char	*stmt;
enum	bool putsemicolon;
{
	register int	c, i;

	/* see if this function is being traced and */
	/* don't trace a statement that was too long to fit in the buffer */
	if (!trace_fcn || too_long) {
		return ;
	}
	/* output the trace function call */
	(void)printf("t_ct_(\"");

	/* if the line number field should be blank */
	if (lineno == NO_LINENO) {

		/* print a blank line number */
		(void)printf("\\n    ");

		/* copy the leading blanks and tabs in the loop statement */
		if (stmt[0] == '\n') /* skip a leading newline */
			i = 1;
		else
			i = 0;
		for ( ; isspace(c = stmt[i]); ++i)
			transchar(c);

		i = 0; /* there isn't a newline in the statement text */
	}
	/* if the statement starts on a new line */
	else if (stmt[0] == '\n') {

		/* adjust the line number for any embedded new lines */
		for (i = 1; (c = stmt[i]) != '\0'; ++i)
			if (c == '\n')
				--lineno;

		/* print the line number */
		(void)printf("\\n%3d ", lineno);
		i = 1; /* skip the newline in the statement text */
	}
	else
		i = 0;

	/* convert special characters in the statement text */
	for ( ; (c = stmt[i]) != '\0'; ++i)
		transchar(c);

	/* terminate the trace function call */
	(void)printf("\")");

	/* see if this is to be a statement */
	if (putsemicolon)
		(void)printf("; ");
	return ;
}

/* translate a character for a string constant */

static void
transchar(c)
register int	c;
{
	switch (c) {
	case '"':
		(void)printf("\\\"");
		break;
	case '\\':
		(void)printf("\\\\");
		break;
	case '\t':
		(void)printf("\\t");
		break;
	case '\n':
		(void)printf("\\n");
		break;
	default:
		(void)putchar(c);
	}
}
