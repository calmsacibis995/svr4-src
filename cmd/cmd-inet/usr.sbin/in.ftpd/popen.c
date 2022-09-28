/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-inet:usr.sbin/in.ftpd/popen.c	1.6.2.1"

/*
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988.1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 */

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>

#ifdef SYSV
#define bzero(s,n)	memset((s), 0, (n))

#ifndef sigmask
#define sigmask(m)      (1 << ((m)-1))
#endif	/* sigmask */

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

#include <sys/resource.h>

#define NOFILES 20      /* just in case */

static int
getdtablesize()
{
	struct rlimit   rl;
	
	if ( getrlimit(RLIMIT_NOFILE, &rl) == 0 )
		return(rl.rlim_max);
	else
		return(NOFILES);
}
#endif /* SYSV */

/*
 * Special version of popen which avoids call to shell.  This insures noone
 * may create a pipe to a hidden program as a side effect of a list or dir
 * command.
 */
static pid_t *pids;
static int fds;

FILE *
popen(program, type)
	char *program, *type;
{
	register char *cp;
	FILE *iop;
	pid_t pid;
	int argc, gargc, pdes[2];
	char **pop, *argv[100], *gargv[1000], *vv[2];
	extern char **glob(), **copyblk(), *strtok();

	if (*type != 'r' && *type != 'w' || type[1])
		return(NULL);

	if (!pids) {
		if ((fds = getdtablesize()) <= 0)
			return(NULL);
		if (!(pids =
		    (pid_t *)malloc((u_int)(fds * sizeof(pid_t)))))
			return(NULL);
		bzero(pids, fds * sizeof(pid_t));
	}
	if (pipe(pdes) < 0)
		return(NULL);

	/* break up string into pieces */
	for (argc = 0, cp = program;; cp = NULL)
		if (!(argv[argc++] = strtok(cp, " \t\n")))
			break;

	/* glob each piece */
	gargv[0] = argv[0];
	for (gargc = argc = 1; argv[argc]; argc++) {
		if (!(pop = glob(argv[argc]))) {	/* globbing failed */
			vv[0] = argv[argc];
			vv[1] = NULL;
			pop = copyblk(vv);
		}
		argv[argc] = (char *)pop;		/* save to free later */
		while (*pop && gargc < 1000)
			gargv[gargc++] = *pop++;
	}
	gargv[gargc] = NULL;

	iop = NULL;
	switch(pid = vfork()) {
	case -1:			/* error */
		(void)close(pdes[0]);
		(void)close(pdes[1]);
		goto free;
		/* NOTREACHED */
	case 0:				/* child */
		if (*type == 'r') {
			if (pdes[1] != 1) {
			/*
			 * Need to grab stderr too for new ls
			 */
				dup2(pdes[1], 2);
				dup2(pdes[1], 1);
				(void)close(pdes[1]);
			}
			(void)close(pdes[0]);
		} else {
			if (pdes[0] != 0) {
				dup2(pdes[0], 0);
				(void)close(pdes[0]);
			}
			(void)close(pdes[1]);
		}
		execv(gargv[0], gargv);
		_exit(1);
	}
	/* parent; assume fdopen can't fail...  */
	if (*type == 'r') {
		iop = fdopen(pdes[0], type);
		(void)close(pdes[1]);
	} else {
		iop = fdopen(pdes[1], type);
		(void)close(pdes[0]);
	}
	pids[fileno(iop)] = pid;

free:	for (argc = 1; argv[argc] != NULL; argc++)
		blkfree((char **)argv[argc]);
	return(iop);
}

pclose(iop)
	FILE *iop;
{
	register int fdes;
	long omask;
	pid_t pid;
	int stat_loc;

	/*
	 * pclose returns -1 if stream is not associated with a
	 * `popened' command, or, if already `pclosed'.
	 */
	if (pids[fdes = fileno(iop)] == 0)
		return(-1);
	(void)fclose(iop);
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGQUIT)|sigmask(SIGHUP));
	while ((pid = wait(&stat_loc)) != pids[fdes] && pid != (pid_t)-1);
	(void)sigsetmask(omask);
	pids[fdes] = 0;
	return(stat_loc);
}
