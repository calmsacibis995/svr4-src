/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:shlib/streval.c	1.2.3.1"
/*
 * arithmetic expression evaluator
 *
 * NOTE: all operands are evaluated as both the parse
 *	 and evaluation are done on the fly
 */

#ifdef KSHELL
#   include	"shtype.h"
#else
#   include	<ctype.h>
#endif	/* KSHELL */
#include "streval.h"

#define MAXLEVEL	9

struct vars			 /* vars stacked per invocation		*/
{
	char*		nextchr; /* next char in current expression	*/
	char*		errchr;	 /* next char after error		*/
	struct lval	errmsg;	 /* error message text			*/
	char*		errstr;  /* error string			*/
#ifdef FLOAT
	char		isfloat; /* set when floating number		*/
#endif /* FLOAT */
};


#define getchr()	(*cur.nextchr++)
#define peekchr()	(*cur.nextchr)
#define ungetchr()	(cur.nextchr--)

#define pushchr(s)	{struct vars old;old=cur;cur.nextchr=(s);cur.errmsg.value=0;cur.errstr=0
#define popchr()	cur=old;}
#define error(msg)	return(seterror(msg))

extern struct Optable optable[];

static struct vars	cur;
static char		noassign;	/* set to skip assignment	*/
static int		level;
static number	(*convert)();	/* external conversion routine		*/

static number		expr();		/* subexpression evaluator	*/
static number		seterror();	/* set error message string	*/
static struct Optable	*findop();	/* lookup operator		*/


/*
 * evaluate an integer arithmetic expression in s
 *
 * (number)(*convert)(char** end, struct lval* string, int type) is a user supplied
 * conversion routine that is called when unknown chars are encountered.
 * *end points to the part to be converted and must be adjusted by convert to
 * point to the next non-converted character; if typ is ERRMSG then string
 * points to an error message string
 *
 * NOTE: (*convert)() may call streval()
 */

number
streval(s, end, conv)
char*	s;
char**	end;
number	(*conv)();
{
	number	n;

	pushchr(s);
#ifdef FLOAT
	cur.isfloat = 0;
#endif /* FLOAT */
	convert = conv;
	if(level++ >= MAXLEVEL)
		(void)seterror(e_recursive);
	else
	{
		n = expr(0);
		if (peekchr() == ':') (void)seterror(e_badcolon);
	}
	if (cur.errmsg.value)
	{
		if(cur.errstr) s = cur.errstr;
		(void)(*convert)( &s , &cur.errmsg, ERRMSG);
		cur.nextchr = cur.errchr;
		n = 0;
	}
	if (end) *end = cur.nextchr;
	if(level>0) level--;
	popchr();
	return(n);
}

/*   
 * evaluate a subexpression with precedence
 */

