/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/display.c	1.6"
/*	cscope - interactive C symbol cross-reference
 *
 *	display functions
 */

#include "global.h"
#ifdef CCS
#include "sgs.h"	/* ESG_PKG and ESG_REL */
#else
#include "version.h"	/* FILEVERSION and FIXVERSION */
#endif
#include <curses.h>	/* LINES, COLS */
#include <setjmp.h>	/* jmp_buf */

int	*displine;		/* screen line of displayed reference */
int	disprefs;		/* displayed references */
int	field;			/* input field */
int	mdisprefs;		/* maximum displayed references */
int	nextline;		/* next line to be shown */
int	topline = 1;		/* top line of page */
int	bottomline;		/* bottom line of page */
int	totallines;		/* total reference lines */
unsigned fldcolumn;		/* input field column */

static	int	fldline;		/* input field line */
static	int	subsystemlen;		/* OGS subsystem name display field length */
static	int	booklen;		/* OGS book name display field length */
static	int	filelen;		/* file name display field length */
static	int	fcnlen;			/* function name display field length */
static	jmp_buf	env;			/* setjmp/longjmp buffer */
static	int	lastdispline;		/* last displayed reference line */
static	char	lastmsg[MSGLEN + 1];	/* last message displayed */
static	int	numlen;			/* line number display field length */
static	int	searchcount;		/* count of files searched */
static	char	helpstring[] = "Press the ? key for help";
static	char	selprompt[] = 
	"Select lines to change (press the ? key for help): ";

#if BSD	/* compiler bug workaround */
#define	Void	char *
#else
#define	Void	void
#endif

char	*findstring();
Void	calling(), calledby(), findallfcns(), finddef(), findfile(), findinclude(), findsymbol();

typedef char *(*FP)();	/* pointer to function returning a character pointer */

static	struct	{		/* text of input fields */
	char	*text1;
	char	*text2;
	FP	findfcn;
} fields[FIELDS + 1] = {	/* samuel has a search that is not part of the cscope display */
	"Find this", "C symbol",			(FP) findsymbol,
	"Find this", "global definition",		(FP) finddef,
	"Find", "functions called by this function",	(FP) calledby,
	"Find", "functions calling this function",	(FP) calling,
	"Find this", "text string",			findstring,
	"Change this", "text string",			findstring,
	"Find this", "egrep pattern",			findregexp,
	"Find this", "file",				(FP) findfile,
	"Find", "files #including this file",		(FP) findinclude,
	"Find all", "function definitions",		(FP) findallfcns,	/* samuel only */
};

char	*pathcomponents();
void	ogsnames();

/* initialize display parameters */

void
dispinit()
{
	/* calculate the maximum displayed reference lines */
	lastdispline = FLDLINE - 2;
	mdisprefs = lastdispline - REFLINE + 1;
	if (mdisprefs <= 0) {
		(void) fprintf(stderr, "%s: screen too small\n", argv0);
		myexit(1);
	}
	if (mouse == NO && mdisprefs > 9) {
		mdisprefs = 9;
	}
	/* allocate the displayed line array */
	displine = (int *) mymalloc(mdisprefs * sizeof(int));
}
/* display a page of the references */

