/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _JBLEN

#ident	"@(#)head:setjmp.h	1.9.2.9"

#if defined(__STDC__)

#if #machine(i386)
#define _SIGJBLEN 128	/* (sizeof(ucontext_t) / sizeof (int)) */
#else
#define _SIGJBLEN 64	/* (sizeof(ucontext_t) / sizeof (int)) */
#endif

#if #machine(pdp11)
#define _JBLEN  3
#else 
#if #machine(u370)
#define _JBLEN  4
#else
#if #machine(u3b)
#define _JBLEN  11
#else   
#define _JBLEN  10
#endif	/* #machine */
#endif	/* #machine */
#endif	/* #machine */

#else 

#if i386
#define _SIGJBLEN 128	/* (sizeof(ucontext_t) / sizeof (int)) */
#else
#define _SIGJBLEN 64	/* (sizeof(ucontext_t) / sizeof (int)) */
#endif

#if pdp11
#define _JBLEN  3
#else 
#if u370
#define _JBLEN  4
#else
#if u3b
#define _JBLEN  11
#else   
#define _JBLEN  10
#endif
#endif
#endif

#endif	/* __STDC__ */

typedef int jmp_buf[_JBLEN];

#if defined(__STDC__)
extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

#if __STDC__ == 0 || defined(_POSIX_SOURCE) || defined(_XOPEN_SOURCE) /* non-ANSI standard compilation */
typedef int sigjmp_buf[_SIGJBLEN];

extern int sigsetjmp(sigjmp_buf, int);
extern void siglongjmp(sigjmp_buf, int);
#endif

#if __STDC__ != 0
#define setjmp(env)	setjmp(env)
#endif

#else
typedef int sigjmp_buf[_SIGJBLEN];

extern int setjmp();
extern void longjmp();
extern int sigsetjmp();
extern void siglongjmp();

#endif  /* __STDC__ */

#endif  /* _JBLEN */
