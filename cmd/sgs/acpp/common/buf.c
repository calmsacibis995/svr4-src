/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/buf.c	1.72"
/* buf.c - input buffer control and tokenization */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <memory.h>
#include "cpp.h"
#include "buf.h"
#include "file.h"
#include "group.h"
#include "predicate.h"
#include "syms.h"


/* This file contains the input buffer, and routines that tokenize
** the input one logical line at a time.
*/
#ifdef __STDC__
#	include <unistd.h>
#else
extern int read(/*	int, char *, unsigned	*/);
#endif

#ifdef DEBUG
#	define	DBGTK()	if (DEBUG('b') > 1)		\
		{							\
			if (tp == &head)				\
			{						\
				debugtk.ptr.string = cp;		\
				debugtk.rlen = lp - cp;			\
				debugtk.code = code;			\
				debugtk.number = bf_lineno;		\
				debugtk.aux.hid = 0;			\
				debugtk.next = 0;			\
			}						\
			(void)fprintf(stderr, "bf_tokenize() got: ");	\
			tk_pr((tp == &head ? &debugtk : tp), '\t');	\
			(void) fprintf(stderr, "dirflag=%s\n",		\
			 saydir[DIRFLAG()]);			\
			fflush(stderr);		\
			fflush(stdout);		\
		}
#else
#	define	DBGTK()
#endif

