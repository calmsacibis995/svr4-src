/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	@(#)make:gram.y	1.8 of 3/31/88	*/

%{
#ident	"@(#)make:gram.y	1.13"

#include "defs"
#define NLEFTS	200
#define INMAX	5000
%}

%term NAME SHELLINE START COLON DOUBLECOLON EQUAL A_STRING VERSION
%term BUILDEC

%union
{
	SHBLOCK	 yshblock;
	DEPBLOCK ydepblock;
	NAMEBLOCK ynameblock;
	CHARSTAR ycharstring;
	BLDBLOCK ybldblock;
}

%type	<yshblock>	SHELLINE, shlist, shellist
%type	<ynameblock>	NAME, namelist, name
%type	<ydepblock>	deplist, dlist
%type	<ycharstring>	A_STRING
%type	<ybldblock>	BUILDEC, bld, bldlist

%%

%{
extern	FILE 	*opdescf();
extern	NAMEBLOCK mainname;	/* from main.c */
void	setvar();
DEPBLOCK 	pp;
NAMEBLOCK	leftp;
LINEBLOCK 	lp, lpp;
FSTATIC SHBLOCK prevshp;
FSTATIC BLDBLOCK prevblp;
FSTATIC NAMEBLOCK lefts[NLEFTS];
FSTATIC DEPBLOCK prevdep;
FSTATIC int	nlefts,
		sepc;
%}


file:
	| file comline
	;

comline:  START
	| START macrodef
	| START namelist deplist bldlist shellist
	{
		if ( !(mainname || IS_ON(INTRULE)) &&
		     ((lefts[0]->namep[0] != DOT) ||
		      ANY(lefts[0]->namep, SLASH)))
			mainname = lefts[0];
	        while( --nlefts >= 0) {
			leftp = lefts[nlefts];
			if ( !(leftp->septype) )
				leftp->septype = sepc;
			else if (leftp->septype != sepc)
				fprintf(stderr, "inconsistent rules lines for `%s' (bu36)\n", leftp->namep);
			else if (sepc==ALLDEPS &&
				 *(leftp->namep)!=DOT &&
				 $4) {
				for (lp = leftp->linep;
				     lp->nextline;
				     lp = lp->nextline)
					if (lp->shp)
						fprintf(stderr, "multiple make lines for %s (bu37)\n", leftp->namep);
					else if (lp->bldecn)
						fprintf(stderr, "multiple build criteria for %s (bu38)\n",leftp->namep);
			}
	
			lp = ALLOC(lineblock);
			lp->nextline = NULL;
			lp->depp = $3;
			lp->bldecn = $4;
			lp->shp = $5;

			if (STREQ(leftp->namep, ".SUFFIXES") && $3==0)
				leftp->linep = 0;
			else if ( !(leftp->linep) )
				leftp->linep = lp;
			else {
				for (lpp = leftp->linep;
				     lpp->nextline;
				     lpp = lpp->nextline)
					;
				if (sepc==ALLDEPS && 
						leftp->namep[0]==DOT &&
		      				!ANY(leftp->namep, SLASH))
					lpp->shp = 0;
				lpp->nextline = lp;
			}
		}
	}
	| error
	;

macrodef: NAME EQUAL A_STRING
	{
		setvar((CHARSTAR) $1, $3);
	}
	;

name:	NAME
	{
		if( !($$ = SRCHNAME((CHARSTAR) $1)) )
			$$ = makename((CHARSTAR) $1);
	}
	| NAME VERSION
	{
		if( !($$ = SRCHNAME((CHARSTAR) $1)) )
			$$ = makename((CHARSTAR) $1);
	}
	;

namelist: name
	{
		lefts[0] = $1;
		nlefts = 1;
	}
	| namelist name
	{
		lefts[nlefts++] = $2;
	    	if (nlefts > NLEFTS)
			fatal("Too many lefts");
	}
	;

dlist:  sepchar
	{
		prevdep = 0;
		$$ = 0;
	}
	| dlist name
	{
		pp = ALLOC(depblock);
		pp->nextdep = NULL;
		pp->depname = $2;
		if ( !prevdep )
			$$ = pp;
		else
			prevdep->nextdep = pp;
		prevdep = pp;
	}
	;

deplist:	
	{
		fatal1("Must be a separator on rules line %d (bu39)",
			yylineno);
	}
	| dlist
	;

sepchar:  COLON
	{
		sepc = ALLDEPS;
	}
	| DOUBLECOLON
	{
		sepc = SOMEDEPS;
	}
	;

bldlist:
	{
		$$ = 0;
	}
	| bld
	{
		$$ = $1;
	}
	;

bld:	BUILDEC
	{
		$$ = $1;
		prevblp = $1;
	}
	| bld BUILDEC
	{
		$$ = $1;
		prevblp->nxtbldblock = $2;
		prevblp = $2;
	}
	;

shlist:	SHELLINE
	{
		$$ = $1;
		prevshp = $1;
	}
	| shlist SHELLINE
	{
		$$ = $1;
		prevshp->nextsh = $2;
		prevshp = $2;
	}
	;

shellist:
	{
		$$ = 0;
	}
	| shlist
	{
		$$ = $1;
	}
	;

%%

/*	@(#)make:gram.y	1.6 of 5/9/86	*/

#include <ctype.h>

CHARSTAR zznextc;	/* zero if need another line; otherwise points to next char */
int yylineno;
static char inmacro = NO;

extern char funny[ARY256];

static char *word;
static int word_count;
init_lex()
{
	word_count = 128;
	if ( ( word = malloc(word_count) ) == 0 )
		fatal("malloc failed");
}

int
yylex()
{
	register CHARSTAR p, q;
	CHARSTAR pword;
	static int	nextlin(), retsh();

	if ( !zznextc )
		return( nextlin() );

	while ( isspace(*zznextc) )
		++zznextc;

	if ( inmacro ) {
		inmacro = NO;
		yylval.ycharstring = copys(zznextc);
		zznextc = 0;
		return(A_STRING);
	}

	if (*zznextc == CNULL)
		return( nextlin() );

	if (*zznextc == KOLON) {
		if(*++zznextc == KOLON)	{
			++zznextc;
			return(DOUBLECOLON);
		} else
			return(COLON);
	}

	if (*zznextc == EQUALS) {
		inmacro = YES;
		++zznextc;
		return(EQUAL);
	}

	if (*zznextc == SKOLON)
		return( retsh(zznextc) );

	p = zznextc;
	{
		int len;
		while ( !( funny[(unsigned char) *p] & TERMINAL) )
			p++;
		len = p - zznextc;
		while ( (len+1) > word_count ) {
			word_count *= 2;
			word = realloc(word,word_count);
		}
	}
	q = pword = word;

	p = zznextc;

	while ( !( funny[(unsigned char) *p] & TERMINAL) )
		*q++ = *p++;

	if (p != zznextc) {
		*q = CNULL;
		yylval.ycharstring = copys(pword);
		if (*p == RCURLY) {
			zznextc = p+1;
			return(VERSION);
		}
		if (*p == LCURLY)
			p++;
		zznextc = p;
		return(NAME);
	} else {
		fprintf(stderr,"bad character %c (octal %o), line %d (bu40)",
			*zznextc,*zznextc,yylineno);
		fatal(Nullstr);
	}
	return(0);	/* never executed */
}


static int
retsh(q)
register CHARSTAR q;
{
	extern CHARSTAR *linesptr;
	register CHARSTAR p;
	SHBLOCK sp = ALLOC(shblock);
	static int	GETC();

	for (p = q + 1; *p==BLANK || *p==TAB; ++p)
		;
	sp->nextsh = 0;
	sp->shbp = ( !fin ? p : copys(p) );
	yylval.yshblock = sp;
	zznextc = 0;
/*
 *	The following if-else "thing" eats up newlines within
 *	shell blocks.
 */
	if ( !fin ) {
		if (linesptr[0])
			while (linesptr[1] && STREQ(linesptr[1], "\n")) {
				yylineno++;
				linesptr++;
			}
	} else {
		register int c;
		while ((c = GETC()) == NEWLINE)
			yylineno++;
		if (c != EOF)
			(void)ungetc(c, fin);
	}
	return(SHELLINE);
}


static int
retbld(q)
CHARSTAR q;
{
	register CHARSTAR p;
	BLDBLOCK sp;

	for (p = q+1; *p==BLANK || *p==TAB; ++p );
	sp = ALLOC(bldblock);
	sp->nxtbldblock = NULL;
	sp->bldecp = ( !fin ? p : copys(p) );
	yylval.ybldblock = sp;
	zznextc = 0;
	return(BUILDEC);
}


static int
nextlin()
{
	extern CHARSTAR *linesptr;
	static char yytext[INMAX];
	static CHARSTAR yytextl = yytext+INMAX;
	register char c;
	register CHARSTAR p, t;
	char 	templin[INMAX], lastch;
	CHARSTAR text, lastchp, subst();
	int	sindex();
	int 	incom, kc, nflg, poundflg;
	static void	fstack();
	static int	GETC();

again:	incom = 0;
	zznextc = 0;
	poundflg = 0;

	if ( !fin ) {
		if ( !(text = *linesptr++) )
			return(0);
		++yylineno;
		(void)copstr(yytext, text);
		CHKARY(nextlin,yytext,INMAX)  /*macro defined in defs*/
	} else {
		yytext[0] = CNULL;
		for (p = yytext; ; ++p) {
			if(p > yytextl)
				fatal("line too long");
			if ((kc = GETC()) == EOF) {
				*p = CNULL;
				return(0);
			}
			else if ((kc == SKOLON) ||
				 (kc == TAB && p == yytext))
				++incom;
			else if (kc == POUND && !incom && yytext[0] != TAB) {
				poundflg++;
				kc = CNULL;
			}
			else if (kc == NEWLINE)	{
				++yylineno;
				if (p == yytext || p[-1] != BACKSLASH)
					break;
				if(incom || yytext[0] == TAB)
					*p++ = NEWLINE;
				else
					p[-1] = BLANK;
				nflg = YES;
				while (kc = GETC()) {
					if ( !( kc == TAB ||
					        kc == BLANK ||
					        kc == NEWLINE) )
						break;
					if (incom || yytext[0] == TAB) {
						if (nflg && kc == TAB) {
							nflg = NO;
							continue;
						}
						if (kc == NEWLINE)
							nflg = YES;
						*p++ = kc;
					}
					if (kc == NEWLINE)
						++yylineno;
				}
				if(kc == EOF) {
					*p = CNULL;
					return(0);
				}
			}
			*p = kc;
		}
		*p = CNULL;
		text = yytext;
	}
	
	if ((c = text[0]) == TAB)  
		return( retsh(text) );
	if (c == EXCLAM)
		return( retbld(text) );
/*
 *	DO include FILES HERE.
 */
	if ( !sindex(text, "include") && 
	     (text[7] == BLANK || text[7] == TAB)) {
		CHARSTAR pfile;

		for (p = &text[8]; *p != CNULL; p++)
			if(*p != TAB &&
			   *p != BLANK)
				break;
		pfile = p;
		for (;	*p != CNULL	&&
			*p != NEWLINE	&&
			*p != TAB	&&
			*p != BLANK; p++)
			;
		if (*p != CNULL)
			*p = CNULL;

/*
 *	Start using new file.
 */
		(void)subst(pfile, templin);
		CHKARY(nextlin,templin,INMAX)  /*macro defined in defs*/
		fstack(templin, &fin, &yylineno);
		goto again;
	}
	if (isalpha(c) || isdigit(c) || c==BLANK || c==DOT || c=='_')
		for (p = text+1; *p!=CNULL; p++)
			if (*p == KOLON || *p == EQUALS)
				break;

/* substitute for macros on dependency line up to the semicolon if any */
	if (*p != EQUALS) {
		for (t = yytext ; *t!=CNULL && *t!=SKOLON ; ++t)
			;

		lastchp = t;
		lastch = *t;
		*t = CNULL;

		(void)subst(yytext, templin);
		CHKARY(nextlin,templin,INMAX)  /*macro defined in defs*/

		if (lastch) {
			for (t = templin ; *t ; ++t)
				;
			*t = lastch;
			while ( *++t = *++lastchp )
				;
		}
		CHKARY(nextlin,templin,INMAX)  /*macro defined in defs*/

		p = templin;
		t = yytext;
		while ( *t++ = *p++ )
			;
	}

	if ( !poundflg || yytext[0] != CNULL)	{
		zznextc = text;
		return(START);
	} else
		goto again;
}


#include "stdio.h"

static int morefiles;

static struct sfiles {
	char sfname[ARY64];
	FILE *sfilep;
	int syylno;
} sfiles[20];


/* GETC() automatically unravels stacked include files.  During
 *	`include' file processing, when a new file is encountered,
 *	fstack will stack the FILE pointer argument. Subsequent
 *	calls to GETC with the new FILE pointer will get characters
 *	from the new file. When an EOF is encountered, GETC will
 *	check to see if the file pointer has been stacked. If so,
 *	a character from the previous file will be returned.
 */
static int
GETC()
{
	register int c;

	while ((c = getc(fin)) == EOF && morefiles) {
		(void)fclose(fin);
		yylineno = sfiles[--morefiles].syylno;
		fin = sfiles[morefiles].sfilep;
	}
	return(c);
}


/* fstack(stfname,ream,lno) is used to stack an old file pointer
 *	before the new file is assigned to the same variable. Also
 *	stacked are the file name and the old current lineno,
 *	generally, yylineno. 
 */
static	void
fstack(newname, oldfp, oldlno)
register char *newname;
register FILE **oldfp;
register int *oldlno;
{
	FILE *newfp;			/* new file descriptor */
	char *strcat();
	static int	GETC();

#ifdef MKDEBUG
	if (IS_ON(DBUG))
		printf("Include file: \"%s\"\n",newname);
#endif
	/* if unable to open file */
	if ( !(newfp = opdescf(newname)) ) {
		/* try sccs get of file */
		if ( !vpget(newname,CD,Nullstr) )
			fatal1("cannot get %s for including\n",newname);
		if ( !(newfp = opdescf(newname)) )
			fatal1("cannot open %s for including\n",newname);
	}
/*
 *	Stack the new file name, the old file pointer and the
 *	old yylineno;
 */
	(void)strcat(sfiles[morefiles].sfname, newname);
	CHKARY(GETC,sfiles[morefiles].sfname,ARY64)  /*macro defined in defs*/
	sfiles[morefiles].sfilep = *oldfp;
	sfiles[morefiles++].syylno = *oldlno;
	yylineno = 0;
	*oldfp = newfp;				/* set the file pointer */
}

