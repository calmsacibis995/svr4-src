/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libgenIO:g_init.c	1.4.1.1"

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mkdev.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ftw.h>
#include <libgenIO.h>

extern
char	*malloc();

static
char	*ctcpath;	/* Used to re-open file descriptor (3B2/CTC) */

static
struct stat	orig_st_buf;	/* Stat structure of original file (3B2/CTC) */
	
/*
 * g_init: Determine the device being accessed, set the buffer size,
 * and perform any device specific initialization.  For the 3B2,
 * a device with major number of 17 (0x11) is an internal hard disk,
 * unless the minor number is 128 (0x80) in which case it is an internal
 * floppy disk.  Otherwise, get the system configuration
 * table and check it by comparing slot numbers to major numbers.
 * For the special case of the 3B2 CTC several unusual things must be done.
 * First, the open file descriptor is stat(2)'d and ftw(3) is used to walk
 * through the /dev directory structure looking for the file coorresponding
 * to the open file descriptor.  This is necessary, because to enable
 * streaming mode on the CTC, the file descriptor must be closed, re-opened
 * (with O_RDWR and O_CTSPECIAL flags set), the STREAMON ioctl(2) command
 * issued, and the file descriptor re-re-opened either read-only or write_only.
 */

/*
 *	On '386-based machines, however, these 3b2-specific functions
 *	are not necessary, so we dispense with them.
 */


int
g_init(devtype, fdes)
int *devtype, *fdes;
{
	major_t maj;
	minor_t min;
	int bufsize, nflag, oflag, i, count, size;
	struct stat st_buf;
	struct statvfs stfs_buf;

	*devtype = G_NO_DEV;
	bufsize = -1;
	if (fstat(*fdes, &st_buf) == -1)
		return(-1);
	if (!(st_buf.st_mode & S_IFCHR) && !(st_buf.st_mode & S_IFBLK)) {
			if (st_buf.st_mode & S_IFIFO ) {
				bufsize = 512;
			} else {
           *devtype = G_FILE; /* find block size for this file system */
	            if (fstatvfs(*fdes, &stfs_buf) < 0) {
   	                 bufsize = -1;
   	                 errno = ENODEV;
   	         } else
   	             bufsize = stfs_buf.f_bsize;
				}
			return(bufsize);

	/* We'll have to add a remote attribute to stat but this
	** should work for now.
	*/
	} else if (st_buf.st_dev & 0x8000)      /* if remote  rdev */
			return (512);
		else
			*devtype = G_386_HD;	/* set it to something so cpio
						 * doesn't balk.
						 */

	maj = major(st_buf.st_rdev);
	min = minor(st_buf.st_rdev);

	switch (*devtype) {
			
		case G_FILE: /* find block size for this file system */
			if (fstatfs(*fdes, &stfs_buf, sizeof(struct statvfs), 0) < 0) {
				bufsize = -1;
				errno = ENODEV;
			} else
				bufsize = stfs_buf.f_bsize;
			break;

		default:
			bufsize = 5120;
	} /* *devtype */
	return(bufsize);
}
