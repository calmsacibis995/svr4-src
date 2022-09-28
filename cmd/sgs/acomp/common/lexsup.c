/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/lexsup.c	1.16"
/* lexsup.c */

/* This module contains functions that are common to lexical
** analysis in the compiler when it is merged with the preprocessor
** or separate.
*/


#include "p1.h"
#include "acgram.h"
#include <ctype.h>
#include <memory.h>
#include <string.h>
#ifdef	__STDC__
#include <locale.h>
#endif

#ifndef	C_ALERT
#define	C_ALERT	07			/* ASCII code for BEL */
#endif

static UCONVAL dochar();
static UCONVAL doesc();
static void lx_wctomb();

extern myVOID *malloc();
extern myVOID *realloc();

/* These two tables are used to discriminate keywords from identifiers.
** Empirically, keywords constitute about 1/3 of all identifier-like
** tokens, so quick identification is worthwhile.
** The algorithm is a perfect hash algorithm that uses the "charval" array
** to give each character a value.  The charval-values of the first and
** last characters of a prospective identifier plus its length form an
** index into an array of string pointers.  If the "identifier" could
** be a keyword, the only possible keyword it could be is the one whose
** pointer was found.  Otherwise it must truly be an identifier.
**
** The algorithm is presented in "Communications of the ACM," January 1980.
** The title of the article is "Minimal Perfect Hashing Functions Made Simple."
*/

static const unsigned char charval[256] = {
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0 - 0x7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x8 - 0xf */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x10 - 0x17 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x18 - 0x1f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x20 - 0x27 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x28 - 0x2f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x30 - 0x37 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x38 - 0x3f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x40 - 0x47 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x48 - 0x4f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x50 - 0x57 */
	0, 0, 0, 0, 0, 0, 0, 21,	/* 0x58 - 0x5f */
	0, 4, 21, 10, 1, 1, 15, 12,	/* 0x60 - 0x67 */
	21, 21, 0, 13, 18, 2, 10, 21,	/* 0x68 - 0x6f */
	0, 0, 17, 5, 3, 21, 11, 4,	/* 0x70 - 0x77 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x78 - 0x7f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x80 - 0x87 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x88 - 0x8f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x90 - 0x97 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0x98 - 0x9f */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xa0 - 0xa7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xa8 - 0xaf */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xb0 - 0xb7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xb8 - 0xbf */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xc0 - 0xc7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xc8 - 0xcf */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xd0 - 0xd7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xd8 - 0xdf */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xe0 - 0xe7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xe8 - 0xef */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xf0 - 0xf7 */
	0, 0, 0, 0, 0, 0, 0, 0,	/* 0xf8 - 0xff */
};

#define	NOKEY		{ 0, 0, 0, 0 }
#define	KEY(s,c,v)	{ s, sizeof(s)-1, c, v }
#define	MAXHASHTAB 43

