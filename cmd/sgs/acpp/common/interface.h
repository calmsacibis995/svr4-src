/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acpp:common/interface.h	1.14"
#ifndef _INTERFACE_H
#define _INTERFACE_H

/* interface for acpp as a merged process */

/* the Token codes consist of two bytes:
** a lower order byte, C_ENUMNO,  which
** uniquely numbers the tokens from 0 by steps of 1;
** an upper order byte, C_CLASS,
** that marks the Token as belonging to a number
** of token categories (or classes).
*/
#define C_ENUMNO	0x00FF	/* mask to get unique enumeration number */
#define C_CLASS		0xFF00	/* mask to get a Token class	*/
#define C_CONSTRAINED	0x0100	/* is an identifier or integer constant */
#define	C_INTERNAL	0x0200	/* internal use, not to be output */
#define	C_CONDEXPR	0x0400	/* used in expanded #[el]if expression */
#define C_FORMAL	0x0800	/* is a function-like macro formal */
#define C_OP		0x1000  /* lexically, an operator or punctuator	*/
#define C_INSUBST	0x2000	/* of special interest during macro substitution */
#define C_INEXPAND	0x4000	/* of special interest when searching for macro expansion */
#define TK_ENUMNO(code)		((code) & C_ENUMNO)
#define TK_ISCONSTRAINED(tp)	((tp)->code & C_CONSTRAINED)
#define TK_ISINTERNAL(tp)	((tp)->code & C_INTERNAL)
#define TK_CONDEXPR(tp)		((tp)->code & C_CONDEXPR)
#define TK_ISFORMAL(tp)		((tp)->code & C_FORMAL)
#define TK_ISOP(tp)		((tp)->code & C_OP)
#define TK_INSUBST(tp)		((tp)->code & C_INSUBST)
#define TK_INEXPAND(tp)		((tp)->code & C_INEXPAND)

#define NX(x)	(((x) & C_ENUMNO) + 1)	/* next enum */

/* used in Token.use to indicate "what" and "where" the Token is being used */
#define	U_REPLIST	0x01	/* a Token in a macro replacement list */
#define U_FROMBUF	0x02	/* Token from array in buf.c */
#define	TK_ISREPL(tp)	((tp)->use & U_REPLIST)
#define TK_ISBUF(tp)	((tp)->use & U_FROMBUF)
#define	TK_SETREPL(tp)	((tp)->use |= U_REPLIST)
#define TK_SETBUF(tp)	((tp)->use |= U_FROMBUF)
#define	TK_CLRREPL(tp)	((tp)->use &= ~U_REPLIST)
#define TK_CLRBUF(tp)	((tp)->use &= ~U_FROMBUF)
#define TK_CLRUSE(tp)	((tp)->use = 0)

/* used in Token.aux.ws */
#define W_LEFT	0x1	/* C_WhiteSpace preceeds C_Paste */
#define W_RIGHT	0x2	/* C_WhiteSpace follows C_Paste	*/

/* used to indicate status when returning from pp_*() */
#define PP_SUCCESS	0	/* successful function execution */
#define PP_FAILURE	1	/* error occured during function execution */
#define PP_IGNORED	-1	/* no action taken */

/* used as the  first argument in calls to the
** function pointed to by second argument of pp_init()
*/
#define PP_IDENT	0	/* call pertaining to `#ident' directive */
#define PP_VFILE	1	/* call pertaining to `#__FILE__' directive */
#define PP_LINE		2	/* call pertaining to `#line' directive,
				** or `# <number>' information
				*/
#define PP_PRAGMA	3	/* call pertaining to `#pragma' directive */
typedef struct _token_ Token;	/* defined below */

typedef struct _macro_	Macro;	/* defined in syms.h */

typedef union _ptr_ {
	 char * string;
	 Macro * macro;	/* for C_Marker only	*/
} Ptr;

typedef union _aux_ {
	short	argno;	/* C_Formal, C_StrFormal, C_UnexpFormal: Argument number
			** C_MacroName, C_Manifest: number of arguments.
			** C_Identifier: in formals list, last formal with same name
			*/
	char	ws;	/* C_Paste: records white space before
			** or after `##' in
			** a macro definition replacement list.
			*/
	char	hid;	/* C_Identifer: boolean: in replacement list.
			** Is this Token hidden from macro expansion?
			*/
	char	type;	/* C_Operand: records if bits in Token.number
			** have a signed or unsigned interpretation.
			*/
#ifdef MERGED_CPP
	char	mlc;	/* C_WhiteSpace: boolean: is this a
			** multi-line comment ?
			*/
#endif
	short	init;	/* used to initialize(clear) Token.aux field,
			** regardless of Token code.
			*/
} Aux;	/* 2 general purpose bytes used in Token.aux.
	** The use of these auxilliary bytes depends on Token.code.
	*/

