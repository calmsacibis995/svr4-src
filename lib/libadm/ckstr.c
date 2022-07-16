/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:ckstr.c	1.2.3.1"

#include <stdio.h>
#include <string.h>

extern int	ckquit;

extern char	*compile();
extern void	puterror(),
		puthelp(),
		putprmpt();
extern int	getinput(),
		step();


#define ESIZE	1024

#define ERRMSG0 "Input is required."
#define ERRMSG1	"Please enter a string containing no more than %d characters."
#define ERRMSG2	\
	"Pattern matching has failed."
#define ERRMSG3 \
	"Please enter a string which contains no imbedded, \
	leading or trailing spaces or tabs."

#define HLPMSG0	"Please enter a string"
#define HLPMSG1 "Please enter a string containing no more than %d characters"
#define HLPMSG2 "matches one of the following patterns:"
#define HLPMSG3 "matches the following pattern:"
#define HLPMSG4 "contains no imbedded, leading or trailing spaces or tabs."

static char	*errstr;

static char *
sethlp(msg, regexp, length)
char	*msg;
char	*regexp[];
int	length;
{
	int	i;

	if(length)
		(void) sprintf(msg, HLPMSG1, length);
	else
		(void) strcpy(msg, HLPMSG0);

	(void) strcat(msg, length ? " and " : " which ");

	if(regexp && regexp[0]) {
		(void) strcat(msg, regexp[1] ? HLPMSG2 : HLPMSG3);
		for(i=0; regexp[i]; i++) {
			(void) strcat(msg, "\\n\\t");
			(void) strcat(msg, regexp[i]);
		}
	} else
		(void) strcat(msg, HLPMSG4);
	return(msg);
}

int
ckstr_val(regexp, length, input)
char *input, *regexp[];
int length;
{
	char	expbuf[ESIZE];
	int	i, valid;

	valid = 1;
	if(length && (strlen(input) > length)) {
		errstr = ERRMSG1;
		return(1);
	} 
	if(regexp && regexp[0]) {
		valid = 0;
		for(i=0; !valid && regexp[i]; ++i) {
			if(!compile(regexp[i], expbuf, &expbuf[ESIZE], '\0'))
				return(2);
			valid = step(input, expbuf);
		}
		if(!valid)
			errstr = ERRMSG2;
	} else if(strpbrk(input, " \t")) {
		errstr = ERRMSG3;
		valid = 0;
	}
	return(valid == 0);
}

void
ckstr_err(regexp, length, error, input)
char	*regexp[];
int	length;
char	*error, *input;
{
	char	*defhlp;
	char	temp[1024];

	if(input) {
		if(ckstr_val(regexp, length, input)) {
			(void) sprintf(temp, errstr, length);
			puterror(stdout, temp, error);
			return;
		}
	}

	defhlp = sethlp(temp, regexp, length);
	puterror(stdout, defhlp, error);
}

void
ckstr_hlp(regexp, length, help)
char	*regexp[];
int	length;
char	*help;
{
	char	*defhlp;
	char	hlpbuf[1024];

	defhlp = sethlp(hlpbuf, regexp, length);
	puthelp(stdout, defhlp, help);
}

int
ckstr(strval, regexp, length, defstr, error, help, prompt)
char *strval, *regexp[];
int length;
char *defstr, *error, *help, *prompt;
{
	int	n;
	char	*defhlp;
	char	input[128],
		hlpbuf[1024],
		errbuf[1024];

	defhlp = NULL;
	if(!prompt)
		prompt = "Enter an appropriate value";

start:
	putprmpt(stderr, prompt, NULL, defstr);
	if(getinput(input))
		return(1);

	n = strlen(input);
	if(n == 0) {
		if(defstr) {
			(void) strcpy(strval, defstr);
			return(0);
		}
		puterror(stderr, ERRMSG0, error);
		goto start;
	}
	if(!strcmp(input, "?")) {
		if(defhlp == NULL)
			defhlp = sethlp(hlpbuf, regexp, length);
		puthelp(stderr, defhlp, help);
		goto start;
	}
	if(ckquit && !strcmp(input, "q")) {
		(void) strcpy(strval, input);
		return(3);
	}
	if(ckstr_val(regexp, length, input)) {
		(void) sprintf(errbuf, errstr, length);
		puterror(stderr, errbuf, error);
		goto start;
	}
	(void) strcpy(strval, input);
	return(0);
}
