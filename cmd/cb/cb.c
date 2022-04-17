/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)cb:cb.c	1.12.6.2"

#include "sgs.h"
#include <stdio.h>

/* character type flags */
#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define _P	020
#define _C      040
#define _X      0100
#define _O	0200

/* this is a local implementation of the concept of ctype.h */
/* in /usr/src/libc/gen/ctype_.c */

#define isop(c)		((my_ctype_+1)[c]&_O)		/* added for cb */
#define	isalpha(c)	((my_ctype_+1)[c]&(_U|_L))
#define	isdigit(c)	((my_ctype_+1)[c]&_N)
#define	isspace(c)	((my_ctype_+1)[c]&_S)
#define ispunct(c)	((my_ctype_+1)[c]&_P)
#define isalnum(c)	((my_ctype_+1)[c]&(_U|_L|_N))
unsigned char my_ctype_[] = {
	0,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C|_S,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
	_S,	_P|_O,	_P,	_P,	_P,	_P|_O,	_P|_O,	_P,
	_P,	_P,	_P|_O,	_P|_O,	_P,	_P|_O,	_P,	_P|_O,
	_N,	_N,	_N,	_N,	_N,	_N,	_N,	_N,
	_N,	_N,	_P,	_P,	_P|_O,	_P|_O,	_P|_O,	_P,
	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
	_U,	_U,	_U,	_P,	_P,	_P,	_P|_O,	_P|_L,
	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
	_L,	_L,	_L,	_P,	_P|_O,	_P,	_P,	_C
};

#define	IF	1
#define	ELSE	2
#define	CASE	3
#define TYPE	4
#define DO	5
#define STRUCT	6
#define OTHER	7
#define ASM	8

#define ALWAYS	01
#define	NEVER	02
#define	SOMETIMES	04

#define YES	1
#define NO	0

#define	KEYWORD	1
#define	DATADEF	2

#define CLEVEL	20
#define IFLEVEL	10
#define DOLEVEL	10
#define OPLENGTH	10
#define LINE	256
#define LINELENG	10000
#define MAXTABS	8
#define TABLENG	8
#define TEMP	1024

#define OUT	outs(clev->tabs); putchar('\n');opflag = lbegin = 1; count = 0
#define OUTK	OUT; keyflag = 0;
#define BUMP	clev->tabs++; clev->pdepth++
#define UNBUMP	clev->tabs -= clev->pdepth; clev->pdepth = 0
#define NOTEOF	(! feof(input))
#define eatspace()	while(NOTEOF && ((cc=getch()) == ' ' || cc == '\t')); unget(cc)
#define eatallsp()	while(NOTEOF && ((cc=getch()) == ' ' || cc == '\t' || cc == '\n')); unget(cc)

struct indent {		/* one for each level of { } */
	int tabs;
	int pdepth;
	int iflev;
	int ifc[IFLEVEL];
	int spdepth[IFLEVEL];
} ind[CLEVEL];

struct indent *clev = ind;

struct keyw {			/* keyword table -- only necessary keywords */
	char	*name;
	char	punc;			/* punctuation for strict option */
	char	type;
} key[] = {
	"if", ' ', IF,
	"else", ' ', ELSE,
	"int", '\t', TYPE,
	"for", ' ', OTHER,
	"char", '\t', TYPE,
	"long", '\t', TYPE,
	"case", ' ', CASE,
	"register", ' ', TYPE,
	"struct", ' ', STRUCT,
	"while", ' ', OTHER,
	"signed", '\t', TYPE,
	"short", '\t', TYPE,
	"unsigned", '\t', TYPE,
	"const", '\t', TYPE,
	"volatile", '\t', TYPE,
	"static", ' ', TYPE,
	"extern", ' ', TYPE,
	"void", '\t', TYPE,
	"switch", ' ', OTHER,
	"default", ' ', CASE,
	"union", ' ', STRUCT,
	"double", '\t', TYPE,
	"float", '\t', TYPE,
	"enum", ' ', STRUCT,
	"typedef", ' ', TYPE,
	"do", ' ', DO,
	"auto", ' ', TYPE,
	"asm", ' ', ASM,
	0, 0, 0
};

struct keyw *lookup();