struct _token_	/* single token (in a list)'s information */
{
	Token	*next;		/* next in linked list			*/
	Ptr	ptr;		/* pointer to string or macro		*/
	unsigned short	rlen;	/* length of what ptr.string points to	*/
	unsigned short	use;	/* holds U_* values			*/
	unsigned short	code;	/* internal code for token		*/
	Aux	aux;		/* see `typedef ... Aux' above		*/
	long	number;		/* line number or value in expression	*/
};


/* possible values for Token.code (see codenames[] in acpp:common/token.c
** for a description of these codes)
*/
/* Implementation Note: normally, I would want to
** declare these identifiers as enumeration constants.
** However, sfsup's cc can't handle  self-referencing enum definitions and
** CI 2.1 can't handle assignment of an enumeration constant to int.
*/
# define C_Nothing	(							0x0	)
# define C_WhiteSpace	( 							NX(0x00))
# define C_Operator	( 				C_OP  |			NX(0x01))
# define C_Identifier	( C_CONSTRAINED	| C_CONDEXPR |		C_INEXPAND |	NX(0x02))
# define C_I_Constant	( C_CONSTRAINED	| C_CONDEXPR |				NX(0x03))
# define C_F_Constant	( C_CONSTRAINED	|					NX(0x04))
# define C_C_Constant	( C_CONSTRAINED	| C_CONDEXPR |				NX(0x05))
# define C_String	( 							NX(0x06))
# define C_Wstring	( 							NX(0x07))
# define C_Header	( 							NX(0x08))
# define C_Dollar	( 		 		C_OP  |			NX(0x09))
# define C_RParen	( 		  C_CONDEXPR |	C_OP  |			NX(0x0A))
# define C_Comma	( 		 		C_OP  |			NX(0x0B))
# define C_Question	( 		  C_CONDEXPR |	C_OP  |			NX(0x0C))
# define C_Colon	( 		  C_CONDEXPR |	C_OP  |			NX(0x0D))
# define C_LogicalOR	( 		  C_CONDEXPR |	C_OP  |			NX(0x0E))
# define C_LogicalAND	( 		  C_CONDEXPR |	C_OP  |			NX(0x0F))
# define C_InclusiveOR	( 		  C_CONDEXPR |	C_OP  |			NX(0x10))
# define C_ExclusiveOR	( 		  C_CONDEXPR |	C_OP  |			NX(0x11))
# define C_BitwiseAND	( 		  C_CONDEXPR |	C_OP  |			NX(0x12))
# define C_Equal	( 		  C_CONDEXPR |	C_OP  |			NX(0x13))
# define C_NotEqual	( 		  C_CONDEXPR |	C_OP  |			NX(0x14))
# define C_GreaterThan	( 		  C_CONDEXPR |	C_OP  |			NX(0x15))
# define C_GreaterEqual	( 		  C_CONDEXPR |	C_OP  |			NX(0x16))
# define C_LessThan	( 		  C_CONDEXPR |	C_OP  |			NX(0x17))
# define C_LessEqual	( 		  C_CONDEXPR |	C_OP  |			NX(0x18))
# define C_LeftShift	( 		  C_CONDEXPR |	C_OP  |			NX(0x19))
# define C_RightShift	( 		  C_CONDEXPR |	C_OP  |			NX(0x1A))
# define C_Plus		( 		  C_CONDEXPR |	C_OP  |			NX(0x1B))
# define C_Minus	( 		  C_CONDEXPR |	C_OP  |			NX(0x1C))
# define C_Mult		( 		  C_CONDEXPR |	C_OP  |			NX(0x1D))
# define C_Div		( 		  C_CONDEXPR |	C_OP  |			NX(0x1E))
# define C_Mod		( 		  C_CONDEXPR |	C_OP  |			NX(0x1F))
# define C_UnaryPlus	( C_INTERNAL	| C_CONDEXPR |				NX(0x20))
# define C_UnaryMinus	( C_INTERNAL	| C_CONDEXPR |				NX(0x21))
# define C_Complement	( 		  C_CONDEXPR |	C_OP  |			NX(0x22))
# define C_Not		( 		  C_CONDEXPR |	C_OP  |			NX(0x23))
# define C_LParen	( 		  C_CONDEXPR |	C_OP  |			NX(0x24))
# define C_Operand	( C_INTERNAL	| C_CONDEXPR |				NX(0x25))
# define C_Sharp 	( 				C_OP  |	C_INEXPAND |	NX(0x26))
# define C_LBracket	(				C_OP  |			NX(0x27))
# define C_RBracket	(				C_OP  | 		NX(0x28))
# define C_LBrace	(				C_OP  | 		NX(0x29))
# define C_RBrace	(				C_OP  | 		NX(0x2A))
# define C_Arrow	(				C_OP  | 		NX(0x2B))
# define C_Assign	(				C_OP  | 		NX(0x2C))
# define C_MultAssign	(				C_OP  | 		NX(0x2D))
# define C_DivAssign	(				C_OP  | 		NX(0x2E))
# define C_ModAssign	(				C_OP  | 		NX(0x2F))
# define C_PlusAssign	(				C_OP  | 		NX(0x30))
# define C_MinusAssign	(				C_OP  | 		NX(0x31))
# define C_LeftAssign	(				C_OP  | 		NX(0x32))
# define C_RightAssign	(				C_OP  | 		NX(0x33))
# define C_ANDAssign	(				C_OP  | 		NX(0x34))
# define C_XORAssign	(				C_OP  | 		NX(0x35))
# define C_ORAssign	(				C_OP  | 		NX(0x36))
# define C_SemiColon	(				C_OP  | 		NX(0x37))
# define C_Ellipsis	(				C_OP  | 		NX(0x38))
# define C_Dot		(				C_OP  | 		NX(0x39))
# define C_Increment	(				C_OP  | 		NX(0x3A))
# define C_Decrement	(				C_OP  | 		NX(0x3B))
#ifdef CPLUSPLUS

