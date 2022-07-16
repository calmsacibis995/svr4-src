/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_PROCFS_H
#define _SYS_PROCFS_H

#ident	"@(#)head.sys:sys/procfs.h	1.24.4.1"

#include <sys/types.h>
#include <sys/time.h>
#include <sys/regset.h>
#include <sys/tss.h>
#include <sys/signal.h>
#include <sys/siginfo.h>
#include <sys/ucontext.h>
#include <sys/fault.h>
#include <sys/syscall.h>

/*
 * ioctl codes and system call interfaces for /proc.
 */

#define	PIOC		('q'<<8)
#define	PIOCSTATUS	(PIOC|1)	/* get process status */
#define	PIOCSTOP	(PIOC|2)	/* post STOP request and... */
#define	PIOCWSTOP	(PIOC|3)	/* wait for process to STOP */
#define	PIOCRUN		(PIOC|4)	/* make process runnable */
#define	PIOCGTRACE	(PIOC|5)	/* get traced signal set */
#define	PIOCSTRACE	(PIOC|6)	/* set traced signal set */
#define	PIOCSSIG	(PIOC|7)	/* set current signal */
#define	PIOCKILL	(PIOC|8)	/* send signal */
#define	PIOCUNKILL	(PIOC|9)	/* delete a signal */
#define	PIOCGHOLD	(PIOC|10)	/* get held signal set */
#define	PIOCSHOLD	(PIOC|11)	/* set held signal set */
#define	PIOCMAXSIG	(PIOC|12)	/* get max signal number */
#define	PIOCACTION	(PIOC|13)	/* get signal action structs */
#define	PIOCGFAULT	(PIOC|14)	/* get traced fault set */
#define	PIOCSFAULT	(PIOC|15)	/* set traced fault set */
#define	PIOCCFAULT	(PIOC|16)	/* clear current fault */
#define	PIOCGENTRY	(PIOC|17)	/* get syscall entry set */
#define	PIOCSENTRY	(PIOC|18)	/* set syscall entry set */
#define	PIOCGEXIT	(PIOC|19)	/* get syscall exit set */
#define	PIOCSEXIT	(PIOC|20)	/* set syscall exit set */
#define	PIOCSFORK	(PIOC|21)	/* set inherit-on-fork flag */
#define	PIOCRFORK	(PIOC|22)	/* reset inherit-on-fork flag */
#define	PIOCSRLC	(PIOC|23)	/* set run-on-last-close flag */
#define	PIOCRRLC	(PIOC|24)	/* reset run-on-last-close flag */
#define	PIOCGREG	(PIOC|25)	/* get general registers */
#define	PIOCSREG	(PIOC|26)	/* set general registers */
#define	PIOCGFPREG	(PIOC|27)	/* get floating-point registers */
#define	PIOCSFPREG	(PIOC|28)	/* set floating-point registers */
#define	PIOCNICE	(PIOC|29)	/* set nice priority */
#define	PIOCPSINFO	(PIOC|30)	/* get ps(1) information */
#define	PIOCNMAP	(PIOC|31)	/* get number of memory mappings */
#define	PIOCMAP		(PIOC|32)	/* get memory map information */
#define	PIOCOPENM	(PIOC|33)	/* open mapped object for reading */
#define	PIOCCRED	(PIOC|34)	/* get process credentials */
#define	PIOCGROUPS	(PIOC|35)	/* get supplementary groups */
#define	PIOCGETPR	(PIOC|36)	/* read struct proc */
#define	PIOCGETU	(PIOC|37)	/* read user area */
#define	PIOCGDBREG	(PIOC|40)	/* get debug registers */
#define	PIOCSDBREG	(PIOC|41)	/* set debug registers */

/* Holds one 3B2 instruction op code */

typedef	char	instr_t;

/* Process status structure */

