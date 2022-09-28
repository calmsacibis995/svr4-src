/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:lib/mpwlib/setsig.c	6.6"
# include	"signal.h"
# include	"sys/types.h"
# include	"macros.h"

#define ONSIG	16

/*
	General-purpose signal setting routine.
	All non-ignored, non-caught signals are caught.
	If a signal other than hangup, interrupt, or quit is caught,
	a "user-oriented" message is printed on file descriptor 2 with
	a number for help(I).
	If hangup, interrupt or quit is caught, that signal	
	is set to ignore.
	Termination is like that of "fatal",
	via "clean_up(sig)" (sig is the signal number)
	and "exit(userexit(1))".
 
	If the file "dump.core" exists in the current directory
	the function commits
	suicide to produce a core dump
	(after calling clean_up, but before calling userexit).
*/


static char	*Mesg[ONSIG]={
	0,
	0,	/* Hangup */
	0,	/* Interrupt */
	0,	/* Quit */
	"Illegal instruction",
	"Trace/BPT trap",
	"IOT trap",
	"EMT trap",
	"Floating exception",
	"Killed",
	"Bus error",
	"Memory fault",
	"Bad system call",
	"Broken pipe",
	"Alarm clock"
};

static void	setsig1();

void
setsig()
{
	register int j;
	register void (*n)();

	for (j=1; j<ONSIG; j++)
		if (n=signal(j,setsig1))
			signal(j,n);
}


static char preface[]="SIGNAL: ";
static char endmsg[]=" (ut12)\n";

static void
setsig1(sig)
int sig;
{
	int userexit(), open(), write();
	unsigned int	strlen();
	void	abort(), clean_up(), exit();

	if (Mesg[sig]) {
		(void) write(2,preface,length(preface));
		(void) write(2,Mesg[sig],length(Mesg[sig]));
		(void) write(2,endmsg,length(endmsg));
	}
	else
		signal(sig,SIG_IGN);
	clean_up(sig);
	if(open("dump.core",0) > 0) {
		signal(SIGIOT,0);
		abort();
	}
	exit(userexit(1));
}
