/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident "@(#)acpp:common/token.c	1.73"
/* token.c - Token allocation and library routines */

/* This file contains the memory allocation code for Tokens.
** Routines that allocate and deallocate Tokens maintain
** a free list of unused Tokens. If any new Tokens are needed
** and the free list is empty, more Tokens are malloc()'d.
**
** This file also contains various Token-related "library"
** routines.
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"

/* for now - the next two ifgroups don't use #elif because
** sfsup's cc doesn't recognize them.
*/
#ifndef	TK_INIT
#	define	TK_INIT	1000
#else
#	if	TK_INIT <= 0
	#error	"TK_INIT must be greater than zero"
#	endif
#endif

#ifndef	TK_ALLOC
#	define	TK_ALLOC	128
#else
#	if	TK_ALLOC <= 0
	#error	"TK_ALLOC must be greater than zero"
#	endif
#endif

#ifdef DEBUG
#	define	DBGRET(code)	if ( DEBUG('v') > 0 )\
		(void)fprintf( stderr, "returns %s\n", tk_saycode((code)) )
#else
#	define	DBGRET(code)
#endif

static void	alloctokens( /* int	*/);
static void	setchtypes(/* unsigned char *, int	*/ );

static	Token *	freetokens;	/* linked list of available Token's */

char	tk_chtypes[256];	/* indexes token type by character */

#ifdef DEBUG
static char *codenames[] = {
	"C_Nothing",	/* no such token (also used to mark a Token as "free") */
	"C_WhiteSpace", /* any white space, comment, or new-line */
	"C_Operator",	/* any operator not otherwise mentioned herein */
	"C_Identifier",	/* identifiers (no keyword distinction) */
	"C_I_Constant",	/* an integer constant */
	"C_F_Constant",	/* a floating point constant */
	"C_C_Constant",	/* a character constant */
	"C_String",	/* a string literal */
	"C_Wstring",	/* a wide string literal */
	"C_Header",	/* a <...> in a #include directive */
	"C_Dollar",	/* $	*/
	"C_RParen",	/* )	*/
	"C_Comma",	/* ,	*/
	"C_Question",	/* ?	*/
	"C_Colon",	/* :	*/
	"C_LogicalOR",	/* ||	*/
	"C_LogicalAND",	/* &&	*/
	"C_InclusiveOR",/* |	*/
	"C_ExclusiveOR",/* ^	*/
	"C_BitwiseAND",	/* &	*/
	"C_Equal",	/* ==	*/
	"C_NotEqual",	/* !=	*/
	"C_GreaterThan",/* >	*/
	"C_GreaterEqual"/* >=	*/,
	"C_LessThan",	/* <	*/
	"C_LessEqual",	/* <=	*/
	"C_LeftShift",	/* <<	*/
	"C_RightShift",	/* >>	*/
	"C_Plus",	/* +	*/
	"C_Minus",	/* -	*/
	"C_Mult",	/* *	*/
	"C_Div",	/* /	*/
	"C_Mod",	/* %	*/
	"C_UnaryPlus",	/* +	*/
	"C_UnaryMinus",	/* -	*/
	"C_Complement",	/* ~	*/
	"C_Not",	/* !	*/
	"C_LParen",	/* (	*/
	"C_Operand",	/* an operand used during #[el]if expression evaluation */
	"C_Sharp",	/* #	*/
	"C_LBracket",	/* [	*/
	"C_RBracket",	/* ]	*/
	"C_LBrace",	/* {	*/
	"C_RBrace",	/* }	*/
	"C_Arrow",	/* ->	*/
	"C_Assign",	/* =	*/
	"C_MultAssign",	/* *=	*/
	"C_DivAssign",	/* /=	*/
	"C_ModAssign",	/* %=	*/
	"C_PlusAssign",	/* +=	*/
	"C_MinusAssign",/* -=	*/
	"C_LeftAssign",	/* <<=	*/
	"C_RightAssign",/* >>=	*/
	"C_ANDAssign",	/* &=	*/
	"C_XORAssign",	/* ^=	*/
	"C_ORAssign",	/* |=	*/
	"C_SemiColon",	/* ;	*/
	"C_Ellipsis",	/* ...	*/
	"C_Dot",	/* .	*/
	"C_Increment",	/* ++	*/
	"C_Decrement",	/* --	*/
#if CPLUSPLUS
	"C_ArrowStar",	/* ->*	*/
	"C_DotStar",	/* .*	*/
	"C_Scope",	/* ::	*/
#endif
	"C_Paste",	/* ##	*/
	"C_Formal",	/* a formal parameter in a #define to be expanded */
	"C_StrFormal",	/* a formal parameter to be stringized */
	"C_MergeFormal",/* formal parameter: for old-style char- and string-izing */
	"C_UnexpFormal",/* a formal parameter to be replaced unexpanded */
	"C_BadInput",	/* illegal input token */
	"C_Invalid",	/* invalid result of a ## operation */
	"C_Goofy",	/* (possibly) partial Token created internally */
	"C_MacroName",	/* the macro name in a #define directive (see C_Manifest) */
	"C_Manifest",	/* manifest constant: C_MacroName, no expansion operations on rep-list */
	"C_Marker",	/* points to a macro "hidden" from expansion */
};
#endif

