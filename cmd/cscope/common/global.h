/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/global.h	1.5"
/*	cscope - interactive C symbol cross-reference
 *
 *	global type, data, and function definitions
 */

#include <ctype.h>	/* isalpha, isdigit, etc. */
#include <signal.h>	/* SIGINT and SIGQUIT */
#include <stdio.h>	/* standard I/O package */
#include <string.h>	/* string functions */
#include "constants.h"	/* misc. constants */
#include "library.h"	/* library function return values */
#ifdef CCS
#include "ccstypes.h"	/* pid_t */
#else
typedef	int	pid_t;
#endif

#if SVR2 || BSD && !sun
#define SIGTYPE int
#else
#define SIGTYPE void
#endif

typedef	enum	{		/* boolean data type */
	NO,
	YES
} BOOL;

typedef	enum	{		/* findinit return code */
	NOERROR,
	NOTSYMBOL,
	REGCMPERROR
} FINDINIT;

typedef	struct {		/* mouse action */
	int	button;
	int	percent;
	int	x1;
	int	y1;
	int	x2;
	int	y2;
} MOUSE;

struct cmd {			/* command history struct */
	struct	cmd *prev, *next;	/* list ptrs */
	int	field;			/* input field number */
	char	*text;			/* input field text */
};

/* digraph data for text compression */
extern	char	dichar1[];	/* 16 most frequent first chars */
extern	char	dichar2[];	/* 8 most frequent second chars 
				   using the above as first chars */
extern	char	dicode1[];	/* digraph first character code */
extern	char	dicode2[];	/* digraph second character code */

/* main.c global data */
extern	char	*editor, *home, *shell;	/* environment variables */
extern	char	*argv0;		/* command name */
extern	BOOL	compress;	/* compress the characters in the crossref */
extern	int	dispcomponents;	/* file path components to display */
#if CCS
extern	BOOL	displayversion;	/* display the C Compilation System version */
#endif
extern	BOOL	editallprompt;	/* prompt between editing files */
extern	int	fileargc;	/* file argument count */
extern	char	**fileargv;	/* file argument values */
extern	int	fileversion;	/* cross-reference file version */
extern	BOOL	incurses;	/* in curses */
extern	BOOL	isuptodate;	/* consider the crossref up-to-date */
extern	BOOL	linemode;	/* use line oriented user interface */
extern	char	*namefile;	/* file of file names */
extern	char	*newreffile;	/* new cross-reference file name */
extern	FILE	*newrefs;	/* new cross-reference */
extern	BOOL	ogs;		/* display OGS book and subsystem names */
extern	char	*prependpath;	/* prepend path to file names */
extern	char	*reffile;	/* cross-reference file path name */
extern	FILE	*refsfound;	/* references found file */
extern	int	symrefs;	/* cross-reference file */
extern	char	temp1[];	/* temporary file name */
extern	char	temp2[];	/* temporary file name */
extern	BOOL	truncate;	/* truncate symbols to 8 characters */


/* command.c global data */
extern	BOOL	caseless;	/* ignore letter case when searching */
extern	BOOL	*change;	/* change this line */
extern	BOOL	changing;	/* changing text */
extern	char	newpat[];	/* new pattern */
extern	char	pattern[];	/* symbol or text pattern */

/* crossref.c global data */
extern	BOOL	errorsfound;	/* prompt before clearing error messages */
extern	int	symbols;	/* number of symbols */

/* dir.c global data */
extern	char	currentdir[];	/* current directory */
extern	char	**incdirs;	/* #include directories */
extern	char	**srcdirs;	/* source directories */
extern	char	**srcfiles;	/* source files */
extern	int	nincdirs;	/* number of #include directories */
extern	int	nsrcdirs;	/* number of source directories */
extern	int	nsrcfiles;	/* number of source files */
extern	int	msrcfiles;	/* maximum number of source files */

/* display.c global data */
extern	int	*displine;	/* screen line of displayed reference */
extern	int	disprefs;	/* displayed references */
extern	int	field;		/* input field */
extern	unsigned fldcolumn;	/* input field column */
extern	int	mdisprefs;	/* maximum displayed references */
extern	int	nextline;	/* next line to be shown */
extern	int	topline;	/* top line of page */
extern	int	bottomline;	/* bottom line of page */
extern	int	totallines;	/* total reference lines */

/* find.c global data */
extern	char	block[];	/* cross-reference file block */
extern	char	blockmark;	/* mark character to be searched for */
extern	char	*blockp;	/* pointer to current character in block */
extern	int	blocklen;	/* length of disk block read */

/* lookup.c global data */
extern	struct	keystruct {
	char	*text;
	char	delim;
	struct	keystruct *next;
} keyword[];

/* mouse.c global data */
extern	BOOL	mouse;		/* mouse interface */

#if UNIXPC
extern	BOOL	unixpcmouse;		/* UNIX PC mouse interface */
#endif

/* scanner.l global data */
extern	int	first;		/* buffer index for first char of symbol */
extern	int	last;		/* buffer index for last char of symbol */
extern	int	lineno;		/* symbol line number */
extern	FILE	*yyin;		/* input file descriptor */
extern	int	yyleng;		/* input line length */
extern	int	yylineno;	/* input line number */
extern	char	yytext[];	/* input line text */

/* cscope functions called from more than one function or between files */ 
char	*filepath(), *findregexp(), *inviewpath(), *lcasify(), *lookup(), 
	*readblock(), *scanpast();
void	addsrcfile(), askforchar(), askforreturn(), atfield(), build(), 
	clearmsg2(), display(), drawscrollbar(), edit(), findcleanup(), 
	freefilelist(), incfile(), makefilelist(), mousecleanup(), mousemenu(),
	mouseinit(), mousereinit(), myexit(), myperror(), progress(), 
	putfilename(), putmsg(), putmsg2(), putstring(), rebuild(), seekline(),
	setfield(), shellpath(), warning();
BOOL	infilelist(), search();
FINDINIT findinit();
MOUSE	*getmouseaction();
struct	cmd *currentcmd(), *prevcmd(), *nextcmd();