#if defined(DEBUG) && defined(__STDC__)
#	define	DBGCALLT(num,funcname) if ( DEBUG('b') > (num) )	\
	{								\
		(void)fprintf(stderr, #funcname" called with:");	\
		tk_pr(tp,'\n');					\
		fflush(stderr);	\
		fflush(stdout);	\
	}
#       define DBGRET(num,funcname,tp) if ( DEBUG('b') > (num) )	\
                {							\
                        (void)fprintf(stderr, #funcname" returns:");	\
                        tk_pr((tp),'\n');				\
			fflush(stderr);	\
			fflush(stdout);	\
                }
#       define DBGRETL(num,funcname,tp) if ( DEBUG('b') > (num) )	\
                {							\
                        (void)fprintf(stderr, #funcname" returns:");	\
                        tk_prl((tp));				\
			fflush(stderr);	\
			fflush(stdout);	\
                }
#else
#	if defined(DEBUG)
#	define	DBGCALLT(num,funcname) if ( DEBUG('b') > (num) )	\
	{								\
		(void)fprintf(stderr, "funcname called with:");		\
		tk_pr(tp,'\n');					\
		fflush(stderr);		\
		fflush(stdout);		\
	}
#       define DBGRET(num,funcname,tp) if ( DEBUG('b') > (num) )	\
                {							\
                        (void)fprintf(stderr, "funcname returns:");	\
                        tk_pr((tp),'\n');				\
			fflush(stderr);		\
			fflush(stdout);		\
                }
#       define DBGRETL(num,funcname,tp) if ( DEBUG('b') > (num) )	\
                {							\
                        (void)fprintf(stderr, "funcname returns:");	\
                        tk_prl((tp));				\
			fflush(stderr);		\
			fflush(stdout);		\
                }
#	else
#	define DBGCALLT(num,funcname) 
#       define DBGRET(num,funcname,tp)
#       define DBGRETL(num,funcname,tp)
#	endif
#endif
#define B_NEXTLINE	0x1	/* requires special handling by nextline() */
#define B_FASTSCAN	0x2	/* requires special handling by fastscan() */
#define CHECKDIR()	if (!TST(D_no|D_other|D_include)) {		\
				if (TST(D_null))			\
					SET(D_other);			\
				else if (TST(D_maybe))			\
					SET(D_no);			\
			}

#define DIR_MASK	0x001F	/* bit mask for `typedef enum _directive' */
#define MODE_MASK	0x1F00	/* bit mask for `typedef enum mode_' */
#define	NLINCR		64	/* increament value for bf_nlposition */

long	bf_lineno;		/* current line number for error reporting	*/
int	bf_newlines;	/* number of <newlines> not accounted for in `bf_lineno' */

/* The input is read into a dynamic buffer, which is maintained 
** by "static char * nextline()" using these variables.
*/
long	bf_cur;		/* current location in the buffer */
long	bf_eod;		/* end of amount read into buffer */
long	bf_prev = -1;	/* end of the last line that can be
			** safely discarded from the buffer. Normally, this
			** is the end of the PREVious logical line. However,
			** many logical lines have to be maintained in the case,
			** for instance,
			** of a multi-line function-like macro invocation.
			*/
char *	bf_ptr;		/* the input buffer */
static unsigned long buf_len;	/* length of the input buffer */
static unsigned short goteof;
/* A depiction of the buffer after nextline() returns
** the second logical line of a three line program:
**
** main(){
** puts("hello");
** }
** <EOF>				NOTE: `*' means a garbage value
**			     1 1 1 1 1 1 1 1 1 1 2 2 2  2 2  2 2
**	0 1 2 3 4 5 6 7  8 9 0 1 2 3 4 5 6 7 8 9 0 1 2  3 4  5 6
**	__________________________________________________________    ___
**	|m|a|i|n|(|)|{|^J|p|u|t|s|(|"|h|e|l|l|o|"|)|;|^J|}|^J|0|*| ~  |*|
**	|_|_|_|_|_|_|_|__|_|_|_|_|_|_|_|_|_|_|_|_|_|_|__|_|__|_|_| ~  |_|
**	^
**	|<--- bf_prev -->|  				|    |  	|
**	|<------------------- bf_cur ------------------>|    |  	|
**	|<--------------------- bf_eod --------------------->|  	|
**	|<------------------------- buf_len ---------------------- ~ -->|
**	|
**   bf_ptr
**
** NOTE: the '\0' at pf_ptr[25] is the BF_SENTINEL character that signifies
** the end of data. As part of the buffering management scheme, a '\0'
** will always be maintained at bf_ptr[bf_eod]. Of course, in this example
** the '\0' is normally thought of as an EOF. However, even if EOF hasn't
** been read(), a BF_SENTINEL will always be at the end of the buffer data.
*/

/* When working with multiple lines (in multi-line comments and
** multi-line macro invocations), it is important to keep track
** of a pointer to the first line in case the buffer realloc()'s
** in a later line and all the Token.ptr.string's pointing
** into the buffer have to be readjusted.
*/
static char *firstline;

/* Bit table of all possible input characters (initialized in bf_init())
** A non-zero bit value  means the character requires special processing
** by a routine in this file. The bits are described by the B_* macros.
*/
static	char	bufchars[256];

/* bf_nlposition is a table containing the positions of `\\n' and commented '\n'
** within the input buffer. Initially it is allocated NLINCR slots, and then when
** necessary reallocated in nlincr().
*/
static	long*	bf_nlposition;
static	long*	bf_nlptr;
static	long	bf_nlnumber;
static	void	nladd();
static	void	nlincr();

/* Because this diagnostic may be issue from multiple routines,
** it is put at file scope to save space.
*/
static char inarglistmsg[] = "directive not honored in macro argument list"; /* used in calls to UERROR and WARN */

/* While tokenizing a logical line, it is important to keep
** track of whether it forms a directive.
** One reason is that tokenization is context-dependent,
** in the case of headers in #include directives.
** The categorization of a logical line is maintained
** by a set of bit variables, which represent states
** in a FSA. The start state is "D_maybe." If the first Token
** is `#', the D_null bit is set, else the D_no bit is set.
** After the D_null bit has been set, an `include' causes
** the D_include bit to be set, else any other token causes
** the D_other bit to be set.
** Note that while processing a line bits are set,
** not cleared. In this scheme, all forms of a
** directive line will always have the D_null bit set:
** this allows a simple bit test to check if the line
** should be handled as a directive.
*/
#define ISDIRECTIVE()	(TST(D_null))	/* non-zero if a directive line */
#ifdef __STDC__
typedef enum _directive {
	D_no	= 0x10,	/* definitly not a directive line	*/
	D_maybe	= 0x08,	/* no pp-tokens have been encountered	*/
	D_null	= 0x04,	/* a null directive	*/
	D_include=0x01,	/* a #include directive */
	D_other	= 0x02	/* a (possibly invalid) directive other than #, #include */
} Dir;	/* directive category of a logical line */
#else	/* for Amdahl cc and 2.1 */
typedef int Dir;
#	define	D_no		(0x10)
#	define	D_maybe		(0x08)
#	define	D_null		(0x04)
#	define	D_include	(0x01)
#	define	D_other		(0x02)
#endif

#ifdef __STDC__
typedef enum _dirnum {
	N_invalid = -1,
	N_if = 0,
	N_ifdef,
	N_ifndef,
	N_elif,
	N_else,
	N_endif,
	N_error,
	N_define,
	N_assert,
	N_unassert,
	N_file,
	N_ident,
	N_lineinfo,
	N_pragma,
	N_include,
	N_line,
	N_undef
} Dirnum;
#else /* 2.1 can't handle assigning enum members to int */
typedef int Dirnum;
#define	N_invalid 	-1
#define	N_if		0
#define	N_ifdef		1
#define	N_ifndef	2
#define	N_elif		3
#define	N_else		4
#define	N_endif		5
#define	N_error		6
#define	N_define	7
#define	N_assert	8
#define	N_unassert	9
#define	N_file		10
#define	N_ident		11
#define	N_lineinfo	12
#define	N_pragma	13
#define	N_include	14
#define	N_line		15
#define	N_undef		16
#endif
#define	IFGROUP_DIRECTIVE(x)	((x) >= N_if && (x) <= N_endif)
#define IS_VALID_DIRECTIVE(x)	(((x) >= N_if) && ((x) <= N_undef))
#define DISALLOWED_IN_ARGLIST(x)((x) >= N_file)
#define COMPILER_CONTROL_LINE(x)(((x) >= N_file) && ((x) <= N_pragma))
#define NON_ANSI_DIRECTIVE(x)	(((x) >= N_assert) && ((x) <= N_lineinfo))

#ifdef DEBUG
#define SAYMODE()	(saymode[(mode) >> 8])
static char * saydir[DIR_MASK];
static char * saymode[] = { /* see `enum { B_text, ... };' in buf.h */
	0,
	"B_text",
	"B_tokens",
	0,
	"B_invocation",
	0,	0,	0,
	"B_macroname",
	0,	0,	0,	0,	0,	0,	0,
	"B_comment"
};
#endif
static Token	head;	/* "anchor" for Token list */
static int	backup;	/* indicates a directive's Tokens are "saved".
			** (see bf_tokenize() function comment and look for
			** the "B_macroname" mode explanantion).
			*/
static int	savedirflag; /* used in concert with (the above) `backup',
			** this records the type of directive that the "saved"
			** line is.
			*/
static Token	macro;	/* used to test if an identifer is a macro.
			** is initialized in bf_init().
			*/
	
#ifdef DEBUG
static	Token debugtk;	/* used to produce debugging output */
#endif

static void	adjust(   /* Token *, ptrdiff_t	*/ );
static Dirnum	dircmp(	  /* Token *	*/ );
static void	directive(/* int	*/ );
static Token *	dofile(  /* Token *, int	*/ );
static Token *	doident(  /* Token *, int	*/ );
static Token *	dopragma( /* Token *, int	*/ );
static char *	nextline( /* int, int 	*/ );

static void
adjust(tp, diff)
	Token * tp;
	ptrdiff_t diff;
/* Given a pointer to a possibly empty linked Token list and
** a pointer offset, this routine adjusts the character pointer
** of each Token into the input buffer by the given offset.
** This maintains the sanity of the linked list when the input
** buffer is realloc()-ed.
** This routine must be careful not to adjust pointers into other
** character arrays
** (those malloc()-ed for the replacement list of a macro invocation).
*/
{
#ifdef DEBUG
	if (DEBUG('b') > 1)
 		(void)fprintf(stderr, 
	 	"adjust() fix pointers: diff=%d\n", diff);
#endif
	for ( ; tp != 0; tp = tp->next)
		if (! TK_ISREPL(tp))
			tp->ptr.string += diff;
}

unsigned int
bf_curline()
/* returns the current logical line number */
{
	return (unsigned int) bf_lineno;
}

void
bf_init()
/* Initializes the data structures in buf.c */
{
	buf_len = 4 * BUFSIZ + sizeof((char) BF_SENTINEL);
	bf_ptr = pp_malloc((unsigned)buf_len);
	bufchars['?']	= B_NEXTLINE;
	bufchars['\0']	= B_NEXTLINE;
	bufchars['\n']	= B_NEXTLINE;
	bufchars['"']	|= B_FASTSCAN;
	bufchars['/']	|= B_FASTSCAN;
	bufchars['<']	|= B_FASTSCAN;
	bufchars['\0']	|= B_FASTSCAN;
	bufchars['\'']	|= B_FASTSCAN;
	macro.code = C_Identifier;
	bf_nlptr =  bf_nlposition = (long *) pp_malloc( NLINCR* sizeof(long*) );
	bf_nlnumber = NLINCR;
#ifdef DEBUG
	saydir[D_maybe] = "D_maybe";
	saydir[D_maybe|D_no] = "D_no";
	saydir[D_maybe|D_null] = "D_null";
	saydir[D_maybe|D_null|D_other] = "D_other";
	saydir[D_maybe|D_null|D_include] = "D_include";
#endif
}

Token *
bf_tokenize(mode, listp)
	Mode mode;
	Token * listp;
/* Given one of 4 modes to operate in and a pointer to a token list,
** this routine decomposes the characters
** in the next logical line of the input buffer into Tokens.
** The second parameter is a possibly null pointer to a list that this routine had
** previously returned, and allows this routine to adjust it's members
** pointers into the input buffer if the buffer has been moved by realloc().
** These Tokens, representing sequences of pp-tokens and white-space required
** by the 3rd phase of translation (sec. 2.1.1.2), are returned to the
** calling function. If the logical line is a non-directive line
** in the false part of an if-group, the tokens are not returned
** and the next logical line undergoes the entire process.
** If the logical line is a directive, directive() is called and the next
** logical line is tokenized. The mode controls certain details of this routine.
**
** The "B_tokens" mode is the normal case, and causes the behavior described above.
** The "B_text" mode causes modifies the behavior for non-directive logical
** lines - the first part of the line which doesn't have to be changed (because
** of a macro invocation, for instance) is written to the output file,
** and only the Tokens after
** this first part of the line (if any) are returned.
** If no Tokens are to be returned, the next logical line is processed.
** The "B_macroname" mode is used when the previous logical line ended with
** a possible function-like macro invocation - if the new
** logical line is a directive,
** this routine returns 0 to indicate there is no invocation, and "saves"
** the Tokens corresponding to the directive. The next time this routine is 
** called, the directive will be processed before
** the normal tokenization takes place.
** This mode insures that none of the logical lines of the
** (possible) invocation within the input buffer
** will be overwritten until the invocation (if any) is finished.
** The "B_invocation" mode is used when tokenizing the next logical line
** in a multi-line function-like macro invocation. This mode insures that
** none of the logical lines of the invocation within the input buffer
** will be overwritten until the invocation is finished. This mode also
** insures that any directives that are not allowed within the arguments
** of a function-like macro invocation are diagnosed.
*/
/* Implementation Note: When tokenizing long strings (L'...' and L"...")
** this routine assumes that there are no multiple character mappings to backslash,
** new-line, single quote, or double quote.
*/
{
#ifdef __STDC__
	typedef enum _state_ {
		/*	`typedef enum _directive' fits in the 0x001F bits */
#	ifdef	MERGED_CPP
		S_MultiLineComment = 0x0040,	/* multi-line comment Token */
#	endif
		S_CreateTokens	= 0x0020	/* create linked Token list */
		/*	`typedef enum mode_' fits in the 0x1F00 bits */
	} State;	
#else
	typedef int State;
#	ifdef	MERGED_CPP
#		define S_MultiLineComment 0x0040
#	endif
#	define S_CreateTokens	0x0020
#endif
	register char *lp;	/* current input character	*/
	register char * cp;	/* pointer to beginning of Token */
	register int code;	/* value for `code' field of Token  */
	register Token *tp;	/* current token	*/
	register State state;	/* state variable bit array	*/
	register char * expect;	/* expected return value (assuming bf_ptr didn't move) */
#define DIRFLAG()	(state & DIR_MASK)
#define MODE()		(state & MODE_MASK)
#define TST(x)		(state & (x)) /* "test" the state bit variable */
#define SET(x)		(state |= ((State) x))/* "set" the state bit variable */
#define CLR(x)		(state &= ~(x))/* "clear" the state bit variable */

	COMMENT(mode == B_text
		|| mode == B_tokens
		|| mode == B_invocation
		|| mode == B_macroname);
#ifdef DEBUG
	if (DEBUG('b') > 0)
		(void)fprintf(stderr, "bf_tokenize(%s)\n", SAYMODE());
#endif
	state = (State)mode;
	if (backup != 0)
	{
		COMMENT(mode & (B_tokens | B_text));
		COMMENT(backup == 1);
		backup = 0;
		directive(MODE(), savedirflag);
	}
again: 
	while (expect = bf_ptr +  bf_cur, (lp = nextline(MODE())) == 0)
		if (fl_isoriginal() || TST(B_invocation|B_macroname))
		{
			DBGRET(0,bf_tokenize,(Token *)0);
			return (Token *)0;
		}
		else
		{
			goteof = 0;
			fl_prev();
		}
	if (TST(B_text|B_tokens))
		firstline = lp;
	else 
	{
		ptrdiff_t diff;	/* difference of expected and actual char *'s */

		COMMENT(TST(B_invocation|B_macroname));
		COMMENT(listp != 0);
		COMMENT(! TK_ISINTERNAL(listp)); 
		if (expect != lp)
		{
			diff = lp - expect;
			adjust(listp, diff);
			firstline = &bf_ptr[bf_prev];
		}
	}
	COMMENT(firstline == &bf_ptr[bf_prev]);
	tp = &head;
	CLR(D_no|D_other|D_include|D_null);
	SET(D_maybe);
	if (TST(B_text))
		CLR(S_CreateTokens);
	else
		SET(S_CreateTokens);
#ifdef	MERGED_CPP
	CLR(S_MultiLineComment);
#endif
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
				goto twochar;
			}
#endif
			code = C_Colon;
			goto onechar;

		case '$':
			code = C_Dollar;
			goto onechar;
			
		case '?':
			code = C_Question;
			goto onechar;

		case '(':
			code = C_LParen;
			goto onechar;

		case ')':
			code = C_RParen;
			goto onechar;

		case ',':
			code = C_Comma;
			goto onechar;

		case '~':
			code = C_Complement;
			goto onechar;

		case '[':
			code = C_LBracket;
			goto onechar;

		case ']':
			code = C_RBracket;
			goto onechar;

		case '{':
			code = C_LBrace;
			goto onechar;

		case '}':
			code = C_RBrace;
			goto onechar;

		case ';':
			code = C_SemiColon;
			goto onechar;

		default:code = C_Operator;
onechar:		lp++;
			CHECKDIR();
			break;

		case '^':
			if (lp[1] == '=')
			{
				code = C_XORAssign;
twochar:			lp += 2;
				CHECKDIR();
				break;
			}
			code = C_ExclusiveOR;
			goto onechar;

		case '%':
			if (lp[1] == '=')
			{
				code = C_ModAssign;
				goto twochar;
			}
			code = C_Mod;
			goto onechar;

		case '*':
			if (lp[1] == '=')
			{
				code = C_MultAssign;
				goto twochar;
			}
			code = C_Mult;
			goto onechar;

		case '=':
			if (lp[1] == '=')
			{
				code = C_Equal;
				goto twochar;
			}
			code = C_Assign;
			goto onechar;

		case '!':
			if (lp[1] == '=')
			{
				code = C_NotEqual;
				goto twochar;
			}
			code = C_Not;
			goto onechar;

		case '#':
			if (lp[1] == '#')
			{
				code = C_Paste;
				goto twochar;
			}
			lp++;
			if (TST(D_no|D_other|D_include))
			/*EMPTY*/	;
			else if (TST(D_null))
			{
				COMMENT(TST(S_CreateTokens) != 0);
				SET(D_other);
			}
			else if (TST(D_maybe))
			{
				SET(S_CreateTokens);
				SET(D_null);
			}
			code = C_Sharp;
			break;

		case '+':
			switch (lp[1])
			{
			case '+':
				code = C_Increment;
				goto twochar;

			case '=':
				code = C_PlusAssign;
				goto twochar;

			default:code = C_Plus;
				goto onechar;
			}

		case '&':
			switch (lp[1])
			{
			case '&':
				code = C_LogicalAND;
				goto twochar;

			case '=':
				code = C_ANDAssign;
				goto twochar;

			default:code = C_BitwiseAND;
				goto onechar;
			}

		case '|':
			switch (lp[1])
			{
			case '|':
				code = C_LogicalOR;
				goto twochar;

			case '=':
				code = C_ORAssign;
				goto twochar;

			default:code = C_InclusiveOR;
				goto onechar;
			}

		case '.':
			if (tk_chtypes[((unsigned char *)lp)[1]] & CH_DECIMAL)
				goto float_const;
			switch (lp[1])
			{
#ifdef CPLUSPLUS
			case '*':
				code = C_DotStar;
				goto twochar;
#endif
			case '.':
				if (lp[2] == '.')
				{
					code = C_Ellipsis;
threechar:				lp += 3;
					CHECKDIR();
					break;
				}
			default:code = C_Dot;
				goto onechar;
			}
			break;

		case '>':
			switch (lp[1])
			{
			case '>':
				if (lp[2] == '=')
				{
					code = C_RightAssign;
					goto threechar;
				}
				code = C_RightShift;
				goto twochar;

			case '=':
				code = C_GreaterEqual;
				goto twochar;

			default:code = C_GreaterThan;
				goto onechar;
			}

		case '-':
			switch (lp[1])
			{
			case '-':
				code = C_Decrement;
				goto twochar;

			case '=':
				code = C_MinusAssign;
				goto twochar;

			case '>':
#ifdef CPLUSPLUS
				if (lp[2] == '*')
				{
					code = C_ArrowStar;
					goto threechar;
				}
#endif
				code = C_Arrow;
				goto twochar;

			default:code = C_Minus;
				goto onechar;
			}

		case '<':
			if (TST(D_include))
			{
				code = C_Header;
				while (*++lp != '>')
				{
					if (*lp == '\0')
					{
						WARN( "#include <... missing '>'" );
						*lp++ = '>';
						goto out;
					}
				}
				lp++;
				break;
			}
			switch (lp[1])
			{
			case '<':
				if (lp[2] == '=')
				{
					code = C_LeftAssign;
					goto threechar;
				}
				code = C_LeftShift;
				goto twochar;

			case '=':
				code = C_LessEqual;
				goto twochar;

			default:code = C_LessThan;
				goto onechar;
			}

		case '/':
			switch (lp[1])
			{
			case '=':
				code = C_DivAssign;
				goto twochar;

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
						ptrdiff_t diff;
						char *new;

#ifdef	MERGED_CPP
						SET(S_MultiLineComment);
#endif
						*lp = '\n';
						if ((new = nextline(B_comment)) == firstline)
							continue;
						if (new == 0)
						{
							WARN("EOF in comment");
							*lp = '\0';
							break;
						}
						diff = &bf_ptr[bf_prev] - firstline;
						if (TST(B_macroname|B_invocation))
							adjust(listp, diff);
						if (TST(S_CreateTokens) && (&head != tp))
							adjust(head.next, diff);
						lp += diff;
						cp += diff;
						firstline = new;
					}
				}
				if (!(pp_flags & F_KEEP_COMMENTS))
					if ((TST(S_CreateTokens) == 0) && (lp[0] == '\0'))
					{
						cp[0] = ' ';
						cp[1] = '\n';
						lp = cp + 2;
						goto out;
					}
					else
						SET(S_CreateTokens);
				break;

			case '/':
				if (pp_flags & F_TWO_COMMENTS)
				{
					code = C_WhiteSpace;
					lp = strchr(lp, '\0');
					if ((!(pp_flags & F_KEEP_COMMENTS))
					 && (TST(S_CreateTokens) == 0))
					{
						cp[0] = ' ';
						cp[1] = '\n';
						lp = cp + 2;
					}
					else
						*lp++ = '\n';
					goto out;
				}
			default:code = C_Div;
				goto onechar;
			}
			break;

		case '\0':
			code = C_WhiteSpace;
			*lp++ = '\n';
			goto out;
