/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/i386/signal.h	1.1"

/*LOCAL COPY*/

#ifndef SIGNALH
#define SIGNALH

#include <sys/signal.h>

#ifdef c_plusplus
typedef int (*SIG_PF) (...);

extern SIG_PF signal(int, SIG_PF);
extern SIG_PF sigset(int, SIG_PF);
extern SIG_PF ssignal(int, SIG_PF);
extern int sighold(int);
extern int sigrelse(int);
extern int sigignore(int);
extern int sigpause(int);

extern int gsignal (int);
extern int kill (int, int);
#else
typedef int (*SIG_PF)();
extern	SIG_PF signal();
extern  SIG_PF sigset();
#endif

#undef SIG_ERR
#undef SIG_DFL
#undef SIG_IGN
#undef SIG_HOLD
#define SIG_ERR	(SIG_PF)-1
#define	SIG_DFL	(SIG_PF)0
#define	SIG_IGN	(SIG_PF)1
#define SIG_HOLD (SIG_PF)2

#endif
