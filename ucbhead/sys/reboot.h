/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbhead:sys/reboot.h	1.4.3.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 

#ifndef _VM_REBOOT_H
#define _VM_REBOOT_H

/*
 * Arguments to reboot system call and flags to init.
 *
 * On the VAX, these are passed to boot program in r11,
 * and on to init.
 *
 * On the Sun, these are parsed from the boot command line
 * and used to construct the argument list for init.
 */
#define	RB_AUTOBOOT	0	/* flags for system auto-booting itself */

#define	RB_ASKNAME	0x001	/* ask for file name to reboot from */
#define	RB_SINGLE	0x002	/* reboot to single user only */
#define	RB_NOSYNC	0x004	/* dont sync before reboot */
#define	RB_HALT		0x008	/* don't reboot, just halt */
#define	RB_INITNAME	0x010	/* name given for /etc/init */
#define	RB_NOBOOTRC	0x020	/* don't run /etc/rc.boot */
#define	RB_DEBUG	0x040	/* being run under debugger */
#define	RB_DUMP		0x080	/* dump system core */
#define	RB_WRITABLE	0x100	/* mount root read/write */
#define	RB_STRING	0x200	/* pass boot args to prom monitor */

#endif	/* _VM_REBOOT_H */
