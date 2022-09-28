/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * 		PROPRIETARY NOTICE (Combined)
 * 
 * This source code is unpublished proprietary information
 * constituting, or derived under license from AT&T's UNIX(r) System V.
 * In addition, portions of such source code were derived from Berkeley
 * 4.3 BSD under license from the Regents of the University of
 * California.
 * 
 * 
 * 
 * 		Copyright Notice 
 * 
 * Notice of copyright on this source code product does not indicate 
 * publication.
 * 
 * 	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
 * 	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 * 	          All rights reserved.
 *  
 */

#ident	"@(#)ufs.cmds:ufs/fsdb/fsdb.c	1.4.3.1"

/*
 * fsdb -z option only
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mntent.h>
#include <sys/vnode.h>
#include <sys/fs/ufs_inode.h>
#include <sys/fs/ufs_fs.h>

#define ISIZE	(sizeof(struct dinode))
#define	NI	(MAXBSIZE/ISIZE)

int	zflag = 0;		/* zero an inode */
int	errflag = 0;
extern	int	optind;

struct	dinode	buf[NI];

union {
	char		dummy[SBSIZE];
	struct fs	sblk;
} sb_un;
#define sblock sb_un.sblk

int	status;

main(argc, argv)
	int argc;
	char *argv[];
{
	register i, f;
	unsigned n;
	int j, k;
	long off;
	long gen;
	int c;

	while ((c = getopt (argc, argv, "z")) != EOF) {
		switch (c) {

		case 'z':		/* zero an inode */
			zflag++;
			break;

		case '?':
			errflag++;
			break;
		}
	}
	if (errflag) {
		usage();
		exit(31+4);
	}
	if (isnumber(argv[optind]))
		n = atoi(argv[optind]);
	else {
		usage();
		exit (31+1);
	}

	if (argc < optind) {
		usage ();
		exit(31+4);
	}
	optind++;
	f = open(argv[optind], 2);
	if (f < 0) {
		perror ("open");
		printf("cannot open %s\n", argv[optind]);
		exit(31+4);
	}
	lseek(f, SBLOCK * DEV_BSIZE, 0);
	if (read(f, &sblock, SBSIZE) != SBSIZE) {
		printf("cannot read %s\n", argv[1]);
		exit(31+4);
	}
	if (sblock.fs_magic != FS_MAGIC) {
		printf("bad super block magic number\n");
		exit(31+4);
	}
	if (n == 0) {
		printf("%s: is zero\n", n);
		exit(31+1);
	}
	off = fsbtodb(&sblock, itod(&sblock, n)) * DEV_BSIZE;
	lseek(f, off, 0);
	if (read(f, (char *)buf, sblock.fs_bsize) != sblock.fs_bsize) {
		printf("%s: read error\n", argv[i]);
		status = 1;
	}
	if (status)
		exit(31+status);
	printf("clearing %u\n", n);
	off = fsbtodb(&sblock, itod(&sblock, n)) * DEV_BSIZE;
	lseek(f, off, 0);
	read(f, (char *)buf, sblock.fs_bsize);
	j = itoo(&sblock, n);
	gen = buf[j].di_gen;
	bzero((caddr_t)&buf[j], ISIZE);
	buf[j].di_gen = gen + 1;
	lseek(f, off, 0);
	write(f, (char *)buf, sblock.fs_bsize);
	exit(31+status);
}

isnumber(s)
	char *s;
{
	register c;

	if (s == NULL)
		return(0);
	while(c = *s++)
		if (c < '0' || c > '9')
			return(0);
	return(1);
}

usage ()
{
	(void) fprintf (stderr, "ufs usage: fsdb [ -F ufs] [ -z i_number ] special\n");
}
