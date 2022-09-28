/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:ipc.c	1.2.3.1"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include <signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "proto.h"

/*
 * Routines related to interprocess communication
 * among the truss processes which are controlling
 * multiple traced processes.
 */

/* This module is carefully coded to contain only read-only data */
/* All read/write data is defined in ramdata.c (see also ramdata.h) */

extern	void	perror();
extern	void	errmsg();
extern	long	strtol();
extern	unsigned alarm();

/*
 * Function prototypes for static routines in this module.
 */
#if	defined(__STDC__)

static	void	Ecritical( int );
static	void	Xcritical( int );
static	void	UnFlush();

#else	/* defined(__STDC__) */

static	void	Ecritical();
static	void	Xcritical();
static	void	UnFlush();

#endif	/* defined(__STDC__) */

void
Flush()		/* ensure everyone keeps out of each other's way */
{		/* while writing lines of trace output		 */

	/* except for regions bounded by Eserialize()/Xserialize(), */
	/* this is the only place anywhere in the program */
	/* where a write() to the trace output file takes place */
	/* so here is where we detect errors writing to the output */

	register FILE * fp = stdout;

	if (fp->_ptr == fp->_base)
		return;

	errno = 0;

	if (Cp->serialize == 0 || semid == -1)
	{
		increment(&Cp->nonserial);
		if (interrupt)
			UnFlush();
		else
			(void) fflush(fp);
		decrement(&Cp->nonserial);
	}
	else {
		Ecritical(0);
		if (interrupt)
			UnFlush();
		else
			(void) fflush(fp);
		Xcritical(0);
	}

	if (ferror(fp) && errno)	/* error on write(), probably EPIPE */
		interrupt = TRUE;		/* post an interrupt */
}

static void
UnFlush()	/* avoid writing what is in the stdout buffer */
{
	register FILE * fp = stdout;

	fp->_cnt -= (fp->_ptr - fp->_base);	/* this is filthy */
	fp->_ptr = fp->_base;
}

/* Eserialize() and Xserialize() are used to bracket */
/* a region which may produce large amounts of output, */
/* such as showargs()/dumpargs() */

void
Eserialize()
{
	/* tell everyone to serialize output */
	increment(&Cp->serialize);

	Ecritical(0);

	/* wait for everyone to synchronize */
	while (Cp->nonserial && !interrupt) {
		timeout = TRUE;
		(void) alarm(1);
		(void) pause();
	}
	if (timeout) {
		(void) alarm(0);
		timeout = FALSE;
	}
}

void
Xserialize()
{
	(void) fflush(stdout);

	Xcritical(0);

	/* OK not to serialize now */
	decrement(&Cp->serialize);
}

static void	/* enter critical region --- */
Ecritical(sem)	/* wait on semaphore, lock out other processes */
	int sem;	/* which semaphore */
{
	struct sembuf sembuf;
	register struct sembuf *sops;

	if (semid == -1)	/* there is no semaphore */
		return;

	sops = &sembuf;
	sops->sem_num = sem;		/* semaphore # */
	sops->sem_op = -1;		/* decrement semaphore */

/* Using SEM_UNDO too easily exhausts the system's */
/* semadj structures when truss follows many processes. */
/* It is only needed as a precaution against a truss process dying */
/* without having properly adjusted one of its semaphores. */
/* So long as truss works correctly, there will be no problem regardless */
#if 0
	sops->sem_flg = SEM_UNDO;	/* wait for semaphore to exceed 0 */
#else
	sops->sem_flg = 0;		/* wait for semaphore to exceed 0 */
#endif

	while (semop(semid, sops, 1) == -1) {
		if (errno != EINTR) {
			char semnum[2];
			semnum[0] = '0' + sem;
			semnum[1] = '\0';

			perror(command);
			errmsg("cannot decrement semaphore #", semnum);
			semid = -1;
			break;
		}
	}
}

static void	/* exit critical region --- */
Xcritical(sem)	/* release other processes waiting on semaphore */
	int sem;	/* which semaphore */
{
	struct sembuf sembuf;
	register struct sembuf *sops;

	if (semid == -1)	/* there is no semaphore */
		return;

	sops = &sembuf;
	sops->sem_num = sem;		/* semaphore # */
	sops->sem_op = 1;		/* increment semaphore */

/* See the comment above about using SEM_UNDO */
#if 0
	sops->sem_flg = SEM_UNDO;
#else
	sops->sem_flg = 0;
#endif

	while (semop(semid, sops, 1) == -1) {
		if (errno != EINTR) {
			char semnum[2];
			semnum[0] = '0' + sem;
			semnum[1] = '\0';

			perror(command);
			errmsg("cannot increment semaphore #", semnum);
			semid = -1;
			break;
		}
	}
}

