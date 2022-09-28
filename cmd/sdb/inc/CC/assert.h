/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/CC/assert.h	1.1"
#ifndef ASSERTH
#define ASSERTH

/*ident	"@(#)cfront:incl/assert.h	1.3" */
#ifdef NDEBUG
#define assert(EX)
#else
extern void _assert(char*, char*, int);
#ifdef __STDC__
#define assert(EX) if (EX) ; else _assert(#EX, __FILE__, __LINE__)
#else
#define assert(EX) if (EX) ; else _assert("EX", __FILE__, __LINE__)
#endif /* __STDC__ */
#endif

#endif
