/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbplot:libplot/dumb/open.c	1.1.3.1"

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved. The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Copyright (c) 1983, 1984 1985, 1986, 1987, 1988, Sun Microsystems, Inc.
 * All Rights Reserved.
 */


/*
 * This accepts plot file formats and produces the appropriate plots
 * for dumb terminals.  It can also be used for printing terminals and
 * lineprinter listings, although there is no way to specify number of
 * lines and columns different from your terminal.  This would be easy
 * to change, and is left as an exercise for the reader.
 */

#include <signal.h>
#include "dumb.h"

int minX, rangeX;	/* min and range of x */
int minY, rangeY;	/* min and range of y */
int currentx,currenty;
int COLS,LINES;

/* A very large screen! (probably should use malloc) */
char screenmat[MAXCOLS][MAXLINES];

openpl()
{
	void closepl();
	int i, j;
	char *term, *getenv();
	char bp[1024];

	term = getenv("TERM");
	tgetent(bp, term);

	COLS = tgetnum("co");
	if (COLS > MAXCOLS)
		COLS = MAXCOLS;
	if (COLS < 0)
		COLS = 48;	/* lower bound on # of cols? */
	COLS--;				/* prevent auto wrap */

	LINES = tgetnum("li");
	if (LINES > MAXLINES)
		LINES = MAXLINES;
	if (LINES < 0)
		LINES = 20;	/* lower bound on # of lines? */

	for(i=0; i<COLS; i++)
		for(j=0; j<LINES; j++)
			screenmat[i][j] = ' ';

	signal(SIGINT, closepl);
	currentx = currenty = 0;
}
