/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ucbhead:sys/signal.h	1.6.4.1"
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
#ifndef _UCB_SYS_SIGNAL_H
#define _UCB_SYS_SIGNAL_H

/*
 * 4.3BSD signal compatibility header
 *
 * this file includes all standard SVR4 header info, plus the 4.3BSD
 * structures  - 4.3BSD signal codes are translated to SVR4 generic
 * signal codes where applicable
 */

/*
 * SysV <signal.h>
 */

/* ---- k16 <signal.h> ---- */

/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SIGNAL_H
#define _SIGNAL_H


typedef int 	sig_atomic_t;

extern char *_sys_siglist[];
extern int _sys_nsig;

/* ---- k16 <sys/signal.h> ---- */

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SIGNAL_H
#define _SYS_SIGNAL_H


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

#if (__STDC__ != 1)	
#define	NSIG	32	/* valid signals range from 1 to NSIG-1 */
#define MAXSIG	32	/* size of u_signal[], NSIG-1 <= MAXSIG */
#endif

#define S_SIGNAL	1
#define S_SIGSET	2
#define S_SIGACTION	3
#define S_NONE		4

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

#if defined(lint)
#define SIG_ERR (void(*)())0
#define	SIG_IGN	(void (*)())0
#define SIG_HOLD (void(*)())0
#else
#define SIG_ERR	(void(*)())-1
#define	SIG_IGN	(void (*)())1
#define SIG_HOLD (void(*)())2
#endif
#define	SIG_DFL	(void(*)())0

typedef struct {		/* signal set type */
	unsigned long	word[4];
} sigset_t;

#ifndef _SYS_SIGACTION_H
#define _SYS_SIGACTION_H
struct sigaction {
        int sa_flags;
        void (*sa_handler)();
        sigset_t sa_mask;
        int sa_resv[2];
};


/* definitions for the sa_flags field */

#define SA_ONSTACK	0x00000001
#define SA_RESETHAND	0x00000002
#define SA_RESTART	0x00000004
#define SA_SIGINFO	0x00000008
#define SA_NODEFER	0x00000010

/* these are only valid for SIGCLD */
#define SA_NOCLDWAIT	0x00010000	/* don't save zombie children	 */
#define SA_NOCLDSTOP	0x00020000	/* don't send job control SIGCLD's */
#endif

struct sigaltstack {
	char	*ss_sp;
	int	ss_size;	/* number of words of type (int *) */
	int	ss_flags;
};

typedef struct sigaltstack stack_t;

#define MINSIGSTKSZ	(512/sizeof(int *))
#define SIGSTKSZ	(8192/sizeof(int *))

#define SS_ONSTACK	0x00000001 /* this is current execution stack */
#define SS_DISABLE	0x00000002 /* this stack cannot be used */

#define SIGNO_MASK	0xFF
#define SIGDEFER	0x100
#define SIGHOLD		0x200
#define SIGRELSE	0x400
#define SIGIGNORE	0x800
#define SIGPAUSE	0x1000

#ifdef _KERNEL 

extern k_sigset_t	
	fillset,		/* set of valid signals,
				 * guaranteed contiguous */
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
#define sigisempty(s)		(*(s) == 0)
#define	sigismember(s,n)	(sigmask(n) & *(s))
#define sigorset(s1,s2)		(*(s1) |= *(s2))
#define	sigandset(s1,s2)	(*(s1) &= *(s2))
#define	sigdiffset(s1,s2)	(*(s1) &= ~(*(s2)))
#define	sigintset(s1,s2)	(*(s1) & *(s2))
#define sigutok(us,ks)		(*(ks) = (us)->word[0])
#define sigktou(ks,us)		((us)->word[0] = *(ks),	\
				 (us)->word[1] = 0,	\
				 (us)->word[2] = 0,	\
				 (us)->word[3] = 0)

typedef struct {
	int	sig;
	int	perm;
	int	checkperm;
} sigsend_t;

#if defined(__STDC__)
extern void setsigact(int, void (*)(), k_sigset_t, int);
#else
extern void setsigact();
#endif	/* __STDC__ */

#endif /* _KERNEL */

#endif /* _SYS_SIGNAL_H */
/* ---- end of SysV <sys/signal.h> ---- */

#if defined(__STDC__)

extern void (*signal(int, void (*)(int)))(int);
extern int raise(int);

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE)
/* extern int kill(pid_t, int); */
extern int sigaction(int, struct sigaction *, struct sigaction *);
extern int sigaddset(sigset_t *, int);
extern int sigdelset(sigset_t *, int);
extern int sigemptyset(sigset_t *);
extern int sigfillset(sigset_t *);
extern int sigismember(sigset_t *, int);
extern int sigpending(sigset_t *);
extern int sigprocmask(int, sigset_t *, sigset_t *);
extern int sigsuspend(sigset_t *);
#endif

