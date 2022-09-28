/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)env:env.c	1.5.1.1"

/*
 *	env [ - ] [ name=value ]... [command arg...]
 *	set environment, then execute command (or print environment)
 *	- says start fresh, otherwise merge with inherited environment
 */
#include <stdio.h>

#define NENV	100
char	*newenv[NENV];
char	*nullp = NULL;

extern	char **environ;
extern	errno;
extern	char *sys_errlist[];
char	*nvmatch(), *strchr();
void	exit();

main(argc, argv, envp)
register char **argv, **envp;
{

	argc--;
	argv++;
	if (argc && strcmp(*argv, "-") == 0) {
		envp = &nullp;
		argc--;
		argv++;
	}

	for (; *envp != NULL; envp++)
		if (strchr(*envp, '=') != NULL)
			addname(*envp);
	while (*argv != NULL && strchr(*argv, '=') != NULL)
		addname(*argv++);

	if (*argv == NULL)
		print(0); /* doesn't return */
	else {
		environ = newenv;
		(void) execvp(*argv, argv);
		(void) fputs(sys_errlist[errno], stderr);
		(void) fputs(": ", stderr);
		(void) fputs(*argv, stderr);
		(void) putc('\n', stderr);
		exit(1);
	}
}

addname(arg)
register char *arg;
{
	register char **p;

	for (p = newenv; *p != NULL; p++) {
		if (p >= &newenv[NENV-1]) {
			(void) fputs("too many values in environment\n", stderr);
			print(1); /* doesn't return */
		}
		if (nvmatch(arg, *p) != NULL)
			break;
	}
	*p = arg;
}

print(code)
{
	register char **p = newenv;

	while (*p != NULL)
		(void) puts(*p++);
	exit(code);
}

/*
 *	s1 is either name, or name=value
 *	s2 is name=value
 *	if names match, return value of s2, else NULL
 */

char *
nvmatch(s1, s2)
register char *s1, *s2;
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return(s2);
	if (*s1 == '\0' && *(s2-1) == '=')
		return(s2);
	return(NULL);
}
