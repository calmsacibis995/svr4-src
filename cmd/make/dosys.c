/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:dosys.c	1.13"
/*	@(#)make:dosys.c	1.7 of 5/1/89	*/

#include "defs"
#include <signal.h>
#include <errno.h>

#ifndef MAKE_SHELL
#define MAKE_SHELL	"/usr/bin/sh"
#endif

extern char funny[ARY256];
extern errno;

/*
**	Declate local functions and make LINT happy.
*/

static int	any_metas();
static int	doshell();
static int	await();
static void	doclose();
static int	doexec();

int
dosys(comstring, nohalt, Makecall)	/* do the system call */
CHARSTAR comstring;
int	nohalt, Makecall;
{
	register CHARSTAR str = comstring;

	while (	*str == BLANK || *str == TAB ) 
		str++;
	if (!*str)
		return(-1);	/* no command */

	if (IS_ON(NOEX) && !Makecall)
		return(0);

	if (any_metas(str))
		return( doshell(str, nohalt) );
	else
		return( doexec(str) );
}


static int
any_metas(s)		/* Are there are any Shell meta-characters? */
register CHARSTAR s;
{
	while ( *s )
		if ( funny[(unsigned char) *s++] & META )
			return(YES);
	return(NO);
}


static int
doshell(comstring, nohalt)
CHARSTAR comstring;
int	nohalt;
{
	extern int waitpid;
	int	fork(), execl(), execlp();

#ifdef unix
	void	enbint();
#endif


	if ( !(waitpid = fork()) ) {
		void	setenv();
		char *getenv(), *myshell;
#ifdef unix
		enbint(SIG_DFL);
#endif
		doclose();
		setenv();

		if (((myshell = getenv("SHELL")) == NULL) || (myshell == CNULL))
			myshell = MAKE_SHELL;
		if (*myshell == '/')
			(void)execl(myshell, "sh", (nohalt ? "-c" : "-ce"), comstring, 0);
		else
			(void)execlp(myshell, "sh", (nohalt ? "-c" : "-ce"), comstring, 0);
		if(errno == E2BIG)
			fatal("couldn't load shell: Argument list too long (bu22)");
		else
			fatal("couldn't load shell (bu22)");
	} else if ( waitpid == -1 )
		fatal("couldn't fork");

	return( await() );
}


static int
await()
{
	extern int	waitpid;
	register int	pid;
	int	wait(), status;
	void	intrupt();
#ifdef unix
	void	enbint();
#endif

#ifdef unix
	enbint(intrupt);
#endif
	while ((pid = wait(&status)) != waitpid)
		if (pid == -1)
			fatal("bad wait code (bu23)");
	waitpid = 0;
	return(status);
}


static void
doclose()	/* Close open directory files before exec'ing */
{
	register OPENDIR od = firstod;
	for (; od; od = od->nextopendir)
		if ( od->dirfc )
			(void)closedir(od->dirfc);
}


static int
doexec(str)
register CHARSTAR str;
{
	extern int	waitpid;
	int	execvp();
#ifdef unix
	void	enbint();
#endif

	if ( *str == CNULL )
		return(-1);	/* no command */
	else {
		int	fork();
		register CHARSTAR t = str, *p;
		int incsize = ARY200, aryelems = incsize, numelems = 0;
		CHARSTAR *argv = (CHARSTAR *)
					malloc(aryelems * sizeof(CHARSTAR));

		if (!argv) fatal1("doexec: out of memory");
		p = argv;
		for ( ; *t ; ) {
			*p++ = t;
			if (++numelems == aryelems) {
				aryelems += incsize;
				argv = (CHARSTAR *)
					realloc(argv, aryelems * sizeof(CHARSTAR));
				if (!argv) fatal1("doexec: out of memory");
				p = argv + (aryelems - incsize);
			}
			while (*t != BLANK && *t != TAB && *t != CNULL)
				++t;
			if (*t)
				for (*t++ = CNULL; *t == BLANK || *t == TAB; ++t)
					;
		}

		*p = NULL;

		if ( !(waitpid = fork()) ) {
			void	setenv();
#ifdef unix
			enbint(SIG_DFL);
#endif
			doclose();
			setenv();
			(void)execvp(str, argv);
			if(errno == E2BIG)
				fatal1("cannot load %s : Argument list too long (bu24)", str);
			else
				fatal1("cannot load %s (bu24)", str);
		} else if ( waitpid == -1 )
			fatal("couldn't fork");

		free(argv);
		return( await() );
	}
}
