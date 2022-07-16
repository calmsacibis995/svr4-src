/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:wait.h	1.1"

#ifndef _WAIT_H
#define _WAIT_H

#include <sys/types.h>
#include <sys/siginfo.h>
#include <sys/procset.h>
#include <sys/wait.h>

#if defined(__STDC__)

extern pid_t wait(int *);
extern pid_t waitpid(pid_t, int *, int);
extern int waitid(idtype_t, id_t, siginfo_t *, int);

#else

extern pid_t wait();
extern pid_t waitpid();
extern int waitid();

#endif

#endif 	/* _WAIT_H */
