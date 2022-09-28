/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident   "@(#)acpp:common/predicate.c	1.22"
/* predicate.c */

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "cpp.h"
#include "file.h"
#include "predicate.h"

#ifndef PREASPREDS		/* pre-asserted predicates */
#	define PREASPREDS ""
#endif
#define	STKSZ		5
#define	NOT_FOUND	0
/* Predicate.c implements the separate name space that is referred
** to by the #assert and #unassert directive predicates.
*/
/* Design Notes:
**
** 	The design of this symbol table was based on the
** assumption that there would be few predicates
** asserted in a "real" program. 
** This is most certainly true for existing programs
** that used C Issue 4.2. Since the #[un]assert directives
** are ANSI C extensions, any user who wants to
** write portable software wouldn't want to
** use #assert in the future.
** In these cases, the only predicates to exist
** would be the pre-asserted ones.
** I expect ~400 macros in "real" programs,
** compared to ~4 predicates.
** Since #assert directives are expected to be
** scarce, the space-efficient linear list implementation is used
** instead of a hashed symbol table.
**
** 	Since this preprocessor has another symbol
** table in syms.c, it may seem advisable to merge
** both into a single table that maintains both name spaces.
** I suppose that argument might be forwarded in the
** hope of saving space. I decided against the single
** table approach for many reasons:
**
**	1) Utilizing the hash table in syms.c is
**	overkill for this presumably small name space.
**	2) The size of syms.c's data structure would
**	be increased by at least 4 bytes per macro definition
**	(for a pointer to a list of assertions),
**	wasting space that would often go unused.
**	Likewise, the `Macro' data structure is far
**	larger than needed to record predicate information.
**	3) Since the name spaces have to be distinct,
**	presumably a name would be tagged as belonging
**	to one or both spaces by bits in a `flags' field.
**	Name entry and search would be slowed down in
**	both name spaces to set, clear, or test the bits.
**	Since macro lookups happen often and are a significant
**	part of the time spent in a preprocessor,
**	it would be foolish to slow macro table operations.
**	4) The architecture of this preprocessor is modular.
**	The design is flexible, and assumes that new directives
**	and the objects that they operate on will be
**	implemented in new modules.
**	This design is easier to understand, and to maintain.
**	Simply, if you've got a problem in this name space,
**	come here, else go to syms.c. But. in any event,
**	be assured the you 
**	can't pollute one name space will "fixing" another.
**	5) Since this table and the specialized routines
**	that access it take up such little space, there
**	aren't much savings to be had.
*/
/* Terminology:
** 
**	<-------------- an "assertion" ----------------->
**					<-"pptokens"->
** 	# assert	machine	(	IBM   /    370	)
**			^
**			|
**		 	the "predicate"
*/
typedef struct _assert_ Assert;

typedef struct _predicate_ {
	char *	name;		/* spelling of predicate, followed by `\0' */
	Assert*	list;		/* list of assertions on this predicate	*/
} Predicate;

struct _assert_ {
	Assert*	next;		/* next in linked list of assertions */
	char *	pptokens;	/* canonicalized spelling of pp-tokens, followed by `\0' */
};

/* These variables are used to implemnent a dynamic table of
** predicate names and the assertions on them.
*/
static	Predicate* stk;		/* stack of predicate names	*/
static	int	sx;		/* `stk' index(next available)	*/
static	int	sxlimit;	/* limit to `stk' index	*/	
/* Here's a depiction of what the dynamic table looks like,
** after procesing the following three lines (with no preasserted predicates):
**	 #assert	m( 1 )
**	 #assert	m(PDP      7)
**	 #assert	x(a)
**	
**	 sxlimit+
**	 stk ->
**		________
**		|	|
**		|NOTUSED|
**		|-------|
**		|NOTUSED|
**		|_______|
**		~	~
**		~	~
**	 stk+	|-------|
**	 sx ->	|NOTUSED|
**		|_______|	_________
**		|	|->  	|  "a"	|-0
**		| "x"	|	|_______|
**		|-------|	_________	_________
**	 stk->	| "m"	|->	|"PDP 7"|->	|  "1"	|-0
**		|_______|	|_______|	|_______|
**		
** NOTE: the pp-token arguments to the predicate name are cononicalized.
** Leading and trailing white space are eliminated,
** as in `#assert	m( 1 )` example.
** All white space sequences are considered to be a single space,
** as in the `#assert	m(PDP      7)` example.
*/