#ifdef DEBUG
		case '\n':
			pp_internal("<new-line> character in bf_tokenize");
			break;
#endif
		case ' ':
		case '\t':
		case '\f':
		case '\v':
			code = C_WhiteSpace;
			if (ISDIRECTIVE())
			{	
				int hadwarned;	/* boolean: already issued a warning ? */

				hadwarned = 0;
				do {
					if (*lp == ' ' || *lp == '\t' || hadwarned)
						continue;
					WARN("invalid white space character in directive");
					hadwarned = 1;
				} while (tk_chtypes[(unsigned char)*++lp] & CH_WHITE);
			}
			else
				while (tk_chtypes[(unsigned char)*++lp] & CH_WHITE)	;
			break;

		case 'L':
			if (cp[1] == '\'')
			{
				lp++;
				goto char_const;
			}
			else if (cp[1] == '\"')
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
			while (tk_chtypes[(unsigned char)*++lp] & CH_IDENT)	;
			if (TST(S_CreateTokens) == 0)
			{
#ifdef FILTER
				switch (lp - cp)
				{
					default:
					case 8:	if ((st_chars[cp[7]] & (0x1 << 7)) == 0)
							break;
						/*FALLTHRU*/
					case 7:	if ((st_chars[cp[6]] & (0x1 << 6)) == 0)
							break;
						/*FALLTHRU*/
					case 6:	if ((st_chars[cp[5]] & (0x1 << 5)) == 0)
							break;
						/*FALLTHRU*/
					case 5:	if ((st_chars[cp[4]] & (0x1 << 4)) == 0)
							break;
						/*FALLTHRU*/
					case 4:	if ((st_chars[cp[3]] & (0x1 << 3)) == 0)
							break;
						/*FALLTHRU*/
					case 3:	if ((st_chars[cp[2]] & (0x1 << 2)) == 0)
							break;
						/*FALLTHRU*/
					case 2:	if ((st_chars[cp[1]] & (0x1 << 1)) == 0)
							break;
						/*FALLTHRU*/
					case 1:	if ((st_chars[cp[0]] & (0x1 << 0)) == 0)
							break;
#endif
					macro.ptr.string = cp;
					macro.rlen = lp - cp;
					if (st_ismacro(&macro) != 0)
						SET(S_CreateTokens);
#ifdef FILTER
				}
#endif
			}
			if (TST(D_no|D_other|D_include))
			/*EMPTY*/	;
			else if (TST(D_null))
			{
				if (((lp - cp) == 7)
				 && (cp[0] == 'i')
				 && (cp[1] == 'n')
				 && (cp[2] == 'c')
				 && (cp[3] == 'l')
				 && (cp[4] == 'u')
				 && (cp[5] == 'd')
				 && (cp[6] == 'e'))
					SET(D_include);
				else
					SET(D_other);
			}
			else if (TST(D_maybe))
				SET(D_no);
			break;

		case '0':
			code = C_I_Constant;
			if (lp[1] == 'x' || lp[1] == 'X')
			{
				lp++;
				while (tk_chtypes[(unsigned char)*++lp] & CH_HEX)	;
				if (lp - cp <= 2)
				{
					code = C_BadInput;
					SET(S_CreateTokens);
				}
			}
			else
			{
octal:				while (tk_chtypes[(unsigned char)*++lp] & CH_OCTAL)	;
				switch (*lp)
				{
				case '8':
				case '9':
#ifdef TRANSITION
					if (pp_flags & F_Xt)
						goto octal;
#endif
					code = C_BadInput;
					SET(S_CreateTokens);
					goto float_digit;

				case '.':
					goto float_const;
				case 'e':
				case 'E':
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
float_digit:		while (tk_chtypes[(unsigned char)*++lp] & CH_DECIMAL)	;
			switch ( *lp )
			{
			case '.':
float_const:			while( tk_chtypes[(unsigned char)*++lp] & CH_DECIMAL )
					;
				code = C_F_Constant;
				if ( *lp != 'e' && *lp != 'E' )
					goto float_suffix;
				/*FALLTHRU*/
			case 'e':
			case 'E':
float_econst:			if (tk_chtypes[(unsigned char)*++lp] & CH_PLUS_MINUS)
					lp++;
				if (tk_chtypes[(unsigned char)*lp] & CH_DECIMAL)
				{
					while (tk_chtypes[(unsigned char)*++lp] & CH_DECIMAL)
						;
					code = C_F_Constant;
				}
				else
				{
					code = C_BadInput;
					SET(S_CreateTokens);
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
int_suffix:		if (tk_chtypes[(unsigned char)*lp] & CH_INT_SUFFIX)
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
ppnumber:		for( ; ; code = C_BadInput, SET(S_CreateTokens), lp++)
				if (tk_chtypes[(unsigned char)*lp] & CH_ALPHANUM_DOT)
					continue;
				else if (tk_chtypes[(unsigned char)*lp] & CH_PLUS_MINUS)
					if (lp[-1] == 'E' || lp[-1] == 'e')
						continue;
					else
						break;
				else
					break;
			CHECKDIR();
			break;

		case '"':
			code = C_String;
string_lit:		 while (*++lp != '"')
			{
				if (*lp == '\\')
				{
					if (TST(D_include))
						continue;
					if (*++lp != '\0')
						continue;
				}
				if (*lp == '\0')
				{
					if (TST(D_include))
					{
						WARN("newline in string literal");
						*lp++ = '"';
						CHECKDIR();
						goto out;
					}
					else
					{
						code = C_BadInput;
						SET(S_CreateTokens);
						lp = cp;
						break;
					}
				}
			}
			lp++;
			CHECKDIR();
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
					SET(S_CreateTokens);
					lp--;
					break;
				}
			}
			lp++;
			CHECKDIR();
			break;

		} /* bottom of: `switch (*(cp = lp))' */
		if (TST(S_CreateTokens))
		{
			tp->next = tk_new();
			tp = tp->next;
			tp->ptr.string = cp;
			tp->rlen = lp - cp;
			tp->code = (unsigned short) code;
			tp->number = bf_lineno;
			if ( (bf_newlines != 0) && (code != C_WhiteSpace) )
				nladd(tp, lp);
			tp->next = 0;
			if (code == C_Identifier)
				tp->aux.hid = 0;
#ifdef MERGED_CPP
			else if (code == C_WhiteSpace)
				if (TST(S_MultiLineComment))
					tp->aux.mlc = 1;
				else
					tp->aux.mlc = 0;
#endif
		}
		DBGTK();
		continue;

out:		COMMENT(code == C_Header && lp[-1] == '>' && fl_numwarns() >= 1
		 || code == C_String && lp[-1] == '\"' && TST(D_include) && fl_numwarns() >= 1
		 || code == C_WhiteSpace && lp[-1] == '\n');
		if (TST(S_CreateTokens))
		{
			tp->next = tk_new();
			tp = tp->next;
			tp->ptr.string = cp;
			tp->rlen = lp - cp;
			tp->code = (unsigned short) code;
			tp->number = bf_lineno;
			if ( (bf_newlines != 0) && (code != C_WhiteSpace) )
				nladd(tp, lp);
			tp->next = 0;
#ifdef MERGED_CPP
			if (code == C_WhiteSpace)
				if (TST(S_MultiLineComment))
					tp->aux.mlc = 1;
				else
					tp->aux.mlc = 0;
#endif
		}
		DBGTK();
		break;
	} /* bottom of: `for(;;)' */
	if (ISDIRECTIVE())
	{
		if (TST(B_macroname))
		{
			backup = 1;
			savedirflag = DIRFLAG();
			DBGRET(0,bf_tokenize,(Token *)0);
			return (Token *)0;
		}
		directive(MODE(), DIRFLAG());
	}
	else if (GR_TRUEPART())
	{
		register char *p = firstline;

		if (TST(S_CreateTokens))
		{
			if (TST(B_text) == 0)
			{
				DBGRETL(0,bf_tokenize,head.next);
				return (head.next);
			}
			fl_sayline();
			cp = head.next->ptr.string;
			if (cp - p)
				(void)fwrite(p, cp - p, 1, stdout);
			DBGRETL(0,bf_tokenize,head.next);
			return (head.next);
		}
		else
		{
			COMMENT(TST(B_text));
			fl_sayline();
			(void)fwrite(p, lp - p, 1, stdout);
#ifdef DEBUG
			if (DEBUG('b') > 0)
				fflush(stdout);
#endif
		}
	}
	else if (TST(S_CreateTokens))
		tk_rml(head.next);
	goto again;
}

