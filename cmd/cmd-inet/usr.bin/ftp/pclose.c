/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.bin/ftp/pclose.c	1.7.2.1"

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */


#include <stdio.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/wait.h>
#ifndef SYSV
#include <vfork.h>
#else	/* SYSV */
extern pid_t vfork();


#ifndef sigmask
#define sigmask(m)      (1 << ((m)-1))
#endif

#define set2mask(setp) ((setp)->sigbits[0])
#define mask2set(mask, setp) \
	((mask) == -1 ? sigfillset(setp) : (((setp)->sigbits[0]) = (mask)))
	

static sigsetmask(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_SETMASK, &nset, &oset);
	return set2mask(&oset);
}

static sigblock(mask)
	int mask;
{
	sigset_t oset;
	sigset_t nset;

	(void) sigprocmask(0, (sigset_t *)0, &nset);
	mask2set(mask, &nset);
	(void) sigprocmask(SIG_BLOCK, &nset, &oset);
	return set2mask(&oset);
}

#define signal(s,f)	sigset(s,f)
#endif /* SYSV */


#define	tst(a,b)	(*mode == 'r'? (b) : (a))
#define	RDR	0
#define	WTR	1

extern	char *malloc();

static	pid_t *popen_pid;
static	int nfiles;

FILE *
mypopen(cmd,mode)
	char *cmd;
	char *mode;
{
	int p[2];
	pid_t pid;
	int myside, remside, i;

	if (nfiles <= 0)
		nfiles = getdtablesize();
	if (popen_pid == NULL) {
		popen_pid = (pid_t *)malloc((unsigned) nfiles * sizeof *popen_pid);
		if (popen_pid == NULL)
			return (NULL);
		for (i = 0; i < nfiles; i++)
			popen_pid[i] = (pid_t)-1;
	}
	if (pipe(p) < 0)
		return (NULL);
	myside = tst(p[WTR], p[RDR]);
	remside = tst(p[RDR], p[WTR]);
	if ((pid = vfork()) == 0) {
		/* myside and remside reverse roles in child */
		(void) close(myside);
		if (remside != tst(0, 1)) {
			(void) dup2(remside, tst(0, 1));
			(void) close(remside);
		}
		execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
		_exit(127);
	}
	if (pid == (pid_t)-1) {
		(void) close(myside);
		(void) close(remside);
		return (NULL);
	}
	popen_pid[myside] = pid;
	(void) close(remside);
	return (fdopen(myside, mode));
}

void
pabort()
{
	extern int mflag;

	mflag = 0;
}

mypclose(ptr)
	FILE *ptr;
{
	pid_t child, pid;
	int omask;
	void (*istat)();
#ifdef SYSV
	int status;
#else
	union wait status;
#endif /* SYSV */

	child = popen_pid[fileno(ptr)];
	popen_pid[fileno(ptr)] = (pid_t)-1;
	(void) fclose(ptr);
	if (child == (pid_t)-1)
		return (-1);
	istat = signal(SIGINT, (void (*)())pabort);
	omask = sigblock(sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&status)) != child && pid != (pid_t)-1)
		;
	(void) sigsetmask(omask);
	(void) signal(SIGINT, istat);
	return (pid == (pid_t)-1 ? -1 : 0);
}

#ifdef SYSV

#include <sys/resource.h>

#define	NOFILES 20	/* just in case */

int
getdtablesize()
{
	struct rlimit	rl;

	if ( getrlimit(RLIMIT_NOFILE, &rl) == 0 )
		return(rl.rlim_max);
	else
		return(NOFILES);
}
#endif /* SYSV */
