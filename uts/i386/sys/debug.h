/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_DEBUG_H
#define _SYS_DEBUG_H

#ident	"@(#)head.sys:sys/debug.h	11.11.5.1"

#define	YES 1
#define	NO  0

#if DEBUG == YES

#if defined(__STDC__)
extern int assfail(char *, char *, int);
#define ASSERT(EX) ((void)((EX) || assfail(#EX, __FILE__, __LINE__)))
#else
extern int assfail();
#define ASSERT(EX) ((void)((EX) || assfail("EX", __FILE__, __LINE__)))
#endif	/* end __STDC__ */

#define DB_ISKV(A) ((unsigned)(A) >= KVBASE && (unsigned)(A) < UVUBLK)

#else	/* else DEBUG */

#define ASSERT(x)
#define DB_ISKV(A) 1
#endif	/* end DEBUG */

#ifdef MONITOR
#define MONITOR(id, w1, w2, w3, w4) monitor(id, w1, w2, w3, w4)
#else
#define MONITOR(id, w1, w2, w3, w4)
#endif

#define STATIC

#endif	/* _SYS_DEBUG_H */
