/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:core.h	1.9.1.2"

/* machine dependent stuff for core files */

#if defined(__STDC__)

#if #machine(vax)
#define TXTRNDSIZ 512L
#define stacktop(siz) (0x80000000L)
#define stackbas(siz) (0x80000000L-siz)

#elif #machine(pdp11)
#define TXTRNDSIZ 8192L
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)

#elif #machine(u3b)
#define TXTRNDSIZ 0x20000
#define stacktop(siz) 0xF00000
#define stackbas(siz) (0xF00000 + siz)

#elif #machine(i386)
#define TXTRNDSIZ 0x400000
#define stacktop(siz)	(0x80000000L - siz)
#define stackbas(siz) 	0x80000000L

#else
#define TXTRNDSIZ 2048L
#define stacktop(siz) (0xF00000 + siz)
#define stackbas(siz) 0xF00000
#endif

#else
#if vax
#define TXTRNDSIZ 512L
#define stacktop(siz) (0x80000000L)
#define stackbas(siz) (0x80000000L-siz)
#endif

#if pdp11
#define TXTRNDSIZ 8192L
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)
#endif

#if u3b
#define TXTRNDSIZ 0x20000
#define stacktop(siz) 0xF00000
#define stackbas(siz) (0xF00000 + siz)
#endif

#if M32 || u3b15 || u3b5 || u3b2
#define TXTRNDSIZ 2048L
#define stacktop(siz) (0xF00000 + siz)
#define stackbas(siz) 0xF00000
#endif

#if i386
#define TXTRNDSIZ 0x400000
#define stacktop(siz)	(0x80000000L - siz)
#define stackbas(siz) 	0x80000000L
#endif

#endif /* __STDC__ */
