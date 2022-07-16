/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _STDARG_H
#define _STDARG_H

#ident	"@(#)head:stdarg.h	1.8"

#if defined(__STDC__)

#ifndef _VA_LIST
#define _VA_LIST
typedef void *va_list;
#endif

#if __STDC__ != 0				/* -Xc compilation */
#define va_start(list, name) (void) (list = \
 	(void *)((char *)&name + ((sizeof(name)+(sizeof(int)-1)) & ~(sizeof(int)-1))))
#else
#define va_start(list, name) (void) (list = (void *)((char *)&...))
#endif 	/* != 0 */

#if #machine(u370)
#define va_arg(list, mode) ((mode *)(list = \
        (char *) ((int)list + 2*sizeof(mode) - 1 & -sizeof(mode))))[-1]
#else
#define va_arg(list, mode) ((mode *)(list = (char *)list + sizeof(mode)))[-1]
#endif	/* u370 */

extern void va_end(va_list);

#define va_end(list) (void)0

#else	/* not __STDC__ */

#include <varargs.h>

#endif	/* __STDC__ */

#endif 	/* _STDARG_H */
