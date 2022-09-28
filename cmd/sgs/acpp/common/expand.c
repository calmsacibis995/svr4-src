/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/expand.c	1.94"
/* macro expansion		Steve Adamski */

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"
#include "predicate.h"
#include "syms.h"

/* This file performs macro expansion, and manages a stack
** which contains the arguments of function-like macros.
*/
#define STKSZ	4
#if defined(DEBUG) && defined(__STDC__)
#    	define 	DEPTH   100
#	define	DBGSTACK()	if (DEBUG('e') > 0) \
		(void)fprintf(stderr, "ax=%d; argbase=%#lx\n", ax, argbase);
#	define	DBGCALL(num,funcname) if ( DEBUG('e') > (num) )		\
	{								\
		(void)fprintf(stderr, #funcname" called with:");	\
		st_mprint(mp);						\
	}
#	define	DBGCALLT(num,funcname) if ( DEBUG('e') > (num) )	\
	{								\
		(void)fprintf(stderr, #funcname" called with:");	\
		tk_pr(tp,'\n');					\
	}
#       define DBGRET(num,funcname,tp) if ( DEBUG('e') > (num) )	\
                {							\
                        (void)fprintf(stderr, #funcname" returns:");	\
                        tk_pr((tp),'\n');				\
                }
#       define DBGRETL(num,funcname,tp) if ( DEBUG('e') > (num) )	\
                {							\
                        (void)fprintf(stderr, #funcname" returns:");	\
                        tk_prl((tp));				\
                }
#else	/*  sfsup's cc doesn't accept #elif */
#	if defined(DEBUG)
#    		define 	DEPTH   100
#		define	DBGSTACK()	if (DEBUG('e') > 0) \
			(void)fprintf(stderr, "ax=%d; argbase=%#lx\n", ax, argbase);
#		define	DBGCALL(num,funcname) if ( DEBUG('e') > (num) )		\
		{								\
			(void)fprintf(stderr, "funcname called with:");		\
			st_mprint(mp);						\
		}
#		define	DBGCALLT(num,funcname) if ( DEBUG('e') > (num) )	\
		{								\
			(void)fprintf(stderr, "funcname called with:");		\
			tk_pr(tp,'\n');					\
		}
#       	define DBGRET(num,funcname,tp) if ( DEBUG('e') > (num) )	\
                	{							\
                       		(void)fprintf(stderr, "funcname returns:");	\
                        	tk_pr((tp),'\n');				\
                	}
#       	define DBGRETL(num,funcname,tp) if ( DEBUG('e') > (num) )	\
                	{							\
                        	(void)fprintf(stderr, "funcname returns:");	\
                        	tk_prl((tp));				\
                	}
#	else
#		define DBGSTACK()
#		define DBGCALL(num,funcname)
#		define DBGCALLT(num,funcname) 
#		define DBGRET(num,funcname,tp)
#		define DBGRETL(num,funcname,tp)
#	endif
#endif

#ifdef __STDC__
static enum {
	E_text		= 0x0,	/* normal text		*/
	E_condexpr	= 0x1,	/* #[el]if directive	*/
	E_directive	= 0x2	/* any directive	*/
} eflags;	/* indicates what is being expanded	*/
#else	/* 2.1 can't handle asigning an enum member to an int */
static int eflags;
#	define	E_text		0x0
#	define	E_condexpr	0x1
#	define	E_directive	0x2
#endif

static Token **	argstk;		/* argument stack	*/
static Token **	argbase;	/* arguments of most recent invocation */
static unsigned	ax;		/* number of arguments in most recent invocation*/
static unsigned	axlimit = STKSZ;/* dynamic argstk size  */
/* The arguments of function-like macros are kept on a stack.
** Given
**	#define	ADD(a,b)	a + b
**	ADD(ADD(1,   2), 3 );
**
** Here's a depiction of the stack right after identifying the
** arguments of the nested function-like macro invocation:
** 		_________
** 		|	|-0
** [axlimit-1]	|_______|
**  .		    .
**  .		    .
**  .		    .
** 		_________
**  argbase+ax->|	|-0
** [4]		|_______|
** 		|	|->	[`2']-0
** [3]		|_______|	
**     argbase->|	|->	[`1']-0
** [2]		|_______|
** 		|	|->	[`3']-0
** [1]		|_______|
** 	argstk->|	|->	[`ADD']->[`(']->[`1']->[`,']->[` ']->[`2']->[`)']-0
** [0]		|_______|
**
** Note that white space before and after an argument gets deleted from the
** argument Token list, as the argstk[1] entry shows. Also, sequences of white
** space among tokens forming a single argument are reduced to a single ` ',
** as the argstk[0] entry shows.
*/

#ifdef TRANSITION
/* To keep track of the possibility of unbounded nmacro reursion under the 
** -Xt flag, the nesting level of macro replacement is maintained.
*/
static	int	nestlevel;
#endif

#ifdef DEBUG
static int exparray[DEPTH];
static int subarray[DEPTH];
static int expdepth;
static int subdepth;
static const char * mark = " ABCDEFGHI";
#endif

static  Token * dodefined(/* Token* */ );
static  Token *	expand(	/* Token* */);
static	Token * getargs(/* Token *, short */);
static	int 	chkargs(/* Token *	*/);
static	void	incrax(	/* void	*/);
static	void	initax( /* void	*/);
static  void	paste(	/* Token * , Token *	*/);
static	Token *	replace(/* Token * , Macro *	*/);
static	void    setlineno( /* Token *  	*/ );
static	Token * splice( /* Token * , Token *	*/ );
static	void	stackfor(	/* unsigned int	*/	);
static  Token *	stringize(/* Token *	*/);
static	Token * subst(	/* Token * , Macro *	*/);

static int
chkargs(itp)
	Token * itp;
/* Given a pointer to an identifier Token
** that may begin a function-like macro invocation,
** this routine checks if a valid argument list exists.
** In the case of a (potential) multi-line macro invocation,
** calls to bf_tokenize() to get more tokens will be made.
** The return value is 1 if a legal argument list is found, else 0.
*/
{
	register Token * tp;	/* a Token in the input list	*/
	register int parendepth;/* numberof(`(') - numberof(`)')*/
	int gotlparen;		/* boolean: have we gotten `('? */

#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr, "chkargs called with:" );
		tk_prl(itp);
	}
#endif
	COMMENT(itp->code == C_Identifier);
	tp = itp->next;
	for (gotlparen = parendepth = 0;	; tp = tp->next)
	{
		if (tp == 0)
			goto bad_arglist;
		else
			switch (tp->code)
			{
			case C_WhiteSpace:
				if (tp->ptr.string[0] == '\n')
				{
					COMMENT(tp->rlen == 1);
					if ((eflags & E_directive) == 0
					 && ((tp->next = bf_tokenize(gotlparen ?
								B_invocation :
								B_macroname, itp)) == 0)
				 	 && gotlparen != 0)
					{
						TKERROR("EOF in argument list of macro", itp);
						goto bad_arglist;
					}
				}
				continue;

			case C_LParen:
				switch (parendepth)
				{
				case 0:	gotlparen++;
					parendepth++;
					continue;

				default:parendepth++;
					continue;
				}


			case C_RParen:
				switch ( parendepth )
				{
				case 0: goto bad_arglist;

				case 1: goto good_arglist;

				default:parendepth--;
					continue;
				}

			default:
                                switch ( parendepth )
                                {
                                case 0: goto bad_arglist;

                                default:continue;
                                }
			}
	}
good_arglist:
#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr,"\tsetarglist returns 1 ");
		tk_prl(itp);
	}
#endif
	return 1;
bad_arglist:
#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr,"\tsetarglist returns 0 ");
		tk_prl(itp);
	}
#endif
	return 0;
}

static Token *
dodefined(inputp)
	Token * inputp;
/* Given a pointer to a Token string, the first Token being {defined},
** this routine replaces the `defined' operator and its
** ioperand with the proper numerical result (1 if the
** identifier is defined, 0 if not). The modified list is returned.
** If there is a misuse of the `defined' operator,
** the syntax error is diagnosed and the offending
** subexpression is replaced by a `1'.
** This reaction is in concert with the CI 4.1 philosophy of
** evaluating an ill-formed constant expression as "true."
*/
{
	register enum _state {
		S_defined,	/* have gotten a {defined} pp-token ...	*/
		S_lparen,	/* followed by a {(} pp-token ...	*/
		S_identifier	/* followed by an identifier pp-token	*/
	} state;
	register Token* tp;	/* a Token in the input list	*/
	register Token * retp;	/* returned Token list */
	unsigned long isdefined;/* boolean: is operand of `defined' op a macro ? */

 	COMMENT(inputp != 0);
	COMMENT(inputp->code == C_Identifier);
	tp = inputp->next;
	for(state = S_defined; ; tp = tp->next)
	{
		switch (tp->code)
		{
		case C_WhiteSpace:
			if (tp->ptr.string[0] == '\n')
				goto error;
			continue;

		case C_LParen:
			if (state != S_defined)
				goto error;
			else
				state = S_lparen;
			continue;

		case C_Identifier:
			isdefined = (unsigned long)st_ismacro(tp);
			switch (state)
			{
			case S_identifier:
				goto error;

			case S_lparen:
				state = S_identifier;
				continue;

			case S_defined:
				goto out;
#ifdef DEBUG
			default:pp_internal("bad state in dodefined");
#endif
			}

		case C_RParen:
			if (state == S_identifier)
				goto out;
error:		default:UERROR( "invalid use of \"defined\" operator" );
			isdefined = 1;
			tk_rml(inputp);
			tp = (Token *) 0;
			goto errout;
		}	
	}
out:	if ((tp = tp->next) != 0)
		tk_rmto(inputp, tp);
	else
		tk_rml(inputp);
errout:	retp = tk_bool(isdefined);
	retp->next = tp;
	DBGRETL(1,dodefined,retp);
	return retp;
}

Token *
ex_condexpr(inputp)
	Token * inputp;
/* Given a pointer to a non-null Token list from a
** conditional inclusion directive, this routine returns
** the fully expanded Token list.
** This routine enable the special handling of the ``defined''
** operator during macro expansion within a conditional inclusion directive.
*/
{
	register Token * tp;	/* a Token in the list */
	register Token * pretp;	/* the Token preceeding `tp' */
	register Macro * mp;	/* definition of macro being expanded */
	Token head;	/* the `anchor' for the return list */

	COMMENT(inputp != 0);
#ifdef TRANSITION
	COMMENT(nestlevel == 0);
#endif
	eflags = (E_condexpr | E_directive);
	(pretp = &head)->next = tp = inputp;
	do {
		if (TK_INEXPAND(tp) == 0)
			continue;
		if (tp->code == C_Identifier)
		{
			if (tp->rlen == 7
			 && tp->ptr.string[0] == 'd'
			 && !strncmp(tp->ptr.string, "defined", 7))
			{
				pretp->next = (tp = dodefined(tp));
				continue;
			}
			else if ((mp = st_ismacro(tp)) == 0)
				continue;
			else if (ST_ISOBJECT(mp))
			{
#ifdef CXREF
				if (pp_flags & F_CXREF)
					pp_xref(mp, tp->number, 'R');
#endif
				if (ST_ISMANIFEST(mp))
				{
					if ((pretp->next = tk_cpl(mp->replist)) != 0)
					{
						setlineno(pretp->next);
						pretp = tk_eol(pretp);
					}
					if ((pretp->next = tp = tk_rm(tp)) == 0)
						break;
					continue;
				}
				else
				{
					pretp->next = replace(tp, mp);
					break;
				}
			}
			else
			{
				COMMENT(ST_ISFUNCTION(mp));
				pretp->next = expand(tp);
				break;
			}
		}
		COMMENT(tp->code != C_Marker);
		COMMENT(tp->code == C_Sharp);
		pretp->next = (tp = pd_replace(tp));
	} while ((tp = (pretp = tp)->next) != 0);
	eflags = E_text;
	return head.next;
}

Token *
ex_directive(inputp)
	Token * inputp;
/* Given a pointer to a non-null Token list from a
** (non-conditional inclusion) directive, this routine returns
** the fully expanded Token list.
*/
{
	register Token * tp;	/* a Token in the list */
	register Token * pretp;	/* the Token preceeding `tp' */
	Macro * mp;	/* definition of macro being expanded */
	Token head;	/* the `anchor' for the return list */

	COMMENT(inputp != 0);
#ifdef TRANSITION
	COMMENT(nestlevel == 0);
#endif
	eflags = E_directive;
	(pretp = &head)->next = tp = inputp;
	do {
		if (tp->code != C_Identifier || (mp = st_ismacro(tp)) == 0)
			continue;
		if (ST_ISOBJECT(mp))
		{
#ifdef CXREF
			if (pp_flags & F_CXREF)
				pp_xref(mp, tp->number, 'R');
#endif
			if (ST_ISMANIFEST(mp))
			{
				if ((pretp->next = tk_cpl(mp->replist)) != 0)
				{
					setlineno(pretp->next);
					pretp = tk_eol(pretp);
				}
				if ((pretp->next = tp = tk_rm(tp)) == 0)
					break;
				continue;
			}
			else
			{
				pretp->next = replace(tp, mp);
				break;
			}
		}
		else
		{
			COMMENT(ST_ISFUNCTION(mp));
			pretp->next = expand(tp);
			break;
		}
	} while ((tp = (pretp = tp)->next) != 0);
	eflags = E_text;
	return head.next;
}

void
ex_init()
/* This routine initializes data structures used during macro expansion. */
{
	register Token ** tpp;

	COMMENT(ax == 0);
	COMMENT(axlimit == STKSZ);
	COMMENT(eflags == E_text);
	argstk = argbase = tpp = (Token **)pp_malloc(sizeof(Token *) * STKSZ);
	while (tpp < argstk + STKSZ)
		*tpp++ = 0;
}

Token *
ex_input(inputp)
	Token * inputp;
/* Given a pointer to an Token that heads a
** list in normal (non-directive) text, this routine returns the
** fully expanded list.
*/
{
	register Token * tp;	/* a Token in the list */
	register Token * pretp;	/* Token that preceeds `tp'*/
	register Macro * mp;	/* definition of macro invocation in list */
	Token head;	/* "anchor" for return list */

	COMMENT(inputp != 0);
	COMMENT(eflags == E_text);
#ifdef TRANSITION
	COMMENT(nestlevel == 0);
#endif
	(pretp = &head)->next = tp = inputp;
	do {
		if (tp->code != C_Identifier || (mp = st_ismacro(tp)) == 0)
			continue;
		if (ST_ISOBJECT(mp))
		{
#ifdef CXREF
			if (pp_flags & F_CXREF)
				pp_xref(mp, tp->number, 'R');
#endif
			if (ST_ISMANIFEST(mp))
			{
				if ((pretp->next = tk_cpl(mp->replist)) != 0)
				{
					setlineno(pretp->next);
					pretp = tk_eol(pretp);
				}
				if ((pretp->next = tp = tk_rm(tp)) == 0)
					return head.next;
			}
			else
			{
				pretp->next = replace(tp, mp);
				break;
			}
		}
		else
		{
			int	saveax;		/* save argstk index */
			ptrdiff_t saveoff;	/* save offset of argbase from argstk */

			COMMENT(ST_ISFUNCTION(mp));
			if (chkargs(tp))	/* any arguments? */
			{
#ifdef CXREF
				if (pp_flags & F_CXREF)
					pp_xref(mp, tp->number, 'R');
#endif
				saveax = ax;
				saveoff = argbase - argstk;
				argbase += saveax;
				DBGSTACK();
				ax = 0;
				tp = getargs(tp, mp->nformals);
				if ((ax != mp->nformals) && !(mp->nformals == 0
		 	 	&& ax == 1 && argbase[0] == (Token *)0))
					WARN( "argument mismatch" );
				pretp->next = replace(tp, mp);
				initax();
				ax = saveax; /* for now - delete tokens */
				argbase = argstk + saveoff;
				DBGSTACK();
				break;
			}
			else
			{

				DBGSTACK();
				COMMENT(tp != 0);
				if ((tp = (pretp = tp)->next) == 0)
					break;
				continue;
			}
		}
	} while ((tp = (pretp = tp)->next) != 0);
	COMMENT(eflags == E_text);
		
	return head.next;
}

static Token *
expand(inputp)
	Token * inputp;
/* Given a pointer to a (possibly null) Token list,
** this routine performs macro expansion and returns the fully expanded list.
*/
{
	register Token * tp;	/* a Token in the list */
	register Token * pretp;	/* the Token preceeding `tp' */
	register Macro * mp;	/* definition of macro being expanded */
	Token head;	/* the `anchor' for the return list */

#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr,"expand<%3d%c> called with:",
                        expdepth, mark[((exparray[expdepth%DEPTH])%10)]);
		exparray[expdepth++]++;
		tk_pr(inputp,'\n');
	}
