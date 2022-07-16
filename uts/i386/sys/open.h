/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_OPEN_H
#define _SYS_OPEN_H

#ident	"@(#)head.sys:sys/open.h	11.4.3.1"
/* Some drivers need to be able to keep accurate records of open/close
 * calls to determine whether a device is still in use.  To allow this
 * open/close calls have been typed and the type is passed as a third
 * argument in open/close calls, as in:
 *	(*cdevsw[getmajor(dev)].d_open)(getminor(dev), flag, OTYP_CHR);
 * or
 *	(*cdevsw[getmajor(dev)].d_close)(getminor(dev), flag, OTYP_CHR);
 * Five types of open/close calls have been defined:
 * OTYP_BLK:	open/close of a block special file
 * OTYP_MNT:	open/close for mounting/unmounting a file system
 * OTYP_CHR:	open/close of a character special file
 * OTYP_SWP:	open/close of a swapping device.
 * OTYP_LYR:	open/close calls from a driver to another driver,
 *		without a file being open for the dev of the lower driver.
 *
 * The first four types of open/close calls obey the protocol rule
 * that many more opens may occur for a given minor(dev) for that type of open,
 * but a close call happens only on the last close of that dev.
 * This protocol allows a flag to be used (set by opens, cleared by closes)
 * to keep track of the state for a given minor device value.
 *
 * Calls of the fifth type (OTYP_LYR) must obey the protocol rule
 * that open and close call calls are always paired.  This protocol
 * permits several drivers to be layers above the same device driver.
 * A counter can be used for this protocol.
 *
 * The value OTYPCNT is defined for the purpose of declaring arrays
 * in drivers and for performing range checks (0 <= otyp < OTYPCNT)
 * on values passed.
 */

#define OTYPCNT		5
#define OTYP_BLK	0
#define OTYP_MNT	1
#define OTYP_CHR	2
#define OTYP_SWP	3
#define OTYP_LYR	4

#endif	/* _SYS_OPEN_H */
