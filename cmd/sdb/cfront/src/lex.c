/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/src/lex.c	1.1"
/*ident	"@(#)cfront:src/lex.c	1.14" */
/***************************************************************************

	C++ source for cfront, the C++ compiler front-end
	written in the computer science research center of Bell Labs

	Copyright (c) 1984 AT&T, Inc. All Rights Reserved
	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T, INC.

	
lex.c:
	lexical analyser based on pcc's and cpre's scanners
	modified to handle classes:
	new keywords:	class
			public
			call
			etc.
	names are not entered in the symbol table by lex()
	names can be of arbitrary length
	error() is used to report errors
	{} and () must match
	numeric constants are not converted into internal representation
	but stored as strings

****************************************************************************/

#include "cfront.h"
#include "yystype.h"
#include "size.h"
#include "tqueue.h"

# define  CCTRANS(x) x

	/* lexical actions */

#define A_ERR	0		/* illegal character */
#define A_LET	1		/* saw a letter */
#define A_DIG	2		/* saw a digit */
#define A_1C	3		/* return a single character */
#define A_STR 	4		/* string */
#define A_CC	5		/* character constant */
#define A_BCD	6		/* GCOS BCD constant */
#define A_SL	7		/* saw a / */
#define A_DOT 	8		/* saw a . */
#define A_2C	9		/* possible two character symbol */
#define A_WS	10		/* whitespace (not \n) */
#define A_NL	11		/* \n */
#define A_LC	12		/* { */
#define A_RC	13		/* } */
#define A_L	14		/* ( */
#define A_R	15		/* ) */
#define A_EOF	16
#define A_ASS	17
#define A_LT	18
#define A_GT	19		/* > */
#define A_ER	20
#define A_OR	21
#define A_AND	22
#define A_MOD	23
#define A_NOT	24
#define A_MIN	25
#define A_MUL	26
#define A_PL	27
#define A_COL	28		/* : */
#define A_SHARP	29		/* # */

	/* character classes */

# define LEXLET 01
# define LEXDIG 02
/* no LEXOCT because 8 and 9 used to be octal digits */
# define LEXHEX 010
# define LEXWS 020
# define LEXDOT 040

	/* text buffer */
static char inbuf[TBUFSZ];
char* txtmax = &inbuf[TBUFSZ-1];
char* txtstart = 0;
char* txtfree = 0;
#define pch(c) ((txtmax<=txtfree)?(error('i',"input buffer overflow"),0):(*txtfree++=c))
#define start_txt()	txtstart = txtfree
#define del_txt()	txtfree = txtstart

static char* file_name[MAXFILE*4];	// source file names
					// file_name[0] == src_file_name
					// file_name[0] == 0 means stdin
static short file_stack[MAXFILE];	// stack of file name indeces
int curr_file;				// current index in file_stack
					// that is current #include nest level

class loc curloc;
FILE * out_file = stdout;
FILE * in_file = stdin;
Ptable ktbl;
int br_level = 0;		/* number of unmatched ``(''s */
int bl_level = 0;		/* number of unmatched ``{''s */

# ifdef ibm

# define CSMASK 0377
# define CSSZ 256

# else

# define CSMASK 0177
# define CSSZ 128

# endif

static short lxmask[CSSZ+1];

int saved = 0;	/* putback character, avoid ungetchar */
extern int lxtitle();

overload rt;
inline YYSTYPE rt(char* x) { YYSTYPE y; y.s  = x;        return y; }
inline YYSTYPE rt(TOK   x) { YYSTYPE y; y.t  = x;        return y; }
inline YYSTYPE rt(loc   x) { YYSTYPE y; y.l  = x;        return y; }
inline YYSTYPE rt(void* x) { YYSTYPE y; y.pn = (Pname)x; return y; }

#define get(c)		(c=getc(in_file))
#define unget(c)	ungetc(c,in_file)

#define reti(a,b)	{ addtok(a, rt(b));          return; }
#define retn(a,b)	{ addtok(a, rt((Pnode)b));   return; }
#define rets(a,b)	{ addtok(a, rt(b));          return; }
#define retl(a)		{ addtok(a, rt(curloc));     return; }

