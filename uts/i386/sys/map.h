/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_MAP_H
#define _SYS_MAP_H

#ident	"@(#)head.sys:sys/map.h	11.6.4.1"

/*
 *		struct map	X[]	.m_size		.m_addr
 *				---	------------	-----------
 *				[0]	mapsize(X)	mapwant(X)
 *					# X[] unused	sleep value
 *
 *		  mapstart(X)->	[1]	# units		unit number
 *				 :	    :		  :
 *				[ ]	    0
 */

struct map
{
	unsigned long m_size;	/* number of units available */
	unsigned long m_addr;	/* address of first available unit */
};

extern struct map sptmap[];	/* Map for system virtual space.   */

#define	mapstart(X)	&X[1]		/* start of map array */
#define	mapwant(X)	X[0].m_addr
#define	mapsize(X)	X[0].m_size	/* number of empty slots \
					   remaining in map array */
#define	mapdata(X) {(X)-2, 0} , {0, 0}
#define	mapinit(X, Y)	X[0].m_size = (Y)-2


#if defined(__STDC__)
extern void rmfree(struct map *, size_t, u_long);
extern void mfree(struct map *, size_t, u_long);
extern u_long rmalloc(struct map *, size_t);
extern u_long malloc(struct map *, size_t);
#else
extern void rmfree();
extern void mfree();
extern u_long malloc();
extern u_long rmalloc();
#endif

#endif	/* _SYS_MAP_H */