#endif
	if (inputp == 0)
		return (Token *)0;
	(pretp = &head)->next = tp = inputp;
	do {
		if (TK_INEXPAND(tp) == 0 && (tp->next == 0 || tp->next->code != C_Marker))
			continue;	/* could make a tighter loop */
		else if (tp->code == C_Marker)
		{
			st_unhide(tp->ptr.macro);
			pretp->next = expand(tk_rm(tp));
			break;
		}
		else if (eflags & E_condexpr
			&& tp->rlen == 7
			&& tp->code == C_Identifier
			&& tp->ptr.string[0] == 'd'
			&& !strncmp(tp->ptr.string, "defined", 7))
		{
			pretp->next = (tp = dodefined(tp));
			continue;
		}
		else if (tp->code == C_Sharp && eflags & E_condexpr)
		{
			pretp->next = (tp = pd_replace(tp));
			continue;
		}
		else if ((mp = (tp->code == C_Identifier?(tp->aux.hid?0:st_ismacro(tp)):0))
			 && st_hidden(mp))
		{
			tk_hide(tp);
			continue;
		}
		else if (mp && ST_ISOBJECT(mp))
		{
#ifdef CXREF
			if (pp_flags & F_CXREF)
				pp_xref(mp, tp->number, 'R');
#endif
			pretp->next = replace(tp, mp);
			break;
		}
		else if (tp->next && tp->next->code == C_Marker)
		{
			st_unhide(tp->next->ptr.macro);
			tp->next = tk_rm(tp->next);
			pretp->next = expand(tp);
			break;
		}
		else if (mp)
		{
			int	saveax;		/* saves current argbase index	*/
			ptrdiff_t saveoff;	/* save offset of argbase from argstk */
	
			COMMENT(ST_ISFUNCTION(mp));
			if (chkargs(tp))	/* any arguments? */
			{
#ifdef CXREF
				if (pp_flags & F_CXREF)
					pp_xref(mp, tp->number, 'R');
#endif
				saveax = ax;
				saveoff = argbase - argstk;
				argbase += saveax;
				DBGSTACK();
				ax = 0;
				tp = getargs(tp, mp->nformals);
				if ((ax != mp->nformals)
				&& !(mp->nformals==0&&ax==1&&argbase[0]==(Token *)0))
					WARN( "argument mismatch" );
				pretp->next = replace(tp, mp);
				initax();
				ax = saveax; /* for now - delete tokens */
				argbase = argstk + saveoff;
				DBGSTACK();
				ax = saveax; /* for now - delete tokens */
				argbase = argstk + saveoff;
				DBGSTACK();
				break;
			}
		}
	} while ((tp = (pretp = tp)->next) != 0);