void ktbl_init()
/*
	enter keywords into keyword table for use by lex()
	and into keyword representation table used for output
*/
{
	ktbl = new table(KTBLSIZE,0,0);

	new_key("asm",ASM,0);
	new_key("auto",AUTO,TYPE);
	new_key("break",LOC,BREAK);
	new_key("case",LOC,CASE);
	new_key("continue",LOC,CONTINUE);
	new_key("char",CHAR,TYPE);
	new_key("do",LOC,DO);
	new_key("double",DOUBLE,TYPE);
	new_key("default",LOC,DEFAULT);
	new_key("enum",ENUM,0);
	new_key("else",LOC,ELSE);
	new_key("extern",EXTERN,TYPE);
	new_key("float",FLOAT,TYPE);
	new_key("for",LOC,FOR);
//	new_key("fortran",FORTRAN,0);
	new_key("goto",LOC,GOTO);
	new_key("if",LOC,IF);
	new_key("int",INT,TYPE);
	new_key("long",LONG,TYPE);
	new_key("return",LOC,RETURN);
	new_key("register",REGISTER,TYPE);
	new_key("static",STATIC,TYPE);
	new_key("struct",STRUCT,AGGR);
	new_key("sizeof",SIZEOF,0);
	new_key("short",SHORT,TYPE);
	new_key("switch",LOC,SWITCH);
	new_key("typedef",TYPEDEF,TYPE);
	new_key("unsigned",UNSIGNED,TYPE);
	new_key("union",UNION,AGGR);
	new_key("void",VOID,TYPE);
	new_key("while",LOC,WHILE);

	new_key("class",CLASS,AGGR);
	new_key("const",CONST,TYPE);
	new_key("delete",LOC,DELETE);
	new_key("friend",FRIEND,TYPE);
	new_key("inline",INLINE,TYPE);
	new_key("new",NEW,0);
	new_key("operator",OPERATOR,0);
	new_key("overload",OVERLOAD,TYPE);
	new_key("private",PRIVATE,PR);
	new_key("protected",PROTECTED,PR);
	new_key("public",PUBLIC,PR);
	new_key("signed",SIGNED,TYPE);
	new_key("this",THIS,0);
	new_key("virtual",VIRTUAL,TYPE);
	new_key("volatile",VOLATILE,TYPE);
#ifdef DK
	new_key("or",OR,0);
	new_key("cor",OROR,0);
	new_key("and",AND,0);
	new_key("cand",ANDAND,0);
	new_key("xor",ER,0);
	new_key("compl",COMPL,0);
#endif
}

extern char* src_file_name;
extern char* line_format;
loc last_line;

void loc::putline()
{
	if (file==0 && line==0) return;
	if (0<=file && file<MAXFILE) {
		char* f = file_name[file];
		if (f==0) f = (src_file_name) ? src_file_name : "";
		fprintf(out_file,line_format,line,f);
		last_line = *this;
	}
}

void loc::put(FILE* p)
{
	if (0<=file && file<MAXFILE) {
		char* f = file_name[file];
		if (f==0) f = (src_file_name) ? src_file_name : "";
		fprintf(p,"\"%s\", line %d: ",f,line);
	}
}	

void lxenter(register char* s, short m)
/* enter a mask into lxmask */
{
	register c;

	while( c= *s++ ) lxmask[c+1] |= m;

}


void lxget(register c, register m)
/*
	put 'c' back then scan for members of character class 'm'
	terminate the string read with \0
	txtfree points to the character position after that \0
*/
{
	pch(c);
	while ( (get(c), lxmask[c+1]&m) ) pch(c);
	unget(c);
	pch('\0');
}

struct LXDOPE {
	short lxch;	/* the character */
	short lxact;	/* the action to be performed */
	TOK   lxtok;	/* the token number to be returned */
} lxdope[] = {
#ifdef apollo
	'@',	A_ERR,	0,	/* illegal characters go here... */
#else
	'$',	A_ERR,	0,	/* illegal characters go here... */
#endif
	'_',	A_LET,	0,	/* letters point here */
	'0',	A_DIG,	0,	/* digits point here */
	' ',	A_WS,	0,	/* whitespace goes here */
	'\n',	A_NL,	0,
	'"',	A_STR,	0,	/* character string */
	'\'',	A_CC,	0,	/* ASCII character constant */
	'`',	A_BCD,	0,	/* 'foreign' character constant, e.g. BCD */
	'(',	A_L,	LP,
	')',	A_R,	RP,
	'{',	A_LC,	LC,
	'}',	A_RC,	RC,
	'[',	A_1C,	LB,
	']',	A_1C,	RB,
	'*',	A_MUL,	MUL,
	'?',	A_1C,	QUEST,
	':',	A_COL,	COLON,
	'+',	A_PL,	PLUS,
	'-',	A_MIN,	MINUS,
	'/',	A_SL,	DIV,
	'%',	A_MOD,	MOD,
	'&',	A_AND,	AND,
	'|',	A_OR,	OR,
	'^',	A_ER,	ER,
	'!',	A_NOT,	NOT,
	'~',	A_1C,	COMPL,
	',',	A_1C,	CM,
	';',	A_1C,	SM,
	'.',	A_DOT,	DOT,
	'<',	A_LT,	LT,
	'>',	A_GT,	GT,
	'=',	A_ASS,	ASSIGN,
	'#',	A_SHARP, 0,
	EOF,	A_EOF,	EOFTOK
	};