#if __STDC__ == 0
extern int gsignal(int);
extern void (*sigset(int, void (*)(int)))(int);
extern int sighold(int);
extern int sigrelse(int);
extern int sigignore(int);
extern int sigpause(int);
extern int (*ssignal(int, int (*)(int)))(int);
#endif

#else
extern	void(*signal())();
extern  void(*sigset())();

#endif 	/* __STDC__ */

#endif 	/* _SIGNAL_H */
/* ---- end of SysV <signal.h> ---- */

#define sigmask(m)	(m > 32 ? 0 : (1 << ((m)-1)))

/*
 * 4.3BSD structure used in sigstack call.
 */

struct  sigstack {
        char    *ss_sp;                 /* signal stack pointer */
        int     ss_onstack;             /* current status */
};

/*
 * 4.3BSD signal vector structure used in sigvec call.
 */
struct  sigvec {
        void    (*sv_handler)();        /* signal handler */
        int     sv_mask;                /* signal mask to apply */
        int     sv_flags;               /* see signal options below */
};

#define SV_ONSTACK      0x0001  /* take signal on signal stack */
#define SV_INTERRUPT    0x0002  /* do not restart system on signal return */
#define SV_RESETHAND    0x0004  /* reset handler to SIG_DFL when signal taken */

#define sv_onstack sv_flags

struct  sigcontext {
        int     sc_onstack;             /* sigstack state to restore */
        int     sc_mask;                /* signal mask to restore */
#ifdef u3b2
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_pc;                  /* pc to restore */
        int   	sc_ps;                  /* psw to restore */
#endif
#ifdef vax
        int     sc_sp;                  /* sp to restore */
        int     sc_fp;                  /* fp to restore */
        int     sc_ap;                  /* ap to restore */
        int     sc_pc;                  /* pc to restore */
        int     sc_ps;                  /* psl to restore */
#endif /* vax */
#ifdef mc68000
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_ps;                  /* psl to restore */
#endif /* mc68000 */
#ifdef sparc
#define MAXWINDOW       31              /* max usable windows in sparc */
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_npc;                 /* next pc to restore */
        int     sc_psr;                 /* psr to restore */
        int     sc_g1;                  /* register that must be restored */
        int     sc_o0;
        int     sc_wbcnt;               /* number of outstanding windows */
        char    *sc_spbuf[MAXWINDOW];   /* sp's for each wbuf */
        int     sc_wbuf[MAXWINDOW][16]; /* outstanding window save buffer */
#endif /* sparc */
#if defined(i386) || defined(sun386)
        int     sc_sp;                  /* sp to restore */
        int     sc_pc;                  /* pc to retore */
        int     sc_ps;                  /* psl to restore */
        int     sc_eax;                 /* eax to restore */
        int     sc_edx;                 /* edx to restore */
#endif
};

#define SI_DFLCODE	1

#ifdef vax
#define ILL_RESAD_FAULT	ILL_ILLADR	/* reserved addressing fault */
#define ILL_PRIVIN_FAULT ILL_PRVOPC	/* privileged instruction fault */
#define ILL_RESOP_FAULT	ILL_ILLOPC	/* reserved operand fault */

#define FPE_INTOVF_TRAP	FPE_INTOVF	/* integer overflow */
#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* floating overflow */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* floating/decimal divide by zero */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* floating underflow */
#define FPE_DECOVF_TRAP	FPE_INTOVF	/* decimal overflow */
#define FPE_SUBRNG_TRAP	FPE_FLTSUB	/* subscript out of range */
#define FPE_FLTOVF_FAULT FPR_FLTOVF	/* floating overflow fault */
#define FPE_FLTDIV_FAULT FPE_FLTDIV	/* divide by zero floating fault */
#define FPE_FLTUND_FAULT FPE_FLTUND	/* floating underflow fault */

#endif /* vax */

#ifdef mc68000

