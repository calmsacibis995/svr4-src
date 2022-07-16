/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:assert.h	1.6.1.4"

#ifdef NDEBUG
#undef assert
#define assert(EX) ((void)0)

#else

#if defined(__STDC__)
extern void __assert(const char *, const char *, int);
#define assert(EX) (void)((EX) || (__assert(#EX, __FILE__, __LINE__), 0))

#else
extern void _assert();
#define assert(EX) (void)((EX) || (_assert("EX", __FILE__, __LINE__), 0))

#endif	/* __STDC__ */

#endif	/* NDEBUG */