#ifdef DEBUG
        if ( DEBUG('e') > 0 )
        {
		--expdepth;
                (void)fprintf(stderr, "expand<%3d%c> returns:",
		 expdepth, mark[((exparray[(expdepth%DEPTH)])-1)%10] );
                tk_prl(head.next);
        }
#endif
	return head.next;
}

static Token *
getargs(itp, nformals)
	Token * itp;
	short	nformals;
/* Given a pointer to an identifier Token
** that may begin a function-like macro invocation,
** and the number of expected arguments,
** this routine searches the argument list and identifies arguments.
** The argument token lists
** will be assigned to the argument stack, the stack index will reflect the
** number of args (including those consisting of no pp-tokens, which will be
** warned about in -Xc mode), and this routine will return a pointer to the ')' 
** token that ends the invocation. 
**
** This implementation defines the behavior that any argument that
** consists of no pp-tokens is counted as an argument and is represented
** by a null token when replacing a formal parameter.
*/
{
	register Token * tp;	/* a Token in the input list	*/
	register Token * pretp;	/* the Token preceding `tp'	*/
	register int parendepth;/* numberof(`(') - numberof(`)')*/
	Token  tmp;		/* temporary */
	Token * prews;		/* Token preceding white space	*/
	int gotlparen;		/* boolean: have we gotten `('? */
	int tokenless;		/* boolean: have we gotten any token-less args? */

#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr, "getargs called with:" );
		tk_prl(itp);
	}
