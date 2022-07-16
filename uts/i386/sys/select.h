/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

#ident	"@(#)head.sys:sys/select.h	1.2.3.1"

/*
 * Select uses bit masks of file descriptors in longs.
 * These macros manipulate such bit fields.
 * FD_SETSIZE may be defined by the user, but the default here
 * should be >= NOFILE (param.h).
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE	1024
#endif

#ifndef NBBY		/* number of bits per byte */
#define NBBY 8
#endif

typedef	long	fd_mask;
#define	NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#ifndef	howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#ifdef _KERNEL
#define	FD_ZERO(p)	bzero((char *)(p), sizeof(*(p)))
#else
#define	FD_ZERO(p)	memset((char *)(p), 0, sizeof(*(p)))
#endif /* _KERNEL */

#endif	/* _SYS_SELECT_H */
