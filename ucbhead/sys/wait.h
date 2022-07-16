/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/wait.h	1.1.3.1"

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
 * This wait.h is a combination of SunOS's wait.h and SysV wait.h
 * The structure 'union wait' is taken from SunOS, while the
 * rest of the #define's are from SysV.
 */

/*
 * Structure of the information in the first word returned by both
 * wait and wait3.  If w_stopval==WSTOPPED, then the second structure
 * describes the information returned, else the first.  See WUNTRACED below.
 */
union wait	{
	int	w_status;		/* used in syscall */
	/*
	 * Terminated process status.
	 */
	struct {
#if defined(vax) || defined(i386)
		unsigned short	w_Termsig:7;	/* termination signal */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
#else
		unsigned short	w_Fill1:16;	/* high 16 bits unused */
		unsigned short	w_Retcode:8;	/* exit code if w_termsig==0 */
		unsigned short	w_Coredump:1;	/* core dump indicator */
		unsigned short	w_Termsig:7;	/* termination signal */
#endif
	} w_T;
	/*
	 * Stopped process status.  Returned
	 * only for traced children unless requested
	 * with the WUNTRACED option bit.
	 */
	struct {
#if defined(vax) || defined(i386)
		unsigned short	w_Stopval:8;	/* == W_STOPPED if stopped */
		unsigned short	w_Stopsig:8;	/* signal that stopped us */
#else
		unsigned short	w_Fill2:16;	/* high 16 bits unused */
		unsigned short	w_Stopsig:8;	/* signal that stopped us */
		unsigned short	w_Stopval:8;	/* == W_STOPPED if stopped */
#endif
	} w_S;
};
#define	w_termsig	w_T.w_Termsig
#define w_coredump	w_T.w_Coredump
#define w_retcode	w_T.w_Retcode
#define w_stopval	w_S.w_Stopval
#define w_stopsig	w_S.w_Stopsig

/* ----- begin SysV wait.h ----- */

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include "sys/types.h"
#include "sys/siginfo.h"
#include "sys/procset.h"

/*
 * arguments to wait functions
 */

#define WEXITED		0001	/* wait for processes that have exite	*/
#define WTRAPPED	0002	/* wait for processes stopped while tracing */
#define WSTOPPED	0004	/* wait for processes stopped by signals */
#define WCONTINUED	0010	/* wait for processes continued */

#define WUNTRACED	WSTOPPED /* for POSIX */

#define WNOHANG		0100	/* non blocking form of wait	*/
#define WNOWAIT		0200	/* non destructive form of wait */

#define WOPTMASK	(WEXITED|WTRAPPED|WSTOPPED|WCONTINUED|WNOHANG|WNOWAIT)

/*
 * macros for stat return from wait functions
 */

#define WSTOPFLG		0177
#define WCONTFLG		0177777
#define WCOREFLG		0200
#define WSIGMASK		0177

#define WLOBYTE(stat)		((int)((stat)&0377))
#define WHIBYTE(stat)		((int)(((stat)>>8)&0377))
#define WWORD(stat)		((int)((stat))&0177777)

#define WIFEXITED(stat)		(WLOBYTE(stat)==0)
#define WIFSIGNALED(stat)	(WLOBYTE(stat)>0&&WHIBYTE(stat)==0)
#define WIFSTOPPED(stat)	(WLOBYTE(stat)==WSTOPFLG&&WHIBYTE(stat)!=0)
#define WIFCONTINUED(stat)	(WWORD(stat)==WCONTFLG)

#define WEXITSTATUS(stat)	WHIBYTE(stat)
#define WTERMSIG(stat)		(WLOBYTE(stat)&WSIGMASK)
#define WSTOPSIG(stat)		WHIBYTE(stat)
#define WCOREDUMP(stat)		((stat)&WCOREFLG)



#if !defined(_KERNEL)
#if defined(__STDC__)

extern pid_t wait(int *);
extern pid_t waitpid(pid_t, int *, int);
extern int waitid(idtype_t, id_t, siginfo_t *, int);

#else

extern pid_t wait();
extern pid_t waitpid();
extern int waitid();

#endif	/* __STDC__ */
#endif	/* _KERNEL */

#endif	/* _SYS_WAIT_H */