#endif
	COMMENT(itp->code == C_Identifier);
	tmp.code = C_Nothing;
	prews = 0;
	tokenless = 0;
	tp = (pretp = itp)->next;
	for (gotlparen = parendepth = 0;	; tp = (pretp = tp)->next)
	{
		if (tp == 0)
			goto not_invocation;
		else
			switch (tp->code)
			{
			case C_Marker:
				st_unhide(tp->ptr.macro);
				pretp->next = tk_rm(tp);
				tp = pretp;
				continue;

			case C_WhiteSpace:
				if (gotlparen != 0)
				{
					if (pretp->code == C_WhiteSpace
					 || argbase[ax] == 0)
					{
						pretp->next = tk_rm(tp);
						tp = pretp;
					}
					else
					{
						prews = pretp;
						tp->ptr.string[0] = ' ';
						tp->rlen = 1;
					}
				}
				continue;

			case C_LParen:
				switch (parendepth)
				{
				case 0:	gotlparen++;
					parendepth++;
					continue;

				case 1: if (argbase[ax] == (Token *)0)
						argbase[ax] = tp;
				default:parendepth++;
					continue;
				}

			case C_Comma:
				switch( parendepth )
				{
				case 0:	goto not_invocation;

				case 1: if (pretp->code != C_WhiteSpace)
						pretp->next = (Token *)0;
					else
					{
						prews->next = 0;
						(void)tk_rm(pretp);
					}
					tmp.next = tk_rm(tp);
					tp = &tmp;
					if (argbase[ax] == (Token *) 0)
						tokenless = 1;
					incrax();
				default:continue;
				}

			case C_RParen:
				switch ( parendepth )
				{
				case 0: goto not_invocation;

				case 1: if (pretp->code != C_WhiteSpace)
						pretp->next = (Token *)0;
					else
					{
						prews->next = 0;
						(void)tk_rm(pretp);
					}
					(void)tk_rm(itp);
					if (argbase[ax] == (Token *) 0)
						tokenless = 1;
					incrax();
					goto ret;

				default:parendepth--;
					continue;
				}

			default:
                                switch ( parendepth )
                                {
                                case 0: goto not_invocation;

                                case 1: if (argbase[ax] == (Token *)0 )
                                        	argbase[ax] = tp;
                                default:continue;
                                }
			}
	}
