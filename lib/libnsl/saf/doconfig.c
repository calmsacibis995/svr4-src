/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

# ident	"@(#)libnet:saf/doconfig.c	1.6.1.2"

# include <stdio.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <stropts.h>
# include <ctype.h>
# include <sys/conf.h>
# include <sys/errno.h>
# include <signal.h>
# include "sac.h"


# define COMMENT	'#'
# define NOWAIT		0
# define WAIT		1


extern	char	*strchr();
extern	char	*getenv();
extern	char	*malloc();
extern	char	**_environ;
extern	int	errno;

char	*eatwhite();


/*
 * doconfig - the configuration script interpreter, if all is ok,
 *	      return 0.  If there is a "system" error, return -1.
 *	      If there is an error performing a command, or there
 *	      is a syntax error, return the line number in error.
 *
 *	args:	fd - file descriptor to push and pop from
 *		script - name of the configuration script
 *		rflag - restriction flag to determine what "commands"
 *			can be run
 */


doconfig(fd, script, rflag)
int fd;
char *script;
long rflag;
{
	int line;		/* line counter */
	struct stat statbuf;	/* place for stat */
	FILE *fp;		/* file pointer for config script */
	char buf[BUFSIZ + 1];	/* scratch buffer */
	register char *bp;	/* scratch pointer */
	register char *p;	/* scratch pointer */

	/* if the script does not exist, then there is nothing to do */
	if (stat(script, &statbuf) < 0)
		return(0);

	fp = fopen(script, "r");
	if (fp == NULL)
		return(-1);

	line = 0;
	while (fgets(buf, BUFSIZ, fp)) {
		line++;
		p = strchr(buf, '\n');
		/* if no \n, then line is too long */
		if (p == NULL) {
			(void) fclose(fp);
			return(line);
		}
		*p = '\0';

		/* remove comments */
		p = strchr(buf, COMMENT);
		if (p)
			*p = '\0';

		/* remove leading whitespace */
		bp = eatwhite(buf);
		/* see if anything is left */
		if (*bp == '\0')
			continue;

		/* remove trailing whitespace */
		p = &buf[strlen(buf) - 1];
		while (*p && isspace(*p))
			*p-- = '\0';

		/* get the command */
		p = bp;
		while (*p && !isspace(*p))
			p++;
		if (*p)
			*p++ = '\0';
		/* skip any whitespace here too (between command and args) */
		p = eatwhite(p);

		if (!strcmp(bp, "assign")) {
			if ((rflag & NOASSIGN) || doassign(p)) {
				(void) fclose(fp);
				return(line);
			}
		}
		else if (!strcmp(bp, "push")) {
			if (dopush(fd, p)) {
				(void) fclose(fp);
				return(line);
			}
		}
		else if (!strcmp(bp, "pop")) {
			if (dopop(fd, p)) {
				(void) fclose(fp);
				return(line);
			}
		}
		else if (!strcmp(bp, "run")) {
			if ((rflag & NORUN) || dorun(p, NOWAIT)) {
				(void) fclose(fp);
				return(line);
			}
		}
		else if (!strcmp(bp, "runwait")) {
			if ((rflag & NORUN) || dorun(p, WAIT)) {
				(void) fclose(fp);
				return(line);
			}
		}
		else {
			/* unknown command */
			(void) fclose(fp);
			return(line);
		}
	}
	if (!feof(fp)) {
		(void) fclose(fp);
		return(-1);
	}
	else {
		(void) fclose(fp);
		return(0);
	}
}


/*
 * doassign - handle an `assign' command
 *
 *	args:	p - assignment string
 */