void
display()
{
	char	*subsystem;		/* OGS subsystem name */
	char	*book;			/* OGS book name */
	char	file[PATHLEN + 1];	/* file name */
	char	function[PATLEN + 1];	/* function name */
	char	linenum[NUMLEN + 1];	/* line number */
	int	screenline;		/* screen line number */
	int	width;			/* source line display width */
	register int	i;
	register char	*s;

	/* see if this is the initial display */
	erase();
	if (refsfound == NULL) {
#if CCS
		if (displayversion == YES) {
			printw("cscope - %s, %s", ESG_PKG, ESG_REL);
		}
		else {
			printw("cscope");
		}
#else
		printw("Cscope version %d%s", FILEVERSION, FIXVERSION);
#endif
		move(0, COLS - (int) sizeof(helpstring));
		addstr(helpstring);
	}
	/* if no references were found */
	else if (totallines == 0) {
		
		/* redisplay the last message */
		addstr(lastmsg);
	}
	else {	/* display the pattern */
		if (changing == YES) {
			printw("Change \"%s\" to \"%s\"", pattern, newpat);
		}
		else {
			printw("%c%s: %s", toupper(fields[field].text2[0]),
				fields[field].text2 + 1, pattern);
		}
		/* display the column headings */
		move(2, 2);
		if (ogs == YES && field != FILENAME) {
			printw("%-*s ", subsystemlen, "Subsystem");
			printw("%-*s ", booklen, "Book");
		}
		if (dispcomponents > 0) {
			printw("%-*s ", filelen, "File");
		}
		if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
			printw("%-*s ", fcnlen, "Function");
		}
		if (field != FILENAME) {
			addstr("Line");
		}
		addch('\n');

		/* if at end of file go back to beginning */
		if (nextline > totallines) {
			seekline(1);
		}
		/* calculate the source text column */
		width = COLS - numlen - 3;
		if (ogs == YES) {
			width -= subsystemlen + booklen + 2;
		}
		if (dispcomponents > 0) {
			width -= filelen + 1;
		}
		if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
			width -= fcnlen + 1;
		}
		/* until the max references have been displayed or 
		   there is no more room */
		topline = nextline;
		for (disprefs = 0, screenline = REFLINE;
		    disprefs < mdisprefs && screenline <= lastdispline;
		    ++disprefs, ++screenline) {
			
			/* read the reference line */
			if (fscanf(refsfound, "%s%s%s %[^\n]", file, function, 
			    linenum, yytext) < 4) {
				break;
			}
			++nextline;
			displine[disprefs] = screenline;
			
			/* if no mouse, display the selection number */
			if (mouse == YES) {
				addch(' ');
			}
			else {
				printw("%d", disprefs + 1);
			}
			/* display any change mark */
			if (changing == YES && 
			    change[topline + disprefs - 1] == YES) {
				addch('>');
			}
			else {
				addch(' ');
			}
			/* display the file name */
			if (field == FILENAME) {
				printw("%-*s ", filelen, file);
			}
			else {
				/* if OGS, display the subsystem and book names */
				if (ogs == YES) {
					ogsnames(file, &subsystem, &book);
					printw("%-*s ", subsystemlen, subsystem);
					printw("%-*s ", booklen, book);
				}
				/* display the requested path components */
				if (dispcomponents > 0) {
					printw("%-*s ", filelen, 
						pathcomponents(file, dispcomponents));
				}
			}
			/* display the function name */
			if (field == SYMBOL || field == CALLEDBY || field == CALLING) {
				printw("%-*s ", fcnlen, function);
			}
			if (field == FILENAME) {
				addch('\n');	/* go to next line */
				continue;
			}
			/* display the line number */
			printw("%*s ", numlen, linenum);

			/* there may be tabs in egrep output */
			while ((s = strchr(yytext, '\t')) != NULL) {
				*s = ' ';
			}
			/* display the source line */
			s = yytext;
			for (;;) {
				/* see if the source line will fit */
				if ((i = strlen(s)) > width) {
					
					/* find the nearest blank */
					for (i = width; s[i] != ' ' && i > 0; --i) {
						;
					}
					if (i == 0) {
						i = width;	/* no blank */
					}
				}
				/* print up to this point */
				printw("%.*s", i, s);
				s += i;
				
				/* if line didn't wrap around */
				if (i < width) {
					addch('\n');	/* go to next line */
				}
				/* skip blanks */
				while (*s == ' ') {
					++s;
				}
				/* see if there is more text */
				if (*s == '\0') {
					break;
				}
				/* if the source line is too long */
				if (++screenline > lastdispline) {
					
					/* erase the reference */
					while (--screenline >= displine[disprefs]) {
						move(screenline, 0);
						clrtoeol();
					}
					++screenline;
					 
					/* go back to the beginning of this reference */
					--nextline;
					seekline(nextline);
					goto endrefs;
				}
				/* indent the continued source line */
				move(screenline, COLS - width);
			}

		}
	endrefs:
		/* position the cursor for the message */
		i = FLDLINE - 1;
		if (screenline < i) {
			addch('\n');
		}
		else {
			move(i, 0);
		}
		/* check for more references */
		i = totallines - nextline + 1;
		bottomline = nextline;
		if (i > 0) {
			s = "s";
			if (i == 1) {
				s = "";
			}
			printw("* %d more line%s - press the space bar to display more *", i, s);
		}
		/* if this is the last page of references */
		else if (topline > 1 && nextline > totallines) {
			addstr("* Press the space bar to display the first lines again *");
		}
	}
	/* display the input fields */
	move(FLDLINE, 0);
	for (i = 0; i < FIELDS; ++i) {
		printw("%s %s:\n", fields[i].text1, fields[i].text2);
	}
	/* display any prompt */
	if (changing == YES) {
		move(PRLINE, 0);
		addstr(selprompt);
	}
	drawscrollbar(topline, nextline);	/* display the scrollbar */
}

