/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/find.c	1.4"
/*	cscope - interactive C symbol or text cross-reference
 *
 *	searching functions
 */

#include "global.h"

/* most of these functions have been optimized so their innermost loops have
 * only one test for the desired character by putting the char and 
 * an end-of-block marker (\0) at the end of the disk block buffer.
 * When the inner loop exits on the char, an outer loop will see if
 * the char is followed by a \0.  If so, it will read the next block
 * and restart the inner loop.
 */

char	*blockp;			/* pointer to current char in block */
char	block[BUFSIZ + 2];		/* leave room for end-of-block mark */
int	blocklen;			/* length of disk block read */
char	blockmark;			/* mark character to be searched for */

static	char	global[] = "<global>";	/* dummy global function name */
static	char	cpattern[PATLEN + 1];	/* compressed pattern */
static	char	*regexp;		/* regular expression */

void	putline(), putref(), putsource();
BOOL	match(), matchrest();

/* find the symbol in the cross-reference */

void
findsymbol()
{
	char	file[PATHLEN + 1];	/* source file name */
	char	function[PATLEN + 1];	/* function name */
	char	symbol[PATLEN + 1];	/* symbol name */
	FILE	*savedrefs;		/* saved references file */
	register int	c;
	register char	*cp;
	char	*s;
	char firstchar;		/* first character of a potential symbol */
	
	/* open the temporary file */
	if ((savedrefs = fopen(temp2, "w")) == NULL) {
		putmsg("Cannot open temporary file");
		return;
	}
	scanpast('\t');			/* find the end of the header */
	skiprefchar();			/* skip the file marker */
	putstring(file);		/* save the file name */
	(void) strcpy(function, global);/* set the dummy global function name */
	
	/* find the next symbol */
	/* note: this code was expanded in-line for speed */
	/* while (scanpast('\n') != NULL) { */
	/* other macros were replaced by code using cp instead of blockp */
	cp = blockp;
	for (;;) {
		setmark('\n');
		do {	/* innermost loop optimized to only one test */
			while (*cp != '\n') {
				++cp;
			}
		} while (*(cp + 1) == '\0' && (cp = readblock()) != NULL);

		/* skip the found character */
		if (cp != NULL && *(++cp + 1) == '\0') {
			cp = readblock();
		}
		if (cp == NULL) {
			break;
		}
		/* look for a source file or function name */
		if (*cp == '\t') {
			blockp = cp;
			switch (getrefchar()) {

			case NEWFILE:		/* file name */

				/* save the name */
				skiprefchar();
				putstring(file);
			
				/* check for the end of the symbols */
				if (*file == '\0') {
					goto endloop;
				}
				progress();
				/* FALLTHROUGH */
				
			case FCNEND:		/* function end */
				(void) strcpy(function, global);
				goto notmatched;	/* don't match name */
				
			case FCNDEF:		/* function name */
				s = function;
				break;
			default:		/* other symbol */
				s = symbol;
			}
			/* save the name */
			skiprefchar();
			putstring(s);

			/* see if this is a regular expression pattern */
			if (regexp != NULL) {
				if (caseless == YES) {
					s = lcasify(s);
				}
				if (*s != '\0' && regex(regexp, s) != NULL) {
					goto matched;
				}
			}
			/* match the symbol to the text pattern */
			else if (strequal(pattern, s)) {
				goto matched;
			}
			goto notmatched;
		}
		/* if this is a regular expression pattern */
		if (regexp != NULL) {
			
			/* if this is a symbol */
			
			/**************************************************
			 * The first character may be a digraph'ed char, so
			 * unpack it into firstchar, and then test that.
			 *
			 * Assume that all digraphed chars have the 8th bit
			 * set (0200).
			 **************************************************/
			if (*cp & 0200) {	/* digraph char? */
				firstchar = dichar1[(*cp & 0177) / 8];
			}
			else {
				firstchar = *cp;
			}
			
			if (isalpha(firstchar) || firstchar == '_') {
				blockp = cp;
				putstring(symbol);
				if (caseless == YES) {
					s = lcasify(symbol);	/* point to lower case version */
				}
				else {
					s = symbol;
				}
				
				/* match the symbol to the regular expression */
				if (regex(regexp, s) != NULL) {
					goto matched;
				}
				goto notmatched;
			}
		}
		/* match the character to the text pattern */
		else if (*cp == cpattern[0]) {
			blockp = cp;

			/* match the rest of the symbol to the text pattern */
			if (matchrest()) {
		matched:
				/* output the file, function and source line */
				/* put global references first */
				if (strcmp(function, global) == 0) {
					putref(refsfound, file, function);
				}
				else {
					putref(savedrefs, file, function);
				}
				if (blockp == NULL) {
					goto endloop;
				}
			}
		notmatched:
			cp = blockp;
		}
	}
	blockp = cp;
endloop:	
	/* append the saved references */
	(void) freopen(temp2, "r", savedrefs);
	while ((c = getc(savedrefs)) != EOF) {
		(void) putc(c, refsfound);
	}
	(void) fclose(savedrefs);
}
/* find the function definition or #define */