static int
doassign(p)
register char *p;
{
	char *var;		/* environment variable to be assigned */
	char val[BUFSIZ];	/* and the value to be assigned to it */
	char scratch[BUFSIZ];	/* scratch buffer */
	char delim;		/* delimiter char seen (for quoted strings ) */
	register char *tp;	/* scratch pointer */

	if (*p == '\0')
		return(-1);
	var = p;
	/* skip first token, but stop if we see a '=' */
	while (*p && !isspace(*p) && (*p != '='))
		p++;

	/* if we found end of string, it's an error */
	if (*p == '\0')
		return(-1);

	/* if we found a space, look for the '=', otherwise it's an error */
	if (isspace(*p)) {
		*p++ = '\0';
		while (*p && isspace(*p))
			p++;
		if (*p == '\0')
			return(-1);
		if (*p == '=')
			p++;
		else
			return(-1);
	}
	else {
		/* skip over '=' */
		*p = '\0';
		p++;
	}

	/* skip over any whitespace */
	p = eatwhite(p);
	if (*p == '\'' || *p == '"') {
		/* handle quoted values */
		delim = *p++;
		tp = val;
		for (;;) {
			if (*p == '\0')
				return(-1);
			else if (*p == delim) {
				if (*(p - 1) != '\\')
					break;
				else
					*(tp - 1) = *p++;
					
			}
			else
				*tp++ = *p++;
		}
		*tp = '\0';
		/* these assignments make the comment below true (values of tp and p */
		tp = ++p;
		p = val;
	}
	else {
		tp = p;
		/* look for end of token */
		while (*tp && !isspace(*tp))
			tp++;
	}

/*
 * at this point, p points to the value, and tp points to the
 * end of the token.  check to make sure there is no garbage on
 * the end of the line
 */

	if (*tp)
		return(-1);
	sprintf(scratch, "%s=%s", var, p);
	/* note: need to malloc fresh space so putenv works */
	tp = malloc(strlen(scratch) + 1);
	if (tp == NULL)
		return(-1);
	strcpy(tp, scratch);
	if (putenv(tp))
		return(-1);
	else
		return(0);
}


/*
 * dopush - handle a `push' command
 *
 *	args:	fd - file descriptor to push on
 *		p - list of modules to push
 */


static int
dopush(fd, p)
int fd;
register char *p;
{
	register char *tp;	/* scratch pointer */
	register int i;		/* scratch variable */
	int npush;		/* count # of modules pushed */

	if (*p == '\0')
		return(-1);
	npush = 0;
	for (;;) {
		if (*p == '\0')
			/* found end of line */
			return(0);
		p = eatwhite(p);
		if (*p == '\0')
			return(-1);
		tp = p;
		while (*tp && !isspace(*tp) && (*tp != ','))
				tp++;
		if (*tp)
			*tp++ = '\0';
		if (ioctl(fd, I_PUSH, p) < 0) {

/*
 * try to pop all that we've done, if pop fails it doesn't matter because
 * nothing can be done anyhow
 */

			for (i = 0; i < npush; ++i)
				ioctl(fd, I_POP, 0);
			return(-1);
		}
		else {
			/* count the number of modules we've pushed */
			npush++;
			p = tp;
		}
	}
}


/*
 * dopop - handle a `pop' command
 *
 *	args:	fd - file descriptor to pop from
 *		p - name of module to pop to or ALL (null means pop top only)
 */


static int
dopop(fd, p)
int fd;
register char *p;
{
	char *modp;		/* module name from argument to pop */
	char buf[FMNAMESZ + 1];	/* scratch buffer */

	if (*p == '\0') {
		/* just a pop with no args */
		if (ioctl(fd, I_POP, 0) < 0)
			return(-1);
		else
			return(0);
	}

	/* skip any whitespace in between */
	p = eatwhite(p);
	modp = p;
	/* find end of module name */
	while (*p && !isspace(*p))
		p++;

	if (*p)
		/* if not end of line, extra junk on line */
		return(-1);
	if (!strcmp(modp, "ALL")) {
		/* it's the magic name, pop them all */
		while (ioctl(fd, I_POP, 0) == 0)
			;
		/* After all popped, we'll get an EINVAL, which is expected */
		if (errno != EINVAL)
			return(-1);
		else
			return(0);
	}
	else {
		/* check to see if the named module is on the stream */
		if (ioctl(fd, I_FIND, modp) != 1)
			return(-1);

		/* pop them until the right one is on top */
		for (;;) {
			if (ioctl(fd, I_LOOK, buf) < 0)
				return(-1);
			if (!strcmp(modp, buf))
				/* we're done */
				return(0);
			if (ioctl(fd, I_POP, 0) < 0)
				return(-1);
		}
	}
}