static	void		enter( /* Token *, int */ );
static  Predicate *	findpredicate( /* token * */ );
static	Assert **	findpptokens( /* Predicate *, char * */ );
static	void		forget( /* Token *, int */ );
static  Predicate * 	newpredicate( /* token * */ );
static	void		newpptokens( /* Assert *, char * */ );
#ifdef DEBUG
static	void		printassertions( /* Predicate * */ );
#endif

static void
enter(itp, len)
	Token * itp;
	int len;
/* Given a pointer to a canonicalized assertion Token list
** and the number of characters needed to store the characters,
** this routine enters the new assertion into assertion table.
** It is assumed that neither argument is 0.
*/
{
	register Token * tp;
	register Predicate * pp;
	register char * pptokens;
	char *	cp;

#ifdef DEBUG
	if (DEBUG('p') > 1)
	{
		(void)fprintf(stderr, "enter(len=%d) with itp:", len);
		tk_prl(itp);
	}
#endif
	if ((pp = findpredicate(tp = itp)) == 0)
		pp = newpredicate(tp);
	if ((tp = tk_rm(tp)) == (Token *)0)
		return;
	else
	{
		pptokens = cp = ch_alloc((unsigned)(len + 1));
		/* for now - tk_merge() */
		do
		{	(void) memcpy(cp, tp->ptr.string, tp->rlen);
			cp += tp->rlen;
		} while ((tp = tk_rm(tp)) != (Token *)0);
		*cp = '\0';
	}
	if ( findpptokens(pp, pptokens) == NOT_FOUND)
		newpptokens(pp, pptokens);
#ifdef DEBUG
	if (DEBUG('p') > 2)	printassertions(pp);
#endif
}

static Predicate *
findpredicate(tp)
	Token * tp;
/* Given a pointer to an identifier Token,
** this routine returns a pointer to the predicate
** entry in the symbol table, or 0 is there is none.
*/
{
	register int i;

#ifdef DEBUG
	if (DEBUG('p') > 1)
		(void)fprintf(stderr, "findpredicate(tp=%#lx=`%.*s') ",
		 tp, tp->rlen, tp->ptr.string );
#	define	DBGRET(retval)	if (DEBUG('p') > 1)\
		 (void)fprintf(stderr, "returns %#lx\n", (retval))
#else
#	define	DBGRET(retval)
#endif
	COMMENT(tp != 0);
	for (i = 0; i < sx; i++)
	{
		if (strlen(stk[i].name) == tp->rlen
		 && strncmp(tp->ptr.string, stk[i].name, tp->rlen) == 0)
		{
			DBGRET(stk + i);
			return stk + i;
		}
	}
	DBGRET(0);
	return (Predicate *)0;
}

static	Assert **
findpptokens(pp, cp)
	Predicate * pp;
	char * cp;
/* Given a pointer to a predicate entry in the symbol table
** and a string containing a canonicalized `pptokens' argument.
** this routine returns a pointer to a pointer
** of the corresponding entry in the symbol table.
** If there is no corresponding entry,
** 0 is returned.
*/
{
	register Assert ** app;

	COMMENT(cp != 0);
	COMMENT(pp != 0);
#ifdef DEBUG
	if (DEBUG('p') > 1)
		(void)fprintf(stderr, "findpptokens(pp=%#lx, cp=`%s')\n", pp, cp);
#endif
	for (app = &(pp->list); *app != 0; app = &((*app)->next))
	{
		if (strcmp((*app)->pptokens, cp) == 0)
			return app;
	}
	return (Assert **) NOT_FOUND;
}

static void
forget(itp, len)
	Token * itp;
	int len;