void
finddef()
{
	char	file[PATHLEN + 1];	/* source file name */

	/* find the next file name or definition */
	while (scanpast('\t') != NULL) {
		switch (*blockp) {
			
		case NEWFILE:
			skiprefchar();	/* save file name */
			putstring(file);
			if (*file == '\0') {	/* if end of symbols */
				return;
			}
			progress();
			break;

		case DEFINE:		/* could be a macro */
		case FCNDEF:
		case CLASSDEF:
		case ENUMDEF:
		case MEMBERDEF:
		case STRUCTDEF:
		case TYPEDEF:
		case UNIONDEF:
		case GLOBALDEF:		/* other global definition */
			skiprefchar();	/* match name to pattern */
			if (match()) {
		
				/* output the file, function and source line */
				putref(refsfound, file, pattern);
			}
			break;
		}
	}
}
/* find all function definitions (used by samuel only) */

void
findallfcns()
{
	char	file[PATHLEN + 1];	/* source file name */
	char	function[PATLEN + 1];	/* function name */

	/* find the next file name or definition */
	while (scanpast('\t') != NULL) {
		switch (*blockp) {
			
		case NEWFILE:
			skiprefchar();	/* save file name */
			putstring(file);
			if (*file == '\0') {	/* if end of symbols */
				return;
			}
			progress();
			/* FALLTHROUGH */
			
		case FCNEND:		/* function end */
			(void) strcpy(function, global);
			break;

		case FCNDEF:
		case CLASSDEF:
			skiprefchar();	/* save function name */
			putstring(function);

			/* output the file, function and source line */
			putref(refsfound, file, function);
			break;
		}
	}
}
/* find the functions called by this function */

void
calledby()
{
	char	file[PATHLEN + 1];	/* source file name */
	BOOL	macro = NO;

	/* find the function definition(s) */
	while (scanpast('\t') != NULL) {
		switch (*blockp) {
			
		case NEWFILE:
			skiprefchar();	/* save file name */
			putstring(file);
			if (*file == '\0') {	/* if end of symbols */
				return;
			}
			progress();
			break;

		case DEFINE:		/* could be a macro */
			if (fileversion < 10) {
				break;
			}
			macro = YES;
			/* FALLTHROUGH */

		case FCNDEF:
			skiprefchar();	/* match name to pattern */
			if (match()) {
	
				/* find the next function call or the end of this function */
				while (scanpast('\t') != NULL) {
					switch (*blockp) {
					
					case DEFINE:		/* #define inside a function */
						if (fileversion >= 10) {	/* skip it */
							while (scanpast('\t') != NULL &&
							    *blockp != DEFINEEND) 
								;
						}
						break;
					
					case FCNCALL:		/* function call */
		
						/* output the file name */
						(void) fprintf(refsfound, "%s ", file);
		
						/* output the function name */
						skiprefchar();
						putline(refsfound);
						(void) putc(' ', refsfound);
		
						/* output the source line */
						putsource(refsfound);
						break;

					case DEFINEEND:		/* #define end */
						if (macro == YES) {
							goto next;
						}
						break;	/* inside a function */

					case FCNEND:		/* function end */
					case FCNDEF:		/* function end (pre 9.5) */
					case NEWFILE:		/* file end */
						goto next;
					}
				}
			}
		next:	
			break;
		}
	}
}
/* find the functions calling this function */