/*
 * dorun - handle a `run' command
 *
 *	args:	p - command line to run
 *		waitflag - flag indicating whether a wait should be done
 */


static int
dorun(p, waitflg)
register char *p;
int waitflg;
{
	register char *tp;	/* scratch pointer */
	register char *ep;	/* scratch pointer (end of token) */
	register int nfiles;	/* # of possibly open files */
	register int i;		/* scratch variable */
	char savech;		/* hold area */
	int status;		/* return status from wait */
	pid_t pid;		/* pid of child proc */
	void (*func)();		/* return from signal */

	if (*p == '\0')
		return(-1);

/*
 * get first token
*/

	for (tp = p; *tp && !isspace(*tp); ++tp)
		;
	savech = '\0';
	if (*tp) {
		savech = *tp;
		*tp = '\0';
	}

/*
 * look for built-in's
 */

	if (!strcmp(p, "cd")) {
		*tp = savech;
		tp = eatwhite(tp);
		if (*tp == '\0')
			/* if nothing there, try to cd to $HOME */
			tp = getenv("HOME");
		if (chdir(tp) < 0)
			return(-1);
	}
	else if (!strcmp(p, "ulimit")) {
		*tp = savech;
		tp = eatwhite(tp);
		/* must have an argument */
		if (*tp == '\0')
			return(-1);
		/* make sure nothing appears on line after arg */
		for (ep = tp; *ep && !isspace(*ep); ++ep)
			;
		ep = eatwhite(ep);
		if (*ep)
			return(-1);
		if (!isdigit(*tp))
			return(-1);
		if (ulimit(2, atoi(tp)) < 0)
			return(-1);
	}
	else if (!strcmp(p, "umask")) {
		*tp = savech;
		tp = eatwhite(tp);
		/* must have an argument */
		if (*tp == '\0')
			return(-1);
		/* make sure nothing appears on line after arg */
		for (ep = tp; *ep && !isspace(*ep); ++ep)
			;
		ep = eatwhite(ep);
		if (*ep)
			return(-1);
		if (!isdigit(*tp))
			return(-1);
		if (umask(strtol(tp, NULL, 8)) < 0)
			return(-1);
	}
	else {
		/* not a built-in */
		*tp = savech;
		func = signal(SIGCLD, SIG_DFL);
		if ((pid = fork()) < 0) {
			signal(SIGCLD, func);
			return(-1);
		}
		else if (pid) {
			if (waitflg == WAIT) {
				status = 0;
				(void) waitpid(pid, &status, 0);
				if (status) {
					/* child failed */
					signal(SIGCLD, func);
					return(-1);
				}
			}
			signal(SIGCLD, func);
		}
		else {
			/* set IFS for security */
			(void) putenv("IFS=\" \"");
			/*
			 * need to close all files to prevent unauthorized
			 * access in the children.  Setup stdin, stdout,
			 * and stderr to /dev/null.
			 */
			nfiles = ulimit(4, 0);
			for (i = 0; i < nfiles; i++)
				(void) close(i);
			/* stdin */
			if (open("/dev/null", O_RDWR) != 0)
				return(-1);
			/* stdout */
			if (dup(0) != 1)
				return(-1);
			/* stderr */
			if (dup(0) != 2)
				return(-1);
			execle("/usr/bin/sh", "sh", "-c", p, 0, _environ);
			/* if we get here, there is a problem - remember that
			   this is the child */
			exit(1);
		}
	}
	return(0);
}


/*
 * eatwhite - swallow any leading whitespace, return pointer to first
 *	      non-white space character or to terminating null character
 *	      if nothing else is there
 *
 *	args:	p - string to parse
 */


static char *
eatwhite(p)
register char *p;
{
	while (*p && isspace(*p))
		p++;
	return(p);
}