/* note: EOF is used as sentinel, so must be <=0 and last entry in table */

static struct LXDOPE *lxcp[CSSZ+1];

extern void lex_init();

void lex_init()
{
	register struct LXDOPE *p;
	register i;
	register char *cp;
	/* set up character classes */

	/* first clear lexmask */
	for(i=0; i<=CSSZ; i++) lxmask[i] = 0;

#ifdef apollo
	lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_$", LEXLET );
#else
	lxenter( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_", LEXLET );
#endif
	lxenter( "0123456789", LEXDIG );
	lxenter( "0123456789abcdefABCDEF", LEXHEX );
		/* \013 should become \v someday; \013 is OK for ASCII and EBCDIC */
	lxenter( " \t\r\b\f\013", LEXWS );
	lxmask['.'+1] |= LEXDOT;

	/* make lxcp point to appropriate lxdope entry for each character */

	/* initialize error entries */

	for( i= 0; i<=CSSZ; ++i ) lxcp[i] = lxdope;

	/* make unique entries */

	for( p=lxdope; ; ++p ) {
		lxcp[p->lxch+1] = p;
		if( p->lxch < 0 ) break;
	}

	/* handle letters, digits, and whitespace */
	/* by convention, first, second, and third places */

	cp = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[1];
	cp = "123456789";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[2];
	cp = "\t\b\r\f\013";
	while( *cp ) lxcp[*cp++ + 1] = &lxdope[3];

	file_name[0] = src_file_name;
	curloc.file = 0;
	curloc.line = 1;

	ktbl_init();

	lex_clear();

	saved = lxtitle();
}

void lex_clear()
{
	txtstart = txtfree = inbuf;
}

int int_val(char hex)
{
	switch (hex) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		return hex-'0';
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return hex-'a'+10;
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
		return hex-'A'+10;
	}
}

void hex_to_oct()
/*
	\x has been seen on input (in char const or string) and \ printed
	read the following hexadecimal integer and replace it with an octal
*/
{
	int i = 0;
	int c;
	get(c);
	if (lxmask[c+1] & LEXHEX) {
		i = int_val(c);
		get(c);					// try for two
		if (lxmask[c+1] & LEXHEX) {
			i = (i<<4) + int_val(c);
			get(c);				// try for three
			if (lxmask[c+1] & LEXHEX) {
				i = (i<<4) + int_val(c);
			}
			else
				unget(c);
		}
		else
			unget(c);
	}
	else {
		error("hexadecimal digitX after \\x");
		unget(c);
	}

	if (0777 < i) error("hexadecimal constant too large after \\x");

	pch(('0'+(i>>6)));
	pch(('0'+((i&070)>>3)));
	pch(('0'+(i&7)));
}


char * chconst()
/*
	read a character constant into inbuf
*/
{
	register c;
	int nch = 0;

	pch('\'');

	for(;;) {
		if (SZ_INT < nch++) {
			error("char constant too long");
			goto ex;
		}

		switch (get(c)) {
		case '\'':
			goto ex;
		case EOF:
			error("eof in char constant");
			goto ex;
		case '\n':
			error("newline in char constant");
			goto ex;
		case '\\':
			pch(c);
			switch (get(c)){
			case '\n':
				++curloc.line;
			default:
				pch(c);
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': // octal
				pch(c);
				get(c);  /* try for 2 */
				if( lxmask[c+1] & LEXDIG && c<'8'){
					pch(c);
					get(c);  /* try for 3 */
					if (lxmask[c+1] & LEXDIG && c<'8')
						pch(c);
					else
						unget(c);
				}
				else
					unget(c);
				break;
			case 'x':	// hexadecimal
				hex_to_oct();
				break;
			};
			break;
		default:
			pch(c);
		}
	}
ex:
	pch('\'');
	pch('\0');
	return txtstart;
}