typedef struct prstatus {
	long	pr_flags;	/* Process flags */
	short	pr_why;		/* Reason for process stop (if stopped) */
	short	pr_what;	/* More detailed reason */
	siginfo_t pr_info;	/* Info associated with signal or fault */
	short	pr_cursig;	/* Current signal */
	short	pr_pad;		/* pad to long boundary */
	sigset_t pr_sigpend;	/* Set of other pending signals */
	sigset_t pr_sighold;	/* Set of of held signals */
	struct	sigaltstack pr_altstack; /* Alternate signal stack info */
	struct	sigaction pr_action; /* Signal action for current signal */
	pid_t	pr_pid;		/* Process id */
	pid_t	pr_ppid;	/* Parent process id */
	pid_t	pr_pgrp;	/* Process group id */
	pid_t	pr_sid;		/* Session id */
	timestruc_t pr_utime;	/* Process user cpu time */
	timestruc_t pr_stime;	/* Process system cpu time */
	timestruc_t pr_cutime;	/* Sum of children's user times */
	timestruc_t pr_cstime;	/* Sum of children's system times */
	char	pr_clname[8];	/* Scheduling class name */
	long	pr_filler[20];	/* Filler area for future expansion */
	long	pr_instr;	/* Current instruction */
	gregset_t pr_reg;	/* General registers */
} prstatus_t;

/* Process status flags */

#define	PR_STOPPED	0x0001	/* Process is stopped */
#define	PR_ISTOP	0x0002	/* Process stopped on an event of interest */
#define	PR_DSTOP	0x0004	/* A stop directive is in effect */
#define	PR_ASLEEP	0x0008	/* Process is sleep()ing in a system call */
#define	PR_FORK		0x0010	/* Inherit-on-fork is in effect */
#define	PR_RLC		0x0020	/* Run-on-last-close is in effect */
#define	PR_PTRACE	0x0040	/* Process is being controlled by ptrace(2) */
#define	PR_PCINVAL	0x0080	/* %pc refers to an invalid virtual address */
#define	PR_ISSYS	0x0100	/* System process */

/* Reasons for stopping */

#define	PR_REQUESTED	1
#define	PR_SIGNALLED	2
#define	PR_SYSENTRY	3
#define	PR_SYSEXIT	4
#define	PR_JOBCONTROL	5
#define	PR_FAULTED	6

/* Information for the ps(1) command */

#define	PRARGSZ		80		/* Number of chars of arguments */

typedef struct prpsinfo {
	char	pr_state;	/* numeric process state (see pr_sname) */
	char	pr_sname;	/* printable character representing pr_state */
	char	pr_zomb;	/* !=0: process terminated but not waited for */
	char	pr_nice;	/* nice for cpu usage */
	u_long	pr_flag;	/* process flags */
	uid_t	pr_uid;		/* real user id */
	gid_t	pr_gid;		/* real group id */
	pid_t	pr_pid;		/* unique process id */
	pid_t	pr_ppid;	/* process id of parent */
	pid_t	pr_pgrp;	/* pid of process group leader */
	pid_t	pr_sid;		/* session id */
	caddr_t	pr_addr;	/* physical address of process */
	long	pr_size;	/* size of process image in pages */
	long	pr_rssize;	/* resident set size in pages */
	caddr_t	pr_wchan;	/* wait addr for sleeping process */
	timestruc_t pr_start;	/* process start time, sec+nsec since epoch */
	timestruc_t pr_time;	/* usr+sys cpu time for this process */
	long	pr_pri;		/* priority, high value is high priority */
	char	pr_oldpri;	/* pre-SVR4, low value is high priority */
	char	pr_cpu;		/* pre-SVR4, cpu usage for scheduling */
	o_dev_t	pr_ottydev;	/* short tty device number */
	dev_t	pr_lttydev;	/* controlling tty device (PRNODEV if none) */
	char	pr_clname[8];	/* Scheduling class name */
	char	pr_fname[16];	/* last component of exec()ed pathname */
	char	pr_psargs[PRARGSZ];	/* initial characters of arg list */
	long	pr_filler[20];	/* for future expansion */
} prpsinfo_t;

#if !defined(_STYPES)
#define	pr_ttydev	pr_lttydev
#else
#define	pr_ttydev	pr_ottydev
#endif	

#define	PRNODEV	(dev_t)(-1)	/* non-existent device */

