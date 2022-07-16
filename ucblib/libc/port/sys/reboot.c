/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


/* 	Portions Copyright(c) 1988, Sun Microsystems Inc.	*/
/*	All Rights Reserved					*/

#ident	"@(#)ucblibc:port/gen/reboot.c	1.2.3.1"

#include <sys/uadmin.h>
#include <sys/errno.h>

extern errno;

/*
 * Not all BSD's semantics are supported
 * including RB_SINGLE, RB_RB_DUMP, RB_STRING
 */

/*
 * BSD reboot.h
 */
#define RB_AUTOBOOT     0       /* flags for system auto-booting itself */

#define RB_ASKNAME      0x001   /* ask for file name to reboot from */
#define RB_SINGLE       0x002   /* reboot to single user only */
#define RB_NOSYNC       0x004   /* dont sync before reboot */
#define RB_HALT         0x008   /* don't reboot, just halt */
#define RB_INITNAME     0x010   /* name given for /etc/init */
#define RB_NOBOOTRC     0x020   /* don't run /etc/rc.boot */
#define RB_DEBUG        0x040   /* being run under debugger */
#define RB_DUMP         0x080   /* dump system core */
#define RB_WRITABLE     0x100   /* mount root read/write */
#define RB_STRING       0x200   /* pass boot args to prom monitor */

reboot(howto, bootargs)
        int     howto;
        char    *bootargs;
{
        int     cmd;
        int     fcn;
        int     mdep;

        if ( getuid() != 0 ) {
                errno = EPERM;
                return -1;
        }

        if ( howto & RB_HALT ) {
                cmd = A_SHUTDOWN;
                fcn = AD_HALT;
        } else if ( howto & RB_ASKNAME ) {
                cmd = A_SHUTDOWN;
                fcn = AD_IBOOT;
        } else {                /* assume RB_AUTOBOOT */
                cmd = A_SHUTDOWN;
                fcn = AD_BOOT;
        }

        (void) uadmin(cmd, fcn, mdep);
}       


