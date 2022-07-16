/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _STDDEF_H
#define _STDDEF_H

#ident	"@(#)head:stddef.h	1.5"

typedef int 	ptrdiff_t;

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int	size_t;
#endif 

#ifndef NULL
#define NULL            0
#endif

#ifndef _WCHAR_T
#define _WCHAR_T
typedef long	wchar_t;
#endif

#define offsetof(s, m)	(size_t)(&(((s *)0)->m))

#endif 	/* _STDDEF_H */