struct op {				/* operator table */
	char	*name;
	char	blanks;				/* blank enclosure key for -s */
	char	setop;
} op[] = {
	"+=", 	ALWAYS,  YES,
	"-=", 	ALWAYS,  YES,
	"*=", 	ALWAYS,  YES,
	"/=", 	ALWAYS,  YES,
	"%=", 	ALWAYS,  YES,
	">>=", 	ALWAYS,  YES,
	"<<=", 	ALWAYS,  YES,
	"&=", 	ALWAYS,  YES,
	"^=", 	ALWAYS,  YES,
	"|=", 	ALWAYS,  YES,
	">>", 	ALWAYS,  YES,
	"<<", 	ALWAYS,  YES,
	"<=", 	ALWAYS,  YES,
	">=", 	ALWAYS,  YES,
	"==", 	ALWAYS,  YES,
	"!=", 	ALWAYS,  YES,
	"=", 	ALWAYS,  YES,
	"&&", 	ALWAYS, YES,
	"||", 	ALWAYS, YES,
	"++", 	NEVER, NO,
	"--", 	NEVER, NO,
	"->", 	NEVER, NO,
	"<", 	ALWAYS, YES,
	">", 	ALWAYS, YES,
	"+", 	ALWAYS, YES,
	"/", 	ALWAYS, YES,
	"%", 	ALWAYS, YES,
	"^", 	ALWAYS, YES,
	"|", 	ALWAYS, YES,
	"!", 	NEVER, YES,
	"~", 	NEVER, YES,
	"*", 	SOMETIMES, YES,
	"&", 	SOMETIMES, YES,
	"-", 	SOMETIMES, YES,
	"?",	ALWAYS,YES,
	":",	ALWAYS,YES,
	0, 	0,0
};

FILE *input = stdin;

char *getnext();
char puttmp();
void exit();

int	tossint;		/* toss away the integer value returned by fcn */
char	tosschar;		/* toss away the char value returned by fcn */
int	initflag = 0;		/* initialization flag, set if inside an init */
int	curlct = 0;		/* { } counter inside an initialization */
int	enumsaw = 0;		/* flags for identifying enums */
int	enumflag = 0;
int	strict = 0;
int	join	= 0;
int	opflag = 1;
int	keyflag = 0;
int	paren	 = 0;
int	split	 = 0;
int	folded	= 0;
int	dolevel	=0;
int	dotabs[DOLEVEL];
int	docurly[DOLEVEL];
int	dopdepth[DOLEVEL];
int	structlev = 0;
int	question	 = 0;
int	err = 0;
int	maxleng	= LINELENG;
int	maxtabs	= MAXTABS;
int	count	= 0;		/* chars in output buffer string */
int	inswitch	=0;	/* input from stdin(0) or input buffer temp(1) */
int	lbegin	 = 1;		/* output line begin flag: 1=beginning of new line*/

char	string[LINE];	/* output buffer */
char	*lastlook;		/* last "token" seen, now in output buffer */
char	*p = string;		/* next available slot in output buffer */
char	temp[TEMP];	/* input buffer -- in use if inswitch == 1 */
char	*tp;			/* next available slot */
char	*lastplace = temp;	/* last position used (tp-1)? */
char	*tptr = temp;		/* current position in temp? */
char	next = '\0';


main(argc, argv)

int	argc;
char	**argv;
{
	extern char *optarg;
	extern int optind;
	int	arg;
	int errflg = 0;

	while ((arg = getopt(argc,argv,"Vsjl:")) != -1){
		switch (arg){
		case 's':
			strict = 1;
			break;
		case 'j':
			join = 1;
			break;
		case 'l':
			maxleng = atoi(optarg);
			if ( (maxleng < 10) || (maxleng > LINELENG) ) {
				(void) fprintf(stderr, "cb: invalid length value specified - must be between %d and %d\n", 10, LINELENG);
				errflg++;
			} else {
				maxtabs = maxleng/TABLENG - 2;
				maxleng = (9 * maxleng)/10;
			}
			break;
		case 'V':
			(void) fprintf(stderr,"%s cb - %s\n",ESG_PKG,ESG_REL);
			break;
		case '?':
			errflg++;
		}
	}
	if(errflg) {
		(void) fprintf(stderr, "Usage: cb [-s] [-j] [-l length] [-V] [file...] \n");
		exit(1);
	}
	if (optind >= argc)work();
	else {
		while (optind < argc){
			if ((input = fopen( argv[optind], "r")) == NULL){
				(void) fprintf(stderr, "cb: cannot open input file %s\n", argv[optind]);
				exit(1);
			}
			work();
			(void) fclose(input);
			optind++;
		}
	}
	return(0);
}
work()