static Dirnum
dircmp(inputp)
	Token * inputp;
/* Given a pointer to an identifer token,
** this routine returns an indication of which
** directive name the Token matches.
** If identifer is not a valid directive name,
** -1 is returned.
** This routine recognizes all valid directive
** names except include, which presumably has been
** recognized during the context-sensitive tokenization
** phase (e.g. the tokenization of "headers" in a #include).
*/
{
	register char * cp;	/* directory name */

	COMMENT(inputp != 0);
	COMMENT(inputp->code == C_Identifier);
	cp = inputp->ptr.string;
	switch (inputp->rlen)
	{
	case 2:	if (cp[0] == 'i' && cp[1] == 'f')
			return N_if;
		break;

	case 4:	if (cp[0] == 'e')
		{
			if (cp[1] == 'l')
			{
				if (cp[2] == 's' && cp[3] == 'e')
					return N_else;
				else if (cp[2] == 'i' && cp[3] == 'f')
					return N_elif;
			}
		}
		else if (cp[0] == 'l' && cp[1] == 'i' && cp[2] == 'n' && cp[3] == 'e')
			return N_line;
		else if (cp[0] == 'f' && cp[1] == 'i' && cp[2] == 'l' && cp[3] == 'e')
			return N_file;
		break;

	case 5:	if (cp[0] == 'i')
		{
			if (cp[1] == 'f' && cp[2] == 'd' && cp[3] == 'e' && cp[4] == 'f')
				return N_ifdef;
			else if (cp[1] == 'd' && cp[2] == 'e' && cp[3] == 'n' && cp[4] == 't')
				return N_ident;
		}
		else if (cp[0] == 'e')
		{
			if (cp[1] == 'n' && cp[2] == 'd' && cp[3] == 'i' && cp[4] == 'f')
				return N_endif;
			else if (cp[1] == 'r' && cp[2] == 'r' && cp[3] == 'o' && cp[4] == 'r')
				return N_error;
		}
		else if (cp[0] == 'u' && cp[1] == 'n' && cp[2] == 'd' && cp[3] == 'e' && cp[4] == 'f')
			return N_undef;
		break;

	case 6:	if (cp[0] == 'd' && cp[1] == 'e' && cp[2] == 'f' && cp[3] == 'i' && cp[4] == 'n' && cp[5] == 'e')
			return N_define;
		else if (cp[0] == 'i' && cp[1] == 'f' && cp[2] == 'n' && cp[3] == 'd' && cp[4] == 'e' && cp[5] == 'f')
			return N_ifndef;
		else if (cp[0] == 'a' && cp[1] == 's' && cp[2] == 's' && cp[3] == 'e' && cp[4] == 'r' && cp[5] == 't')
			return N_assert;
		else if (strncmp(cp, "pragma", 6) == 0)
			return N_pragma;
		break;

	case 8: if (strncmp(cp, "unassert", 8) == 0)
			return N_unassert;
		break;
	}
	return N_invalid;
}

