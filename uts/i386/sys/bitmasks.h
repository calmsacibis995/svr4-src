/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_BITMASKS_H
#define _SYS_BITMASKS_H

#ident	"@(#)head.sys:sys/bitmasks.h	11.2.7.1"

/*	setmask[i] has the low order i bits set.  For example,
 *	setmask[5] == 0x1F.
 */

extern int setmask[33];

/*	sbittab[i] has bit number i set.  For example,
 *	sbittab[5] == 0x20.
 */

extern int sbittab[];

/*	cbittab[i] has all bits on except bit i which is off.  For example,
 *	cbittab[5] == 0xFFFFFFDF.
 */

extern int cbittab[];

#endif	/* _SYS_BITMASKS_H */