{
	register int c;			/* generally the input character */
	register struct keyw *lptr;
	char *pt;			/* holds return value from getnext */
	char cc;			/* lookahead one on input stream */
	int ct;				/* holds return value from getnl */

	while ( NOTEOF && ((c = getch()) != EOF)) {
		switch (c){
		case '{':
			if ((lptr = lookup(lastlook,p)) != 0){
				if (lptr->type == ELSE)gotelse();
				else if(lptr->type == DO)gotdo();
				else if(lptr->type == STRUCT)structlev++;
			}
			if(++clev >= &ind[CLEVEL-1]){
				(void) fprintf(stderr,"cb: too many levels of curly brackets\n");
				clev = &ind[CLEVEL-1];
			}
			clev->pdepth = 0;
			clev->tabs = (clev-1)->tabs;
			clearif(clev);
			if(strict && clev->tabs > 0)
				putspace(' ',NO);
			putch(c,NO);
			tossint=getnl();
			if(keyflag == DATADEF){
				OUT;
			}
			else {
				OUTK;
			}
			clev->tabs++;
			if (initflag && !enumflag) {	/* to handle initialization*/
				int sawquote;
				curlct=1;
				sawquote = 0;
				while( NOTEOF ){
					c=getch();
					if(sawquote || c == '"') {
						if(sawquote && c == '"')
							sawquote = 0;
						else	sawquote = 1;
						putch(c,NO);
						continue;
					}
					if(c == ',' && strict){
						putch(c,NO);
						putspace(' ',NO);
						eatspace();
						continue;
					}
					if(c == '\n'){
						OUT;
						continue;
					}
					if(c=='{'){
						curlct++;
						putch(c,NO);
						if(strict){
							putch(' ',NO);
							eatspace();
						}
						continue;
					} else if (c=='}'){
						curlct--;
						if(strict) putspace(' ',NO);
						eatallsp();
					}
					if(curlct==0){
						unget(c);
						break;
					} else putch(c,NO);
				}
			} 
			continue;
		case '}':
			pt=getnext(0);
			outs(clev->tabs);
			if(enumflag){
				putchar('\n');
			}
			if(--clev < ind)clev = ind;
			ptabs(clev->tabs);
			putch(c,NO);
			lbegin = 0;
			lptr=lookup(pt,lastplace+1);
			c = *pt;
			if(*pt == ';' || *pt == ','){
				putch(*pt,NO);
				if(*pt == ';' && initflag) initflag=0;
				*pt = '\0';
				lastplace=pt;
			}
			ct = getnl();
			if((dolevel && clev->tabs <= dotabs[dolevel]) || (structlev )
			    || (lptr != 0 &&lptr->type == ELSE&& clev->pdepth == 0)){
				if(c == ';'){
					OUTK;
				}
				else if(strict || (lptr != 0 && lptr->type == ELSE && ct == 0)){
					putspace(' ',NO);
					eatspace();
				}
				else if(lptr != 0 && lptr->type == ELSE){
					OUTK;
				}
				if(structlev){
					structlev--;
					if (enumflag) enumflag=0;
					keyflag = DATADEF;
				}
			}
			else {
				OUTK;
				if(!initflag && strict && clev->tabs == 0){
					if((c=getch()) != '\n'){
						putchar('\n');
						putchar('\n');
						unget(c);
					}
					else {
						putchar('\n');
						if((c=getch()) != '\n')unget(c);
						putchar('\n');
					}
				}
			}
			if(lptr != 0 && lptr->type == ELSE && clev->pdepth != 0){
				UNBUMP;
			}
			if(lptr == 0 || lptr->type != ELSE){
				clev->iflev = 0;
				if(dolevel && docurly[dolevel] == NO && clev->tabs == dotabs[dolevel]+1)
					clev->tabs--;
				else if(clev->pdepth != 0){
					UNBUMP;
				}
			}
			continue;
		case '(':
			paren++;
			if ((lptr = lookup(lastlook,p)) != 0){
				if(!(lptr->type == TYPE || lptr->type == STRUCT))keyflag=KEYWORD;
				if (strict){
					putspace(lptr->punc,NO);
					opflag = 1;
				}
				putch(c,NO);
				if (lptr->type == IF)gotif();
			}
			else {
				putch(c,NO);
				lastlook = p;
				opflag = 1;
			}
			continue;
		case ')':
			if(--paren < 0)paren = 0;
			putch(c,NO);
			if((lptr = lookup(lastlook,p)) != 0){
				if(lptr->type == TYPE || lptr->type == STRUCT)
					opflag = 1;
			}
			else if(keyflag == DATADEF)opflag = 1;
			else opflag = 0;
			outs(clev->tabs);
			pt = getnext(1);
			if ((ct = getnl()) == 1 && !strict){
				if(dolevel && clev->tabs <= dotabs[dolevel])
					resetdo();
				if(clev->tabs > 0 && (paren != 0 || keyflag == 0)){
					if(join){
						eatspace();
						putch(' ',YES);
						continue;
					} else {
						OUT;
						split = 1;
						continue;
					}
				}
				else if(clev->tabs > 0 && *pt != '{'){
					BUMP;
				}
				OUTK;
			}
			else if(strict){
				if(clev->tabs == 0){
					if(*pt != ';' && *pt != ',' && *pt != '(' && *pt != '[' && *pt != ')' ){
						OUTK;
					}
				}
				else {
					if(keyflag == KEYWORD && paren == 0){
						if(dolevel && clev->tabs <= dotabs[dolevel]){
							resetdo();
							eatspace();
							continue;
						}
					    	if(*pt != ';'){
							if(*pt != '{'){
								BUMP;
								OUTK;
							}
							else {
								*pt='\0';
								eatspace();
								unget('{');
							}
					    	}
					}
					else if(ct){
						if(paren){
							if(join){
								eatspace();
							} else {
								split = 1;
								OUT;
							}
						}
						else {
							OUTK;
						}
					}
				}
			}
			if(dolevel && clev->tabs <= dotabs[dolevel])
				resetdo();
			continue;
		case ' ':
		case '\t':
			if ((lptr = lookup(lastlook,p)) != 0){
				if(!(lptr->type==TYPE||lptr->type==STRUCT))
					keyflag = KEYWORD;
				else if(paren == 0)keyflag = DATADEF;
				if(strict){
					if(lptr->type != ELSE){
						if(lptr->type == TYPE){
							if(paren != 0)putch(' ',YES);
						}
						else
							putch(lptr->punc,NO);
						eatspace();
					}
				}
				else putch(c,YES);
				switch(lptr->type){
				case CASE:
					outs(clev->tabs-1);
					continue;
				case ELSE:
					pt = getnext(1);
					eatspace();
					if((cc = getch()) == '\n' && !strict){
						unget(cc);
					}
					else {
						unget(cc);
						if(checkif(pt))continue;
					}
					gotelse();
					if(strict) unget(c);
					if(getnl() == 1 && !strict){
						OUTK;
						if(*pt != '{'){
							BUMP;
						}
					}
					else if(strict){
						if(*pt != '{'){
							OUTK;
							BUMP;
						}
					}
					continue;
				case IF:
					gotif();
					continue;
				case DO:
					gotdo();
					pt = getnext(1);
					if(*pt != '{'){
						eatallsp();
						OUTK;
						docurly[dolevel] = NO;
						dopdepth[dolevel] = clev->pdepth;
						clev->pdepth = 0;
						clev->tabs++;
					}
					continue;
				case TYPE:
					if(paren)continue;
					if(!strict)continue;
					gottype(lptr);
					continue;
				case STRUCT:
					if(!strcmp(lptr->name,"enum")) enumsaw=1;
					gotstruct(enumsaw);
					enumsaw = 0;
					continue;
				case ASM:
					gotasm();
					continue;
				}
			}
			else if (lbegin == 0 || p > string) 
				if(strict)
					putch(c,NO);
				else putch(c,YES);
			continue;
		case ';':
			if(initflag) initflag=0;
			putch(c,NO);
			if(paren != 0){
				if(strict){
					putch(' ',YES);
					eatspace();
				}
				opflag = 1;
				continue;
			}
			outs(clev->tabs);
			pt = getnext(0);
			lptr=lookup(pt,lastplace+1);
			if(lptr == 0 || lptr->type != ELSE){
				clev->iflev = 0;
				if(clev->pdepth != 0){
					UNBUMP;
				}
				if(dolevel && docurly[dolevel] == NO && clev->tabs <= dotabs[dolevel]+1)
					clev->tabs--;
/*
				else if(clev->pdepth != 0){
					UNBUMP;
				}
*/
			}
			tossint=getnl();
			OUTK;
			continue;
		case '\n':
			if ((lptr = lookup(lastlook,p)) != 0){
				pt = getnext(1);
				if (lptr->type == ELSE){
					if(strict)
						if(checkif(pt))continue;
					OUTK;
					gotelse();
					if(*pt != '{'){
						BUMP;
					}
				}
				else if(lptr->type == DO){
					OUTK;
					gotdo();
					if(*pt != '{'){
						docurly[dolevel] = NO;
						dopdepth[dolevel] = clev->pdepth;
						clev->pdepth = 0;
						clev->tabs++;
					}
				}
				else {
					OUTK;
					if(lptr->type == STRUCT){
						if(!strcmp(lptr->name,"enum")) enumsaw=1;
						gotstruct(enumsaw);
						enumsaw = 0;
					}
				}
			}
			else if(p == string)putchar('\n');
			else {
				if(clev->tabs > 0 &&(paren != 0 || keyflag == 0)){
					if(join){
						putch(' ',YES);
						eatspace();
						continue;
					} else {
						OUT;
						split = 1;
						continue;
					}
				}
				else if(keyflag == KEYWORD){
					OUTK;
					continue;
				}
				OUT;
			}
			continue;
		case '"':
		case '\'':
			putch(c,NO);
			while (NOTEOF && ((cc = getch()) != c)){
				putch(cc,NO);
				if (cc == '\\'){
					putch(getch(),NO);
				}
				if (cc == '\n'){
					outs(clev->tabs);
					lbegin = 1;
					count = 0;
				}
			}
			putch(cc,NO);
			opflag=0;
			if (getnl() == 1){
				unget('\n');
			}
			continue;
		case '\\':
			putch(c,NO);
			putch(getch(),NO);
			continue;
		case '?':
			question = 1;
			gotop(c);
			continue;
		case ':':
			if (question == 1){
				question = 0;
				gotop(c);
				continue;
			}
			putch(c,NO);
			if(structlev)continue;
			if ((lptr = lookup(lastlook,p)) != 0){
				if (lptr->type == CASE)outs(clev->tabs - 1);
			}
			else {
				lbegin = 0;
				outs(clev->tabs);
			}
			tossint=getnl();
			OUTK;
			continue;
		case '/':
			if ((cc = getch()) != '*'){
				unget(cc);
				gotop(c);
				continue;
			}
			putch(c,NO);
			putch(cc,NO);
			cc = comment(YES);
			if(getnl() == 1){
				if(cc == 0){
					OUT;
				}
				else {
					outs(0);
					putchar('\n');
					lbegin = 1;
					count = 0;
				}
				lastlook = 0;
			}
			continue;
		case '[':
			putch(c,NO);
			ct = 0;
			while(NOTEOF && ((c = getch()) != ']' || ct > 0)) {
				putch(c,NO);
				if(c == '[')ct++;
				if(c == ']')ct--;
			}
			putch(c,NO);
			continue;
		case '#':
			if ( strict && (lbegin == 0) ) { OUT;}
			putch(c,NO);
			while (NOTEOF && ((cc = getch()) != '\n')) {
				if (cc == '\\'){
					putch(cc,NO);
					cc = getch();
				}
				putch(cc,NO);
				if (cc == '/'){
					if ( (cc=getch()) == '*'){
						putch(cc,NO);
						cc = comment(NO);
					} else
						unget(cc);
				}
			}
			putch(cc,NO);
			lbegin = 0;
			outs(clev->tabs);
			lbegin = 1;
			count = 0;
			continue;
		default:
			if (c == ','){
				opflag = 1;
				putch(c,YES);
				if (strict){
					if ((cc = getch()) != ' ')unget(cc);
					if(cc != '\n')putch(' ',YES);
				}
			}
			else if(isop(c))gotop(c);
			else {
				if(isalnum(c) && lastlook == 0)lastlook = p;
				putch(c,NO);
				if(keyflag != DATADEF)opflag = 0;
			}
		}
	}
	if (inswitch) {		/* flush last line in buffer */
		c = strlen(tptr) - 1;	/* using 'c' as a temp */
		*(tptr + c) = '\0';
		(void) fputs(tptr, stdout);
		inswitch = 0;
	}
	/* now need to reset state to (optionally) process next file */
	initflag = 0;
	curlct = 0;
	enumsaw = 0;
	enumflag = 0;
	opflag = 1;
	keyflag = 0;
	paren = 0;
	split = 0;
	folded = 0;
	dolevel	= 0;
	structlev = 0;
	question = 0;
	err = 0;
	count = 0;
	lbegin = 1;
	p = string;
	lastplace = temp;
	tptr = temp;
	next = '\0';
}