#define ILL_ILLINSTR_FAULT ILL_ILLOPC	/* illegal instruction fault */
#define ILL_PRIVVIO_FAULT ILL_PRVREG	/* privilege violation fault */
#define ILL_COPROCERR_FAULT ILL_COPERR	/* [coprocessor protocol error fault] */
#define ILL_TRAP1_FAULT	ILL_ILLTRP	/* trap #1 fault */
#define ILL_TRAP2_FAULT	ILL_ILLTRP	/* trap #2 fault */
#define ILL_TRAP3_FAULT	ILL_ILLTRP	/* trap #3 fault */
#define ILL_TRAP4_FAULT	ILL_ILLTRP	/* trap #4 fault */
#define ILL_TRAP5_FAULT	ILL_ILLTRP	/* trap #5 fault */
#define ILL_TRAP6_FAULT	ILL_ILLTRP	/* trap #6 fault */
#define ILL_TRAP7_FAULT	ILL_ILLTRP	/* trap #7 fault */
#define ILL_TRAP8_FAULT	ILL_ILLTRP	/* trap #8 fault */
#define ILL_TRAP9_FAULT	ILL_ILLTRP	/* trap #9 fault */
#define ILL_TRAP10_FAULT ILL_ILLTRP	/* trap #10 fault */
#define ILL_TRAP11_FAULT ILL_ILLTRP	/* trap #11 fault */
#define ILL_TRAP12_FAULT ILL_ILLTRP	/* trap #12 fault */
#define ILL_TRAP13_FAULT ILL_ILLTRP	/* trap #13 fault */
#define ILL_TRAP14_FAULT ILL_ILLTRP	/* trap #14 fault */

#define EMT_EMU1010	SI_DFLCODE	/* line 1010 emulator trap */
#define EMT_EMU1111	SI_DFLCODE	/* line 1111 emulator trap */

#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_CHKINST_TRAP SI_DFLCODE	/* CHK [CHK2] instruction */
#define FPE_TRAPV_TRAP	SI_DFLCODE	/* TRAPV [cpTRAPcc TRAPcc] instr */
#define FPE_FLTBSUN_TRAPSI_DFLCODE	/* [branch or set on unordered cond] */
#define FPE_FLTINEX_TRAP FPE_FLTRES	/* [floating inexact result] */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* [floating divide by zero] */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* [floating underflow] */
#define FPE_FLTOPERR_TRAP FPE_FLTINV	/* [floating operand error] */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* [floating overflow] */
#define FPE_FLTNAN_TRAP	FPE_FLTINV	/* [floating Not-A-Number] */

#ifdef sun
#define FPE_FPA_ENABLE	SI_DFLCODE	/* [FPA not enabled] */
#define FPE_FPA_ERROR	SI_DFLCODE	/* [FPA arithmetic exception] */
#endif /* sun */

#endif /* mc68000 */

#ifdef sparc

#define ILL_STACK	ILL_STKERR	/* bad stack */
#define ILL_ILLINSTR_FAULT ILL_ILLOPC	/* illegal instruction fault */
#define ILL_PRIVINSTR_FAULT ILL_PRVOPC	/* privileged instruction fault */
#define ILL_TRAP_FAULT(n) ILL_ILLTRP	 /* trap n fault */

#define	EMT_TAG		SI_DFLCODE	/* tag overflow */

#define FPE_INTOVF_TRAP	FPE_INTOVF	/* integer overflow */
#define FPE_INTDIV_TRAP	FPE_INTDIV	/* integer divide by zero */
#define FPE_FLTINEX_TRAP FPE_FLTRES	/* [floating inexact result] */
#define FPE_FLTDIV_TRAP	FPE_FLTDIV	/* [floating divide by zero] */
#define FPE_FLTUND_TRAP	FPE_FLTUND	/* [floating underflow] */
#define FPE_FLTOPERR_TRAP FPE_FLTSUB	/* [floating operand error] */
#define FPE_FLTOVF_TRAP	FPE_FLTOVF	/* [floating overflow] */

#endif /* sparc */

#define BUS_HWERR	BUS_ADRERR	/* misc hardware error (e.g. timeout) */
#define BUS_ALIGN	BUS_ADRALN	/* hardware alignment error */

#define SEGV_NOMAP	SEGV_MAPERR	/* no mapping at the fault address */
#define SEGV_PROT	SEGV_ACCERR	/* access exceeded protections */

/*
 * The SEGV_CODE(code) will be SEGV_NOMAP, SEGV_PROT, or SEGV_OBJERR.
 * In the SEGV_OBJERR case, doing a SEGV_ERRNO(code) gives an errno value
 * reported by the underlying file object mapped at the fault address.
 */

#define SIG_NOADDR	((char *)~0)

#define	SEGV_CODE(fc)	((fc) & 0xff)
#define	SEGV_ERRNO(fc)	((unsigned)(fc) >> 8)
#define	SEGV_MAKE_ERR(e) (((e) << 8) | SEGV_MAPERR)

#if defined(lint)
#define BADSIG (void(*)())0
#else
#define BADSIG (void(*)())-1
#endif

#endif /*_UCB_SYS_SIGNAL_H*/
