/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)csh:sh.proc.h	1.2.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley Software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * C shell - process structure declarations
 */

/*
 * Structure for each process the shell knows about:
 *	allocated and filled by pcreate.
 *	flushed by pflush; freeing always happens at top level
 *	    so the interrupt level has less to worry about.
 *	processes are related to "friends" when in a pipeline;
 *	    p_friends links makes a circular list of such jobs
 */
struct process	{
	struct	process *p_next;	/* next in global "proclist" */
	struct	process	*p_friends;	/* next in job list (or self) */
	struct	directory *p_cwd;	/* cwd of the job (only in head) */
	short	unsigned p_flags;	/* various job status flags */
 tchar p_reason;		/* reason for entering this state */
 tchar p_index;		/* shorthand job index */
	int	P_pid;
	int	p_jobid;		/* pid of job leader */
	/* if a job is stopped/background p_jobid gives its pgrp */
	struct	timeval p_btime;	/* begin time */
	struct	timeval p_etime;	/* end time */
	struct	rusage p_rusage;
 tchar *p_command;		/* first PMAXLEN chars of command */
};

/* flag values for p_flags */
#define	PRUNNING	(1<<0)		/* running */
#define	PSTOPPED	(1<<1)		/* stopped */
#define	PNEXITED	(1<<2)		/* normally exited */
#define	PAEXITED	(1<<3)		/* abnormally exited */
#define	PSIGNALED	(1<<4)		/* terminated by a signal != SIGINT */

#define	PALLSTATES	(PRUNNING|PSTOPPED|PNEXITED|PAEXITED|PSIGNALED|PINTERRUPTED)
#define	PNOTIFY		(1<<5)		/* notify async when done */
#define	PTIME		(1<<6)		/* job times should be printed */
#define	PAWAITED	(1<<7)		/* top level is waiting for it */
#define	PFOREGND	(1<<8)		/* started in shells pgrp */
#define	PDUMPED		(1<<9)		/* process dumped core */
#define	PDIAG		(1<<10)		/* diagnostic output also piped out */
#define	PPOU		(1<<11)		/* piped output */
#define	PREPORTED	(1<<12)		/* status has been reported */
#define	PINTERRUPTED	(1<<13)		/* job stopped via interrupt signal */
#define	PPTIME		(1<<14)		/* time individual process */
#define	PNEEDNOTE	(1<<15)		/* notify as soon as practical */

#define	PNULL		(struct process *)0
#define	PMAXLEN		80

/* defines for arguments to pprint */
#define	NUMBER		01
#define	NAME		02
#define	REASON		04
#define	AMPERSAND	010
#define	FANCY		020
#define	SHELLDIR	040		/* print shell's dir if not the same */
#define	JOBDIR		0100		/* print job's dir if not the same */
#define	AREASON		0200

struct	process	proclist;		/* list head of all processes */
bool	pnoprocesses;			/* pchild found nothing to wait for */

struct	process *pholdjob;		/* one level stack of current jobs */

struct	process *pcurrjob;		/* current job */
struct	process	*pcurrent;		/* current job in table */
struct	process *pprevious;		/* previous job in table */

short	pmaxindex;			/* current maximum job index */

int	psigint();
struct	process	*pgetcurr();
struct	process	*plookup();
struct	process *pfind();