gotif()		/* have an "if" -- bump iflevel */

{
	outs(clev->tabs);
	if(++clev->iflev >= IFLEVEL-1){
		(void) fprintf(stderr,"cb: too many levels of if\n");   /* warning only */
		clev->iflev = IFLEVEL-1;
	}
	clev->ifc[clev->iflev] = clev->tabs;
	clev->spdepth[clev->iflev] = clev->pdepth;
}

gotelse()	/* inverse of gotif -- unbump iflevel */

{
	clev->tabs = clev->ifc[clev->iflev];
	clev->pdepth = clev->spdepth[clev->iflev];
	if(--(clev->iflev) < 0)clev->iflev = 0;
}

checkif(pt)	/* format "if" for output */

char *pt;
{
	register struct keyw *lptr;
	int cc;
	if((lptr=lookup(pt,lastplace+1))!= 0){
		if(lptr->type == IF){
			if(strict)putch(' ',YES);
			copy(lptr->name);
			*pt='\0';
			lastplace = pt;
			if(strict){
				putch(lptr->punc,NO);
				eatallsp();
			}
			clev->tabs = clev->ifc[clev->iflev]; /* see gotelse above?*/
			clev->pdepth = clev->spdepth[clev->iflev];
			keyflag = KEYWORD;
			return(1);
		}
	}
	return(0);
}

