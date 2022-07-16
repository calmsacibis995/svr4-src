/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SIGACTION_H
#define _SYS_SIGACTION_H

#ident	"@(#)head.sys:sys/sigaction.h	1.1.3.1"
#ifndef _SIGACTION_
#define _SIGACTION_

struct sigaction {
	int sa_flags;
	void (*sa_handler)();
	sigset_t sa_mask;
	int sa_resv[2];
};
#endif	/* _SIGACTION_ */

/* definitions for the sa_flags field */

#define SA_ONSTACK	0x00000001
#define SA_RESETHAND	0x00000002
#define SA_RESTART	0x00000004
#define SA_SIGINFO	0x00000008
#define SA_NODEFER	0x00000010

/* these are only valid for SIGCLD */
#define SA_NOCLDWAIT	0x00010000	/* don't save zombie children	 */
#define SA_NOCLDSTOP	0x00020000	/* don't send job control SIGCLD's */

#endif /* _SYS_SIGACTION_H */
