/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mail:xgetenv.c	1.6.3.1"
/*
    NAME
	xsetenv, xgetenv, Xgetenv - manage an alternate environment space

    SYNOPSIS
	int ret = xsetenv(file)
	char *x = xgetenv("FOO");
	char *x = Xgetenv("FOO");

    DESCRIPTION
	xsetenv() reads the given file into an internal buffer
	and sets up an alternate environment.

	Return values:	 1 - OKAY
			 0 - troubles reading the file
			-1 - troubles opening the file

	xgetenv() returns the environment value from the
	alternate environment.

	Return values:	(char*)0 - no value for that variable
			pointer  - the value

	Xgetenv() returns the environment value from the
	alternate environment.

	Return values:	"" - no value for that variable
			pointer  - the value

    LIMITATIONS
	Assumes the environment is < 5120 bytes (as in the UNIX
	System environment). Assumes < 512 lines in the file.
	These values may be adjusted below.

*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#ifdef __STDC__
#include <stdlib.h>
#include <unistd.h>
#else
extern int atoi();
extern int read();
extern int close();
extern char *getenv();
#endif

#define MAXVARS  512
#define MAXENV  5120

static char **xenv = 0;
static char *(xenvptrs[MAXVARS]);
static char xbuf[MAXENV];

#ifdef __STDC__
static void reduce(char *from);
#else
static void reduce();
#endif

/*
 *	set up an environment buffer
 *	and the pointers into it
 */
#ifdef __STDC__
int xsetenv(char *xfile)
#else
int xsetenv(xfile)
char *xfile;
#endif
{
    register int i, envctr, nread, infd;

    /* Open the file */
    infd = open(xfile, O_RDONLY);
    if (infd == -1) {
	return (-1);
    }

    /* Read in the entire file. */
    nread = read(infd, xbuf, sizeof(xbuf));
    if (nread < 0) {
	(void) close(infd);
	return (0);
    }

    /*
	Set up pointers into the buffer.
	Replace \n with \0.
	Collapse white space around the = sign and at the
	beginning and end of the line.
    */
    xenv = xenvptrs;
    xenv[0] = &xbuf[0];
    for (i = 0, envctr = 0; i < nread; i++) {
	if (xbuf[i] == '\n') {
	    xbuf[i] = '\0';
	    reduce(xenv[envctr]);
	    xenv[++envctr] = &xbuf[i+1];
	    if (envctr == MAXVARS) {
		break;
	    }
	}
    }

    xenv[envctr] = 0;
    (void) close(infd);
    return (1);
}

/*
 *	Let getenv() do the dirty work
 *	of looking up the variable. We
 *	do this by temporarily resetting
 *	environ to point to the local area.
 */
#ifdef __STDC__
char *xgetenv(char *env)
#else
char *xgetenv(env)
char *env;
#endif
{
    extern char **environ;
    register char *ret, **svenviron = environ;

    environ = xenv;
    ret = getenv(env);
    environ = svenviron;
    return ret;
}

/*
 *	Let xgetenv() do the dirty work
 *	of looking up the variable.
 */
#ifdef __STDC__
char *Xgetenv(char *env)
#else
char *Xgetenv(env)
char *env;
#endif
{
    char *ret = xgetenv(env);
    return ret ? ret : "";
}

/*
 * Remove the spaces within the environment variable.
 * The variable can look like this:
 *
 * <sp1> variable <sp2> = <sp3> value <sp4> \0
 *
 * All spaces can be removed, except within
 * the variable name and the value.
 */

#ifdef __STDC__
static void reduce(register char *from)
#else
static void reduce(from)
register char *from;
#endif
{
    register char *to = from;
    register char *svfrom = from;

    /* <sp1> */
    while (*from &&isspace(*from))
	from++;

    /* variable */
    while (*from && (*from != '=') && !isspace(*from))
	*to++ = *from++;

    /* <sp2> */
    while (*from && isspace(*from))
	from++;

    /* = */
    if (*from == '=')
	*to++ = *from++;

    /* <sp3> */
    while (*from && isspace(*from))
	from++;

    /* value */
    while (*from)
	*to++ = *from++;

    /* <sp4> */
    while ((to > svfrom) && isspace(to[-1]))
	to--;
    *to = '\0';
}
