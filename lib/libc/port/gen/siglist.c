/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/siglist.c	1.2"

#include "synonyms.h"
#include <signal.h>

int _sys_nsig = NSIG;

char *_sys_siglist[NSIG] = {
		"UNKNOWN SIGNAL",
		"Hangup",			/* SIGHUP 	*/
		"Interrupt",			/* SIGINT	*/
		"Quit",				/* SIGQUIT	*/
		"Illegal Instruction",		/* SIGILL	*/
		"Trace/Breakpoint Trap",	/* SIGTRAP	*/
		"Abort",			/* SIGABRT	*/
		"Emulation Trap",		/* SIGEMT	*/
		"Arithmetic Exception",		/* SIGFPE	*/
		"Killed",			/* SIGKILL	*/
		"Bus Error",			/* SIGBUS	*/
		"Segmentation Fault",		/* SIGSEGV	*/
		"Bad System Call",		/* SIGSYS	*/
		"Broken Pipe",			/* SIGPIPE	*/
		"Alarm Clock",			/* SIGALRM	*/
		"Terminated",			/* SIGTERM	*/
		"User Signal 1",		/* SIGUSR1	*/
		"User Signal 2",		/* SIGUSR2	*/
		"Child Status Changed",		/* SIGCLD	*/
		"Power-Fail/Restart",		/* SIGPWR	*/
		"Window Size Change",		/* SIGWINCH	*/
		"Urgent Socket Condition",	/* SIGURG	*/
		"Pollable Event",		/* SIGPOLL	*/
		"Stopped (signal)",		/* SIGSTOP	*/
		"Stopped (user)",		/* SIGTSTP	*/
		"Continued",			/* SIGCONT	*/
		"Stopped (tty input)",		/* SIGTTIN	*/
		"Stopped (tty output)",		/* SIGTTOU	*/
		"Virtual Timer Expired",	/* SIGVTALRM	*/
		"Profiling Timer Expired",	/* SIGPROF	*/
		"Cpu Limit Exceeded",		/* SIGXCPU	*/
		"File Size Limit Exceeded"	/* SIGXFSZ	*/
};
