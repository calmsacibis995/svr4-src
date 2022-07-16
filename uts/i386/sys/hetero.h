/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_HETERO_H
#define _SYS_HETERO_H

#ident	"@(#)head.sys:sys/hetero.h	11.5.6.1"

/*
 *	Define machine attributes for heterogeneity.
 *
 *	Also define macros for determining the resultant size of a
 *	conversion.
 *
 *	Machine attribute consists of three components -
 *	byte ordering, alignment, and data unit size.
 *	The machine attributes are defined in a byte (8 bits),
 *	the lower 2 bits are used to define the byte ordering,
 *	the middle 3 bits are used to define the alignment,
 *	the higher 3 bits are used to define the data unit size.
 *
 *
 *	BYTE_ORDER	0x01	3B, IBM byte ordering
 *			0x02	VAX byte ordering
 *	ALIGNMENT	0x04	word aligned (4 bytes boundary)
 *			0x08	half-word aligned (2 bytes boundary)
 *			0x0c	byte aligned
 *	UNIT_SIZE	0x20	4 bytes integer, 2 bytes short, 4 bytes pointer
 *			0x40	2 bytes integer, 2 bytes short, 2 bytes pointer
 */


/*
 *	Define masks for machine attributes
 */

#define BYTE_MASK	0x03
#define ALIGN_MASK	0x1c
#define UNIT_MASK	0xe0


/*
 *	Define what need to be converted - header or data parts
 */

#define ALL_CONV	0	/* convert both header and data parts */
#define DATA_CONV	1	/* convert data part */
#define NO_CONV		2	/* no conversion needed */



#ifdef u3b2
/*
 *	Define machine attributes for 3B
 */

#define BYTE_ORDER	0x01
#define ALIGNMENT	0x04
#define UNIT_SIZE	0x20
#endif

#ifdef i286
/*
 *	Define machine attributes for Intel 286
 */

#define BYTE_ORDER	0x02
#define ALIGNMENT	0x08
#define UNIT_SIZE	0x40
#endif

#ifdef i386
/*
 *	Define machine attributes for Intel 386
 */

#define BYTE_ORDER	0x02
#define ALIGNMENT	0x04
#define UNIT_SIZE	0x20
#endif


#define MACHTYPE	(BYTE_ORDER | ALIGNMENT | UNIT_SIZE)


/*
 *	Define macros for determining the size in bytes of the result of a
 * 	conversion.
 *
 */

/* 	A c0 conversion produces a buffer aligned on long boundary both at
 *	the beginning and at the end, containing a long for the character
 *	string length (including a NULL) and the character string itself
 *	(including a NULL).
 */

#define C0SIZE(stringlen) (stringlen) + 2 * (sizeof(long) - 1) + \
	    sizeof(long) + sizeof(char)

#endif		/* _SYS_HETERO_H */
