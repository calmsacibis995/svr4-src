/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_BYTEORDER_H
#define _SYS_BYTEORDER_H

#ident	"@(#)/usr/src/uts/i386/sys/byteorder.h.sl 1.1 4.0 12/08/90 52314 AT&T-USL"

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * macros for conversion between host and (internet) network byte order
 */

#if !defined(vax) && !defined(ntohl) && !defined(lint) && !defined(i386)
/* big-endian */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)

#elif defined(i386)

/*
 *	unsigned long htonl( hl )
 *	long hl;
 *	reverses the byte order of 'long hl'
 */

asm unsigned long htonl( hl )
{
%mem	hl;	
	movl	hl, %eax
	xchgb	%ah, %al
	rorl	$16, %eax
	xchgb	%ah, %al
	clc
}

/*
 *	unsigned long ntohl( nl )
 *	unsigned long nl;
 *	reverses the byte order of 'ulong nl'
 */

asm unsigned long ntohl( nl )
{
%mem	nl;
	movl	nl, %eax
	xchgb	%ah, %al
	rorl	$16, %eax
	xchgb	%ah, %al
	clc
}

/*
 *	unsigned short htons( hs )
 *	short hs;
 *
 *	reverses the byte order in hs.
 */

asm unsigned short htons( hs )
{
%mem	hs;
	movl	hs, %eax
	xchgb	%ah, %al
	clc
}

/*
 *	unsigned short ntohs( ns )
 *	unsigned short ns;
 *
 *	reverses the bytes in ns.
 */


asm unsigned short ntohs( ns )
{
%mem	ns;
	movl	ns, %eax
	xchgb	%ah, %al
	clc
}

#elif !defined(ntohl)  /* little-endian, not i386 */

u_short	ntohs(), htons();
u_long	ntohl(), htonl();

#endif

#endif /* _SYS_BYTEORDER_H */