gotdo()		/* have a do -- bump dolevel */

{
	if(++dolevel >= DOLEVEL-1){
		(void) fprintf(stderr,"cb: too many levels of do\n");
		dolevel = DOLEVEL-1;
	}
	dotabs[dolevel] = clev->tabs;
	docurly[dolevel] = YES;
}

resetdo()

{
	if(docurly[dolevel] == NO)
		clev->pdepth = dopdepth[dolevel];
	if(--dolevel < 0)dolevel = 0;
}

gottype(lptr)

struct keyw *lptr;
{
	char *pt;
	struct keyw *tlptr;
	int c;
	while(1){
		pt = getnext(1);
		if((tlptr=lookup(pt,lastplace+1))!=0){
			if(!strcmp(tlptr->name,"enum")) enumsaw=1;
			putch(' ',YES);
			copy(tlptr->name);
			*pt='\0';
			lastplace = pt;
			if(tlptr->type == STRUCT){
				putch(tlptr->punc,YES);
				gotstruct(enumsaw);
				enumsaw = 0;
				break;
			}
			lptr=tlptr;
			continue;
		}
		else{
			putch(lptr->punc,NO);
			while(NOTEOF && ((c=getch())== ' ' || c == '\t'));
			unget(c);
			break;
		}
	}
}