/* Optional actions to take when process continues */

typedef struct prrun {
	long	pr_flags;	/* Flags */
	sigset_t pr_trace;	/* Set of signals to be traced */
	sigset_t pr_sighold;	/* Set of signals to be held */
	fltset_t pr_fault;	/* Set of faults to be traced */
	caddr_t	pr_vaddr;	/* Virtual address at which to resume */
	long	pr_filler[8];	/* Filler area for future expansion */
} prrun_t;

#define	PRCSIG		0x001	/* Clear current signal */
#define	PRCFAULT	0x002	/* Clear current fault */
#define	PRSTRACE	0x004	/* Use traced-signal set in pr_trace */
#define	PRSHOLD		0x008	/* Use held-signal set in pr_sighold */
#define	PRSFAULT	0x010	/* Use traced-fault set in pr_fault */
#define	PRSVADDR	0x020	/* Resume at virtual address in pr_vaddr */
#define	PRSTEP		0x040	/* Single-step the process */
#define	PRSABORT	0x080	/* Abort syscall */
#define	PRSTOP		0x100	/* Set directed stop request */

/* Memory-management interface */

typedef struct prmap {
	caddr_t		pr_vaddr;	/* Virtual address base */
	u_long		pr_size;	/* Size of mapping in bytes */
	off_t		pr_off;		/* Offset into mapped object, if any */
	long		pr_mflags;	/* Protection and attribute flags */
	long		pr_filler[4];	/* Filler for future expansion */
} prmap_t;

/* Protection and attribute flags */

#define	MA_READ		0x04	/* Readable by the traced process */
#define	MA_WRITE	0x02	/* Writable by the traced process */
#define	MA_EXEC		0x01	/* Executable by the traced process */
#define	MA_SHARED	0x08	/* Changes are shared by mapped object */
#define	MA_BREAK	0x10	/* Grown by brk(2) */
#define	MA_STACK	0x20	/* Grown automatically on stack faults */

/* Process credentials */

typedef struct prcred {
	uid_t	pr_euid;	/* Effective user id */
	uid_t	pr_ruid;	/* Real user id */
	uid_t	pr_suid;	/* Saved user id (from exec) */
	gid_t	pr_egid;	/* Effective group id */
	gid_t	pr_rgid;	/* Real group id */
	gid_t	pr_sgid;	/* Saved group id (from exec) */
	u_int	pr_ngroups;	/* Number of supplementary groups */
} prcred_t;

/*
 * Macros for manipulating sets of flags.
 * sp must be a pointer to one of sigset_t, fltset_t, or sysset_t.
 * flag must be a member of the enumeration corresponding to *sp.
 */

/* turn on all flags in set */
#define	prfillset(sp) \
	{ register int _i_ = sizeof(*(sp))/sizeof(u_long); \
		while(_i_) ((u_long*)(sp))[--_i_] = 0xFFFFFFFF; }

/* turn off all flags in set */
#define	premptyset(sp) \
	{ register int _i_ = sizeof(*(sp))/sizeof(u_long); \
		while(_i_) ((u_long*)(sp))[--_i_] = 0L; }

/* turn on specified flag in set */
#define	praddset(sp, flag) \
	(((unsigned)((flag)-1) < 32*sizeof(*(sp))/sizeof(u_long)) ? \
	(((u_long*)(sp))[((flag)-1)/32] |= (1L<<(((flag)-1)%32))) : 0)

/* turn off specified flag in set */
#define	prdelset(sp, flag) \
	(((unsigned)((flag)-1) < 32*sizeof(*(sp))/sizeof(u_long)) ? \
	(((u_long*)(sp))[((flag)-1)/32] &= ~(1L<<(((flag)-1)%32))) : 0)

/* query: != 0 iff flag is turned on in set */
#define	prismember(sp, flag) \
	(((unsigned)((flag)-1) < 32*sizeof(*(sp))/sizeof(u_long)) \
	&& (((u_long*)(sp))[((flag)-1)/32] & (1L<<(((flag)-1)%32))))


#endif	/* _SYS_PROCFS_H */