static void
alloctokens(n)
	register int n;
/* Given the (positive) number of Tokens to be malloc()-ed, this routine gets
** more space, creates a linked list of Tokens, and assigns them to the
** free Token list.
*/
{
	register Token * tp;	/* token in new free list	*/
	register Token * nextp;	/* next token in new free list	*/
	register int i;		/* loop counter			*/

	DBGASSERT(n > 0);
	tp = freetokens = (Token *)pp_malloc(sizeof(Token) * n);
	n--;
	for (i=0; i < n; i++)
	{
		DBGCODE(tp->code = C_Nothing);
		TK_CLRUSE(tp);
		tp->next = (nextp = tp + 1);
		tp = nextp;
	}
	DBGCODE(tp->code = C_Nothing);
	TK_CLRUSE(tp);
	tp->next = (Token *)0;
}

Token *
tk_bool(n)
	unsigned long n;
/* Given an unsigned long argument, this routine returns a pointer
** to a new C_I_Constant Token of `0' if the argument is zero
** or `1' if it is not. Those fields of the Token
** that are not to identify the constant are set to zero.
** The character buffer that
** the returned token points to is shared by the Tokens returned from
** multiple calls of this routine.
*/
{
	register Token * tp;

	tp = tk_new();
	tp->ptr.string = n?"1":"0";
	tp->rlen = 1;
	tp->code = C_I_Constant;
	tp->aux.argno = 0;
	tp->number = 0;
	tp->next = (Token *)0;
	return tp;
}

Token *
tk_cp(tp)
	Token * tp;
/* Given a pointer to a Token, this routine
** returns a pointer to a copy of the Token.
** Only the Token is copied, not any buffer it points to.
** If the token pointer argument is zero, this routine
** returns zero.
*/
{
	Token * ntp;	/* new token pointer	*/

#ifdef DEBUG
	if ( DEBUG('p') > 0 )
	{
		(void)fputs("tk_cp(",stderr);
		tk_pr(tp, ' ');
		(void)fputs("\t) returns ",stderr);
	}
#endif
	if (tp != 0)	
		*(ntp = tk_new()) = *tp;
	else
		ntp = 0;
#ifdef DEBUG
        if ( DEBUG('p') > 0 )
        {
                tk_pr(ntp, '\n');
        }
#endif
	return ntp;
}

Token *
tk_cpl(tp)
	register Token * tp;
/* Given a pointer to a (possibly null) Token list, this routine
** returns a pointer to a copy of the Token list.
** Only the Tokens are copied, not any buffer they may refer to.
*/
/* for now - I don't know if I like this code as opposed to 1.29,
** at the request of the code reviewers I've added 24 bytes.
** I'm not sure that it's measurably faster this way.
*/
{
	register Token * retp;	/* return token pointer		*/
	register Token * ntp;	/* new token pointer		*/
	register Token * eoretp;/* end of return (list) pointer	*/

#ifdef DEBUG
	if ( DEBUG('p') > 0 )
	{
		(void)fputs("tk_cpl(",stderr);
		tk_prl(tp);
		(void)fputs("\t) returns ",stderr);
	}
#endif
	retp = (Token *)0;
	if ( tp )
	{
		*(ntp=tk_new()) = *tp;
		eoretp = retp = ntp;
		tp = tp->next;
		for( ; tp != 0; tp = tp->next)
		{
			*(ntp=tk_new()) = *tp;
			eoretp->next = ntp;
			eoretp = ntp;
		}
	}
#ifdef DEBUG
        if ( DEBUG('p') > 0 )
        {
                tk_prl(retp);
        }
#endif
	return retp;
}

