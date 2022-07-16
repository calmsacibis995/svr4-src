/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:ucontext.h	1.2"

#ifndef _UCONTEXT_H
#define _UCONTEXT_H

#include <sys/ucontext.h>

#if defined(__STDC__)

extern int getcontext(ucontext_t *);
extern int setcontext(ucontext_t *);
extern int swapcontext(ucontext_t *, ucontext_t *);
extern void makecontext(ucontext_t *, void(*)(), int, ...);

#else

extern int getcontext();
extern int setcontext();
extern int swapcontext();
extern void makecontext();

#endif

#endif 	/* _UCONTEXT_H */