static void
directive(mode, dirflag)
	Mode mode;
	Dir dirflag;
/* Given a Mode and a flag that categorizes the directive line,
** this routine handles directives.
** Called when the current line is known to be a directive
** (although it might be an invalid one), this routine identifies
** the directive and if it is valid, calls the proper routine 
** to handle it. Invalid directives are diagnosed.
** For "control-line" and implementation-defined
** directives, the handling routine is not called if the line is in the
** false part of an if-group.
*/
{
	register Token	* tp;	/* Token in a directive	*/
	register Dirnum dirnum; /* identifies directive name */
#ifdef __STDC__
	static Token * (* fparray[])() =
#else
	static Token * (* fparray[])() =
#endif
	{	gr_if,
		gr_ifdef,
		gr_ifndef,
		gr_elif,
		gr_else,
		gr_endif,
		fl_error,
		st_define,
		pd_assert,
		pd_unassert,
		dofile,
		doident,
		fl_lineinfo,
		dopragma,
		fl_include,
		fl_line,
		st_undef
	};

#ifdef LINT
	if (mode & B_invocation)
		WARN ( "directive within macro argument list might not be portable");
#endif
	tp = head.next;
	while (tp->code == C_WhiteSpace)
		tp = tp->next;
	COMMENT(tp->code == C_Sharp);
	do { tp = tp->next; } while (tp && tp->code == C_WhiteSpace);
	COMMENT(tp ? tp->code != 0 : 1);
	switch (tp ? tp->code : 0)
	{
	case C_BadInput:
	case C_I_Constant:
#ifdef MERGED_CPP
		if (!fl_dotisource())
#endif
			if ((pp_flags & F_Xc) && !fl_stdhdr())
				WARN( "interpreted as a #line directive" );
		dirnum = N_lineinfo;
		break;

	case C_Identifier:
		if (dirflag & D_include)
		{
			dirnum = N_include;
			break;
		}
		else if ((dirnum = dircmp(tp)) != N_invalid)
			break;
	/* FALLTHROUGH */
	default:
		if (GR_TRUEPART())
			UERROR( "invalid directive" );
		else
			WARN( "invalid directive" );
	/* FALLTHROUGH */
	case 0:	tk_rml(head.next);
		return;
	}
	COMMENT(IS_VALID_DIRECTIVE(dirnum));
#ifdef MERGED_CPP
	if (fl_dotisource())
	{
		COMMENT(mode != B_invocation && mode != B_macroname);
		if (COMPILER_CONTROL_LINE(dirnum) == 0)
		{
			UERROR("invalid compiler control line in \".i\" file");
			tk_rml(head.next);
			return;
		}
			
	}
	else
#endif
	{
		if ((NON_ANSI_DIRECTIVE(dirnum)) && (pp_flags & F_Xc) && !fl_stdhdr() )
			WARN("directive is an upward-compatible ANSI C extension");
		if ((mode == B_invocation) && DISALLOWED_IN_ARGLIST(dirnum))
		{
			if (GR_TRUEPART())
				UERROR(inarglistmsg);
			else
				WARN(inarglistmsg);
			tk_rml(head.next);
			return;
		}
		else if ((!IFGROUP_DIRECTIVE(dirnum)) && (GR_TRUEPART() == 0))
		{
			tk_rml(head.next);
			return;
		}
	}
	if (tp->code == C_Identifier)
		do { tp = tp->next; } while (tp && tp->code == C_WhiteSpace);
	else
	/*EMPTY*/ COMMENT(dirnum == N_lineinfo);
	if (tp == 0)
		tk_rml(head.next);
	else
		tk_rmto(head.next, tp);
	tp = (*fparray[dirnum])(tp, mode);
	if (tp != 0)
		tk_rml(tp);
	return;
}

