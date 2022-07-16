/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:memory.h	1.4.1.2"

/* from stddef.h */
#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned        size_t;
#endif

#if defined(__STDC__)
extern void *memccpy(void *, const void *, int, size_t);
extern void *memchr(const void *, int, size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
extern int memcmp(const void *, const void *, size_t);

#else
extern char
	*memccpy(),
	*memchr(),
	*memcpy(),
	*memset();
extern int memcmp();
#endif