struct hashtab { const char *key; int len; int lexcode; int lexval; };
static const struct hashtab hashtab[MAXHASHTAB] = {
	NOKEY,					/* 0 */
	NOKEY,					/* 1 */
	NOKEY,					/* 2 */
	NOKEY,					/* 3 */
	NOKEY,					/* 4 */
	NOKEY,					/* 5 */
	KEY("else", L_ELSE, 0),			/* 6 */
	KEY("enum", L_ENUM, KT_ENUM),		/* 7 */
	KEY("double", L_type, KT_DOUBLE),	/* 8 */
	KEY("asm", L_ASM, SC_ASM),		/* 9 */
	KEY("while", L_WHILE, 0),		/* 10 */
	KEY("default", L_DEFAULT, 0),		/* 11 */
	KEY("signed", L_type, KT_SIGNED),	/* 12 */
	KEY("short", L_type, KT_SHORT),		/* 13 */
	KEY("struct", L_SORU, KT_STRUCT),	/* 14 */
	KEY("case", L_CASE, 0),			/* 15 */
	KEY("void", L_type, KT_VOID),		/* 16 */
	KEY("extern", L_class, SC_EXTERN),	/* 17 */
	KEY("const", L_type, KT_CONST),		/* 18 */
	KEY("continue", L_CONTINUE, 0),		/* 19 */
	KEY("volatile", L_type, KT_VOLATILE),	/* 20 */
	KEY("static", L_class, SC_STATIC),	/* 21 */
#if 0	/* noalias removed from ANSI standard */
	KEY("noalias", L_type, KT_NOALIAS),	/* 22 */
#else
	NOKEY,					/* 22 */
#endif
	KEY("float", L_type, KT_FLOAT),		/* 23 */
	KEY("do", L_DO, 0),			/* 24 */
	KEY("typedef", L_class, SC_TYPEDEF),	/* 25 */
	KEY("sizeof", L_SIZEOF, 0),		/* 26 */
	KEY("int", L_type, KT_INT),		/* 27 */
	KEY("__asm", L_ASM, SC_ASM),		/* 28 */
	KEY("auto", L_class, SC_AUTO),		/* 29 */
	KEY("unsigned", L_type, KT_UNSIGNED),	/* 30 */
	KEY("char", L_type, KT_CHAR),		/* 31 */
	KEY("switch", L_SWITCH, 0),		/* 32 */
	KEY("return", L_RETURN, 0),		/* 33 */
	KEY("long", L_type, KT_LONG),		/* 34 */
	KEY("for", L_FOR, 0),			/* 35 */
	KEY("union", L_SORU, KT_UNION),		/* 36 */
	KEY("goto", L_GOTO, 0),			/* 37 */
	KEY("if", L_IF, 0),			/* 38 */
	KEY("break", L_BREAK, 0),		/* 39 */
	NOKEY,					/* 40 */
	NOKEY,					/* 41 */
	KEY("register", L_class, SC_REGISTER),	/* 42 */
};

int
lx_keylook(s, len)
char * s;
unsigned int len;
/* Look up string s of length len to see if it's a keyword.
** If not a keyword, do the stuff necessary for an identifier
** and return L_IDENT.  For keyword, return its token code and
** set yylval.intval to the appropriate value from the table.
*/
{
    /* Distinguish keyword from identifier, do appropriate
    ** processing.
    */
    register const struct hashtab * htp;
    register int tindex =
	    charval[(unsigned char) (s[0])] + 
	    charval[(unsigned char)(s[len-1])] + 
	    len;
    char *id;
    SX symid;
    char savechar;

    if (   tindex < MAXHASHTAB
	&& (htp = &hashtab[tindex])->len == len
	&& strncmp(htp->key, s, len) == 0
    ) {
	/* Found a keyword. */
	yylval.intval = htp->lexval; /* parse stack value */
	if (htp->lexcode == L_ASM) {
	    /* Special magic check for asm's */
	    if (version & V_STD_C) {
		static int warned = 0;
		if (s[0] == 'a')
		    goto isident;
		/* Just do one of these warnings per compilation. */
		if (!warned) {
		    WERROR("__asm is an extension of ANSI C");
		    warned = 1;
		}
	    }
	}
	return( htp->lexcode );
    }
isident:;
    /* Have an identifier, not a keyword. */

    /* Must preserve trailing character for merged preprocessor. */
    savechar = s[len];
    s[len] = '\0';
    id = st_nlookup(s, len+1);
    s[len] = savechar;

    /* Check for typedef name, so we can return the correct token
    ** type.  This depends, of course, on whether the symbol has
    ** already been entered as a typedef, and whether the context
    ** is appropriate for such an identifier.
    ** Only look symbol up -- don't create new entry.
    */
    symid = sy_lookup(id, SY_NORMAL, SY_LOOKUP);
    if (   symid != SY_NOSYM
	&& SY_CLASS(symid) == SC_TYPEDEF
	&& dcl_oktype()
    ) {
	yylval.intval = symid;
	return( L_TYPENAME );
    }
    /* Make a normal identifier token. */
    yylval.charp = id;
    return( L_IDENT );
}