Token *
tk_eol(tp)
	register Token * tp;
/* Given a pointer to a (possibly null) Token list, this routine
** returns a pointer to the last Token in the list.
** If the input list is null, zero is returned.
*/
{
	if (tp == 0)
	/*EMPTY*/ ;
	else
		while (tp->next != 0)
			tp = tp->next;
	return tp;
}

void
tk_extra(inputp)
	Token * inputp;
/* Given a pointer to a (possibly null) Token list that comes from
** the end of a directive, this routine places the
** Tokens on the free token list. If any of the Tokens are pp-tokens,
** a diagnostic about the trailing Tokens is written on stdout.
*/
{
	register Token * tp;	/* Token in input list	*/
	int	warned;	/* boolean: printed a warning ?	*/

	warned = 0;
	if ((tp = inputp) == 0)
		return;
	do
	{
		switch (tp->code)
		{
		case C_WhiteSpace:
			break;

		case C_BadInput:
			TKERROR("invalid token in directive", tp);
			break;

		case C_Invalid:
			TKERROR("invalid token", tp);
			break;

		default:if (warned == 0)
			{
				warned++;
				WARN("tokens ignored at end of directive line");
			}
			break;
		}
		COMMENT(tp->code != C_Nothing);
		DBGCODE(tp->code = C_Nothing);
		if (tp->next != 0)
			tp = tp->next;
		else
		{
			tp->next = freetokens;
			freetokens = inputp;
			return;
		}
	}
	while /*CONSTANTCONDITION*/ (1);
}

void
tk_hide(tp)
	Token * tp;
/* Given a pointer to an identifier Token,
** this routine sets the "hidden" field of
** the Token to ``true'' so further macro expansion is not attempted
** on this Token.
*/
{
#ifdef DEBUG
	if (DEBUG('s') > 0)
		(void)fprintf(stderr,"tk_hide(tp=%#lx=%.*s)\n", tp, tp->rlen, tp->ptr.string);
#endif
	COMMENT(tp != 0 && tp->code == C_Identifier);
	DBGASSERT(tp->aux.hid == 0);
	tp->aux.hid = 1;
}

unsigned int
tk_lenl(tp)
	register Token * tp;
/* Given a pointer to a (possibly null) list of Tokens, this routine
** returns the sum of the length of all the Tokens in the list.
** The total includes the complete spelling of white space and comments.
*/
{
	register unsigned int len;

	for (len = 0; tp != 0; tp = tp->next)
		len += tp->rlen;
	return len;
}

void
tk_merge(fromtp, totp, len)
	register Token * fromtp;
	register Token * totp;
	unsigned int len;
/* Given a non-zero token list bounded by pointers to
** the first and a later token, and the sum length of all included Tokens,
** this routine merges
** the contents of the list into the first token and
** removes all other tokens up to, and including, the latter.
** This routine assumes that the first and the later token are
** not the same (that is, the list contains more than one token).
*/
{
	register Token * tp;	/* pointer in list	*/
	char * resultcp;	/* result buffer	*/
	register char * cp;	/* pointer into resultcp*/

	DBGASSERT(fromtp != totp);
#ifdef DEBUG
	if ( DEBUG('i') > 3 )
	{
		(void)fprintf(stderr, "tk_merge(%.*s, %.*s, %d)\n",
		 fromtp->rlen, fromtp->ptr.string,
		 totp->rlen, totp->ptr.string, len);
	}
#endif
	resultcp = pp_malloc(len + sizeof((char)'\0'));
	(void) memcpy(resultcp, fromtp->ptr.string, fromtp->rlen);
	cp = resultcp + fromtp->rlen;
	do
	{
		tp = fromtp->next;
		(void) memcpy(cp, tp->ptr.string, tp->rlen);
		cp += tp->rlen;
		fromtp->next = tk_rm(tp);
	} while(tp != totp);
	*cp = '\0';
	fromtp->ptr.string = resultcp;
	fromtp->rlen = (unsigned short) len;
#ifdef DEBUG
	if ( DEBUG('i') > 3 )
	{
		fromtp->code = C_Header;
		(void)fputs("tk_merge result:", stderr);
		tk_prl(fromtp);
	}
#endif
}