void
procadd(spid)	/* add process to list of those being traced */
	register pid_t spid;
{
	register int i;
	register int j = -1;

	if (Cp == NULL)
		return;

	Ecritical(1);
	for (i = 0; i < sizeof(Cp->tpid)/sizeof(Cp->tpid[0]); i++) {
		if (Cp->tpid[i] == 0) {
			if (j == -1)	/* remember first vacant slot */
				j = i;
			if (Cp->spid[i] == 0)	/* this slot is better */
				break;
		}
	}
	if (i < sizeof(Cp->tpid)/sizeof(Cp->tpid[0]))
		j = i;
	if (j >= 0) {
		Cp->tpid[j] = getpid();
		Cp->spid[j] = spid;
	}
	Xcritical(1);
}

void
procdel()	/* delete process from list of those being traced */
{
	register int i;
	register pid_t tpid;

	if (Cp == NULL)
		return;

	tpid = getpid();

	Ecritical(1);
	for (i = 0; i < sizeof(Cp->tpid)/sizeof(Cp->tpid[0]); i++) {
		if (Cp->tpid[i] == tpid) {
			Cp->tpid[i] = 0;
			break;
		}
	}
	Xcritical(1);
}

int				/* check for open of /proc/nnnnn file */
checkproc(Pr, path, err)	/* return TRUE iff process opened its own */
	register process_t *Pr;	/* else inform controlling truss process */
	register char * path;
	int err;
{
	register int pid;
	register int i;
	register char *fname;
	CONST char * dirname;
	char * next;
	char * sp;
	int rc = FALSE;		/* assume not self-open */

	if ((sp = strrchr(path, '/')) == NULL) {	/* last component */
		fname = path;
		dirname = ".";
	} else {
		*sp = '\0';
		fname = sp + 1;
		dirname = path;
	}

	if ((pid = strtol(fname, &next, 10)) < 0  /* filename not a number */
	 || *next != '\0'		/* filename not a number */
	 || !isprocdir(Pr, dirname)	/* file not in a /proc directory */
	 || pid == getpid()		/* process opened truss's /proc file */
	 || pid == 0) {			/* process opened process 0 */
		if (sp != NULL)
			*sp = '/';
		return rc;
	}

	if (sp != NULL)		/* restore the embedded '/' */
		*sp = '/';

	/* process did open a /proc file --- */

	if (pid == Pr->upid)	/* process opened its own /proc file */
		rc = TRUE;
	else {			/* send signal to controlling truss process */
		for (i = 0; i < sizeof(Cp->tpid)/sizeof(Cp->tpid[0]); i++) {
			if (Cp->spid[i] == pid) {
				pid = Cp->tpid[i];
				break;
			}
		}
		if (i >= sizeof(Cp->tpid)/sizeof(Cp->tpid[0]))
			err = 0;	/* don't attempt retry of open() */
		else {	/* wait for controlling process to terminate */
			while (pid && Cp->tpid[i] == pid) {
				if (kill(pid, SIGUSR1) == -1)
					break;
				timeout = TRUE;
				(void) alarm(1);
				(void) pause();
			}
			if (timeout) {
				(void) alarm(0);
				timeout = FALSE;
			}
			Ecritical(1);
			if (Cp->tpid[i] == 0)
				Cp->spid[i] = 0;
			Xcritical(1);
		}
	}

	if (err) {	/* prepare to reissue the open() system call */
		UnFlush();	/* don't print the failed open() */
		if (rc && !cflag && prismember(&trace,SYS_open)) { /* last gasp */
			(void) sysentry(Pr);
			(void) printf("%s%s\n", pname, sys_string);
			sys_leng = 0;
			*sys_string = '\0';
		}
		(void) Psetsysnum(Pr, SYS_open);

#ifdef i386
		Pr->REG[R_PC] -= 7;	/* sizeof syscall instruction */
#else
		Pr->REG[R_PC] -= 2;	/* sizeof syscall instruction */
#endif

		(void) Pputregs(Pr);
	}

	return rc;
}