ret:
#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		register int i = ax-1;
		(void)fprintf(stderr,"\tgetargs returns with ax=%d\n",ax);
		(void)fprintf(stderr,"\tgetargs returns with stack:\n");
		if ( ax >= 1 )
			do
			{
				(void)fprintf(stderr,"\targbase[%d]: ",i);
				tk_prl(argbase[i]);
			} while ( i-- );
	}
#endif
	if ( (nformals != 0) && tokenless && (pp_flags  & F_Xc) )
		WARN("token-less macro argument");
	return tp;
not_invocation:
	DBGRETL(0,getargs,itp);
	return itp;
}

static void
incrax()
/* Increments the index to the argument stack.
** Calls stackfor to grow the stack.
*/
{
	++ax;
	stackfor(1);
}

static void
stackfor(n)
	unsigned int n;
/* Given a number of stack elements to allocate,
** this routine checks the stack limit, and grows
** the stack if necessary.
*/
{
	register ptrdiff_t diff;

	diff = argbase - argstk;
#ifdef DEBUG
	if (DEBUG('e') > 3)
		(void)fprintf(stderr, "stackfor() called\n");
#endif
	if ((diff + ax + n) >= axlimit)
	{
		register Token ** tpp;
#ifdef DEBUG
		Token ** saveargstk;

		if (DEBUG('e') > 3)
			(void)fprintf(stderr, "stackfor() grows stack\n");
		saveargstk = argstk;
#endif
		axlimit += STKSZ;
		argstk = (Token **)pp_realloc( (char*) argstk, axlimit*sizeof(Token **)/sizeof(char) );
		argbase = argstk + diff;
#ifdef DEBUG
		if (DEBUG('e') > 3 && argstk != saveargstk)
			(void)fprintf(stderr, "argstk changed: saveargstk=%#lx,argstk=%#lx\n", saveargstk, argstk);
#endif
		tpp = argbase + ax;
		while (tpp < argstk + axlimit)
			*tpp++ = 0;
	}
}