Token *
tk_new()
/* Returns a pointer to a new Token.
** All fields of the Token have "garbage" values in them
** except the 'use' field, which is cleared.
*/
{
	register Token * tp;	/* new token	*/

#ifdef DEBUG
	if ( DEBUG('t') > 0 )
		(void)fprintf(stderr, "tk_new()\n");
#endif
	if (freetokens == 0)
	{
		DBGASSERT(TK_ALLOC > 0);
		alloctokens(TK_ALLOC);
	}
	tp = freetokens;
	COMMENT(!TK_ISBUF(tp));
	TK_CLRUSE(tp);
	freetokens = tp->next;
	COMMENT(tp->code == C_Nothing);
	DBGCODE(tp->next = (Token *)0x0FFFFFFF);
	return (tp);
}

Token *
tk_paste()
/* Returns a pointer to a new `##' Token.
** The character buffer that the return
** Token references is shared by all Tokens returned by multiple calls
** to this routine. The `aux.ws' field of the Token is set to 0.
*/
{
	register Token * tp; /* new `##' Token */

	tp = tk_new();
	tp->ptr.string = "##";
	tp->rlen = 2;
	tp->code = C_Paste;
	tp->aux.ws = 0;
	return tp;
}

Token *
tk_rm(tp)
	register Token *tp;
/* Given a pointer to a Token, this routine returns it to a free
** list and returns a pointer to the next Token (if any).
*/
{
	register Token *retp;	/* return token	*/

	DBGASSERT(tp != 0);
	DBGASSERT(tp->code != C_Nothing);
	/* for now - DBGCALL() ? */
	DBGCODE(if (DEBUG('z') > 0)	(void)fprintf(stderr, "tk_rm(tp=%#lx)\n", tp));
	DBGCODE(tp->code = C_Nothing);
	retp = tp->next;
	tp->next = freetokens;
	freetokens = tp;
	return (retp);
}

void
tk_rml(tp)
	Token *tp;
/* Given a - possibly null - pointer to a Token list, this routine
** deallocates all Tokens to the free Token list.
*/
{
	register Token * eotp;	/* end of token list	*/

#ifdef DEBUG
	if ( DEBUG('z') > 0 )
	{
		(void)fprintf(stderr, "tk_rml called with:");
		tk_prl(tp);
	}
#endif
	if (tp != 0)
	{
		for(eotp = tp; eotp->next != 0; eotp = eotp->next)
		{
			DBGASSERT(eotp->code != C_Nothing);
			DBGCODE(eotp->code = C_Nothing);
		}
		DBGASSERT(eotp->code != C_Nothing);
		DBGCODE(eotp->code = C_Nothing);
		eotp->next = freetokens;
		freetokens = tp;
	}
}

void
tk_rmto(fromtp, totp)
	Token * fromtp;
	register Token * totp;
/* Given pointers to the beginning of a Token list and a Token
** within that list, this routine deallocates all Tokens in the list
** up to, but not including the second argument pointer.
** It is expected that neither argument is 0.
*/
{
	register Token * tp;	/* token in the list	*/

	COMMENT(fromtp != 0 && totp != 0);
	if ((tp = fromtp) == totp)
		return;
	else
	{
		for ( ; tp->next != totp; tp = tp->next)
		{
			DBGASSERT(tp->code != C_Nothing);
			DBGCODE(tp->code = C_Nothing);
		}
		DBGASSERT(tp->code != C_Nothing);
		DBGCODE(tp->code = C_Nothing);
		tp->next = freetokens;
		freetokens = fromtp;
	}
}

Token *
tk_rmws(itp)
	Token * itp;
/* Given a pointer to a (possibly null) Token list, this routine 
** returns a pointer to the first pp-token
** in the list, or 0 if the list is null or all white space Tokens.
** Any leading white space Tokens are placed
** on the free list of Tokens.
*/
{
	register Token * tp;	/* finds first pp-token	*/
	Token * wstp;		/* white space Token	*/

	if (itp == 0)
	{
#ifdef DEBUG
		if (DEBUG('t') > 1)
			(void)fprintf(stderr, "tkrmws() returns (Token *)0\n");
#endif
		return 0;
	}
	wstp = 0;
	for(tp = itp; tp->code == C_WhiteSpace;)
	{
		COMMENT(tp->code != C_Nothing);
		DBGCODE(tp->code = C_Nothing);
		wstp = tp;
		if ((tp = tp->next) == 0)
			break;
	}
	if (wstp != 0)
	{
		wstp->next = freetokens;
		freetokens = itp;
	}
#ifdef DEBUG
	if (DEBUG('t') > 1)
	{
		(void)fprintf(stderr, "tkrmws() returns:");
		tk_pr(tp, '\n');
	}
#endif
	return tp;
}

