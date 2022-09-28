/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:ramdata.h	1.3.3.1"

/* ramdata.h -- read/write data declarations */

/* requires:
	<stdio.h>
	<signal.h>
	<sys/types.h>
	<sys/fault.h>
	<sys/syscall.h>
	<sys/param.h>
	"pcontrol.h"
*/

#ifdef NOFILES_MAX
#	undef NOFILES_MAX
#endif
#define	NOFILES_MAX	512

typedef struct {		/* set type for possible filedescriptors */
	long	word[(NOFILES_MAX+31)/32];
} fileset_t;

/* maximum sizes of things */
#define	PRMAXSIG	(32*sizeof(sigset_t)/sizeof(long))
#define	PRMAXFAULT	(32*sizeof(fltset_t)/sizeof(long))
#define	PRMAXSYS	(32*sizeof(sysset_t)/sizeof(long))
#define	PRMAXFILE	(32*sizeof(fileset_t)/sizeof(long))

/* last stop state enumeration (used by signalled() and requested()) */
#define	SLEEPING	1
#ifdef PTRACED
#undef PTRACED
#define	PTRACED		2
#endif
#define	JOBSIG		3
#define	JOBSTOP		4

extern	char *	command;	/* name of command ("truss") */
extern	int	length;		/* length of printf() output so far */
extern	pid_t	child;		/* pid of fork()ed child process */
extern	char	pname[8];	/* formatted pid of controlled process */
extern	int	interrupt;	/* interrupt signal was received */
extern	int	sigusr1;	/* received SIGUSR1 (release process) */
extern	int	sigsys;		/* received SIGSYS (no semaphores or shmem) */
extern	pid_t	created;	/* if process was created, its process id */
extern	int	Errno;		/* errno for controlled process's syscall */
extern	int	Rval1;		/* rval1 (%r0) for syscall */
extern	int	Rval2;		/* rval2 (%r1) for syscall */
extern	uid_t	Euid;		/* truss's effective uid */
extern	uid_t	Egid;		/* truss's effective gid */
extern	uid_t	Ruid;		/* truss's real uid */
extern	uid_t	Rgid;		/* truss's real gid */
extern	prcred_t credentials;	/* traced process credentials */
extern	int	istty;		/* TRUE iff output is a tty */
extern	int	wasptraced;	/* TRUE iff process is being ptrace()d */

extern	int	Fflag;		/* option flags from getopt() */
extern	int	qflag;
extern	int	fflag;
extern	int	cflag;
extern	int	aflag;
extern	int	eflag;
extern	int	iflag;
extern	int	tflag;
extern	int	pflag;
extern	int	sflag;
extern	int	mflag;
extern	int	oflag;
extern	int	vflag;
extern	int	xflag;

extern	sysset_t trace;		/* sys calls to trace */
extern	sysset_t traceeven;	/* sys calls to trace even if not reported */
extern	sysset_t verbose;	/* sys calls to be verbose about */
extern	sysset_t rawout;	/* sys calls to show in raw mode */
extern	sigset_t signals;	/* signals to trace */
extern	fltset_t faults;	/* faults to trace */
extern	fileset_t readfd;	/* read() file descriptors to dump */
extern	fileset_t writefd;	/* write() file descriptors to dump */

struct counts {		/* structure for keeping counts */
	long sigcount[PRMAXSIG+1];	/* signals count [0..PRMAXSIG] */
	long fltcount[PRMAXFAULT+1];	/* faults count [0..MAXFAULT] */
	long syscount[PRMAXSYS+1];	/* sys calls count [0..PRMAXSYS] */
	long syserror[PRMAXSYS+1];	/* sys calls returning error */
	timestruc_t systime[PRMAXSYS+1]; /* time spent in sys call */
	timestruc_t systotal;		/* total time spent in kernel */
	timestruc_t usrtotal;		/* total time spent in user mode */
		/* the following is for internal control */
	int serialize;		/* != 0 : serialize output using semaphores */
	int nonserial;		/* count of outstanding non-serialized writes */
	pid_t tpid[500];	/* truss process pid */
	pid_t spid[500];	/* subject process pid */
};

extern	struct counts * Cp;	/* for counting: malloc() or shared memory */
extern	timestruc_t sysbegin;	/* initial value of stime */
extern	timestruc_t syslast;	/* most recent value of stime */
extern	timestruc_t usrbegin;	/* initial value of utime */
extern	timestruc_t usrlast;	/* most recent value of utime */

extern	int	shmid;		/* shared memory identifier */
extern	int	semid;		/* semaphore identifier */

extern	pid_t	ancestor;	/* top-level parent process id */
extern	int	descendent;	/* TRUE iff descendent of top level */

extern	int	sys_args[8];	/* the arguments to last syscall */
extern	int	sys_nargs;	/* number of arguments to last syscall */

extern	char	sys_name[12];	/* name of unknown system call */
extern	char	sig_name[12];	/* name of unknown signal */
extern	char	flt_name[12];	/* name of unknown fault */

extern	char *	sys_path;	/* first pathname given to syscall */
extern	unsigned sys_psize;	/* sizeof(*sys_path) */
extern	int	sys_valid;	/* pathname was fetched and is valid */

extern	char *	sys_string;	/* buffer for formatted syscall string */
extern	unsigned sys_ssize;	/* sizeof(*sys_string) */
extern	unsigned sys_leng;	/* strlen(sys_string) */

extern	char *	str_buffer;	/* fetchstring() buffer */
extern	unsigned str_bsize;	/* sizeof(*str_buffer) */

#define	IOBSIZE	12		/* number of bytes shown by prt_iob() */
extern	char iob_buf[2*IOBSIZE+8];	/* where prt_iob() leaves its stuff */

extern	char	code_buf[100];	/* for symbolic arguments, e.g., ioctl codes */

#define MAXGRAB	128		/* max number of grabbed processes */
extern	int	ngrab;		/* number of pid's to grab */
extern	pid_t	grab[MAXGRAB];	/* process id's to grab */
extern	char *	grabdir[MAXGRAB];	/* path prefix for grabbed process */

extern	process_t	Proc;	/* the process structure */
extern	process_t *	PR;	/* pointer to same (for abend()) */

extern	int	debugflag;	/* for debugging */
extern	char *	procdir;	/* default PROC directory */
extern	sigset_t psigs;		/* pending signals (used by Psyscall()) */

extern	int	recur;		/* show_strioctl() -- to prevent recursion */

extern	int	no_inherit;	/* set TRUE iff ioctl(PIOC[RS]FORK) failed */
extern	int	timeout;	/* set TRUE by SIGALRM catchers */
