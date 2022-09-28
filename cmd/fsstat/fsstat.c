/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)fsstat:fsstat.c	1.3.1.1"

#include <stdio.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/fs/s5param.h>
#include <sys/fs/s5filsys.h>
#include <sys/stat.h>
#include <ustat.h>

/*
 * exit 0 - file system is unmounted and okay
 * exit 1 - file system is unmounted and needs checking
 * exit 2 - file system is mounted
 *           for root file system
 * exit 0 - okay
 * exit 1 - needs checking
 *
 * exit 3 - unexpected failures
 */
main(argc, argv)
char *argv[];
{
	register dev;
	register char *fp;
	struct filsys fb;
	struct stat stbd, stbr;
	struct ustat usb;

	if (argc != 2) {
		fprintf(stderr, "usage: fsstat special\n");
		exit(3);
	}
	fp = argv[1];
	if ((dev = open(fp, O_RDONLY)) < 0) {
		fprintf(stderr, "fsstat: cannot open %s\n", fp);
		exit(3);
	}
	fstat(dev, &stbd);
	if ((stbd.st_mode&S_IFMT) != S_IFBLK) {
		fprintf(stderr, "fsstat: %s not a block device\n", fp);
		exit(3);
	}
	stat("/", &stbr);

#ifdef i386
	lseek(dev, (long) SUPERBOFF, 0);
#else
	lseek(dev, SUPERBOFF, 0);
#endif 

	if (read(dev, &fb, sizeof fb) != sizeof fb) {
		fprintf(stderr, "fsstat: cannot read %s\n", fp);
		exit(3);
	}
	if (stbr.st_dev == stbd.st_rdev) {	/* root file system */
		if (fb.s_state != FsACTIVE) {
			fprintf(stderr, "fsstat: root file system needs checking\n");
			exit(1);
		} else {
			fprintf(stderr, "fsstat: root file system okay\n");
			exit(0);
		}
	}
	if (ustat(stbd.st_rdev, &usb) == 0) {
		fprintf(stderr, "fsstat: %s mounted\n", fp);
		exit(2);
	}
	if (fb.s_magic != FsMAGIC) {
		fprintf(stderr, "fsstat: %s not a valid file system\n", fp);
		exit(3);
	}
	if ((fb.s_state + (long)fb.s_time) != FsOKAY) {
		fprintf(stderr, "fsstat: %s needs checking\n", fp);
		exit(1);
	}
	fprintf(stderr, "fsstat: %s okay\n", fp);
	exit(0);
}