gotstruct(enumseen)

int enumseen;		/* set to 1 if this struct is an enum */

{
	int c;
	int cc;
	char *pt;

	while(NOTEOF && ((c=getch()) == ' ' || c == '\t'))
		if(!strict)putch(c,NO);
	if(c == '{'){
		structlev++;
		enumflag = enumseen;
		unget(c);
		return;
	}
	if(isalpha(c)){
		putch(c,NO);
		while(isalnum(c=getch()))putch(c,NO);
	}
	unget(c);
	pt = getnext(1);
	if(*pt == '{'){
		structlev++;
		enumflag = enumseen;
	}
	if(strict){
		eatallsp();
		putch(' ',NO);
	}
}

gotasm()		/* process an asm by passing through source until */
			/* matching } seen */
{
	register int ch;
	int bracect = 0;

	while( (ch=getch()) != EOF ) {
		putch(ch,NO);
		if ( ch == '{' ) {
			bracect++;
			continue;
		}
		if ( ch == '}' ) {
			if ( --bracect == 0 ) return;
		}
	}
}

gotop(c)		/* find the operator in the optable and ... */

{
	char optmp[OPLENGTH];
	char *op_ptr;
	struct op *s_op;
	char *a, *b;
	if(c == '=' && keyflag==DATADEF) initflag=1;
	op_ptr = optmp;
	*op_ptr++ = c;
	while (isop(( *op_ptr = getch())))op_ptr++;
	if(!strict)unget(*op_ptr);
	else if (*op_ptr != ' ')unget( *op_ptr);
	*op_ptr = '\0';
	s_op = op;
	b = optmp;
	while ((a = s_op->name) != 0){
		op_ptr = b;
		while ((*op_ptr == *a) && (*op_ptr != '\0')){
			a++;
			op_ptr++;
		}
		if (*a == '\0'){
			keep(s_op);
			opflag = s_op->setop;
			if (*op_ptr != '\0'){
				b = op_ptr;
				s_op = op;
				continue;
			}
			else break;
		}
		else s_op++;
	}
}