void
calling()
{
	char	file[PATHLEN + 1];	/* source file name */
	char	function[PATLEN + 1];	/* function name */
	char	macro[PATLEN + 1];	/* macro name */

	/* find the next file name or function definition */
	*macro = '\0';	/* a macro can be inside a function, but not vice versa */
	while (scanpast('\t') != NULL) {
		switch (*blockp) {
			
		case NEWFILE:		/* save file name */
			skiprefchar();
			putstring(file);
			if (*file == '\0') {	/* if end of symbols */
				return;
			}
			progress();
			(void) strcpy(function, global);
			break;
			
		case DEFINE:		/* could be a macro */
			if (fileversion >= 10) {
				skiprefchar();
				putstring(macro);
			}
			break;

		case DEFINEEND:
			*macro = '\0';
			break;

		case FCNDEF:		/* save calling function name */
			skiprefchar();
			putstring(function);
			break;
			
		case FCNCALL:		/* match function called to pattern */
			skiprefchar();
			if (match()) {
				
				/* output the file, calling function or macro, and source */
				if (*macro != '\0') {
					putref(refsfound, file, macro);
				}
				else {
					putref(refsfound, file, function);
				}
			}
		}
	}
}

/* find the text in the source files */

char *
findstring()
{
	char	egreppat[2 * PATLEN];
	char	*cp, *pp;

	/* translate special characters in the regular expression */
	cp = egreppat;
	for (pp = pattern; *pp != '\0'; ++pp) {
		if (strchr(".*[\\^$+?|()", *pp) != NULL) {
			*cp++ = '\\';
		}
		*cp++ = *pp;
	}
	*cp = '\0';
	
	/* search the source files */
	return(findregexp(egreppat));
}

/* find this regular expression in the source files */

char *
findregexp(egreppat)
char	*egreppat;
{
	register int	i;
	char	*egreperror;

	/* compile the pattern */
	if ((egreperror = egrepinit(egreppat)) == NULL) {

		/* search the files */
		for (i = 0; i < nsrcfiles; ++i) {
			register char *file = filepath(srcfiles[i]);
			progress();
			if (egrep(file, refsfound, "%s <unknown> %ld ") < 0) {
				move(1, 0);
				clrtoeol();
				printw("Cannot open file %s", file);
				refresh();
			}
		}
	}
	return(egreperror);
}

/* find matching file names */

void
findfile()
{
	register int	i;
	
	for (i = 0; i < nsrcfiles; ++i) {
		char *s;
		if (caseless == YES) {
			s = lcasify(srcfiles[i]);
		}
		else {
			s = srcfiles[i];
		}
		if (regex(regexp, s) != NULL) {
			(void) fprintf(refsfound, "%s <unknown> 1 <unknown>\n", 
				srcfiles[i]);
		}
	}
}

/* find files #including this file */

void
findinclude()
{
	char	file[PATHLEN + 1];	/* source file name */

	/* find the next file name or function definition */
	while (scanpast('\t') != NULL) {
		switch (*blockp) {
			
		case NEWFILE:		/* save file name */
			skiprefchar();
			putstring(file);
			if (*file == '\0') {	/* if end of symbols */
				return;
			}
			progress();
			break;
			
		case INCLUDE:		/* match function called to pattern */
			skiprefchar();
			skiprefchar();	/* skip global or local #include marker */
			if (match()) {
				
				/* output the file and source line */
				putref(refsfound, file, global);
			}
		}
	}
}