static	void
initax()
/* Initializes the argument stack index to zero. In this process,
** this routine insures that all the Tokens corresponding to
** any previous arguments are deleted.
*/
{
	register int i;

	for(i = ax; i >= 1; )
	{
		--i;
		tk_rml( argbase[i] );
		argbase[i] = (Token *)0;
	}
}

static void
paste(leftp, rightp)
        Token * leftp;
	register Token * rightp;
/* Given pointers to two non-null Token lists,
** this routine pastes the last Token of the left string
** to the first Token of the right string. 
** This implements the ANSI C ## ("paste") operation.
*/
/* The resulting char[] is trailed by a '\0' to delimit the string
** so tk_tokenize() can tokenize the result.
*/
{
	register char *	cp;	/* buffer to hold result 	*/
	register Token *lastp;	/* last Token in the left list	*/

	COMMENT(leftp != 0);
	COMMENT(rightp != 0);
	lastp = tk_eol(leftp);
	{
		register int	len;	/* length of two operand Tokens	*/

		cp = ch_alloc((len = lastp->rlen) + rightp->rlen + 1);
		(void) memcpy(cp, lastp->ptr.string, len);
		(void) memcpy(cp + len, rightp->ptr.string, rightp->rlen);
		lastp->ptr.string = cp;
		lastp->rlen = (len += rightp->rlen);
		cp[len] = '\0';
	}
	{
		register Token * resultp; /* tokenization of buffer	*/

		resultp = tk_tokenize(cp);
		COMMENT(resultp->code  == C_WhiteSpace ? resultp->ptr.string[0] == '/' : 1);
		if (resultp->code == C_BadInput
		 || resultp->code == C_WhiteSpace
		 || resultp->next != 0)
			lastp->code = C_Invalid;
		else if ((lastp->code = resultp->code) == C_Identifier)
			lastp->aux.hid = 0;
		tk_rml(resultp);
	}
	lastp->next = tk_rm(rightp);
	DBGRET(0,paste,leftp);
}

