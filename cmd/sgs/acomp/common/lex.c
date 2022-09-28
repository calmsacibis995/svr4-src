/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/lex.c	55.1"
/* lex.c */

/* This module is the interface between the built-in preprocessor
** and the rest of the compiler.
*/

#include "p1.h"
#include "interface.h"
#include "acgram.h"
#include <ctype.h>
#include <string.h>
#include <memory.h>

#ifndef	MERGED_CPP
#include "shouldn't compile this code"
#endif

int elineno = 0;			/* current line number */
int lx_getcur = 1;			/* 1 if next token is new sdb line */

int s1debug;				/* debug flag for source code */

#define	MARKERCNT	20		/* put out a marker this often */

#define	LXCH ptr.string			/* for Token char pointer */

static void doccon();
static void dostlit();
static void skiptonl();
static Token * gettoken();
static int peektoken();
static int insharp;			/* 1 if processing for # line, else 0 */


static void
out_source(tk)
Token * tk;
/* Output the C source line provided by the preprocessor.
** tk points to the list of tokens that constitutes the line.
*/
{
    if (elineno % MARKERCNT == 0)
	er_markline();
    /* Skip true leading whitespace if it isn't an empty line. */
    if (   tk->code == C_WhiteSpace
	&& tk->LXCH[0] != '\n'
	&& tk->LXCH[0] != '/'
    )
	tk = tk->next;
    /* Don't print empty lines. */
    if (!tk || tk->LXCH[0] == '\n') 
	return;
    CG_PRINTF(("%s ", COMMENTSTR));
    /* Have to be careful about comments with embedded \n. */
    for ( ; tk; tk = tk->next) {
	if (tk->code == C_WhiteSpace && tk->aux.mlc) {
	    char * cur = tk->LXCH;
	    char * bound = &tk->LXCH[tk->rlen];

	    for (;;) {
		char * end;
		if ((end = memchr(cur, '\n', bound-cur)) == NULL)
		    end = bound-1;
		++end;
		CG_PRINTF(("%.*s", end-cur, cur));
		cur = end;
		if (cur >= bound)
		    break;
		CG_PRINTF(("%s ", COMMENTSTR));
	    }
	} else
	    CG_PRINTF(("%.*s", (int) tk->rlen, tk->LXCH));

	/* Do special checking for juxtaposed identifier and number,
	** and print special character.
	*/
	if (s1debug > 1 && tk->next) {
	    int lcode = tk->code;
	    int rcode = tk->next->code;
	    if (   (lcode == C_Identifier || lcode == C_I_Constant || lcode == C_F_Constant)
		&& (rcode == C_Identifier || rcode == C_I_Constant || rcode == C_F_Constant)
		)
		CG_PUTCHAR('@');
	}
    }
    return;
}


/* Used by gettoken(), peektoken(), lx_ungetc(), lx_sh_yylex() (only!):
** remember next token.
*/
static Token * curtoken = 0;
/* Used to remember last token yylex() returned. */
static Token * lasttoken = 0;

static Token *
gettoken(wantws)
int wantws;
/* Return next token.  If wantws is non-zero, return any token.
** Otherwise, return first non-whitespace token.
*/
{
    register Token * tk = curtoken;

    for (;;) {
	if (tk == 0) {
	    /* Get new line of tokens from cpp. */
	    curtoken = 0;		/* interlock with lx_s_sharp() */
	    tk = PP_GETTOKENS();
	    if (tk == 0) {
		elineno = PP_CURLINE();	/* get current line number for errors */
		return( 0 );		/* at EOF */
	    }
	    elineno = tk->number;	/* set current line number */
	    if (s1debug)
		out_source(tk);
	}
	/* tk now non-null. */
	if (!wantws && tk->code == C_WhiteSpace) {
#ifdef LINT
	    if (tk->LXCH[0] == '/')	/* handle special comments */
		ln_setflags(tk->LXCH, tk->rlen);
#endif
	    tk = tk->next;
	}
	else {
	    /* Have a non-NULL token. */
	    if (lx_getcur) {
		lx_getcur = 0;
		db_curline = PP_CURLINE();
	    }
	    curtoken = tk->next;	/* Remember where we are */
	    return( tk );
	}
    }
    /*NOTREACHED*/
}


static void
skiptonl()
/* Skip tokens until a token with newline is reached. */
{
    Token * tk;

    while ((tk = gettoken(1)) != 0) {
	if (tk->LXCH[0] == '\n')
	    return;
    }
    return;				/* got to EOF */
}