keep(o)		/* output operator with proper punctuation under strict */

struct op *o;
{
	char	*s;
	int ok;
	char *bp;
	int eflag = 0;		/* check for E notation */
	ok = !strict;
	if(strict && (*o->name == '-' || *o->name == '+')){
		if(p>string && (*(p-1) == 'E' || *(p-1) == 'e')) {
			if((bp=p-2) >= string && !isspace(*bp)){
				while(bp>string && (isdigit(*bp)||*bp == '.')) bp--;
				eflag = !isalpha(*bp);
			}
		}
	}
	if ( !eflag && (strict && ((o->blanks & ALWAYS)
	    || ((opflag == 0 && o->blanks & SOMETIMES) && clev->tabs != 0))))
		putspace(' ',YES);
	for(s=o->name; *s != '\0'; s++){
		putch(*s,NO);
	}
	if ( !eflag && (strict && ((o->blanks & ALWAYS)
	    || ((opflag == 0 && o->blanks & SOMETIMES) && clev->tabs != 0)))) putch(' ',YES);
}

getnl()

{
	register int ch;
	char *savp;
	int gotcmt;
	gotcmt = 0;
	savp = p;
	while ((ch = getch()) == '\t' || ch == ' ')putch(ch,NO);
	if (ch == '/'){
		if ((ch = getch()) == '*'){
			putch('/',NO);
			putch('*',NO);
			tossint = comment(YES);
			ch = getch();
			gotcmt=1;
		}
		else {
			if(inswitch)*(++lastplace) = ch;
			else {
				inswitch = 1;
				*lastplace = ch;
			}
			unget('/');
			return(0);
		}
	}
	if(ch == '\n'){
		if(gotcmt == 0)p=savp;
		return(1);
	}
	unget(ch);
	return(0);
}

ptabs(n)	/* output n tabs */

{
	int	i;
	int num;
	if(n > maxtabs){
		if(!folded){
			(void) printf("/* code folded from here */\n");
			folded = 1;
		}
		num = n-maxtabs;
	}
	else {
		num = n;
		if(folded){
			folded = 0;
			(void) printf("/* unfolding */\n");
		}
	}
	for (i = 0; i < num; i++)putchar('\t');
}

outs(n)		/* print the output buffer prefaced by the appropriate */
		/* number of tabs;  set lbegin to 0 */
{
	if (p > string){
		if (lbegin){
			ptabs(n);
			lbegin = 0;
			if (split == 1){
				split = 0;
				if (clev->tabs > 0)(void) printf("    ");
			}
		}
		*p = '\0';
		(void) printf("%s", string);
		lastlook = p = string;
	}
	else {
		if (lbegin != 0){
			lbegin = 0;
			split = 0;
		}
	}
}

putch(c,ok)	/* put a character in the output buffer */

char	c;
int	ok;
{
	register int cc;

	if(p < &string[LINE-1]){
		if(count+TABLENG*clev->tabs >= maxleng && ok && !folded){
			if(c != ' ')*p++ = c;
			OUT;
			split = 1;
			if((cc=getch()) != '\n')unget(cc);
		}
		else {
			*p++ = c;
			count++;
		}
	}
	else {
		outs(clev->tabs);
		*p++ = c;
		count = 0;
	}
}

struct keyw *
lookup(first, last)	/* find the symbol beginning at first and ending at */
			/* last-1 in the keyword table and return a pointer */
