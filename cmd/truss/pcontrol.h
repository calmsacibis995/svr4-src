/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:pcontrol.h	1.2.3.1"

/* include file for process management */

/* requires:
 *	<stdio.h>
 *	<signal.h>
 *	<sys/types.h>
 *	<sys/fault.h>
 *	<sys/syscall.h>
 */
#include <sys/procfs.h>

#ifdef i386
#define R_1	9	/* EDX */
#define R_0	11	/* EAX */
#define	R_PC	14	/* EIP */
#define	R_PS	16	/* EFL */
#define	R_SP	17	/* UESP */
#endif

/*
 * If we are compiling with ANSI-C, define CONST to be const, else
 * define it to be empty.  Also, define VOID to be void, else char
 * (for things like malloc() and shmat()).  This relies on this header
 * file being the first (local) one included in all the .c files.
 */
#if	defined(__STDC__)
#	define	CONST	const
#	define	VOID	void
#else	/* defined(__STDC__) */
#	define	CONST
#	define	VOID	char
#endif	/* defined(__STDC__) */

#define	TRUE	1
#define	FALSE	0

/* definition of the process (program) table */
typedef struct {
	char	cntrl;		/* if TRUE then controlled process */
	char	child;		/* TRUE :: process created by fork() */
	char	state;		/* state of the process, see flags below */
	char	sig;		/* if dead, signal which caused it */
	char	rc;		/* exit code if process terminated normally */
	char	pfd;		/* /proc/<upid> filedescriptor */
	pid_t	upid;		/* UNIX process ID */
	prstatus_t why;		/* from /proc -- status values when stopped */
	long	sysaddr;	/* address of most recent syscall instruction */
	sigset_t sigmask;	/* signals which stop the process */
	fltset_t faultmask;	/* faults which stop the process */
	sysset_t sysentry;	/* system calls which stop process on entry */
	sysset_t sysexit;	/* system calls which stop process on exit */
	char	setsig;		/* set signal mask before continuing */
	char	sethold;	/* set signal hold mask before continuing */
	char	setfault;	/* set fault mask before continuing */
	char	setentry;	/* set sysentry mask before continuing */
	char	setexit;	/* set sysexit mask before continuing */
	char	setregs;	/* set registers before continuing */
} process_t;

extern CONST char * CONST regname[NGREG];	/* register name strings */

/* shorthand for register array */
#define	REG	why.pr_reg

/* state values */
#define	PS_NULL	0	/* no process in this table entry */
#define	PS_RUN	1	/* process running */
#define	PS_STEP	2	/* process running single stepped */
#define	PS_STOP	3	/* process stopped */
#define	PS_LOST	4	/* process lost to control (EAGAIN) */
#define	PS_DEAD	5	/* process terminated */

/* machine-specific stuff */

#ifdef i386
typedef	long	syscall_t;
#define	SYSCALL	0x9a000000000700
#define	ERRBIT	0x1
#endif

#if u3b
typedef	unsigned short	syscall_t;	/* holds a syscall instruction */
#define	SYSCALL	0xd900	/* value of syscall (OST) instruction */
#define	FRMSZ	13	/* frame size, in words */
#endif

#if u3b2 || u3b15
typedef	unsigned short	syscall_t;	/* holds a syscall instruction */
#define	SYSCALL	0x3061	/* value of syscall (GATE) instruction */
#define	FRMSZ	9	/* frame size, in words */
#define	ERRBIT	0x40000	/* bit in psw indicating syscall error */
#endif

#if mc68k
typedef	unsigned short	syscall_t;	/* holds a syscall instruction */
#define	SYSCALL	0x4e40	/* value of syscall (trap 0) instruction */
#define	ERRBIT	0x1	/* bit in psw indicating syscall error */
#endif

struct	argdes	{	/* argument descriptor for system call (Psyscall) */
	int	value;		/* value of argument given to system call */
	char *	object;		/* pointer to object in controlling process */
	char	type;		/* AT_BYVAL, AT_BYREF */
	char	inout;		/* AI_INPUT, AI_OUTPUT, AT_INOUT */
	short	len;		/* if AT_BYREF, length of object in bytes */
};

struct	sysret	{	/* return values from system call (Psyscall) */
	int	errno;		/* syscall error number */
	greg_t	r0;		/* %r0 from system call */
	greg_t	r1;		/* %r1 from system call */
};

/* values for type */
#define	AT_BYVAL	0
#define	AT_BYREF	1

/* values for inout */
#define	AI_INPUT	0
#define	AI_OUTPUT	1
#define	AI_INOUT	2

/* maximum number of syscall arguments */
#define	MAXARGS		8

/* maximum size in bytes of a BYREF argument */
#define	MAXARGL		(4*1024)


/* external data used by the package */
extern int debugflag;		/* for debugging */
extern char * procdir;		/* "/proc" */

/*
 * Function prototypes for routines in the process control package.
 */

#if	defined(__STDC__)

extern	int	Pcreate( process_t * , char ** );
extern	int	Pgrab( process_t * , pid_t , int );
extern	int	Preopen( process_t * , int );
extern	int	Prelease( process_t * );
extern	int	Pwait( process_t * );
extern	int	Pstop( process_t * );
extern	int	Pstatus( process_t * , int , unsigned );
extern	int	Pgetsysnum( process_t * );
extern	int	Psetsysnum( process_t * , int );
extern	int	Pgetregs( process_t * );
extern	int	Pgetareg( process_t * , int );
extern	int	Pputregs( process_t * );
extern	int	Pputareg( process_t * , int );
extern	int	Psetrun( process_t * , int , int );
extern	int	Pstart( process_t * , int );
extern	int	Pterm( process_t * );
extern	int	Pread( process_t * , long , char * , int );
extern	int	Pwrite( process_t * , long , CONST char * , int );
extern	int	Psignal( process_t * , int , int );
extern	int	Pfault( process_t * , int , int );
extern	int	Psysentry( process_t * , int , int );
extern	int	Psysexit( process_t * , int , int );
extern	struct sysret	Psyscall( process_t * , int , int , struct argdes * );
extern	int	Ioctl( int , int , int );
extern	int	is_empty( CONST long * , unsigned );

#else	/* defined(__STDC__) */

extern	int	Pcreate();
extern	int	Pgrab();
extern	int	Preopen();
extern	int	Prelease();
extern	int	Pwait();
extern	int	Pstop();
extern	int	Pstatus();
extern	int	Pgetsysnum();
extern	int	Psetsysnum();
extern	int	Pgetregs();
extern	int	Pgetareg();
extern	int	Pputregs();
extern	int	Pputareg();
extern	int	Psetrun();
extern	int	Pstart();
extern	int	Pterm();
extern	int	Pread();
extern	int	Pwrite();
extern	int	Psignal();
extern	int	Pfault();
extern	int	Psysentry();
extern	int	Psysexit();
extern	struct sysret	Psyscall();
extern	int	Ioctl();
extern	int	is_empty();

#endif	/* defined(__STDC__) */

/*
 * Test for empty set.
 * is_empty() should not be called directly.
 */
#define	isemptyset(sp)	is_empty((long *)(sp), sizeof(*(sp))/sizeof(long))