static number
expr(precedence)
register int	precedence;
{
	register int	c;
	register number	n;
	register number	x;
	register struct Optable *op;
	int incr = 0;
	struct lval	assignop;
	struct lval	lvalue;
	char*		pos;
	char		invalid=0;

	while ((c=getchr()) && isspace(c));
	switch (c)
	{
	case 0:
		if(precedence>5)
			error(e_moretokens);
		return(0);

#ifdef future
	case '-':
		incr = -2;
	case '+':
		incr++;
		if(c != peekchr())
		{
			/* unary plus or minus */
			n = incr*expr(2*MAXPREC-1);
			incr = 0;
		}
		else /* ++ or -- */
		{
			invalid = 1;
			getchr();
		}
		break;
#else
	case '-':
		n = -expr(2*MAXPREC-1);
		break;
#endif

	case '!':
		n = !expr(2*MAXPREC-1);
		break;
	case '~':
#ifdef FLOAT
		if(cur.isfloat)
			error(e_incompatible);
#endif /* FLOAT */
		n = ~(long)expr(2*MAXPREC-1);
		break;
	default:
		ungetchr();
		invalid = 1;
		break;
	}
	lvalue.value = 0;
	while(1)
	{
		cur.errchr = cur.nextchr;
		if((c=getchr()) && isspace(c))
			continue;
		assignop.value = 0;
		if(isalnum(c))
			goto alphanumeric;
		op = findop(c);
		/* check for assignment operation */
		if(peekchr()== '=' && !(op->precedence&NOASSIGN))
		{
			if(!noassign && (!lvalue.value || precedence > 2))
				error(e_notlvalue);
			assignop = lvalue;
			getchr();
		}
		/* from here on c is the new precedence level */
		c = (op->precedence&PRECMASK);
		c *= 2;
		if(lvalue.value && (op->opcode!=ASSIGNMENT))
		{
			n = (*convert)(&cur.nextchr, &lvalue, VALUE);
			if(cur.nextchr==0)
				error(e_number);
			if(!(op->precedence&SEQPOINT))
				lvalue.value = 0;
			invalid = 0;
		}
		if(invalid && op->opcode>ASSIGNMENT)
			error(e_synbad);
		if(precedence >= c)
			goto done;
		if(op->precedence&RASSOC)
			c--;
		if(c < 2*MAXPREC && !(op->precedence&SEQPOINT))
			x = expr(c);
#ifdef FLOAT
		if((op->precedence&NOFLOAT)&& cur.isfloat)
			error(e_incompatible);
#endif /* FLOAT */
		switch(op->opcode)
		{
		case RPAREN:
			error(e_paren);

		case LPAREN:
		{
#ifdef FLOAT
			char savefloat = cur.isfloat;
			cur.isfloat = 0;
#endif /* FLOAT */
			n = expr(2);
#ifdef FLOAT
			cur.isfloat = savefloat;
#endif /* FLOAT */
			if (getchr() != ')')
				error(e_paren);
			break;
		}

#ifdef future
		case PLUSPLUS:
			incr = 1;
			goto common;
		case MINUSMINUS:
			incr = -1;
		common:
			x = n;
#endif
		case ASSIGNMENT:
			if(!noassign && !lvalue.value)
				error(e_notlvalue);
			n = x;
			assignop = lvalue;
			lvalue.value = 0;
			break;

#ifdef future
		case QUEST:
		{
			register int nextc;
			if(n)
			{
				x = expr(c);
				nextc = getchr();
			}
			noassign++;
			(void)expr(c);
			noassign--;
			if(!n)
			{
				nextc = getchr();
				x = expr(c);
			}
			n = x;
			if (nextc == ':')
				break;
		}
		case COLON:
			(void)seterror(e_badcolon);
			break;
#endif

		case OR:
			n = (long)n | (long)x;
			break;

		case QCOLON:
		case OROR:
			if(n)
			{
				noassign++;
				expr(c);
				noassign--;
			}
			else
				n = expr(c);
#ifdef future
			if(op->opcode==OROR)
#endif
				n = (n!=0);
			break;

		case XOR:
			n = (long)n ^ (long)x;
			break;

		case NOT:
			error(e_synbad);

		case AND:
			n = (long)n & (long)x;
			break;

		case ANDAND:
			if(n==0)
			{
				noassign++;
				expr(c);
				noassign--;
			}
			else
				n = (expr(c)!=0);
			break;

		case EQ:
			n = n == x;
			break;

		case NEQ:
			n = n != x;
			break;

		case LT:
			n = n < x;
			break;

		case LSHIFT:
			n = (long)n << (long)x;
			break;

		case LE:
			n = n <= x;
			break;

		case GT:
			n = n > x;
			break;

		case RSHIFT:
			n = (long)n >> (long)x;
			break;

		case GE:
			n = n >= x;
			break;

		case PLUS:
			n +=  x;
			break;

		case MINUS:
			n -=  x;
			break;

		case TIMES:
			n *=  x;
			break;

		case DIVIDE:
			if(x!=0)
			{
				n /=  x;
				break;
			}

		case MOD:
			if(x!=0)
				n = (long)n % (long)x;
			else
				error(e_divzero);
			break;

		default:
		alphanumeric:
			pos = --cur.nextchr;
			n = (*convert)(&cur.nextchr, &lvalue, LOOKUP);
			if (cur.nextchr == pos)
			{
				if(cur.errmsg.value = lvalue.value)
					cur.errstr = pos;
				error(e_synbad);
			}
#ifdef future
			/* this handles ++x and --x */
			if(incr)
			{
				if(lvalue.value)
					n = (*convert)(&cur.nextchr, &lvalue, VALUE);
				n += incr;
				incr = 0;
				goto common;
			}
#endif
			break;
		}
		invalid = 0;
#ifdef FLOAT
		if((long)n != n)
			cur.isfloat++;
#endif /* FLOAT */
		if(!noassign && assignop.value)
			(void)(*convert)(&cur.nextchr,&assignop,ASSIGN,n+incr);
		incr = 0;
	}
 done:
	cur.nextchr = cur.errchr;
	return(n);
}

/*
 * set error message string
 */

static number
seterror(msg)
char*	msg;
{
	if(!cur.errmsg.value)
		cur.errmsg.value = msg;
	cur.errchr = cur.nextchr;
	cur.nextchr = "";
	level = 0;
	return(0);
}

/*
 * look for operator in table
 */

static
struct Optable *findop(c)
register int c;
{
	register struct Optable *op = optable;
	if(c > '|')
		return(op);
	while(++op)
	{
		if(c > op->name[0])
			continue;
		if(c < op->name[0])
			return(optable);
		if(op->name[1]==0)
			break;
		c = getchr();
		if(op->name[1]==c)
			break;
		if((++op)->name[1]==c)
			break;
		if(op->name[1])
			op++;
		ungetchr();
		break;
	}
	return(op);
}

#ifdef FLOAT
set_float()
{
	cur.isfloat++;
}
#endif /* FLOAT */