# define C_ArrowStar	(				C_OP  |			NX(0x3C))
# define C_DotStar	(				C_OP  |			NX(0x3D))
# define C_Scope	( 				C_OP  |			NX(0x3E))
# define C_Paste	( 				C_OP  |	C_INSUBST |	NX(0x3F))
# define C_Formal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x40))
# define C_StrFormal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x41))
# define C_MergeFormal	( C_INTERNAL	| C_FORMAL   |				NX(0x42))
# define C_UnexpFormal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x43))
# define C_BadInput	( 							NX(0x44))
# define C_Invalid	( 							NX(0x45))
# define C_Goofy	( C_INTERNAL	|					NX(0x46))
# define C_MacroName	( C_INTERNAL	|					NX(0x47))
# define C_Manifest	( C_INTERNAL	|					NX(0x48))
# define C_Marker	( C_INTERNAL	|			C_INEXPAND |	NX(0x49))

#else

# define C_Paste	( 				C_OP  |	C_INSUBST |	NX(0x3C))
# define C_Formal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x3D))
# define C_StrFormal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x3E))
# define C_MergeFormal	( C_INTERNAL	| C_FORMAL   |				NX(0x3F))
# define C_UnexpFormal	( C_INTERNAL	| C_FORMAL   |		C_INSUBST |	NX(0x40))
# define C_BadInput	( 							NX(0x41))
# define C_Invalid	( 							NX(0x42))
# define C_Goofy	( C_INTERNAL	|					NX(0x43))
# define C_MacroName	( C_INTERNAL	|					NX(0x44))
# define C_Manifest	( C_INTERNAL	|					NX(0x45))
# define C_Marker	( C_INTERNAL	|			C_INEXPAND |	NX(0x46))

#endif
#define PP_CURLINE()	bf_curline()

#define PP_CURNAME()	fl_curname()
#define PP_NUMERRORS()	fl_numerrors()
#define PP_NUMWARNS()	fl_numwarns()

#define PP_GETTOKENS()	lx_gettokens()
#define PP_INPUT()	lx_input()
#define PP_TOKEN()	lx_token()

#define PP_CLEANUP()	pp_cleanup()
#define PP_INIT(a1,a2)	pp_init((a1), (a2))
#define PP_OPT(a1,a2)	pp_opt((a1), (a2))
#define PP_OPTSTRING()	pp_optstring()

#ifdef __STDC__
extern	unsigned int	bf_curline( void  );

extern	char *	fl_curname(	void	);
extern	int	fl_numerrors(	void	);
extern	int	fl_numwarns(	void	);

extern	Token * lx_gettokens(	void	);
extern	Token * lx_input(	void	);
extern	Token * lx_token(	void	);

extern	int	pp_cleanup(	void 	);
extern	int	pp_init(  char *, void (*)(int, Token *) );
extern	int	pp_opt(  int, char *  );
extern	char *	pp_optstring(	void	);

#else

extern	unsigned int bf_curline(/* void */ );

extern	char *	fl_curname( /* void	*/ );
extern	int	fl_numerrors(	/* void	*/);
extern	int	fl_numwarns(	/* void	*/);

extern	Token * lx_gettokens(/* void	*/ );
extern	Token * lx_input(/* void	*/ );
extern	Token * lx_token(/* void	*/ );

extern	int	pp_cleanup( /* void */ );
extern	int	pp_init( /* char *, void (*)(int, Token *) */ );
extern	int	pp_opt( /* int, char * */ );
extern	char *	pp_optstring( /* void */ );

#endif

#endif