/* set the cursor position for the field */
void
setfield()
{
	fldline = FLDLINE + field;
	fldcolumn = strlen(fields[field].text1) + strlen(fields[field].text2) + 3;
}

/* move to the current input field */

void
atfield()
{
	move(fldline, fldcolumn);
}

/* move to the changing lines prompt */

void
atchange()
{
	move(PRLINE, (int) sizeof(selprompt) - 1);
}

/* search for the symbol or text pattern */

/*ARGSUSED*/
SIGTYPE
jumpback(sig)
{
	longjmp(env, 1);
}

BOOL
search()
{
	char	*subsystem;		/* OGS subsystem name */
	char 	*book;			/* OGS book name */
	char	file[PATHLEN + 1];	/* file name */
	char	function[PATLEN + 1];	/* function name */
	char	linenum[NUMLEN + 1];	/* line number */
	char	*egreperror = NULL;	/* egrep error message */
	FINDINIT rc = NOERROR;		/* findinit return code */
	SIGTYPE	(*savesig)();		/* old value of signal */
	FP	f;			/* searching function */
	register int	c, i;
	
	/* open the references found file for writing */
	if (refsfound == NULL) {
		if ((refsfound = fopen(temp1, "w")) == NULL) {
			putmsg("Cannot open temporary file");
			return(NO);
		}
	}
	else {
		freopen(temp1, "w", refsfound);
	}
	/* find the pattern - stop on an interrupt */
	if (linemode == NO) {
		putmsg("Searching");
	}
	searchcount = 0;
	if (setjmp(env) == 0) {
		savesig = signal(SIGINT, jumpback);
		f = fields[field].findfcn;
		if (f == findregexp || f == findstring) {
			egreperror = (*f)(pattern);
		}
		else if ((rc = findinit()) == NOERROR) {
			(void) readblock(); /* read the first database block */
			(*f)();
			findcleanup();
		}
	}
	signal(SIGINT, savesig);

	/* rewind the cross-reference file */
	lseek(symrefs, (long) 0, 0);
	
	/* reopen the references found file for reading */
	freopen(temp1, "r", refsfound);
	nextline = 1;
	totallines = 0;
	
	/* see if it is empty */
	if ((c = getc(refsfound)) == EOF) {
		if (egreperror != NULL) {
			(void) sprintf(lastmsg, "Egrep %s in this pattern: %s", 
				egreperror, pattern);
		}
		else if (rc == NOTSYMBOL) {
			(void) sprintf(lastmsg, "This is not a C symbol: %s", 
				pattern);
		}
		else if (rc == REGCMPERROR) {
			(void) sprintf(lastmsg, "Error in this regcmp(3X) regular expression: %s", 
				pattern);
			
		}
		else {
			(void) sprintf(lastmsg, "Could not find the %s: %s", 
				fields[field].text2, pattern);
		}
		return(NO);
	}
	/* put back the character read */
	(void) ungetc(c, refsfound);

	/* count the references found and find the length of the file,
	   function, and line number display fields */
	subsystemlen = 9;	/* strlen("Subsystem") */
	booklen = 4;		/* strlen("Book") */
	filelen = 4;		/* strlen("File") */
	fcnlen = 8;		/* strlen("Function") */
	numlen = 0;
	while (fscanf(refsfound, "%s%s%s", file, function, linenum) == 3) {
		if ((i = strlen(pathcomponents(file, dispcomponents))) > filelen) {
			filelen = i;
		}
		if (ogs == YES) {
			ogsnames(file, &subsystem, &book);
			if ((i = strlen(subsystem)) > subsystemlen) {
				subsystemlen = i;
			}
			if ((i = strlen(book)) > booklen) {
				booklen = i;
			}
		}
		if ((i = strlen(function)) > fcnlen) {
			fcnlen = i;
		}
		if ((i = strlen(linenum)) > numlen) {
			numlen = i;
		}
		/* skip the line text */
		while ((c = getc(refsfound)) != EOF && c != '\n') {
			;
		}
		++totallines;
	}
	rewind(refsfound);
	return(YES);
}

