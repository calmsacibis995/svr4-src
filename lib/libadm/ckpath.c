/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libadm:ckpath.c	1.3.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "valtools.h"

extern int	ckquit;

extern void	*calloc(),
		free(),
		puthelp(),
		puterror(),
		putprmpt();
extern int	creat(),
		close(),
		getinput();

#define E_SYNTAX	"does not meet suggested filename syntax standard"
#define E_READ		"is not readable"
#define E_WRITE		"is not writable"
#define E_EXEC		"is not executable"
#define E_CREAT		"cannot be created"
#define E_ABSOLUTE	"must begin with a slash (/)"
#define E_RELATIVE	"must not begin with a slash (/)"
#define E_EXIST		"does not exist"
#define E_NEXIST	"must not already exist"
#define E_BLK		"must specify a block special device"
#define E_CHR		"must specify a character special device"
#define E_DIR		"must specify a directory"
#define E_REG		"must be a regular file"
#define E_NONZERO	"must be a file of non-zero length"

#define H_READ		"must be readable"
#define H_WRITE		"must be writable"
#define H_EXEC		"must be executable"
#define H_CREAT		"will be created if it does not exist"
#define H_ABSOLUTE	E_ABSOLUTE
#define H_RELATIVE	E_RELATIVE
#define H_EXIST		"must already exist"
#define H_NEXIST	"must not already exist"
#define H_BLK		E_BLK
#define H_CHR		E_CHR
#define H_DIR		E_DIR
#define H_REG		E_REG
#define H_NONZERO	E_NONZERO

#define MSGSIZ	1024
#define STDHELP \
	"A pathname is a filename, optionally preceded by parent directories." 

static char	*errstr;
static char	*badset = "*?[]{}()<> \t'`\"\\|^";

static void
addhlp(msg, text)
char	*msg, *text;
{
	static int count;

	if(text == NULL) {
		count = 0;
		return;
	}
	if(!count++)
		(void) strcat(msg, " The pathname you enter:");
	(void) strcat(msg, "\\n\\t-\\ ");
	(void) strcat(msg, text);
}

static char *
sethlp(pflags)
int	pflags;
{
	char	*msg;

	msg = (char *) calloc(MSGSIZ, sizeof(char));
	addhlp(msg, (char *)0); /* initialize count */
	(void) strcpy(msg, STDHELP);

	if(pflags & P_EXIST)
		addhlp(msg, H_EXIST);
	else if(pflags & P_NEXIST)
		addhlp(msg, H_NEXIST);

	if(pflags & P_ABSOLUTE)
		addhlp(msg, H_ABSOLUTE);
	else if(pflags & P_RELATIVE)
		addhlp(msg, H_RELATIVE);

	if(pflags & P_READ)
		addhlp(msg, H_READ);
	if(pflags & P_WRITE)
		addhlp(msg, H_WRITE);
	if(pflags & P_EXEC)
		addhlp(msg, H_EXEC);
	if(pflags & P_CREAT)
		addhlp(msg, H_CREAT);

	if(pflags & P_BLK)
		addhlp(msg, H_BLK);
	else if(pflags & P_CHR)
		addhlp(msg, H_CHR);
	else if(pflags & P_DIR)
		addhlp(msg, H_DIR);
	else if(pflags & P_REG)
		addhlp(msg, H_REG);

	if(pflags & P_NONZERO)
		addhlp(msg, H_NONZERO);

	return(msg);
}

ckpath_stx(pflags)
int	pflags;
{
	if(((pflags & P_ABSOLUTE) && (pflags & P_RELATIVE)) ||
	   ((pflags & P_NEXIST) && (pflags & 
		(P_EXIST|P_NONZERO|P_READ|P_WRITE|P_EXEC))) ||
	   ((pflags & P_CREAT) && (pflags & (P_EXIST|P_NEXIST|P_BLK|P_CHR))) ||
	   ((pflags & P_BLK) && (pflags & (P_CHR|P_REG|P_DIR|P_NONZERO))) ||
	   ((pflags & P_CHR) && (pflags & (P_REG|P_DIR|P_NONZERO))) ||
	   ((pflags & P_DIR) && (pflags & P_REG))) {
		return(1);
	}
	return(0);
}

