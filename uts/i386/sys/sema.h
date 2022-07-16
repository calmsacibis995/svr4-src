/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SEMA_H
#define _SYS_SEMA_H

#ident	"@(#)head.sys:sys/sema.h	11.1.8.1"
/*
 * defines for semaphore mapping
 */

typedef int	sema_t;

#define ALLOC_LOCK(X)
#define EXTERN_LOCK(X)
#define INITLOCK(X,Y)
#define SPSEMA(X)
#define SVSEMA(X)
#define	PSEMA(X,Y)
#define	VSEMA(X,Y)

#define appsema(a,b)	1
#define apvsema(a,b)	1
#define psema(a,b)	sleep(a,b)
#define vsema(a,b)	wakeup(a)
#define initsema(a,b)	*a = b
#define initlock(a,b)	*a = b
#define cvsema(a)	wakeup(a)
#define splrf()		spl5()

#endif	/* _SYS_SEMA_H */