static int
peektoken()
/* Return token code for next non-whitespace token.  At EOF,
** return C_Nothing.
*/
{
    register Token * tk;

    while ((tk = gettoken(0)) != 0 && tk->code == C_WhiteSpace)
	;
    curtoken = tk;			/* want to return this one next time */
    return( tk ? tk->code : C_Nothing );
}

int
yylex()
/* Analyze the next preprocessor token and pass it to the parser. */
{
    register Token * tk;
    char savechar;
    
    /* Loop to EOF */
    while ((tk = gettoken(0)) != 0) {	/* don't want white space */

	lasttoken = tk;			/* remember token for errors */

    /* BEWARE non-standard indentation here */

#define TKC(x) case TK_ENUMNO(x)

    switch( TK_ENUMNO(tk->code) ){
    TKC(C_Identifier):
    {
	int keyword;

	if (insharp)
	    return( L_IDENT );
	if ( (keyword = lx_keylook(tk->LXCH, tk->rlen)) == L_ASM) {
#ifdef	IN_LINE
	    /* Treat asm as storage class if not followed by (. */
	    if (peektoken() != C_LParen)
		keyword = L_class;	/* yylval.intval already set */
#endif
	}
	else if (keyword == L_ELSE)
	    lx_getcur = 1;
	return( keyword );
    }
			/* For these int, float constant, must temporarily
			** make NUL-terminated strings.
			*/
    TKC(C_I_Constant):
			if (!insharp) {
			    savechar = tk->LXCH[tk->rlen];
			    tk->LXCH[tk->rlen] = '\0';;
			    yylval.nodep = tr_int_const(tk->LXCH);
			    tk->LXCH[tk->rlen] = savechar;
			}
			return( L_INT_CONST );
    TKC(C_F_Constant):
			if (!insharp) {
			    savechar = tk->LXCH[tk->rlen];
			    tk->LXCH[tk->rlen] = '\0';;
			    yylval.nodep = tr_fcon(tk->LXCH, (int) tk->rlen);
			    tk->LXCH[tk->rlen] = savechar;
			}
			return( L_FLOAT_CONST );
character_constant: ;
    TKC(C_C_Constant):
			if (!insharp)
			    doccon(tk);
			return( L_CHAR_CONST );
string_literal: ;
    TKC(C_String):
    TKC(C_Wstring):
			if (!insharp)
			    dostlit(tk);
			return( L_STRING );
    TKC(C_LParen):				return( L_LP );
    TKC(C_RParen):				return( L_RP );
    TKC(C_Comma):				return( L_COMMA );
    TKC(C_Question):	yylval.intval = QUEST;	return( L_QUEST );
    TKC(C_Colon):				return( L_COLON );
    TKC(C_LogicalOR):	yylval.intval = OROR;	return( L_OROR );
    TKC(C_LogicalAND):	yylval.intval = ANDAND;	return( L_ANDAND );
    TKC(C_InclusiveOR):	yylval.intval = OR;	return( L_OR );
    TKC(C_ExclusiveOR):	yylval.intval = ER;	return( L_XOR );
    TKC(C_BitwiseAND):	yylval.intval = AND;	return( L_AND );
    TKC(C_Equal):	yylval.intval = EQ;	return( L_EQUALOP );	/* == */
    TKC(C_NotEqual):	yylval.intval = NE;	return( L_EQUALOP );
    TKC(C_GreaterThan):	yylval.intval = GT;	return( L_RELOP );
    TKC(C_GreaterEqual):yylval.intval = GE;	return( L_RELOP );
    TKC(C_LessThan):	yylval.intval = LT;	return( L_RELOP );
    TKC(C_LessEqual):	yylval.intval = LE;	return( L_RELOP );
    TKC(C_LeftShift):	yylval.intval = LS;	return( L_SHIFTOP );
    TKC(C_RightShift):	yylval.intval = RS;	return( L_SHIFTOP );
    TKC(C_UnaryPlus):
    TKC(C_Plus):	yylval.intval = PLUS;	return( L_PLUS );
    TKC(C_UnaryMinus):
    TKC(C_Minus):	yylval.intval = MINUS;	return( L_MINUS );
    TKC(C_Mult):	yylval.intval = MUL;	return( L_STAR );
    TKC(C_Div):		yylval.intval = DIV;	return( L_DIVOP );
    TKC(C_Mod):		yylval.intval = MOD;	return( L_DIVOP );
    TKC(C_Complement):	yylval.intval = COMPL;	return( L_UNARYOP );
    TKC(C_Not):		yylval.intval = NOT;	return( L_UNARYOP );
    TKC(C_LBracket):	yylval.intval = LB;	return( L_LB );
    TKC(C_RBracket):				return( L_RB );
    TKC(C_LBrace):	db_curline = PP_CURLINE(); /* for start of function */
			lx_getcur = 1;
			return( L_LC );
    TKC(C_RBrace):	lx_getcur = 1;		return( L_RC );
    TKC(C_Arrow):	yylval.intval = STREF;	return( L_STROP );
    TKC(C_Dot):		yylval.intval = DOT;	return( L_STROP );
    TKC(C_Assign):	yylval.intval = ASSIGN;	return( L_EQ );
    TKC(C_MultAssign):	yylval.intval = ASG MUL;return( L_ASGOP );
    TKC(C_DivAssign):	yylval.intval = ASG DIV;return( L_ASGOP );
    TKC(C_ModAssign):	yylval.intval = ASG MOD;return( L_ASGOP );
    TKC(C_PlusAssign):	yylval.intval = ASG PLUS;return( L_ASGOP );
    TKC(C_MinusAssign):	yylval.intval = ASG MINUS;return( L_ASGOP );
    TKC(C_LeftAssign):	yylval.intval = ASG LS;	return( L_ASGOP );
    TKC(C_RightAssign):	yylval.intval = ASG RS;	return( L_ASGOP );
    TKC(C_ANDAssign):	yylval.intval = ASG AND;return( L_ASGOP );
    TKC(C_XORAssign):	yylval.intval = ASG ER;	return( L_ASGOP );
    TKC(C_ORAssign):	yylval.intval = ASG OR;	return( L_ASGOP );
    TKC(C_SemiColon):	lx_getcur = 1;		return( L_SEMI );
    TKC(C_Ellipsis):				return( L_DOTDOTDOT );
    TKC(C_Increment):	yylval.intval = INCR;	return( L_INCOP );
    TKC(C_Decrement):	yylval.intval = DECR;	return( L_INCOP );
    default:
    {
	/* Be discriminating about what we say, depending on
	** the token.  Detect things that look like unterminated
	** character constants and string literals.
	** If in special #-line processing, only diagnose unterminated
	** string literal, character constant.
	*/
#define	OUTLEN 20			/* maximum amount to show in msg. */
	char firstchar = tk->LXCH[0];

	if (firstchar == 'L')
	    firstchar = tk->LXCH[1];
	if (firstchar == '"') {
	    UERROR("newline in string literal");
	    skiptonl();
	    if (!insharp)
		goto string_literal;
	}
	else if (firstchar == '\'') {
	    UERROR("newline in character constant");
	    skiptonl();
	    if (!insharp)
		goto character_constant;
	}
	else if (tk->rlen == 1) {
	    if (!insharp) {
		if (isprint(firstchar))
		    UERROR("invalid source character: '%c'", firstchar);
		else
		    UERROR("invalid source character: <%#x>", firstchar);
	    }
	}
	else {
	    /*FALLTHRU*/
    TKC(C_Invalid):
	/* All invalid tokens (resulting from bad pasting) come here.
	** Restrict printing of token to OUTLEN characters.
	*/
	    UERROR("invalid token: %.*s%s",
			tk->rlen > OUTLEN ? OUTLEN : tk->rlen, tk->LXCH,
			tk->rlen > OUTLEN ? "..." : "");
	}
	/* Return illegal token if in #-line processing. */
	if (insharp)
	    return( L_ILL_TOKEN );
    } /* end default case */
    } /* end switch */
    } /* end while */
    lasttoken = 0;
    return( 0 );			/* end of file */
}



