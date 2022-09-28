/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/edit.c	1.2"
/*	cscope - interactive C symbol cross-reference
 *
 *	file editing functions
 */

#include "global.h"

/* edit this displayed reference */

void
editref(i)
int	i;
{
	char	file[PATHLEN + 1];	/* file name */
	char	linenum[NUMLEN + 1];	/* line number */

	/* verify that there is a references found file */
	if (refsfound == NULL) {
		return;
	}
	/* get the selected line */
	seekline(i + topline);
	
	/* get the file name and line number */
	if (fscanf(refsfound, "%s%*s%s", file, linenum) == 2) {
		edit(file, linenum);	/* edit it */
	}
	seekline(topline);	/* restore the line pointer */
}

/* edit all references */

void
editall()
{
	char	file[PATHLEN + 1];	/* file name */
	char	linenum[NUMLEN + 1];	/* line number */
	int	c;

	/* verify that there is a references found file */
	if (refsfound == NULL) {
		return;
	}
	/* get the first line */
	seekline(1);
	
	/* get each file name and line number */
	while (fscanf(refsfound, "%s%*s%s%*[^\n]", file, linenum) == 2) {
		edit(file, linenum);	/* edit it */
		if (editallprompt == YES) {
			addstr("Type ^D to stop editing all lines, or any other character to continue: ");
			if ((c = mygetch()) == EOF || c == ctrl('D') || c == ctrl('Z')) {
				break;
			}
		}
	}
	seekline(topline);
}
	
/* call the editor */

void
edit(file, linenum)
char	*file;
char	*linenum;
{
	char	msg[MSGLEN + 1];	/* message */
	char	plusnum[NUMLEN + 2];	/* line number option */
	char	*s;

	file = filepath(file);
	(void) sprintf(msg, "%s +%s %s", basename(editor), linenum, file);
	putmsg(msg);
	(void) sprintf(plusnum, "+%s", linenum);
	
	/* if this is the more or page commands */
	if (strcmp(s = basename(editor), "more") == 0 || strcmp(s, "page") == 0) {
		
		/* get it to pause after displaying a file smaller than the screen
		   length */
		execute(editor, editor, plusnum, file, "/dev/null", (char *) 0);
	}
	else {
		execute(editor, editor, plusnum, file, (char *) 0);
	}
	clear();	/* redisplay screen */
}

/* if requested, prepend a path to a relative file name */

char *
filepath(file)
char	*file;
{
	static	char	path[PATHLEN + 1];
	
	if (prependpath != NULL && *file != '/') {
		(void) sprintf(path, "%s/%s", prependpath, file);
		file = path;
	}
	return(file);
}