/* Given a pointer to a canonicalized Token list from
** a #unassert directive and the length of characters
** in the `pptokens', this routine deletes the
** corresponding entry from the symbol table.
** If there is no corresponding entry,
** no action is performed or diagnostic given.
*/
{
	register Token * tp;
	register Predicate * pp;

#ifdef DEBUG
	if (DEBUG('p') > 1)
	{
		(void)fprintf(stderr, "forget(len=%d) with itp:", len);
		tk_prl(itp);
	}
#endif
	if ((pp = findpredicate(tp = itp)) == 0)
	/*EMPTY*/	;
	else if ((tp = tk_rm(tp)) == (Token *)0)
	{
		register Assert * ap;
		Assert * nextap;

		for (ap = pp->list; ap != 0; ap = nextap)
		{
			nextap = ap->next;
			/*free(ap->pptokens);*/
			free(ap);
		}
		pp->list = 0;
	}
	else
	{
		register Assert ** app;
		register char *	cp;
		char * pptokens;

		pptokens = cp = ch_alloc((unsigned)(len + 1));
		/* for now - tk_merge() ? */
		do
		{
			(void) memcpy(cp, tp->ptr.string, tp->rlen);
			cp += tp->rlen;
		} while ((tp = tk_rm(tp)) != (Token *)0);
		*cp = '\0';
		if ((app = findpptokens(pp, pptokens)) != 0)
		{
			Assert * ap;

			ap = *app;
			*app = ap->next;
			/*free(ap->pptokens); */
			free(ap);
		}
	}
#ifdef DEBUG
	if (DEBUG('p') > 2)	if (pp != 0)	printassertions(pp);
#endif
	return;
}

static Predicate *
newpredicate(tp)
	Token * tp;
/* Given a pointer to an identifier Token,
** this routine creates a new predicate entry
** of the given name in the symbol table.
*/
{
	register Predicate * pp;

#ifdef DEBUG
	if (DEBUG('p') > 1)
		(void)fprintf(stderr, "newpredicate(tp=%#lx=`%.*s') ",
		 tp, tp->rlen, tp->ptr.string );
#endif
	COMMENT(tp != 0);
	COMMENT(tp->code == C_Identifier);
	if ((sx + 1) == sxlimit)
		stk = (Predicate *)pp_realloc((char*) stk, (sxlimit += STKSZ) * sizeof(Predicate));
	pp = stk + sx++;
	pp->name = ch_saven(tp->ptr.string, tp->rlen);
	pp->name[tp->rlen] = '\0';
	pp->list = 0;
#ifdef DEBUG
	if (DEBUG('p') > 1)
		(void)fprintf(stderr, "returns pp=%#lx\n", pp);
#endif
	return pp;
}
		
static	void
newpptokens(pp, cp)
	Predicate * pp;
	char * cp;
/* Given a pointer to a predicate in the symbol table
** and a string containing the canonicalized spelling of the `pptokens',
** this routine enters the assertion into the symbol table..
*/
{
	register Assert * ap;

#ifdef DEBUG
	if (DEBUG('p') > 1)
		(void)fprintf(stderr, "newpptokens(ap=%#lx, cp=`%s')\n", ap, cp);
#endif
	ap = (Assert *)pp_malloc(sizeof(Assert));
	ap->pptokens = cp;
	ap->next = pp->list;
	pp->list = ap;
}

Token *
pd_assert(itp)
	Token * itp;