static void
doccon(tk)
Token * tk;
/* Process a character constant, look for escapes.
** If 'L', make a wide character constant.
*/
{
    int iswide = (tk->LXCH[0] == 'L');	/* flag whether wide char */
    char * s;
    unsigned int inlen = tk->rlen - 2;	/* exclude both ' s */
    char * inptr = tk->LXCH + 1;	/* skip first ' */
    unsigned int len;

    /* If invalid token, add back 1 for missing trailing '.
    ** Avoids potential "empty character constant" message.
    */
    if (tk->code != C_C_Constant)
	++inlen;

    if (iswide) {
	++inptr;			/* skip L */
	--inlen;			/* don't count L */
    }
    len = doescape(&s, inptr, inlen, 0, iswide);
    if (len == 0)
	UERROR("empty character constant");
    yylval.nodep = tr_ccon(s, len, iswide);
    return;
}

static void
dostlit(tk)
Token * tk;
/* Process a string literal token tk.  Look for escapes.
** If 'L', make a wide string literal.  Also,
** concatenate consecutive string literals into one.
*/
{
    int oldiswide = (tk->LXCH[0] == 'L'); /* note wide first literal */
    char * s;
    int iswide = oldiswide;		/* state of "current" literal */
    unsigned int outlen = 0;
    int warned = 0;
    static const char mesg[] = 
	"cannot concatenate wide and regular string literals";	/*ERROR*/

    for (;;) {
	unsigned int inlen = tk->rlen - 2; /* don't count " s */
	char * inptr = tk->LXCH + 1;	/* skip leading " */
	int token;

	if (iswide) {
	    ++inptr;			/* skip L */
	    --inlen;			/* don't count L */
	}
	outlen = doescape(&s, inptr, inlen, outlen, oldiswide);

	if ((token = peektoken()) != C_String && token != C_Wstring)
	    break;

	/* Do concatenation. */
	tk = gettoken(0);		/* don't want whitespace */

	iswide = (tk->LXCH[0] == 'L'); /* note whether wide literal */

	/* Figure out if having a wide string makes sense.
	** We can handle (with a warning) a non-wide string
	** after a wide string, but not the reverse.
	*/
	if (! warned && (oldiswide ^ iswide) != 0) {
	    if (oldiswide)
		WERROR(mesg);
	    else
		UERROR(mesg);
	    warned = 1;
	}
    }
    yylval.nodep = tr_string(s, outlen+1, oldiswide);
    return;
}