static Token *
dofile(tp, mode)
	Token *tp;
	Mode mode;
/* Given a pointer to a list of Tokens in a #file directive
** and a mode to operate in,
** this routine checks the syntax of the directive and
** processes the directive.
*/
{
	COMMENT(mode & (B_text | B_tokens));
	if (tp == 0 || tp->code != C_String)
	{
		UERROR("string literal expected after #file");
		return tp;
	}
	else if (mode == B_text)
	{
		(void)fprintf(stdout, "#file\t%.*s\n",
				(int) tp->rlen, tp->ptr.string);
	}
	else
	{
		/*EMPTY*/ COMMENT(mode == B_tokens);
#ifdef MERGED_CPP
			(* pp_interface)(PP_VFILE, tp);
#endif
	}
	tk_extra(tk_rm(tp));
	return (Token *)0;

}

static Token *
doident(tp, mode)
	register Token *tp;
	Mode mode;
/* Given a pointer to a list of Tokens in a #ident directive
** and a mode to operate in,
** this routine checks the syntax of the directive and
** processes the directive. If the mode is B_text and the command
** line options allow it, #ident information will be printed on stdout.
*/
{
#ifdef DEBUG
	if (DEBUG('o') > 0)
		(void)fprintf(stderr, "doident(tp=%#lx)\n", tp);
#endif
	COMMENT(mode & (B_text | B_tokens));
	if ( (tp != (Token *) 0)
	&& ((tp = tk_rmws(ex_directive(tp))) != 0 && tp->code == C_String) )
	{
#ifdef MERGED_CPP
		if (mode == B_tokens) 
		{
			(* pp_interface)(PP_IDENT, tp);
			tk_extra(tk_rm(tp));
			tp = 0;
		}
		else
#endif
		if (mode == B_text) 
		{
			fl_sayline();
			(void)printf("#ident\t%.*s\n",
				(int) tp->rlen, tp->ptr.string);
			tk_extra(tk_rm(tp));
			tp = 0;
		}

	}
	else 
		UERROR( "string literal expected after #ident" );
	return tp;
}


