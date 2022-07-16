/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mb1:uts/i386/master.d/console/space.c	1.3"

#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"		/* definitins for console switch */
#include "config.h"		/* etc/conf/cf.d/config.h */
#include "sys/at_ansi.h"	/* hack to get AT sepcific structures */

struct attrmask kdstrmap;
struct attrmask kb_attrmask;
struct attrmask nattrmsks;

/********************************
 *
 * For MB1 and MB2, We took the link of /dev/console
 * (character major 1) and wrote routines that use the major number
 * in co_dev to goto the real console device.  Therefore, once this
 * is in place, a driver that can be a console not only has the usual
 * entries (open, close, read, ...) but also "co" and "ci" that the
 * conssw structure points to. 
 * 
 * One of the bad side effects of remapping the device numbers of
 * /dev/console onto the real device is that shell layers stops
 * working.  This is because the sxt driver expects to get to the
 * controlling terminal's tty structure by taking the major number
 * and finding the base of the tty array on cdevsw and then indexing
 * to the proper structure using the minor number.  The only real
 * fix is to repair sxt.
 *
 * Beware that you do not create a device node for the device
 * that is being used by /dev/console.  Closing is controlled by
 * the major/minor numbers.  If, for instance, you created both
 * /dev/console (1,0) and /dev/i354.0 (23,0) there would be two
 * major/minors for the same device and close would get confused.
 *
 ********************************/

#ifdef I8251
/* 386/2x and 386/3x onboard usart console driver for MB1 */
extern i8251co();
extern i8251ci();
struct conssw conssw = { 
			i8251co,
			(int)makedev(37,0), 
			i8251ci
			};
#else
struct conssw conssw;	/* Fill it in at boot-time */
#endif