/* Externally visible routines. */

static Token * lx_curtoken;		/* for asm support */
static unsigned int lx_curcount;

void
lx_s_getc()
/* Prepare to get characters. */
{
    lx_curtoken = 0;
    return;
}


void
lx_e_getc()
/* Done fetching characters. */
{
    if (lx_curtoken == 0 || lx_curcount != lx_curtoken->rlen)
	cerror("lx_e_getc() out of sync");
    return;
}


int
lx_getc()
/* Fetch next lex character. */
{
    if (lx_curtoken == 0 || lx_curcount >= lx_curtoken->rlen) {
	lx_curtoken = gettoken(1);	/* get whitespace, too */
	lx_curcount = 0;
    }
    if (lx_curtoken == 0)
	return( EOF );
    else
	return(lx_curtoken->LXCH[lx_curcount++]);
}


void
lx_ungetc(c)
char c;
/* Push back a lex character.  Must be the same as the last
** character returned.
*/
{
    if (   lx_curtoken == 0
	|| lx_curcount-- == 0
	|| lx_curtoken->LXCH[lx_curcount] != c
    )
	cerror("lx_ungetc() out of sync");
    /* If at start of token, make token look new. */
    if (lx_curcount == 0) {
	curtoken = lx_curtoken;		/* push back this token */
	lx_curtoken = 0;
    }
    return;
}


void
lx_errtoken(ps,plen)
char ** ps;
unsigned int * plen;
/* Return pointer, length of last token's spelling. */
{
#define EOF_STRING "<EOF>"
    if (lasttoken) {
	*ps = &lasttoken->LXCH[0];
	*plen = lasttoken->rlen;
    }
    else {
	*ps = EOF_STRING;
	*plen = sizeof(EOF_STRING) - 1;
    }
    return;
}

/* Support for recursive calls from # line processing. */

static Token * sh_curtoken;

void
lx_s_sharp(tk)
Token *tk;
/* Begin # line processing.  We presume this occurs at the
** beginning of a logical line.  Therefore "curtoken" should
** be null.  tk is a list of tokens for a # line.
*/
{
    if (curtoken)
	cerror("non-null curtoken in lx_s_sharp()");
    
    sh_curtoken = tk;
    insharp = 1;
    return;
}

void
lx_e_sharp()
/* Disable # line processing. */
{
    insharp = 0;
    sh_curtoken = 0;
    curtoken = 0;
    return;
}


int
lx_sh_yylex(ps, pi)
char ** ps;
int * pi;
/* Get next token for # line processing.  ps points to a place
** to store the pointer to the string, pi points to a place to
** store the string length.
**
** Cheat a lot:  scan whitespace here, avoid trying to read
** another logical line.  Rely on yylex to classify the token.
*/
{
    Token *tk;

    for (tk = sh_curtoken; tk != (Token *) 0; tk = tk->next) {
	if (tk->code != C_WhiteSpace)
	    break;
    }
    if (tk) {
	*ps = tk->LXCH;
	*pi = tk->rlen;
	elineno = tk->number;		/* set current line number */
	/* This is a kludge:  use yylex to classify token. */
	curtoken = tk;
	sh_curtoken = tk->next;
	return( yylex() );
    }
    else
	elineno = PP_CURLINE();		/* set line number anyway for errors */
    sh_curtoken = 0;
    return( 0 );			/* no more tokens */
}