/* Handles #assert directives, and diagnoses syntax errors.
** It is assumed that the first input Token is C_Sharp.
** Modifies the input token list by: stripping the leading and trailing
** parentheses (if any); canonicalizing white space (if any) within the sequence
** of predicate argument pp-tokens; stripping all other white space.
** If the directive is syntactically correct, the modified 
** input token list is passed to enter(). 
*/
{
	register Token * tp;	/* a token in the `itp' linked list	*/
	register Token * prev;	/* token preceeding `tp'		*/
	register int len;	/* length of canonicalized pptokens	*/

#ifdef DEBUG
	if (DEBUG('p') > 0)
		(void)fprintf(stderr, "pd_assert(itp=%#lx=`%.*s')\n",
		 itp, itp ? itp->rlen : 4, itp ? itp->ptr.string : "NULL");
#endif
	len = 0;
	if ((tp = itp) == 0)
	{
		UERROR( "empty #assert directive" );
		return (Token *)0;
	}
	else if (tp->code != C_Identifier)
	{
		UERROR( "\"#assert identifier\" expected" );
		return tp;
	}
	tp = (prev = tp)->next;
	if ((prev->next = tp = tk_rmws(tp)) == 0)
		return itp;
	else if (tp->code != C_LParen)
	{
		UERROR( "\"#assert identifier (...\" expected" );
		return itp;
	}
	else
	{
		prev->next = tp = tk_rm(tp);
		if ((prev->next = tp = tk_rmws(tp)) == 0)
		{
			UERROR( "no tokens following \"#assert name (\"" );
			return itp;
		}
		else if (tp->code == C_RParen)
		{
			UERROR( "missing tokens between parentheses" );
			return itp;
		}
		else
		{
			register int parendepth;

			for (parendepth = 1; ; tp = (prev = tp)->next)
			{
				if (tp == 0)
				{
					UERROR( "\"#assert\" missing \")\"" );
					return itp;
				}
				switch (tp->code)
				{
				case C_WhiteSpace:
					tp->ptr.string = " ";
					tp->rlen = 1;
					len++;
					if ((tp->next = tk_rmws(tp->next)) == 0)
						continue;
					else if ((tp->next->code == C_RParen) && parendepth == 1)
					{
						prev->next = tk_rm(tp);
						tp = prev;
						len--;
					}
					continue;

				case C_LParen:
					parendepth++;
					break;

				case C_RParen:
					if (--parendepth == 0)
						goto out;
				default:break;
				}
				len += tp->rlen;
				continue;
			}
		}
	}
out:	if ( prev->next != (Token *)0 )
	{
		COMMENT(tp->code == C_RParen);
		prev->next = tk_rm(prev->next);
		tk_extra(prev->next);
		prev->next = 0;
	}
	enter(itp, len);
	return (Token *)0;
}

void
pd_preassert()
/*
** The static array preasserted is inititalized to contain the value of 
** the macro PREASPRED, whic is set in the makefile. These are the preasserted
** predicates like "system(unix)" and "machine(u3b2)".
**
*/
{
	static char * preasserted[] = {PREASPREDS};

	if (preasserted != 0)
	{
		register char ** cpp;	/* points to a member of predefined[] */

		cpp = preasserted + (sizeof(preasserted)/sizeof(char *)) - 1;
		do
			pd_option(*cpp);
		while (cpp-- != preasserted);
	}
}

void
pd_init()
/* Initialize the data structures in predicate.c */
{
	stk = (Predicate *)pp_malloc(sizeof(Predicate) * STKSZ);
	COMMENT(sx == 0);
	sxlimit = STKSZ;
}

void
pd_option(arg)
	char * arg;
/* Given the pointer to the argument string of a -A command line option,
** this routine enforces the option syntax and fulfills the semantics.
*/
{
	register Token * tp;	/* Token in option argument */

#ifdef DEBUG
	if (DEBUG('p') > 0)
		(void)fprintf(stderr, "pd_option(arg=%s)\n", arg);
#endif
	tp = tk_tokenize(arg);
#ifdef DEBUG
	if (DEBUG('p') > 1)	tk_prl(tp);
#endif
	switch (tp->code)
	{
	case C_Identifier:
		(void) pd_assert(tp);
		break;

	case C_Minus:
		pp_nodefaults();
		if ((tp = tk_rm(tp)) != 0)
		{
			WARN( "tokens after -A- are ignored" );
			tk_rml(tp);
		}
		break;

	default:UERROR( "identifier or \"-\" expected after -A" );
		tk_rml(tp);
		break;
	}
}

Token *
pd_replace(inputp)
	Token * inputp;		/* also used to hold return value */
