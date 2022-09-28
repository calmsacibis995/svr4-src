/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:ramdata.c	1.3.3.1"

#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "proto.h"

/* ramdata.c -- read/write data definitions for process management */

char *	command = NULL;		/* name of command ("truss") */
int	length = 0;		/* length of printf() output so far */
pid_t	child = 0;		/* pid of fork()ed child process */
char	pname[8] = "";		/* formatted pid of controlled process */
int	interrupt = FALSE;	/* interrupt signal was received */
int	sigusr1 = FALSE;	/* received SIGUSR1 (release process) */
int	sigsys = FALSE;		/* received SIGSYS (no semaphores or shmem) */
pid_t	created = 0;		/* if process was created, its process id */
int	Errno = 0;		/* errno for controlled process's syscall */
int	Rval1 = 0;		/* rval1 (%r0) for syscall */
int	Rval2 = 0;		/* rval2 (%r1) for syscall */
uid_t	Euid = 0;		/* truss's effective uid */
uid_t	Egid = 0;		/* truss's effective gid */
uid_t	Ruid = 0;		/* truss's real uid */
uid_t	Rgid = 0;		/* truss's real gid */
prcred_t credentials;		/* traced process credentials */
int	istty = FALSE;		/* TRUE iff output is a tty */
int	wasptraced = FALSE;	/* TRUE iff process is being ptrace()d */

int	Fflag = FALSE;		/* option flags from getopt() */
int	qflag = FALSE;
int	fflag = FALSE;
int	cflag = FALSE;
int	aflag = FALSE;
int	eflag = FALSE;
int	iflag = FALSE;
int	tflag = FALSE;
int	pflag = FALSE;
int	sflag = FALSE;
int	mflag = FALSE;
int	oflag = FALSE;
int	vflag = FALSE;
int	xflag = FALSE;

sysset_t trace;			/* sys calls to trace */
sysset_t traceeven;		/* sys calls to trace even if not reported */
sysset_t verbose;		/* sys calls to be verbose about */
sysset_t rawout;		/* sys calls to show in raw mode */
sigset_t signals;		/* signals to trace */
fltset_t faults;		/* faults to trace */

fileset_t readfd;		/* read() file descriptors to dump */
fileset_t writefd;		/* write() file descriptors to dump */

struct counts * Cp = NULL;	/* for counting: malloc() or shared memory */
timestruc_t sysbegin;		/* initial value of stime */
timestruc_t syslast;		/* most recent value of stime */
timestruc_t usrbegin;		/* initial value of utime */
timestruc_t usrlast;		/* most recent value of utime */

int	shmid = -1;		/* shared memory identifier */
int	semid = -1;		/* semaphore identifier */

pid_t	ancestor = 0;		/* top-level parent process id */
int	descendent = FALSE;	/* TRUE iff descendent of top level */

int	sys_args[8];		/* the arguments to last syscall */
int	sys_nargs = 0;		/* number of arguments to last syscall */

char	sys_name[12] = "sys#ddd";/* name of unknown system call */
char	sig_name[12] = "SIG#dd";/* name of unknown signal */
char	flt_name[12] = "FLT#dd";/* name of unknown fault */

char *	sys_path = NULL;	/* first pathname given to syscall */
unsigned sys_psize = 0;		/* sizeof(*sys_path) */
int	sys_valid = FALSE;	/* pathname was fetched and is valid */

char *	sys_string = NULL;	/* buffer for formatted syscall string */
unsigned sys_ssize = 0;		/* sizeof(*sys_string) */
unsigned sys_leng = 0;		/* strlen(sys_string) */

char *	str_buffer = NULL;	/* fetchstring() buffer */
unsigned str_bsize = 0;		/* sizeof(*str_buffer) */

char iob_buf[2*IOBSIZE+8];	/* where prt_iob() leaves its stuff */

char	code_buf[100];		/* for symbolic arguments, e.g., ioctl codes */

int	ngrab = 0;		/* number of pid's to grab */
pid_t	grab[MAXGRAB];		/* process id's to grab */
char *	grabdir[MAXGRAB];	/* path prefix for grabbed process */

process_t	Proc;		/* the process structure */
process_t *	PR = NULL;	/* pointer to same (for abend()) */

int	debugflag = FALSE;	/* for debugging */
char *	procdir = "/proc";	/* default PROC directory */
sigset_t psigs;			/* pending signals (used by Psyscall()) */

int	recur = 0;		/* show_strioctl() -- to prevent recursion */

int	no_inherit = FALSE;	/* set TRUE iff ioctl(PIOC[RS]FORK) failed */
int	timeout = FALSE;	/* set TRUE by SIGALRM catchers */