int
ckpath_val(path, pflags)
char	*path;
int	pflags;
{
	struct stat status;
	int	fd;
	char	*pt;

	if((pflags & P_RELATIVE) && (*path == '/')) {
		errstr = E_RELATIVE;
		return(1);
	}
	if((pflags & P_ABSOLUTE) && (*path != '/')) {
		errstr = E_ABSOLUTE;
		return(1);
	}
	if(stat(path, &status)) {
		if(pflags & P_EXIST) {
			errstr = E_EXIST;
			return(1);
		}
		for(pt=path; *pt; pt++) {
			if(!isprint(*pt) || strchr(badset, *pt)) {
				errstr = E_SYNTAX;
				return(1);
			}
		}
		if(pflags & P_CREAT) {
			if((fd = creat(path, 0644)) < 0) {
				errstr = E_CREAT;
				return(1);
			}
			(void) close(fd);
		}
		return(0);
	} else if(pflags & P_NEXIST) {
		errstr = E_NEXIST;
		return(1);
	}
	if((status.st_mode & S_IFMT) == S_IFREG) {
		/* check non zero status */
		if((pflags & P_NONZERO) && (status.st_size < 1)) {
			errstr = E_NONZERO;
			return(1);
		}
	}
	if((pflags & P_CHR) && ((status.st_mode & S_IFMT) != S_IFCHR)) {
		errstr = E_CHR;
		return(1);
	}
	if((pflags & P_BLK) && ((status.st_mode & S_IFMT) != S_IFBLK)) {
		errstr = E_BLK;
		return(1);
	}
	if((pflags & P_DIR) && ((status.st_mode & S_IFMT) != S_IFDIR)) {
		errstr = E_DIR;
		return(1);
	}
	if((pflags & P_REG) && ((status.st_mode & S_IFMT) != S_IFREG)) {
		errstr = E_REG;
		return(1);
	}
	if((pflags & P_READ) && !(status.st_mode & S_IREAD)) {
		errstr = E_READ;
		return(1);
	}
	if((pflags & P_WRITE) && !(status.st_mode & S_IWRITE)) {
		errstr = E_WRITE;
		return(1);
	}
	if((pflags & P_EXEC) && !(status.st_mode & S_IEXEC)) {
		errstr = E_EXEC;
		return(1);
	}
	return(0);
}

void
ckpath_err(pflags, error, input)
int	pflags;
char	*error, *input;
{
	char	buffer[2048];
	char	*defhlp;

	if(input) {
		if(ckpath_val(input, pflags)) {
			(void) sprintf(buffer, "Pathname %s.", errstr);
			puterror(stdout, buffer, error);
			return;
		}
	}
	defhlp = sethlp(pflags);
	puterror(stdout, defhlp, error);
	free(defhlp);
}

void
ckpath_hlp(pflags, help)
int	pflags;
char	*help;
{
	char	*defhlp;

	defhlp = sethlp(pflags);
	puthelp(stdout, defhlp, help);
	free(defhlp);
}

int
ckpath(pathval, pflags, defstr, error, help, prompt)
char *pathval;
int pflags;
char *defstr, *error, *help, *prompt;
{
	char	*defhlp,
		input[128],
		buffer[256];

	if((pathval == NULL) || ckpath_stx(pflags))
		return(2); /* usage error */

	if(!prompt) {
		if(pflags & P_ABSOLUTE)
			prompt = "Enter an absolute pathname";
		else if(pflags & P_RELATIVE)
			prompt = "Enter a relative pathname";
		else
			prompt = "Enter a pathname";
	}
	defhlp = sethlp(pflags);

start:
	putprmpt(stderr, prompt, NULL, defstr);
	if(getinput(input)) {
		free(defhlp);
		return(1);
	}

	if(strlen(input) == 0) {
		if(defstr) {
			(void) strcpy(pathval, defstr);
			free(defhlp);
			return(0);
		}
		puterror(stderr, NULL, "Input is required.");
		goto start;
	}
	if(!strcmp(input, "?")) {
		puthelp(stderr, defhlp, help);
		goto start;
	}
	if(ckquit && !strcmp(input, "q")) {
		free(defhlp);
		return(3);
	}

	if(ckpath_val(input, pflags)) {
		(void) sprintf(buffer, "Pathname %s.", errstr);
		puterror(stderr, buffer, error);
		goto start;
	}
	(void) strcpy(pathval, input);
	free(defhlp);
	return(0);
}
