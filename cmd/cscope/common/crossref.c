/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/crossref.c	1.3"
/*	cscope - interactive C symbol cross-reference
 *
 *	build cross-reference file
 */

#include "global.h"

#define	SYMBOLINC	20	/* symbol list size increment */

BOOL	errorsfound;		/* prompt before clearing messages */
int	symbols;		/* number of symbols */

static	char	*filename;	/* file name for warning messages */
static	int	msymbols = SYMBOLINC;	/* maximum number of symbols */
static	struct	symbol {	/* symbol data */
	int	type;		/* type */
	int	first;		/* index of first character in text */
	int	last;		/* index of last+1 character in text */
	int	length;		/* symbol length */
} *symbol;

void
crossref(srcfile)
char	*srcfile;
{
	register int	i;
	register int	length;		/* symbol length */
	int	token;			/* current token */
	void	putcrossref(), initscanner();

	/* open the source file */
	if ((yyin = fopen(srcfile, "r")) == NULL) {
		(void) fprintf(stderr, "cscope: cannot open file %s\n", srcfile);
		errorsfound = YES;
		return;
	}
	filename = srcfile;	/* save the file name for warning messages */
	putfilename(srcfile);	/* output the file name */
	(void) putc('\n', newrefs);
	(void) putc('\n', newrefs);

	/* read the source file */
	initscanner(srcfile);
	symbols = 0;
	if (symbol == NULL) {
		symbol = (struct symbol *) mymalloc(msymbols * sizeof(struct symbol));
	}
	for (;;) {
		
		/* get the next token */
		switch (token = yylex()) {
		default:
			/* if requested, truncate C symbols */
			length = last - first;
			if (truncate == YES && length > 8 &&
			    token != INCLUDE && token != NEWFILE) {
				length = 8;
				last = first + 8;
			}
			/* see if the token has a symbol */
			if (length == 0) {
				goto save;
			}
			/* see if the symbol is already in the list */
			for (i = 0; i < symbols; ++i) {
				if (length == symbol[i].length &&
				    strncmp(yytext + first, yytext +
					symbol[i].first, length) == 0 &&
				    token == symbol[i].type) {	/* could be a::a() */
					break;
				}
			}
			if (i == symbols) {	/* if not already in list */
		save:
				/* make sure there is room for the symbol */
				if (symbols == msymbols) {
					msymbols += SYMBOLINC;
					symbol = (struct symbol *) myrealloc(symbol,
					    msymbols * sizeof(struct symbol));
				}
				/* save the symbol */
				symbol[symbols].type = token;
				symbol[symbols].first = first;
				symbol[symbols].last = last;
				symbol[symbols].length = length;
				++symbols;
			}
			break;

		case NEWLINE:	/* end of line contaning symbols */
			--yyleng;	/* remove the newline */
			putcrossref();	/* output the symbols and source line */
			lineno = yylineno;	/* save the symbol line number */
			break;
			
		case LEXEOF:	/* end of file; last line may not have \n */
			
			/* if there were symbols, output them and the source line */
			if (symbols > 0) {
				putcrossref();
			}
			(void) fclose(yyin);	/* close the source file */

			/* output the leading tab expected by the next call */
			(void) putc('\t', newrefs);
			return;
		}
	}
}

/* output the file name */

void
putfilename(srcfile)
char	*srcfile;
{
	/* check for file system out of space */
	if (putc(NEWFILE, newrefs) == EOF) {
		(void) perror("cscope: write failed");
		(void) fclose(newrefs);
		(void) unlink(newreffile);
		myexit(1);
	}
	(void) fputs(srcfile, newrefs);
}

/* output the symbols and source line */

void
putcrossref()
{
	register int	i, j;
	register unsigned int c;
	register BOOL	blank;		/* blank indicator */
	register int	symput = 0;	/* symbols output */

	/* output the source line */
	(void) fprintf(newrefs, "%d ", lineno);
	blank = NO;
	for (i = 0; i < yyleng; ++i) {
		
		/* change a tab to a blank and compress blanks */
		if ((c = (unsigned)yytext[i]) == ' ' || c == '\t') {
			blank = YES;
		}
		/* look for the start of a symbol */
		else if (symput < symbols && i == symbol[symput].first) {

			/* check for compressed blanks */
			if (blank == YES) {
				blank = NO;
				(void) putc(' ', newrefs);
			}
			(void) putc('\n', newrefs);	/* symbols start on a new line */
			
			/* output any symbol type */
			if (symbol[symput].type != IDENT) {
				(void) putc('\t', newrefs);
				(void) putc(symbol[symput].type, newrefs);
			}
			/* output the symbol */
			j = symbol[symput].last;
			for ( ; i < j; ++i) {

				/* compress digraphs */
				c = (unsigned)yytext[i];
				if (dicode1[c] && dicode2[(unsigned)(yytext[i + 1])] &&
				    i < j - 1) {
					c = (0200 - 2) + dicode1[c] + dicode2[(unsigned)(yytext[i + 1])];
					++i;
				}
				(void) putc(c, newrefs);
			}
			(void) putc('\n', newrefs);
			++symput;
			--i;
		}
		else {
			/* check for compressed blanks */
			if (blank == YES) {
				if (dicode2[c]) {
					c = (0200 - 2) + dicode1[' '] + dicode2[c];
				}
				else {
					(void) putc(' ', newrefs);
				}
			}
			/* compress digraphs */
			else if (dicode1[c] && (j = (unsigned)dicode2[(unsigned)yytext[i + 1]]) != 0 && 
			    symput < symbols && i + 1 != symbol[symput].first) {
				c = (0200 - 2) + dicode1[c] + j;
				++i;
			}
			(void) putc(c, newrefs);
			blank = NO;
			
			/* skip compressed characters */
			if (c < ' ') {
				++i;
				
				/* skip blanks before a preprocesor keyword */
				/* note: don't use isspace() because \f and \v
				   are used for keywords */
				while ((j = yytext[i]) == ' ' || j == '\t') {
					++i;
				}
				/* skip the rest of the keyword */
				while (isalpha(yytext[i])) {
					++i;
				}
				/* skip space after certain keywords */
				if (keyword[c].delim != '\0') {
					while ((j = yytext[i]) == ' ' || j == '\t') {
						++i;
					}
				}
				/* skip a '(' after certain keywords */
				if (keyword[c].delim == '(' && yytext[i] == '(') {
					++i;
				}
				--i;	/* compensate for ++i in for() */
			}
		}
	}
	/* ignore trailing blanks */
	(void) putc('\n', newrefs);
	(void) putc('\n', newrefs);

	/* output any #define end marker */
	if (symput < symbols && symbol[symput].type == DEFINEEND) {
		(void) putc('\t', newrefs);
		(void) putc(symbol[symput].type, newrefs);
		(void) putc('\n', newrefs);
	}
	symbols = 0;
}

/* print a warning message with the file name and line number */

void
warning(text, line)
char *text;
{
	(void) fprintf(stderr, "cscope: \"%s\", line %d: warning: %s\n", filename, 
		line, text);
	errorsfound = YES;
}
