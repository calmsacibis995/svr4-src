/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cscope:common/input.c	1.4"
/*	cscope - interactive C symbol cross-reference
 *
 *	terminal input functions
 */

#include "global.h"
#include <curses.h>
#include <setjmp.h>	/* jmp_buf */

static	jmp_buf	env;		/* setjmp/longjmp buffer */
static	int	prevchar;	/* previous, ungotten character */

/* catch the interrupt signal */

/*ARGSUSED*/
SIGTYPE
catchint(sig)
{
	signal(SIGINT, catchint);
	longjmp(env, 1);
}

/* unget a character */

int
ungetch(c)
{
	prevchar = c;
}

/* get a character from the terminal */

int
mygetch()
{
	SIGTYPE	(*savesig)();		/* old value of signal */
	int	c;

	/* change an interrupt signal to a break key character */
	if (setjmp(env) == 0) {
		savesig = signal(SIGINT, catchint);
		refresh();	/* update the display */
		mousereinit();	/* curses can change the menu number */
		if(prevchar) {
			c = prevchar;
			prevchar = 0;
		}
		else
			c = getch();	/* get a character from the terminal */
	}
	else {	/* longjmp to here from signal handler */
		c = KEY_BREAK;
	}
	signal(SIGINT, savesig);
	return(c);
}

/* get a line from the terminal in non-canonical mode */

getline(s, size, firstchar, iscaseless)
register char	s[];
unsigned size;
int	firstchar;
BOOL iscaseless;
{
	register int	c, i = 0;
	int	j;

	/* if a character already has been typed */
	if (firstchar != '\0') {
		if(iscaseless == YES) {
			firstchar = tolower(firstchar);
		}
		addch(firstchar);	/* display it */
		s[i++] = firstchar;	/* save it */
	}
	/* until the end of the line is reached */
	while ((c = mygetch()) != '\r' && c != '\n' && c != KEY_ENTER) {
		if (c == erasechar() || c == KEY_BACKSPACE) {	/* erase */
			if (i > 0) {
				addstr("\b \b");
				--i;
			}
		}
		else if (c == killchar() || c == KEY_BREAK) {	/* kill */
			for (j = 0; j < i; ++j) {
				addch('\b');
			}
			for (j = 0; j < i; ++j) {
				addch(' ');
			}
			for (j = 0; j < i; ++j) {
				addch('\b');
			}
			i = 0;
		}
		else if (isprint(c) || c == '\t') {		/* printable */
			if(iscaseless == YES) {
				c = tolower(c);
			}
			/* if it will fit on the line */
			if (i < size) {
				addch(c);	/* display it */
				s[i++] = c;	/* save it */
			}
		}
#if UNIXPC
		else if (unixpcmouse == YES && c == ESC) {	/* mouse */
			(void) getmouseaction(ESC);	/* ignore it */
		}
#endif
		else if (mouse == YES && c == ctrl('X')) {
			(void) getmouseaction(ctrl('X'));	/* ignore it */
		}
		else if (c == EOF) {				/* end-of-file */
			break;
		}
		/* return on an empty line to allow a command to be entered */
		if (firstchar != '\0' && i == 0) {
			break;
		}
	}
	s[i] = '\0';
	return(i);
}

/* ask user to enter a character after reading the message */

void
askforchar()
{
	addstr("Type any character to continue: ");
	(void) mygetch();
}

/* ask user to press the RETURN key after reading the message */

void
askforreturn()
{
	(void) fprintf(stderr, "Press the RETURN key to continue: ");
	(void) getchar();
}

/* expand the ~ and $ shell meta characters in a path */

void
shellpath(out, limit, in) 
register char	*out;
int	limit; 
register char	*in;
{
	register char	*lastchar;
	char	*s, *v;

	/* skip leading white space */
	while (isspace(*in)) {
		++in;
	}
	lastchar = out + limit - 1;

	/* a tilde (~) by itself represents $HOME; followed by a name it
	   represents the $LOGDIR of that login name */
	if (*in == '~') {
		*out++ = *in++;	/* copy the ~ because it may not be expanded */

		/* get the login name */
		s = out;
		while (s < lastchar && *in != '/' && *in != '\0' && !isspace(*in)) {
			*s++ = *in++;
		}
		*s = '\0';

		/* if the login name is null, then use $HOME */
		if (*out == '\0') {
			v = getenv("HOME");
		}
		else {	/* get the home directory of the login name */
			v = logdir(out);
		}
		/* copy the directory name */
		if (v != NULL) {
			(void) strcpy(out - 1, v);
			out += strlen(v) - 1;
		}
		else {	/* login not found, so ~ must be part of the file name */
			out += strlen(out);
		}
	}
	/* get the rest of the path */
	while (out < lastchar && *in != '\0' && !isspace(*in)) {

		/* look for an environment variable */
		if (*in == '$') {
			*out++ = *in++;	/* copy the $ because it may not be expanded */

			/* get the variable name */
			s = out;
			while (s < lastchar && *in != '/' && *in != '\0' &&
			    !isspace(*in)) {
				*s++ = *in++;
			}
			*s = '\0';
	
			/* get its value */
			if ((v = getenv(out)) != NULL) {
				(void) strcpy(out - 1, v);
				out += strlen(v) - 1;
			}
			else {	/* var not found, so $ must be part of the file name */
				out += strlen(out);
			}
		}
		else {	/* ordinary character */
			*out++ = *in++;
		}
	}
	*out = '\0';
}
