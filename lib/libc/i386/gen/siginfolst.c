/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-i386:gen/siginfolst.c	1.1"

#include "synonyms.h"
#include <signal.h>
#include <siginfo.h>


char * _sys_traplist[NSIGTRAP] = {
	"process breakpoint",
	"process trace"
};

char * _sys_illlist[NSIGILL] = {
	"illegal opcode",
	"illegal operand",
	"illegal addressing mode",
	"illegal trap",
	"privileged opcode",
	"privileged register",
	"co-processor",
	"bad stack"
};

char * _sys_fpelist[NSIGFPE] = {
	"integer divide by zero",
	"integer overflow",
	"floating point divide by zero",
	"floating point overflow",
	"floating point underflow",
	"floating point inexact result",
	"invalid floating point operation",
	"floating point subscript out of range"
};

char * _sys_segvlist[NSIGSEGV] = {
	"address not mapped to object",
	"invalid permissions"
};

char * _sys_buslist[NSIGBUS] = {
	"invalid address alignment",
	"non-existent physical address",
	"object specific"
};

char * _sys_cldlist[NSIGCLD] = {
	"child has exited",
	"child was killed",
	"child has coredumped",
	"traced child has trapped",
	"child has stopped",
	"stopped child has continued"
};

char * _sys_polllist[NSIGPOLL] = {
	"input available",
	"output possible",
	"message available",
	"I/O error",
	"high priority input available",
	"device disconnected"
};

struct siginfolist _sys_siginfolist[NSIG-1] = {
	0,		0,		/* SIGHUP */
	0,		0,		/* SIGINT */
	0,		0,		/* SIGQUIT */
	NSIGILL,	_sys_illlist,	/* SIGILL */
	NSIGTRAP,	_sys_traplist,	/* SIGTRAP */
	0,		0,		/* SIGABRT */
	0,		0,		/* SIGEMT */
	NSIGFPE,	_sys_fpelist,	/* SIGFPE */
	0,		0,		/* SIGKILL */
	NSIGBUS,	_sys_buslist,	/* SIGBUS */
	NSIGSEGV,	_sys_segvlist,	/* SIGSEGV */
	0,		0,		/* SIGSYS */
	0,		0,		/* SIGPIPE */
	0,		0,		/* SIGALRM */
	0,		0,		/* SIGTERM */
	0,		0,		/* SIGUSR1 */
	0,		0,		/* SIGUSR2 */
	NSIGCLD,	_sys_cldlist,	/* SIGCLD */
	0,		0,		/* SIGPWR */
	0,		0,		/* SIGWINCH */
	0,		0,		/* SIGURG */
	NSIGPOLL,	_sys_polllist,	/* SIGPOLL */
	0,		0,		/* SIGSTOP */
	0,		0,		/* SIGTSTP */
	0,		0,		/* SIGCONT */
	0,		0,		/* SIGTTIN */
	0,		0,		/* SIGTTOU */
	0,		0,		/* SIGVTALRM */
	0,		0,		/* SIGPROF */
	0,		0,		/* SIGXCPU */
	0,		0,		/* SIGXFSZ */
};
