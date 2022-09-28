/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ksh:include/brkincr.h	1.7.3.1"
/*
 *	UNIX shell
 *	S. R. Bourne
 *	Rewritten by David Korn
 *
 */

#if u370 || uts
#define BRKINCR		4096
#else
#define BRKINCR		((int)(0400*sizeof(char*)))
#endif	/* u370 */
#ifdef INT16
#define BRKMAX		((unsigned)(077777-BRKINCR))
#else
#define BRKMAX		(1024*BRKINCR)
#endif /* INT16 */
#define ERRTRAP		(MAXTRAP-2)
#define DEBUGTRAP	(MAXTRAP-1)

#ifndef SIGCHLD
#   ifdef SIGCLD
#	define SIGCHLD SIGCLD
#   endif
#endif

#define SIGBITS		8

#undef SIGIGNORE
#define SIGIGNORE	1	/* signal handler set to ignore */
#define TRAPSET		2	/* signal received and trap is set */
#define SIGSET		4	/* signal received and trap not set */
#define SIGMOD		8	/* modified signal handler */
#undef SIGCAUGHT
#define SIGCAUGHT	16	/* default action is to catch signal */
#undef SIGOFF
#define SIGOFF		32	/* signal ignored by parent */
#undef SIGDONE
#define SIGDONE		64	/* default action is clean up and exit */
#undef SIGFAULT
#define SIGFAULT	128	/* signal handler set to sh_fault */
#undef SIGFAIL
#define SIGSLOW		256	/* slow signal received */
#define SIGBEGIN	512	/* set during signal initialization */
#define SIGFAIL		0600
#undef SIGFLG
#define SIGFLG		0200
#define EXITMASK	0377

#ifdef __STDC__
    extern void 	sh_fault(int);
    extern void 	sh_done(int);
    extern void 	sh_chktrap(void);
    extern void 	sh_exit(int);
    extern void 	sh_addmem(int);
    extern void 	sh_addblok(int);
    extern void 	sig_clear(int);
    extern int		sig_ignore(int);
    extern void 	sig_init(void);
    extern void 	sig_ontrap(int);
    extern void 	sig_reset(int);
#else
    extern void 	sh_fault();
    extern void 	sh_done();
    extern void 	sh_chktrap();
    extern void 	sh_exit();
    extern void 	sh_addmem();
    extern void 	sh_addblok();
    extern void 	sig_clear();
    extern int		sig_ignore();
    extern void 	sig_init();
    extern void 	sig_ontrap();
    extern void 	sig_reset();
    extern void 	sig_funset();
#endif /* PROTO */

extern const char		e_space[];
extern const struct sysnod	sig_messages[];
extern const struct sysnod	sig_names[];