static Token *
dopragma(tp, mode)
/* Given a pointer to a list of Tokens in a #pragma directive,
** this routine checks the syntax and processes the directive.
*/
	register Token *tp;
	Mode mode;
{
#ifdef DEBUG
	if (DEBUG('o') > 0)
		(void)fprintf(stderr, "dopragma(tp=%#lx)\n", tp);
#endif
	COMMENT(mode & (B_text | B_tokens | B_invocation));

	if (tp && (tp->code == C_Identifier) ) 
		if (tp->rlen == 5 && strncmp(tp->ptr.string, "ident", 5)==0)
		{
			if (mode & B_invocation)
				WARN(inarglistmsg);
			else
				tp = doident(tk_rmws(tk_rm(tp)), mode);
			return(tp);
		}

	if (mode == B_text)
	{
		register Token *tmptp;

		(void)fprintf(stdout, "#pragma\t");
		for (tmptp = tp; tmptp != 0; tmptp = tmptp->next)
			(void)fprintf(stdout, "%.*s",
				 (int)tmptp->rlen, tmptp->ptr.string);
	}
#ifdef MERGED_CPP
	else if (mode == B_tokens)
		(* pp_interface)(PP_PRAGMA, tp);
#endif
	return (tp);
}


static char *
nextline(mode)
	Mode	mode;
