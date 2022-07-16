/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libadm:ckyorn.c	1.1.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

extern int	ckquit;

extern void	puterror(),
		puthelp(),
		putprmpt();
extern int	getinput();

static char *choices[] = { "y", "n", NULL };
static char *vchoices[] = { "y", "n", "yes", "no", NULL };

#define TMPSIZ	7
#define REQMSG	"Input is required."
#define ERRMSG	"Please enter yes or no."
#define HLPMSG	\
	"To respond in the affirmative, enter y, yes, Y, or YES. \
	To respond in the negative, enter n, no, N, or NO."

int
ckyorn_val(str)
char	*str;
{
	int	i;
	char	*pt,
		temp[TMPSIZ+1];

	(void) strncpy(temp, str, TMPSIZ);
	for(pt=temp; *pt; pt++) {
		if(isupper(*pt))
			*pt = tolower(*pt);
	}
	for(i=0; vchoices[i]; ) {
		if(!strcmp(temp, vchoices[i++]))
			return(0);
	}
	return(-1);
}

void
ckyorn_err(error)
char	*error;
{
	puterror(stdout, ERRMSG, error);
}

void
ckyorn_hlp(help)
char	*help;
{
	puthelp(stdout, HLPMSG, help);
}

int
ckyorn(yorn, defstr, error, help, prompt)
char *yorn, *defstr, *error, *help, *prompt;
{
	int	n;
	char	input[128];

	if(!prompt)
		prompt = "Yes or No";
start:
	putprmpt(stderr, prompt, choices, defstr);
	if(getinput(input))
		return(1);

	n = strlen(input);
	if(n == 0) {
		if(defstr) {
			(void) strcpy(yorn, defstr);
			return(0);
		}
		puterror(stderr, REQMSG, error);
		goto start;
	}
	if(!strcmp(input, "?")) {
		puthelp(stderr, HLPMSG, help);
		goto start;
	}
	if(ckquit && !strcmp(input, "q"))
		return(3);

	if(ckyorn_val(input)) {
		puterror(stderr, ERRMSG, error);
		goto start;
	}
	(void) strcpy(yorn, input);
	return(0);
}
