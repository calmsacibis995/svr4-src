/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <sys/types.h>
#include <sys/siginfo.h>
#if !defined(_POSIX_SOURCE) 
#include <sys/procset.h>
#endif /* !defined(_POSIX_SOURCE) */


#ident	"@(#)head.sys:sys/wait.h	1.12.3.1"

/*
 * arguments to wait functions
 */

#if !defined(_POSIX_SOURCE) 
#define WEXITED		0001	/* wait for processes that have exited	*/
#define WTRAPPED	0002	/* wait for processes stopped while tracing */
#define WSTOPPED	0004	/* wait for processes stopped by signals */
#define WCONTINUED	0010	/* wait for processes continued */
#endif /* !defined(_POSIX_SOURCE) */

#define WUNTRACED	0004	/* for POSIX */

#define WNOHANG		0100	/* non blocking form of wait	*/

#if !defined(_POSIX_SOURCE) 
#define WNOWAIT		0200	/* non destructive form of wait */

#define WOPTMASK	(WEXITED|WTRAPPED|WSTOPPED|WCONTINUED|WNOHANG|WNOWAIT)

/*
 * macros for stat return from wait functions
 */

#define WCONTFLG		0177777
#define WCOREFLG		0200

#define WWORD(stat)		((int)((stat))&0177777)
#endif /* !defined(_POSIX_SOURCE) */

#if !defined(_POSIX_SOURCE) 
#define WSTOPFLG		0177
#define WSIGMASK		0177
#define WLOBYTE(stat)		((int)((stat)&0377))
#define WHIBYTE(stat)		((int)(((stat)>>8)&0377))
#endif /* !defined(_POSIX_SOURCE) */ 

#define WIFEXITED(stat)		(((int)((stat)&0377))==0)
#define WIFSIGNALED(stat)	(((int)((stat)&0377))>0&&((int)(((stat)>>8)&0377))==0)
#define WIFSTOPPED(stat)	(((int)((stat)&0377))==0177&&((int)(((stat)>>8)&0377))!=0)

#if !defined(_POSIX_SOURCE) 
#define WIFCONTINUED(stat)	(WWORD(stat)==WCONTFLG)
#endif /* !defined(_POSIX_SOURCE) */

#define WEXITSTATUS(stat)	((int)(((stat)>>8)&0377))
#define WTERMSIG(stat)		(((int)((stat)&0377))&0177)
#define WSTOPSIG(stat)		((int)(((stat)>>8)&0377))

#if !defined(_POSIX_SOURCE) 
#define WCOREDUMP(stat)		((stat)&WCOREFLG)
#endif /* !defined(_POSIX_SOURCE) */



#if !defined(_KERNEL)
#if defined(__STDC__)

extern pid_t wait(int *);
extern pid_t waitpid(pid_t, int *, int);

#if !defined(_POSIX_SOURCE) 
extern int waitid(idtype_t, id_t, siginfo_t *, int);
#endif /* !defined(_POSIX_SOURCE) */

#else

extern pid_t wait();
extern pid_t waitpid();

#if !defined(_POSIX_SOURCE) 
extern int waitid();
#endif /* !defined(_POSIX_SOURCE) */

#endif	/* __STDC__ */
#endif	/* _KERNEL */

#endif	/* _SYS_WAIT_H */