/* Given a pointer to a `#' operator in a sub-expression
** in a conditional inclusion directive, this routine replaces
** a predicate sub-expression with its value (0 or 1).
** This routine diagnoses any syntax errors within the
** sub-expression.
*/
{
	register Token * tp;	/* a token in the itp linked list	*/
	register Token * prev;	/* a token preceeding `tp'		*/
	register int parendepth;/* number_of('(') - number_of(')')	*/
	register int len;	/* length of canonicalized pptokens	*/
	Predicate * pp;		/* predicate that is being tested	*/
	Token * argp;		/* beginning of argument pptokens	*/

	COMMENT(inputp != 0);
	COMMENT(inputp->code == C_Sharp);
	if ((tp = tk_rmws(tk_rm(inputp))) == 0)
	{
		UERROR( "identifier expected after \"#\"" );
		return tk_bool(1);
	}
	else if (tp->code != C_Identifier)
	{
		UERROR( "identifier expected after \"#\"" );
		return tp;
	}
	else
		pp = findpredicate(tp);
	if ((tp = tk_rmws(tk_rm(tp))) == 0)
	{
		UERROR( "\"(\" expected after \"# identifier\"");
		return tk_bool(1);
	}
	else if (tp->code != C_LParen)
	{
		UERROR( "\"(\" expected after \"# identifier\"");
		return tp;
	}
	if ((tp = tk_rmws(tk_rm(tp))) == 0)
	{
		UERROR( "tokens expected after \"# identifier (\"");
		return tk_bool(1);
	}
	else if (tp->code == C_RParen)
	{
		UERROR( "tokens expected between parentheses" );
		return tk_bool(1);
	}
	prev = argp = tp;
	for (len = 0, parendepth = 1; ; tp = (prev = tp)->next)
	{
		if (tp == 0)
		{
			UERROR( "missing \")\"" );
			tk_rml(argp);
			return tk_bool(1);
		}
		switch (tp->code)
		{
		case C_WhiteSpace:
			tp->ptr.string = " ";
			tp->rlen = 1;
			len++;
			if ((tp->next = tk_rmws(tp->next)) == 0)
				continue;
			else if ((tp->next->code == C_RParen) && parendepth == 1)
			{
				prev->next = tk_rm(tp);
				tp = prev;
				len--;
			}
			continue;

		case C_LParen:
			parendepth++;
			break;

		case C_RParen:
			if (--parendepth == 0)
			{
				tp = prev->next = tk_rm(tp);
				goto out;
			}
		default:break;
		}
		len += tp->rlen;
		continue;
	}
out:	{	
		char * pptokens;	/* canonicalized predicate argument */
		register char * cp;	/* a char in `pptokens' */
		unsigned long  boolval = 0;

		if ( pp != (Predicate *)0 )
		{
			prev = argp;
			pptokens = cp = ch_alloc((unsigned)(len + 1));
			COMMENT(tp != prev);
			do
			{	(void) memcpy(cp, prev->ptr.string, prev->rlen);
				cp += prev->rlen;
			} while ((prev = tk_rm(prev)) != tp);
			*cp = '\0';
			if (findpptokens(pp, pptokens) != 0)
				boolval = 1; /* for now - free(pptokens) ? */
		}
		inputp = tk_bool(boolval);
	}
	inputp->next = tp;
#ifdef DEBUG
	if (DEBUG('p') > 0)
	{
		(void)fprintf(stderr, "pd_replace() returns:\n");
		tk_prl(inputp);
	}
#endif
	return inputp;
}

Token *
pd_unassert(itp)
	Token * itp;