char *first, *last;	/* to it if found, return 0 if not found            */
{
	struct keyw *ptr;
	char	*cptr, *ckey, *k;

	if(first == last || first == 0)return(0);
	cptr = first;
	while (*cptr == ' ' || *cptr == '\t')cptr++;
	if(cptr >= last)return(0);
	ptr = key;
	while ((ckey = ptr->name) != 0){
		for (k = cptr; (*ckey == *k && *ckey != '\0'); k++, ckey++);
		if(*ckey=='\0' && (k==last|| (k<last && !isalnum(*k)))){
			opflag = 1;
			lastlook = 0;
			return(ptr);
		}
		ptr++;
	}
	return(0);
}

comment(ok)

int	ok;
{
	register int ch;
	int hitnl, firstar;

	hitnl = 0;
	while ((ch  = getch()) != EOF){
		hitnl = 0;
		if (ch == '*'){
			firstar = 1;
gotstar:
			if ((ch  = getch()) == '/'){
				putch('*',NO);
				putch(ch,ok);
				return(hitnl);
			}
			if (firstar) putch('*',ok);
			firstar = 0;
			putch(ch,ok);
			if (ch == '*')goto gotstar;
		} else
			putch(ch, ok);
		if (ch == '\n'){
			if(ok && !hitnl){
				outs(clev->tabs);
			}
			else {
				outs(0);
			}
			lbegin = 1;
			count = 0;
			hitnl = 1;
		}
	}
	return(hitnl);
}

putspace(ch,ok)

char	ch;
int	ok;

{
	if(p == string)putch(ch,ok);
	else if (*(p - 1) != ch) putch(ch,ok);
}

getch()		/* get a character from the input stream(inswitch=0) or buffer(1)*/

{
	register char c;

	if(inswitch){
		if(next != '\0'){
			c=next;
			next = '\0';
			return(c);
		}
		if(tptr <= lastplace){
			if(*tptr != '\0')return(*tptr++);
			else if(++tptr <= lastplace)return(*tptr++);
		}
		inswitch=0;
		lastplace = tptr = temp;
	}
	return(getc(input));
}

unget(c)	/* inverse of getch */

{
	if(inswitch){
		if(tptr != temp)
			*(--tptr) = c;
		else next = c;
	}
	else	(void) ungetc(c,input);
}

/* ARGSUSED */
char *
getnext(must)

{
	int c;
	char *beg;
	int incomment;

	if(tptr > lastplace){
		tptr = lastplace = temp;
		err = 0;
		inswitch = 0;
	}
	tp = beg = lastplace;
	if(inswitch && tptr <= lastplace)
		if (isalnum(*lastplace)||ispunct(*lastplace)||isop(*lastplace))return(lastplace);
space:
	while(isspace(c=getc(input))) tosschar=puttmp(c,1);
	beg = tp;
	tosschar=puttmp(c,1);
	if(c == '/'){
		incomment = 0;
		if(puttmp((c=getc(input)),1) == '*'){
			incomment = 1;
			while (incomment) {
				while((c=getc(input)) != '*'){
					tosschar=puttmp(c,0);
				}
				tosschar=puttmp(c,1);
				if(puttmp((c=getc(input)),1) == '/'){
					incomment = 0;
					beg = tp;
					tosschar=puttmp((c=getc(input)),1);
				}
			}
			if(isspace(c))goto space;
		} else	 goto done;
	}

	if(isalnum(c)){
		while(isalnum(c = getc(input))) tosschar=puttmp(c,1);
		(void) ungetc(c,input);
	}
done:
	tosschar=puttmp('\0',1);
	lastplace = tp-1;
	inswitch = 1;
	return(beg);
}

copy(s)

char *s;
{
	while(*s != '\0')putch(*s++,NO);
}

clearif(cl)

struct indent *cl;
{
	int i;
	for(i=0;i<IFLEVEL-1;i++)cl->ifc[i] = 0;
}
char
puttmp(c,kept)	/* save 'c' in temp array; if kept = 0 then it's ok to
		 * toss 'c' if array has overflowed */

char	c;
int	kept;
{
	if(tp < &temp[TEMP-120])
		*tp++ = c;
	else {
		if(kept){
			if(tp >= &temp[TEMP-2]){  /* room for char and null */
				(void) fprintf(stderr,"cb: can't look past huge comment - quitting\n");
				exit(1);
			}
			*tp++ = c;
		}
		else if(err == 0){
			err++;
			(void) fprintf(stderr,"cb: truncating long comment\n"); /* warning */
		}
	}
	return(c);
}
