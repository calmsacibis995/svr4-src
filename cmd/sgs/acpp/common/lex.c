/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/lex.c	1.31"
/* lexically analyze the input, and output the tokens */
#include <stdio.h>
#include <memory.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"
#include "syms.h"

/* This file contains output routines and a buffer
** to temporarily store the output tokens.
*/
#ifdef DEBUG
#    	define 	DEPTH   100
#ifdef __STDC__
#	define	DBGCALL(num,funcname) \
	if ( DEBUG('e') > (num) )					\
	{								\
		(void)fprintf(stderr, #funcname " called with:");	\
		st_mprint(mp);						\
	}
#	define	DBGCALLT(num,funcname) \
	if ( DEBUG('e') > (num) )					\
	{								\
		(void)fprintf(stderr, #funcname " called with:");	\
		tk_pr(tp,'\n');					\
	}
#       define DBGRET(num,funcname,tp) \
                if ( DEBUG('e') > (num) )				\
                {							\
                        (void)fprintf(stderr, #funcname " returns:");	\
                        tk_pr((tp),'\n');				\
                }
#       define DBGRETL(num,funcname,tp) \
                if ( DEBUG('e') > (num) )				\
                {							\
                        (void)fprintf(stderr, #funcname " returns:");	\
                        tk_prl((tp));				\
                }
#else
#	define	DBGCALL(num,funcname) \
	if ( DEBUG('e') > (num) )					\
	{								\
		(void)fprintf(stderr, "funcname called with:");		\
		st_mprint(mp);						\
	}
#	define	DBGCALLT(num,funcname) \
	if ( DEBUG('e') > (num) )					\
	{								\
		(void)fprintf(stderr, "funcname called with:");		\
		tk_pr(tp,'\n');					\
	}
#       define DBGRET(num,funcname,tp) \
                if ( DEBUG('e') > (num) )				\
                {							\
                        (void)fprintf(stderr, "funcname returns:");	\
                        tk_pr((tp),'\n');				\
                }
#       define DBGRETL(num,funcname,tp) \
                if ( DEBUG('e') > (num) )				\
                {							\
                        (void)fprintf(stderr, "funcname returns:");	\
                        tk_prl((tp));				\
                }
#endif
#else
#	define DBGCALL(num,funcname)
#	define DBGCALLT(num,funcname) 
#       define DBGRET(num,funcname,tp)
#       define DBGRETL(num,funcname,tp)
#endif
#define BFLUSH()	\
	{\
		COMMENT(p <= plimit);\
		if (p > buf)\
		{\
			(void)fwrite(buf, p - buf, 1, stdout);\
			p = buf;\
		}\
	}
/* An output buffer of fixed length is maintained.
** The convention is to always allow space for one more
** character in the buffer, so that a single ' ' can
** be written to separate tokens without
** making a costly check for "end of buffer".
*/
#define BUFLEN	BUFSIZ		/* length of output buffer */
#define BUFEOD	(BUFLEN - 1)	/* number of chars allowed before space for one more ' ' */

static	char 	buf[BUFLEN];	/* output buffer	*/
static	char *	plimit;		/* end of buffer, with room for one more ' ' */

#if	defined(MERGED_CPP) || defined(DEBUG)
static	Token	head;	/* anchor for linked list of tokens */
static	Token	*pretp;	/* pointer to token returned in previous lx_token() call */
#endif


#if	defined(MERGED_CPP)
Token *
lx_gettokens()
/* Returns the Token list corresponding to the next preprocessed
** logical line, or 0 if at the end of the original source file.
** This is the driving routine when the tokens are to be returned to the parser,
** one line  at a time.
** The contents of the returned list and any memory that 
** any internal pointers may access is guarenteed by `acpp`
** until the next call to this routine.
** Furthermore, `acpp` guarentees that each line of tokens
** returned by this routine shall end with a white-space
** Token of length one, which shall be a new-line.
** It is expected that the caller treat the output of this routine
** as "read only" and not perform any other operations on it.
** This routine is responsible for deallocating Tokens returned
** by previous invocations of this routine.
*/
{
	tk_rml(head.next);
	if ((head.next = bf_tokenize(B_tokens, (Token *)0)) == 0)
		return (Token *)0;
	if (!fl_dotisource())
		head.next = ex_input(head.next);
#ifdef DEBUG
	if(DEBUG('l') > 1)
	{
		(void)fprintf(stderr, "lx_gettokens() returns:");
		tk_prl(head.next);
	}
#endif
	COMMENT((tk_eol(head.next))->rlen == 1);
	COMMENT((tk_eol(head.next))->code == C_WhiteSpace);
	COMMENT((tk_eol(head.next))->ptr.string[0] == '\n');
	return head.next;
}
#endif

void
lx_init()
/* Initializes the data structures in lex.c. */
{
	COMMENT(head.next == 0);
	plimit = buf + BUFEOD;
#if	defined(MERGED_CPP) || defined(DEBUG)
	pretp = tk_new();			/* to be removed by tk_token() */
	pretp->code = C_Goofy;
#endif
}

static char dotifile;
/*
** return 1 if generating a '.i' file
 */
char
lx_dotifile()
{
	return(dotifile);
}

Token *
lx_input()
/* Writes the all preprocessed output to a file, and then returns 0.
** This is the driving routine when the output is to be directived to
** a file (see lx_token()).
*/
{
	register Token *tp;	/* a Token in a list	*/
	register enum {
		P_nospace,	/* don't precede Token with a space */
		P_prspace	/* precede Token with a space */
	} space;
	register char * p;	/* next available element in output buffer */
	register char * cp;	/* first char in Token */
	register int len;	/* length of Token */
	register Token * firstp;/* first Token returned by ex_input() */

	dotifile = 1;
#ifdef MERGED_CPP
	if (fl_dotisource())
		WARN("preprocessing a .i file");
#endif
	for (p = buf; (tp = bf_tokenize(B_text, (Token *)0)) != 0; )
	{
		space = P_prspace;	/* for now - bf_lastws()  */
					/* to communicate with buf.c about spaces*/
		for (firstp = tp = ex_input(tp); tp != 0; tp = tp->next)
		{
			cp = tp->ptr.string;
			switch (TK_ENUMNO(tp->code))
			{
			CASE(C_Operator)
			CASE(C_Identifier)
			CASE(C_I_Constant)
			CASE(C_F_Constant)
			CASE(C_C_Constant)
			CASE(C_String)
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				break;

			CASE(C_Dot)
			CASE(C_Dollar)
			CASE(C_RParen)
			CASE(C_Comma)
			CASE(C_Question)
			CASE(C_Colon)
			CASE(C_InclusiveOR)
			CASE(C_ExclusiveOR)
			CASE(C_BitwiseAND)
			CASE(C_GreaterThan)
			CASE(C_SemiColon)
			CASE(C_LessThan)
			CASE(C_Plus)
			CASE(C_Minus)
			CASE(C_Mult)
			CASE(C_Div)
			CASE(C_Mod)
			CASE(C_Complement)
			CASE(C_Not)
			CASE(C_LParen)
			CASE(C_Sharp)
			CASE(C_LBracket)
			CASE(C_RBracket)
			CASE(C_LBrace)
			CASE(C_RBrace)
			CASE(C_Assign)
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				if (p + 1 >= plimit)
					BFLUSH()
				*p++ = *cp;
				continue;

			CASE(C_Equal)
			CASE(C_LogicalOR)
			CASE(C_LogicalAND)
			CASE(C_NotEqual)
			CASE(C_GreaterEqual)
			CASE(C_LessEqual)
			CASE(C_LeftShift)
			CASE(C_RightShift)
			CASE(C_Arrow)
			CASE(C_MultAssign)
			CASE(C_DivAssign)
			CASE(C_ModAssign)
			CASE(C_PlusAssign)
			CASE(C_MinusAssign)
			CASE(C_ANDAssign)
			CASE(C_Increment)
			CASE(C_Decrement)
			CASE(C_Paste)
			CASE(C_XORAssign)
			CASE(C_ORAssign)
#ifdef CPLUSPLUS
			CASE(C_DotStar)
			CASE(C_Scope)
#endif

				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				if (p + 2 >= plimit)
					BFLUSH()
				p[0] = cp[0];
				p[1] = cp[1];
				p += 2;
				continue;

			CASE(C_LeftAssign)
			CASE(C_RightAssign)
			CASE(C_Ellipsis)
#ifdef CPLUSPLUS
			CASE(C_ArrowStar)
#endif
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				if (p + 3 >= plimit)
					BFLUSH()
				p[0] = cp[0];
				p[1] = cp[1];
				p[2] = cp[2];
				p += 3;
				continue;

			CASE(C_WhiteSpace)
				space = P_nospace;
				if (!(pp_flags & F_KEEP_COMMENTS) && cp[0] == '/')
				{
					if (cp[1] == '/')
						*p++ = '\n';
					else
						*p++ = ' ';
					if (p >= plimit)
						BFLUSH()
					continue;
				}
				if (tp->rlen == 1)
				{
					COMMENT(cp[0] != '/');
					*p++ = cp[0];
					if (p >= plimit)
						BFLUSH()
					continue;
				}
				break;

			default:COMMENT(TK_ISINTERNAL(tp) == 0);
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				break;

			CASE(C_Invalid)
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				TKERROR("invalid token",tp);
				break;

			CASE(C_BadInput)
				if (space == P_prspace)
					*p++ = ' ';
				else
					space = P_prspace;
				TKERROR( "invalid input token", tp);
				break;
			}
			COMMENT(p <= plimit);
			if (p + (len = tp->rlen) >= plimit)
			{
				BFLUSH()
				if (len >= BUFEOD)
				{
					(void)fwrite(cp, len, 1, stdout);
					continue;
				}
			}
			(void) memcpy(p, cp, len);
			p += len;
		}
		tk_rml(firstp);
		BFLUSH()
	}
	return (Token *)0;
}

#if	defined(MERGED_CPP) || defined(DEBUG)
Token *
lx_token()
/* Returns the next pp-token, or 0 if at the end of the original source file.
** This is the driving routine when the tokens are to be returned to the parser,
** one at a time.
** The caller should process one Token at a time, and not
** expect to look-ahead through the Token.next field,
** nor expect the values of the fields of any previous lx_token() call
** to be valid after another Token has been recieved from this routine.
*/
/* NOTE: it would help the speed of this routine 
** performed a tk_rml() at the end of every logical line.
*/
{
	register Token * tp;	/* a Token in the return queue	*/
	

	(void)tk_rm(pretp);
	for (tp = head.next;	;)
	{
		if (tp != 0)
		{
			if (tp->code == C_WhiteSpace )
			{
#ifdef LINT
				if (tp->ptr.string[0] != '/') {
#endif
					tp = tk_rm(tp);
					continue;
#ifdef LINT
				}
#endif
			}
			else
				/*EMPTY*/ COMMENT(TK_ISINTERNAL(tp) == 0);
			head.next = tp->next;
			tp->next = 0;
			pretp = tp;
			return tp;
		}
		if ((head.next = bf_tokenize(B_tokens, (Token *)0)) == 0)
			return (Token *)0;
#ifdef MERGED_CPP
		if (!fl_dotisource())
#endif
			head.next = ex_input(head.next);
		tp = head.next;
	}
}
#endif