Token *
tk_tokenize(buffer)
	char * buffer;
/* Given a pointer to a null-terminated buffer,
** this routine returns a Token list that corresponds
** to the contents of the buffer.
** The null byte is not tokenized.
** If there is a new-line character in the buffer,
** is is tokenized as white-space of length 1.
** Because the buffer is assumed not to contain
** a directive, there is no directive processing
** or context-sensitive tokenization.
*/
/* Implementation Note: When tokenizing long strings (L'...' and L"...")
** this routine assumes that there are no multiple character mappings to backslash,
** new-line, single quote, or double quote.
*/
{
	register char *lp;	/* current input character	*/
	register char * cp;	/* pointer to beginning of Token */
	register int code;	/* value for `code' field of Token  */
	register Token *tp;	/* current token	*/
	Token head;		/* anchor for return Token list */
#define GOTO_ONECHAR	goto onechar
#define GOTO_TWOCHAR	goto twochar
#define GOTO_THREECHAR	goto threechar

#ifdef DEBUG
	if (DEBUG('b') > 0)
		(void)fprintf(stderr, "tk_tokenize()\n");
#endif
	lp = buffer;
	head.next = 0;
	tp = &head;
	for (;;)
	{
#ifdef DEBUG
		if (DEBUG('b') > 5)
		{
			(void)fprintf(stderr, "loop on:`%c'\n", *lp);
		}
#endif
		switch (*(cp = lp))
		{
		case ':':
#ifdef CPLUSPLUS
			if (lp[1] == ':')
			{
				code = C_Scope;
				GOTO_TWOCHAR;
			}
#endif
			code = C_Colon;
			GOTO_ONECHAR;

		case '$':
			code = C_Dollar;
			GOTO_ONECHAR;
			
		case '?':
			code = C_Question;
			GOTO_ONECHAR;

		case '(':
			code = C_LParen;
			GOTO_ONECHAR;

		case ')':
			code = C_RParen;
			GOTO_ONECHAR;

		case ',':
			code = C_Comma;
			GOTO_ONECHAR;

		case '~':
			code = C_Complement;
			GOTO_ONECHAR;

		case '[':
			code = C_LBracket;
			GOTO_ONECHAR;

		case ']':
			code = C_RBracket;
			GOTO_ONECHAR;

		case '{':
			code = C_LBrace;
			GOTO_ONECHAR;

		case '}':
			code = C_RBrace;
			GOTO_ONECHAR;

		case ';':
			code = C_SemiColon;
			GOTO_ONECHAR;

		default:code = C_Operator;
onechar:		lp++;
			break;

		case '^':
			if (lp[1] == '=')
			{
				code = C_XORAssign;
twochar:			lp += 2;
				break;
			}
			code = C_ExclusiveOR;
			GOTO_ONECHAR;

		case '%':
			if (lp[1] == '=')
			{
				code = C_ModAssign;
				GOTO_TWOCHAR;
			}
			code = C_Mod;
			GOTO_ONECHAR;

		case '*':
			if (lp[1] == '=')
			{
				code = C_MultAssign;
				GOTO_TWOCHAR;
			}
			code = C_Mult;
			GOTO_ONECHAR;

		case '=':
			if (lp[1] == '=')
			{
				code = C_Equal;
				GOTO_TWOCHAR;
			}
			code = C_Assign;
			GOTO_ONECHAR;

		case '!':
			if (lp[1] == '=')
			{
				code = C_NotEqual;
				GOTO_TWOCHAR;
			}
			code = C_Not;
			GOTO_ONECHAR;

		case '#':
			if (lp[1] == '#')
			{
				code = C_Paste;
				GOTO_TWOCHAR;
			}
			lp++;
			code = C_Sharp;
			break;

		case '+':
			switch (lp[1])
			{
			case '+':
				code = C_Increment;
				GOTO_TWOCHAR;

			case '=':
				code = C_PlusAssign;
				GOTO_TWOCHAR;

			default:code = C_Plus;
				GOTO_ONECHAR;
			}

		case '&':
			switch (lp[1])
			{
			case '&':
				code = C_LogicalAND;
				GOTO_TWOCHAR;

			case '=':
				code = C_ANDAssign;
				GOTO_TWOCHAR;

			default:code = C_BitwiseAND;
				GOTO_ONECHAR;
			}

		case '|':
			switch (lp[1])
			{
			case '|':
				code = C_LogicalOR;
				GOTO_TWOCHAR;

			case '=':
				code = C_ORAssign;
				GOTO_TWOCHAR;

			default:code = C_InclusiveOR;
				GOTO_ONECHAR;
			}

		case '.':
			if (tk_chtypes[((unsigned char *)lp)[1]] & CH_DECIMAL)
				goto float_const;
			switch (lp[1])
			{
#ifdef CPLUSPLUS
			case '*':
				code = C_DotStar;
				GOTO_TWOCHAR;
#endif
			case '.':
				if (lp[2] == '.')
				{
					code = C_Ellipsis;
threechar:				lp += 3;
					break;
				}
			default:code = C_Dot;
				GOTO_ONECHAR;
			}
			break;

		case '>':
			switch (lp[1])
			{
			case '>':
				if (lp[2] == '=')
				{
					code = C_RightAssign;
					GOTO_THREECHAR;
				}
				code = C_RightShift;
				GOTO_TWOCHAR;

			case '=':
				code = C_GreaterEqual;
				GOTO_TWOCHAR;

			default:code = C_GreaterThan;
				GOTO_ONECHAR;
			}

		case '-':
			switch (lp[1])
			{
			case '-':
				code = C_Decrement;
				GOTO_TWOCHAR;

			case '=':
				code = C_MinusAssign;
				GOTO_TWOCHAR;

			case '>':
#ifdef CPLUSPLUS
				if (lp[2] == '*')
				{
					code = C_ArrowStar;
					GOTO_THREECHAR;
				}
#endif
				code = C_Arrow;
				GOTO_TWOCHAR;

			default:code = C_Minus;
				GOTO_ONECHAR;
			}

		case '<':
			switch (lp[1])
			{
			case '<':
				if (lp[2] == '=')
				{
					code = C_LeftAssign;
					GOTO_THREECHAR;
				}
				code = C_LeftShift;
				GOTO_TWOCHAR;

			case '=':
				code = C_LessEqual;
				GOTO_TWOCHAR;

			default:code = C_LessThan;
				GOTO_ONECHAR;
			}

		case '/':
			switch (lp[1])
			{
			case '=':
				code = C_DivAssign;
				GOTO_TWOCHAR;

			case '*':
				code = C_WhiteSpace;
				for (lp++; ; )
				{
					if (*++lp == '*' && lp[1] == '/')
					{
						lp += 2;
						break;
					}
					if (*lp == '\0')
					{
						code = C_BadInput;
						goto out;
					}
				}
				break;

			case '/':
				if (pp_flags & F_TWO_COMMENTS)
				{
					code = C_WhiteSpace;
					lp = strchr(lp, '\0');
					goto out;
				}
			default:code = C_Div;
				GOTO_ONECHAR;
			}
			break;

		case '\0':
			code = C_Nothing;
			goto out;

		case '\n':
			code = C_WhiteSpace;
			lp++;
			break;

		case ' ':
		case '\t':
		case '\f':
		case '\v':
			code = C_WhiteSpace;
			while (tk_chtypes[(unsigned char)(*++lp)] & CH_WHITE)	;
			break;

		case 'L':
			if (cp[1] == '\'')
			{
				lp++;
				goto char_const;
			}
			else if (cp[1] == '"')
			{
				lp++;
				code = C_Wstring;
				goto string_lit;
			}
			/*FALLTHRU*/
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		case 'g':
		case 'h':
		case 'i':
		case 'j':
		case 'k':
		case 'l':
		case 'm':
		case 'n':
		case 'o':
		case 'p':
		case 'q':
		case 'r':
		case 's':
		case 't':
		case 'u':
		case 'v':
		case 'w':
		case 'x':
		case 'y':
		case 'z':
		case '_':
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
		case 'H':
		case 'I':
		case 'J':
		case 'K':
		case 'M':
		case 'N':
		case 'O':
		case 'P':
		case 'Q':
		case 'R':
		case 'S':
		case 'T':
		case 'U':
		case 'V':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			code = C_Identifier;
			while (tk_chtypes[(unsigned char)(*++lp)] & CH_IDENT)	;
			break;

		case '0':
			code = C_I_Constant;
			if (lp[1] == 'x' || lp[1] == 'X')
			{
				lp++;
				while (tk_chtypes[(unsigned char)(*++lp)] & CH_HEX)	;
				if (lp - cp <= 2)
				{
					code = C_BadInput;
				}
			}
			else
			{
octal:				while (tk_chtypes[(unsigned char)(*++lp)] & CH_OCTAL)	;
				switch (*lp)
				{
				case '8':
				case '9':
#ifdef TRANSITION
					if (pp_flags & F_Xt)
						goto octal;
#endif
					code = C_BadInput;
					goto float_digit;

				case '.':
					goto float_const;
				case 'E':
				case 'e':
					code = C_F_Constant;
					goto float_econst;
				}
			}
			goto int_suffix;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			code = C_I_Constant;
float_digit:		while (tk_chtypes[(unsigned char)(*++lp)] & CH_DECIMAL)	;
			switch ( *lp )
			{
			case '.':
float_const:			while( tk_chtypes[(unsigned char)(*++lp)] & CH_DECIMAL )
					;
				code = C_F_Constant;
				if ( *lp != 'e' && *lp != 'E' )
					goto float_suffix;
				/*FALLTHRU*/
			case 'e':
			case 'E':
float_econst:			if (tk_chtypes[(unsigned char)(*++lp)] & CH_PLUS_MINUS)
					lp++;
				if (tk_chtypes[(unsigned char)(*lp)] & CH_DECIMAL)
				{
					while (tk_chtypes[(unsigned char)(*++lp)] & CH_DECIMAL)
						;
					code = C_F_Constant;
				}
				else
				{
					code = C_BadInput;
				}
float_suffix:			switch (*lp)
				{
				case 'f':
				case 'F':
				case 'l':
				case 'L':
					lp++;
				}
				goto ppnumber;
			}
int_suffix:		if (tk_chtypes[*((unsigned char *)lp)] & CH_INT_SUFFIX)
				switch (*++lp)
				{
				case 'l':
				case 'L':
					if (lp[-1] == 'l' || lp[-1] == 'L')
						break;
					lp++;
					break;
	
				case 'u':
				case 'U':
					if (lp[-1] == 'u' || lp[-1] == 'U')
						break;
					lp++;
					break;
				}
ppnumber:		for( ; ; code = C_BadInput, lp++)
				if (tk_chtypes[*((unsigned char *)lp)] & CH_ALPHANUM_DOT)
					continue;
				else if (tk_chtypes[*((unsigned char *)lp)] & CH_PLUS_MINUS)
					if (lp[-1] == 'E' || lp[-1] == 'e')
						continue;
					else
						break;
				else
					break;
			break;

		case '"':
			code = C_String;
string_lit:		while (*++lp != '"')
			{
				if (*lp == '\\')
				{
					if (*++lp != '\0')
						continue;
				}
				if (*lp == '\0')
				{
					code = C_BadInput;
					goto out;
				}
			}
			lp++;
			break;

		case '\'':
char_const:		code = C_C_Constant;
			while (*++lp != '\'')
			{
				if ((*lp == '\\') && (*++lp != '\0'))
					continue;
				if (*lp == '\0')
				{
					code = C_BadInput;
					goto out;
				}
			}
			lp++;
			break;

		} /* bottom of: `switch (*(cp = lp))' */
		tp->next = tk_new();
		tp = tp->next;
		tp->ptr.string = cp;
		tp->rlen = lp - cp;
		tp->code = (unsigned short) code;
		tp->number = bf_lineno;
		tp->next = 0;
		if (code == C_Identifier)
			tp->aux.hid = 0;
		continue;

out:		if (code != C_Nothing)
		{
			COMMENT(code == C_BadInput
			 && (cp[0] == '\''
				 || cp[0] == '\"'
				 || cp[0] == 'L'
				 || (cp[0] == '/' && cp[1] == '*')));
			tp->next = tk_new();
			tp = tp->next;
			tp->ptr.string = cp;
			tp->rlen = lp - cp;
			tp->code = (unsigned short) code;
			tp->number = bf_lineno;
			tp->next = 0;
		}
		break;
	} /* bottom of: `for(;;)' */
#ifdef DEBUG
	if (DEBUG('b') > 1)
	{
		(void)fprintf(stderr, "tk_tokenize() returns:");
		tk_prl(head.next);
	}
#endif
	return head.next;
}

