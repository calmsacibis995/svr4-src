/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cpio:cpiostat.c	1.1.2.1"
#define	_STYPES
#include <sys/types.h>
#include <sys/stat.h>

struct cpioinfo {
	o_dev_t st_dev;
	o_ino_t st_ino;
	o_mode_t	st_mode;
	o_nlink_t	st_nlink;
	o_uid_t st_uid;
	o_gid_t st_gid;
	o_dev_t st_rdev;
	off_t	st_size;
	time_t	st_modtime;
} tmpinfo;


struct cpioinfo *
svr32stat(fname)
char *fname;
{
	struct stat stbuf;

	if (stat(fname, &stbuf) < 0) {
		stbuf.st_ino = 0;
	}
	tmpinfo.st_dev = stbuf.st_dev;
	tmpinfo.st_ino = stbuf.st_ino;
	tmpinfo.st_mode = stbuf.st_mode;
	tmpinfo.st_nlink = stbuf.st_nlink;
	tmpinfo.st_uid = stbuf.st_uid;
	tmpinfo.st_gid = stbuf.st_gid;
	tmpinfo.st_rdev = stbuf.st_rdev;
	tmpinfo.st_size = stbuf.st_size;
	tmpinfo.st_modtime = stbuf.st_mtime;
	return(&tmpinfo);
}

struct cpioinfo *
svr32lstat(fname)
char *fname;
{
	struct stat stbuf;

	if(!lstat(fname, &stbuf)) {
		tmpinfo.st_dev = stbuf.st_dev;
		tmpinfo.st_ino = stbuf.st_ino;
		tmpinfo.st_mode = stbuf.st_mode;
		tmpinfo.st_nlink = stbuf.st_nlink;
		tmpinfo.st_uid = stbuf.st_uid;
		tmpinfo.st_gid = stbuf.st_gid;
		tmpinfo.st_rdev = stbuf.st_rdev;
		tmpinfo.st_size = stbuf.st_size;
		tmpinfo.st_modtime = stbuf.st_mtime;
		return(&tmpinfo);
	}
	tmpinfo.st_ino = 0;
	return(&tmpinfo);
}