/* display search progress */

void
progress()
{
	static	long	start;
	long	now;
	char	msg[MSGLEN + 1];
	long	time();

	/* save the start time */
	if (searchcount == 0) {
		start = time((long *) NULL);
	}
	/* display the progress every 3 seconds */
	else if ((now = time((long *) NULL)) - start >= 3) {
		start = now;
		(void) sprintf(msg, "%d of %d files searched", 
			searchcount, nsrcfiles);
		if (linemode == NO) putmsg(msg);
	}
	++searchcount;
}

/* print error message on system call failure */

void
myperror(text) 
char	*text; 
{
	extern	int	errno, sys_nerr;
	extern	char	*sys_errlist[];
	char	msg[MSGLEN + 1];	/* message */
	register char	*s;

	s = "Unknown error";
	if (errno < sys_nerr) {
		s = sys_errlist[errno];
	}
	(void) sprintf(msg, "%s: %s", text, s);
	putmsg(msg);
}

/* putmsg clears the message line and prints the message */

void
putmsg(msg) 
char	*msg; 
{
	if (linemode == YES) {
		(void) printf("%s\n", msg);
	}
	else {
		move(MSGLINE, 0);
		clrtoeol();
		addstr(msg);
		refresh();
	}
	(void) strncpy(lastmsg, msg, sizeof(lastmsg) - 1);
}

/* clearmsg2 clears the second message line */

void
clearmsg2() 
{
	if (linemode == NO) {
		move(MSGLINE + 1, 0);
		clrtoeol();
	}
}

/* putmsg2 clears the second message line and prints the message */

void
putmsg2(msg) 
char	*msg; 
{
	if (linemode == YES) {
		(void) printf("%s\n", msg);
	}
	else {
		clearmsg2();
		addstr(msg);
	}
}
/* position references found file at specified line */

void
seekline(line) 
int	line; 
{
	register int	c;

	/* verify that there is a references found file */
	if (refsfound == NULL) {
		return;
	}
	/* go to the beginning of the file */
	rewind(refsfound);
	
	/* find the requested line */
	nextline = 1;
	while (nextline < line && (c = getc(refsfound)) != EOF) {
		if (c == '\n') {
			nextline++;
		}
	}
}

/* get the OGS subsystem and book names */

void
ogsnames(file, subsystem, book)
char	*file;
char	**subsystem;
char	**book;
{
	static	char	buf[PATHLEN + 1];
	register char	*s, *slash;

	*subsystem = *book = "";
	(void) strcpy(buf,file);
	s = buf;
	if (*s == '/') {
		++s;
	}
	while ((slash = strchr(s, '/')) != NULL) {
		*slash = '\0';
		if ((int)strlen(s) >= 3 && strncmp(slash - 3, ".ss", 3) == 0) {
			*subsystem = s;
			s = slash + 1;
			if ((slash = strchr(s, '/')) != NULL) {
				*book = s;
				*slash = '\0';
			}
			break;
		}
		s = slash + 1;
	}
}

/* get the requested path components */

char *
pathcomponents(path, components)
char	*path;
int	components;
{
	int	i;
	register char	*s;
	
	s = path + strlen(path) - 1;
	for (i = 0; i < components; ++i) {
		while (s > path && *--s != '/') {
			;
		}
	}
	if (s > path && *s == '/') {
		++s;
	}
	return(s);
}