#ifdef DEBUG
#include "syms.h"
void
tk_pr(tp, ch)
/* Given a (possibly null) Token pointer and a character, this
** routine prints (on stderr) the single Token's information
** followed by the character.
*/
	register Token *tp;
	register int ch;
{
	register char *p;

	if (tp == 0)
	{
		(void)fprintf(stderr, "Token@0%c", ch);
		return;
	}
	if (tp->code < 0 || (TK_ENUMNO(tp->code) > sizeof(codenames) / sizeof(char *)))
		p = "UNKNOWN!";
	else
		p = tk_saycode(tp->code);
	if (TK_ISFORMAL(tp))
	{
		(void)fprintf(stderr, "Token@%#lx,%d:%s#%d%c",
			tp, tp->number, p, tp->aux.argno, ch);
	}
	else if (tp->code == C_Marker)
	{
		(void)fprintf(stderr, "Token@%#lx,%d:`", tp, tp->number);
		pp_printmem(tp->ptr.macro->name, tp->ptr.macro->namelen);
		(void)fprintf(stderr, "',%s%c", p, ch);
	}
	else if (tp->code == C_Paste)
	{
		(void)fprintf(stderr, "Token@%#lx,%d:`", tp,
		 (tp->code != C_Nothing) ? tp->number : 0);
		if (tp->code != C_Nothing)
			pp_printmem(tp->ptr.string, tp->rlen);
		(void)fprintf(stderr, "',%s,%s%c", p,
		 (tp->aux.ws & W_LEFT)
		 ?	(tp->aux.ws & W_RIGHT) ? "LR" : "L"
		 :	(tp->aux.ws & W_RIGHT) ? "R" : "noWS",
		 ch);
	}
	else if (tp->code == C_Identifier)
	{
		(void)fprintf(stderr, "Token@%#lx,%d:`", tp,
		 (tp->code != C_Nothing) ? tp->number : 0);
		if (tp->code != C_Nothing)
			pp_printmem(tp->ptr.string, tp->rlen);
		(void)fprintf(stderr, "',%s,%c%c", p, (tp->aux.hid)?'H':'u', ch);
	}
	else
	{
		(void)fprintf(stderr, "Token@%#lx,%d:`", tp,
		 (tp->code != C_Nothing) ? tp->number : 0);
		if (tp->code != C_Nothing)
			pp_printmem(tp->ptr.string, tp->rlen);
		(void)fprintf(stderr, "',%s%c", p, ch);
	}
}