/* initialize */

FINDINIT
findinit()
{
	char	buf[PATLEN + 3];
	BOOL	isregexp = NO;
	register int	i;
	register unsigned int c;
	register char	*s;

	/* remove trailing white space */
	for (s = pattern + strlen(pattern) - 1; isspace(*s); --s) {
		*s = '\0';
	}
	/* allow a partial match for a file name */
	if (field == FILENAME) {
		if ((regexp = regcmp(pattern, (char *) NULL)) == NULL) {
			return(REGCMPERROR);
		}
		return(NOERROR);
	}
	/* see if the pattern is a regular expression */
	if (strpbrk(pattern, "^.[{*+$") != NULL) {
		isregexp = YES;
	}
	/* else if searching for a C symbol */
	else if (field <= CALLING) {
		
		/* check for a valid C symbol */
		s = pattern;
		if (!isalpha(*s) && *s != '_') {
			return(NOTSYMBOL);
		}
		while (*++s != '\0') {
			if (!isalnum(*s) && *s != '_') {
				return(NOTSYMBOL);
			}
		}
	}
	/* if this is a regular expression or letter case is to be ignored */
	if (isregexp == YES || caseless == YES) {

		/* remove a leading ^ */
		s = pattern;
		if (*s == '^') {
			++s;
		}
		/* remove a trailing $ */
		i = strlen(s) - 1;
		if (s[i] == '$') {
			s = strcpy(newpat, s);
			s[i] = '\0';
		}
		/* if requested, try to truncate a C symbol pattern */
		if (truncate == YES && field <= CALLING && strpbrk(s, "[{*+") == NULL) {
			s[8] = '\0';
		}
		/* must be an exact match */
		/* note: regcmp doesn't recognize ^*keypad$ as an syntex error
		   unless it is given as a single arg */
		(void) sprintf(buf, "^%s$", s);
		if ((regexp = regcmp(buf, (char *) NULL)) == NULL) {
			return(REGCMPERROR);
		}
	}
	else {
		/* if requested, truncate a C symbol pattern */
		if (truncate == YES && field <= CALLING) {
			pattern[8] = '\0';
		}
		/* compress the string pattern for matching */
		s = cpattern;
		for (i = 0; (c = (unsigned)pattern[i]) != '\0'; ++i) {
			if (dicode1[c] && dicode2[(unsigned)pattern[i + 1]]) {
				c = (0200 - 2) + dicode1[c] + dicode2[(unsigned)pattern[i + 1]];
				++i;
			}
			*s++ = c;
		}
		*s = '\0';
	}
	return(NOERROR);
}

void
findcleanup()
{
	/* discard any regular expression */
	if (regexp != NULL) {
		free(regexp);
		regexp = NULL;
	}
}

/* match the pattern to the string */

BOOL
match()
{
	char	string[PATLEN + 1];

	/* see if this is a regular expression pattern */
	if (regexp != NULL) {
		putstring(string);
		if (*string == '\0') {
			return(NO);
		}
		if (caseless == YES) {
			return(regex(regexp, lcasify(string)) ? YES : NO);
		}
		else {
			return(regex(regexp, string) ? YES : NO);
		}
	}
	/* it is a string pattern */
	return((BOOL) (*blockp == cpattern[0] && matchrest()));
}

/* match the rest of the pattern to the name */

BOOL
matchrest()
{
	int	i = 1;
	
	skiprefchar();
	do {
		while (*blockp == cpattern[i]) {
			++blockp;
			++i;
		}
	} while (*(blockp + 1) == '\0' && readblock() != NULL);
	
	if (*blockp == '\n' && cpattern[i] == '\0') {
		return(YES);
	}
	return(NO);
}

/* put the reference into the file */

