/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-port:gen/str2sig.c	1.1"
#ifdef __STDC__
	#pragma weak str2sig = _str2sig
	#pragma weak sig2str = _sig2str
#endif
#include "synonyms.h"
#include <signal.h>

static struct signame {
	char *sigstr;
	int   signum;
} signames[] = {
	{ "EXIT",	0 },
	{ "HUP",	SIGHUP },
	{ "INT",	SIGINT },
	{ "QUIT",	SIGQUIT },
	{ "ILL",	SIGILL },
	{ "TRAP",	SIGTRAP },
	{ "ABRT",	SIGABRT },
	{ "IOT",	SIGIOT },
	{ "EMT",	SIGEMT },
	{ "FPE",	SIGFPE },
	{ "KILL",	SIGKILL },
	{ "BUS",	SIGBUS },
	{ "SEGV",	SIGSEGV },
	{ "SYS",	SIGSYS },
	{ "PIPE",	SIGPIPE },
	{ "ALRM",	SIGALRM },
	{ "TERM",	SIGTERM },
	{ "USR1",	SIGUSR1 },
	{ "USR2",	SIGUSR2 },
	{ "CLD",	SIGCLD },
	{ "CHLD",	SIGCHLD },
	{ "PWR",	SIGPWR },
	{ "WINCH",	SIGWINCH },
	{ "URG",	SIGURG },
	{ "POLL",	SIGPOLL },
	{ "IO",		SIGPOLL },
	{ "STOP",	SIGSTOP },
	{ "TSTP",	SIGTSTP },
	{ "CONT",	SIGCONT },
	{ "TTIN",	SIGTTIN },
	{ "TTOU",	SIGTTOU },
	{ "VTALRM",	SIGVTALRM },
	{ "PROF",	SIGPROF },
	{ "XCPU",	SIGXCPU },
	{ "XFSZ",	SIGXFSZ },
};

#define SIGCNT (sizeof(signames)/sizeof(struct signame))

str2sig(s,sigp)
char *s;
int *sigp;
{
	register struct signame *sp;
	
	if (*s >= '0' && *s <= '9') {
		int i = atoi(s);
		for (sp = signames; sp < &signames[SIGCNT]; sp++) {
			if (sp->signum == i) {
				*sigp = sp->signum;
				return (0);
			}
		}
		return(-1);
	} else {
		for (sp = signames; sp < &signames[SIGCNT]; sp++) {
			if (strcmp(sp->sigstr,s) == 0) {
				*sigp = sp->signum;
				return (0);
			}
		}
		return(-1);
	}
}

sig2str(i,s)
int i;
char *s;
{
	register struct signame *sp;
	
	for (sp = signames; sp < &signames[SIGCNT]; sp++) {
		if (sp->signum == i) {
			strcpy(s,sp->sigstr);
			return (0);
		}
	}
	return (-1);
}
