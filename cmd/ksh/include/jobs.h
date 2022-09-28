/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/jobs.h	1.2.3.1"
#define LOBYTE	0377
#define MAXMSG	25
#define JOBTTY	2

#include	"sh_config.h"
#ifdef JOBS
#   include	"terminal.h"
#   undef ESCAPE

#   ifdef FIOLOOKLD
	/* Ninth edition */
	extern int tty_ld, ntty_ld;
#	define OTTYDISC	tty_ld
#	define NTTYDISC	ntty_ld
#	define killpg(pgrp,sig)		kill(-(pgrp),sig)
#   endif	/* FIOLOOKLD */
#   define job_nonstop()	(st.states &= ~MONITOR)
#   define job_terminate()	job_walk(job_kill,SIGHUP,(char**)0)
			 /* send a hang-up signal to each of the jobs */
#else
#   undef SIGTSTP
#   undef SIGCHLD
#   undef SIGCLD
#   undef MONITOR
#   define MONITOR	0
#   define job_nonstop()
#   define job_set(x)
#   define job_reset(x)
#endif	/* JOBS */

#ifdef CHILD_MAX
#   define MAXJ	(CHILD_MAX-1)
#else
#   if u370 || uts
#	define MAXJ	75
#   else
#	define MAXJ	32
#   endif /* u370 */
#endif /* CHILD_MAX */

/* JBYTES is the number of char's needed for MAXJ bits */
#define JBYTES		(1+((MAXJ-1)/(8)))

struct process
{
	struct process *p_nxtjob;	/* next job structure */
	struct process *p_nxtproc;	/* next process in current job */
	pid_t		p_pid;		/* process id */
	pid_t		p_pgrp;		/* process group */
	unsigned char	p_job;		/* job number of process */
	pid_t		p_fgrp;		/* process group when stopped */
	unsigned char	p_flag;		/* flags - see below */
	unsigned short	p_exit;		/* exit value or signal number */
#ifdef JOBS
	off_t		p_name;		/* history file offset for command */
	struct termios	p_stty;		/* terminal state for job */
#endif /* JOBS */
};

/* Process states */

#define P_RUNNING	01
#define P_STOPPED	02
#define P_NOTIFY	04
#define P_SIGNALLED	010
#define P_STTY		020
#define P_DONE		040
#define P_COREDUMP	0100
#define P_BYNUM		0200

struct jobs
{
	struct process	*pwlist;	/* head of process list */
	pid_t		curpgid;	/* current process gid id */
	int		topfd;		/* fd of tty for pipelines  */
	pid_t		parent;		/* set by fork() */
	pid_t		mypid;		/* process id of shell */
	pid_t		mypgid;		/* process group id of shell */
	pid_t		mytgid;		/* terminal group id of shell */
	int		numpost;	/* number of posted jobs */
#ifdef JOBS
	int		suspend;	/* suspend character */
	int		linedisc;	/* line dicipline */
	char		jobcontrol;	/* turned on for real job control */
#endif /* JOBS */
	char		pipeflag;	/* set for pipelines */
	char		waitsafe;	/* wait will not block */
	unsigned char	freejobs[JBYTES];	/* free jobs numbers */
};

extern struct jobs job;

#ifdef JOBS
extern const char	e_jobusage[];
extern const char	e_kill[];
extern const char	e_Done[];
extern const char	e_Running[];
extern const char	e_coredump[];
extern const char	e_killcolon[];
extern const char	e_no_proc[];
extern const char	e_no_job[];
extern const char	e_running[];
extern const char	e_nlspace[];
extern const char	e_ambiguous[];
#ifdef SIGTSTP
   extern const char	e_no_start[];
   extern const char	e_terminate[];
   extern const char	e_no_jctl[];
#endif /* SIGTSTP */
#ifdef NTTYDISC
   extern const char	e_newtty[];
   extern const char	e_oldtty[];
#endif /* NTTYDISC */
#endif	/* JOBS */

/*
 * The following are defined in jobs.c
 */

#ifdef PROTO
    extern void job_clear(void);
    extern void job_bwait(char*[]);
    extern int	job_walk(int(*)(),int,char*[]);
    extern int	job_kill(struct process*,int);
#else
    extern void job_clear();
    extern int	job_post();
    extern void job_bwait();
    extern int	job_walk();
    extern int	job_kill();
#endif /* PROTO */
extern void	job_wait();
extern int	job_post();

#ifdef JOBS
#   ifdef PROTO
	extern void	job_init(int);
	extern int	job_close(void);
	extern int	job_list(struct process*,int);
#   else
	extern void	job_init();
	extern int	job_close();
	extern int	job_list();
#   endif /* PROTO */
#else
#	define job_init(flag)
#	define job_close()	(0)
#endif	/* JOBS */

#ifdef SIGTSTP
#   ifdef PROTO
	extern int	job_switch(struct process*,int);
#   else
	extern int	job_switch();
#   endif /* PROTO */
#endif /* SIGTSTP */
