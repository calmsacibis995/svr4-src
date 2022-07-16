/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libadm:putprmpt.c	1.1.4.1"

#include <stdio.h>
#include <string.h>

extern int	ckwidth;
extern int	ckquit;
extern int	puttext();
extern void	*calloc(),
		free();

void
putprmpt(fp, prompt, choices, defstr)
FILE	*fp;
char	*prompt;
char	*choices[];
char	*defstr;
{
	char buffer[1024];
	int i, n;
	char	*tmp;
	char	empty = '\0'; /* null ptr protection */

	tmp = NULL;

	(void) fputc('\n', fp);
	(void) strcpy(buffer, prompt);
	/*
	** In the prompt case you get the default string appended
	** whether you like it or not.
	*/
	if(defstr)
		(void) sprintf(buffer+strlen(buffer), " (default:\\ %s)", 
			defstr);

	n = strlen(prompt);
	if(!n || !strchr(":?", prompt[n-1])) {
		(void) strcat(buffer, "\\ [");
		for(i=0; choices && choices[i]; ++i) {
			(void) strcat(buffer, choices[i]);
			(void) strcat(buffer, ",");
		}
		(void) strcat(buffer, ckquit ? "?,q] " : "?] ");
	} else
		(void) strcat(buffer, " ");
	/*
	** Now that the buffer is built with the default message
	** already appended. Check to see if the default message
	** should be prepended.
	*/
	n = strlen(buffer);
	if(buffer[0] == '~') {
		if(defstr == NULL)  defstr = &empty; /* null ptr protection */
		/* prepend default message */
		tmp = calloc(n+strlen(defstr), sizeof(char));
		(void) strcpy(tmp, defstr);
		(void) strcat(tmp, "\n");
		(void) strcat(tmp, &(buffer[1]));
		(void) strcpy(buffer, tmp);
		free(tmp);
	}
	(void) puttext(fp, buffer, 0, ckwidth);
}