unsigned int
doescape(ps, in, len, outpos, iswide)
char ** ps;
char * in;
unsigned int len;
unsigned int outpos;
int iswide;
/* This routine is essentially two routines in one, but they both do
** the same thing:  scan a string for escape sequences, replace the
** escape sequences with their semantic equivalent.
** in points to the incoming string.
** ps points to the place to store the pointer to the result string.
** len is the number of characters available, not including a mandatory
** trailing NUL character.
** outpos is position to start writing into in the buffer.
** iswide determines whether we're dealing with wide characters (1) or not.
**
** We keep a single dynamic buffer here, which is reinitialized whenever
** outpos is zero.
**
** We guarantee there's a NUL (of the appropriate size) at the end of the
** result array, but it's not included in outpos when we return.
*/
{
#ifndef	STRHUNK
#define	STRHUNK 200
#endif

#define	WCSIZE (sizeof(wchar_t))

    static char instrbuf[STRHUNK];	/* initial buffer */
    static char * strbufp = instrbuf;
    static int strbufl = STRHUNK;	/* upper bound */
    char * s = in;
    char * bound = &in[len];
    wchar_t c;
    int locale_dflt = 0;		/* set to 1 if ever switch to default */

    /* Normalize outpos. */
    if (iswide)
	outpos *= WCSIZE;

    while (s < bound) {
	int clen;			/* length to add to output string */
	unsigned int newlen;

	if (*s == '\\') {
	    UCONVAL escchar;
	    ++s;
	    escchar = doesc(&s);
	    if (iswide) {
		clen = WCSIZE;
		if (escchar > (UCONVAL) T_UWCHAR_MAX)
		    WERROR("character escape does not fit in wide character");
	    }
	    else {
		clen = 1;
		if (escchar > T_UCHAR_MAX)
		    WERROR("character escape does not fit in character");
	    }
	    c = escchar;
	}
	else if (iswide) {
	    int wclen;

	    /* Have to do conversions in default locale. */
	    if (!locale_dflt) {
		(void) setlocale(LC_CTYPE, "");
		locale_dflt = 1;
	    }
	    wclen = mbtowc(&c, s, bound-s);

	    if (wclen <= 0) {
		UERROR("invalid multibyte character");
		break;
	    }
	    s += wclen;
	    clen = WCSIZE;
	}
	else {
	    char * nextbs = memchr(s, '\\', bound-s); /* next \ */

	    if (nextbs == 0)
		nextbs = bound;
	    clen = nextbs - s;
	    if (clen == 1)
		c = *s++;			/* allow for special case:
						** must look like non-wide
						** escape sequence
						*/
	}

	/* Store new character.  Allow enough room for chars and NUL. */
	if ((newlen = outpos + clen+WCSIZE) >= strbufl) {
	    /* Round up new length to multiple of STRHUNK. */
	    newlen = ((newlen+STRHUNK-1)/STRHUNK) * STRHUNK;

	    if (strbufp == instrbuf) {		/* first time */
		myVOID *tbuf = malloc(newlen);
		if (tbuf == 0)
		    cerror("out of space in doescape()");
		strbufp = memcpy(tbuf, strbufp, strbufl);
	    }
	    else if ((strbufp = realloc(strbufp, newlen)) == 0)
		cerror("out of space in doescape()");
	    strbufl = newlen;
	}
	if (iswide) {
	    lx_wctomb(c, &strbufp[outpos]);
	    outpos += WCSIZE;
	}
	else if (clen == 1) {			/* single byte */
	    strbufp[outpos] = (char) c;
	    ++outpos;
	}
	else {					/* multibyte normal string */
	    (void) memcpy(&strbufp[outpos], s, clen);
	    s += clen;
	    outpos += clen;
	}
    }
    if (iswide)
	lx_wctomb((wchar_t) 0, &strbufp[outpos]);
    else
	strbufp[outpos] = 0;
    *ps = strbufp;			/* pass back start of buffer */

    /* Restore C locale if we change to the default. */
    if (locale_dflt)
	(void) setlocale(LC_CTYPE, "C");
    return( iswide ? outpos/WCSIZE : outpos );
}