void
putref(output, file, function)
FILE	*output;
char	*file;
char	*function;
{
	(void) fprintf(output, "%s %s ", file, function);
	putsource(output);
}

/* put the source line into the file */

void
putsource(output)
FILE	*output;
{
	register char	*cp, nextc = '\0';
	
	if (fileversion <= 5) {
		scanpast(' ');
		putline(output);
		(void) putc('\n', output);
		return;
	}
	/* scan back to the beginning of the source line */
	cp = blockp;
	while (*cp != '\n' || nextc != '\n') {
		nextc = *cp;
		if (--cp < block) {
			
			/* read the previous block */
			if (lseek(symrefs, (long) -(BUFSIZ + blocklen), 1) == -1) {
				myperror("lseek failed");
				sleep(3);
			}
			readblock();
			cp = &block[BUFSIZ - 1];
		}
	}
	blockp = cp;
	skiprefchar();	/* skip the double newline */
	skiprefchar();
	
	/* until a double newline is found */
	do {
		/* skip a symbol type */
		if (*blockp == '\t') {
			skiprefchar();
			skiprefchar();
		}
		/* output a piece of the source line */
		putline(output);
	} while (blockp != NULL && getrefchar() != '\n');
	(void) putc('\n', output);
}

/* put the rest of the cross-reference line into the file */

void
putline(output)
FILE	*output;
{
	register char	*cp;
	register unsigned c;
	
	setmark('\n');
	cp = blockp;
	do {
		while ((c = (unsigned)(*cp)) != '\n') {
			
			/* check for a compressed digraph */
			if (c > '\177') {
				c &= 0177;
				(void) putc(dichar1[c / 8], output);
				(void) putc(dichar2[c & 7], output);
			}
			/* check for a compressed keyword */
			else if (c < ' ') {
				fputs(keyword[c].text, output);
				if (keyword[c].delim != '\0') {
					(void) putc(' ', output);
				}
				if (keyword[c].delim == '(') {
					(void) putc('(', output);
				}
			}
			else {
				(void) putc((int) c, output);
			}
			++cp;
		}
	} while (*(cp + 1) == '\0' && (cp = readblock()) != NULL);
	blockp = cp;
}

/* put the rest of the cross-reference line into the string */

void
putstring(s)
register char	*s;
{
	register char	*cp;
	register unsigned c;
	
	setmark('\n');
	cp = blockp;
	do {
		while ((c = (unsigned)(*cp)) != '\n') {
			if (c > '\177') {
				c &= 0177;
				*s++ = dichar1[c / 8];
				*s++ = dichar2[c & 7];
			}
			else {
				*s++ = c;
			}
			++cp;
		}
	} while (*(cp + 1) == '\0' && (cp = readblock()) != NULL);
	blockp = cp;
	*s = '\0';
}
/* scan past the next occurence of this character in the cross-reference */

char	*
scanpast(c)
register char	c;
{
	register char	*cp;
	
	setmark(c);
	cp = blockp;
	do {	/* innermost loop optimized to only one test */
		while (*cp != c) {
			++cp;
		}
	} while (*(cp + 1) == '\0' && (cp = readblock()) != NULL);
	blockp = cp;
	if (cp != NULL) {
		skiprefchar();	/* skip the found character */
	}
	return(blockp);
}

/* read a block of the cross-reference */

char	*
readblock()
{
	/* read the next block */
	blocklen = read(symrefs, block, BUFSIZ);
	blockp = block;
	
	/* add the search character and end-of-block mark */
	block[blocklen] = blockmark;
	block[blocklen + 1] = '\0';
	
	/* return NULL on end-of-file */
	if (blocklen == 0) {
		blockp = NULL;
	}
	return(blockp);
}

char	*
lcasify(s)
register char	*s;
{
	static char ls[PATLEN+1];	/* largest possible match string */
	register char *lptr = ls;
	
	while(*s) {
		*lptr = tolower(*s);
		lptr++;
		s++;
	}
	*lptr = '\0';
	return(ls);
}