static Token *
replace(tp, mp)
	Token * tp;
	Macro * mp;
/* Given a pointer to the last pp-token in a macro invocation
** and it's macro definition,
** this routine returns the fully expanded Token list.
*/
{
	long number;
	COMMENT(tp->code == C_Identifier || tp->code == C_RParen);
#ifdef TRANSITION
	if (nestlevel++ > (st_nmacros() * st_nmacros()))
	{
		COMMENT(pp_flags & F_Xt);
		FATAL("macro recursion", (char *)0);
	}
	number = tp->number;
	tp = expand(splice(subst(mp, number), tk_rm(tp)));
	nestlevel--;
	return tp;
#else
	return expand(splice(subst(mp, number), tk_rm(tp)));
#endif
}

static void
setlineno(tp)
	register Token *tp;
/*
** set line numbers in token list tp to lineno.
** used to set the line numbers of replacement lists to the
** current line number. ( The line number of the definition is 
** replaced with the line number of the invocation.
*/
{
	for(; tp != 0; tp = tp->next)
		tp->number = bf_lineno;
}

static Token *
splice(leftp,rightp)
	Token *	leftp;
	Token *	rightp;
/* Given pointers to 2 Token lists (a left and a right), this routine
** returns the single list formed by appending the right 
** list to the end of the left list.
** Either or both of the input lists may be null.
*/
{
	Token *	lastp; /* last Token in the left list */

	if ((lastp = tk_eol(leftp)) == 0)
	{
		DBGRETL(4,splice,rightp);
		return rightp;
	}
	else
	{
		lastp->next = rightp;
		DBGRETL(4,splice,leftp);
		return leftp;
	}
}

static Token *
subst(mp, number)
	Macro * mp;
	long	number;
