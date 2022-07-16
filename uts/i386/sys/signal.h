/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H

#ident	"@(#)head.sys:sys/signal.h	11.44.3.1"

#define	SIGHUP	1	/* hangup */
#define	SIGINT	2	/* interrupt (rubout) */
#define	SIGQUIT	3	/* quit (ASCII FS) */
#define	SIGILL	4	/* illegal instruction (not reset when caught) */
#define	SIGTRAP	5	/* trace trap (not reset when caught) */
#define	SIGIOT	6	/* IOT instruction */
#define SIGABRT 6	/* used by abort, replace SIGIOT in the future */
#define	SIGEMT	7	/* EMT instruction */
#define	SIGFPE	8	/* floating point exception */
#define	SIGKILL	9	/* kill (cannot be caught or ignored) */
#define	SIGBUS	10	/* bus error */
#define	SIGSEGV	11	/* segmentation violation */
#define	SIGSYS	12	/* bad argument to system call */
#define	SIGPIPE	13	/* write on a pipe with no one to read it */
#define	SIGALRM	14	/* alarm clock */
#define	SIGTERM	15	/* software termination signal from kill */
#define	SIGUSR1	16	/* user defined signal 1 */
#define	SIGUSR2	17	/* user defined signal 2 */
#define	SIGCLD	18	/* child status change */
#define	SIGCHLD	18	/* child status change alias (POSIX) */
#define	SIGPWR	19	/* power-fail restart */
#define SIGWINCH 20	/* window size change */
#define SIGURG	21	/* urgent socket condition */
#define SIGPOLL 22	/* pollable event occured */
#define SIGIO	22	/* socket I/O possible (SIGPOLL alias) */
#define SIGSTOP 23	/* stop (cannot be caught or ignored) */
#define SIGTSTP 24	/* user stop requested from tty */
#define SIGCONT 25	/* stopped process has been continued */
#define SIGTTIN 26	/* background tty read attempted */
#define SIGTTOU 27	/* background tty write attempted */
#define SIGVTALRM 28	/* virtual timer expired */
#define SIGPROF 29	/* profiling timer expired */
#define SIGXCPU 30	/* exceeded cpu limit */
#define SIGXFSZ 31	/* exceeded file size limit */

#define	SIG_DFL	(void(*)())0

#if defined(lint)
#define SIG_ERR (void(*)())0
#define	SIG_IGN	(void (*)())0
#define SIG_HOLD (void(*)())0
#else
#define SIG_ERR	(void(*)())-1
#define	SIG_IGN	(void (*)())1
#define SIG_HOLD (void(*)())2
#endif

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

#define SIGNO_MASK	0xFF
#define SIGDEFER	0x100
#define SIGHOLD		0x200
#define SIGRELSE	0x400
#define SIGIGNORE	0x800
#define SIGPAUSE	0x1000

#if (__STDC__ - 0 == 0) || defined(_POSIX_SOURCE)
typedef struct {		/* signal set type */
	unsigned long	sigbits[4];
} sigset_t;

struct sigaction {
	int sa_flags;
	void (*sa_handler)();
	sigset_t sa_mask;
	int sa_resv[2];
};

/* these are only valid for SIGCLD */
#define SA_NOCLDSTOP	0x00020000	/* don't send job control SIGCLD's */
#endif

#if (__STDC__ - 0 == 0) && !defined(_POSIX_SOURCE)
			/* non-comformant ANSI compilation	*/

/* definitions for the sa_flags field */
#define SA_ONSTACK	0x00000001
#define SA_RESETHAND	0x00000002
#define SA_RESTART	0x00000004
#define SA_SIGINFO	0x00000008
#define SA_NODEFER	0x00000010

/* these are only valid for SIGCLD */
#define SA_NOCLDWAIT	0x00010000	/* don't save zombie children	 */

#define NSIG	32	/* valid signals range from 1 to NSIG-1 */
#define MAXSIG	32	/* size of u_signal[], NSIG-1 <= MAXSIG */

#define S_SIGNAL	1
#define S_SIGSET	2
#define S_SIGACTION	3
#define S_NONE		4

#define MINSIGSTKSZ	512
#define SIGSTKSZ	8192

#define SS_ONSTACK	0x00000001
#define SS_DISABLE	0x00000002

struct sigaltstack {
	char	*ss_sp;
	int	ss_size;
	int	ss_flags;
};

typedef struct sigaltstack stack_t;

#endif

#ifdef _KERNEL 

extern k_sigset_t	

	fillset,		/* valid signals, guaranteed contiguous */
	holdvfork,		/* held while doing vfork */
	cantmask,		/* cannot be caught or ignored */
	cantreset,		/* cannot be reset after catching */
	ignoredefault,		/* ignored by default */
	stopdefault,		/* stop by default */
	coredefault;		/* dumps core by default */

#define	sigmask(n)		((unsigned long)1 << ((n) - 1))

#define sigemptyset(s)		(*(s) = 0)
#define sigfillset(s)		(*(s) = fillset)
#define sigaddset(s,n)		(*(s) |= sigmask(n))
#define sigdelset(s,n)		(*(s) &= ~sigmask(n))
#define	sigismember(s,n)	(sigmask(n) & *(s))
#if !defined(_POSIX_SOURCE) 
#define sigisempty(s)		(*(s) == 0)
#define sigorset(s1,s2)		(*(s1) |= *(s2))
#define	sigandset(s1,s2)	(*(s1) &= *(s2))
#define	sigdiffset(s1,s2)	(*(s1) &= ~(*(s2)))
#define sigutok(us,ks)		(*(ks) = (us)->sigbits[0])
#define sigktou(ks,us)		((us)->sigbits[0] = *(ks),	\
				 (us)->sigbits[1] = 0,	\
				 (us)->sigbits[2] = 0,	\
				 (us)->sigbits[3] = 0)

#endif /* !defined(_POSIX_SOURCE) */ 
typedef struct {
	int	sig;
	int	perm;
	int	checkperm;
} sigsend_t;

#if !defined(_POSIX_SOURCE) 
#if defined(__STDC__)
extern void setsigact(int, void (*)(), k_sigset_t, int);
#else
extern void setsigact();
#endif	/* __STDC__ */
#endif /* !defined(_POSIX_SOURCE) */ 

#endif /* _KERNEL */

#endif /* _SYS_SIGNAL_H */
