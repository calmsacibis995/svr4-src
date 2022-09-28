/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/signal.h	1.1"
/*ident	"@(#)cfront:incl/signal.h	1.10"*/

#ifndef SIGNALH
#define SIGNALH

#include<sys/signal.h>

typedef void	SIG_FUNC_TYPE(int);
typedef SIG_FUNC_TYPE*	SIG_PF;

#undef 		SIG_ERR
#undef 		SIG_DFL
#undef 		SIG_IGN
#undef 		SIG_HOLD
#define 	SIG_ERR  (SIG_PF)-1
#define       	SIG_DFL  (SIG_PF)0
#define       	SIG_IGN  (SIG_PF)1
#define 	SIG_HOLD (SIG_PF)2

extern SIG_PF signal(int, SIG_PF);
extern SIG_PF sigset(int, SIG_PF);
extern SIG_PF ssignal(int, SIG_PF);

extern int gsignal (int);
extern int kill (int, int);

#endif