/* Given a pointer to a Macro definition, and the line number of the 
** token to be substituted, this routine
** performs macro substitution on the replacement list.
** It performs any operations on the list, substitutes actuals
** for formals in function-like macros, and expands the result
** further if required by the macro substitution rules.
** The resulting Token list is returned.
*/
{
	register Token * tp;	/* tokens in the replacement list	*/
	Token	head;		/* "anchor" for return list	*/
	register Token * eos;	/* end of return list	*/

	tp = tk_cpl(mp->replist);
	if (ST_ISMANIFEST(mp))
	{
		head.next = tp;
		setlineno(tp);
		goto ret;
	}
#ifdef DEBUG
	if ( DEBUG('e') > 0 )
	{
		(void)fprintf(stderr,"subst<%3d%c>(tp=%#lx=%.*s,mp=%#lx=%.*s)\n",
		 subdepth, mark[((subarray[subdepth%DEPTH])%10)],
		 tp, tp?tp->rlen:0, tp?tp->ptr.string:"", mp, mp->namelen, mp->name);
		subarray[subdepth++]++;
	}
#endif
	if ( !ST_ISOBJECT(mp) && (mp->nformals >= ax) )
		stackfor(mp->nformals - ax);
	for((eos = &head)->next = 0; tp != 0; )
	{
		tp->number = number;
		if (TK_INSUBST(tp) == 0)
		{
			eos->next = tp;
			tp = tp->next;
			(eos = eos->next)->next = 0;
			continue;
		}
		else if (tp->code == C_StrFormal)
				eos->next = stringize(argbase[tp->aux.argno]);
		else if (tp->code == C_Paste && tp->next->code == C_UnexpFormal)
		{
			tp = tk_rm(tp);
			if (eos == &head)
			{
				COMMENT(mp->replist->code == C_Paste
					|| (mp->replist->code == C_UnexpFormal &&
					   argbase[mp->replist->aux.argno] == 0));
				tp->code = C_Formal;
				eos = tk_eol(eos);
				continue;
			}
			else if (argbase[tp->aux.argno] != 0)
				paste(head.next, tk_cpl(argbase[tp->aux.argno]));
		}
		else if (tp->code == C_Paste && tp->next->code == C_StrFormal)
		{
			tp = tk_rm(tp);
			if (eos == &head)
			{
				COMMENT(mp->replist->code == C_Paste
					|| (mp->replist->code == C_UnexpFormal &&
					   argbase[mp->replist->aux.argno] == 0));
				eos->next = stringize(argbase[tp->aux.argno]);
				eos = tk_eol(eos);
				continue;
			}
			else if (argbase[tp->aux.argno] != 0)
				paste(head.next, stringize(argbase[tp->aux.argno]));
		}
#ifdef TRANSITION
		else if (tp->code == C_Paste && tp->next->code == C_MergeFormal)
		{
			tp = tk_rm(tp);
			if (argbase[tp->aux.argno] == 0)
			/*EMPTY*/	;
			else if ((argbase[tp->aux.argno])->next == 0)
				paste(head.next, tk_cp(argbase[tp->aux.argno]));
			else
			{
				Token * temp;

				temp = tk_cpl(argbase[tp->aux.argno]);
				tk_merge(temp, tk_eol(temp), tk_lenl(temp));
				temp->code = C_Goofy;
				COMMENT(temp->next == 0);
				paste(head.next, temp);
			}
		}
#endif
		else if (tp->code == C_Paste && tp->next != 0)
		{
			COMMENT(tp->next->code != C_WhiteSpace);
			COMMENT(tp->next->code != C_StrFormal);
			COMMENT(tp->next->code != C_Formal);
			COMMENT(tp->next->code != C_UnexpFormal);
			if (eos != &head)
			{
				Token * save;

				save = tk_cp(tp = tk_rm(tp));
				save->next = 0;
				/* for now - paste(eos, save); */
				paste(head.next, save);
			}
		}
		else if (tp->code == C_UnexpFormal && tp->next->code == C_Paste)
			eos->next = tk_cpl(argbase[tp->aux.argno]);
		else if (tp->code == C_Formal)
		{
#ifdef TRANSITION
			int savenestlevel;

			savenestlevel = nestlevel;
			nestlevel = 0;
#endif
			eos->next = expand(tk_cpl(argbase[tp->aux.argno]));
#ifdef TRANSITION
			nestlevel = savenestlevel;
#endif
		}
		else
		{
			if (tp->code == C_Paste)
			/*EMPTY*/ COMMENT(tp == tk_eol(tp));
			else
			{
				eos->next = tp;
				tp = tp->next;
				(eos = eos->next)->next = 0;
				continue;
			}
		}
		tp = tk_rm(tp);
		eos = tk_eol(eos);
	}
#ifdef TRANSITION
	if ((pp_flags & F_Xt) == 0)
#endif
	{
		st_hide(mp);
		(tp = tk_new())->code = C_Marker;
		tp->ptr.macro = mp;
		DBGCODE(tp->rlen = 0);
		tp->next = (Token *)0;
		eos->next = tp;
	}
#ifdef DEBUG
	if (DEBUG('e') > 0)
	{
		(void)fprintf(stderr,"subst<%3d%c> returns",
		 --subdepth,
		 mark[((subarray[(subdepth%DEPTH)])-1)%10] );
		tk_prl(head.next);
	}
#endif
ret:
	return head.next;
}

static Token *
stringize(tp)
	register Token * tp;
/* Given a Token list of actuals, this routine performs the # operation on
** the list and returns a pointer to the result string literal pp-token that
** it has created.
*/
{
	register char *	buf;	/* result buffer	*/
	register char *	cp;	/* a character in `buf'	*/

	DBGCALLT(0,stringize);
	buf = cp = ch_alloc((tk_lenl(tp) * 2) + (2 * sizeof((char)'"')) + sizeof((char)'\0'));
	*cp++ = '"';
	for (; tp != 0; tp = tp->next)
	{
		register char *	tkcp;	/* character in a token	*/
		register int	len;	/* length of token	*/

		len = tp->rlen;
		tkcp = tp->ptr.string;
		switch (tp->code)
		{
		case C_String:
		case C_C_Constant:
			while (len--)
			{
				int	c;	/* character in a token	*/

				switch (c = *tkcp++)
				{
				case '"':
				case '\\':
					*cp++ = '\\';
				default:break;
				}
				*cp++ = (char) c;
			}
			continue;

		default:(void) memcpy(cp, tkcp, len);
			cp += len;
			continue;
		}
	}
	*cp++ = '"';
	*cp = '\0';
	tp = tk_tokenize(buf);
	if (tp->code != C_String)
		tp->code = C_Invalid;
	if ( tp->next != (Token *) 0 ) {
		tk_rml(tp->next);
		tp->ptr.string = buf;
		tp->rlen = cp - buf;
		tp->code = C_Invalid;
		tp->next = (Token *) 0;
	}
	DBGRET(0,stringize,tp);
	return tp;
}

		
