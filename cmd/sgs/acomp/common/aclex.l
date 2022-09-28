%{
/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
%}
%{
/*	Copyright (c) 1988 AT&T	*/
%}
%{
/*	  All Rights Reserved  	*/
%}

%{
/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
%}
%{
/*	UNIX System Laboratories, Inc.                     	*/
%}
%{
/*	The copyright notice above does not evidence any   	*/
%}
%{
/*	actual or intended publication of such source code.	*/
%}

%{
#ident	"@(#)acomp:common/aclex.l	55.1"
/* aclex.l */

/* Lex-based scanner for ANSI C compiler.  The scanning is two-
** level.  The lowest level identifies the characters of a token
** and assigns (usually) a group code.  The upper level may further
** refine the token code, and it may have to call the lower level
** more than once for a particular token.
**
*/

#include "p1.h"
#include "acgram.h"
#include <ctype.h>
#include <string.h>
#include <memory.h>

/* HACK:  extend yytext[]. */
#undef	YYLMAX
#define	YYLMAX	1024

/* These are functions that appear later. */

#undef	input
#undef	unput

/* Set this to 1 when doing #-line processing. */
static int insharp;

int elineno = 0;			/* current line number */
int lx_getcur = 1;
int s1debug;

#define	myEOF	0			/* for lex interaction */

static int input();
static void unput();
static int skip();
static int doccon();
static int dostlit();
static int lx_yylex();
static void discard();
static int peektoken();

static void
skipcomment()
/* Skip characters after initial '/' '*' and up to next '*' '/'. */
{
    int temp;
    int startlineno = elineno;		/* remember for error */
    int oldinsharp = insharp;

    insharp = 0;			/* can read more lines if comment */

#ifdef LINT
    /*
    ** this routine will eventually go away -
    ** read a comment - a lint directive only has UPPER-case of digits
    ** and less than 20 characters long.  If any of these is not true
    ** while reading the comment, let the main routine handle the rest
    */
    while (((temp=input())==' ')||(temp=='\t')||(temp=='\n')) ;

    if (isupper(temp)) {
        char direct[20];
        int i=0;
        direct[i++] = temp;
        while ((isupper(temp=input()) || isdigit(temp)) && (i <= 19))
            direct[i++] = temp;
        if (i != 20) {
            direct[i] = '\0';
            ln_setflags(direct);
        }
    }
    unput(temp);
#endif
    do {
	while ((temp = input()) != '*') {
	    if (temp == myEOF) {
		elineno = startlineno;	/* restore for error */
		WERROR("EOF in comment");
		goto out;
	    }
	}
	while (temp == '*')
	    temp = input();
    } while (temp != '/');
out:;
    insharp = oldinsharp;
    return;
}


static int
doccon()
/* Process a character constant.  The leading ' (with possible
** leading L) has been recognized.  Scan across the constant,
** then go back and look for escapes.  If 'L', make a wide
** character constant.  Return the correct token code
** (L_CHAR_CONST or L_ILL_TOKEN).
*/
{
    int iswide = (yytext[0] == 'L');	/* flag whether wide char */
    char * s;
    int retval;

    retval = skip('\'');		/* eat to matching ' */
    if (!insharp) {
	unsigned int len =
	    doescape(&s, yytext+1+iswide, (unsigned int) yyleng-2-iswide,
			0, iswide);
	if (len == 0)
	    UERROR("empty character constant");
	yylval.nodep = tr_ccon(s, len, iswide);
	retval = L_CHAR_CONST;
    }
    return( retval );
}

static int
dostlit()
/* Process a string literal.  The leading " (with possible leading L)
** has been recognized.  Scan across the string literal, then go back
** and look for escapes.  If 'L', make a wide string literal.  Also,
** concatenate consecutive string literals into one.  Return correct
** token type, L_STRING or L_ILL_TOKEN.
*/
{
    int oldiswide = (yytext[0] == 'L');	/* note wide literal */
    int iswide = oldiswide;
    char * s;
    unsigned int outlen = 0;
    int warned = 0;
    int retval;
    static const char mesg[] = 
	"cannot concatenate wide and regular string literals";	/*ERROR*/

    for (;;) {

	retval = skip('"');
	if (insharp)
	    return( retval );

	outlen = doescape(&s, yytext+1+iswide, (unsigned int) yyleng-2-iswide,
					outlen, oldiswide);

	/* Look ahead for another string that follows immediately.
	** If one is found, concatenate.  Don't concatenate strings
	** on # line.
	*/
	if (peektoken() != L_STRING)
	    break;

	iswide = yytext[0] == 'L';

	discard();			/* forget remembered token */

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
    return( L_STRING );
}


static int
skip(skipchar)
char skipchar;
/* Eat characters until one is found that matches skipchar.
** Store them in yytext[].  Store length of whole thing in yyleng.
*/
{
    int startlineno = elineno;		/* for error message */
    int i;
    int retval = (skipchar == '"' ? L_STRING : L_CHAR_CONST);
    char * s = &yytext[yyleng];		/* append to stuff there */
					/* store characters here */

    /* Just scan over everything, including escape sequences. */
    while ((i = input()) != skipchar) {
	if (i == '\\') {
	    if ((i = input()) == '\n')
		continue;		/* splice, discard \ and \n */
	    *s++ = '\\';		/* keep the \ */
	}
	switch( i ) {			/* catches stuff in/outside of escape */
	case myEOF:
	    elineno = startlineno;		/* restore for message */
	    /*FALLTHRU*/
	case '\n':
	    UERROR("%s in %s",
		i == myEOF ? "EOF" : "newline",
		skipchar == '"' ? "string literal" : "character constant");
	    if (s[-1] == '\\')		/* avoid \ NUL */
		--s;
	    if (i == '\n')
		unput(i);		/* push back to recognize start of
					** next line
					*/
	    retval = L_ILL_TOKEN;
	    goto done;
	}
	*s++ = (char) i;
    }
done:;
    s[0] = skipchar;
    s[1] = '\0';
    yyleng = s-yytext+1;		/* length including quotes */
    return( retval );
}


/* These are buffering routines that support printing each line of C
** code as it's seen.
*/
#define	LX_LINELEN 1000

static unsigned char charbuf[LX_LINELEN]; /* current line buffer */
static unsigned char * curp = charbuf;	/* pointer to current buffer position */
#define	MARKERCNT	20		/* put out a marker this often */
/* Note that *curp is initially 0. */

static int
input()
{
    static int sawEOF = 0;

    if (sawEOF) return( 0 );
    if (yytext[YYLMAX-1] != 0)
	cerror("token too long");
    if (*curp) return( *curp++ );
    if (insharp)
	return( 0 );			/* only get one line in # */
    /* At end of line.  Get non-empty line. */
    if (elineno++, fgets((char *) charbuf, LX_LINELEN, stdin)) {
	/* Skip initial tabs, spaces, print that way. */
	curp = charbuf;
	if (s1debug) {
	    unsigned char * nonwhite =
		charbuf + strspn((char *) charbuf, " \t");
	    if (elineno % MARKERCNT == 0)
		er_markline();

	    if (*nonwhite != '\n')	/* avoid null lines */
		CG_PRINTF(("%s %s", COMMENTSTR, nonwhite));
	}
	return( *curp++ );
    }
    sawEOF = 1;
    return( 0 );
}

static void
unput(c)
int c;
{
    *--curp = (unsigned char) c;
    return;
}


/* Externally visible routines. */

void
lx_s_getc()
/* Prepare to get characters. */
{
    return;
}


void
lx_e_getc()
/* Done fetching characters. */
{
    return;
}


int
lx_getc()
/* Fetch next lex character. */
{
    int i = input();

    if (i != myEOF)
	return( i );
    return( EOF );
}


void
lx_ungetc(c)
char c;
/* Push back a lex character. */
{
    unput(c);
    return;
}

void
lx_errtoken(ps, plen)
char ** ps;
unsigned int *plen;
/* Return pointer, length of last token. */
{
    if (yyleng == 0 || yytext[0] == 0) {
	*ps = "<EOF>";
	*plen = sizeof("<EOF>") - 1;
    }
    else {
	*ps = yytext;
	*plen = yyleng;
    }
    return;
}


/* Remembered stuff for token look-ahead.
** curtoken is current token code, if non-zero; intval is current
** yylval.intval for it.
*/
static int curcode;
static int intval;

static int
peektoken()
{
    if (curcode == 0) {
	curcode = lx_yylex();
	intval = yylval.intval;
    }
    return( curcode );
}

static void
discard()
{
    curcode = 0;
    return;
}


int
yylex()
{

    int retcode;

    for (;;){				/* usually just one trip */
	extern void sh_proc();

	retcode = curcode;
	curcode = 0;
	if (retcode == 0)
	    retcode = lx_yylex();
	else
	    yylval.intval = intval;
	if (lx_getcur) {
	    db_curline = elineno;
	    lx_getcur = 0;
	}
	
	switch( retcode ){
	case L_INT_CONST:
	    if (!insharp)
		yylval.nodep = tr_int_const(yytext);
	    break;
	case L_FLOAT_CONST:
	    if (!insharp)
		yylval.nodep = tr_fcon(yytext, yyleng);
	    break;
	case L_CHAR_CONST:
	    retcode = doccon();
	    break;
	case L_STRING:
	    retcode = dostlit();
	    break;
	case L_IDENT:
	    /* asm or __asm, followed by (, is old-style asm.
	    ** Don't do keyword lookup if in # line.
	    */
	    if (insharp)
		return( retcode );
	    if ((retcode = lx_keylook(yytext, (unsigned int) yyleng)) == L_ASM) {
#ifdef	IN_LINE
		int temp = yylval.intval;
		if (peektoken() != L_LP) {
		    /* Treat asm as storage class. */
		    yylval.intval = temp;
		    retcode = L_class;
		}
#endif
	    }
	    else if (retcode == L_ELSE)
	        lx_getcur = 1;
	    return( retcode );
	case L_ISHARP:
	    insharp = 1;
	    sh_proc();
	    while (lx_yylex())
		;			/* eat remaining tokens */
	    insharp = 0;
	    continue;
	case L_ILL_TOKEN:
	    if (insharp)
		break;			/* return invalid tokens quietly */
	    if (isprint(yytext[0]))
		UERROR("invalid source character: '%c'", yytext[0]);
	    else
		UERROR("invalid source character: <%#x>", yytext[0]);
	    continue;
	case L_LC:
	    db_curline = elineno; /* for beginning of function */
	    /* FALLTHRU */
	case L_RC:
	case L_SEMI:
	    /* Don't automatically get new line after ')' or ':'; This
	     * is handled in acgram.y so that we only get a new line
	     * after seeing a condition or label, respectively.
	     */
	    lx_getcur = 1;
	    break;
	default:
	    break;
	}
	break;
    }
    return( retcode );
}


/* Turn lex's yylex into static lx_yylex. */
#define	yylex static lx_yylex
%}

WS	[ \t\f\n\013]
XD	[0-9a-fA-F]
D	[0-9]
L	[_a-zA-Z]
EXP	((e|E)(\+|-)?{D}+)
ISUF	([lL]|[uU]|([lL][uU])|([uU][lL]))
%%

{WS}+		{ ; }
^{WS}*#		{ return( L_ISHARP ); }
"/*"		{ skipcomment(); }

{L}({L}|{D})*	{ return( L_IDENT ); }
({D}+{ISUF}?)|(0[xX]{XD}+{ISUF}?) {
		/* Distinguish decimal from octal in tr_int_const(). */
		return( L_INT_CONST );
		}
({D}*\.{D}+|{D}+\.){EXP}?[fFlL]? 	{ return( L_FLOAT_CONST ); }
{D}+{EXP}[fFlL]?			{ return( L_FLOAT_CONST ); }

L?'		{ return( L_CHAR_CONST ); }
L?\"		{ return( L_STRING ); }

";"		{ return( L_SEMI ); }
"("		{ return( L_LP ); }
")"		{ return( L_RP ); }
"{"		{ return( L_LC ); }
"}"		{ return( L_RC ); }
"["		{ yylval.intval = LB; return( L_LB ); }
"]"		{ return( L_RB ); }
","		{ return( L_COMMA ); }
"+"		{ yylval.intval = PLUS; return( L_PLUS ); }
"&"		{ yylval.intval = AND; return( L_AND ); }
"&&"		{ yylval.intval = ANDAND; return( L_ANDAND ); }
":"		{ return( L_COLON ); }
"..."		{ return( L_DOTDOTDOT ); }
"="		{ yylval.intval = ASSIGN; return( L_EQ ); }
("!"|"=")=	{ yylval.intval = yytext[0] == '!' ? NE : EQ;
			return( L_EQUALOP ); }
("++"|"--")	{ yylval.intval = yytext[0] == '+' ? INCR : DECR;
			return( L_INCOP ); }
"-"		{ yylval.intval = MINUS; return( L_MINUS ); }
"|"		{ yylval.intval = OR; return( L_OR ); }
"||"		{ yylval.intval = OROR; return( L_OROR ); }
"?"		{ yylval.intval = QUEST; return( L_QUEST ); }
"*"		{ yylval.intval = MUL; return( L_STAR ); }
("->"|".")	{ yylval.intval = yytext[0] == '.' ? DOT : STREF;
			return( L_STROP ); }
"^"		{ yylval.intval = ER; return( L_XOR ); }
"~"|"!"		{ yylval.intval = yytext[0] == '~' ? COMPL : NOT;
			return( L_UNARYOP); }
"<="		{ yylval.intval = LE; return( L_RELOP ); }
"<"		{ yylval.intval = LT; return( L_RELOP ); }
">="		{ yylval.intval = GE; return( L_RELOP ); }
">"		{ yylval.intval = GT; return( L_RELOP ); }
"<<"|">>"	{ yylval.intval = yytext[0] == '<' ? LS : RS;
			return( L_SHIFTOP ); }
(([+\-*/%&|^])|"<<"|">>")= { 
			switch( yytext[0] ) {
			case '+': yylval.intval = ASG PLUS; break;
			case '-': yylval.intval = ASG MINUS; break;
			case '*': yylval.intval = ASG MUL; break;
			case '/': yylval.intval = ASG DIV; break;
			case '%': yylval.intval = ASG MOD; break;
			case '&': yylval.intval = ASG AND; break;
			case '|': yylval.intval = ASG OR; break;
			case '^': yylval.intval = ASG ER; break;
			case '<': yylval.intval = ASG LS; break;
			case '>': yylval.intval = ASG RS; break;
			default: cerror("confused asgop");
			}
			return( L_ASGOP );
		}
"%"|"/"		{ yylval.intval = yytext[0] == '%' ? MOD : DIV;
			return( L_DIVOP); }
.		{ return( L_ILL_TOKEN ); }