void lxcom()
/* process a "block comment" */
{
	register c;

	for(;;)
	switch (get(c)) {
	case EOF:
		error("eof in comment");
		return;
	case '\n':
		curloc.line++;
		Nline++;
		break;
	case '*':
		if (get(c) == '/') return;
		unget(c);
		break;
	case '/':
		if (get(c) == '*') error('w',"``/*'' in comment");
		unget(c);
		break;
	}
}


void linecom()
// process a "line comment"
{
	register c;

	for(;;)
	switch (get(c)) {
	case EOF:
		error("eof in comment");
		return;
	case '\n':
		curloc.line++;
		Nline++;
		saved = lxtitle();
		return;
	}
}


void tlex()
{
	TOK ret;
	Pname n;

	Ntoken++;

	for(;;) {
		register lxchar;
		register struct LXDOPE *p;

		start_txt();

		if (saved) {
			lxchar = saved;
			saved = 0;
		}
		else
			get(lxchar);

		switch( (p=lxcp[lxchar+1])->lxact ){

		case A_1C:	// eat up a single character, and return an opcode 
			reti(p->lxtok,p->lxtok);

		case A_EOF:
			if (br_level || bl_level)
				error("'%s' missing at end of input",(bl_level) ? "}" : ")");
			reti(EOFTOK,0);

		case A_SHARP:
#ifdef DK
			if (get(lxchar)==')') goto arc;
			unget(lxchar);
#endif
			// cope with header file not ended with '\n'
			unget('#');
			saved = lxtitle();
			continue;

		case A_ERR:
			if (' '<=lxchar && lxchar<='~')	// ASCII printable
				error("illegal character '%c' (ignored)",lxchar);
			else
				error("illegal character '0%o' (ignored)",lxchar);
			continue;

		case A_LET:	// collect an identifier and check for keyword
			lxget( lxchar, LEXLET|LEXDIG );

			if (n = ktbl->look(txtstart,0)) {
				TOK x;
				del_txt();
				switch (x=n->base) {
				case TNAME:
					retn(TNAME,n);
					break;
				case LOC:
					retl(n->syn_class);
				default:
#ifdef DK
					if (get(lxchar) == '=')
						switch (x) {
						case OR:	reti(ASOP,ASOR);
						case ER:	reti(ASOP,ASER);
						case AND:	reti(ASOP,ASAND);
						}
					saved = lxchar;
									
#endif
					reti(n->syn_class,x);
				}
			}
			else
				rets(ID,txtstart);

		case A_DIG:

			ret = ICON;

			if (lxchar=='0') {	/* octal or hexadecimal number */
				pch('0');
				switch (get(lxchar)) {
				case 'l':
				case 'L':
					pch('L');
					pch(0);
					rets(ICON,txtstart);
				case 'e':
				case 'E':
				//	lxget(lxchar,LEXDIG);
				//	goto getfp;
					goto getfp2;
				case 'x':
				case 'X':
					lxget('X',LEXHEX);
					if (txtfree-txtstart<4) // minimum "0Xd\0"
						error("hexadecimal digitX after \"0x\"");
					switch (get(lxchar)) {
					case 'l':
					case 'L':
						txtfree--;
						pch('L');
						pch(0);
						break;
					default:
						saved = lxchar;
					}
					rets(ICON,txtstart);
				case '8':
				case '9':
					error("%c used as octal digit",lxchar);
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
					pch(lxchar);
				ox:
					switch (get(lxchar)) {
					case '8':
					case '9':
						error("%c used as octal digit",lxchar);
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
						pch(lxchar);
						goto ox;
					case 'l':
					case 'L':
						pch('L');
						pch(0);
						break;
					default:
						pch(0);
						saved = lxchar;
					}
					rets(ICON,txtstart);
				case '.':
					lxget('.',LEXDIG);
					goto getfp;
				default:
					saved = lxchar;
					reti(ZERO,0);
				}
			}
			else
				lxget(lxchar,LEXDIG);

			if (get(lxchar) == '.') {
				txtfree--;
				lxget('.', LEXDIG );
		getfp:
				ret = FCON;
				get(lxchar);
			};
		
			switch (lxchar) {
			case 'e':
			case 'E':
				txtfree--;
				switch (get(lxchar)) {
				case '-':
				case '+':
					pch('e');
					break;
				default:
					unget(lxchar);
					lxchar = 'e';
				};
		getfp2:
				lxget( lxchar, LEXDIG );
				ret = FCON;
				break;
			case 'l':
			case 'L':
			case 'u':
			case 'U':
				if ( ret == FCON )
					error("illegal suffix after floating point constant: %c", lxchar);
				else {
					txtfree--;
					pch(lxchar);
				}
				switch (get(lxchar)) {	// ul, Lu, etc.
				case 'l':
				case 'L':
				case 'u':
				case 'U':
				if ( ret == FCON )
					error("illegal suffix after floating point constant: %c", lxchar);
				else pch(lxchar);
				break;
				default:
					saved = lxchar;
				}
				break;
			default:
				saved = lxchar;
			};

			pch(0);
			rets(ret,txtstart);

		case A_DOT:
			if (get(lxchar) == '.') {	/* look for ellipsis */
				if (get(lxchar) != '.') {
					error("token .. ?");
					saved = lxchar;
				}
				reti(ELLIPSIS,0);
			}
			if( lxmask[lxchar+1] & LEXDIG ){/* look for floating constant */
				unget(lxchar);
				lxget( '.', LEXDIG );
				goto getfp;
			}
			saved = lxchar;
			reti(DOT,0);

		case A_STR:
			/* save string constant in buffer */
			for(;;)
			switch (get(lxchar)) {
			case '\\':
				pch('\\');
				switch (get(lxchar)) {
				case '\n':
					++curloc.line;
				default:
					pch(lxchar);
					break;
				case 'x':
					hex_to_oct();
   					break;
				};
				break;
			case '"':
				pch(0);
				rets(STRING,txtstart);
			case '\n':
				error("newline in string");
				pch(0);
				rets(STRING,txtstart);
			case EOF:
				error("eof in string");
				pch(0);
				rets(STRING,txtstart);
			default:
				pch(lxchar);
			}

		case A_CC:
			/* character constant */
			rets(CCON,chconst());

		case A_BCD:
			{
				register i;
				int j;
	
				pch('`');
	
				for (i=0; i<7; ++i) {
					pch(get(j));
					if (j == '`' ) break;
				}
				pch(0);
				if (6<i)
					error("bcd constant exceeds 6 characters" );
				rets(CCON,txtstart);
			}

		case A_SL:	/* / */
			switch (get(lxchar))  {
			case '*':
				lxcom();
				break;
			case '/':
				linecom();
				break;
			case '=':
				reti(ASOP,ASDIV);
			default:
				saved = lxchar;
				reti(DIVOP,DIV);
			}

		case A_WS:
			continue;

		case A_NL:
			++curloc.line;
			Nline++;
			saved = lxtitle();
			continue;

		case A_LC:
#ifdef DK
		alc:
#endif
			if (BLMAX <= bl_level++) {
				error('s',"blocks too deeply nested");
				ext(3);
			}
			retl(LC);

		case A_RC:
#ifdef DK
		arc:
#endif
			if (bl_level-- <= 0) {
				error("unexpected '}'");
				bl_level = 0;
			}
			retl(RC);

		case A_L:
#ifdef DK
			if (get(lxchar) == '#')	// (# is {
				goto alc;
			else
				saved = lxchar;
#endif
			br_level++;
			reti(LP,0);

		case A_R:
			if (br_level-- <= 0) {
				error("unexpected ')'");
				br_level = 0;
			}
			reti(RP,0);

		case A_ASS:
			switch (get(lxchar)) {
			case '=':
				reti(EQUOP,EQ);
			default:
				saved = lxchar;
				reti(ASSIGN,ASSIGN);
			}

		case A_COL:
			switch (get(lxchar)) {
			case ':':
				reti(MEM,0);
			case '=':
				error("':=' is not a c++ operator");
				reti(ASSIGN,ASSIGN);
			default:
				saved = lxchar;
				reti(COLON,COLON);
			}
		case A_NOT:
			switch (get(lxchar)) {
			case '=':
				reti(EQUOP,NE);
			default:
				saved = lxchar;
				reti(NOT,NOT);
			}
		case A_GT:
			switch(get(lxchar)) {
			case '>':
				switch (get(lxchar)) {
				case '=':
					reti(ASOP,ASRS);
					break;
				default:
					saved = lxchar;
					reti(SHIFTOP,RS);
				}
			case '=':
				reti(RELOP,GE);
			default:
				saved = lxchar;
				reti(RELOP,GT);
			}
		case A_LT:
			switch (get(lxchar)) {
			case '<':
				switch (get(lxchar)) {
				case '=':
					reti(ASOP,ASLS);
				default:
					saved = lxchar;
					reti(SHIFTOP,LS);
				}
			case '=':
				reti(RELOP,LE);
			default:
				saved = lxchar;
				reti(RELOP,LT);
			}
		case A_AND:
			switch (get(lxchar)) {
			case '&':
				reti(ANDAND,ANDAND);
			case '=':
				reti(ASOP,ASAND);
			default:
				saved = lxchar;
				reti(AND,AND);
			}
		case A_OR:
			switch (get(lxchar)) {
			case '|':
				reti(OROR,OROR);
			case '=':
				reti(ASOP,ASOR);
			default:
				saved = lxchar;
				reti(OR,OR);
			}
		case A_ER:
			switch (get(lxchar)) {
			case '=':
				reti(ASOP,ASER);
			default:
				saved = lxchar;
				reti(ER,ER);
			}
		case A_PL:
			switch (get(lxchar)) {
			case '=':
				reti(ASOP,ASPLUS);
			case '+':
				reti(ICOP,INCR);
			default:
				saved = lxchar;
				reti(PLUS,PLUS);
			}
		case A_MIN:
			switch (get(lxchar)) {
			case '=':
				reti(ASOP,ASMINUS);
			case '-':
				reti(ICOP,DECR);
			case '>':
				reti(REF,REF);
			default:
				saved = lxchar;
				reti(MINUS,MINUS);
			}
		case A_MUL:
			switch (get(lxchar)) {
			case '=':
				reti(ASOP,ASMUL);
			case '/':
				error('w',"*/ not as end of comment");
			default:
				saved = lxchar;
				reti(MUL,MUL);
			}
		case A_MOD:
			switch (get(lxchar)) {
			case '=':
				reti(ASOP,ASMOD);
			default:
				saved = lxchar;
				reti(DIVOP,MOD);
			}
		default:
			error('i',"lex act==%d getc()->%d",p,lxchar);

		}

		error('i',"lex, main switch");

	}

}