void
tk_prl(tp)
/* Given a pointer to a (possibly null) list of Tokens,
** this routine prints the Token list contents on stderr.
*/
	register Token *tp;
{
	register int cnt;

	(void)fprintf(stderr, "Token list================\n");
	cnt = 0;
	while (tp != 0)
	{
		if (++cnt < 2)
		{
			fputc('\t',stderr);
			tk_pr(tp, ' ');
		}
		else
		{
			tk_pr(tp, '\n');
			cnt = 0;
		}
		tp = tp->next;
	}
	if ( cnt % 2 == 0 )
		(void)fputc('\t',stderr);
	(void)fprintf(stderr, "============End Token list\n");
}
#endif
 
#ifdef DEBUG
char *
tk_saycode(code)
	int code;
/* Given a Token, this routine returns a pointer
** to the corresponding name string.
*/
{
	return codenames[TK_ENUMNO(code)];
}
#endif
 
static void
setchtypes(list, bits)
	register const unsigned char *list;
	register int bits;
/* Given a pointer to a string of characters and a bit mask
** that describes what lexical classification the characters
** fall into, this routine initializes a data structure used
** during the tokenization process.
*/
{
	register int c;	/* a character in the `list' */

	while ((c = *list++) != '\0')
		tk_chtypes[c] |= bits;
}

void
tk_init()
/* Initialize data structures in token.c. */
{
	COMMENT(TK_INIT > 0);
	alloctokens(TK_INIT);
	setchtypes((unsigned char*) "+-", CH_PLUS_MINUS);
	setchtypes((unsigned char*) " \t\f\v", CH_WHITE);
	setchtypes((unsigned char*) "01234567", CH_IDENT | CH_OCTAL | CH_HEX | CH_DECIMAL | CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) "89", CH_IDENT | CH_HEX | CH_DECIMAL | CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) "abcdefABCDEF", CH_IDENT | CH_HEX | CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) "luLU", CH_INT_SUFFIX | CH_IDENT | CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) "ghijkmnopqrstvwxyzGHIJKMNOPQRSTVWXYZ", CH_IDENT | CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) ".", CH_ALPHANUM_DOT);
	setchtypes((unsigned char*) "_", CH_IDENT);
	COMMENT(TK_ENUMNO(C_Marker) + 1 == sizeof(codenames)/sizeof(char *));
}