static UCONVAL
doesc(ps)
char ** ps;
/* Return the character corresponding to an escape sequence.
** We've seen the \ already.  ps points to the character
** pointer into the buffer.
*/
{
    UCONVAL c;

    switch( c = (unsigned char) *(*ps)++ ){
    case 'a':
	/* UNIX C took \a as 'a'.  Treat as "alert" in all variants. */
	if (version & V_CI4_1)
	    WERROR("\\a is ANSI C \"alert\" character");
	c = C_ALERT;
	break;
    case 'b':	c = '\b'; break;
    case 'f':	c = '\f'; break;
    case 'n':	c = '\n'; break;
    case 'r':	c = '\r'; break;
    case 't':	c = '\t'; break;
    case 'v':	c = '\v'; break;
    case '\'':
    case '"':
    case '?':
    case '\\':
		break;		/* these are themselves */
    case '0':	case '1':	case '2':
    case '3':	case '4':	case '5':
    case '6':	case '7':
	c = dochar(ps, (int) c - '0'); break;
    case 'x':
	c = dochar(ps, -1); break;
    default:
    if (isprint(c))
	WERROR("dubious escape: \\%c", c);
    else
	WERROR("dubious escape: \\<%#x>", c);
    }
    return( c );
}


static UCONVAL
dochar(ps, n)
char ** ps;
int n;
/* Calculate character constant value.  If 0 <= n < 8, the
** constant is octal, we have 2 more octal digits.  Otherwise
** the constant is hex, we have 1 or more digits.
** Warn -Xt users about \x becoming a new escape.
*/
{
    int base = 8;
    int nchars = 0;			/* characters seen */
    int toobig = 0;			/* flag whether constant overflows */
    UCONVAL retval = n;

    if (n < 0) {
	base = 16;
	retval = 0;
    }

    for ( ; base == 16 || nchars < 2; ++nchars ) {
	int c = **ps;
	int bias;			/* 0 if not a digit */

	++*ps;
	switch( c ) {
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
	    bias = 10 - 'a'; break;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
	    bias = 10 - 'A'; break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    bias = -'0'; break;
	default:
	    bias = 0; break;
	}
	/* If bias non-0, check whether result value > base.  If
	** zero bias or too big, push back character and exit.
	** Also check for overflow accumulating hex constants.
	*/
	if (bias && (bias += c) < base) {
	    /* Check hex (only) for overflow. */
	    if (base == 16 && (retval & ( ~((UCONVAL)(~0L)>>4) )) != 0)
		toobig = 1;
	    retval = (retval * base) + bias;	/* new char value */
	}
	else {				/* Not valid character.  Back up. */
	    --*ps;
	    break;
	}
    }
    if (base == 16) {
	if (version & V_CI4_1)
	    WERROR("\\x is ANSI C hex escape");
	else if (nchars == 0)		/* got no hex digits */
	    WERROR("no hex digits follow \\x");
	if (nchars == 0)		/* got no hex digits */
	    retval = 'x';		/* treat as character x */
	if (toobig)
	    WERROR("overflow in hex escape");
    }
    return( retval );
}


wchar_t
lx_mbtowc(s)
register char * s;
/* Perform the inverse operation of lx_wctomb().
** Recreate the wide character from a byte stream.
*/
{
    union { wchar_t wchar; char wca[sizeof(wchar_t)]; } wcu;

    switch(sizeof(wchar_t)){
    default:
	cerror("unknown wchar_t size");
	/*NOTREACHED*/
    case 4:	wcu.wca[3] = *s++;	/*FALLTHRU*/
    case 3:	wcu.wca[2] = *s++;	/*FALLTHRU*/
    case 2:	wcu.wca[1] = *s++;	/*FALLTHRU*/
    case 1:	wcu.wca[0] = *s++;
    }
    return( wcu.wchar );
}


static void
lx_wctomb(wc, s)
wchar_t wc;
register char * s;
/* Convert wide character wc to a stream of bytes of
** size sizeof(wchar_t) and write them to s.
*/
{
    union { wchar_t wchar; char wca[sizeof(wchar_t)]; } wcu;

    wcu.wchar = wc;
    switch(sizeof(wchar_t)){
    default:
	cerror("unknown wchar_t size");
	/*NOTREACHED*/
    case 4:	*s++ = wcu.wca[3];	/*FALLTHRU*/
    case 3:	*s++ = wcu.wca[2];	/*FALLTHRU*/
    case 2:	*s++ = wcu.wca[1];	/*FALLTHRU*/
    case 1:	*s   = wcu.wca[0];
    }
    return;
}