int lxtitle()
/*
	called after a newline; set linenumber and file name
*/
{
	register c;

	for(;;)
	switch ( get(c) ) {
	default:
		return c;
/*	case EOF:
		return EOF;	*/
	case '\n':
		curloc.line++;
		Nline++;
		break;
	ll:
		break;
	case '#': 	/* # lineno "filename" */
		curloc.line = 0;
		for(;;) 
		switch (get(c)) {
		case '"':
			start_txt();
			for(;;)
			switch (get(c)) {
			case '"':
				pch('\0');
				while (get(c) != '\n') ;	// skip to eol
				if (*txtstart) {	// stack file name
					if (curr_file == 0) goto push;

					char* fn;

					if ( (fn=file_name[file_stack[curr_file]])
					&& (strcmp(txtstart,fn)==0) ) {
						// same file: ignore
					}
					else if ( (fn=file_name[file_stack[curr_file-1]])
					&& (strcmp(txtstart,fn)==0) ) {
						// previous file: pop
						curr_file--;
					}
					else {	// new file name: push
				push:
						if (MAXFILE*4<Nfile++) error('i',"fileN buffer overflow");
						if (MAXFILE<curr_file++) error('i',"fileN stack overflow");
						file_stack[curr_file] = Nfile;
					
						char* p = new char[txtfree-txtstart];
						(void) strcpy(p,txtstart);
						file_name[Nfile] = p;
						Nstr++;
					}
				}
				else {	// back to the original .c file: ""
					curr_file = 0;
				}
				del_txt();
				curloc.file = file_stack[curr_file];
				goto ll;
			case '\n':
				error("unexpected end of line on '# line'");
			default:
				pch(c);
			}
		case ' ':
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			curloc.line = curloc.line*10+c-'0'; 
			break;
		default:	/* pass #rubbish through */
			pch('#');
			pch(c);
			while (get(c) != '\n') pch(c);
			pch('\0');
			fprintf(out_file,"\n%s\n",txtstart);
			start_txt();
			curloc.line++;
			Nline++;
			goto ll;
		case '\n':
			curloc.putline();
			goto ll;
		}
	}
} 
