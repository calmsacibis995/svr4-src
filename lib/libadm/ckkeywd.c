/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:ckkeywd.c	1.1.3.1"

#include <stdio.h>
#include <string.h>

extern int	ckquit;

extern void	puterror(),
		puthelp(),
		putprmpt();
extern int	getinput();

static int	match();

int
ckkeywd(strval, keyword, defstr, error, help, prompt)
char *strval, *keyword[];
char *defstr, *error, *help, *prompt;
{
	int valid, i, n;
	char input[128];
	char defmesg[128];
	char *ept;

	(void) sprintf(defmesg, "Please enter one of the following keywords: ");
	ept = defmesg + strlen(defmesg);
	for(i=0; keyword[i]; ) {
		if(i)
			(void) strcat(ept, ", ");
		(void) strcat(ept, keyword[i++]);
	}
	(void) strcat(ept, ckquit ? ", q." : ".");

	if(!prompt)
		prompt = "Enter appropriate value";

start:
	putprmpt(stderr, prompt, keyword, defstr);
	if(getinput(input))
		return(1);

	n = strlen(input);
	if(n == 0) {
		if(defstr) {
			(void) strcpy(strval, defstr);
			return(0);
		}
		puterror(stderr, defmesg, error);
		goto start;
	}
	if(!strcmp(input, "?")) {
		puthelp(stderr, defmesg, help);
		goto start;
	}
	if(ckquit && !strcmp(input, "q")) {
		(void) strcpy(strval, input);
		return(3);
	}

	valid = 1;
	if(keyword)
		valid = !match(input, keyword);

	if(!valid) {
		puterror(stderr, defmesg, error);
		goto start;
	}
	(void) strcpy(strval, input);
	return(0);
}

static int
match(strval, set)
char *set[], *strval;
{
	char *found;
	int i, len;

	len = strlen(strval);

	found = NULL;
	for(i=0; set[i]; i++) {
		if(!strncmp(set[i], strval, len)) {
			if(found)
				return(-1); /* not unique */
			found = set[i];
		}
	}

	if(found) {
		(void) strcpy(strval, found);
		return(0);
	}
	return(1);
}

