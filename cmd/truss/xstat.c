/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)truss:xstat.c	1.5.3.1"

#include <stdio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/fault.h>
#include <sys/syscall.h>
#include "pcontrol.h"
#include "ramdata.h"
#include "proto.h"

/*
 * For SVR4.0, we must include definitions for both stat and xstat.
 * The only way we can do that is to pretend we are kernel code.
 * This is not far off-base since truss(1) is very system-dependent.
 */
#define	_KERNEL 1
#include <sys/stat.h>


/*ARGSUSED*/
void
show_xstat(Pr, offset)
register process_t *Pr;
register long offset;
{
#ifndef SVR3	/* SVR4.0 xstat() */

	struct xstat statb;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&statb, sizeof(statb)) == sizeof(statb)) {
		(void) printf(
	"%s    d=0x%.8lX i=%-5lu m=0%.6lo l=%-2lu u=%-5lu g=%-5lu",
			pname,
			statb.st_dev,
			statb.st_ino,
			statb.st_mode,
			statb.st_nlink,
			statb.st_uid,
			statb.st_gid);

		switch (statb.st_mode&S_IFMT) {
		case S_IFCHR:
		case S_IFBLK:
			(void) printf(" rdev=0x%.8lX\n", statb.st_rdev);
			break;
		default:
			(void) printf(" sz=%lu\n", statb.st_size);
			break;
		}

		prtimestruc("at = ", statb.st_atime);
		prtimestruc("mt = ", statb.st_mtime);
		prtimestruc("ct = ", statb.st_ctime);

		(void) printf(
			"%s    bsz=%-5ld blks=%-5ld fs=%.*s\n",
			pname,
			statb.st_blksize,
			statb.st_blocks,
			_ST_FSTYPSZ,
			statb.st_fstype);
	}

#endif
}

void
show_stat(Pr, offset)
register process_t *Pr;
register long offset;
{
	struct stat statb;

	if (offset != NULL
	 && Pread(Pr, offset, (char *)&statb, sizeof(statb)) == sizeof(statb)) {
		(void) printf(
			"%s    d=0x%.4X i=%-5u m=0%.6o l=%-2u u=%-5u g=%-5u",
			pname,
			statb.st_dev&0xffff,
			statb.st_ino&0xffff,
			statb.st_mode&0xffff,
			statb.st_nlink&0xffff,
			statb.st_uid&0xffff,
			statb.st_gid&0xffff);

		switch (statb.st_mode&S_IFMT) {
		case S_IFCHR:
		case S_IFBLK:
			(void) printf(" rdev=0x%.4X\n", statb.st_rdev);
			break;
		default:
			(void) printf(" sz=%lu\n", statb.st_size);
			break;
		}

		prtime("at = ", statb.st_atime);
		prtime("mt = ", statb.st_mtime);
		prtime("ct = ", statb.st_ctime);
	}
}
