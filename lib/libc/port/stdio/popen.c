/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:stdio/popen.c	1.29"
/*LINTLIBRARY*/
#ifdef __STDC__
	#pragma weak pclose = _pclose
	#pragma weak popen = _popen
#endif
#include <sys/utsname.h>
#undef uname
#include "synonyms.h"
#include "shlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define	tst(a,b) (*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

#define BIN_SH "/sbin/sh"
#define SH "sh"
#define SHFLG "-c"
#if DSHLIB
static int *popen_pid;
#else
static int popen_pid[256];
#endif

FILE *
popen(cmd, mode)
const char	*cmd, *mode;
{
	int	p[2];
	register int *poptr;
	register int myside, yourside, pid;

#if DSHLIB
	if (popen_pid == NULL && (popen_pid = (int *)calloc(256, sizeof(int))) == NULL)
		return (NULL);
#endif
	
	if(pipe(p) < 0)
		return(NULL);
	myside = tst(p[WTR], p[RDR]);
	yourside = tst(p[RDR], p[WTR]);
	if((pid = fork()) == 0) {
		/* myside and yourside reverse roles in child */
		int	stdio;

		/* close all pipes from other popen's */
		for (poptr = popen_pid; poptr < popen_pid+256; poptr++) {
			if(*poptr)
				close(poptr - popen_pid);
		}
		stdio = tst(0, 1);
		(void) close(myside);
		(void) close(stdio);
		(void) fcntl(yourside, F_DUPFD, stdio);
		(void) close(yourside);
		(void) execl(BIN_SH, SH, SHFLG, cmd, (char *)0);
		_exit(1);
	}
	if(pid == -1)
		return(NULL);
	popen_pid[myside] = pid;
	(void) close(yourside);
	return(fdopen(myside, mode));
}

int
pclose(ptr)
FILE	*ptr;
{
	static int	vers;
	struct utsname	uname_buf;
	register int f, r;
	int status;

#if DSHLIB
	if (!popen_pid)
		return -1;
#endif

	if (vers == 0) {
		if (uname(&uname_buf) > 0)
			vers = 2;	/* SVR4 system */
		else
			vers = 1;	/* non-SVR4 system */
	}

	f = fileno(ptr);
	(void) fclose(ptr);

	if (vers == 1) {
		sighold(SIGINT);
		sighold(SIGQUIT);
		sighold(SIGHUP);

		/* while the child is not done and no error has occured wait in the loop*/
		while((r = wait(&status)) != popen_pid[f] && (r != -1 || errno == EINTR))
			;
		if(r == -1)
			status = -1;

		sigrelse(SIGINT);
		sigrelse(SIGQUIT);
		sigrelse(SIGHUP);
	} else {
		if (waitpid(popen_pid[f], &status, 0) < 0)
			status = -1;
	}

	/* mark this pipe closed */
	popen_pid[f] = 0;
	return(status);
}