/* Given an int to describe the mode to operate in,
** this routine reads the physical source into the input buffer and
** performs phases 1 and 2 of the phases of translation ( 2.1.1.2),
** returning the next input line (with no trigraphs or  <backslash><nl>'s)
** This returned line is normally a logical line, except in the case of
** multi-line comments. The <new-line> at the end of the returned line
** is replaced by a NULL character. Any NULL characters in the source file
** are diagnosed and replaced by a ' ' character.
**
** The modes are a superset of the modes described in the bf_tokenize()
** function comment. "B_invocation" and "B_macroname" inhibit nextline()
** from clearing the input buffer of any previous logical
** lines that are part of the multi-line macro invocation,
** and causes only the last line to be returned.
** "B_text" and "B_tokens" allow all previous logical lines
** to be cleared from the buffer, and causes the current logical line
** to be returned. A new mode is "B_comment", which deals with multi-line comments.
** In this mode, previous lines are not cleared and the entire logical line
** (including previous lines of the multi-line comment) is returned.
*/
/*
** This function uses its own inline copy loops as there
** is no guarenteed overlapping memory copy function in
** the library.  Also, the copying is hopefully minimized
** as trigraphs and \<newline>s are not supposed to occur
** very often.
*/
{
	register char *cur; /* current character	*/
	register char *eod; /* end of data	*/
	register char *beg; /* first character that cannot be cleared */
	register char *end; /* end of buffer; also used as a temp	*/
	char  * last;	/* last line (used for macro invocations) */
	int at_eof = 0;


	/* for now - make enum for at_eof */
	COMMENT(bf_cur <= bf_eod);
	COMMENT(bf_eod <= buf_len);
	COMMENT(bf_cur <= buf_len);
	COMMENT(bf_prev <= bf_cur);
	if (goteof)
	{
		if (bf_newlines != 0)
		{
			bf_lineno += bf_newlines;
			bf_newlines = 0;
		}
		gr_check();
		return(0);
	}
	end = bf_ptr;
	eod = end + bf_eod;
	cur = end + bf_cur;
	if (cur == eod && *cur != BF_SENTINEL)
	{
		COMMENT(*cur == '\"' || *cur == '\'' || *cur == '\n');
		cur[0] = BF_SENTINEL;
	}
	beg = (B_macroname | B_invocation | B_comment) & mode ? end + bf_prev : cur;
	if ((B_macroname | B_invocation) & mode)	last = cur;
	end += buf_len;
	if (mode & B_comment) {
		bf_newlines++;
		*bf_nlptr = (long) (cur - beg);	/*save posiotion of commented '\n'*/
		nlincr();
	}
	else
	{
		bf_lineno++;
		if (bf_newlines != 0)
		{
			bf_lineno += bf_newlines;
			bf_newlines = 0;
		}
		bf_nlptr = bf_nlposition;
	}
	for (;;)
	{
		while ((bufchars[(unsigned char)*cur] & B_NEXTLINE) == 0)
			cur++;
		switch (*cur)
		{
		case '\0':
			if (cur == eod)
			{
				COMMENT(cur[0] == BF_SENTINEL);
				if (beg != bf_ptr
				 && end - eod < BUFSIZ
				 && fl_isoriginal()
				 && (mode & (B_macroname | B_invocation | B_comment)) == 0)
				{
#ifdef DEBUG
					if (DEBUG('n') > 2) 
					{
						(void)fprintf(stderr,
							"nextline(): copy %d bytes\n",
							eod - beg);
					}
#endif
					cur = bf_ptr;
					bf_cur -= beg - cur;	/* for EOF check */
					while (beg < eod)
						*cur++ = *beg++;
					eod = cur;
					eod[0] = BF_SENTINEL;
					beg = bf_ptr;
				}
fillup:				if (end - eod < BUFSIZ)
				{
					register ptrdiff_t diff;
	
					end = pp_realloc(bf_ptr, (unsigned int) (buf_len += BUFSIZ));
#ifdef DEBUG
					if (DEBUG('n') > 1) 
					{
						(void)fprintf(stderr,
							"nextline(): buffer grows to %ld\n",
							buf_len);
						fflush(stderr);
						fflush(stdout);
					}
#endif
					diff = end - bf_ptr;
					bf_ptr = end;
					cur += diff;
					eod += diff;
					beg += diff;
					end += buf_len;
					last += diff;
					eod[0] = BF_SENTINEL;
				}
				{
					register int n;
	
					if ((n = read((int)fileno(fl_curfile()), eod, BUFSIZ)) < 0)
						FATAL("read error: ", "");
					else if (n == 0)
					{
						if (cur == eod)
						{
							if (eod >= end)
								goto fillup;
							*cur = '\0';
							goto eof;
						}
						at_eof = 1;
					}
					eod += n;
					eod[0] = BF_SENTINEL;
				}
			}
			else
			{
				UERROR( "null character in input" );
				cur[0] = ' ';
				cur++;
			}
			continue;

		case '\n':
			if (++cur - beg < 2 || cur[-2] != '\\')
			{
				cur[-1] = '\0';
				goto out;	/* got a line */
			}
#ifdef DEBUG
			if (DEBUG('n') > 2)
			{
				(void)fputs("nextline(): got \\<newline>\n",
					stderr);
				fflush(stderr);
				fflush(stdout);
			}
#endif
			bf_newlines++;
			cur -= 2;
			*bf_nlptr= cur - beg;	/* save position of '\\n' */
			nlincr();		/* increament pointer */
			break;

		case '?':
			if (++cur + 1 >= eod)
			{
				if (at_eof)
					continue;
				cur--;
				goto fillup;
			}
			if (*cur != '?')
				continue;
			switch (cur[1])
			{
			default:
				continue;
			case '=':
				cur[-1] = '#';
				break;
			case '(':
				cur[-1] = '[';
				break;
			case '/':
				cur[-1] = '\\';
				break;
			case ')':
				cur[-1] = ']';
				break;
			case '\'':
				cur[-1] = '^';
				break;
			case '<':
				cur[-1] = '{';
				break;
			case '!':
				cur[-1] = '|';
				break;
			case '>':
				cur[-1] = '}';
				break;
			case '-':
				cur[-1] = '~';
				break;
			}
#ifdef TRANSITION
			if (pp_flags & F_Xt)
				WARN( "trigraph sequence replaced" );
#endif
#ifdef DEBUG
			if (DEBUG('n') > 2)
			{
				(void)fprintf(stderr,
					"nextline(): trigraph ??%c->%c\n",
					cur[1], cur[-1]);
				fflush(stderr);
				fflush(stdout);
			}
#endif
			break;
#ifdef DEBUG
		default:pp_internal( "bufchars[] is incorrectly initialized" );
#endif
		}
		{
			register char *sav_cur;

#ifdef DEBUG
			if (DEBUG('n') > 2)
			{
				(void)fprintf(stderr,
					"nextline(): copy %d bytes\n",
					eod - cur + 2);
				fflush(stderr);
				fflush(stdout);
			}
#endif
			sav_cur = cur;
			eod -= 2;
			for ( ; cur < eod; cur++)
				cur[0] = cur[2];
			eod[0] = BF_SENTINEL;
			cur = sav_cur;
		}
	}
	/* for now : COMMENT( "only here if at eof") */
eof:	gr_check();
	goteof = 1;
	if (&bf_ptr[bf_cur] < cur)
		WARN("newline not last character in file");
	else
		at_eof = -1;	/* mark real EOF */
out:;
	/*
	* Only here if at EOF or had a '\n' in cur[-1].
	*/
	end = bf_ptr;
	bf_cur = cur - end;
	bf_prev = beg - end;
	bf_eod = eod - end;
	if (at_eof < 0)
	{
#ifdef DEBUG
		if (DEBUG('n') > 1)
		{
			(void)fputs("nextline(): at EOF\n", stderr);
			fflush(stderr);
			fflush(stdout);
		}
#endif
		return (0);
	}
#ifdef DEBUG
	if (DEBUG('n') > 2)
	{

		(void)fprintf(stderr, "nextline(%s): returns `", saymode[mode >> 8]);
		pp_printmem(((B_macroname | B_invocation) & mode) ? last : beg,
		 cur - ((B_macroname | B_invocation) & mode ? last : beg));
		(void)fputs("'\n", stderr);
		fflush(stderr);
		fflush(stdout);
	}
	if (DEBUG('n') > 3)
	{
		(void)fprintf(stderr, "\tbf_ptr: %#lx\n", bf_ptr);
		(void)fprintf(stderr, "\tbeg: %#lx\tbf_prev: %d\n", beg, bf_prev);
		(void)fprintf(stderr, "\tcur: %#lx\tbf_cur:  %d\n", cur, bf_cur);
		(void)fprintf(stderr, "\teod: %#lx\tbf_eod:  %d\n", eod, bf_eod);
		(void)fprintf(stderr, "\tend: %#lx\tbuf_len:  %d\n\n", end, buf_len);
		fflush(stderr);
		fflush(stdout);
	}
#endif
	return ((B_macroname | B_invocation) & mode) ? last : beg;
}



static void
nladd(tp, lp)
Token *tp;
char* lp;
/*
** add the number of '\\n' and commented '\n' that occured from the beginning of
** the current logical line to the current token, to the line number field of tp.
*/
{
	register long *pos = bf_nlposition;
	register int nlcount;

	for( nlcount = 0; nlcount < bf_newlines; nlcount++ ) 
		if ( lp  >  (bf_ptr + (bf_prev + *pos++)) )
				tp->number++;
}


static void
nlincr()
/*
** increament bf_nlptr to the next available slot in bf_nlposition.
** reallocate bf_nlposition if necessary.
*/
{
	register long offset;

	if ( bf_newlines == bf_nlnumber ) {
		offset = bf_nlnumber - 1;
		bf_nlnumber += NLINCR;
		bf_nlposition = (long *)pp_realloc( (char*) bf_nlposition, 
				(unsigned int)	(bf_nlnumber * sizeof(long*)) );
		bf_nlptr = bf_nlposition + offset;
	}
	bf_nlptr++;
}
