/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/setjmp.h	1.2"
/*ident	"@(#)cfront:incl/setjmp.h	1.5"*/

#ifndef SETJMPH
#define SETJMPH

#ifndef _JBLEN

#if vax | M32 | u3b15 | u3b5 | u3b2
#define _JBLEN	10
#endif

#if pdp11
#define _JBLEN	3
#endif

#if i386
#define _JBLEN	6
#endif

#if u370
#define _JBLEN	4
#endif

#if u3b
#define _JBLEN	11
#endif

#if uts
#undef _JBLEN
#define _JBLEN 14
#endif

typedef int jmp_buf[_JBLEN];

extern int setjmp(jmp_buf);
extern void longjmp(jmp_buf, int);

#endif
#endif

