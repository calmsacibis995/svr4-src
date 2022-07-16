/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_STRMDEP_H
#define _SYS_STRMDEP_H

#ident	"@(#)head.sys:sys/strmdep.h	1.3.6.1"

/*
 * This file contains all machine-dependent declarations
 * in STREAMS.
 */

/*
 * Copy data from one data buffer to another.
 * The addresses must be word aligned - if not, use bcopy!
 */

#if defined(u3b2) && !defined(lint)

/*
 * Use the MOVBLW instruction on the 3b2.  
 */
asm	void
strbcpy(s, d, c)
{
%mem	s,d,c;
	
	MOVW	s,%r0
	MOVW	d,%r1
	MOVW	c,%r2
	ADDW2	&3,%r2
	LRSW3	&2,%r2,%r2
	MOVBLW
}

#else

#define	strbcpy(s, d, c)	bcopy(s, d, c)

#endif

/*
 * save the address of the calling function on the 3b2 to
 * enable tracking of who is allocating message blocks
 */

#if defined (u3b2) && !defined(lint)

asm	void
saveaddr(funcp)
{
%mem	funcp;

	MOVW	-36(%fp),*funcp
}

#else

#define saveaddr(funcp)

#endif

/*
 * macro to check pointer alignment
 * (true if alignment is sufficient for worst case)
 */
#ifdef u3b2

#define str_aligned(X)	(((uint)(X) & 03) == 0)

#else

#define str_aligned(X)	(((uint)(X) & (sizeof(int) - 1)) == 0)

#endif


#endif	/* _SYS_STRMDEP_H */