/* Given a Token list from a #unassert directive,
** this routine diagnoses any syntax errors.
** If there are none, this routine ensures
** the given assertion is deleted from the symbol table.
*/
{
	register Token * tp;	/* a token in the itp linked list	*/
	register Token * prev;	/* token preceeding tp			*/
	register enum {
		S_predicate,	/* got an identifier pp-token ...	*/
		S_lparen,	/* followed by a `(' pp-token ...	*/
		S_pptoken	/* followed by one non-')' pp-token	*/
	} state;
	register int parendepth;/* number_of('(') - number_of(')')	*/
	register int len;	/* length of canonicalized pptokens	*/
	register Token * prews; /* token preceeding white-space token	*/

#ifdef DEBUG
	if (DEBUG('p') > 0)
		(void)fprintf(stderr, "pd_unassert(itp=%#lx=`%.*s')\n",
		 itp, itp ? itp->rlen : 4, itp ? itp->ptr.string : "NULL");
#endif
	if ((tp = itp) == (Token *)0)
	{
		UERROR( "empty #unassert directive" );
		return (Token *)0;
	}
	if (tp->code != C_Identifier)
	{
		UERROR( "#unassert requires an identifier token" );
		return tp;
	}
	prev = tp;
	tp = tp->next;
	state = S_predicate;
	prews = (Token *)0;
	for (len = parendepth = 0; ; tp = (prev = tp)->next)
	{
#ifdef DEBUG
		if (DEBUG('p') > 3)
		{
			static char * saystate[] =
			    {"S_predicate", "S_lparen   ", "S_pptoken  "};

			(void)fprintf(stderr, "pd_unassert:%s, len=%d, depth=%d: ",
			 saystate[state], len, parendepth);
			tk_pr(tp, '\n');
		}
#endif
		switch (tp->code)
		{
	/*	case C_Comment:	*/
		case C_WhiteSpace:
			if (!(tp->rlen == 1 && tp->ptr.string[0] == '\n'))
			{
				switch (state)
				{
				case S_pptoken:
					if (prev->code != C_WhiteSpace)
					{
						tp->ptr.string = " ";
						tp->rlen = 1;
						prews = prev;
						len++;
						continue;
					}
					/*FALLTHRU*/
				case S_lparen:
				case S_predicate:
					prev->next = tk_rm(tp);
					tp = prev;
					continue;
				}
		    	}
			else
	/*	case C_NewLine:	*/
			{
				switch (state)
				{
				case S_predicate:
					prev->next = tk_rm(tp);
					forget(itp, len);
					return (Token *)0;
						  	 
				case S_lparen:
					UERROR( "tokens expected after \"(\"" );
					return itp;
	
				case S_pptoken:
					UERROR( "\")\" expected" );
					return itp;
				}
			}
			/*FALLTHRU*/

		case C_LParen:
			switch (++parendepth)
			{
			case 1: prev->next = tk_rm(tp);
				tp = prev;
				if (state == S_predicate) state = S_lparen;
				continue;

			default:if (state == S_lparen)	state = S_pptoken;
				len++;
				continue;
			}

		case C_RParen:
			switch (state)
			{
			case S_lparen:
				UERROR( "empty predicate argument" );
				return itp;

			case S_pptoken:
				switch (--parendepth)
				{
				case 0: if (prev->code == C_WhiteSpace)
					{
						(void)tk_rm(prev);
						prews->next = (Token *)0;
					}
					else
						prev->next = (Token *)0;
					forget(itp, len);
					if ((tp = tk_rm(tp)) != 0)	tk_extra(tp);
					return (Token *)0;

				default:len++;
					continue;
				}
			}
		default:if (state == S_predicate)
			{
				UERROR( "\"(\" expected after first identifier" );
				return itp;
			}
			if (state == S_lparen)	state = S_pptoken;
			len += tp->rlen;
			break;
		}
	}
}

#ifdef DEBUG
static	void
printassertions(pp)
	Predicate *pp;
/* Given a pointer to a predicate in the symbol table,
** this debugging routine prints out all of the
** assertions on the given predicate on stderr.
*/
{
	register Assert * ap;

	(void)fprintf(stderr, "PREDICATE `%s':\n", pp->name);
	for (ap = pp->list; ap != 0; ap = ap->next)
		(void)fprintf(stderr, "\t`%s'\n", ap->pptokens == (char *)0 ? "NULL" : ap->pptokens);
	(void)fprintf(stderr, "________ END OF LIST\n");
}
#endif
